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

#include "address.hxx"
#include "rangelst.hxx"
#include "markmulti.hxx"
#include "scdllapi.h"

#include <set>
#include <vector>

namespace sc {

struct ColRowSpan;

}

class ScMarkArray;

//!     todo:
//!     It should be possible to have MarkArrays for each table, in order to
//!     enable "search all" across more than one table again!

class ScMarkData
{
public:
    typedef std::set<SCTAB> MarkedTabsType;
private:
    MarkedTabsType  maTabMarked;

    ScRange         aMarkRange;             // area
    ScRange         aMultiRange;            // maximum area altogether
    ScMultiSel      aMultiSel;              // multi selection
    ScRangeList     aTopEnvelope;           // list of ranges in the top envelope of the multi selection
    ScRangeList     aBottomEnvelope;        // list of ranges in the bottom envelope of the multi selection
    ScRangeList     aLeftEnvelope;          // list of ranges in the left envelope of the multi selection
    ScRangeList     aRightEnvelope;         // list of ranges in the right envelope of the multi selection
    const ScSheetLimits& mrSheetLimits;
    bool            bMarked:1;              // rectangle marked
    bool            bMultiMarked:1;

    bool            bMarking:1;             // area is being marked -> no MarkToMulti
    bool            bMarkIsNeg:1;           // cancel if multi selection

public:
    SC_DLLPUBLIC ScMarkData(const ScSheetLimits& rSheetLimits);
    ScMarkData(const ScSheetLimits& rSheetLimits, const ScRangeList& rList);
    ScMarkData(const ScMarkData& rData) = default;
    ScMarkData(ScMarkData&& rData) = default;
    ScMarkData& operator=(const ScMarkData& rData);
    ScMarkData& operator=(ScMarkData&& rData);

    SC_DLLPUBLIC void ResetMark();
    SC_DLLPUBLIC void SetMarkArea( const ScRange& rRange );

    // bSetupMulti must be set to true only for recursive calls to SetMultiMarkArea
    SC_DLLPUBLIC void SetMultiMarkArea( const ScRange& rRange, bool bMark = true, bool bSetupMulti = false );

    SC_DLLPUBLIC void MarkToMulti();
    SC_DLLPUBLIC void MarkToSimple();

    bool        IsMarked() const                { return bMarked; }
    bool        IsMultiMarked() const           { return bMultiMarked; }

    const ScRange& GetMarkArea() const { return aMarkRange; }
    const ScRange& GetMultiMarkArea() const { return aMultiRange; }
    const ScRange& GetArea() const { return bMultiMarked ? aMultiRange : aMarkRange; }

    void        SetAreaTab( SCTAB nTab );

    SC_DLLPUBLIC void SelectTable( SCTAB nTab, bool bNew );
    bool        GetTableSelect( SCTAB nTab ) const;

    SC_DLLPUBLIC void SelectOneTable( SCTAB nTab );
    SC_DLLPUBLIC SCTAB GetSelectCount() const;
    SC_DLLPUBLIC SCTAB GetFirstSelected() const;
    SCTAB       GetLastSelected() const;

    const MarkedTabsType& GetSelectedTabs() const { return maTabMarked;}
    SC_DLLPUBLIC void SetSelectedTabs(const MarkedTabsType& rTabs);

    void        SetMarkNegative( bool bFlag )   { bMarkIsNeg = bFlag; }
    bool        IsMarkNegative() const          { return bMarkIsNeg;  }
    void        SetMarking( bool bFlag )        { bMarking = bFlag;   }
    bool        GetMarkingFlag() const          { return bMarking;    }

    //  for FillInfo / Document etc.
    const ScMultiSel& GetMultiSelData() const   { return aMultiSel;   }
    ScMarkArray GetMarkArray( SCCOL nCol ) const { return aMultiSel.GetMarkArray( nCol ); }

    SC_DLLPUBLIC bool IsCellMarked( SCCOL nCol, SCROW nRow, bool bNoSimple = false ) const;

    /** Create a range list of marks.
        @param  nForTab
                If -1, use start-sheet-tab of the multi-area in ranges.
                If >= 0, use given sheet-tab in ranges.
     */
    void        FillRangeListWithMarks( ScRangeList* pList, bool bClear, SCTAB nForTab = -1 ) const;
    void        ExtendRangeListTables( ScRangeList* pList ) const;

    SC_DLLPUBLIC ScRangeList GetMarkedRanges() const;
    /** Get marked ranges with sheet-tab set to nTab.
        Marks are stored for the currently active sheet respectively the
        multi-area start-sheet-tab, update ranges with the sheet for which this
        is called. */
    ScRangeList GetMarkedRangesForTab( SCTAB nTab ) const;

    void        MarkFromRangeList( const ScRangeList& rList, bool bReset );

    std::vector<sc::ColRowSpan> GetMarkedRowSpans() const;
    std::vector<sc::ColRowSpan> GetMarkedColSpans() const;

    SC_DLLPUBLIC bool IsColumnMarked( SCCOL nCol ) const;
    SC_DLLPUBLIC bool IsRowMarked( SCROW nRow ) const;
    SC_DLLPUBLIC bool IsAllMarked( const ScRange& rRange ) const;     // Multi

    // Returns the first column of the range [column,nLastCol] for which
    // all those columns have equal marks. Value returned is not less than nMinCol.
    SCCOL       GetStartOfEqualColumns( SCCOL nLastCol, SCCOL nMinCol = 0 ) const;

                /// May return -1
    SC_DLLPUBLIC SCROW GetNextMarked( SCCOL nCol, SCROW nRow, bool bUp ) const;
    SC_DLLPUBLIC bool HasMultiMarks( SCCOL nCol ) const;
    bool        HasAnyMultiMarks() const;

    //  adjust table marking:
    SC_DLLPUBLIC void InsertTab( SCTAB nTab );
    SC_DLLPUBLIC void DeleteTab( SCTAB nTab );

    void        ShiftCols(const ScDocument& rDoc, SCCOL nStartCol, sal_Int32 nColOffset);
    void        ShiftRows(const ScDocument& rDoc, SCROW nStartRow, sal_Int32 nRowOffset);

    // Generate envelopes if multimarked and fills the passed ScRange object with
    // the smallest range that includes the marked area plus its envelopes.
    SC_DLLPUBLIC void GetSelectionCover( ScRange& rRange );
    // Get top, bottom, left and right envelopes
    const ScRangeList& GetTopEnvelope() const    { return aTopEnvelope;    }
    const ScRangeList& GetBottomEnvelope() const { return aBottomEnvelope; }
    const ScRangeList& GetLeftEnvelope() const   { return aLeftEnvelope;   }
    const ScRangeList& GetRightEnvelope() const  { return aRightEnvelope;  }

    // iterators for table access
    typedef std::set<SCTAB>::const_iterator const_iterator;
    typedef std::set<SCTAB>::const_reverse_iterator const_reverse_iterator;
    const_iterator begin() const { return maTabMarked.begin(); }
    const_iterator end() const { return maTabMarked.end(); }
    const_reverse_iterator rbegin() const { return maTabMarked.rbegin(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
