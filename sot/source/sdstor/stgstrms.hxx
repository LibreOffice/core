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

#ifndef INCLUDED_SOT_SOURCE_SDSTOR_STGSTRMS_HXX
#define INCLUDED_SOT_SOURCE_SDSTOR_STGSTRMS_HXX

#include <tools/stream.hxx>
#include <rtl/ref.hxx>
#include <vector>

class StgIo;
class StgStrm;
class StgPage;
class StgDirEntry;

// The FAT class performs FAT operations on an underlying storage stream.
// This stream is either the physical FAT stream (bPhys == true ) or a normal
// storage stream, which then holds the FAT for small data allocations.

class StgFAT
{                                       // FAT allocator
    StgStrm& m_rStrm;                     // underlying stream
    sal_Int32 m_nMaxPage;                     // highest page allocated so far
    short m_nPageSize;                    // physical page size
    short m_nEntries;                     // FAT entries per page
    short m_nOffset;                      // current offset within page
    sal_Int32 m_nLimit;                       // search limit recommendation
    bool  m_bPhys;                        // true: physical FAT
    rtl::Reference< StgPage > GetPhysPage( sal_Int32 nPage );
    bool  MakeChain( sal_Int32 nStart, sal_Int32 nPages );
    bool  InitNew( sal_Int32 nPage1 );
public:
    StgFAT( StgStrm& rStrm, bool bMark );
    sal_Int32 FindBlock( sal_Int32& nPages );
    sal_Int32 GetNextPage( sal_Int32 nPg );
    sal_Int32 AllocPages( sal_Int32 nStart, sal_Int32 nPages );
    bool  FreePages( sal_Int32 nStart, bool bAll );
    sal_Int32 GetMaxPage() { return m_nMaxPage; }
    void  SetLimit( sal_Int32 n ) { m_nLimit = n; }
};

// The base stream class provides basic functionality for seeking
// and accessing the data on a physical basis. It uses the built-in
// FAT class for the page allocations.

class StgStrm {                         // base class for all streams
protected:
    StgIo& rIo;                         // I/O system
    StgFAT* pFat;                       // FAT stream for allocations
    StgDirEntry* pEntry;                // dir entry (for ownership)
    sal_Int32 nStart;                       // 1st data page
    sal_Int32 nSize;                        // stream size in bytes
    sal_Int32 nPos;                         // current byte position
    sal_Int32 nPage;                        // current logical page
    short nOffset;                      // offset into current page
    short nPageSize;                    // logical page size
    std::vector<sal_Int32> m_aPagesCache;
    void scanBuildPageChainCache(sal_Int32 *pOptionalCalcSize = NULL);
    bool  Copy( sal_Int32 nFrom, sal_Int32 nBytes );
    explicit StgStrm( StgIo& );
public:
    virtual ~StgStrm();
    StgIo&  GetIo()     { return rIo;    }
    sal_Int32   GetPos() const   { return nPos;   }
    sal_Int32   GetStart() const { return nStart; }
    sal_Int32   GetSize() const  { return nSize;  }
    sal_Int32   GetPage() const  { return nPage;  }
    sal_Int32   GetPages() const { return ( nSize + nPageSize - 1 ) / nPageSize;}
    short       GetOffset() const { return nOffset;}
    void        SetEntry( StgDirEntry& );
    virtual bool SetSize( sal_Int32 );
    virtual bool Pos2Page( sal_Int32 nBytePos );
    virtual sal_Int32 Read( void*, sal_Int32 )        { return 0; }
    virtual sal_Int32 Write( const void*, sal_Int32 ) { return 0; }
    virtual bool IsSmallStrm() const { return false; }
};

// The FAT stream class provides physical access to the master FAT.
// Since this access is implemented as a StgStrm, we can use the
// FAT allocator.

class StgFATStrm : public StgStrm {     // the master FAT stream
    virtual bool Pos2Page( sal_Int32 nBytePos ) SAL_OVERRIDE;
    bool  SetPage( short, sal_Int32 );
public:
    explicit StgFATStrm( StgIo& );
    virtual ~StgFATStrm() {}
    using StgStrm::GetPage;
    sal_Int32 GetPage( short, bool, sal_uInt16 *pnMasterAlloc = 0);
    virtual bool SetSize( sal_Int32 ) SAL_OVERRIDE;
};

// The stream has a size increment which normally is 1, but which can be
// set to any value is you want the size to be incremented by certain values.

class StgDataStrm : public StgStrm      // a physical data stream
{
    short nIncr;                        // size adjust increment
    void Init( sal_Int32 nBgn, sal_Int32 nLen );
public:
    StgDataStrm( StgIo&, sal_Int32 nBgn, sal_Int32 nLen=-1 );
    StgDataStrm( StgIo&, StgDirEntry& );
    void* GetPtr( sal_Int32 nPos, bool bForce, bool bDirty );
    void SetIncrement( short n ) { nIncr = n ; }
    virtual bool SetSize( sal_Int32 ) SAL_OVERRIDE;
    virtual sal_Int32 Read( void*, sal_Int32 ) SAL_OVERRIDE;
    virtual sal_Int32 Write( const void*, sal_Int32 ) SAL_OVERRIDE;
};

// The small stream class provides access to streams with a size < 4096 bytes.
// This stream is a StgStream containing small pages. The FAT for this stream
// is also a StgStream. The start of the FAT is in the header at DataRootPage,
// the stream itself is pointed to by the root entry (it holds start & size).

class StgSmallStrm : public StgStrm     // a logical data stream
{
    StgStrm* pData;                     // the data stream
    void Init( sal_Int32 nBgn, sal_Int32 nLen );
public:
    StgSmallStrm( StgIo&, sal_Int32 nBgn, sal_Int32 nLen );
    StgSmallStrm( StgIo&, StgDirEntry& );
    virtual sal_Int32 Read( void*, sal_Int32 ) SAL_OVERRIDE;
    virtual sal_Int32 Write( const void*, sal_Int32 ) SAL_OVERRIDE;
    virtual bool IsSmallStrm() const SAL_OVERRIDE { return true; }
};

class StgTmpStrm : public SvMemoryStream
{
    OUString m_aName;
    SvFileStream* m_pStrm;
    using SvMemoryStream::GetData;
    virtual sal_uLong GetData( void* pData, sal_uLong nSize ) SAL_OVERRIDE;
    virtual sal_uLong PutData( const void* pData, sal_uLong nSize ) SAL_OVERRIDE;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) SAL_OVERRIDE;
    virtual void FlushData() SAL_OVERRIDE;

public:
    explicit StgTmpStrm( sal_uLong=16 );
    virtual ~StgTmpStrm();
    bool Copy( StgTmpStrm& );
    virtual void SetSize( sal_uInt64 ) SAL_OVERRIDE;
    sal_uLong GetSize() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
