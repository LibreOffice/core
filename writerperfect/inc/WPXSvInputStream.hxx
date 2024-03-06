/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <librevenge-stream/librevenge-stream.h>
#include <com/sun/star/uno/Reference.h>
#include <tools/long.hxx>
#include "writerperfectdllapi.h"
#include <memory>
#include <tools/stream.hxx>
#include <sot/storage.hxx>

namespace com::sun::star::io
{
class XInputStream;
class XSeekable;
}

namespace writerperfect
{
struct OLEStorageImpl;
struct ZipStorageImpl;

class SAL_DLLPUBLIC_RTTI WPXSvInputStream final : public librevenge::RVNGInputStream
{
public:
    WRITERPERFECT_DLLPUBLIC
    WPXSvInputStream(css::uno::Reference<css::io::XInputStream> const& xStream);
    WRITERPERFECT_DLLPUBLIC virtual ~WPXSvInputStream() override;

    virtual bool isStructured() override;
    virtual unsigned subStreamCount() override;
    virtual const char* subStreamName(unsigned id) override;
    virtual bool existsSubStream(const char* name) override;
    virtual librevenge::RVNGInputStream* getSubStreamByName(const char* name) override;
    virtual librevenge::RVNGInputStream* getSubStreamById(unsigned id) override;

    WRITERPERFECT_DLLPUBLIC virtual const unsigned char* read(unsigned long numBytes,
                                                              unsigned long& numBytesRead) override;
    WRITERPERFECT_DLLPUBLIC virtual int seek(long offset,
                                             librevenge::RVNG_SEEK_TYPE seekType) override;
    virtual long tell() override;
    virtual bool isEnd() override;

private:
    tools::Long tellImpl();
    int seek(tools::Long offset);
    bool isOLE();
    void ensureOLEIsInitialized();
    bool isZip();
    void ensureZipIsInitialized();
    static librevenge::RVNGInputStream*
    createWPXStream(const rtl::Reference<SotStorageStream>& rxStorage);
    static librevenge::RVNGInputStream*
    createWPXStream(const css::uno::Reference<css::io::XInputStream>& rxStream);

    css::uno::Reference<css::io::XInputStream> mxStream;
    css::uno::Reference<css::io::XSeekable> mxSeekable;
    css::uno::Sequence<sal_Int8> maData;
    std::unique_ptr<OLEStorageImpl> mpOLEStorage;
    std::unique_ptr<ZipStorageImpl> mpZipStorage;
    sal_Int64 mnLength;
    unsigned long mnReadBufferPos;
    bool mbCheckedOLE;
    bool mbCheckedZip;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
