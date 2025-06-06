#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_getWeatherBtn_clicked();
    void onResult(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QSettings *settings;
    QStringList history;

    QString getRangeCount(int index);
    void loadSettings();
    void saveSettings();
    void updateHistory(const QString &entry);
    bool isTargetHour(const QString &dateTimeStr);
};

#endif // MAINWINDOW_H
