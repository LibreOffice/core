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

#ifndef INCLUDED_SC_INC_MARKDATA_HXX
#define INCLUDED_SC_INC_MARKDATA_HXX

#include "address.hxx"
#include "rangelst.hxx"
#include "markmulti.hxx"
#include "scdllapi.h"

#include <set>
#include <vector>

namespace sc {

struct RowSpan;
struct ColRowSpan;

}

class ScMarkArray;
class ScRangeList;

//!     todo:
//!     It should be possible to have MarkArrays for each table, in order to
//!     enable "search all" across more than one table again!

class SC_DLLPUBLIC ScMarkData
{
public:
    typedef std::set<SCTAB> MarkedTabsType;
private:
    MarkedTabsType  maTabMarked;

    ScRange         aMarkRange;             // area
    ScRange         aMultiRange;            // maximum area altogether
    ScMultiSel      aMultiSel;              // multi selection
    bool            bMarked:1;                // rectangle marked
    bool            bMultiMarked:1;

    bool            bMarking:1;               // area is being marked -> no MarkToMulti
    bool            bMarkIsNeg:1;             // cancel if multi selection
    ScRangeList     aTopEnvelope;             // list of ranges in the top envelope of the multi selection
    ScRangeList     aBottomEnvelope;          // list of ranges in the bottom envelope of the multi selection
    ScRangeList     aLeftEnvelope;            // list of ranges in the left envelope of the multi selection
    ScRangeList     aRightEnvelope;           // list of ranges in the right envelope of the multi selection


public:
                ScMarkData();
                ScMarkData(const ScMarkData& rData);
                ~ScMarkData();

    ScMarkData& operator=(const ScMarkData& rData);

    void        ResetMark();
    void        SetMarkArea( const ScRange& rRange );

    // bSetupMulti must be set to true only for recursive calls to SetMultiMarkArea
    void        SetMultiMarkArea( const ScRange& rRange, bool bMark = true, bool bSetupMulti = false );

    void        MarkToMulti();
    void        MarkToSimple();

    bool        IsMarked() const                { return bMarked; }
    bool        IsMultiMarked() const           { return bMultiMarked; }

    void        GetMarkArea( ScRange& rRange ) const;
    void        GetMultiMarkArea( ScRange& rRange ) const;

    void        SetAreaTab( SCTAB nTab );

    void        SelectTable( SCTAB nTab, bool bNew );
    bool        GetTableSelect( SCTAB nTab ) const;

    void        SelectOneTable( SCTAB nTab );
    SCTAB       GetSelectCount() const;
    SCTAB       GetFirstSelected() const;
    SCTAB       GetLastSelected() const;

    const MarkedTabsType& GetSelectedTabs() const { return maTabMarked;}
    void SetSelectedTabs(const MarkedTabsType& rTabs);

    void        SetMarkNegative( bool bFlag )   { bMarkIsNeg = bFlag; }
    bool        IsMarkNegative() const          { return bMarkIsNeg;  }
    void        SetMarking( bool bFlag )        { bMarking = bFlag;   }
    bool        GetMarkingFlag() const          { return bMarking;    }

    //  for FillInfo / Document etc.
    const ScMultiSel& GetMultiSelData() const   { return aMultiSel;   }
    ScMarkArray GetMarkArray( SCCOL nCol ) const;

    bool        IsCellMarked( SCCOL nCol, SCROW nRow, bool bNoSimple = false ) const;

    /** Create a range list of marks.
        @param  nForTab
                If -1, use start-sheet-tab of the multi-area in ranges.
                If >= 0, use given sheet-tab in ranges.
     */
    void        FillRangeListWithMarks( ScRangeList* pList, bool bClear, SCTAB nForTab = -1 ) const;
    void        ExtendRangeListTables( ScRangeList* pList ) const;

    ScRangeList GetMarkedRanges() const;
    /** Get marked ranges with sheet-tab set to nTab.
        Marks are stored for the currently active sheet respectively the
        multi-area start-sheet-tab, update ranges with the sheet for which this
        is called. */
    ScRangeList GetMarkedRangesForTab( SCTAB nTab ) const;

    void        MarkFromRangeList( const ScRangeList& rList, bool bReset );

    std::vector<sc::ColRowSpan> GetMarkedRowSpans() const;
    std::vector<sc::ColRowSpan> GetMarkedColSpans() const;

    bool        IsColumnMarked( SCCOL nCol ) const;
    bool        IsRowMarked( SCROW nRow ) const;
    bool        IsAllMarked( const ScRange& rRange ) const;     // Multi

                /// May return -1
    SCsROW      GetNextMarked( SCCOL nCol, SCsROW nRow, bool bUp ) const;
    bool        HasMultiMarks( SCCOL nCol ) const;
    bool        HasAnyMultiMarks() const;

    //  adjust table marking:
    void        InsertTab( SCTAB nTab );
    void        DeleteTab( SCTAB nTab );

    // Generate envelopes if mutimarked and fills the passed ScRange object with
    // the smallest range that includes the marked area plus its envelopes.
    void        GetSelectionCover( ScRange& rRange );
    // Get top, bottom, left and right envelopes
    const ScRangeList& GetTopEnvelope() const    { return aTopEnvelope;    }
    const ScRangeList& GetBottomEnvelope() const { return aBottomEnvelope; }
    const ScRangeList& GetLeftEnvelope() const   { return aLeftEnvelope;   }
    const ScRangeList& GetRightEnvelope() const  { return aRightEnvelope;  }

    // iterators for table access
    typedef std::set<SCTAB>::iterator iterator;
    typedef std::set<SCTAB>::const_iterator const_iterator;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
