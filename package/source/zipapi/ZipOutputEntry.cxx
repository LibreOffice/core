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

#include <ZipOutputEntry.hxx>

#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

#include <osl/diagnose.h>

#include <PackageConstants.hxx>
#include <ThreadedDeflater.hxx>
#include <ZipEntry.hxx>
#include <ZipFile.hxx>
#include <ZipPackageStream.hxx>

#include <algorithm>
#include <utility>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to deflate Zip entries
 */
ZipOutputEntryBase::ZipOutputEntryBase(
        css::uno::Reference< css::io::XOutputStream > xOutput,
        uno::Reference< uno::XComponentContext > xContext,
        ZipEntry* pEntry,
        ZipPackageStream* pStream,
        bool bEncrypt,
        bool checkStream)
: m_xContext(std::move(xContext))
, m_xOutStream(std::move(xOutput))
, m_pCurrentEntry(pEntry)
, m_nDigested(0)
, m_pCurrentStream(pStream)
, m_bEncryptCurrentEntry(bEncrypt)
{
    assert(pEntry);
    assert(m_pCurrentEntry->nMethod == DEFLATED && "Use ZipPackageStream::rawWrite() for STORED entries");
    (void)checkStream;
    assert(!checkStream || m_xOutStream.is());
    if (m_bEncryptCurrentEntry)
    {
        m_xCipherContext = ZipFile::StaticGetCipher( m_xContext, pStream->GetEncryptionData(), true );
        if (pStream->GetEncryptionData()->m_oCheckAlg)
        {
            assert(pStream->GetEncryptionData()->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C);
            m_xDigestContext = ZipFile::StaticGetDigestContextForChecksum(m_xContext, pStream->GetEncryptionData());
        }
    }
}

void ZipOutputEntryBase::closeEntry()
{
    finishDeflater();

    if ((m_pCurrentEntry->nFlag & 8) == 0)
    {
        if (m_pCurrentEntry->nSize != getDeflaterTotalIn())
        {
            OSL_FAIL("Invalid entry size");
        }
        if (m_pCurrentEntry->nCompressedSize != getDeflaterTotalOut())
        {
            // Different compression strategies make the merit of this
            // test somewhat dubious
            m_pCurrentEntry->nCompressedSize = getDeflaterTotalOut();
        }
        if (m_pCurrentEntry->nCrc != m_aCRC.getValue())
        {
            OSL_FAIL("Invalid entry CRC-32");
        }
    }
    else
    {
        if ( !m_bEncryptCurrentEntry )
        {
            m_pCurrentEntry->nSize = getDeflaterTotalIn();
            m_pCurrentEntry->nCompressedSize = getDeflaterTotalOut();
        }
        m_pCurrentEntry->nCrc = m_aCRC.getValue();
    }
    deflaterReset();
    m_aCRC.reset();

    if (!m_bEncryptCurrentEntry)
        return;

    m_xCipherContext.clear();

    uno::Sequence< sal_Int8 > aDigestSeq;
    if ( m_xDigestContext.is() )
    {
        aDigestSeq = m_xDigestContext->finalizeDigestAndDispose();
        m_xDigestContext.clear();
    }

    if ( m_pCurrentStream )
        m_pCurrentStream->setDigest( aDigestSeq );
}

void ZipOutputEntryBase::processDeflated( const uno::Sequence< sal_Int8 >& deflateBuffer, sal_Int32 nLength )
{
    if ( nLength > 0 )
    {
        uno::Sequence< sal_Int8 > aTmpBuffer( deflateBuffer.getConstArray(), nLength );
        if (m_bEncryptCurrentEntry && m_xCipherContext.is())
        {
            // Need to update our digest before encryption...
            sal_Int32 nDiff = n_ConstDigestLength - m_nDigested;
            if (m_xDigestContext.is() && nDiff)
            {
                sal_Int32 nEat = ::std::min( nLength, nDiff );
                uno::Sequence< sal_Int8 > aTmpSeq( aTmpBuffer.getConstArray(), nEat );
                m_xDigestContext->updateDigest( aTmpSeq );
                m_nDigested = m_nDigested + static_cast< sal_Int16 >( nEat );
            }

            // FIXME64: uno::Sequence not 64bit safe.
            uno::Sequence< sal_Int8 > aEncryptionBuffer = m_xCipherContext->convertWithCipherContext( aTmpBuffer );

            m_xOutStream->writeBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated here
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
        else
        {
            m_xOutStream->writeBytes ( aTmpBuffer );
        }
    }

    if (!(isDeflaterFinished() && m_bEncryptCurrentEntry && m_xCipherContext.is()))
        return;

    // FIXME64: sequence not 64bit safe.
    uno::Sequence< sal_Int8 > aEncryptionBuffer = m_xCipherContext->finalizeCipherContextAndDispose();
    if ( aEncryptionBuffer.hasElements() )
    {
        m_xOutStream->writeBytes( aEncryptionBuffer );

        // the sizes as well as checksum for encrypted streams are calculated here
        m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
        m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
        m_aCRC.update( aEncryptionBuffer );
    }
}

void ZipOutputEntryBase::processInput( const uno::Sequence< sal_Int8 >& rBuffer )
{
    if (!m_bEncryptCurrentEntry)
        m_aCRC.updateSegment(rBuffer, rBuffer.getLength());
}

ZipOutputEntry::ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry* pEntry,
        ZipPackageStream* pStream,
        bool bEncrypt,
        bool checkStream)
: ZipOutputEntryBase(rxOutput, rxContext, pEntry, pStream, bEncrypt, checkStream)
, m_aDeflateBuffer(n_ConstBufferSize)
, m_aDeflater(DEFAULT_COMPRESSION, true)
{
}

ZipOutputEntry::ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry* pEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntry( rxOutput, rxContext, pEntry, pStream, bEncrypt, true)
{
}

void ZipOutputEntry::write( const Sequence< sal_Int8 >& rBuffer )
{
    if (!m_aDeflater.finished())
    {
        m_aDeflater.setInputSegment(rBuffer);
        while (!m_aDeflater.needsInput())
            doDeflate();
        processInput(rBuffer);
    }
}

void ZipOutputEntry::doDeflate()
{
    sal_Int32 nLength = m_aDeflater.doDeflateSegment(m_aDeflateBuffer, m_aDeflateBuffer.getLength());
    processDeflated( m_aDeflateBuffer, nLength );
}

void ZipOutputEntry::finishDeflater()
{
    m_aDeflater.finish();
    while (!m_aDeflater.finished())
        doDeflate();
}

sal_Int64 ZipOutputEntry::getDeflaterTotalIn() const
{
    return m_aDeflater.getTotalIn();
}

sal_Int64 ZipOutputEntry::getDeflaterTotalOut() const
{
    return m_aDeflater.getTotalOut();
}

void ZipOutputEntry::deflaterReset()
{
    m_aDeflater.reset();
}

bool ZipOutputEntry::isDeflaterFinished() const
{
    return m_aDeflater.finished();
}


ZipOutputEntryInThread::ZipOutputEntryInThread(
        const uno::Reference< uno::XComponentContext >& rxContext,
        std::unique_ptr<ZipEntry>&& pEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntry( uno::Reference< css::io::XOutputStream >(), rxContext, pEntry.get(), pStream, bEncrypt, false )
, m_pOwnedZipEntry(std::move(pEntry))
, m_bFinished(false)
{
}

void ZipOutputEntryInThread::createBufferFile()
{
    assert(!m_xOutStream && !m_xTempFile &&
           "should only be called in the threaded mode where there is no existing stream yet");
    m_xTempFile = new utl::TempFileFastService;
    m_xOutStream = m_xTempFile->getOutputStream();
}

void ZipOutputEntryInThread::closeBufferFile()
{
    m_xOutStream->closeOutput();
    m_xOutStream.clear();
}

void ZipOutputEntryInThread::deleteBufferFile()
{
    assert(!m_xOutStream.is() && m_xTempFile);
    m_xTempFile.clear();
}

uno::Reference< io::XInputStream > ZipOutputEntryInThread::getData() const
{
    return m_xTempFile->getInputStream();
}

class ZipOutputEntryInThread::Task : public comphelper::ThreadTask
{
    ZipOutputEntryInThread *mpEntry;
    uno::Reference< io::XInputStream > mxInStream;

public:
    Task( const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, ZipOutputEntryInThread *pEntry,
          uno::Reference< io::XInputStream > xInStream )
        : comphelper::ThreadTask(pTag)
        , mpEntry(pEntry)
        , mxInStream(std::move(xInStream))
    {}

private:
    virtual void doWork() override
    {
        try
        {
            mpEntry->createBufferFile();
            mpEntry->writeStream(mxInStream);
            mxInStream.clear();
            mpEntry->closeBufferFile();
            mpEntry->setFinished();
        }
        catch (...)
        {
            mpEntry->setParallelDeflateException(std::current_exception());
            try
            {
                if (mpEntry->m_xOutStream.is())
                    mpEntry->closeBufferFile();
                if (mpEntry->m_xTempFile)
                    mpEntry->deleteBufferFile();
            }
            catch (uno::Exception const&)
            {
            }
            mpEntry->setFinished();
        }
    }
};

std::unique_ptr<comphelper::ThreadTask> ZipOutputEntryInThread::createTask(
    const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
    const uno::Reference< io::XInputStream >& xInStream )
{
    return std::make_unique<Task>(pTag, this, xInStream);
}

void ZipOutputEntry::writeStream(const uno::Reference< io::XInputStream >& xInStream)
{
    sal_Int32 nLength = 0;
    uno::Sequence< sal_Int8 > aSeq(n_ConstBufferSize);
    do
    {
        nLength = xInStream->readBytes(aSeq, n_ConstBufferSize);
        if (nLength != n_ConstBufferSize)
            aSeq.realloc(nLength);

        write(aSeq);
    }
    while (nLength == n_ConstBufferSize);
    closeEntry();
}


ZipOutputEntryParallel::ZipOutputEntryParallel(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry* pEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntryBase(rxOutput, rxContext, pEntry, pStream, bEncrypt, true)
, totalIn(0)
, totalOut(0)
, finished(false)
{
}

void ZipOutputEntryParallel::writeStream(const uno::Reference< io::XInputStream >& xInStream)
{
    ZipUtils::ThreadedDeflater deflater( DEFAULT_COMPRESSION );
    deflater.deflateWrite(xInStream,
            [this](const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nLen) {
                if (!m_bEncryptCurrentEntry)
                    m_aCRC.updateSegment(rBuffer, nLen);
            },
            [this](const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nLen) {
                processDeflated(rBuffer, nLen);
            }
    );
    finished = true;
    processDeflated( uno::Sequence< sal_Int8 >(), 0 ); // finish encrypting, etc.
    totalIn = deflater.getTotalIn();
    totalOut = deflater.getTotalOut();
    closeEntry();
}

void ZipOutputEntryParallel::finishDeflater()
{
    // ThreadedDeflater is called synchronously in one call, so nothing to do here.
}

sal_Int64 ZipOutputEntryParallel::getDeflaterTotalIn() const
{
    return totalIn;
}

sal_Int64 ZipOutputEntryParallel::getDeflaterTotalOut() const
{
    return totalOut;
}

void ZipOutputEntryParallel::deflaterReset()
{
    totalIn = 0;
    totalOut = 0;
    finished = false;
}

bool ZipOutputEntryParallel::isDeflaterFinished() const
{
    return finished;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
