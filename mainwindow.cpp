#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QIODevice>
#include <QTemporaryDir>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

QTemporaryDir tmpDir;
QProcess process;
bool busy = false;
QString esptool = "";
QString fileName;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    on_pushButton_3_clicked();
    ui->baud->addItem("921000");
    ui->baud->addItem("115200");
    ui->baud->addItem("57600");
    ui->baud->addItem("38400");
    ui->baud->addItem("28800");
    ui->baud->addItem("19200");
    ui->baud->addItem("14400");
    ui->baud->addItem("9600");
}

MainWindow::~MainWindow()
{
    delete ui;
    tmpDir.remove();
}

void MainWindow::on_pushButton_clicked()
{
    QMessageBox msgBox;
    if (busy) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("device is busy");
        msgBox.exec();
        return;
    }

    QString port = ((ui->port->currentText()).indexOf("cu.") > -1) ? ("/dev/" + ui->port->currentText()) : ui->port->currentText();
    QString sector = ui->sector->text();
    QString baud = ui->baud->currentText();

    if (esptool == "") {
        QString res_esptool = ":/new/prefix1/esptool.py";
        if (tmpDir.isValid()) {
            QString tmp = tmpDir.path() + "/esptool.py";
            if (QFile::copy(res_esptool, tmp)) {
                esptool = tmp;
            } else {
                ui->statusbar->showMessage(":(");
            }
        }
    }

    if ((port == "") || (sector == "") || (fileName == "") || (esptool == "")) {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText("check fields and repeat, please");
        msgBox.exec();
        return;
    }

    QString cmd = "python " + esptool + " --port " + port + " --baud " + baud + " write_flash " + sector + " " + fileName;

    busy = true;
    ui->progressBar->setValue(0);
    process.start(cmd);
    process.waitForStarted();
    connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(rtr()));
}

void MainWindow::rtr() {
    QString strresult = QString::fromLocal8Bit(process.readAllStandardOutput());
    ui->progressBar->setRange(0, 100);
    if (strresult.split('\n')[0].indexOf("Writing at ") > -1) {
        QString ps = (strresult.split(' ')[3]).remove(0, 1);
        int p = ps.toInt();
        if (p == 100) {
            busy = false;
        }
        ui->progressBar->setValue(p);
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QMessageBox msgBox;
    if (busy) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("device is busy");
        msgBox.exec();
        return;
    }

    ui->port->clear();
    QList<QSerialPortInfo> com_ports = QSerialPortInfo::availablePorts();
    QSerialPortInfo port;
    foreach(port, com_ports) {
        ui->port->addItem(port.portName());
    }

    int index = ui->port->findText("cu.SLAB_USBtoUART");
    if ( index != -1 ) {
       ui->port->setCurrentIndex(index);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QMessageBox msgBox;
    if (busy) {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("device is busy");
        msgBox.exec();
        return;
    }

    QString fullFileName = QFileDialog::getOpenFileName(this, tr("Open Firmware"), "", tr("Binary Files (*.bin)"));
    if (fullFileName.length() == 0) {
        return;
    }

    fileName = fullFileName;
    QStringList qst = fullFileName.split("/");
    QString fileName = qst[qst.size() - 1];
    ui->fileName->setText(fileName);
}
