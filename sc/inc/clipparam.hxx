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

#ifndef SC_CLIPPARAM_HXX
#define SC_CLIPPARAM_HXX

#include "rangelst.hxx"
#include "rangenam.hxx"
#include "charthelper.hxx"

#include <vector>

/**
 * This struct stores general clipboard parameters associated with a
 * ScDocument instance created in clipboard mode.
 */
struct ScClipParam
{
    enum Direction { Unspecified, Column, Row };

    ScRangeList         maRanges;
    Direction           meDirection;
    bool                mbCutMode;
    sal_uInt32          mnSourceDocID;
    ScRangeListVector   maProtectedChartRangesVector;

    ScClipParam();
    ScClipParam(const ScRange& rRange, bool bCutMode);
    explicit ScClipParam(const ScClipParam& r);

    bool isMultiRange() const;

    /**
     * Get the column size of a pasted range.  Note that when the range is
     * non-contiguous, we first compress all individual ranges into a single
     * range, and the size of that compressed range is returned.
     */
    SCCOL getPasteColSize();

    /**
     * Same as the above method, but returns the row size of the compressed
     * range.
     */
    SCROW getPasteRowSize();

    /**
     * Return a single range that encompasses all individual ranges.
     */
    ScRange getWholeRange() const;

    void transpose();

    sal_uInt32 getSourceDocID() const { return mnSourceDocID; }
    void setSourceDocID( sal_uInt32 nVal ) { mnSourceDocID = nVal; }
};

// ============================================================================

struct ScClipRangeNameData
{
    ScRangeData::IndexMap       maRangeMap;
    ::std::vector<ScRangeData*> mpRangeNames;
    bool                        mbReplace;

    ScClipRangeNameData();
    ~ScClipRangeNameData();
    void insert(sal_uInt16 nOldIndex, sal_uInt16 nNewIndex);
};

#endif
