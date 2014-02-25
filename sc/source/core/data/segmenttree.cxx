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


#include "segmenttree.hxx"

#include <mdds/flat_segment_tree.hpp>

#include <limits>

using ::std::numeric_limits;



template<typename _ValueType, typename _ExtValueType = _ValueType>
class ScFlatSegmentsImpl
{
public:
    typedef _ValueType ValueType;
    typedef _ExtValueType ExtValueType;

    struct RangeData
    {
        SCCOLROW    mnPos1;
        SCCOLROW    mnPos2;
        ValueType   mnValue;
    };

    ScFlatSegmentsImpl(SCCOLROW nMax, ValueType nDefault);
    ScFlatSegmentsImpl(const ScFlatSegmentsImpl& r);
    ~ScFlatSegmentsImpl();

    bool setValue(SCCOLROW nPos1, SCCOLROW nPos2, ValueType nValue);
    ValueType getValue(SCCOLROW nPos);
    ExtValueType getSumValue(SCCOLROW nPos1, SCCOLROW nPos2);
    bool getRangeData(SCCOLROW nPos, RangeData& rData);
    bool getRangeDataLeaf(SCCOLROW nPos, RangeData& rData);
    void removeSegment(SCCOLROW nPos1, SCCOLROW nPos2);
    void insertSegment(SCCOLROW nPos, SCCOLROW nSize, bool bSkipStartBoundary);

    SCROW findLastNotOf(ValueType nValue) const;

    // range iteration
    bool getFirst(RangeData& rData);
    bool getNext(RangeData& rData);

    void enableTreeSearch(bool b)
    {
        mbTreeSearchEnabled = b;
    }

private:
    typedef ::mdds::flat_segment_tree<SCCOLROW, ValueType> fst_type;
    fst_type maSegments;
    typename fst_type::const_iterator maItr;

    bool mbTreeSearchEnabled:1;
};

template<typename _ValueType, typename _ExtValueType>
ScFlatSegmentsImpl<_ValueType, _ExtValueType>::ScFlatSegmentsImpl(SCCOLROW nMax, ValueType nDefault) :
    maSegments(0, nMax+1, nDefault),
    mbTreeSearchEnabled(true)
{
}

template<typename _ValueType, typename _ExtValueType>
ScFlatSegmentsImpl<_ValueType, _ExtValueType>::ScFlatSegmentsImpl(const ScFlatSegmentsImpl<_ValueType, _ExtValueType>& r) :
    maSegments(r.maSegments),
    mbTreeSearchEnabled(r.mbTreeSearchEnabled)
{
}

template<typename _ValueType, typename _ExtValueType>
ScFlatSegmentsImpl<_ValueType, _ExtValueType>::~ScFlatSegmentsImpl()
{
}

template<typename _ValueType, typename _ExtValueType>
bool ScFlatSegmentsImpl<_ValueType, _ExtValueType>::setValue(SCCOLROW nPos1, SCCOLROW nPos2, ValueType nValue)
{
    ::std::pair<typename fst_type::const_iterator, bool> ret;
    ret = maSegments.insert(maItr, nPos1, nPos2+1, nValue);
    maItr = ret.first;
    return ret.second;
}

template<typename _ValueType, typename _ExtValueType>
typename ScFlatSegmentsImpl<_ValueType, _ExtValueType>::ValueType ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getValue(SCCOLROW nPos)
{
    ValueType nValue = 0;
    if (!mbTreeSearchEnabled)
    {
        maSegments.search(nPos, nValue);
        return nValue;
    }

    if (!maSegments.is_tree_valid())
        maSegments.build_tree();

    maSegments.search_tree(nPos, nValue);
    return nValue;
}

template<typename _ValueType, typename _ExtValueType>
typename ScFlatSegmentsImpl<_ValueType, _ExtValueType>::ExtValueType
ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getSumValue(SCCOLROW nPos1, SCCOLROW nPos2)
{
    RangeData aData;
    if (!getRangeData(nPos1, aData))
        return 0;

    sal_uInt32 nValue = 0;

    SCROW nCurPos = nPos1;
    SCROW nEndPos = aData.mnPos2;
    while (nEndPos <= nPos2)
    {
        nValue += aData.mnValue * (nEndPos - nCurPos + 1);
        nCurPos = nEndPos + 1;
        if (!getRangeData(nCurPos, aData))
            break;

        nEndPos = aData.mnPos2;
    }
    if (nCurPos <= nPos2)
    {
        nEndPos = ::std::min(nEndPos, nPos2);
        nValue += aData.mnValue * (nEndPos - nCurPos + 1);
    }
    return nValue;
}

template<typename _ValueType, typename _ExtValueType>
bool ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getRangeData(SCCOLROW nPos, RangeData& rData)
{
    if (!mbTreeSearchEnabled)
        return getRangeDataLeaf(nPos, rData);

    if (!maSegments.is_tree_valid())
        maSegments.build_tree();

    if (!maSegments.search_tree(nPos, rData.mnValue, &rData.mnPos1, &rData.mnPos2).second)
        return false;

    rData.mnPos2 = rData.mnPos2-1; // end point is not inclusive.
    return true;
}

template<typename _ValueType, typename _ExtValueType>
bool ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getRangeDataLeaf(SCCOLROW nPos, RangeData& rData)
{
    // Conduct leaf-node only search.  Faster when searching between range insertion.
    const ::std::pair<typename fst_type::const_iterator, bool> &ret =
        maSegments.search(maItr, nPos, rData.mnValue, &rData.mnPos1, &rData.mnPos2);

    if (!ret.second)
        return false;

    maItr = ret.first;

    rData.mnPos2 = rData.mnPos2-1; // end point is not inclusive.
    return true;
}

template<typename _ValueType, typename _ExtValueType>
void ScFlatSegmentsImpl<_ValueType, _ExtValueType>::removeSegment(SCCOLROW nPos1, SCCOLROW nPos2)
{
    maSegments.shift_left(nPos1, nPos2);
    maItr = maSegments.begin();
}

template<typename _ValueType, typename _ExtValueType>
void ScFlatSegmentsImpl<_ValueType, _ExtValueType>::insertSegment(SCCOLROW nPos, SCCOLROW nSize, bool bSkipStartBoundary)
{
    maSegments.shift_right(nPos, nSize, bSkipStartBoundary);
    maItr = maSegments.begin();
}

template<typename _ValueType, typename _ExtValueType>
SCCOLROW ScFlatSegmentsImpl<_ValueType, _ExtValueType>::findLastNotOf(ValueType nValue) const
{
    SCCOLROW nPos = numeric_limits<SCCOLROW>::max(); // position not found.
    typename fst_type::const_reverse_iterator itr = maSegments.rbegin(), itrEnd = maSegments.rend();
    // Note that when searching in reverse direction, we need to skip the first
    // node, since the right-most leaf node does not store a valid value.
    for (++itr; itr != itrEnd; ++itr)
    {
        if (itr->second != nValue)
        {
            nPos = (--itr)->first - 1;
            break;
        }
    }
    return nPos;
}

template<typename _ValueType, typename _ExtValueType>
bool ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getFirst(RangeData& rData)
{
    maItr = maSegments.begin();
    return getNext(rData);
}

template<typename _ValueType, typename _ExtValueType>
bool ScFlatSegmentsImpl<_ValueType, _ExtValueType>::getNext(RangeData& rData)
{
    typename fst_type::const_iterator itrEnd = maSegments.end();
    if (maItr == itrEnd)
        return false;

    rData.mnPos1 = maItr->first;
    rData.mnValue = maItr->second;

    ++maItr;
    if (maItr == itrEnd)
        return false;

    rData.mnPos2 = maItr->first - 1;
    return true;
}



class ScFlatUInt16SegmentsImpl : public ScFlatSegmentsImpl<sal_uInt16, sal_uInt32>
{
public:
    explicit ScFlatUInt16SegmentsImpl(SCCOLROW nMax, sal_uInt16 nDefault) :
        ScFlatSegmentsImpl<sal_uInt16, sal_uInt32>(nMax, nDefault)
    {
    }
};



class ScFlatBoolSegmentsImpl : public ScFlatSegmentsImpl<bool>
{
public:
    explicit ScFlatBoolSegmentsImpl(SCCOLROW nMax) :
        ScFlatSegmentsImpl<bool>(nMax, false)
    {
    }

    bool setTrue(SCCOLROW nPos1, SCCOLROW nPos2);
    bool setFalse(SCCOLROW nPos1, SCCOLROW nPos2);
};

bool ScFlatBoolSegmentsImpl::setTrue(SCCOLROW nPos1, SCCOLROW nPos2)
{
    return setValue(nPos1, nPos2, true);
}

bool ScFlatBoolSegmentsImpl::setFalse(SCCOLROW nPos1, SCCOLROW nPos2)
{
    return setValue(nPos1, nPos2, false);
}



ScFlatBoolRowSegments::ForwardIterator::ForwardIterator(ScFlatBoolRowSegments& rSegs) :
    mrSegs(rSegs), mnCurPos(0), mnLastPos(-1), mbCurValue(false)
{
}

bool ScFlatBoolRowSegments::ForwardIterator::getValue(SCROW nPos, bool& rVal)
{
    if (nPos >= mnCurPos)
        // It can only go in a forward direction.
        mnCurPos = nPos;

    if (mnCurPos > mnLastPos)
    {
        // position not in the current segment.  Update the current value.
        ScFlatBoolRowSegments::RangeData aData;
        if (!mrSegs.getRangeData(mnCurPos, aData))
            return false;

        mbCurValue = aData.mbValue;
        mnLastPos = aData.mnRow2;
    }

    rVal = mbCurValue;
    return true;
}

SCROW ScFlatBoolRowSegments::ForwardIterator::getLastPos() const
{
    return mnLastPos;
}



ScFlatBoolRowSegments::RangeIterator::RangeIterator(ScFlatBoolRowSegments& rSegs) :
    mrSegs(rSegs)
{
}

bool ScFlatBoolRowSegments::RangeIterator::getFirst(RangeData& rRange)
{
    ScFlatBoolSegmentsImpl::RangeData aData;
    if (!mrSegs.mpImpl->getFirst(aData))
        return false;

    rRange.mnRow1  = static_cast<SCROW>(aData.mnPos1);
    rRange.mnRow2  = static_cast<SCROW>(aData.mnPos2);
    rRange.mbValue = static_cast<bool>(aData.mnValue);
    return true;
}

bool ScFlatBoolRowSegments::RangeIterator::getNext(RangeData& rRange)
{
    ScFlatBoolSegmentsImpl::RangeData aData;
    if (!mrSegs.mpImpl->getNext(aData))
        return false;

    rRange.mnRow1  = static_cast<SCROW>(aData.mnPos1);
    rRange.mnRow2  = static_cast<SCROW>(aData.mnPos2);
    rRange.mbValue = static_cast<bool>(aData.mnValue);
    return true;
}



ScFlatBoolRowSegments::ScFlatBoolRowSegments() :
    mpImpl(new ScFlatBoolSegmentsImpl(static_cast<SCCOLROW>(MAXROW)))
{
}

ScFlatBoolRowSegments::ScFlatBoolRowSegments(const ScFlatBoolRowSegments& r) :
    mpImpl(new ScFlatBoolSegmentsImpl(*r.mpImpl))
{
}

ScFlatBoolRowSegments::~ScFlatBoolRowSegments()
{
}

bool ScFlatBoolRowSegments::setTrue(SCROW nRow1, SCROW nRow2)
{
    return mpImpl->setTrue(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2));
}

bool ScFlatBoolRowSegments::setFalse(SCROW nRow1, SCROW nRow2)
{
    return mpImpl->setFalse(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2));
}

bool ScFlatBoolRowSegments::getRangeData(SCROW nRow, RangeData& rData)
{
    ScFlatBoolSegmentsImpl::RangeData aData;
    if (!mpImpl->getRangeData(static_cast<SCCOLROW>(nRow), aData))
        return false;

    rData.mbValue = aData.mnValue;
    rData.mnRow1  = static_cast<SCROW>(aData.mnPos1);
    rData.mnRow2  = static_cast<SCROW>(aData.mnPos2);
    return true;
}

bool ScFlatBoolRowSegments::getRangeDataLeaf(SCROW nRow, RangeData& rData)
{
    ScFlatBoolSegmentsImpl::RangeData aData;
    if (!mpImpl->getRangeDataLeaf(static_cast<SCCOLROW>(nRow), aData))
        return false;

    rData.mbValue = aData.mnValue;
    rData.mnRow1  = static_cast<SCROW>(aData.mnPos1);
    rData.mnRow2  = static_cast<SCROW>(aData.mnPos2);
    return true;
}

void ScFlatBoolRowSegments::removeSegment(SCROW nRow1, SCROW nRow2)
{
    mpImpl->removeSegment(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2));
}

void ScFlatBoolRowSegments::insertSegment(SCROW nRow, SCROW nSize, bool bSkipStartBoundary)
{
    mpImpl->insertSegment(static_cast<SCCOLROW>(nRow), static_cast<SCCOLROW>(nSize), bSkipStartBoundary);
}

SCROW ScFlatBoolRowSegments::findLastNotOf(bool bValue) const
{
    return static_cast<SCROW>(mpImpl->findLastNotOf(bValue));
}



ScFlatBoolColSegments::ScFlatBoolColSegments() :
    mpImpl(new ScFlatBoolSegmentsImpl(static_cast<SCCOLROW>(MAXCOL)))
{
}

ScFlatBoolColSegments::ScFlatBoolColSegments(const ScFlatBoolColSegments& r) :
    mpImpl(new ScFlatBoolSegmentsImpl(*r.mpImpl))
{
}

ScFlatBoolColSegments::~ScFlatBoolColSegments()
{
}

bool ScFlatBoolColSegments::setTrue(SCCOL nCol1, SCCOL nCol2)
{
    return mpImpl->setTrue(static_cast<SCCOLROW>(nCol1), static_cast<SCCOLROW>(nCol2));
}

bool ScFlatBoolColSegments::setFalse(SCCOL nCol1, SCCOL nCol2)
{
    return mpImpl->setFalse(static_cast<SCCOLROW>(nCol1), static_cast<SCCOLROW>(nCol2));
}

bool ScFlatBoolColSegments::getRangeData(SCCOL nCol, RangeData& rData)
{
    ScFlatBoolSegmentsImpl::RangeData aData;
    if (!mpImpl->getRangeData(static_cast<SCCOLROW>(nCol), aData))
        return false;

    rData.mbValue = aData.mnValue;
    rData.mnCol1  = static_cast<SCCOL>(aData.mnPos1);
    rData.mnCol2  = static_cast<SCCOL>(aData.mnPos2);
    return true;
}

void ScFlatBoolColSegments::removeSegment(SCCOL nCol1, SCCOL nCol2)
{
    mpImpl->removeSegment(static_cast<SCCOLROW>(nCol1), static_cast<SCCOLROW>(nCol2));
}

void ScFlatBoolColSegments::insertSegment(SCCOL nCol, SCCOL nSize, bool bSkipStartBoundary)
{
    mpImpl->insertSegment(static_cast<SCCOLROW>(nCol), static_cast<SCCOLROW>(nSize), bSkipStartBoundary);
}



ScFlatUInt16RowSegments::ForwardIterator::ForwardIterator(ScFlatUInt16RowSegments& rSegs) :
    mrSegs(rSegs), mnCurPos(0), mnLastPos(-1), mnCurValue(0)
{
}

bool ScFlatUInt16RowSegments::ForwardIterator::getValue(SCROW nPos, sal_uInt16& rVal)
{
    if (nPos >= mnCurPos)
        // It can only go in a forward direction.
        mnCurPos = nPos;

    if (mnCurPos > mnLastPos)
    {
        // position not in the current segment.  Update the current value.
        ScFlatUInt16RowSegments::RangeData aData;
        if (!mrSegs.getRangeData(mnCurPos, aData))
            return false;

        mnCurValue = aData.mnValue;
        mnLastPos = aData.mnRow2;
    }

    rVal = mnCurValue;
    return true;
}

SCROW ScFlatUInt16RowSegments::ForwardIterator::getLastPos() const
{
    return mnLastPos;
}



ScFlatUInt16RowSegments::ScFlatUInt16RowSegments(sal_uInt16 nDefault) :
    mpImpl(new ScFlatUInt16SegmentsImpl(static_cast<SCCOLROW>(MAXROW), nDefault))
{
}

ScFlatUInt16RowSegments::ScFlatUInt16RowSegments(const ScFlatUInt16RowSegments& r) :
    mpImpl(new ScFlatUInt16SegmentsImpl(*r.mpImpl))
{
}

ScFlatUInt16RowSegments::~ScFlatUInt16RowSegments()
{
}

void ScFlatUInt16RowSegments::setValue(SCROW nRow1, SCROW nRow2, sal_uInt16 nValue)
{
    mpImpl->setValue(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2), nValue);
}

sal_uInt16 ScFlatUInt16RowSegments::getValue(SCROW nRow)
{
    return mpImpl->getValue(static_cast<SCCOLROW>(nRow));
}

sal_uInt32 ScFlatUInt16RowSegments::getSumValue(SCROW nRow1, SCROW nRow2)
{
    return mpImpl->getSumValue(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2));
}

bool ScFlatUInt16RowSegments::getRangeData(SCROW nRow, RangeData& rData)
{
    ScFlatUInt16SegmentsImpl::RangeData aData;
    if (!mpImpl->getRangeData(static_cast<SCCOLROW>(nRow), aData))
        return false;

    rData.mnRow1  = aData.mnPos1;
    rData.mnRow2  = aData.mnPos2;
    rData.mnValue = aData.mnValue;
    return true;
}

void ScFlatUInt16RowSegments::removeSegment(SCROW nRow1, SCROW nRow2)
{
    mpImpl->removeSegment(static_cast<SCCOLROW>(nRow1), static_cast<SCCOLROW>(nRow2));
}

void ScFlatUInt16RowSegments::insertSegment(SCROW nRow, SCROW nSize, bool bSkipStartBoundary)
{
    mpImpl->insertSegment(static_cast<SCCOLROW>(nRow), static_cast<SCCOLROW>(nSize), bSkipStartBoundary);
}

SCROW ScFlatUInt16RowSegments::findLastNotOf(sal_uInt16 nValue) const
{
    return static_cast<SCROW>(mpImpl->findLastNotOf(nValue));
}

void ScFlatUInt16RowSegments::enableTreeSearch(bool bEnable)
{
    mpImpl->enableTreeSearch(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
