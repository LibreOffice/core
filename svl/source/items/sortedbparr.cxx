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

#include <svl/sortedbparr.hxx>
#include <tools/long.hxx>
#include <limits.h>
#include <string.h>

/** Resize block management by this constant.
    As a result there are approx. 20 * MAXENTRY == 20000 entries available */
const sal_uInt16 nBlockGrowSize = 20;

#if OSL_DEBUG_LEVEL > 2
#define CHECKIDX(p, n, i, c) CheckIdx(p, n, i, c);
void CheckIdx(BlockInfo** ppInf, sal_uInt16 nBlock, sal_Int32 nSize, sal_uInt16 nCur)
{
    assert(!nSize || nCur < nBlock); // BigPtrArray: CurIndex invalid

    sal_Int32 nIdx = 0;
    for (sal_uInt16 nCnt = 0; nCnt < nBlock; ++nCnt, ++ppInf)
    {
        nIdx += (*ppInf)->nElem;
        // Array with holes is not allowed
        assert(!nCnt || (*(ppInf - 1))->nEnd + 1 == (*ppInf)->nStart);
    }
    assert(nIdx == nSize); // invalid count in nSize
}
#else
#define CHECKIDX(p, n, i, c)
#endif

SortedBigPtrArray::SortedBigPtrArray()
{
    m_nCur = 0;
    m_nSize = 0;
    m_vBlocks.reserve(nBlockGrowSize);
}

SortedBigPtrArray::~SortedBigPtrArray() {}

// Also moving is done simply here. Optimization is useless because of the
// division of this field into multiple parts.
void SortedBigPtrArray::Move(sal_Int32 from, sal_Int32 to)
{
    if (from != to)
    {
        sal_uInt16 cur = Index2Block(from);
        BlockInfo* p = m_vBlocks[cur].get();
        SfxPoolItem* pElem = p->mvData[from - p->nStart];
        Insert(pElem, to); // insert first, then delete!
        Remove((to < from) ? (from + 1) : from);
    }
}
/*
SfxPoolItem* SortedBigPtrArray::operator[]( sal_Int32 idx ) const
{
    assert(idx < m_nSize); // operator[]: Index out of bounds
    m_nCur = Index2Block( idx );
    BlockInfo* p = m_vBlocks[ m_nCur ];
    return p->mvData[ idx - p->nStart ];
}
*/
/** Search a block at a given position */
sal_uInt16 SortedBigPtrArray::Index2Block(sal_Int32 pos) const
{
    // last used block?
    BlockInfo* p = m_vBlocks[m_nCur].get();
    if (p->nStart <= pos && p->nEnd >= pos)
        return m_nCur;
    // Index = 0?
    if (!pos)
        return 0;

    // following one?
    if (m_nCur < (m_vBlocks.size() - 1))
    {
        p = m_vBlocks[m_nCur + 1].get();
        if (p->nStart <= pos && p->nEnd >= pos)
            return m_nCur + 1;
    }
    // previous one?
    else if (pos < p->nStart && m_nCur > 0)
    {
        p = m_vBlocks[m_nCur - 1].get();
        if (p->nStart <= pos && p->nEnd >= pos)
            return m_nCur - 1;
    }

    // binary search: always successful
    sal_uInt16 lower = 0, upper = m_vBlocks.size() - 1;
    sal_uInt16 cur = 0;
    for (;;)
    {
        sal_uInt16 n = lower + (upper - lower) / 2;
        cur = (n == cur) ? n + 1 : n;
        p = m_vBlocks[cur].get();
        if (p->nStart <= pos && p->nEnd >= pos)
            return cur;

        if (p->nStart > pos)
            upper = cur;
        else
            lower = cur;
    }
}

/** Update all index areas

    @param pos last correct block (starting point)
*/
void SortedBigPtrArray::UpdIndex(sal_uInt16 pos)
{
    std::unique_ptr<BlockInfo>* pp = m_vBlocks.data() + pos;
    sal_Int32 idx = (*pp)->nEnd + 1;
    while (++pos < m_vBlocks.size())
    {
        ++pp;
        BlockInfo* p = pp->get();
        p->nStart = idx;
        idx += p->nElem;
        p->nEnd = idx - 1;
    }
}

/** Create and insert new block

    Existing blocks will be moved rearward.

    @param pos Position at which the new block should be created.
*/
SortedBigPtrArray::BlockInfo* SortedBigPtrArray::InsBlock(sal_uInt16 pos)
{
    BlockInfo* p = new BlockInfo{};
    m_vBlocks.insert(m_vBlocks.begin() + pos, std::unique_ptr<BlockInfo>(p));

    if (pos)
        p->nStart = p->nEnd = m_vBlocks[pos - 1]->nEnd + 1;
    else
        p->nStart = p->nEnd = 0;

    p->nEnd--; // no elements
    p->nElem = 0;
    return p;
}

void SortedBigPtrArray::Insert(SfxPoolItem* pElem, sal_Int32 pos)
{
    CHECKIDX(m_vBlocks.get(), m_vBlocks.size(), m_nSize, m_nCur);

    BlockInfo* p;
    sal_uInt16 cur;
    if (!m_nSize)
    {
        // special case: insert first element
        cur = 0;
        p = InsBlock(cur);
    }
    else if (pos == m_nSize)
    {
        // special case: insert at end
        cur = m_vBlocks.size() - 1;
        p = m_vBlocks[cur].get();
        if (p->nElem == MaxEntries)
            // the last block is full, create a new one
            p = InsBlock(++cur);
    }
    else
    {
        // standard case:
        cur = Index2Block(pos);
        p = m_vBlocks[cur].get();
    }

    if (p->nElem == MaxEntries)
    {
        // does the last entry fit into the next block?
        BlockInfo* q;
        if (cur < (m_vBlocks.size() - 1) && m_vBlocks[cur + 1]->nElem < MaxEntries)
        {
            q = m_vBlocks[cur + 1].get();
            if (q->nElem)
            {
                int nCount = q->nElem;
                auto pFrom = q->mvData.begin() + nCount;
                auto pTo = pFrom + 1;
                while (nCount--)
                {
                    *--pTo = *--pFrom;
                    //                    ++((*pTo)->m_nOffset);
                }
            }
            q->nStart--;
            q->nEnd--;
        }
        else
        {
            // If it does not fit, then insert a new block.
            q = InsBlock(cur + 1);
        }

        // entry does not fit anymore - clear space
        SfxPoolItem* pLast = p->mvData[MaxEntries - 1];
        //        pLast->m_nOffset = 0;
        //        pLast->m_pBlock = q;

        q->mvData[0] = pLast;
        q->nElem++;
        q->nEnd++;

        p->nEnd--;
        p->nElem--;
    }
    // now we have free space - insert
    pos -= p->nStart;
    assert(pos < MaxEntries);
    if (pos != p->nElem)
    {
        int nCount = p->nElem - sal_uInt16(pos);
        auto pFrom = p->mvData.begin() + p->nElem;
        auto pTo = pFrom + 1;
        while (nCount--)
        {
            *--pTo = *--pFrom;
            //            ++( *pTo )->m_nOffset;
        }
    }
    // insert element and update indices
    //    pElem->m_nOffset = sal_uInt16(pos);
    //    pElem->m_pBlock = p;
    p->mvData[pos] = pElem;
    p->nEnd++;
    p->nElem++;
    m_nSize++;
    if (cur != (m_vBlocks.size() - 1))
        UpdIndex(cur);
    m_nCur = cur;

    CHECKIDX(m_vBlocks.get(), m_vBlocks.size(), m_nSize, m_nCur);
}

void SortedBigPtrArray::Remove(sal_Int32 pos, sal_Int32 n)
{
    sal_uInt16 cur = Index2Block(pos); // current block number
    Remove2(cur, pos - m_vBlocks[cur]->nStart, n);
}

void SortedBigPtrArray::Remove2(sal_Int32 blockIndex, sal_Int32 nIndexOfEntryInBlock, sal_Int32 n)
{
    CHECKIDX(m_vBlocks.get(), m_vBlocks.size(), m_nSize, m_nCur);

    sal_uInt16 nBlkdel = 0; // deleted blocks
    sal_uInt16 cur = blockIndex; // current block number
    sal_uInt16 nBlk1 = cur; // 1st treated block
    sal_uInt16 nBlk1del = USHRT_MAX; // 1st deleted block
    BlockInfo* p = m_vBlocks[cur].get();
    sal_Int32 pos = nIndexOfEntryInBlock;

    sal_Int32 nElem = n;
    while (nElem)
    {
        sal_uInt16 nel = p->nElem - sal_uInt16(pos);
        if (sal_Int32(nel) > nElem)
            nel = sal_uInt16(nElem);
        // move elements if needed
        if ((pos + nel) < sal_Int32(p->nElem))
        {
            auto pTo = p->mvData.begin() + pos;
            auto pFrom = pTo + nel;
            int nCount = p->nElem - nel - sal_uInt16(pos);
            while (nCount--)
            {
                *pTo = *pFrom++;
                //                (*pTo)->m_nOffset = (*pTo)->m_nOffset - nel;
                ++pTo;
            }
        }
        p->nEnd -= nel;
        p->nElem = p->nElem - nel;
        // possibly delete block completely
        if (!p->nElem)
        {
            nBlkdel++;
            if (USHRT_MAX == nBlk1del)
                nBlk1del = cur;
        }
        nElem -= nel;
        if (!nElem)
            break;
        p = m_vBlocks[++cur].get();
        pos = 0;
    }

    // update table if blocks were removed
    if (nBlkdel)
    {
        m_vBlocks.erase(m_vBlocks.begin() + nBlk1del, m_vBlocks.begin() + nBlk1del + nBlkdel);

        if ((nBlk1del + nBlkdel) < m_vBlocks.size())
        {
            // UpdateIdx updates the successor thus start before first elem
            if (!nBlk1)
            {
                p = m_vBlocks[0].get();
                p->nStart = 0;
                p->nEnd = p->nElem - 1;
            }
            else
            {
                --nBlk1;
            }
        }
    }

    m_nSize -= n;
    if (nBlk1 != (m_vBlocks.size() - 1) && m_nSize)
        UpdIndex(nBlk1);
    m_nCur = nBlk1;

    CHECKIDX(m_vBlocks.get(), m_vBlocks.size(), m_nSize, m_nCur);
}

void SortedBigPtrArray::Replace(sal_Int32 idx, SfxPoolItem* pElem)
{
    assert(idx < m_nSize); // Index out of bounds
    m_nCur = Index2Block(idx);
    BlockInfo* p = m_vBlocks[m_nCur].get();
    //    pElem->m_nOffset = sal_uInt16(idx - p->nStart);
    //    pElem->m_pBlock = p;
    p->mvData[idx - p->nStart] = pElem;
}

SfxPoolItem* SortedBigPtrArray::front() const { return *begin(); }

SortedBigPtrArray::const_iterator SortedBigPtrArray::begin() const
{
    if (!m_nSize)
        return end();
    return { this, 0, 0 };
}

SortedBigPtrArray::const_iterator SortedBigPtrArray::end() const { return { this, -1, -1 }; }

SortedBigPtrArray::const_iterator& SortedBigPtrArray::const_iterator::operator++()
{
    assert(this->operator->()->Which());
    ++mnIndexOfEntryInBlock;
    if (mnIndexOfEntryInBlock >= mpArray->m_vBlocks[mnBlockIndex]->nElem)
    {
        ++mnBlockIndex;
        if (mnBlockIndex >= static_cast<sal_Int32>(mpArray->m_vBlocks.size()))
        {
            mnBlockIndex = -1;
            mnIndexOfEntryInBlock = -1;
            return *this;
        }
        else
            mnIndexOfEntryInBlock = 0;
    }
    assert(this->operator->()->Which());
    return *this;
}

SortedBigPtrArray::const_iterator SortedBigPtrArray::find(SfxPoolItem* pItem) const
{
    return std::lower_bound(begin(), end(), pItem);
}

void SortedBigPtrArray::erase(SortedBigPtrArray::const_iterator it)
{
    Remove2(it.mnBlockIndex, it.mnIndexOfEntryInBlock);
}

bool SortedBigPtrArray::insert(SfxPoolItem* pItem)
{
    const_iterator const it = std::lower_bound(begin(), end(), pItem);
    if (it == end())
    {
        Insert(pItem, m_nSize);
        return true;
    }
    else if (pItem < *it)
    {
        sal_Int32 pos1 = m_vBlocks[it.mnBlockIndex]->nStart + it.mnIndexOfEntryInBlock;
        sal_Int32 pos2 = m_vBlocks[0]->nStart;
        Insert(pItem, pos1 - pos2);
        return true;
    }
    return false;
}

void SortedBigPtrArray::clear()
{
    if (m_nSize)
        Remove(0, size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
