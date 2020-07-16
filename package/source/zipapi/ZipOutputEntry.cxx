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
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>

#include <osl/diagnose.h>

#include <PackageConstants.hxx>
#include <ThreadedDeflater.hxx>
#include <ZipEntry.hxx>
#include <ZipFile.hxx>
#include <ZipPackageBuffer.hxx>
#include <ZipPackageStream.hxx>

#include <algorithm>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to deflate Zip entries
 */
ZipOutputEntryBase::ZipOutputEntryBase(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt,
        bool checkStream)
: m_xContext(rxContext)
, m_xOutStream(rxOutput)
, m_pCurrentEntry(&rEntry)
, m_nDigested(0)
, m_pCurrentStream(pStream)
, m_bEncryptCurrentEntry(bEncrypt)
{
    assert(m_pCurrentEntry->nMethod == DEFLATED && "Use ZipPackageStream::rawWrite() for STORED entries");
    (void)checkStream;
    assert(!checkStream || m_xOutStream.is());
    if (m_bEncryptCurrentEntry)
    {
        m_xCipherContext = ZipFile::StaticGetCipher( m_xContext, pStream->GetEncryptionData(), true );
        m_xDigestContext = ZipFile::StaticGetDigestContextForChecksum( m_xContext, pStream->GetEncryptionData() );
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
        if ( m_bEncryptCurrentEntry && m_xDigestContext.is() && m_xCipherContext.is() )
        {
            // Need to update our digest before encryption...
            sal_Int32 nDiff = n_ConstDigestLength - m_nDigested;
            if ( nDiff )
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

    if ( !(isDeflaterFinished() && m_bEncryptCurrentEntry && m_xDigestContext.is() && m_xCipherContext.is()) )
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
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt,
        bool checkStream)
: ZipOutputEntryBase(rxOutput, rxContext, rEntry, pStream, bEncrypt, checkStream)
, m_aDeflateBuffer(n_ConstBufferSize)
, m_aDeflater(DEFAULT_COMPRESSION, true)
{
}

ZipOutputEntry::ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntry( rxOutput, rxContext, rEntry, pStream, bEncrypt, true)
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
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntry( uno::Reference< css::io::XOutputStream >(), rxContext, rEntry, pStream, bEncrypt, false )
, m_bFinished(false)
{
}

void ZipOutputEntryInThread::createBufferFile()
{
    assert(!m_xOutStream.is() && m_aTempURL.isEmpty() &&
           "should only be called in the threaded mode where there is no existing stream yet");
    uno::Reference < beans::XPropertySet > xTempFileProps(
            io::TempFile::create(m_xContext),
            uno::UNO_QUERY_THROW );
    xTempFileProps->setPropertyValue("RemoveFile", uno::makeAny(false));
    uno::Any aUrl = xTempFileProps->getPropertyValue( "Uri" );
    aUrl >>= m_aTempURL;
    assert(!m_aTempURL.isEmpty());

    uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess(ucb::SimpleFileAccess::create(m_xContext));
    m_xOutStream = xTempAccess->openFileWrite(m_aTempURL);
}

void ZipOutputEntryInThread::closeBufferFile()
{
    m_xOutStream->closeOutput();
    m_xOutStream.clear();
}

void ZipOutputEntryInThread::deleteBufferFile()
{
    assert(!m_xOutStream.is() && !m_aTempURL.isEmpty());
    uno::Reference < ucb::XSimpleFileAccess3 > xAccess(ucb::SimpleFileAccess::create(m_xContext));
    xAccess->kill(m_aTempURL);
}

uno::Reference< io::XInputStream > ZipOutputEntryInThread::getData() const
{
    uno::Reference < ucb::XSimpleFileAccess3 > xTempAccess(ucb::SimpleFileAccess::create(m_xContext));
    return xTempAccess->openFileRead(m_aTempURL);
}

class ZipOutputEntryInThread::Task : public comphelper::ThreadTask
{
    ZipOutputEntryInThread *mpEntry;
    uno::Reference< io::XInputStream > mxInStream;

public:
    Task( const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, ZipOutputEntryInThread *pEntry,
          const uno::Reference< io::XInputStream >& xInStream )
        : comphelper::ThreadTask(pTag)
        , mpEntry(pEntry)
        , mxInStream(xInStream)
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
                if (!mpEntry->m_aTempURL.isEmpty())
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
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: ZipOutputEntryBase(rxOutput, rxContext, rEntry, pStream, bEncrypt, true)
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
