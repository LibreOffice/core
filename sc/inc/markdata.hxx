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

#ifndef SC_MARKDATA_HXX
#define SC_MARKDATA_HXX

#include "address.hxx"
#include <tools/solar.h>
#include "scdllapi.h"

#include <set>

namespace sc {

struct RowSpan;

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
    ScMarkArray*    pMultiSel;              // multi selection
    bool            bMarked:1;                // rectangle marked
    bool            bMultiMarked:1;

    bool            bMarking:1;               // area is being marked -> no MarkToMulti
    bool            bMarkIsNeg:1;             // cancel if multi selection

public:
                ScMarkData();
                ScMarkData(const ScMarkData& rData);
                ~ScMarkData();

    ScMarkData& operator=(const ScMarkData& rData);

    void        ResetMark();
    void        SetMarkArea( const ScRange& rRange );

    void        SetMultiMarkArea( const ScRange& rRange, bool bMark = true );

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

    const MarkedTabsType& GetSelectedTabs() const;
    void SetSelectedTabs(const MarkedTabsType& rTabs);

    void        SetMarkNegative( bool bFlag )   { bMarkIsNeg = bFlag; }
    bool        IsMarkNegative() const          { return bMarkIsNeg;  }
    void        SetMarking( bool bFlag )        { bMarking = bFlag;   }
    bool        GetMarkingFlag() const          { return bMarking;    }

    //  for FillInfo / Document etc.
    const ScMarkArray* GetArray() const         { return pMultiSel; }

    bool        IsCellMarked( SCCOL nCol, SCROW nRow, bool bNoSimple = false ) const;
    void        FillRangeListWithMarks( ScRangeList* pList, bool bClear ) const;
    void        ExtendRangeListTables( ScRangeList* pList ) const;

    ScRangeList GetMarkedRanges() const;

    void        MarkFromRangeList( const ScRangeList& rList, bool bReset );

    SCCOLROW    GetMarkColumnRanges( SCCOLROW* pRanges );
    SCCOLROW    GetMarkRowRanges( SCCOLROW* pRanges );

    void GetMarkedRowSpans( SCTAB nTab, std::vector<sc::RowSpan>& rSpans );

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
