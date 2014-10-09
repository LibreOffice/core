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

#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>

#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipFile.hxx>
#include <ZipPackageStream.hxx>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to deflate Zip entries
 */
ZipOutputEntry::ZipOutputEntry( const uno::Reference< uno::XComponentContext >& rxContext,
                        ByteChucker& rChucker,
                        ZipEntry& rEntry,
                        ZipPackageStream* pStream,
                        bool bEncrypt)
: m_aDeflateBuffer(n_ConstBufferSize)
, m_aDeflater(DEFAULT_COMPRESSION, true)
, m_rChucker(rChucker)
, m_pCurrentEntry(&rEntry)
, m_nDigested(0)
, m_bEncryptCurrentEntry(false)
, m_pCurrentStream(NULL)
{
    if (rEntry.nTime == -1)
        rEntry.nTime = getCurrentDosTime();
    if (rEntry.nMethod == -1)
        rEntry.nMethod = DEFLATED;
    rEntry.nVersion = 20;
    rEntry.nFlag = 1 << 11;
    if (rEntry.nSize == -1 || rEntry.nCompressedSize == -1 ||
        rEntry.nCrc == -1)
    {
        rEntry.nSize = rEntry.nCompressedSize = 0;
        rEntry.nFlag |= 8;
    }

    if (bEncrypt)
    {
        m_bEncryptCurrentEntry = true;

        m_xCipherContext = ZipFile::StaticGetCipher( rxContext, pStream->GetEncryptionData(), true );
        m_xDigestContext = ZipFile::StaticGetDigestContextForChecksum( rxContext, pStream->GetEncryptionData() );
        m_nDigested = 0;
        rEntry.nFlag |= 1 << 4;
        m_pCurrentStream = pStream;
    }
    sal_Int32 nLOCLength = writeLOC(rEntry);
    rEntry.nOffset = m_rChucker.GetPosition() - nLOCLength;
}

ZipOutputEntry::~ZipOutputEntry( void )
{
}

void SAL_CALL ZipOutputEntry::closeEntry(  )
    throw(IOException, RuntimeException)
{
    ZipEntry *pEntry = m_pCurrentEntry;
    if (pEntry)
    {
        switch (pEntry->nMethod)
        {
            case DEFLATED:
                m_aDeflater.finish();
                while (!m_aDeflater.finished())
                    doDeflate();
                if ((pEntry->nFlag & 8) == 0)
                {
                    if (pEntry->nSize != m_aDeflater.getTotalIn())
                    {
                        OSL_FAIL("Invalid entry size");
                    }
                    if (pEntry->nCompressedSize != m_aDeflater.getTotalOut())
                    {
                        // Different compression strategies make the merit of this
                        // test somewhat dubious
                        pEntry->nCompressedSize = m_aDeflater.getTotalOut();
                    }
                    if (pEntry->nCrc != m_aCRC.getValue())
                    {
                        OSL_FAIL("Invalid entry CRC-32");
                    }
                }
                else
                {
                    if ( !m_bEncryptCurrentEntry )
                    {
                        pEntry->nSize = m_aDeflater.getTotalIn();
                        pEntry->nCompressedSize = m_aDeflater.getTotalOut();
                    }
                    pEntry->nCrc = m_aCRC.getValue();
                    writeEXT(*pEntry);
                }
                m_aDeflater.reset();
                m_aCRC.reset();
                break;
            case STORED:
                if (!((pEntry->nFlag & 8) == 0))
                    OSL_FAIL( "Serious error, one of compressed size, size or CRC was -1 in a STORED stream");
                break;
            default:
                OSL_FAIL("Invalid compression method");
                break;
        }

        if (m_bEncryptCurrentEntry)
        {
            m_bEncryptCurrentEntry = false;

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
        m_pCurrentEntry = NULL;
        m_pCurrentStream = NULL;
    }
}

void SAL_CALL ZipOutputEntry::write( const Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
{
    switch (m_pCurrentEntry->nMethod)
    {
        case DEFLATED:
            if (!m_aDeflater.finished())
            {
                m_aDeflater.setInputSegment(rBuffer, nNewOffset, nNewLength);
                 while (!m_aDeflater.needsInput())
                    doDeflate();
                if (!m_bEncryptCurrentEntry)
                    m_aCRC.updateSegment(rBuffer, nNewOffset, nNewLength);
            }
            break;
        case STORED:
            {
                Sequence < sal_Int8 > aTmpBuffer ( rBuffer.getConstArray(), nNewLength );
                m_rChucker.WriteBytes( aTmpBuffer );
            }
            break;
    }
}

void SAL_CALL ZipOutputEntry::rawWrite( Sequence< sal_Int8 >& rBuffer, sal_Int32 /*nNewOffset*/, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
{
    Sequence < sal_Int8 > aTmpBuffer ( rBuffer.getConstArray(), nNewLength );
    m_rChucker.WriteBytes( aTmpBuffer );
}

void SAL_CALL ZipOutputEntry::rawCloseEntry(  )
    throw(IOException, RuntimeException)
{
    if ( m_pCurrentEntry->nMethod == DEFLATED && ( m_pCurrentEntry->nFlag & 8 ) )
        writeEXT(*m_pCurrentEntry);
    m_pCurrentEntry = NULL;
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

            m_rChucker.WriteBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated here
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
        else
        {
            m_rChucker.WriteBytes ( aTmpBuffer );
        }
    }

    if ( m_aDeflater.finished() && m_bEncryptCurrentEntry && m_xDigestContext.is() && m_xCipherContext.is() )
    {
        // FIXME64: sequence not 64bit safe.
        uno::Sequence< sal_Int8 > aEncryptionBuffer = m_xCipherContext->finalizeCipherContextAndDispose();
        if ( aEncryptionBuffer.getLength() )
        {
            m_rChucker.WriteBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated hier
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
    }
}

static sal_uInt32 getTruncated( sal_Int64 nNum, bool *pIsTruncated )
{
    if( nNum >= 0xffffffff )
    {
        *pIsTruncated = true;
        return 0xffffffff;
    }
    else
        return static_cast< sal_uInt32 >( nNum );
}

void ZipOutputEntry::writeEXT( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    bool bWrite64Header = false;

    m_rChucker << EXTSIG;
    m_rChucker << static_cast < sal_uInt32> ( rEntry.nCrc );
    m_rChucker << getTruncated( rEntry.nCompressedSize, &bWrite64Header );
    m_rChucker << getTruncated( rEntry.nSize, &bWrite64Header );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }
}

sal_Int32 ZipOutputEntry::writeLOC( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException("Unexpected character is used in file name." );

    OString sUTF8Name = OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    m_rChucker << LOCSIG;
    m_rChucker << rEntry.nVersion;

    if (rEntry.nFlag & (1 << 4) )
    {
        // If it's an encrypted entry, we pretend its stored plain text
        sal_Int16 nTmpFlag = rEntry.nFlag;
        nTmpFlag &= ~(1 <<4 );
        m_rChucker << nTmpFlag;
        m_rChucker << static_cast < sal_Int16 > ( STORED );
    }
    else
    {
        m_rChucker << rEntry.nFlag;
        m_rChucker << rEntry.nMethod;
    }

    bool bWrite64Header = false;

    m_rChucker << static_cast < sal_uInt32 > (rEntry.nTime);
    if ((rEntry.nFlag & 8) == 8 )
    {
        m_rChucker << static_cast < sal_Int32 > (0);
        m_rChucker << static_cast < sal_Int32 > (0);
        m_rChucker << static_cast < sal_Int32 > (0);
    }
    else
    {
        m_rChucker << static_cast < sal_uInt32 > (rEntry.nCrc);
        m_rChucker << getTruncated( rEntry.nCompressedSize, &bWrite64Header );
        m_rChucker << getTruncated( rEntry.nSize, &bWrite64Header );
    }
    m_rChucker << nNameLength;
    m_rChucker << static_cast < sal_Int16 > (0);

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }

    Sequence < sal_Int8 > aSequence( (sal_Int8*)sUTF8Name.getStr(), sUTF8Name.getLength() );
    m_rChucker.WriteBytes( aSequence );

    return LOCHDR + nNameLength;
}
sal_uInt32 ZipOutputEntry::getCurrentDosTime( )
{
    oslDateTime aDateTime;
    TimeValue aTimeValue;
    osl_getSystemTime ( &aTimeValue );
    osl_getDateTimeFromTimeValue( &aTimeValue, &aDateTime);

    // at year 2108, there is an overflow
    // -> some decision needs to be made
    // how to handle the ZIP file format (just overflow?)

    // if the current system time is before 1980,
    // then the time traveller will have to make a decision
    // how to handle the ZIP file format before it is invented
    // (just underflow?)

    assert(aDateTime.Year > 1980 && aDateTime.Year < 2108);

    sal_uInt32 nYear = static_cast <sal_uInt32> (aDateTime.Year);

    if (nYear>=1980)
        nYear-=1980;
    else if (nYear>=80)
    {
        nYear-=80;
    }
    sal_uInt32 nResult = static_cast < sal_uInt32>( ( ( ( aDateTime.Day) +
                                          ( 32 * (aDateTime.Month)) +
                                          ( 512 * nYear ) ) << 16) |
                                        ( ( aDateTime.Seconds/2) +
                                            ( 32 * aDateTime.Minutes) +
                                          ( 2048 * static_cast <sal_uInt32 > (aDateTime.Hours) ) ) );
    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
