/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
