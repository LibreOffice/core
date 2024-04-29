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
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/storagehelper.hxx>

#include <osl/time.h>
#include <osl/thread.hxx>

#include <PackageConstants.hxx>
#include <ZipEntry.hxx>
#include <ZipOutputEntry.hxx>
#include <ZipPackageStream.hxx>

#include <thread>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip::ZipConstants;

/** This class is used to write Zip files
 */
ZipOutputStream::ZipOutputStream( const uno::Reference < io::XOutputStream > &xOStream )
: m_xStream(xOStream)
, mpThreadTaskTag( comphelper::ThreadPool::createThreadTaskTag() )
, m_aChucker(xOStream)
, m_pCurrentEntry(nullptr)
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

void ZipOutputStream::addDeflatingThreadTask( ZipOutputEntryInThread *pEntry, std::unique_ptr<comphelper::ThreadTask> pTask )
{
    comphelper::ThreadPool::getSharedOptimalPool().pushTask(std::move(pTask));
    m_aEntries.push_back(pEntry);
}

void ZipOutputStream::rawWrite( const Sequence< sal_Int8 >& rBuffer )
{
    m_aChucker.WriteBytes( rBuffer );
}

void ZipOutputStream::rawCloseEntry( bool bEncrypt )
{
    assert(m_pCurrentEntry && "Forgot to call writeLOC()?");
    if ( m_pCurrentEntry->nMethod == DEFLATED && ( m_pCurrentEntry->nFlag & 8 ) )
        writeDataDescriptor(*m_pCurrentEntry);

    if (bEncrypt)
        m_pCurrentEntry->nMethod = STORED;

    m_pCurrentEntry = nullptr;
}

void ZipOutputStream::consumeScheduledThreadTaskEntry(std::unique_ptr<ZipOutputEntryInThread> pCandidate)
{
    //Any exceptions thrown in the threads were caught and stored for now
    const std::exception_ptr& rCaughtException(pCandidate->getParallelDeflateException());
    if (rCaughtException)
    {
        m_aDeflateException = rCaughtException; // store it for later throwing
        // the exception handler in DeflateThreadTask should have cleaned temp file
        return;
    }

    writeLOC(pCandidate->getZipEntry(), pCandidate->isEncrypt());

    sal_Int32 nRead;
    uno::Sequence< sal_Int8 > aSequence(n_ConstBufferSize);
    uno::Reference< io::XInputStream > xInput = pCandidate->getData();
    do
    {
        nRead = xInput->readBytes(aSequence, n_ConstBufferSize);
        if (nRead < n_ConstBufferSize)
            aSequence.realloc(nRead);

        rawWrite(aSequence);
    }
    while (nRead == n_ConstBufferSize);
    xInput.clear();

    rawCloseEntry(pCandidate->isEncrypt());

    pCandidate->getZipPackageStream()->successfullyWritten(pCandidate->getZipEntry());
    pCandidate->deleteBufferFile();
}

void ZipOutputStream::consumeFinishedScheduledThreadTaskEntries()
{
    std::vector< ZipOutputEntryInThread* > aNonFinishedEntries;

    for(ZipOutputEntryInThread* pEntry : m_aEntries)
    {
        if(pEntry->isFinished())
        {
            consumeScheduledThreadTaskEntry(std::unique_ptr<ZipOutputEntryInThread>(pEntry));
        }
        else
        {
            aNonFinishedEntries.push_back(pEntry);
        }
    }

    // always reset to non-consumed entries
    m_aEntries = aNonFinishedEntries;
}

void ZipOutputStream::reduceScheduledThreadTasksToGivenNumberOrLess(std::size_t nThreadTasks)
{
    while(m_aEntries.size() > nThreadTasks)
    {
        consumeFinishedScheduledThreadTaskEntries();

        if(m_aEntries.size() > nThreadTasks)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void ZipOutputStream::finish()
{
    assert(!m_aZipList.empty() && "Zip file must have at least one entry!");

    // Wait for all thread tasks to finish & write
    comphelper::ThreadPool::getSharedOptimalPool().waitUntilDone(mpThreadTaskTag);

    // consume all processed entries
    while(!m_aEntries.empty())
    {
        ZipOutputEntryInThread* pCandidate = m_aEntries.back();
        m_aEntries.pop_back();
        consumeScheduledThreadTaskEntry(std::unique_ptr<ZipOutputEntryInThread>(pCandidate));
    }

    sal_Int32 nOffset= static_cast < sal_Int32 > (m_aChucker.GetPosition());
    for (ZipEntry* p : m_aZipList)
    {
        writeCEN( *p );
        delete p;
    }
    writeEND( nOffset, static_cast < sal_Int32 > (m_aChucker.GetPosition()) - nOffset);
    m_aZipList.clear();

    if (m_aDeflateException)
    {   // throw once all thread tasks are finished and m_aEntries can be released
        std::rethrow_exception(m_aDeflateException);
    }
}

const css::uno::Reference< css::io::XOutputStream >& ZipOutputStream::getStream() const
{
    return m_xStream;
}

void ZipOutputStream::writeEND(sal_uInt32 nOffset, sal_uInt32 nLength)
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
{
    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException(u"Unexpected character is used in file name."_ustr );

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
    sal_uInt32 nOffset32bit = getTruncated( rEntry.nOffset, &bWrite64Header );
    m_aChucker.WriteInt16(nNameLength);
    m_aChucker.WriteInt16( bWrite64Header? 32 : 0 );    //in ZIP64 case extra field is 32byte
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt16( 0 );
    m_aChucker.WriteInt32( 0 );
    m_aChucker.WriteUInt32( nOffset32bit );

    Sequence < sal_Int8 > aSequence( reinterpret_cast<sal_Int8 const *>(sUTF8Name.getStr()), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );

    if (bWrite64Header)
    {
        writeExtraFields( rEntry );
    }
}

void ZipOutputStream::writeDataDescriptor(const ZipEntry& rEntry)
{
    bool bWrite64Header = false;

    m_aChucker.WriteInt32( EXTSIG );
    m_aChucker.WriteUInt32( rEntry.nCrc );
    // For ZIP64(tm) format archives, the compressed and uncompressed sizes are 8 bytes each.
    // TODO: Not sure if this is the "when ZIP64(tm) format is used"
    bWrite64Header = rEntry.nCompressedSize >= 0x100000000 || rEntry.nSize >= 0x100000000;
    if (!bWrite64Header)
    {
        m_aChucker.WriteUInt32( static_cast<sal_uInt32>(rEntry.nCompressedSize) );
        m_aChucker.WriteUInt32( static_cast<sal_uInt32>(rEntry.nSize) );
    }
    else
    {
        m_aChucker.WriteUInt64( rEntry.nCompressedSize );
        m_aChucker.WriteUInt64( rEntry.nSize );
    }
}

void ZipOutputStream::writeExtraFields(const ZipEntry& rEntry)
{
    //Could contain more fields, now we only save Zip64 extended information
    m_aChucker.WriteInt16( 1 );  //id of Zip64 extended information extra field
    m_aChucker.WriteInt16( 28 ); //data size of this field = 3*8+4 byte
    m_aChucker.WriteUInt64( rEntry.nSize );
    m_aChucker.WriteUInt64( rEntry.nCompressedSize );
    m_aChucker.WriteUInt64( rEntry.nOffset );
    m_aChucker.WriteInt32( 0 );  //Number of the disk on which this file starts
}

void ZipOutputStream::writeLOC( ZipEntry *pEntry, bool bEncrypt )
{
    assert(!m_pCurrentEntry && "Forgot to close an entry with rawCloseEntry()?");
    m_pCurrentEntry = pEntry;
    m_aZipList.push_back( m_pCurrentEntry );
    const ZipEntry &rEntry = *m_pCurrentEntry;

    if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( rEntry.sPath, true ) )
        throw IOException(u"Unexpected character is used in file name."_ustr );

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
    m_aChucker.WriteInt16( bWrite64Header ? 32 : 0 );

    Sequence < sal_Int8 > aSequence( reinterpret_cast<sal_Int8 const *>(sUTF8Name.getStr()), sUTF8Name.getLength() );
    m_aChucker.WriteBytes( aSequence );

    m_pCurrentEntry->nOffset = m_aChucker.GetPosition() - (LOCHDR + nNameLength);

    if (bWrite64Header)
    {
        writeExtraFields(rEntry);
    }
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
