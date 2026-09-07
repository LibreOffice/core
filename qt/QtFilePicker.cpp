/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QtFilePicker.hpp"

#include <COKit/COKit.hxx>

#include <QApplication>
#include <QFileDialog>
#include <QMetaObject>
#include <QStringList>
#include <QUrl>

// The engine asks the user for a file. Convert the details on the calling thread, then
// show a QFileDialog on the GUI thread and answer through the completion; the engine's
// main loop keeps running while the dialog is open.
static void filePickerProviderPick(const char* pTitle, const COKitFilePickerFilter* pFilters,
                                   size_t nFilters,
                                   void (*pfnPicked)(void* pContext, const char* pUrl),
                                   void* pContext)
{
    QString title = pTitle && pTitle[0] ? QString::fromUtf8(pTitle) : QObject::tr("Open File");

    QStringList nameFilters;
    for (size_t i = 0; i < nFilters; ++i)
    {
        // "Name (*.png *.jpg)" is the QFileDialog form of one name filter.
        const QString wildcards = QString::fromUtf8(pFilters[i].pWildcards)
                                      .replace(QLatin1Char(';'), QLatin1Char(' '));
        if (!wildcards.isEmpty())
            nameFilters << QString::fromUtf8(pFilters[i].pName) + " (" + wildcards + ")";
    }

    QMetaObject::invokeMethod(
        qApp,
        [title, nameFilters, pfnPicked, pContext]()
        {
            QFileDialog* dialog = new QFileDialog(QApplication::activeWindow(), title);
            if (!nameFilters.isEmpty())
                dialog->setNameFilters(nameFilters);
            dialog->setFileMode(QFileDialog::ExistingFile);
            dialog->setAttribute(Qt::WA_DeleteOnClose);

            // finished fires exactly once, for a pick and for a cancel alike.
            QObject::connect(dialog, &QDialog::finished,
                             [dialog, pfnPicked, pContext](int result)
                             {
                                 const QStringList files = dialog->selectedFiles();
                                 if (result == QDialog::Accepted && !files.isEmpty())
                                     pfnPicked(pContext, QUrl::fromLocalFile(files.first())
                                                             .toString()
                                                             .toUtf8()
                                                             .constData());
                                 else
                                     pfnPicked(pContext, nullptr);
                             });

            dialog->open();
        },
        Qt::QueuedConnection);
}

void install_filepicker_provider(COKit& rOffice)
{
    static COKitFilePickerProvider provider{};
    provider.pick = filePickerProviderPick;
    rOffice.installFilePickerProvider(&provider);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
