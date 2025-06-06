#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      manager(new QNetworkAccessManager(this)),
      settings(new QSettings("history.ini", QSettings::IniFormat, this))
{
    ui->setupUi(this);
    connect(ui->getWeatherBtn, &QPushButton::clicked, this, &MainWindow::on_getWeatherBtn_clicked);
    connect(manager, &QNetworkAccessManager::finished, this, &MainWindow::onResult);
    loadSettings();
}

MainWindow::~MainWindow() {
    saveSettings();
    delete ui;
}

void MainWindow::on_getWeatherBtn_clicked() {
    QString city = ui->cityLineEdit->text();
    QString range = getRangeCount(ui->rangeComboBox->currentIndex());
    QString apiKey = "YOUR_API_KEY";  // Замените на ваш ключ!

    QString url = QString("https://api.openweathermap.org/data/2.5/forecast?q=%1&appid=%2&units=metric&cnt=%3")
                    .arg(city)
                    .arg(apiKey)
                    .arg(range);

    manager->get(QNetworkRequest(QUrl(url)));
}

QString MainWindow::getRangeCount(int index) {
    switch(index) {
        case 0: return "1";   // Сегодня
        case 1: return "8";   // Завтра (8 промежутков по 3ч = 24ч)
        case 2: return "24";  // 3 дня (72ч / 3ч = 24 записей)
        default: return "1";
    }
}

void MainWindow::onResult(QNetworkReply *reply) {
    if (reply->error()) {
        ui->resultTextEdit->setText("Ошибка: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    if (!root.contains("list")) {
        QString message = root.contains("message") ? root["message"].toString() : "Нет данных";
        ui->resultTextEdit->setText("Ошибка: " + message);
        return;
    }

    QJsonArray list = root["list"].toArray();
    QString result;

    for (const QJsonValue &val : list) {
        QJsonObject obj = val.toObject();
        QString dt_txt = obj["dt_txt"].toString();

        // Оставляем только утро/день/вечер: 09:00, 15:00, 21:00
        if (!isTargetHour(dt_txt)) continue;

        double temp = obj["main"].toObject()["temp"].toDouble();
        QString weather = obj["weather"].toArray()[0].toObject()["description"].toString();

        result += QString("%1
Температура: %2°C
Погода: %3

")
                    .arg(dt_txt)
                    .arg(temp)
                    .arg(weather);
    }

    ui->resultTextEdit->setText(result);
    updateHistory(result);
}

bool MainWindow::isTargetHour(const QString &dateTimeStr) {
    return dateTimeStr.contains("09:00:00") ||
           dateTimeStr.contains("15:00:00") ||
           dateTimeStr.contains("21:00:00");
}

void MainWindow::loadSettings() {
    ui->cityLineEdit->setText(settings->value("city", "").toString());
    ui->rangeComboBox->setCurrentIndex(settings->value("rangeIndex", 0).toInt());
    history = settings->value("history").toStringList();
    ui->historyTextEdit->setPlainText(history.join("
-----------------
"));
}

void MainWindow::saveSettings() {
    settings->setValue("city", ui->cityLineEdit->text());
    settings->setValue("rangeIndex", ui->rangeComboBox->currentIndex());
    settings->setValue("history", history);
}

void MainWindow::updateHistory(const QString &entry) {
    history.append(entry);
    if (history.size() > 10) history.removeFirst();
    ui->historyTextEdit->setPlainText(history.join("
-----------------
"));
}
