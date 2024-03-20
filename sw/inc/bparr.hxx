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

#pragma once

#include <assert.h>

#include "swdllapi.h"
#include <array>
#include <memory>

struct BlockInfo;
class BigPtrArray;

class BigPtrEntry
{
    friend class BigPtrArray;
    BlockInfo*  m_pBlock;
    sal_uInt16  m_nOffset;
public:
    BigPtrEntry() : m_pBlock(nullptr), m_nOffset(0) {}
    BigPtrEntry(BigPtrEntry const &) = default;
    virtual ~BigPtrEntry() = default;
    BigPtrEntry & operator =(BigPtrEntry const &) = default;

    inline sal_Int32 GetPos() const;
    inline BigPtrArray& GetArray() const;
};

// 1000 entries per Block = a bit less than 4K
#define MAXENTRY 1000

// number of entries that may remain free during compression
// this value is for the worst case; because we defined MAXBLOCK with ca 25%
// overhead, 80% = 800 entries are enough
// if complete compression is desired, 100 has to be specified
#define COMPRESSLVL 80

struct BlockInfo final
{
    BigPtrArray* pBigArr;              ///< in this array the block is located
    sal_Int32    nStart, nEnd;         ///< start- and end index
    sal_uInt16   nElem;                ///< number of elements
    std::array<BigPtrEntry*, MAXENTRY>
                 mvData;               ///< data block
};

class BigPtrArray
{
protected:
    std::unique_ptr<BlockInfo*[]>
                    m_ppInf;              ///< block info
    sal_Int32       m_nSize;              ///< number of elements
    sal_uInt16      m_nMaxBlock;          ///< current max. number of blocks
    sal_uInt16      m_nBlock;             ///< number of blocks
    mutable
        sal_uInt16  m_nCur;               ///< last used block

    sal_uInt16  Index2Block( sal_Int32 ) const; ///< block search
    BlockInfo*  InsBlock( sal_uInt16 );         ///< insert block
    void        BlockDel( sal_uInt16 );         ///< some blocks were deleted
    void        UpdIndex( sal_uInt16 );         ///< recalculate indices

    // fill all blocks
    sal_uInt16 Compress();

public:
    BigPtrArray();
    ~BigPtrArray();

    sal_Int32 Count() const { return m_nSize; }

    void Insert( BigPtrEntry* p, sal_Int32 pos );
    void Remove( sal_Int32 pos, sal_Int32 n = 1 );
    void Move( sal_Int32 from, sal_Int32 to );
    void Replace( sal_Int32 pos, BigPtrEntry* p);

    /** Speed up the complicated removal logic in SwNodes::RemoveNode.
        Returns the entry before pNotTheOne.
    */
    BigPtrEntry* ReplaceTheOneAfter( BigPtrEntry* pNotTheOne, BigPtrEntry* pNewEntry);

    SW_DLLPUBLIC BigPtrEntry* operator[]( sal_Int32 ) const;
};

inline sal_Int32 BigPtrEntry::GetPos() const
{
    assert(this == m_pBlock->mvData[ m_nOffset ]); // element not in the block
    return m_pBlock->nStart + m_nOffset;
}

inline BigPtrArray& BigPtrEntry::GetArray() const
{
    return *m_pBlock->pBigArr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
