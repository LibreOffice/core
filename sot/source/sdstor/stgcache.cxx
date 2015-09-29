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

#include <string.h>
#include <osl/endian.h>
#include <osl/diagnose.h>
#include <tools/solar.h>

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"

#include <algorithm>

////////////////////////////// class StgPage
// This class implements buffer functionality. The cache will always return
// a page buffer, even if a read fails. It is up to the caller to determine
// the correctness of the I/O.

StgPage::StgPage( short nSize, sal_Int32 nPage )
    : mnPage( nPage )
    , mpData( new sal_uInt8[ nSize ] )
    , mnSize( nSize )
{
    OSL_ENSURE( mnSize >= 512, "Unexpected page size is provided!" );
    // We will write this data to a permanent file later
    // best to clear if first.
    memset( mpData, 0, mnSize );
}

StgPage::~StgPage()
{
    delete [] mpData;
}

rtl::Reference< StgPage > StgPage::Create( short nData, sal_Int32 nPage )
{
    return rtl::Reference< StgPage >( new StgPage( nData, nPage ) );
}

void StgCache::SetToPage ( const rtl::Reference< StgPage >& rPage, short nOff, sal_Int32 nVal )
{
    if( ( nOff < (short) ( rPage->GetSize() / sizeof( sal_Int32 ) ) ) && nOff >= 0 )
    {
#ifdef OSL_BIGENDIAN
        nVal = OSL_SWAPDWORD(nVal);
#endif
        static_cast<sal_Int32*>(rPage->GetData())[ nOff ] = nVal;
        SetDirty( rPage );
    }
}

bool StgPage::IsPageGreater( const StgPage *pA, const StgPage *pB )
{
    return pA->mnPage < pB->mnPage;
}

//////////////////////////////// class StgCache

// The disk cache holds the cached sectors. The sector type differ according
// to their purpose.

static sal_Int32 lcl_GetPageCount( sal_uLong nFileSize, short nPageSize )
{
//    return (nFileSize >= 512) ? (nFileSize - 512) / nPageSize : 0;
    // #i61980# real life: last page may be incomplete, return number of *started* pages
    return (nFileSize >= 512) ? (nFileSize - 512 + nPageSize - 1) / nPageSize : 0;
}

StgCache::StgCache()
   : nError( SVSTREAM_OK )
   , nPages( 0 )
   , nRef( 0 )
   , nReplaceIdx( 0 )
   , maLRUPages( 8 ) // entries in the LRU lookup
   , nPageSize( 512 )
   , pStorageStream( NULL )
   , pStrm( NULL )
   , bMyStream( false )
   , bFile( false )
{
}

StgCache::~StgCache()
{
    Clear();
    SetStrm( NULL, false );
}

void StgCache::SetPhysPageSize( short n )
{
    OSL_ENSURE( n >= 512, "Unexpecte page size is provided!" );
    if ( n >= 512 )
    {
        nPageSize = n;
        sal_uLong nPos = pStrm->Tell();
        sal_uLong nFileSize = pStrm->Seek( STREAM_SEEK_TO_END );
        nPages = lcl_GetPageCount( nFileSize, nPageSize );
        pStrm->Seek( nPos );
    }
}

// Create a new cache element

rtl::Reference< StgPage > StgCache::Create( sal_Int32 nPg )
{
    rtl::Reference< StgPage > xElem( StgPage::Create( nPageSize, nPg ) );
    maLRUPages[ nReplaceIdx++ % maLRUPages.size() ] = xElem;
    return xElem;
}

// Delete the given element

void StgCache::Erase( const rtl::Reference< StgPage > &xElem )
{
    OSL_ENSURE( xElem.is(), "The pointer should not be NULL!" );
    if ( xElem.is() ) {
        for ( LRUList::iterator it = maLRUPages.begin(); it != maLRUPages.end(); ++it ) {
            if ( it->is() && (*it)->GetPage() == xElem->GetPage() ) {
                it->clear();
                break;
            }
        }
    }
}

// remove all cache elements without flushing them

void StgCache::Clear()
{
    maDirtyPages.clear();
    for ( LRUList::iterator it = maLRUPages.begin(); it != maLRUPages.end(); ++it )
        it->clear();
}

// Look for a cached page

rtl::Reference< StgPage > StgCache::Find( sal_Int32 nPage )
{
    for ( LRUList::iterator it = maLRUPages.begin(); it != maLRUPages.end(); ++it )
        if ( it->is() && (*it)->GetPage() == nPage )
            return *it;
    IndexToStgPage::iterator it2 = maDirtyPages.find( nPage );
    if ( it2 != maDirtyPages.end() )
        return it2->second;
    return rtl::Reference< StgPage >();
}

// Load a page into the cache

rtl::Reference< StgPage > StgCache::Get( sal_Int32 nPage, bool bForce )
{
    rtl::Reference< StgPage > p = Find( nPage );
    if( !p.is() )
    {
        p = Create( nPage );
        if( !Read( nPage, p->GetData(), 1 ) && bForce )
        {
            Erase( p );
            p.clear();
            SetError( SVSTREAM_READ_ERROR );
        }
    }
    return p;
}

// Copy an existing page into a new page. Use this routine
// to duplicate an existing stream or to create new entries.
// The new page is initially marked dirty. No owner is copied.

rtl::Reference< StgPage > StgCache::Copy( sal_Int32 nNew, sal_Int32 nOld )
{
    rtl::Reference< StgPage > p = Find( nNew );
    if( !p.is() )
        p = Create( nNew );
    if( nOld >= 0 )
    {
        // old page: we must have this data!
        rtl::Reference< StgPage > q = Get( nOld, true );
        if( q.is() )
        {
            OSL_ENSURE( p->GetSize() == q->GetSize(), "Unexpected page size!" );
            memcpy( p->GetData(), q->GetData(), p->GetSize() );
        }
    }
    SetDirty( p );

    return p;
}

// Historically this wrote pages in a sorted, ascending order;
// continue that tradition.
bool StgCache::Commit()
{
    if ( Good() ) // otherwise Write does nothing
    {
        std::vector< StgPage * > aToWrite;
        for ( IndexToStgPage::iterator aIt = maDirtyPages.begin();
              aIt != maDirtyPages.end(); ++aIt )
            aToWrite.push_back( aIt->second.get() );

        std::sort( aToWrite.begin(), aToWrite.end(), StgPage::IsPageGreater );
        for ( std::vector< StgPage * >::iterator aWr = aToWrite.begin();
              aWr != aToWrite.end(); ++aWr)
        {
            const rtl::Reference< StgPage > &pPage = *aWr;
            if ( !Write( pPage->GetPage(), pPage->GetData(), 1 ) )
                return false;
        }
    }

    maDirtyPages.clear();

    pStrm->Flush();
    SetError( pStrm->GetError() );

    return true;
}

// Set a stream

void StgCache::SetStrm( SvStream* p, bool bMy )
{
    if( pStorageStream )
    {
        pStorageStream->ReleaseRef();
        pStorageStream = NULL;
    }

    if( bMyStream )
        delete pStrm;
    pStrm = p;
    bMyStream = bMy;
}

void StgCache::SetStrm( UCBStorageStream* pStgStream )
{
    if( pStorageStream )
        pStorageStream->ReleaseRef();
    pStorageStream = pStgStream;

    if( bMyStream )
        delete pStrm;

    pStrm = NULL;

    if ( pStorageStream )
    {
        pStorageStream->AddFirstRef();
        pStrm = pStorageStream->GetModifySvStream();
    }

    bMyStream = false;
}

void StgCache::SetDirty( const rtl::Reference< StgPage > &rPage )
{
    assert( IsWritable() );
    maDirtyPages[ rPage->GetPage() ] = rPage;
}

// Open/close the disk file

bool StgCache::Open( const OUString& rName, StreamMode nMode )
{
    // do not open in exclusive mode!
    if( nMode & StreamMode::SHARE_DENYALL )
        nMode = ( ( nMode & ~StreamMode::SHARE_DENYALL ) | StreamMode::SHARE_DENYWRITE );
    SvFileStream* pFileStrm = new SvFileStream( rName, nMode );
    // SvStream "Feature" Write Open auch erfolgreich, wenns nicht klappt
    bool bAccessDenied = false;
    if( ( nMode & StreamMode::WRITE ) && !pFileStrm->IsWritable() )
    {
        pFileStrm->Close();
        bAccessDenied = true;
    }
    SetStrm( pFileStrm, true );
    if( pFileStrm->IsOpen() )
    {
        sal_uLong nFileSize = pStrm->Seek( STREAM_SEEK_TO_END );
        nPages = lcl_GetPageCount( nFileSize, nPageSize );
        pStrm->Seek( 0L );
    }
    else
        nPages = 0;
    bFile = true;
    SetError( bAccessDenied ? ERRCODE_IO_ACCESSDENIED : pStrm->GetError() );
    return Good();
}

void StgCache::Close()
{
    if( bFile )
    {
        static_cast<SvFileStream*>(pStrm)->Close();
        SetError( pStrm->GetError() );
    }
}

// low level I/O

bool StgCache::Read( sal_Int32 nPage, void* pBuf, sal_Int32 nPg )
{
    if( Good() )
    {
        /*  #i73846# real life: a storage may refer to a page one-behind the
            last valid page (see document attached to the issue). In that case
            (if nPage==nPages), just do nothing here and let the caller work on
            the empty zero-filled buffer. */
        if ( nPage > nPages )
            SetError( SVSTREAM_READ_ERROR );
        else if ( nPage < nPages )
        {
            sal_uInt32 nPos = Page2Pos( nPage );
            sal_Int32 nPg2 = ( ( nPage + nPg ) > nPages ) ? nPages - nPage : nPg;
            sal_uInt32 nBytes = nPg2 * nPageSize;
            // fixed address and size for the header
            if( nPage == -1 )
            {
                nPos = 0L, nBytes = 512;
                nPg2 = nPg;
            }
            if( pStrm->Tell() != nPos )
            {
                pStrm->Seek(nPos);
            }
            pStrm->Read( pBuf, nBytes );
            if ( nPg != nPg2 )
                SetError( SVSTREAM_READ_ERROR );
            else
                SetError( pStrm->GetError() );
        }
    }
    return Good();
}

bool StgCache::Write( sal_Int32 nPage, void* pBuf, sal_Int32 nPg )
{
    if( Good() )
    {
        sal_uInt32 nPos = Page2Pos( nPage );
        sal_uInt32 nBytes = 0;
        if ( SAL_MAX_INT32 / nPg > nPageSize )
            nBytes = nPg * nPageSize;

        // fixed address and size for the header
        // nPageSize must be >= 512, otherwise the header can not be written here, we check it on import
        if( nPage == -1 )
            nPos = 0L, nBytes = 512;
        if( pStrm->Tell() != nPos )
        {
            pStrm->Seek(nPos);
        }
        sal_uLong nRes = pStrm->Write( pBuf, nBytes );
        if( nRes != nBytes )
            SetError( SVSTREAM_WRITE_ERROR );
        else
            SetError( pStrm->GetError() );
    }
    return Good();
}

// set the file size in pages

bool StgCache::SetSize( sal_Int32 n )
{
    // Add the file header
    sal_Int32 nSize = n * nPageSize + 512;
    pStrm->SetStreamSize( nSize );
    SetError( pStrm->GetError() );
    if( !nError )
        nPages = n;
    return Good();
}

void StgCache::SetError( sal_uLong n )
{
    if( n && !nError )
        nError = n;
}

void StgCache::ResetError()
{
    nError = SVSTREAM_OK;
    pStrm->ResetError();
}

void StgCache::MoveError( StorageBase& r )
{
    if( nError != SVSTREAM_OK )
    {
        r.SetError( nError );
        ResetError();
    }
}

// Utility functions

sal_Int32 StgCache::Page2Pos( sal_Int32 nPage )
{
    if( nPage < 0 ) nPage = 0;
    return( nPage * nPageSize ) + nPageSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
