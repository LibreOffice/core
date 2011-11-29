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

#ifndef SC_MARKDATA_HXX
#define SC_MARKDATA_HXX

#include "address.hxx"
#include <tools/solar.h>
#include "scdllapi.h"

#include <set>

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

    void        MarkFromRangeList( const ScRangeList& rList, bool bReset );

    SCCOLROW    GetMarkColumnRanges( SCCOLROW* pRanges );
    SCCOLROW    GetMarkRowRanges( SCCOLROW* pRanges );

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
