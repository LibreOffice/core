/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdlib>
#include <mdds/multi_type_vector_types.hpp>

namespace sc {

template<typename SizeT, typename Ret = bool>
struct FuncElseNoOp
{
    Ret operator() (mdds::mtv::element_t, SizeT, SizeT) const
    {
        return Ret();
    }
};

/**
 * Generic algorithm to parse blocks of multi_type_vector either partially
 * or fully.
 */
template<typename StoreT, typename Func>
typename StoreT::const_iterator
ParseBlock(
    const typename StoreT::const_iterator& itPos, const StoreT& rStore, Func& rFunc,
    typename StoreT::size_type nStart, typename StoreT::size_type nEnd)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        rFunc(*it, nOffset, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
}

/**
 * Non-const variant of the above function. TODO: Find a way to merge these
 * two in an elegant way.
 */
template<typename StoreT, typename Func>
typename StoreT::iterator
ProcessBlock(const typename StoreT::iterator& itPos, StoreT& rStore, Func& rFunc, typename StoreT::size_type nStart, typename StoreT::size_type nEnd)
{
    typedef std::pair<typename StoreT::iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nCurRow = nStart;

    for (; it != rStore.end() && nCurRow <= nEnd; ++it, nOffset = 0, nCurRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nCurRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nCurRow + 1;
            bLastBlock = true;
        }

        rFunc(*it, nOffset, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
}

template<typename BlkT, typename ItrT, typename NodeT, typename FuncElem>
void EachElem(NodeT& rNode, size_t nOffset, size_t nDataSize, FuncElem& rFuncElem)
{
    ItrT it = BlkT::begin(*rNode.data);
    std::advance(it, nOffset);
    ItrT itEnd = it;
    std::advance(itEnd, nDataSize);
    size_t nRow = rNode.position + nOffset;
    for (; it != itEnd; ++it, ++nRow)
        rFuncElem(nRow, *it);
}

template<typename BlkT, typename ItrT, typename NodeT, typename FuncElem>
void EachElem(NodeT& rNode, FuncElem& rFuncElem)
{
    auto it = BlkT::begin(*rNode.data);
    auto itEnd = BlkT::end(*rNode.data);
    size_t nRow = rNode.position;
    for (; it != itEnd; ++it, ++nRow)
        rFuncElem(nRow, *it);
}

template<typename BlkT, typename ItrT, typename NodeT, typename FuncElem>
void EachElemReverse(NodeT& rNode, FuncElem& rFuncElem)
{
    auto it = BlkT::rbegin(*rNode.data);
    auto itEnd = BlkT::rend(*rNode.data);
    size_t nRow = rNode.position;
    for (; it != itEnd; ++it, ++nRow)
        rFuncElem(nRow, *it);
}

template<typename BlkT, typename StoreT, typename FuncElem>
std::pair<typename StoreT::const_iterator, size_t>
CheckElem(
    const StoreT& rStore, const typename StoreT::const_iterator& it, size_t nOffset, size_t nDataSize,
    FuncElem& rFuncElem)
{
    typedef std::pair<typename StoreT::const_iterator, size_t> PositionType;

    typename BlkT::const_iterator itData = BlkT::begin(*it->data);
    std::advance(itData, nOffset);
    typename BlkT::const_iterator itDataEnd = itData;
    std::advance(itDataEnd, nDataSize);
    size_t nTopRow = it->position + nOffset;
    size_t nRow = nTopRow;
    for (; itData != itDataEnd; ++itData, ++nRow)
    {
        if (rFuncElem(nRow, *itData))
            return PositionType(it, nRow - it->position);
    }

    return PositionType(rStore.end(), 0);
}

template<typename StoreT, typename BlkT, typename FuncElem, typename FuncElse>
void ParseElements1(const StoreT& rStore, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typename StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename StoreT::const_iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        if (it->type != BlkT::block_type)
        {
            rFuncElse(it->type, nTopRow, nDataSize);
            continue;
        }

        EachElem<BlkT, typename BlkT::const_iterator>(*it, rFuncElem);
    }
}

template<typename StoreT, typename BlkT, typename FuncElem, typename FuncElse>
typename StoreT::const_iterator
ParseElements1(
    const typename StoreT::const_iterator& itPos, const StoreT& rStore,
    typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        if (it->type == BlkT::block_type)
            EachElem<BlkT, typename BlkT::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
        else
            rFuncElse(it->type, nTopRow, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
};

template<typename StoreT, typename Blk1, typename Blk2, typename FuncElem, typename FuncElse>
typename StoreT::const_iterator
ParseElements2(
    const typename StoreT::const_iterator& itPos, const StoreT& rStore, typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        switch (it->type)
        {
            case Blk1::block_type:
                EachElem<Blk1, typename Blk1::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case Blk2::block_type:
                EachElem<Blk2, typename Blk2::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }

        if (bLastBlock)
            break;
    }

    return it;
}

template<typename StoreT, typename Blk1, typename Blk2, typename Blk3, typename Blk4, typename FuncElem, typename FuncElse>
typename StoreT::const_iterator
ParseElements4(
    const typename StoreT::const_iterator& itPos, const StoreT& rStore, typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        switch (it->type)
        {
            case Blk1::block_type:
                EachElem<Blk1, typename Blk1::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case Blk2::block_type:
                EachElem<Blk2, typename Blk2::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case Blk3::block_type:
                EachElem<Blk3, typename Blk3::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case Blk4::block_type:
                EachElem<Blk4, typename Blk4::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }

        if (bLastBlock)
            break;
    }

    return it;
}

template<typename StoreT, typename BlkT, typename FuncElem, typename FuncElse>
void ProcessElements1(StoreT& rStore, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typename StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename StoreT::iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        if (it->type != BlkT::block_type)
        {
            rFuncElse(it->type, nTopRow, nDataSize);
            continue;
        }

        EachElem<BlkT, typename BlkT::iterator>(*it, rFuncElem);
    }
}

/**
 * This variant specifies start and end positions.
 */
template<typename StoreT, typename BlkT, typename FuncElem, typename FuncElse>
typename StoreT::iterator
ProcessElements1(
    const typename StoreT::iterator& itPos, StoreT& rStore,
    typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::iterator, typename StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename StoreT::iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        if (it->type == BlkT::block_type)
            EachElem<BlkT, typename BlkT::iterator>(*it, nOffset, nDataSize, rFuncElem);
        else
            rFuncElse(it->type, nTopRow, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
};

template<typename StoreT, typename Blk1, typename Blk2, typename FuncElem, typename FuncElse>
void ProcessElements2(StoreT& rStore, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typename StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename StoreT::iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        switch (it->type)
        {
            case Blk1::block_type:
                EachElem<Blk1, typename Blk1::iterator>(*it, rFuncElem);
            break;
            case Blk2::block_type:
                EachElem<Blk2, typename Blk2::iterator>(*it, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }
    }
}

template<typename StoreT, typename Blk1, typename Blk2, typename FuncElem, typename FuncElse>
void ProcessElements2Reverse(StoreT& rStore, FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typename StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename StoreT::iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        switch (it->type)
        {
            case Blk1::block_type:
                EachElemReverse<Blk1, typename Blk1::iterator>(*it, rFuncElem);
            break;
            case Blk2::block_type:
                EachElemReverse<Blk2, typename Blk2::iterator>(*it, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }
    }
}

template<typename StoreT, typename Blk1, typename FuncElem, typename FuncElse>
std::pair<typename StoreT::const_iterator, typename StoreT::size_type>
FindElement1(
    const StoreT& rStore, typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;
    typedef std::pair<typename StoreT::size_type, bool> ElseRetType;

    PositionType aPos = rStore.position(nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        switch (it->type)
        {
            case Blk1::block_type:
            {
                PositionType aRet = CheckElem<Blk1>(rStore, it, nOffset, nDataSize, rFuncElem);
                if (aRet.first != rStore.end())
                    return aRet;
            }
            break;
            default:
            {
                ElseRetType aRet = rFuncElse(it->type, nTopRow, nDataSize);
                if (aRet.second)
                    return PositionType(it, aRet.first);
            }
        }

        if (bLastBlock)
            break;
    }

    return PositionType(rStore.end(), 0);
}

template<typename StoreT, typename Blk1, typename Blk2, typename FuncElem, typename FuncElse>
std::pair<typename StoreT::const_iterator, typename StoreT::size_type>
FindElement2(
    const StoreT& rStore, typename StoreT::size_type nStart, typename StoreT::size_type nEnd,
    FuncElem& rFuncElem, FuncElse& rFuncElse)
{
    typedef std::pair<typename StoreT::const_iterator, typename StoreT::size_type> PositionType;
    typedef std::pair<typename StoreT::size_type, bool> ElseRetType;

    PositionType aPos = rStore.position(nStart);
    typename StoreT::const_iterator it = aPos.first;
    typename StoreT::size_type nOffset = aPos.second;
    typename StoreT::size_type nDataSize = 0;
    typename StoreT::size_type nTopRow = nStart;

    for (; it != rStore.end() && nTopRow <= nEnd; ++it, nOffset = 0, nTopRow += nDataSize)
    {
        bool bLastBlock = false;
        nDataSize = it->size - nOffset;
        if (nTopRow + nDataSize - 1 > nEnd)
        {
            // Truncate the block.
            nDataSize = nEnd - nTopRow + 1;
            bLastBlock = true;
        }

        switch (it->type)
        {
            case Blk1::block_type:
            {
                PositionType aRet = CheckElem<Blk1>(rStore, it, nOffset, nDataSize, rFuncElem);
                if (aRet.first != rStore.end())
                    return aRet;
            }
            break;
            case Blk2::block_type:
            {
                PositionType aRet = CheckElem<Blk2>(rStore, it, nOffset, nDataSize, rFuncElem);
                if (aRet.first != rStore.end())
                    return aRet;
            }
            break;
            default:
            {
                ElseRetType aRet = rFuncElse(*it, nOffset, nDataSize);
                if (aRet.second)
                    return PositionType(it, aRet.first);
            }
        }

        if (bLastBlock)
            break;
    }

    return PositionType(rStore.end(), 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
