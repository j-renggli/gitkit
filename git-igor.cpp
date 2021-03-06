#include <iostream>

#include <QtCore/QFile>
#include <QtWidgets/QApplication>

#include "mainwin.h"

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication app(argc, argv);

    gitigor::MainWin window;
    window.resize(1024, 768);
    // window.setMinimumSize(400, 300);
    if (!window.initialise())
        return 1;

    QFile stylesheet(window.style().absoluteFilePath());
    stylesheet.open(QFile::ReadOnly);
    QString sheet = QLatin1String(stylesheet.readAll());

    app.setStyleSheet(sheet);

    window.show();
    return app.exec();
}
