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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGCACHE_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGCACHE_HXX

#include <osl/endian.h>
#include <rtl/ref.hxx>
#include <tools/solar.h>
#include <tools/stream.hxx>
#include "stgelem.hxx"
#include <salhelper/simplereferenceobject.hxx>

#include <memory>
#include <unordered_map>
#include <functional>

class UCBStorageStream;
class StgPage;
class StgDirEntry;
class StorageBase;

class StgCache
{
    typedef std::unordered_map
    <
        sal_Int32, rtl::Reference< StgPage >
    > IndexToStgPage;

    typedef std::vector< rtl::Reference< StgPage > > LRUList;

    ErrCode m_nError;                         // error code
    sal_Int32 m_nPages;                       // size of data area in pages
    sal_uInt16 m_nRef;                        // reference count
    IndexToStgPage maDirtyPages;            // hash of all dirty pages
    int     m_nReplaceIdx;                    // index into maLRUPages to replace next
    LRUList maLRUPages;                     // list of last few non-dirty pages.
    short m_nPageSize;                        // page size of the file
    UCBStorageStream* m_pStorageStream;       // holds reference to UCB storage stream

    void Erase( const rtl::Reference< StgPage >& ); // delete a cache element
    rtl::Reference< StgPage > Create( sal_Int32  ); // create a cached page
    SvStream* m_pStrm;                        // physical stream
    bool  m_bMyStream;                        // true: delete stream in dtor
protected:
    bool  m_bFile;                            // true: file stream
    sal_Int32 Page2Pos( sal_Int32 ) const;    // page address --> file position
public:
    StgCache();
    ~StgCache();
    void  IncRef()                          { m_nRef++;           }
    sal_uInt16 DecRef()                     { return --m_nRef;    }
    void  SetPhysPageSize( short );
    sal_Int32 GetPhysPages() const          { return m_nPages;    }
    short GetPhysPageSize() const           { return m_nPageSize; }
    SvStream* GetStrm()                     { return m_pStrm;     }
    void  SetStrm( SvStream*, bool );
    void  SetStrm( UCBStorageStream* );
    bool  Good() const                      { return m_nError == ERRCODE_NONE; }
    ErrCode const & GetError() const        { return m_nError;    }
    void  MoveError( StorageBase const & );
    void  SetError( ErrCode );
    void  ResetError();
    bool  Open( const OUString& rName, StreamMode );
    void  Close();
    bool  Read( sal_Int32 nPage, void* pBuf );
    bool  Write( sal_Int32 nPage, void const * pBuf );

    // two routines for accessing FAT pages
    // Assume that the data is a FAT page and get/put FAT data.
    void  SetToPage   ( const rtl::Reference< StgPage >& rPage, short nOff, sal_Int32 nVal );
    static inline sal_Int32 GetFromPage ( const rtl::Reference< StgPage >& rPage, short nOff );
    void  SetDirty    ( const rtl::Reference< StgPage > &rPage );
    bool  SetSize( sal_Int32 nPages );
    rtl::Reference< StgPage > Find( sal_Int32 );                 // find a cached page
    rtl::Reference< StgPage > Get( sal_Int32, bool );            // get a cached page
    rtl::Reference< StgPage > Copy( sal_Int32, sal_Int32=STG_FREE ); // copy a page
    bool Commit(); // flush all pages
    void Clear();                           // clear the cache
};

class StgPage : public salhelper::SimpleReferenceObject
{
    const sal_Int32 mnPage;                // page index
    std::unique_ptr<sal_uInt8[]>
                    mpData;                // nSize bytes
    short           mnSize;                // size of this page
             StgPage( short nData, sal_Int32 nPage );
    virtual ~StgPage() override;
public:
             StgPage(const StgPage&) = delete;
    StgPage& operator=(const StgPage&) = delete;
    static rtl::Reference< StgPage > Create( short nData, sal_Int32 nPage );

    sal_Int32 GetPage() const { return mnPage; }
    void*     GetData()  { return mpData.get(); }
    short     GetSize() const { return mnSize; }

public:
    static bool IsPageGreater( const StgPage *pA, const StgPage *pB );
};

inline sal_Int32 StgCache::GetFromPage ( const rtl::Reference< StgPage >& rPage, short nOff )
{
    if( ( nOff >= static_cast<short>( rPage->GetSize() / sizeof( sal_Int32 ) ) ) || nOff < 0 )
        return -1;
    sal_Int32 n = static_cast<sal_Int32*>(rPage->GetData())[ nOff ];
#ifdef OSL_BIGENDIAN
    return OSL_SWAPDWORD(n);
#else
    return n;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
