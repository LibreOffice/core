/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "QtFilePicker.hpp"

#include "DBusService.hpp"

#include <COKit/COKit.hxx>

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMetaObject>
#include <QStringList>
#include <QUrl>

// "Name (*.png *.jpg)" is the QFileDialog form of one name filter.
static QStringList toNameFilters(const COKitFilePickerFilter* pFilters, size_t nFilters)
{
    QStringList nameFilters;
    for (size_t i = 0; i < nFilters; ++i)
    {
        const QString wildcards = QString::fromUtf8(pFilters[i].pWildcards)
                                      .replace(QLatin1Char(';'), QLatin1Char(' '));
        if (wildcards.isEmpty())
            continue;

        // The flatpak FileChooser portal rejects a nameless filter, and Qt then falls back
        // to a dialog inside the sandbox.
        QString name = QString::fromUtf8(pFilters[i].pName).trimmed();
        if (name.isEmpty())
            name = QObject::tr("Supported files");

        nameFilters << name + " (" + wildcards + ")";
    }
    return nameFilters;
}

// The engine asks the user for a file. Convert the details on the calling thread, then
// show a QFileDialog on the GUI thread and answer through the completion; the engine's
// main loop keeps running while the dialog is open.
static void filePickerProviderPick(const char* pTitle, const COKitFilePickerFilter* pFilters,
                                   size_t nFilters,
                                   void (*pfnPicked)(void* pContext, const char* pUrl),
                                   void* pContext)
{
    QString title = pTitle && pTitle[0] ? QString::fromUtf8(pTitle) : QObject::tr("Open File");
    const QStringList nameFilters = toNameFilters(pFilters, nFilters);

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
                                                             .toEncoded()
                                                             .constData());
                                 else
                                     pfnPicked(pContext, nullptr);
                             });

            dialog->open();
        },
        Qt::QueuedConnection);
}

// The engine asks where a new file goes, the chapter of a master document for example. As for
// an open-file pick, a QFileDialog on the GUI thread answers through the completion.
static void filePickerProviderPickSaveLocation(const char* pTitle, const char* pSuggestedUrl,
                                               const COKitFilePickerFilter* pFilters,
                                               size_t nFilters,
                                               void (*pfnPicked)(void* pContext, const char* pUrl),
                                               void* pContext)
{
    QString title = pTitle && pTitle[0] ? QString::fromUtf8(pTitle) : QObject::tr("Save File");
    const QStringList nameFilters = toNameFilters(pFilters, nFilters);
    const QString suggested = pSuggestedUrl && pSuggestedUrl[0]
                                  ? QUrl::fromEncoded(QByteArray(pSuggestedUrl)).toLocalFile()
                                  : QString();
    // The extension of the first filter, which the dialog adds to a name typed without one.
    QString suffix;
    if (nFilters > 0 && pFilters[0].pWildcards)
    {
        const QString wildcard = QString::fromUtf8(pFilters[0].pWildcards).section(';', 0, 0);
        if (wildcard.startsWith(QLatin1String("*.")))
            suffix = wildcard.mid(2);
    }

    QMetaObject::invokeMethod(
        qApp,
        [title, nameFilters, suggested, suffix, pfnPicked, pContext]()
        {
            QFileDialog* dialog = new QFileDialog(QApplication::activeWindow(), title);
            dialog->setAcceptMode(QFileDialog::AcceptSave);
            dialog->setFileMode(QFileDialog::AnyFile);
            if (!nameFilters.isEmpty())
                dialog->setNameFilters(nameFilters);
            if (!suffix.isEmpty())
                dialog->setDefaultSuffix(suffix);
            if (!suggested.isEmpty())
                dialog->selectFile(suggested);
            dialog->setAttribute(Qt::WA_DeleteOnClose);

            // finished fires exactly once, for a pick and for a cancel alike.
            QObject::connect(dialog, &QDialog::finished,
                             [dialog, pfnPicked, pContext](int result)
                             {
                                 const QStringList files = dialog->selectedFiles();
                                 if (result == QDialog::Accepted && !files.isEmpty())
                                     pfnPicked(pContext, QUrl::fromLocalFile(files.first())
                                                             .toEncoded()
                                                             .constData());
                                 else
                                     pfnPicked(pContext, nullptr);
                             });

            dialog->open();
        },
        Qt::QueuedConnection);
}

// Inside flatpak and snap the FileChooser portal gives a document-portal path for a file the
// sandbox does not show, which the app may read in this session only. Qt picks through the
// portal there, unless told not to.
static bool picksThroughPortal()
{
    if (qEnvironmentVariableIsSet("QT_NO_XDG_DESKTOP_PORTAL"))
        return false;
    return QFile::exists(QStringLiteral("/.flatpak-info")) || qEnvironmentVariableIsSet("SNAP");
}

// A file the user picks to link to, through the portal: the document links to the host path of
// the file and reads it through the portal path.
static void filePickerProviderPickLinkTarget(const char* pTitle,
                                             const COKitFilePickerFilter* pFilters,
                                             size_t nFilters,
                                             void (*pfnPicked)(void* pContext, const char* pLinkUrl,
                                                               const char* pReadUrl),
                                             void* pContext)
{
    QString title = pTitle && pTitle[0] ? QString::fromUtf8(pTitle) : QObject::tr("Open File");
    const QStringList nameFilters = toNameFilters(pFilters, nFilters);

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
            QObject::connect(
                dialog, &QDialog::finished,
                [dialog, pfnPicked, pContext](int result)
                {
                    const QStringList files = dialog->selectedFiles();
                    if (result != QDialog::Accepted || files.isEmpty())
                    {
                        pfnPicked(pContext, nullptr, nullptr);
                        return;
                    }
                    // A file the app may read where it is, in a folder the sandbox gives, has
                    // no other path.
                    const QByteArray readUrl = QUrl::fromLocalFile(files.first()).toEncoded();
                    const std::string hostUrl = coda::hostDisplayUriForPath(files.first());
                    pfnPicked(pContext, hostUrl.empty() ? readUrl.constData() : hostUrl.c_str(),
                              readUrl.constData());
                });

            dialog->open();
        },
        Qt::QueuedConnection);
}

void install_filepicker_provider(COKit& rOffice)
{
    static COKitFilePickerProvider provider{};
    provider.pick = filePickerProviderPick;
    // A new file the portal gives a place for has no path outside this session to link to.
    if (!picksThroughPortal())
        provider.pickSaveLocation = filePickerProviderPickSaveLocation;
    // The native picker gives the file the user picked, which a document may link to.
    provider.picksFilesInPlace = !picksThroughPortal();
    if (picksThroughPortal())
        provider.pickLinkTarget = filePickerProviderPickLinkTarget;
    rOffice.installFilePickerProvider(&provider);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
