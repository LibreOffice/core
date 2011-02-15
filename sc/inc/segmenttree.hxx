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

#ifndef SC_SEGMENTTREE_HXX
#define SC_SEGMENTTREE_HXX

#include "address.hxx"

#include <memory>

class ScFlatBoolSegmentsImpl;

class ScFlatBoolRowSegments
{
public:
    struct RangeData
    {
        SCROW   mnRow1;
        SCROW   mnRow2;
        bool    mbValue;
    };

    class ForwardIterator
    {
    public:
        explicit ForwardIterator(ScFlatBoolRowSegments& rSegs);

        bool getValue(SCROW nPos, bool& rVal);
        SCROW getLastPos() const;

    private:
        ScFlatBoolRowSegments&  mrSegs;

        SCROW   mnCurPos;
        SCROW   mnLastPos;
        bool    mbCurValue;
    };

    class RangeIterator
    {
    public:
        explicit RangeIterator(ScFlatBoolRowSegments& rSegs);
        bool getFirst(RangeData& rRange);
        bool getNext(RangeData& rRange);
    private:
        ScFlatBoolRowSegments& mrSegs;
    };

    ScFlatBoolRowSegments();
    ScFlatBoolRowSegments(const ScFlatBoolRowSegments& r);
    ~ScFlatBoolRowSegments();

    bool setTrue(SCROW nRow1, SCROW nRow2);
    bool setFalse(SCROW nRow1, SCROW nRow2);
    bool getValue(SCROW nRow);
    bool getRangeData(SCROW nRow, RangeData& rData);
    void removeSegment(SCROW nRow1, SCROW nRow2);
    void insertSegment(SCROW nRow, SCROW nSize, bool bSkipStartBoundary);

    void enableTreeSearch(bool bEnable);
    SCROW findLastNotOf(bool bValue) const;

private:
    ::std::auto_ptr<ScFlatBoolSegmentsImpl> mpImpl;
};

// ============================================================================

class ScFlatBoolColSegments
{
public:
    struct RangeData
    {
        SCCOL   mnCol1;
        SCCOL   mnCol2;
        bool    mbValue;
    };
    ScFlatBoolColSegments();
    ScFlatBoolColSegments(const ScFlatBoolColSegments& r);
    ~ScFlatBoolColSegments();

    bool setTrue(SCCOL nCol1, SCCOL nCol2);
    bool setFalse(SCCOL nCol1, SCCOL nCol2);
    bool getRangeData(SCCOL nCol, RangeData& rData);
    void removeSegment(SCCOL nCol1, SCCOL nCol2);
    void insertSegment(SCCOL nCol, SCCOL nSize, bool bSkipStartBoundary);

private:
    ::std::auto_ptr<ScFlatBoolSegmentsImpl> mpImpl;
};

// ============================================================================

class ScFlatUInt16SegmentsImpl;

class ScFlatUInt16RowSegments
{
public:
    struct RangeData
    {
        SCROW       mnRow1;
        SCROW       mnRow2;
        sal_uInt16  mnValue;
    };

    class ForwardIterator
    {
    public:
        explicit ForwardIterator(ScFlatUInt16RowSegments& rSegs);

        bool getValue(SCROW nPos, sal_uInt16& rVal);
        SCROW getLastPos() const;

    private:
        ScFlatUInt16RowSegments&  mrSegs;

        SCROW       mnCurPos;
        SCROW       mnLastPos;
        sal_uInt16  mnCurValue;
    };

    ScFlatUInt16RowSegments(sal_uInt16 nDefault);
    ScFlatUInt16RowSegments(const ScFlatUInt16RowSegments& r);
    ~ScFlatUInt16RowSegments();

    void setValue(SCROW nRow1, SCROW nRow2, sal_uInt16 nValue);
    sal_uInt16 getValue(SCROW nRow);
    sal_uInt32 getSumValue(SCROW nRow1, SCROW nRow2);
    bool getRangeData(SCROW nRow, RangeData& rData);
    void removeSegment(SCROW nRow1, SCROW nRow2);
    void insertSegment(SCROW nRow, SCROW nSize, bool bSkipStartBoundary);

    SCROW findLastNotOf(sal_uInt16 nValue) const;

    void enableTreeSearch(bool bEnable);

private:
    ::std::auto_ptr<ScFlatUInt16SegmentsImpl> mpImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
