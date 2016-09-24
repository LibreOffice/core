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

#ifndef INCLUDED_SW_INC_BPARR_HXX
#define INCLUDED_SW_INC_BPARR_HXX

#include <assert.h>

#include <tools/solar.h>
#include <swdllapi.h>

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo* pBlock;
    sal_uInt16 nOffset;
public:
    BigPtrEntry() : pBlock(nullptr), nOffset(0) {}
    virtual ~BigPtrEntry() {}

    inline sal_uLong GetPos() const;
    inline BigPtrArray& GetArray() const;
};
typedef BigPtrEntry* ElementPtr;

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
protected:
    BlockInfo**     ppInf;              // block info
    sal_uLong       nSize;              ///< number of elements
    sal_uInt16      nMaxBlock;          ///< current max. number of blocks
    sal_uInt16      nBlock;             ///< number of blocks
    mutable
        sal_uInt16  nCur;               ///< last used block

    sal_uInt16  Index2Block( sal_uLong ) const; ///< block search
    BlockInfo*  InsBlock( sal_uInt16 );         ///< insert block
    void        BlockDel( sal_uInt16 );         ///< some blocks were deleted
    void        UpdIndex( sal_uInt16 );         ///< recalculate indices

    // fill all blocks
    sal_uInt16 Compress();

public:
    BigPtrArray();
    ~BigPtrArray();

    sal_uLong Count() const { return nSize; }

    void Insert( const ElementPtr& r, sal_uLong pos );
    void Remove( sal_uLong pos, sal_uLong n = 1 );
    void Move( sal_uLong from, sal_uLong to );
    void Replace( sal_uLong pos, const ElementPtr& r);

    ElementPtr operator[]( sal_uLong ) const;
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
