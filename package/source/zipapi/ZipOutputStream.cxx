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

#include <ZipOutputStream.hxx>

#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>

#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipPackageStream.hxx>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( const uno::Reference < io::XOutputStream > &xOStream )
: m_xStream(xOStream)
, m_aChucker(xOStream)
, m_bFinished(false)
, m_pCurrentEntry(NULL)
{
}

ZipOutputStream::~ZipOutputStream( void )
{
    for (sal_Int32 i = 0, nEnd = m_aZipList.size(); i < nEnd; i++)
        delete m_aZipList[i];
}

void ZipOutputStream::putNextEntry( ZipEntry& rEntry, bool bEncrypt )
    throw(IOException, RuntimeException)
{
    assert(!m_pCurrentEntry && "Forgot to close an entry before putNextEntry()?");
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
    m_bEncrypt = bEncrypt;

    sal_Int32 nLOCLength = writeLOC(rEntry);
    rEntry.nOffset = m_aChucker.GetPosition() - nLOCLength;
    m_aZipList.push_back( &rEntry );
    m_pCurrentEntry = &rEntry;
}

void ZipOutputStream::rawWrite( Sequence< sal_Int8 >& rBuffer, sal_Int32 /*nNewOffset*/, sal_Int32 nNewLength )
    throw(IOException, RuntimeException)
{
    m_aChucker.WriteBytes( Sequence< sal_Int8 >(rBuffer.getConstArray(), nNewLength) );
}

void ZipOutputStream::rawCloseEntry()
    throw(IOException, RuntimeException)
{
    if ( m_pCurrentEntry->nMethod == DEFLATED && ( m_pCurrentEntry->nFlag & 8 ) )
        writeEXT(*m_pCurrentEntry);

    if (m_bEncrypt)
        m_pCurrentEntry->nMethod = STORED;

    m_pCurrentEntry = NULL;
}

void ZipOutputStream::finish(  )
    throw(IOException, RuntimeException)
{
    if (m_bFinished)
        return;

    if (m_aZipList.size() < 1)
        OSL_FAIL("Zip file must have at least one entry!\n");

    sal_Int32 nOffset= static_cast < sal_Int32 > (m_aChucker.GetPosition());
    for (sal_Int32 i =0, nEnd = m_aZipList.size(); i < nEnd; i++)
        writeCEN( *m_aZipList[i] );
    writeEND( nOffset, static_cast < sal_Int32 > (m_aChucker.GetPosition()) - nOffset);
    m_bFinished = true;
    m_xStream->flush();
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
    m_aChucker << rEntry.nFlag;
    m_aChucker << rEntry.nMethod;
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

    m_aChucker << rEntry.nFlag;
    // If it's an encrypted entry, we pretend its stored plain text
    if (m_bEncrypt)
        m_aChucker << static_cast < sal_Int16 > ( STORED );
    else
        m_aChucker << rEntry.nMethod;

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

sal_uInt32 ZipOutputStream::getCurrentDosTime()
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
