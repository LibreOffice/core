/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BPARR_HXX
#define _BPARR_HXX

#include <tools/solar.h>
#include <tools/debug.hxx>
#include <swdllapi.h>

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    USHORT nOffset;
public:
    virtual ~BigPtrEntry() {}
protected:
    BigPtrEntry() : pBlock(0), nOffset(0) {}

    inline ULONG GetPos() const;
    inline BigPtrArray& GetArray() const;
};
typedef BigPtrEntry* ElementPtr;


typedef BOOL (*FnForEach)( const ElementPtr&, void* pArgs );

// 1000 entries per Block = a bit less then 4K
#define MAXENTRY 1000


// number of entries that may remain free during compression
// this value is for the worst case; because we defined MAXBLOCK with ca 25%
// overhead, 80% = 800 entries are enough
// if complete compression is desired, 100 has to be specified
#define COMPRESSLVL 80

struct BlockInfo {                  // block info:
    BigPtrArray* pBigArr;           // in this array the block is located
    ElementPtr* pData;              // data block
    ULONG nStart, nEnd;             // start- and end index
    USHORT nElem;                   // number of elements
};

class SW_DLLPUBLIC BigPtrArray
{
    BlockInfo** ppInf;              // block info
    ULONG       nSize;              // number of elements
    USHORT      nMaxBlock;          // current max. number of blocks
    USHORT      nBlock;             // number of blocks
    USHORT      nCur;               // last block

    USHORT      Index2Block( ULONG ) const; // block search
    BlockInfo*  InsBlock( USHORT );         // insert block
    void        BlockDel( USHORT );         // some blocks were deleted
    void        UpdIndex( USHORT );         // recalculate indices

protected:
    // fill all blocks
    // the short parameter specifies in percent, how full the blocks should be
    // made
    // the return value specifies that something was done
    USHORT Compress( short = COMPRESSLVL );

public:
    BigPtrArray();
    ~BigPtrArray();

    ULONG Count() const { return nSize; }

    void Insert( const ElementPtr& r, ULONG pos );
    void Remove( ULONG pos, ULONG n = 1 );
    void Move( ULONG from, ULONG to );
    void Replace( ULONG pos, const ElementPtr& r);

    ElementPtr operator[]( ULONG ) const;
    void ForEach( FnForEach fn, void* pArgs = NULL )
    {
        ForEach( 0, nSize, fn, pArgs );
    }
    void ForEach( ULONG nStart, ULONG nEnd, FnForEach fn, void* pArgs = NULL );
};



inline ULONG BigPtrEntry::GetPos() const
{
    DBG_ASSERT( this == pBlock->pData[ nOffset ], "element not in the block" );
    return pBlock->nStart + nOffset;
}

inline BigPtrArray& BigPtrEntry::GetArray() const
{
    return *pBlock->pBigArr;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
