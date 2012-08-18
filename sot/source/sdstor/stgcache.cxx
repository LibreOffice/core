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


#if defined(_MSC_VER) && (_MSC_VER<1200)
#include <tools/presys.h>
#endif
#include <boost/unordered_map.hpp>
#if defined(_MSC_VER) && (_MSC_VER<1200)
#include <tools/postsys.h>
#endif

#include <string.h>
#include <osl/endian.h>
#include <tools/string.hxx>

#include "sot/stg.hxx"
#include "stgelem.hxx"
#include "stgcache.hxx"
#include "stgstrms.hxx"
#include "stgdir.hxx"
#include "stgio.hxx"

/*************************************************************************/
//-----------------------------------------------------------------------------
typedef boost::unordered_map
<
    sal_Int32,
    StgPage *,
    boost::hash< sal_Int32 >,
    std::equal_to< sal_Int32 >
> UsrStgPagePtr_Impl;
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

//#define   CHECK_DIRTY 1
//#define   READ_AFTER_WRITE 1

////////////////////////////// class StgPage /////////////////////////////
// This class implements buffer functionality. The cache will always return
// a page buffer, even if a read fails. It is up to the caller to determine
// the correctness of the I/O.

StgPage::StgPage( StgCache* p, short n )
{
    OSL_ENSURE( n >= 512, "Unexpected page size is provided!" );
    pCache = p;
    nData  = n;
    bDirty = sal_False;
    nPage  = 0;
    pData  = new sal_uInt8[ nData ];
    pNext1 =
    pNext2 =
    pLast1 =
    pLast2 = NULL;
    pOwner = NULL;
}

StgPage::~StgPage()
{
    delete [] pData;
}

void StgPage::SetPage( short nOff, sal_Int32 nVal )
{
    if( ( nOff < (short) ( nData / sizeof( sal_Int32 ) ) ) && nOff >= 0 )
    {
#ifdef OSL_BIGENDIAN
      nVal = OSL_SWAPDWORD(nVal);
#endif
        ((sal_Int32*) pData )[ nOff ] = nVal;
        bDirty = sal_True;
    }
}

//////////////////////////////// class StgCache ////////////////////////////

// The disk cache holds the cached sectors. The sector type differ according
// to their purpose.

sal_Int32 lcl_GetPageCount( sal_uLong nFileSize, short nPageSize )
{
//    return (nFileSize >= 512) ? (nFileSize - 512) / nPageSize : 0;
    // #i61980# reallife: last page may be incomplete, return number of *started* pages
    return (nFileSize >= 512) ? (nFileSize - 512 + nPageSize - 1) / nPageSize : 0;
}

StgCache::StgCache()
{
    nRef = 0;
    pStrm = NULL;
    pCur = pElem1 = NULL;
    nPageSize = 512;
    nError = SVSTREAM_OK;
    bMyStream = sal_False;
    bFile = sal_False;
    pLRUCache = NULL;
    pStorageStream = NULL;
}

StgCache::~StgCache()
{
    Clear();
    SetStrm( NULL, sal_False );
    delete (UsrStgPagePtr_Impl*)pLRUCache;
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
// pCur points to this element

StgPage* StgCache::Create( sal_Int32 nPg )
{
    StgPage* pElem = new StgPage( this, nPageSize );
    pElem->nPage = nPg;
    // For data security, clear the buffer contents
    memset( pElem->pData, 0, pElem->nData );

    // insert to LRU
    if( pCur )
    {
        pElem->pNext1 = pCur;
        pElem->pLast1 = pCur->pLast1;
        pElem->pNext1->pLast1 =
        pElem->pLast1->pNext1 = pElem;
    }
    else
        pElem->pNext1 = pElem->pLast1 = pElem;
    if( !pLRUCache )
        pLRUCache = new UsrStgPagePtr_Impl();
    (*(UsrStgPagePtr_Impl*)pLRUCache)[pElem->nPage] = pElem;
    pCur = pElem;

    // insert to Sorted
    if( !pElem1 )
        pElem1 = pElem->pNext2 = pElem->pLast2 = pElem;
    else
    {
        StgPage* p = pElem1;
        do
        {
            if( pElem->nPage < p->nPage )
                break;
            p = p->pNext2;
        } while( p != pElem1 );
        pElem->pNext2 = p;
        pElem->pLast2 = p->pLast2;
        pElem->pNext2->pLast2 =
        pElem->pLast2->pNext2 = pElem;
        if( p->nPage < pElem1->nPage )
            pElem1 = pElem;
    }
    return pElem;
}

// Delete the given element

void StgCache::Erase( StgPage* pElem )
{
    OSL_ENSURE( pElem, "The pointer should not be NULL!" );
    if ( pElem )
    {
        OSL_ENSURE( pElem->pNext1 && pElem->pLast1, "The pointers may not be NULL!" );
        //remove from LRU
        pElem->pNext1->pLast1 = pElem->pLast1;
        pElem->pLast1->pNext1 = pElem->pNext1;
        if( pCur == pElem )
            pCur = ( pElem->pNext1 == pElem ) ? NULL : pElem->pNext1;
        if( pLRUCache )
            ((UsrStgPagePtr_Impl*)pLRUCache)->erase( pElem->nPage );
        // remove from Sorted
        pElem->pNext2->pLast2 = pElem->pLast2;
        pElem->pLast2->pNext2 = pElem->pNext2;
        if( pElem1 == pElem )
            pElem1 = ( pElem->pNext2 == pElem ) ? NULL : pElem->pNext2;
        delete pElem;
    }
}

// remove all cache elements without flushing them

void StgCache::Clear()
{
    StgPage* pElem = pCur;
    if( pCur ) do
    {
        StgPage* pDelete = pElem;
        pElem = pElem->pNext1;
        delete pDelete;
    }
    while( pCur != pElem );
    pCur = NULL;
    pElem1 = NULL;
    delete (UsrStgPagePtr_Impl*)pLRUCache;
    pLRUCache = NULL;
}

// Look for a cached page

StgPage* StgCache::Find( sal_Int32 nPage )
{
    if( !pLRUCache )
        return NULL;
    UsrStgPagePtr_Impl::iterator aIt = ((UsrStgPagePtr_Impl*)pLRUCache)->find( nPage );
    if( aIt != ((UsrStgPagePtr_Impl*)pLRUCache)->end() )
    {
        // page found
        StgPage* pFound = (*aIt).second;
        OSL_ENSURE( pFound, "The pointer may not be NULL!" );

        if( pFound != pCur )
        {
            OSL_ENSURE( pFound->pNext1 && pFound->pLast1, "The pointers may not be NULL!" );
            // remove from LRU
            pFound->pNext1->pLast1 = pFound->pLast1;
            pFound->pLast1->pNext1 = pFound->pNext1;
            // insert to LRU
            pFound->pNext1 = pCur;
            pFound->pLast1 = pCur->pLast1;
            pFound->pNext1->pLast1 =
            pFound->pLast1->pNext1 = pFound;
        }
        return pFound;
    }
    return NULL;
}

// Load a page into the cache

StgPage* StgCache::Get( sal_Int32 nPage, sal_Bool bForce )
{
    StgPage* p = Find( nPage );
    if( !p )
    {
        p = Create( nPage );
        if( !Read( nPage, p->pData, 1 ) && bForce )
        {
            Erase( p );
            p = NULL;
            SetError( SVSTREAM_READ_ERROR );
        }
    }
    return p;
}

// Copy an existing page into a new page. Use this routine
// to duplicate an existing stream or to create new entries.
// The new page is initially marked dirty. No owner is copied.

StgPage* StgCache::Copy( sal_Int32 nNew, sal_Int32 nOld )
{
    StgPage* p = Find( nNew );
    if( !p )
        p = Create( nNew );
    if( nOld >= 0 )
    {
        // old page: we must have this data!
        StgPage* q = Get( nOld, sal_True );
        if( q )
        {
            OSL_ENSURE( p->nData == q->nData, "Unexpected page size!" );
            memcpy( p->pData, q->pData, p->nData );
        }
    }
    p->SetDirty();
    return p;
}

// Flush the cache whose owner is given. NULL flushes all.

sal_Bool StgCache::Commit()
{
    StgPage* p = pElem1;
    if( p ) do
    {
        if( p->bDirty )
        {
            sal_Bool b = Write( p->nPage, p->pData, 1 );
            if( !b )
                return sal_False;
            p->bDirty = sal_False;
        }
        p = p->pNext2;
    } while( p != pElem1 );
    pStrm->Flush();
    SetError( pStrm->GetError() );
#ifdef CHECK_DIRTY
    p = pElem1;
    if( p ) do
    {
        if( p->bDirty )
        {
            ErrorBox( NULL, WB_OK, String("SO2: Dirty Block in Ordered List") ).Execute();
            sal_Bool b = Write( p->nPage, p->pData, 1 );
            if( !b )
                return sal_False;
            p->bDirty = sal_False;
        }
        p = p->pNext2;
    } while( p != pElem1 );
    p = pElem1;
    if( p ) do
    {
        if( p->bDirty )
        {
            ErrorBox( NULL, WB_OK, String("SO2: Dirty Block in LRU List") ).Execute();
            sal_Bool b = Write( p->nPage, p->pData, 1 );
            if( !b )
                return sal_False;
            p->bDirty = sal_False;
        }
        p = p->pNext1;
    } while( p != pElem1 );
#endif
    return sal_True;
}

// Set a stream

void StgCache::SetStrm( SvStream* p, sal_Bool bMy )
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
        pStorageStream->AddRef();
        pStrm = pStorageStream->GetModifySvStream();
    }

    bMyStream = sal_False;
}

// Open/close the disk file

sal_Bool StgCache::Open( const String& rName, StreamMode nMode )
{
    // do not open in exclusive mode!
    if( nMode & STREAM_SHARE_DENYALL )
        nMode = ( ( nMode & ~STREAM_SHARE_DENYALL ) | STREAM_SHARE_DENYWRITE );
    SvFileStream* pFileStrm = new SvFileStream( rName, nMode );
    // SvStream "Feature" Write Open auch erfolgreich, wenns nicht klappt
    sal_Bool bAccessDenied = sal_False;
    if( ( nMode & STREAM_WRITE ) && !pFileStrm->IsWritable() )
    {
        pFileStrm->Close();
        bAccessDenied = sal_True;
    }
    SetStrm( pFileStrm, sal_True );
    if( pFileStrm->IsOpen() )
    {
        sal_uLong nFileSize = pStrm->Seek( STREAM_SEEK_TO_END );
        nPages = lcl_GetPageCount( nFileSize, nPageSize );
        pStrm->Seek( 0L );
    }
    else
        nPages = 0;
    bFile = sal_True;
    SetError( bAccessDenied ? ERRCODE_IO_ACCESSDENIED : pStrm->GetError() );
    return Good();
}

void StgCache::Close()
{
    if( bFile )
    {
        ((SvFileStream*) pStrm)->Close();
        SetError( pStrm->GetError() );
    }
}

// low level I/O

sal_Bool StgCache::Read( sal_Int32 nPage, void* pBuf, sal_Int32 nPg )
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
            sal_uLong nPos = Page2Pos( nPage );
            sal_Int32 nPg2 = ( ( nPage + nPg ) > nPages ) ? nPages - nPage : nPg;
            sal_uLong nBytes = nPg2 * nPageSize;
            // fixed address and size for the header
            if( nPage == -1 )
            {
                nPos = 0L, nBytes = 512;
                nPg2 = nPg;
            }
            if( pStrm->Tell() != nPos )
            {
                if( pStrm->Seek( nPos ) != nPos ) {
    #ifdef CHECK_DIRTY
                    ErrorBox( NULL, WB_OK, String("SO2: Seek failed") ).Execute();
    #endif
                }
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

sal_Bool StgCache::Write( sal_Int32 nPage, void* pBuf, sal_Int32 nPg )
{
    if( Good() )
    {
        sal_uLong nPos = Page2Pos( nPage );
        sal_uLong nBytes = 0;
        if ( SAL_MAX_INT32 / nPg > nPageSize )
            nBytes = nPg * nPageSize;

        // fixed address and size for the header
        // nPageSize must be >= 512, otherwise the header can not be written here, we check it on import
        if( nPage == -1 )
            nPos = 0L, nBytes = 512;
        if( pStrm->Tell() != nPos )
        {
            if( pStrm->Seek( nPos ) != nPos ) {
#ifdef CHECK_DIRTY
                ErrorBox( NULL, WB_OK, String("SO2: Seek failed") ).Execute();
#endif
            }
        }
        sal_uLong nRes = pStrm->Write( pBuf, nBytes );
        if( nRes != nBytes )
            SetError( SVSTREAM_WRITE_ERROR );
        else
            SetError( pStrm->GetError() );
#ifdef READ_AFTER_WRITE
        sal_uInt8 cBuf[ 512 ];
        pStrm->Flush();
        pStrm->Seek( nPos );
        sal_Bool bRes = ( pStrm->Read( cBuf, 512 ) == 512 );
        if( bRes )
            bRes = !memcmp( cBuf, pBuf, 512 );
        if( !bRes )
        {
            ErrorBox( NULL, WB_OK, String("SO2: Read after Write failed") ).Execute();
            pStrm->SetError( SVSTREAM_WRITE_ERROR );
        }
#endif
    }
    return Good();
}

// set the file size in pages

sal_Bool StgCache::SetSize( sal_Int32 n )
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
