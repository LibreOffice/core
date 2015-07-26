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
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>
#include <osl/diagnose.h>

#include <PackageConstants.hxx>
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
ZipOutputEntry::ZipOutputEntry(
        const css::uno::Reference< css::io::XOutputStream >& rxOutput,
        const uno::Reference< uno::XComponentContext >& rxContext,
        ZipEntry& rEntry,
        ZipPackageStream* pStream,
        bool bEncrypt)
: m_aDeflateBuffer(n_ConstBufferSize)
, m_aDeflater(DEFAULT_COMPRESSION, true)
, m_pCurrentEntry(&rEntry)
, m_nDigested(0)
, m_bEncryptCurrentEntry(bEncrypt)
, m_pCurrentStream(pStream)
{
    if (rxOutput.is())
    {
        m_xOutStream = rxOutput;
    }
    else
    {
        m_xTempFile = io::TempFile::create(rxContext);
        m_xOutStream = m_xTempFile->getOutputStream();
    }
    assert(m_pCurrentEntry->nMethod == DEFLATED && "Use ZipPackageStream::rawWrite() for STORED entries");
    if (m_bEncryptCurrentEntry)
    {
        m_xCipherContext = ZipFile::StaticGetCipher( rxContext, pStream->GetEncryptionData(), true );
        m_xDigestContext = ZipFile::StaticGetDigestContextForChecksum( rxContext, pStream->GetEncryptionData() );
    }
}

ZipOutputEntry::~ZipOutputEntry()
{
}

uno::Reference< io::XInputStream > ZipOutputEntry::getData()
{
    m_xOutStream->closeOutput();
    uno::Reference< io::XSeekable > xTempSeek(m_xOutStream, UNO_QUERY_THROW);
    xTempSeek->seek(0);
    return m_xTempFile->getInputStream();
}

void ZipOutputEntry::closeEntry()
{
    m_aDeflater.finish();
    while (!m_aDeflater.finished())
        doDeflate();

    if ((m_pCurrentEntry->nFlag & 8) == 0)
    {
        if (m_pCurrentEntry->nSize != m_aDeflater.getTotalIn())
        {
            OSL_FAIL("Invalid entry size");
        }
        if (m_pCurrentEntry->nCompressedSize != m_aDeflater.getTotalOut())
        {
            // Different compression strategies make the merit of this
            // test somewhat dubious
            m_pCurrentEntry->nCompressedSize = m_aDeflater.getTotalOut();
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
            m_pCurrentEntry->nSize = m_aDeflater.getTotalIn();
            m_pCurrentEntry->nCompressedSize = m_aDeflater.getTotalOut();
        }
        m_pCurrentEntry->nCrc = m_aCRC.getValue();
    }
    m_aDeflater.reset();
    m_aCRC.reset();

    if (m_bEncryptCurrentEntry)
    {
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
}

void ZipOutputEntry::write( const Sequence< sal_Int8 >& rBuffer )
{
    if (!m_aDeflater.finished())
    {
        m_aDeflater.setInputSegment(rBuffer);
         while (!m_aDeflater.needsInput())
            doDeflate();
        if (!m_bEncryptCurrentEntry)
            m_aCRC.updateSegment(rBuffer, rBuffer.getLength());
    }
}

void ZipOutputEntry::doDeflate()
{
    sal_Int32 nLength = m_aDeflater.doDeflateSegment(m_aDeflateBuffer, 0, m_aDeflateBuffer.getLength());

    if ( nLength > 0 )
    {
        uno::Sequence< sal_Int8 > aTmpBuffer( m_aDeflateBuffer.getConstArray(), nLength );
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

    if ( m_aDeflater.finished() && m_bEncryptCurrentEntry && m_xDigestContext.is() && m_xCipherContext.is() )
    {
        // FIXME64: sequence not 64bit safe.
        uno::Sequence< sal_Int8 > aEncryptionBuffer = m_xCipherContext->finalizeCipherContextAndDispose();
        if ( aEncryptionBuffer.getLength() )
        {
            m_xOutStream->writeBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated hier
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
