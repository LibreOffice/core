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

#ifndef _STGCACHE_HXX
#define _STGCACHE_HXX

#include <osl/endian.h>
#include <rtl/ref.hxx>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include <stgelem.hxx>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

class UCBStorageStream;
class StgPage;
class StgDirEntry;
class StorageBase;

class StgCache {
    typedef boost::unordered_map
    <
        sal_Int32, rtl::Reference< StgPage >,
        boost::hash< sal_Int32 >, std::equal_to< sal_Int32 >
    > IndexToStgPage;

    typedef std::vector< rtl::Reference< StgPage > > LRUList;

    sal_uLong nError;                       // error code
    sal_Int32 nPages;                       // size of data area in pages
    sal_uInt16 nRef;                        // reference count
    IndexToStgPage maDirtyPages;            // hash of all dirty pages
    int     nReplaceIdx;                    // index into maLRUPages to replace next
    LRUList maLRUPages;                     // list of last few non-dirty pages.
    short nPageSize;                        // page size of the file
    UCBStorageStream* pStorageStream;       // holds reference to UCB storage stream

    void Erase( const rtl::Reference< StgPage >& ); // delete a cache element
    rtl::Reference< StgPage > Create( sal_Int32  ); // create a cached page
protected:
    SvStream* pStrm;                        // physical stream
    sal_Bool  bMyStream;                    // sal_True: delete stream in dtor
    sal_Bool  bFile;                        // sal_True: file stream
    sal_Int32 Page2Pos( sal_Int32 );        // page address --> file position
public:
    StgCache();
    ~StgCache();
    void  IncRef()                          { nRef++;           }
    sal_uInt16 DecRef()                     { return --nRef;    }
    void  SetPhysPageSize( short );
    sal_Int32 GetPhysPages()                { return nPages;    }
    short GetPhysPageSize()                 { return nPageSize; }
    SvStream* GetStrm()                     { return pStrm;     }
    void  SetStrm( SvStream*, sal_Bool );
    void  SetStrm( UCBStorageStream* );
    sal_Bool  IsWritable()                  { return ( pStrm && pStrm->IsWritable() ); }
    sal_Bool  Good()                        { return sal_Bool( nError == SVSTREAM_OK ); }
    sal_Bool  Bad()                         { return sal_Bool( nError != SVSTREAM_OK ); }
    sal_uLong GetError()                    { return nError;    }
    void  MoveError( StorageBase& );
    void  SetError( sal_uLong );
    void  ResetError();
    sal_Bool  Open( const String& rName, StreamMode );
    void  Close();
    sal_Bool  Read        ( sal_Int32 nPage, void* pBuf, sal_Int32 nPages );
    sal_Bool  Write       ( sal_Int32 nPage, void* pBuf, sal_Int32 nPages );

    // two routines for accessing FAT pages
    // Assume that the data is a FAT page and get/put FAT data.
    void             SetToPage   ( const rtl::Reference< StgPage > xPage, short nOff, sal_Int32 nVal );
    inline sal_Int32 GetFromPage ( const rtl::Reference< StgPage > xPage, short nOff );
    void      SetDirty    ( const rtl::Reference< StgPage > &xPage );
    sal_Bool  SetSize( sal_Int32 nPages );
    rtl::Reference< StgPage > Find( sal_Int32 );                 // find a cached page
    rtl::Reference< StgPage > Get( sal_Int32, sal_Bool );            // get a cached page
    rtl::Reference< StgPage > Copy( sal_Int32, sal_Int32=STG_FREE ); // copy a page
    sal_Bool Commit(); // flush all pages
    void Clear();                           // clear the cache
};

class StgPage : public rtl::IReference, private boost::noncopyable {
    sal_uInt32      mnRefCount;
    const sal_Int32 mnPage;                // page index
    sal_uInt8*      mpData;                // nSize bytes
    short           mnSize;                // size of this page
             StgPage( short nData, sal_Int32 nPage );
    virtual ~StgPage();
public:
    static rtl::Reference< StgPage > Create( short nData, sal_Int32 nPage );

    sal_Int32 GetPage()  { return mnPage; }
    void*     GetData()  { return mpData; }
    short     GetSize()  { return mnSize; }

public:
    virtual oslInterlockedCount SAL_CALL acquire()
    {
        return ++mnRefCount;
    }
    virtual oslInterlockedCount SAL_CALL release()
    {
        if ( --mnRefCount == 0)
        {
            delete this;
            return 0;
        }
        return mnRefCount;
    }
    static bool IsPageGreater( const StgPage *pA, const StgPage *pB );
};

inline sal_Int32 StgCache::GetFromPage ( const rtl::Reference< StgPage > xPage, short nOff )
{
    if( ( nOff >= (short) ( xPage->GetSize() / sizeof( sal_Int32 ) ) ) || nOff < 0 )
        return -1;
    sal_Int32 n = ((sal_Int32*) xPage->GetData() )[ nOff ];
#ifdef OSL_BIGENDIAN
    return OSL_SWAPDWORD(n);
#else
    return n;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
