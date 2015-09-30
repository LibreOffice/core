/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_MTVFUNCTIONS_HXX
#define INCLUDED_SC_INC_MTVFUNCTIONS_HXX

#include <cstdlib>
#include <mdds/multi_type_vector_types.hpp>

namespace sc {

template<typename _SizeT, typename _Ret = bool>
struct FuncElseNoOp
{
    _Ret operator() (mdds::mtv::element_t, _SizeT, _SizeT) const
    {
        return _Ret();
    }
};

/**
 * Generic algorithm to parse blocks of multi_type_vector either partially
 * or fully.
 */
template<typename _StoreT, typename _Func>
typename _StoreT::const_iterator
ParseBlock(
    const typename _StoreT::const_iterator& itPos, const _StoreT& rStore, _Func& rFunc,
    typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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
template<typename _StoreT, typename _Func>
typename _StoreT::iterator
ProcessBlock(const typename _StoreT::iterator& itPos, _StoreT& rStore, _Func& rFunc, typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd)
{
    typedef std::pair<typename _StoreT::iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nCurRow = nStart;

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

template<typename _BlkT, typename _ItrT, typename _NodeT, typename _FuncElem>
void EachElem(_NodeT& rNode, size_t nOffset, size_t nDataSize, _FuncElem& rFuncElem)
{
    _ItrT it = _BlkT::begin(*rNode.data);
    std::advance(it, nOffset);
    _ItrT itEnd = it;
    std::advance(itEnd, nDataSize);
    size_t nRow = rNode.position + nOffset;
    for (; it != itEnd; ++it, ++nRow)
        rFuncElem(nRow, *it);
}

template<typename _BlkT, typename _ItrT, typename _NodeT, typename _FuncElem>
void EachElem(_NodeT& rNode, _FuncElem& rFuncElem)
{
    _ItrT it = _BlkT::begin(*rNode.data);
    _ItrT itEnd = _BlkT::end(*rNode.data);
    size_t nRow = rNode.position;
    for (; it != itEnd; ++it, ++nRow)
        rFuncElem(nRow, *it);
}

template<typename _BlkT, typename _StoreT, typename _FuncElem>
std::pair<typename _StoreT::const_iterator, size_t>
CheckElem(
    const _StoreT& rStore, const typename _StoreT::const_iterator& it, size_t nOffset, size_t nDataSize,
    _FuncElem& rFuncElem)
{
    typedef std::pair<typename _StoreT::const_iterator, size_t> PositionType;

    typename _BlkT::const_iterator itData = _BlkT::begin(*it->data);
    std::advance(itData, nOffset);
    typename _BlkT::const_iterator itDataEnd = itData;
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

template<typename _StoreT, typename _BlkT, typename _FuncElem, typename _FuncElse>
void ParseElements1(const _StoreT& rStore, _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typename _StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename _StoreT::const_iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        if (it->type != _BlkT::block_type)
        {
            rFuncElse(it->type, nTopRow, nDataSize);
            continue;
        }

        EachElem<_BlkT, typename _BlkT::const_iterator>(*it, rFuncElem);
    }
}

template<typename _StoreT, typename _BlkT, typename _FuncElem, typename _FuncElse>
typename _StoreT::const_iterator
ParseElements1(
    const typename _StoreT::const_iterator& itPos, const _StoreT& rStore,
    typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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

        if (it->type == _BlkT::block_type)
            EachElem<_BlkT, typename _BlkT::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
        else
            rFuncElse(it->type, nTopRow, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
};

template<typename _StoreT, typename _Blk1, typename _Blk2, typename _FuncElem, typename _FuncElse>
typename _StoreT::const_iterator
ParseElements2(
    const typename _StoreT::const_iterator& itPos, const _StoreT& rStore, typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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
            case _Blk1::block_type:
                EachElem<_Blk1, typename _Blk1::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case _Blk2::block_type:
                EachElem<_Blk2, typename _Blk2::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }

        if (bLastBlock)
            break;
    }

    return it;
}

template<typename _StoreT, typename _Blk1, typename _Blk2, typename _Blk3, typename _Blk4, typename _FuncElem, typename _FuncElse>
typename _StoreT::const_iterator
ParseElements4(
    const typename _StoreT::const_iterator& itPos, const _StoreT& rStore, typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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
            case _Blk1::block_type:
                EachElem<_Blk1, typename _Blk1::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case _Blk2::block_type:
                EachElem<_Blk2, typename _Blk2::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case _Blk3::block_type:
                EachElem<_Blk3, typename _Blk3::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            case _Blk4::block_type:
                EachElem<_Blk4, typename _Blk4::const_iterator>(*it, nOffset, nDataSize, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }

        if (bLastBlock)
            break;
    }

    return it;
}

template<typename _StoreT, typename _BlkT, typename _FuncElem, typename _FuncElse>
void ProcessElements1(_StoreT& rStore, _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typename _StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename _StoreT::iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        if (it->type != _BlkT::block_type)
        {
            rFuncElse(it->type, nTopRow, nDataSize);
            continue;
        }

        EachElem<_BlkT, typename _BlkT::iterator>(*it, rFuncElem);
    }
}

/**
 * This variant specifies start and end positions.
 */
template<typename _StoreT, typename _BlkT, typename _FuncElem, typename _FuncElse>
typename _StoreT::iterator
ProcessElements1(
    const typename _StoreT::iterator& itPos, _StoreT& rStore,
    typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::iterator, typename _StoreT::size_type> PositionType;

    PositionType aPos = rStore.position(itPos, nStart);
    typename _StoreT::iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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

        if (it->type == _BlkT::block_type)
            EachElem<_BlkT, typename _BlkT::iterator>(*it, nOffset, nDataSize, rFuncElem);
        else
            rFuncElse(it->type, nTopRow, nDataSize);

        if (bLastBlock)
            break;
    }

    return it;
};

template<typename _StoreT, typename _Blk1, typename _Blk2, typename _FuncElem, typename _FuncElse>
void ProcessElements2(_StoreT& rStore, _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typename _StoreT::size_type nTopRow = 0, nDataSize = 0;
    typename _StoreT::iterator it = rStore.begin(), itEnd = rStore.end();
    for (; it != itEnd; ++it, nTopRow += nDataSize)
    {
        nDataSize = it->size;
        switch (it->type)
        {
            case _Blk1::block_type:
                EachElem<_Blk1, typename _Blk1::iterator>(*it, rFuncElem);
            break;
            case _Blk2::block_type:
                EachElem<_Blk2, typename _Blk2::iterator>(*it, rFuncElem);
            break;
            default:
                rFuncElse(it->type, nTopRow, nDataSize);
        }
    }
}

template<typename _StoreT, typename _Blk1, typename _FuncElem, typename _FuncElse>
std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type>
FindElement1(
    const _StoreT& rStore, typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;
    typedef std::pair<typename _StoreT::size_type, bool> ElseRetType;

    PositionType aPos = rStore.position(nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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
            case _Blk1::block_type:
            {
                PositionType aRet = CheckElem<_Blk1>(rStore, it, nOffset, nDataSize, rFuncElem);
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

template<typename _StoreT, typename _Blk1, typename _Blk2, typename _FuncElem, typename _FuncElse>
std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type>
FindElement2(
    const _StoreT& rStore, typename _StoreT::size_type nStart, typename _StoreT::size_type nEnd,
    _FuncElem& rFuncElem, _FuncElse& rFuncElse)
{
    typedef std::pair<typename _StoreT::const_iterator, typename _StoreT::size_type> PositionType;
    typedef std::pair<typename _StoreT::size_type, bool> ElseRetType;

    PositionType aPos = rStore.position(nStart);
    typename _StoreT::const_iterator it = aPos.first;
    typename _StoreT::size_type nOffset = aPos.second;
    typename _StoreT::size_type nDataSize = 0;
    typename _StoreT::size_type nTopRow = nStart;

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
            case _Blk1::block_type:
            {
                PositionType aRet = CheckElem<_Blk1>(rStore, it, nOffset, nDataSize, rFuncElem);
                if (aRet.first != rStore.end())
                    return aRet;
            }
            break;
            case _Blk2::block_type:
            {
                PositionType aRet = CheckElem<_Blk2>(rStore, it, nOffset, nDataSize, rFuncElem);
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
