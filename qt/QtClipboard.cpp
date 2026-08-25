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

// The COKitClipboardProvider backend for the Qt app: the engine drives both
// clipboard directions and this file only does the raw QClipboard input and
// output, like the macOS (COWrapper.mm) and Windows (CODA.cpp) apps.
//
// Unlike NSPasteboard and the Win32 clipboard, QClipboard may only be used on
// the GUI thread, while the provider callbacks fire on the kit thread with the
// SolarMutex held. Advertising is therefore queued to the GUI thread (with the
// ownership flag set synchronously so an immediate paste still sees its own
// copy), and the paste-direction reads block on the GUI thread: the format
// list on every query, the bytes once per foreign clipboard change, taken as
// one snapshot of every advertised format. That blocking is deadlock-free
// because the engine only reads through the provider when ownsClipboard()
// said the clipboard is foreign, and every GUI-thread path into the engine is
// guarded on that same ownership flag.

#include <config.h>

#include "QtClipboard.hpp"

#include <COKit/COKit.hxx>

#include <common/MobileApp.hpp>

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QHash>
#include <QLatin1String>
#include <QMetaObject>
#include <QMimeData>
#include <QString>
#include <QStringList>
#include <QThread>

#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace
{
// The engine office handle, captured when the clipboard provider is installed,
// so a clipboard read can go straight to the process-shared clipboard without
// needing a particular document.
COKit* sOffice = nullptr;

// Whether the system clipboard still holds our own last advertise. Set
// synchronously on the kit thread when the engine advertises a copy, and
// maintained by the GUI thread's dataChanged watcher afterwards.
std::atomic<bool> sWeOwnClipboard{ false };

// Counts system clipboard changes, bumped by the GUI thread's dataChanged
// watcher, so a paste can tell whether its byte snapshot is still current.
std::atomic<quint64> sClipboardGeneration{ 0 };

// One consistent read of the foreign clipboard: every advertised format's
// bytes, taken in a single GUI-thread visit. Touched only on the kit thread
// (every provider callback fires there), so it needs no lock.
struct ForeignClipboardSnapshot
{
    bool valid = false;
    quint64 generation = 0;
    QHash<QString, QByteArray> data;
};
ForeignClipboardSnapshot sForeignSnapshot;

/// Run fn on the GUI thread and wait for it to finish. The paste-direction
/// provider callbacks fire on the kit thread, but QClipboard may only be used
/// on the GUI thread.
void runOnGuiThreadBlocking(std::function<void()> fn)
{
    if (QThread::currentThread() == qApp->thread())
    {
        fn();
        return;
    }
    QMetaObject::invokeMethod(qApp, std::move(fn), Qt::BlockingQueuedConnection);
}

/// Pull the bytes the engine holds for one clipboard format. The clipboard is
/// process-global (one shared clipboard for the whole app), so it is read
/// straight from the office; no document is involved.
QByteArray fetchEngineClipboardData(const QString& mimeType)
{
    std::vector<char> data;
    if (!sOffice || !sOffice->getGlobalClipboardData(mimeType.toStdString().c_str(), data))
        return {};

    return QByteArray(data.data(), static_cast<qsizetype>(data.size()));
}

/// QMimeData subclass that advertises the engine's MIME types without
/// serializing any data. The bytes are fetched from the engine's shared
/// clipboard on demand, when a paste target actually requests a format.
class LazyEngineMimeData : public QMimeData
{
    QStringList _mimeTypes;
    mutable QHash<QString, QByteArray> _cache;

public:
    explicit LazyEngineMimeData(QStringList mimeTypes)
        : _mimeTypes(std::move(mimeTypes))
    {
    }

    QStringList formats() const override
    {
        // A paste target asks whether plain text is present under the bare name
        // "text/plain", but the engine offers it as "text/plain;charset=utf-8".
        // Advertise the bare name too so the plain-text paste is offered.
        QStringList result = _mimeTypes;
        if (_mimeTypes.contains(QLatin1String("text/plain;charset=utf-8"))
            && !result.contains(QLatin1String("text/plain")))
            result.append(QStringLiteral("text/plain"));
        return result;
    }

    bool hasFormat(const QString& mimeType) const override
    {
        return formats().contains(mimeType);
    }

protected:
    QVariant retrieveData(const QString& mimeType, QMetaType /*type*/) const override
    {
        // A request for the bare "text/plain" name maps to the UTF-8 plain text
        // the engine holds as "text/plain;charset=utf-8".
        const QString fetchType = (mimeType == QLatin1String("text/plain"))
                                      ? QStringLiteral("text/plain;charset=utf-8")
                                      : mimeType;

        auto it = _cache.constFind(fetchType);
        if (it != _cache.constEnd())
            return *it;

        // Enter the engine only while the clipboard is still ours: once it is
        // foreign, the kit thread may be blocking on the GUI thread inside a
        // paste-direction provider callback while holding the SolarMutex, and
        // the engine read below would wait for that mutex - a deadlock. A
        // transfer still in flight from a replaced offer comes back empty.
        if (!sWeOwnClipboard.load())
            return QByteArray();

        const QByteArray bytes = fetchEngineClipboardData(fetchType);
        // Cache empty results too, to suppress repeated probes for unavailable formats.
        _cache.insert(fetchType, bytes);
        return bytes;
    }
};

void onClipboardDataChanged()
{
    sClipboardGeneration.fetch_add(1, std::memory_order_relaxed);
    const QMimeData* data = QGuiApplication::clipboard()->mimeData();
    sWeOwnClipboard.store(dynamic_cast<const LazyEngineMimeData*>(data) != nullptr);
}

/// The clipboard formats the engine can consume, under the names it expects.
/// GUI thread only: reads the passed QMimeData.
QStringList engineMimeTypes(const QMimeData* data)
{
    QStringList types;
    if (!data)
        return types;
    bool havePlainText = data->hasText();
    for (const QString& format : data->formats())
    {
        // Fold every text/plain variant into the one canonical UTF-8 name the
        // engine advertises itself; the bytes are converted to match when the
        // format is read.
        if (format.startsWith(QLatin1String("text/plain")))
        {
            havePlainText = true;
            continue;
        }
        // Qt synthesizes entries like "application/x-qt-image" aside, non-MIME
        // names mean nothing to the engine's format tables.
        if (!format.contains(QLatin1Char('/')) || types.contains(format))
            continue;
        types.append(format);
    }
    if (havePlainText)
        types.append(QStringLiteral("text/plain;charset=utf-8"));
    return types;
}

/**
 * The clipboard provider the engine drives. On copy the engine advertises its
 * formats through advertise; on an external paste it reads the system
 * clipboard one format at a time. The callbacks act on the process, not one
 * document: every document shares the one engine clipboard.
 */

void clipboardProviderAdvertise(const char** pMimeTypes)
{
    QStringList types;
    for (size_t i = 0; pMimeTypes && pMimeTypes[i]; ++i)
        types.append(QString::fromUtf8(pMimeTypes[i]));

    // Claim ownership synchronously: the copy completes on the kit thread
    // before the queued QClipboard update below has run, and a paste in
    // between must already take the engine's in-memory shortcut.
    sWeOwnClipboard.store(true);

    // The foreign bytes cannot be pasted any more; free them.
    sForeignSnapshot = {};

    QMetaObject::invokeMethod(
        qApp,
        [types = std::move(types)]() mutable
        { QGuiApplication::clipboard()->setMimeData(new LazyEngineMimeData(std::move(types))); },
        Qt::QueuedConnection);
}

bool clipboardProviderOwns() { return sWeOwnClipboard.load(); }

std::vector<std::string> clipboardProviderGetMimeTypes()
{
    QStringList types;
    runOnGuiThreadBlocking(
        [&types]() { types = engineMimeTypes(QGuiApplication::clipboard()->mimeData()); });

    std::vector<std::string> result;
    result.reserve(static_cast<size_t>(types.size()));
    for (const QString& type : types)
        result.push_back(type.toStdString());
    return result;
}

bool clipboardProviderGetData(const char* pMimeType, std::vector<char>* pOutData)
{
    // The advertised text/plain;charset=utf-8 stands for whatever text/plain
    // variant the platform holds; the snapshot stores it under that name.
    QString mimeType = QString::fromUtf8(pMimeType);
    if (mimeType.startsWith(QLatin1String("text/plain")))
        mimeType = QStringLiteral("text/plain;charset=utf-8");

    // Read the whole foreign clipboard in one GUI-thread visit, and serve
    // every format from that snapshot until the clipboard changes: one
    // blocking hop and one consistent clipboard state per paste, instead of
    // one inter-process fetch per format the engine probes.
    if (!sForeignSnapshot.valid
        || sForeignSnapshot.generation != sClipboardGeneration.load())
    {
        sForeignSnapshot = {};
        QHash<QString, QByteArray> snapshot;
        quint64 generation = 0;
        bool changedUnderUs = false;
        runOnGuiThreadBlocking(
            [&snapshot, &generation, &changedUnderUs]()
            {
                generation = sClipboardGeneration.load();
                const QClipboard* clipboard = QGuiApplication::clipboard();
                for (const QString& type : engineMimeTypes(clipboard->mimeData()))
                {
                    // Each fetch can spin the event loop waiting on the source
                    // app, so the clipboard can change mid-visit; re-fetch the
                    // (then re-created) mime data and give up rather than mix
                    // two clipboards' bytes.
                    const QMimeData* data = clipboard->mimeData();
                    if (!data || sClipboardGeneration.load() != generation)
                    {
                        changedUnderUs = true;
                        return;
                    }
                    // QMimeData::text() decodes whatever text/plain variant
                    // the platform holds; toUtf8() matches the advertise.
                    if (type == QLatin1String("text/plain;charset=utf-8"))
                        snapshot.insert(type, data->text().toUtf8());
                    else
                        snapshot.insert(type, data->data(type));
                }
            });
        if (changedUnderUs)
            return false;
        sForeignSnapshot = { true, generation, std::move(snapshot) };
    }

    const QByteArray bytes = sForeignSnapshot.data.value(mimeType);
    if (bytes.isEmpty())
        return false;

    pOutData->assign(bytes.constData(), bytes.constData() + bytes.size());
    return true;
}
}

void initializeQtClipboard()
{
    QObject::connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
                     &onClipboardDataChanged);
}

void install_clipboard_provider(COKit& rOffice)
{
    sOffice = &rOffice;

    static COKitClipboardProvider provider{};
    provider.advertiseToPlatform = clipboardProviderAdvertise;
    provider.ownsClipboard = clipboardProviderOwns;
    provider.getMimeTypes = clipboardProviderGetMimeTypes;
    provider.getDataForMimeType = clipboardProviderGetData;
    rOffice.installClipboardProvider(&provider);
}

void flushClipboardOnDocClose(unsigned appDocId)
{
    // When the clipboard is foreign the engine's in-memory copy will never be
    // read again, and entering the engine here could deadlock against a kit
    // thread blocking on the GUI thread in a paste-direction callback.
    if (!sWeOwnClipboard.load())
        return;

    DocumentData* docData = DocumentData::getIfExists(appDocId);
    if (!docData || !docData->loKitDocument)
        return;

    // Render the lazy transferable into engine-held bytes now: the shared
    // clipboard outlives the document, but a Writer or Impress transferable
    // still references the document it was copied from.
    docData->loKitDocument->flushClipboard();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
