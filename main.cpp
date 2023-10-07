#include <QtWidgets>
#include <cstdlib>
#include <ctime>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QtConcurrent>
QMutex mutex;

const int SudokuSize = 9;

// 函数用于检查数独中某个位置是否合法
bool isValid(int row, int col, int num, int sudoku[9][9]) {
    // 检查行
    for (int i = 0; i < 9; ++i) {
        if (sudoku[row][i] == num) {
            return false;
        }
    }

    // 检查列
    for (int i = 0; i < 9; ++i) {
        if (sudoku[i][col] == num) {
            return false;
        }
    }

    // 检查3x3小宫格
    int startRow = row / 3 * 3;
    int startCol = col / 3 * 3;
    int endRow = startRow + 3;
    int endCol = startCol + 3;
    for (int i = startRow; i < endRow; i++)
    {
        for (int j = startCol; j < endCol; j++) {
            if (sudoku[i][j] == num) {
                return false;
            }
        }
    }

    return true;
}

void swapCol(int m, int n, int sudoku[9][9])
{
    int temp[9];
    for (int i = 0; i < 9; i++)
    {
        temp[i] = sudoku[i][m];
        sudoku[i][m] = sudoku[i][n];
        sudoku[i][n] = temp[i];
    }
}
void swapRow(int m, int n, int sudoku[9][9])
{
    int temp[9];
    for(int i = 0; i < 9; i++)
    {
        temp[i] = sudoku[m][i];
        sudoku[m][i] = sudoku[n][i];
        sudoku[n][i] = temp[i];
    }
}

bool dfs(int sudoku[9][9], int start)//从０开始依次遍历81个格子，计算此数独
{
    if (start == 81)//start=81，说明已经成功解出数独
    {
        return true;
    }
    else
    {
        bool ok = false;
        int row = start / 9;//根据此时方格的序号，计算出此方格的行和列
        int col = start % 9;
        if (sudoku[row][col] == 0)
        {
            for (int i = 1; i <= 9; i++)
            {
                if (isValid(row, col, i, sudoku))//从１－９依次放入空格，并判断是否合法
                {
                    sudoku[row][col] = i;//如果有数字合法，就写入该数字的字符
                    ok = dfs(sudoku, start + 1);//判断此方格的下一个方格是否成功写入
                    if (!ok)//如果它的下一个方格是不合法的，说明它现在填入的数，不是正确的解，需回溯
                    {
                        sudoku[row][col] = 0;//回溯
                    }
                }
            }
        }
        else
        {
            ok = dfs(sudoku, start + 1);
        }
        return ok;
    }
}
//随机挖空数独
void randomRemoveNumbers(int sudoku[9][9], int numToRemove) {
    srand(static_cast<unsigned>(time(0))); // 初始化随机数生成器

    for (int i = 0; i < numToRemove; ++i) {
        int row = rand() % 9;
        int col = rand() % 9;
        sudoku[row][col] = 0;
    }
}
// 函数用于生成一个合法的数独
void Createsudoku(int sudoku[9][9], QRandomGenerator &randomGenerator) {
    // 初始化数独为0，表示空格
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            sudoku[i][j] = 0;
        }
    }
    dfs(sudoku, 0);

    // 随机打乱数独的数字顺序
    int choice[9][2] = { {0,1},{0,2},{1,2},{3,4},{3,5},{4,5},{6,7},{6,8},{7,8} };
    for (int j = 0; j < 3; j++)//Ｊ代表交换次数，也可以不用这个循环，就交换一次
    {
       int i = randomGenerator.bounded(9);
        swapRow(choice[i][0], choice[i][1], sudoku);//随机交换两行
        swapCol(choice[i][0], choice[i][1], sudoku);//随机交换两列
    }
}


class SudokuGeneratorThread : public QThread {
public:
    int sudoku[SudokuSize][SudokuSize];
    QRandomGenerator randomGenerator = QRandomGenerator::securelySeeded();; // 每个线程都有自己的随机生成器

    SudokuGeneratorThread() {
        randomGenerator = QRandomGenerator::securelySeeded(); // 在构造函数中初始化随机生成器
    }

protected:
    void run() override {
        Createsudoku(sudoku, randomGenerator);
        randomRemoveNumbers(sudoku, randomGenerator.bounded(30, 70)); // 使用bounded函数生成随机数
    }
};
void solveSudoku(int sudoku[9][9]) {
    // 创建原数独的副本
    int solvedSudoku[9][9];
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            solvedSudoku[i][j] = sudoku[i][j];
        }
    }

    if (dfs(solvedSudoku, 0)) {
        // 如果找到解决方案，则将解应用于原始数独网格
        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                sudoku[i][j] = solvedSudoku[i][j];
            }
        }
    }
}
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    QWidget *centralWidget = new QWidget(&mainWindow);
    mainWindow.setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    QPushButton *generateButton = new QPushButton("生成");
    QPushButton *checkButton = new QPushButton("检查");
    QPushButton *solveButton = new QPushButton("显示题解");
    leftLayout->addWidget(generateButton);
    leftLayout->addWidget(checkButton);
    leftLayout->addWidget(solveButton);

    QGridLayout *rightLayout = new QGridLayout;
    QList<QTableWidget*> sudokuGrids;
    SudokuGeneratorThread threads[9];

    for (int i = 0; i < 9; ++i) {
        QTableWidget *sudokuGrid = new QTableWidget(SudokuSize, SudokuSize);
        sudokuGrid->horizontalHeader()->setVisible(false);
        sudokuGrid->verticalHeader()->setVisible(false);
        sudokuGrid->setShowGrid(true);
        sudokuGrid->setSelectionMode(QAbstractItemView::NoSelection);

        for (int row = 0; row < SudokuSize; ++row) {
            sudokuGrid->setRowHeight(row, 25);
            for (int col = 0; col < SudokuSize; ++col) {
                sudokuGrid->setColumnWidth(col, 25);
            }
        }

        sudokuGrids.append(sudokuGrid);
        rightLayout->addWidget(sudokuGrid, i / 3, i % 3);
    }

    QObject::connect(generateButton, &QPushButton::clicked, [&threads, &sudokuGrids]() {
        QList<QFuture<void>> futures;
        for (int i = 0; i < 9; ++i) {
            futures.append(QtConcurrent::run([&threads, i, &sudokuGrids]() {
                threads[i].start();
                threads[i].wait();
                QTableWidget *sudokuGrid = sudokuGrids[i];
                sudokuGrid->clear();
                for (int row = 0; row < SudokuSize; ++row) {
                    for (int col = 0; col < SudokuSize; ++col) {
                        int value = threads[i].sudoku[row][col];
                        QTableWidgetItem *item = new QTableWidgetItem();
                        if (value != 0) {
                            item->setText(QString::number(value)); // Set the text of the QTableWidgetItem
                        }
                        sudokuGrid->setItem(row, col, item);
                    }
                }
            }));
        }

        for (int i = 0; i < 9; ++i) {
            futures[i].waitForFinished();
        }
    });
    QObject::connect(solveButton, &QPushButton::clicked, [&threads, &sudokuGrids]() {
    for (int i = 0; i < 9; ++i) {
        QTableWidget *sudokuGrid = sudokuGrids[i];
        for (int row = 0; row < SudokuSize; ++row) {
            for (int col = 0; col < SudokuSize; ++col) {
                QTableWidgetItem *item = sudokuGrid->item(row, col);
                if (item) {
                    item->setText(""); // 清除已有的文本
                }
            }
        }
        solveSudoku(threads[i].sudoku);
        for (int row = 0; row < SudokuSize; ++row) {
            for (int col = 0; col < SudokuSize; ++col) {
                int value = threads[i].sudoku[row][col];
                QTableWidgetItem *item = new QTableWidgetItem();
                if (value != 0) {
                    item->setText(QString::number(value));
                }
                sudokuGrid->setItem(row, col, item);
            }
        }
    }
});
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    mainWindow.setWindowTitle("数独游戏");
    mainWindow.setGeometry(300, 50, 795, 715);

    mainWindow.show();

    return app.exec();
}







