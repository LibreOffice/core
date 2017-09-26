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
#include <memory>

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
    StgIo& m_rIo;                         // I/O system
    std::unique_ptr<StgFAT> m_pFat;       // FAT stream for allocations
    StgDirEntry* m_pEntry;                // dir entry (for ownership)
    sal_Int32 m_nStart;                       // 1st data page
    sal_Int32 m_nSize;                        // stream size in bytes
    sal_Int32 m_nPos;                         // current byte position
    sal_Int32 m_nPage;                        // current logical page
    short m_nOffset;                      // offset into current page
    short m_nPageSize;                    // logical page size
    std::vector<sal_Int32> m_aPagesCache;
    void scanBuildPageChainCache(sal_Int32 *pOptionalCalcSize);
    bool  Copy( sal_Int32 nFrom, sal_Int32 nBytes );
    explicit StgStrm( StgIo& );
public:
    virtual ~StgStrm();
    StgIo&  GetIo()     { return m_rIo;    }
    sal_Int32   GetPos() const   { return m_nPos;   }
    sal_Int32   GetStart() const { return m_nStart; }
    sal_Int32   GetSize() const  { return m_nSize;  }
    sal_Int32   GetPage() const  { return m_nPage;  }
    sal_Int32   GetPages() const { return ( m_nSize + m_nPageSize - 1 ) / m_nPageSize;}
    short       GetOffset() const { return m_nOffset;}
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
    virtual bool Pos2Page( sal_Int32 nBytePos ) override;
    bool  SetPage( short, sal_Int32 );
public:
    explicit StgFATStrm(StgIo&, sal_Int32 nFatStrmSize);
    using StgStrm::GetPage;
    sal_Int32 GetPage( short, bool, sal_uInt16 *pnMasterAlloc = nullptr);
    virtual bool SetSize( sal_Int32 ) override;
};

// The stream has a size increment which normally is 1, but which can be
// set to any value is you want the size to be incremented by certain values.

class StgDataStrm : public StgStrm      // a physical data stream
{
    short m_nIncr;                        // size adjust increment
    void Init( sal_Int32 nBgn, sal_Int32 nLen );
public:
    StgDataStrm( StgIo&, sal_Int32 nBgn, sal_Int32 nLen=-1 );
    StgDataStrm( StgIo&, StgDirEntry& );
    void* GetPtr( sal_Int32 nPos, bool bDirty );
    void SetIncrement( short n ) { m_nIncr = n ; }
    virtual bool SetSize( sal_Int32 ) override;
    virtual sal_Int32 Read( void*, sal_Int32 ) override;
    virtual sal_Int32 Write( const void*, sal_Int32 ) override;
};

// The small stream class provides access to streams with a size < 4096 bytes.
// This stream is a StgStream containing small pages. The FAT for this stream
// is also a StgStream. The start of the FAT is in the header at DataRootPage,
// the stream itself is pointed to by the root entry (it holds start & size).

class StgSmallStrm : public StgStrm     // a logical data stream
{
    StgStrm* m_pData;                     // the data stream
    void Init( sal_Int32 nBgn, sal_Int32 nLen );
public:
    StgSmallStrm( StgIo&, sal_Int32 nBgn );
    StgSmallStrm( StgIo&, StgDirEntry& );
    virtual sal_Int32 Read( void*, sal_Int32 ) override;
    virtual sal_Int32 Write( const void*, sal_Int32 ) override;
    virtual bool IsSmallStrm() const override { return true; }
};

class StgTmpStrm : public SvMemoryStream
{
    OUString m_aName;
    SvFileStream* m_pStrm;
    using SvMemoryStream::GetData;
    virtual std::size_t GetData( void* pData, std::size_t nSize ) override;
    virtual std::size_t PutData( const void* pData, std::size_t nSize ) override;
    virtual sal_uInt64 SeekPos( sal_uInt64 nPos ) override;
    virtual void FlushData() override;

public:
    explicit StgTmpStrm( sal_uInt64=16 );
    virtual ~StgTmpStrm() override;
    bool Copy( StgTmpStrm& );
    virtual void SetSize( sal_uInt64 ) override;
    sal_uInt64 GetSize() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
