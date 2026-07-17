/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "DocumentOperations.hpp"

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>

#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/MobileApp.hpp>

#include <Poco/Path.h>
#include <Poco/URI.h>

#include <filesystem>
#include <system_error>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPrinterInfo>
#include <QPushButton>
#include <QString>

#include <cstdlib>

std::vector<SaveAsFormat> getSaveAsFormats(int docType)
{
    std::vector<SaveAsFormat> formats;

    if (docType == KIT_DOCTYPE_TEXT)
    {
        formats = {
            {QStringLiteral("saveas-odt"), QStringLiteral("odt"), QObject::tr("ODF text document (.odt)")},
            {QStringLiteral("saveas-rtf"), QStringLiteral("rtf"), QObject::tr("Rich Text (.rtf)")},
            {QStringLiteral("saveas-docx"), QStringLiteral("docx"), QObject::tr("Word Document (.docx)")},
            {QStringLiteral("saveas-doc"), QStringLiteral("doc"), QObject::tr("Word 2003 Document (.doc)")}
        };
    }
    else if (docType == KIT_DOCTYPE_SPREADSHEET)
    {
        formats = {
            {QStringLiteral("saveas-ods"), QStringLiteral("ods"), QObject::tr("ODF spreadsheet (.ods)")},
            {QStringLiteral("saveas-xlsx"), QStringLiteral("xlsx"), QObject::tr("Excel Spreadsheet (.xlsx)")},
            {QStringLiteral("saveas-xls"), QStringLiteral("xls"), QObject::tr("Excel 2003 Spreadsheet (.xls)")}
        };
    }
    else if (docType == KIT_DOCTYPE_PRESENTATION)
    {
        formats = {
            {QStringLiteral("saveas-odp"), QStringLiteral("odp"), QObject::tr("ODF presentation (.odp)")},
            {QStringLiteral("saveas-pptx"), QStringLiteral("pptx"), QObject::tr("PowerPoint Presentation (.pptx)")},
            {QStringLiteral("saveas-ppt"), QStringLiteral("ppt"), QObject::tr("PowerPoint 2003 Presentation (.ppt)")}
        };
    }
    else if (docType == KIT_DOCTYPE_DRAWING)
    {
        formats = {
            {QStringLiteral("saveas-odg"), QStringLiteral("odg"), QObject::tr("ODF drawing (.odg)")}
        };
    }

    return formats;
}

void printDocument(unsigned appDocId, QWidget* parent)
{
    // Create a temporary PDF file for printing
    const std::string tempFile = FileUtil::createRandomTmpDir() + "/print.pdf";
    const std::string tempFileUri = Poco::URI(Poco::Path(tempFile)).toString();

    kit::Document* loKitDoc = DocumentData::get(appDocId).loKitDocument;
    if (!loKitDoc)
    {
        LOG_ERR("printDocument: no loKitDocument");
        return;
    }

    loKitDoc->saveAs(tempFileUri.c_str(), "pdf", nullptr);

    // Verify the PDF was created
    struct stat st;
    if (FileUtil::getStatOfFile(tempFile, st) != 0)
    {
        LOG_ERR("printDocument: failed to create PDF file: " << tempFile);
        return;
    }

    // Create a simple custom print dialog, qt's print dialog is overkill for now.
    QDialog* customPrintDialog = new QDialog(parent);
    customPrintDialog->setWindowTitle(QObject::tr("Print Document"));
    customPrintDialog->setModal(true);
    customPrintDialog->resize(400, 200);
    customPrintDialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* layout = new QVBoxLayout(customPrintDialog);

    // Printer selection
    QLabel* printerLabel = new QLabel(QObject::tr("Select Printer:"), customPrintDialog);
    layout->addWidget(printerLabel);

    QComboBox* printerCombo = new QComboBox(customPrintDialog);
    // Get available printers
    QStringList printers = QPrinterInfo::availablePrinterNames();
    printerCombo->addItems(printers);
    if (printers.isEmpty())
    {
        printerCombo->addItem(QObject::tr("Default Printer"));
    }
    layout->addWidget(printerCombo);

    // Print to file option
    QCheckBox* printToFileCheck = new QCheckBox(QObject::tr("Print to File"), customPrintDialog);
    layout->addWidget(printToFileCheck);

    QLineEdit* filePathEdit = new QLineEdit(customPrintDialog);
    filePathEdit->setPlaceholderText(QObject::tr("Enter file path..."));
    filePathEdit->setEnabled(false);
    layout->addWidget(filePathEdit);

    // Connect print to file checkbox
    QObject::connect(printToFileCheck, &QCheckBox::toggled,
                     [filePathEdit](bool checked)
                     {
                         filePathEdit->setEnabled(checked);
                         if (checked)
                         {
                             QFileDialog* fileDialog = new QFileDialog(
                                 filePathEdit, QObject::tr("Save Print Output As"),
                                 QDir::home().filePath("document.pdf"),
                                 QObject::tr("PDF Files (*.pdf);;All Files (*)"));

                             fileDialog->setAcceptMode(QFileDialog::AcceptSave);
                             fileDialog->setAttribute(Qt::WA_DeleteOnClose);

                             QObject::connect(fileDialog, &QFileDialog::fileSelected,
                                             [filePathEdit](const QString& fileName) {
                                 filePathEdit->setText(fileName);
                             });

                             fileDialog->open();
                         }
                     });

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* printButton = new QPushButton(QObject::tr("Print"), customPrintDialog);
    QPushButton* cancelButton = new QPushButton(QObject::tr("Cancel"), customPrintDialog);
    buttonLayout->addWidget(printButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    // Connect print button
    QObject::connect(printButton, &QPushButton::clicked,
                     [customPrintDialog, printerCombo, printToFileCheck, filePathEdit, tempFile, parent]() {
        customPrintDialog->accept();

        // Check if user selected "Print to File"
        if (printToFileCheck->isChecked() && !filePathEdit->text().isEmpty())
        {
            QString outputFile = filePathEdit->text();
            LOG_INF("printDocument: User selected print to file: " << outputFile.toStdString());

            if (FileUtil::copyAtomic(tempFile, outputFile.toStdString(), false))
            {
                LOG_INF(
                    "printDocument: PDF successfully saved to file: " << outputFile.toStdString());
            }
            else
            {
                LOG_ERR("printDocument: Failed to copy PDF to file: " << outputFile.toStdString());
                QMessageBox::warning(parent, QObject::tr("Print to File Error"),
                                     QObject::tr("Failed to save document to file. Please check "
                                                 "the file path and permissions."));
            }
        }
        else
        {
            // User selected a physical printer - print using system commands
            QString printerName = printerCombo->currentText();
            if (printerName == QObject::tr("Default Printer"))
            {
                printerName = "";
            }

            // Print the PDF using system command with the selected printer
            std::string printCmd;
            if (!printerName.isEmpty())
            {
                printCmd = "lp -d \"" + printerName.toStdString() + "\" \"" + tempFile + "\"";
            }
            else
            {
                printCmd = "lp \"" + tempFile + "\"";
            }

            int result = std::system(printCmd.c_str());

            if (result != 0)
            {
                // Fallback to lpr with printer name
                if (!printerName.isEmpty())
                {
                    printCmd = "lpr -P \"" + printerName.toStdString() + "\" \"" + tempFile + "\"";
                }
                else
                {
                    printCmd = "lpr \"" + tempFile + "\"";
                }
                result = std::system(printCmd.c_str());

                if (result != 0)
                {
                    LOG_ERR(
                        "printDocument: failed to print PDF. Tried both 'lp' and 'lpr' commands");
                    QMessageBox::warning(
                        parent, QObject::tr("Print Error"),
                        QObject::tr(
                            "Failed to print document. Please check your printer settings."));
                }
                else
                {
                    LOG_INF("printDocument: PDF sent to printer '" << printerName.toStdString()
                                                                   << "' using 'lpr'");
                }
            }
            else
            {
                LOG_INF("printDocument: PDF sent to printer '" << printerName.toStdString()
                                                               << "' using 'lp'");
            }
        }

        // Clean up the temporary file
        FileUtil::unlinkFile(tempFile);
    });

    // Connect cancel button
    QObject::connect(cancelButton, &QPushButton::clicked,
                     [customPrintDialog, tempFile]() {
        customPrintDialog->reject();
        LOG_INF("printDocument: Print cancelled by user");
        FileUtil::unlinkFile(tempFile);
    });

    customPrintDialog->open();
}

bool moveOrCopyFile(const std::string& fromPath, const std::string& toPath)
{
    // A rename moves the data without a second write and atomically replaces
    // an existing destination, but only works within one filesystem; across
    // filesystems it fails and the atomic copy takes over. Either way an
    // existing destination file is replaced only once the new content is
    // complete on disk.
    std::error_code errorCode;
    std::filesystem::rename(fromPath, toPath, errorCode);
    if (!errorCode)
        return true;

    return FileUtil::copyAtomic(fromPath, toPath, /*preserveTimestamps=*/false);
}

QFileDialog* showSaveFileDialog(QWidget* parent, const QString& title,
                                const QString& suggestedName, const std::string& srcPath,
                                std::function<void(bool ok)> onFinished)
{
    QFileDialog* dialog = new QFileDialog(parent, title, QDir::home().filePath(suggestedName),
                                          QObject::tr("All Files (*)"));

    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(dialog, &QFileDialog::fileSelected,
                     [srcPath, onFinished = std::move(onFinished)](const QString& destPath)
                     {
                         const bool ok = moveOrCopyFile(srcPath, destPath.toStdString());
                         if (ok)
                             LOG_INF("export: saved to " << destPath.toStdString());
                         else
                             LOG_ERR("export: failed to copy to '" << destPath.toStdString()
                                                                   << "'");
                         if (onFinished)
                             onFinished(ok);
                     });

    dialog->open();
    return dialog;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
