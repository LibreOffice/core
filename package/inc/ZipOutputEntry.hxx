/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_PACKAGE_INC_ZIPOUTPUTENTRY_HXX
#define INCLUDED_PACKAGE_INC_ZIPOUTPUTENTRY_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <package/Deflater.hxx>
#include <comphelper/threadpool.hxx>
#include "CRC32.hxx"
#include <atomic>

struct ZipEntry;
class ZipPackageBuffer;
class ZipPackageStream;

class ZipOutputEntryBase
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::io::XOutputStream > m_xOutStream;

    css::uno::Reference< css::xml::crypto::XCipherContext > m_xCipherContext;
    css::uno::Reference< css::xml::crypto::XDigestContext > m_xDigestContext;

    CRC32               m_aCRC;
    ZipEntry            *m_pCurrentEntry;
    sal_Int16           m_nDigested;
    ZipPackageStream*   m_pCurrentStream;
    bool const          m_bEncryptCurrentEntry;

public:
    virtual ~ZipOutputEntryBase() = default;

    virtual void writeStream(const css::uno::Reference< css::io::XInputStream >& xInStream) = 0;

    ZipEntry* getZipEntry() { return m_pCurrentEntry; }
    ZipPackageStream* getZipPackageStream() { return m_pCurrentStream; }
    bool isEncrypt() const { return m_bEncryptCurrentEntry; }

    void closeEntry();

protected:
    ZipOutputEntryBase(
        const css::uno::Reference< css::io::XOutputStream >& rxOutStream,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt, bool checkStream);

    // Inherited classes call this with deflated data buffer.
    void processDeflated( const css::uno::Sequence< sal_Int8 >& deflateBuffer, sal_Int32 nLength );
    // Inherited classes call this with the input buffer.
    void processInput( const css::uno::Sequence< sal_Int8 >& rBuffer );

    virtual void finishDeflater() = 0;
    virtual sal_Int64 getDeflaterTotalIn() const = 0;
    virtual sal_Int64 getDeflaterTotalOut() const = 0;
    virtual void deflaterReset() = 0;
    virtual bool isDeflaterFinished() const = 0;
};

// Normal non-threaded case.
class ZipOutputEntry : public ZipOutputEntryBase
{
    css::uno::Sequence< sal_Int8 > m_aDeflateBuffer;
    ZipUtils::Deflater m_aDeflater;

public:
    ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutStream,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt);
    void writeStream(const css::uno::Reference< css::io::XInputStream >& xInStream) override;
    void write(const css::uno::Sequence< sal_Int8 >& rBuffer);

protected:
    ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutStream,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt, bool checkStream);
    virtual void finishDeflater() override;
    virtual sal_Int64 getDeflaterTotalIn() const override;
    virtual sal_Int64 getDeflaterTotalOut() const override;
    virtual void deflaterReset() override;
    virtual bool isDeflaterFinished() const override;
    void doDeflate();
};

// Class that runs the compression in a background thread.
class ZipOutputEntryInThread : public ZipOutputEntry
{
    class Task;
    OUString m_aTempURL;
    std::exception_ptr m_aParallelDeflateException;
    std::atomic<bool>   m_bFinished;

public:
    ZipOutputEntryInThread(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt);
    std::unique_ptr<comphelper::ThreadTask> createTask(
        const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
        const css::uno::Reference< css::io::XInputStream >& xInStream );
    /* This block of methods is for threaded zipping, where we compress to a temp stream, whose
       data is retrieved via getData */
    void createBufferFile();
    void setParallelDeflateException(const std::exception_ptr& exception) { m_aParallelDeflateException = exception; }
    css::uno::Reference< css::io::XInputStream > getData() const;
    const std::exception_ptr& getParallelDeflateException() const { return m_aParallelDeflateException; }
    void closeBufferFile();
    void deleteBufferFile();
    bool isFinished() const { return m_bFinished; }
private:
    void setFinished() { m_bFinished = true; }
};

// Class that synchronously runs the compression in multiple threads (using ThreadDeflater).
class ZipOutputEntryParallel : public ZipOutputEntryBase
{
    sal_Int64 totalIn;
    sal_Int64 totalOut;
public:
    ZipOutputEntryParallel(
        const css::uno::Reference< css::io::XOutputStream >& rxOutStream,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ZipEntry& rEntry, ZipPackageStream* pStream, bool bEncrypt);
    void writeStream(const css::uno::Reference< css::io::XInputStream >& xInStream) override;
protected:
    virtual void finishDeflater() override;
    virtual sal_Int64 getDeflaterTotalIn() const override;
    virtual sal_Int64 getDeflaterTotalOut() const override;
    virtual void deflaterReset() override;
    virtual bool isDeflaterFinished() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
