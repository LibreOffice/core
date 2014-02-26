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
    bool getRangeData(SCROW nRow, RangeData& rData);
    bool getRangeDataLeaf(SCROW nRow, RangeData& rData);
    void removeSegment(SCROW nRow1, SCROW nRow2);
    void insertSegment(SCROW nRow, SCROW nSize, bool bSkipStartBoundary);

    SCROW findLastNotOf(bool bValue) const;

private:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScFlatBoolSegmentsImpl> mpImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScFlatBoolSegmentsImpl> mpImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScFlatUInt16SegmentsImpl> mpImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
