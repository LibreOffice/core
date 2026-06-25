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

#include "QtClipboard.hpp"

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>

#include <common/MobileApp.hpp>

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QHash>
#include <QLatin1String>
#include <QMimeData>
#include <QString>

#include <atomic>
#include <cstdlib>
#include <memory>
#include <vector>

std::atomic<unsigned> sClipboardSourceDocId{0};

namespace
{
/// Make the doc's view current before any clipboard API call: doc_getClipboard
/// and doc_setClipboard both route by the kit's current view.
bool selectDocViewAsCurrent(kit::Document* loKitDoc)
{
    int nViewId = -1;
    if (!loKitDoc->getViewIds(&nViewId, 1) || nViewId < 0)
        return false;
    loKitDoc->setView(nViewId);
    return true;
}

std::unique_ptr<QMimeData> fetchClipboardData(unsigned appDocId,
                                              const char** pMimeTypes = nullptr)
{
    // A still-mounted LazyClipboardMimeData can outlive its source doc;
    // DocumentData::get crashes on a removed id, so check first.
    DocumentData* docData = DocumentData::getIfExists(appDocId);
    if (!docData)
        return nullptr;

    kit::Document* loKitDoc = docData->loKitDocument;
    if (!loKitDoc || !selectDocViewAsCurrent(loKitDoc))
        return nullptr;

    size_t outCount = 0;
    char** outMimeTypes = nullptr;
    size_t* outSizes = nullptr;
    char** outStreams = nullptr;

    if (!loKitDoc->getClipboard(pMimeTypes, &outCount, &outMimeTypes, &outSizes, &outStreams)
        || outCount == 0)
        return nullptr;

    auto mimeData = std::make_unique<QMimeData>();
    for (size_t i = 0; i < outCount; ++i)
    {
        if (outStreams[i] && outSizes[i] > 0)
            mimeData->setData(QString::fromUtf8(outMimeTypes[i]),
                              QByteArray(outStreams[i], static_cast<int>(outSizes[i])));
        free(outMimeTypes[i]);
        free(outStreams[i]);
    }
    free(outMimeTypes);
    free(outSizes);
    free(outStreams);

    return mimeData;
}
}

/// QMimeData subclass that advertises MIME types without serializing data.
/// Data is fetched on demand from LOKit when an external app (or cross-document
/// paste) actually requests it via retrieveData().
class LazyClipboardMimeData : public QMimeData
{
    unsigned _appDocId;
    QStringList _mimeTypes;
    mutable QHash<QString, QByteArray> _cache;

public:
    LazyClipboardMimeData(unsigned appDocId, QStringList mimeTypes)
        : _appDocId(appDocId)
        , _mimeTypes(std::move(mimeTypes))
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

    unsigned sourceDocId() const { return _appDocId; }

    /// Must be called while the source document is still alive.
    void materialize() const
    {
        std::unique_ptr<QMimeData> data;
        for (const QString& f : _mimeTypes)
        {
            if (_cache.contains(f))
                continue;
            if (!data)
            {
                data = fetchClipboardData(_appDocId);
                if (!data)
                    return;
            }
            _cache.insert(f, data->data(f));
        }
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

        const std::string mimeStr = fetchType.toStdString();
        const char* pMimeTypes[] = { mimeStr.c_str(), nullptr };
        std::unique_ptr<QMimeData> data = fetchClipboardData(_appDocId, pMimeTypes);
        // Cache empty results too, to suppress repeated probes for unavailable formats.
        QByteArray bytes = data ? data->data(fetchType) : QByteArray{};
        _cache.insert(fetchType, bytes);
        return bytes;
    }
};

void setClipboard(unsigned appDocId)
{
    kit::Document* loKitDoc = DocumentData::get(appDocId).loKitDocument;
    if (!loKitDoc)
        return;

    const QMimeData* data = QApplication::clipboard()->mimeData();
    if (!data)
        return;

    // Limit MIME types that LOKit can consume. Keeping this set small also avoids IPC round-trips
    // to clipboard owners for formats they won't provide usefully (e.g. Emacs advertises many X11
    // atoms and app-specific types alongside text/plain).
    auto isLoKitFormat = [](const QString& f)
    {
        return f.startsWith(QLatin1String("text/"))
            || f == QLatin1String("image/png")
            || f == QLatin1String("image/jpeg")
            || f == QLatin1String("image/bmp")
            || f.startsWith(QLatin1String("image/svg+"))   // image/svg+xml and ;params
            || f.startsWith(QLatin1String("application/x-openoffice-"))
            || f.startsWith(QLatin1String("application/x-libreoffice-"))
            || f.startsWith(QLatin1String("application/vnd.oasis.opendocument."))
            || f.startsWith(QLatin1String("application/vnd.sun.xml."))
            || f == QLatin1String("application/msword")
            || f == QLatin1String("application/mathml+xml")
            || f == QLatin1String("application/pdf");
    };

    std::vector<std::string> mimeTypeStrings;
    std::vector<QByteArray> byteArrays;

    // Enforce UTF-8 for text data as that is what COKit expects.
    if (data->hasText())
    {
        QByteArray utf8 = data->text().toUtf8();
        if (!utf8.isEmpty())
        {
            mimeTypeStrings.push_back("text/plain;charset=utf-8");
            byteArrays.push_back(std::move(utf8));
        }
    }

    for (const QString& format : data->formats())
    {
        // Text already extracted as UTF-8 above; don't forward any raw text/plain* variant.
        if (format.startsWith(QLatin1String("text/plain")))
            continue;
        if (!isLoKitFormat(format))
            continue;
        QByteArray bytes = data->data(format);
        if (bytes.isEmpty())
            continue;
        mimeTypeStrings.push_back(format.toStdString());
        byteArrays.push_back(std::move(bytes));
    }

    std::vector<const char*> mimeTypePtrs;
    std::vector<size_t> sizes;
    std::vector<const char*> streams;
    for (size_t i = 0; i < mimeTypeStrings.size(); ++i)
    {
        mimeTypePtrs.push_back(mimeTypeStrings[i].c_str());
        sizes.push_back(byteArrays[i].size());
        streams.push_back(byteArrays[i].data());
    }

    if (!mimeTypePtrs.empty())
    {
        if (!selectDocViewAsCurrent(loKitDoc))
            return;
        loKitDoc->setClipboard(mimeTypePtrs.size(), mimeTypePtrs.data(), sizes.data(),
                               streams.data());
    }
}

void setLazyClipboard(unsigned appDocId, QStringList mimeTypes)
{
    QGuiApplication::clipboard()->setMimeData(
        new LazyClipboardMimeData(appDocId, std::move(mimeTypes)));
    sClipboardSourceDocId.store(appDocId);
}

void materializeClipboard(unsigned appDocId)
{
    const QMimeData* current = QGuiApplication::clipboard()->mimeData();
    const LazyClipboardMimeData* lazy = dynamic_cast<const LazyClipboardMimeData*>(current);
    if (!lazy || lazy->sourceDocId() != appDocId)
        return;
    lazy->materialize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
