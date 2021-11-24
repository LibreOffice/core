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
#include <tools/solar.h>
#include <array>
#include <memory>
#include <svl/svldllapi.h>

class SfxPoolItem;

class SVL_DLLPUBLIC SortedBigPtrArray
{
    // 1000 entries per Block = a bit less than 4K
    static constexpr int MaxEntries = 1000;

    struct BlockInfo final
    {
        sal_Int32 nStart, nEnd; ///< start- and end index
        sal_uInt16 nElem; ///< number of elements
        std::array<SfxPoolItem*, MaxEntries> mvData; ///< data block
    };

    std::unique_ptr<BlockInfo* []> m_ppInf; ///< block info
    sal_Int32 m_nSize; ///< number of elements
    sal_uInt16 m_nMaxBlock; ///< current max. number of blocks
    sal_uInt16 m_nBlock; ///< number of blocks
    mutable sal_uInt16 m_nCur; ///< last used block

    sal_uInt16 Index2Block(sal_Int32) const; ///< block search
    BlockInfo* InsBlock(sal_uInt16); ///< insert block
    void BlockDel(sal_uInt16); ///< some blocks were deleted
    void UpdIndex(sal_uInt16); ///< recalculate indices

    // fill all blocks
    sal_uInt16 Compress();

    void Insert(SfxPoolItem* p, sal_Int32 pos);
    void Remove(sal_Int32 pos, sal_Int32 n = 1);
    void Remove2(sal_Int32 blockIndex, sal_Int32 offset, sal_Int32 n = 1);
    void Move(sal_Int32 from, sal_Int32 to);
    void Replace(sal_Int32 pos, SfxPoolItem* p);

public:
    SortedBigPtrArray();
    ~SortedBigPtrArray();

    struct SVL_DLLPUBLIC const_iterator
    {
        typedef sal_Int32 difference_type;
        typedef SfxPoolItem* value_type;
        typedef SfxPoolItem* pointer;
        typedef SfxPoolItem* reference;
        // TODO make this bidirectional or random access
        typedef std::forward_iterator_tag iterator_category;

        const SortedBigPtrArray* mpArray;
        sal_Int32 mnBlockIndex;
        sal_Int32 mnIndexOfEntryInBlock;

        bool operator==(const const_iterator& other) const
        {
            return mpArray == other.mpArray && mnBlockIndex == other.mnBlockIndex
                   && mnIndexOfEntryInBlock == other.mnIndexOfEntryInBlock;
        }
        bool operator!=(const const_iterator& other) const { return !operator==(other); }
        const_iterator& operator++();
        SfxPoolItem*& operator*() const
        {
            return mpArray->m_ppInf[mnBlockIndex]->mvData[mnIndexOfEntryInBlock];
        }
        SfxPoolItem*& operator->() const
        {
            return mpArray->m_ppInf[mnBlockIndex]->mvData[mnIndexOfEntryInBlock];
        }
    };

    SfxPoolItem* front() const;
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(SfxPoolItem*) const;
    void erase(const_iterator);
    bool insert(SfxPoolItem*);
    sal_Int32 size() const { return m_nSize; }
    bool empty() const { return m_nSize == 0; }
    void clear();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
