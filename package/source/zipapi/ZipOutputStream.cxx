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
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/diagnose.h>

#include <osl/time.h>

#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipOutputEntry.hxx>
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
, m_pCurrentEntry(NULL)
, m_rSharedThreadPool(comphelper::ThreadPool::getSharedOptimalPool())
{
}

ZipOutputStream::~ZipOutputStream()
{
}

void ZipOutputStream::setEntry( ZipEntry *pEntry )
{
    if (pEntry->nTime == -1)
        pEntry->nTime = getCurrentDosTime();
    if (pEntry->nMethod == -1)
        pEntry->nMethod = DEFLATED;
    pEntry->nVersion = 20;
    pEntry->nFlag = 1 << 11;
    if (pEntry->nSize == -1 || pEntry->nCompressedSize == -1 ||
        pEntry->nCrc == -1)
    {
        pEntry->nSize = pEntry->nCompressedSize = 0;
        pEntry->nFlag |= 8;
    }
}

void ZipOutputStream::addDeflatingThread( ZipOutputEntry *pEntry, comphelper::ThreadTask *pThread )
{
    m_rSharedThreadPool.pushTask(pThread);
    m_aEntries.push_back(pEntry);
}

void ZipOutputStream::rawWrite( const Sequence< sal_Int8 >& rBuffer )
    throw(IOException, RuntimeException)
{
    m_aChucker.WriteBytes( rBuffer );
}

void ZipOutputStream::rawCloseEntry( bool bEncrypt )
    throw(IOException, RuntimeException)
{
    assert(m_pCurrentEntry && "Forgot to call writeLOC()?");
    if ( m_pCurrentEntry->nMethod == DEFLATED && ( m_pCurrentEntry->nFlag & 8 ) )
        writeEXT(*m_pCurrentEntry);

    if (bEncrypt)
        m_pCurrentEntry->nMethod = STORED;

    m_pCurrentEntry = NULL;
}

void ZipOutputStream::finish()
    throw(IOException, RuntimeException)
{
    assert(!m_aZipList.empty() && "Zip file must have at least one entry!");

    // Wait for all threads to finish & write
    m_rSharedThreadPool.waitUntilEmpty();
    for (size_t i = 0; i < m_aEntries.size(); i++)
    {
        //Any exceptions thrown in the threads were caught and stored for now
        ::css::uno::Any aCaughtException(m_aEntries[i]->getParallelDeflateException());
        if (aCaughtException.hasValue())
            ::cppu::throwException(aCaughtException);

        writeLOC(m_aEntries[i]->getZipEntry(), m_aEntries[i]->isEncrypt());

        sal_Int32 nRead;
        uno::Sequence< sal_Int8 > aSequence(n_ConstBufferSize);
        uno::Reference< io::XInputStream > xInput = m_aEntries[i]->getData();
        do
        {
            nRead = xInput->readBytes(aSequence, n_ConstBufferSize);
            if (nRead < n_ConstBufferSize)
                aSequence.realloc(nRead);

            rawWrite(aSequence);
        }
        while (nRead == n_ConstBufferSize);
        xInput.clear();

        rawCloseEntry(m_aEntries[i]->isEncrypt());

        m_aEntries[i]->getZipPackageStream()->successfullyWritten(m_aEntries[i]->getZipEntry());
        m_aEntries[i]->deleteBufferFile();
        delete m_aEntries[i];
    }

    sal_Int32 nOffset= static_cast < sal_Int32 > (m_aChucker.GetPosition());
    for (size_t i = 0; i < m_aZipList.size(); i++)
    {
        writeCEN( *m_aZipList[i] );
        delete m_aZipList[i];
    }
    writeEND( nOffset, static_cast < sal_Int32 > (m_aChucker.GetPosition()) - nOffset);
    m_xStream->flush();
    m_aZipList.clear();
}

css::uno::Reference< css::io::XOutputStream > ZipOutputStream::getStream()
{
    return m_xStream;
}

void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
    throw(IOException, RuntimeException)
{
    m_aChucker.WriteInt32( ENDSIG );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( m_aZipList.size() );
    m_aChucker.WriteInt16( m_aZipList.size() );
    m_aChucker.WriteUInt32( nLength );
    m_aChucker.WriteUInt32( nOffset );
    m_aChucker.WriteInt16( 0 );
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

    m_aChucker.WriteInt32( CENSIG );
    m_aChucker.WriteInt16( rEntry.nVersion );
    m_aChucker.WriteInt16( rEntry.nVersion );
    m_aChucker.WriteInt16( rEntry.nFlag );
    m_aChucker.WriteInt16( rEntry.nMethod );
    bool bWrite64Header = false;

    m_aChucker.WriteUInt32( rEntry.nTime );
    m_aChucker.WriteUInt32( rEntry.nCrc );
    m_aChucker.WriteUInt32( getTruncated( rEntry.nCompressedSize, &bWrite64Header ) );
    m_aChucker.WriteUInt32( getTruncated( rEntry.nSize, &bWrite64Header ) );
    m_aChucker.WriteInt16( nNameLength );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt32( 0 );
    m_aChucker.WriteUInt32( getTruncated( rEntry.nOffset, &bWrite64Header ) );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }

    Sequence < sal_Int8 > aSequence( reinterpret_cast<sal_Int8 const *>(sUTF8Name.getStr()), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );
}

void ZipOutputStream::writeEXT( const ZipEntry &rEntry )
    throw(IOException, RuntimeException)
{
    bool bWrite64Header = false;

    m_aChucker.WriteInt32( EXTSIG );
    m_aChucker.WriteUInt32( rEntry.nCrc );
    m_aChucker.WriteUInt32( getTruncated( rEntry.nCompressedSize, &bWrite64Header ) );
    m_aChucker.WriteUInt32( getTruncated( rEntry.nSize, &bWrite64Header ) );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }
}

void ZipOutputStream::writeLOC( ZipEntry *pEntry, bool bEncrypt )
    throw(IOException, RuntimeException)
{
    assert(!m_pCurrentEntry && "Forgot to close an entry with rawCloseEntry()?");
    m_pCurrentEntry = pEntry;
    m_aZipList.push_back( m_pCurrentEntry );
    const ZipEntry &rEntry = *m_pCurrentEntry;

    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException("Unexpected character is used in file name." );

    OString sUTF8Name = OUStringToOString( rEntry.sPath, RTL_TEXTENCODING_UTF8 );
    sal_Int16 nNameLength       = static_cast < sal_Int16 > ( sUTF8Name.getLength() );

    m_aChucker.WriteInt32( LOCSIG );
    m_aChucker.WriteInt16( rEntry.nVersion );

    m_aChucker.WriteInt16( rEntry.nFlag );
    // If it's an encrypted entry, we pretend its stored plain text
    if (bEncrypt)
        m_aChucker.WriteInt16( STORED );
    else
        m_aChucker.WriteInt16( rEntry.nMethod );

    bool bWrite64Header = false;

    m_aChucker.WriteUInt32( rEntry.nTime );
    if ((rEntry.nFlag & 8) == 8 )
    {
        m_aChucker.WriteInt32( 0 );
        m_aChucker.WriteInt32( 0 );
        m_aChucker.WriteInt32( 0 );
    }
    else
    {
        m_aChucker.WriteUInt32( rEntry.nCrc );
        m_aChucker.WriteUInt32( getTruncated( rEntry.nCompressedSize, &bWrite64Header ) );
        m_aChucker.WriteUInt32( getTruncated( rEntry.nSize, &bWrite64Header ) );
    }
    m_aChucker.WriteInt16( nNameLength );
    m_aChucker.WriteInt16( 0 );

    if( bWrite64Header )
    {
        // FIXME64: need to append a ZIP64 header instead of throwing
        // We're about to silently lose people's data - which they are
        // unlikely to appreciate so fail instead:
        throw IOException( "File contains streams that are too large." );
    }

    Sequence < sal_Int8 > aSequence( reinterpret_cast<sal_Int8 const *>(sUTF8Name.getStr()), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );

    m_pCurrentEntry->nOffset = m_aChucker.GetPosition() - (LOCHDR + nNameLength);
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
