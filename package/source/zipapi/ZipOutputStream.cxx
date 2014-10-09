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

#include <PackageConstants.hxx>
#include <ZipEntry.hxx>

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
{
}

ZipOutputStream::~ZipOutputStream( void )
{
    for (sal_Int32 i = 0, nEnd = m_aZipList.size(); i < nEnd; i++)
        delete m_aZipList[i];
}

void ZipOutputStream::addEntry( ZipEntry *pZipEntry )
{
    m_aZipList.push_back( pZipEntry );
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

ByteChucker& ZipOutputStream::getChucker()
{
    return m_aChucker;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
