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

#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>

#include <EncryptionData.hxx>
#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipFile.hxx>
#include <ZipPackageStream.hxx>
#include <ZipOutputStream.hxx>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( const uno::Reference< uno::XComponentContext >& rxContext,
                                  const uno::Reference < XOutputStream > &xOStream )
: m_xContext( rxContext )
, m_xStream(xOStream)
, m_aDeflateBuffer(n_ConstBufferSize)
, m_aDeflater(DEFAULT_COMPRESSION, true)
, m_aChucker(xOStream)
, m_pCurrentEntry(NULL)
, m_nDigested(0)
, m_bFinished(false)
, m_bEncryptCurrentEntry(false)
, m_pCurrentStream(NULL)
{
}

ZipOutputStream::~ZipOutputStream( void )
{
    for (sal_Int32 i = 0, nEnd = m_aZipList.size(); i < nEnd; i++)
        delete m_aZipList[i];
}

void SAL_CALL ZipOutputStream::putNextEntry( ZipEntry& rEntry,
                        ZipPackageStream* pStream,
                        bool bEncrypt)
    throw(IOException, RuntimeException)
{
    if (m_pCurrentEntry != NULL)
        closeEntry();
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

        m_xCipherContext = ZipFile::StaticGetCipher( m_xContext, pStream->GetEncryptionData(), true );
        m_xDigestContext = ZipFile::StaticGetDigestContextForChecksum( m_xContext, pStream->GetEncryptionData() );
        m_nDigested = 0;
        rEntry.nFlag |= 1 << 4;
        m_pCurrentStream = pStream;
    }
    sal_Int32 nLOCLength = writeLOC(rEntry);
    rEntry.nOffset = m_aChucker.GetPosition() - nLOCLength;
    m_aZipList.push_back( &rEntry );
    m_pCurrentEntry = &rEntry;
}

void SAL_CALL ZipOutputStream::closeEntry(  )
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

void SAL_CALL ZipOutputStream::write( const Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
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
                m_aChucker.WriteBytes( aTmpBuffer );
            }
            break;
    }
}

void SAL_CALL ZipOutputStream::rawWrite( Sequence< sal_Int8 >& rBuffer, sal_Int32 /*nNewOffset*/, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
{
    Sequence < sal_Int8 > aTmpBuffer ( rBuffer.getConstArray(), nNewLength );
    m_aChucker.WriteBytes( aTmpBuffer );
}

void SAL_CALL ZipOutputStream::rawCloseEntry(  )
    throw(IOException, RuntimeException)
{
    if ( m_pCurrentEntry->nMethod == DEFLATED && ( m_pCurrentEntry->nFlag & 8 ) )
        writeEXT(*m_pCurrentEntry);
    m_pCurrentEntry = NULL;
}

void SAL_CALL ZipOutputStream::finish(  )
    throw(IOException, RuntimeException)
{
    if (m_bFinished)
        return;

    if (m_pCurrentEntry != NULL)
        closeEntry();

    if (m_aZipList.size() < 1)
        OSL_FAIL("Zip file must have at least one entry!\n");

    sal_Int32 nOffset= static_cast < sal_Int32 > (m_aChucker.GetPosition());
    for (sal_Int32 i =0, nEnd = m_aZipList.size(); i < nEnd; i++)
        writeCEN( *m_aZipList[i] );
    writeEND( nOffset, static_cast < sal_Int32 > (m_aChucker.GetPosition()) - nOffset);
    m_bFinished = true;
    m_xStream->flush();
}

void ZipOutputStream::doDeflate()
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

            m_aChucker.WriteBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated here
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
        else
        {
            m_aChucker.WriteBytes ( aTmpBuffer );
        }
    }

    if ( m_aDeflater.finished() && m_bEncryptCurrentEntry && m_xDigestContext.is() && m_xCipherContext.is() )
    {
        // FIXME64: sequence not 64bit safe.
        uno::Sequence< sal_Int8 > aEncryptionBuffer = m_xCipherContext->finalizeCipherContextAndDispose();
        if ( aEncryptionBuffer.getLength() )
        {
            m_aChucker.WriteBytes( aEncryptionBuffer );

            // the sizes as well as checksum for encrypted streams is calculated hier
            m_pCurrentEntry->nCompressedSize += aEncryptionBuffer.getLength();
            m_pCurrentEntry->nSize = m_pCurrentEntry->nCompressedSize;
            m_aCRC.update( aEncryptionBuffer );
        }
    }
}

void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
    throw(IOException, RuntimeException)
{
    m_aChucker << ENDSIG;
    m_aChucker << static_cast < sal_Int16 > ( 0 );
    m_aChucker << static_cast < sal_Int16 > ( 0 );
    m_aChucker << static_cast < sal_Int16 > ( m_aZipList.size() );
    m_aChucker << static_cast < sal_Int16 > ( m_aZipList.size() );
    m_aChucker << nLength;
    m_aChucker << nOffset;
    m_aChucker << static_cast < sal_Int16 > ( 0 );
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

void ZipOutputStream::writeCEN( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException("Unexpected character is used in file name." );

    OString sUTF8Name = OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    m_aChucker << CENSIG;
    m_aChucker << rEntry.nVersion;
    m_aChucker << rEntry.nVersion;
    if (rEntry.nFlag & (1 << 4) )
    {
        // If it's an encrypted entry, we pretend its stored plain text
        ZipEntry *pEntry = const_cast < ZipEntry * > ( &rEntry );
        pEntry->nFlag &= ~(1 <<4 );
        m_aChucker << rEntry.nFlag;
        m_aChucker << static_cast < sal_Int16 > ( STORED );
    }
    else
    {
        m_aChucker << rEntry.nFlag;
        m_aChucker << rEntry.nMethod;
    }
    bool bWrite64Header = false;

    m_aChucker << static_cast < sal_uInt32> ( rEntry.nTime );
    m_aChucker << static_cast < sal_uInt32> ( rEntry.nCrc );
    m_aChucker << getTruncated( rEntry.nCompressedSize, &bWrite64Header );
    m_aChucker << getTruncated( rEntry.nSize, &bWrite64Header );
    m_aChucker << nNameLength;
    m_aChucker << static_cast < sal_Int16> (0);
    m_aChucker << static_cast < sal_Int16> (0);
    m_aChucker << static_cast < sal_Int16> (0);
    m_aChucker << static_cast < sal_Int16> (0);
    m_aChucker << static_cast < sal_Int32> (0);
    m_aChucker << getTruncated( rEntry.nOffset, &bWrite64Header );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }

    Sequence < sal_Int8 > aSequence( (sal_Int8*)sUTF8Name.getStr(), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );
}
void ZipOutputStream::writeEXT( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    bool bWrite64Header = false;

    m_aChucker << EXTSIG;
    m_aChucker << static_cast < sal_uInt32> ( rEntry.nCrc );
    m_aChucker << getTruncated( rEntry.nCompressedSize, &bWrite64Header );
    m_aChucker << getTruncated( rEntry.nSize, &bWrite64Header );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }
}

sal_Int32 ZipOutputStream::writeLOC( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException("Unexpected character is used in file name." );

    OString sUTF8Name = OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    m_aChucker << LOCSIG;
    m_aChucker << rEntry.nVersion;

    if (rEntry.nFlag & (1 << 4) )
    {
        // If it's an encrypted entry, we pretend its stored plain text
        sal_Int16 nTmpFlag = rEntry.nFlag;
        nTmpFlag &= ~(1 <<4 );
        m_aChucker << nTmpFlag;
        m_aChucker << static_cast < sal_Int16 > ( STORED );
    }
    else
    {
        m_aChucker << rEntry.nFlag;
        m_aChucker << rEntry.nMethod;
    }

    bool bWrite64Header = false;

    m_aChucker << static_cast < sal_uInt32 > (rEntry.nTime);
    if ((rEntry.nFlag & 8) == 8 )
    {
        m_aChucker << static_cast < sal_Int32 > (0);
        m_aChucker << static_cast < sal_Int32 > (0);
        m_aChucker << static_cast < sal_Int32 > (0);
    }
    else
    {
        m_aChucker << static_cast < sal_uInt32 > (rEntry.nCrc);
        m_aChucker << getTruncated( rEntry.nCompressedSize, &bWrite64Header );
        m_aChucker << getTruncated( rEntry.nSize, &bWrite64Header );
    }
    m_aChucker << nNameLength;
    m_aChucker << static_cast < sal_Int16 > (0);

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }

    Sequence < sal_Int8 > aSequence( (sal_Int8*)sUTF8Name.getStr(), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );

    return LOCHDR + nNameLength;
}
sal_uInt32 ZipOutputStream::getCurrentDosTime( )
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
/*

   This is actually never used, so I removed it, but thought that the
   implementation details may be useful in the future...mtg 20010307

   I stopped using the time library and used the OSL version instead, but
   it might still be useful to have this code here..

void ZipOutputStream::dosDateToTMDate ( tm &rTime, sal_uInt32 nDosDate)
{
    sal_uInt32 nDate = static_cast < sal_uInt32 > (nDosDate >> 16);
    rTime.tm_mday = static_cast < sal_uInt32 > ( nDate & 0x1F);
    rTime.tm_mon  = static_cast < sal_uInt32 > ( ( ( (nDate) & 0x1E0)/0x20)-1);
    rTime.tm_year = static_cast < sal_uInt32 > ( ( (nDate & 0x0FE00)/0x0200)+1980);

    rTime.tm_hour = static_cast < sal_uInt32 > ( (nDosDate & 0xF800)/0x800);
    rTime.tm_min  = static_cast < sal_uInt32 > ( (nDosDate & 0x7E0)/0x20);
    rTime.tm_sec  = static_cast < sal_uInt32 > ( 2 * (nDosDate & 0x1F) );
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
