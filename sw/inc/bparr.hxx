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

#ifndef SW_BPARR_HXX
#define SW_BPARR_HXX

#include <assert.h>

#include <tools/solar.h>
#include <osl/diagnose.h>
#include <swdllapi.h>

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    sal_uInt16 nOffset;
public:
    virtual ~BigPtrEntry() {}
protected:
    BigPtrEntry() : pBlock(0), nOffset(0) {}

    inline sal_uLong GetPos() const;
    inline BigPtrArray& GetArray() const;
};
typedef BigPtrEntry* ElementPtr;


typedef bool (*FnForEach)( const ElementPtr&, void* pArgs );

// 1000 entries per Block = a bit less then 4K
#define MAXENTRY 1000


// number of entries that may remain free during compression
// this value is for the worst case; because we defined MAXBLOCK with ca 25%
// overhead, 80% = 800 entries are enough
// if complete compression is desired, 100 has to be specified
#define COMPRESSLVL 80

struct BlockInfo {                  // block info:
    BigPtrArray* pBigArr;           ///< in this array the block is located
    ElementPtr* pData;              ///< data block
    sal_uLong nStart, nEnd;         ///< start- and end index
    sal_uInt16 nElem;               ///< number of elements
};

class SW_DLLPUBLIC BigPtrArray
{
    BlockInfo** ppInf;              // block info
    sal_uLong       nSize;              ///< number of elements
    sal_uInt16      nMaxBlock;          ///< current max. number of blocks
    sal_uInt16      nBlock;             ///< number of blocks
    sal_uInt16      nCur;               ///< last block

    sal_uInt16      Index2Block( sal_uLong ) const; ///< block search
    BlockInfo*  InsBlock( sal_uInt16 );         ///< insert block
    void        BlockDel( sal_uInt16 );         ///< some blocks were deleted
    void        UpdIndex( sal_uInt16 );         ///< recalculate indices

protected:
    // fill all blocks
    // the short parameter specifies in percent, how full the blocks should be
    // made
    sal_uInt16 Compress( short = COMPRESSLVL );

public:
    BigPtrArray();
    ~BigPtrArray();

    sal_uLong Count() const { return nSize; }

    void Insert( const ElementPtr& r, sal_uLong pos );
    void Remove( sal_uLong pos, sal_uLong n = 1 );
    void Move( sal_uLong from, sal_uLong to );
    void Replace( sal_uLong pos, const ElementPtr& r);

    ElementPtr operator[]( sal_uLong ) const;
    void ForEach( FnForEach fn, void* pArgs = NULL )
    {
        ForEach( 0, nSize, fn, pArgs );
    }
    void ForEach( sal_uLong nStart, sal_uLong nEnd, FnForEach fn, void* pArgs = NULL );
};



inline sal_uLong BigPtrEntry::GetPos() const
{
    assert(this == pBlock->pData[ nOffset ]); // element not in the block
    return pBlock->nStart + nOffset;
}

inline BigPtrArray& BigPtrEntry::GetArray() const
{
    return *pBlock->pBigArr;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
