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

class ScMarkArray;
class ScRangeList;

//!     todo:
//!     Es muss auch die Moeglichkeit geben, MarkArrays pro Tabelle zu halten,
//!     damit "alle suchen" ueber mehrere Tabellen wieder funktioniert!


class SC_DLLPUBLIC ScMarkData
{
private:
    ScRange         aMarkRange;             // Bereich
    ScRange         aMultiRange;            // maximaler Bereich insgesamt
    ScMarkArray*    pMultiSel;              // Mehrfachselektion
    BOOL            bTabMarked[MAXTABCOUNT];// Tabelle selektiert
    BOOL            bMarked;                // Rechteck markiert
    BOOL            bMultiMarked;           // mehrfach markiert

    BOOL            bMarking;               // Bereich wird aufgezogen -> kein MarkToMulti
    BOOL            bMarkIsNeg;             // Aufheben bei Mehrfachselektion

public:
                ScMarkData();
                ScMarkData(const ScMarkData& rData);
                ~ScMarkData();

    ScMarkData& operator=(const ScMarkData& rData);

    void        ResetMark();
    void        SetMarkArea( const ScRange& rRange );

    void        SetMultiMarkArea( const ScRange& rRange, BOOL bMark = TRUE );

    void        MarkToMulti();
    void        MarkToSimple();

    BOOL        IsMarked() const                { return bMarked; }
    BOOL        IsMultiMarked() const           { return bMultiMarked; }

    void        GetMarkArea( ScRange& rRange ) const;
    void        GetMultiMarkArea( ScRange& rRange ) const;

    void        SetAreaTab( SCTAB nTab );

    void        SelectTable( SCTAB nTab, BOOL bNew )        { bTabMarked[nTab] = bNew; }
    BOOL        GetTableSelect( SCTAB nTab ) const          { return bTabMarked[nTab]; }

    void        SelectOneTable( SCTAB nTab );
    SCTAB       GetSelectCount() const;
    SCTAB       GetFirstSelected() const;

    void        SetMarkNegative( BOOL bFlag )   { bMarkIsNeg = bFlag; }
    BOOL        IsMarkNegative() const          { return bMarkIsNeg;  }
    void        SetMarking( BOOL bFlag )        { bMarking = bFlag;   }
    BOOL        GetMarkingFlag() const          { return bMarking;    }

    //  fuer FillInfo / Document etc.
    const ScMarkArray* GetArray() const         { return pMultiSel; }

    BOOL        IsCellMarked( SCCOL nCol, SCROW nRow, BOOL bNoSimple = FALSE ) const;
    void        FillRangeListWithMarks( ScRangeList* pList, BOOL bClear ) const;
    void        ExtendRangeListTables( ScRangeList* pList ) const;

    void        MarkFromRangeList( const ScRangeList& rList, BOOL bReset );

    SCCOLROW    GetMarkColumnRanges( SCCOLROW* pRanges );
    SCCOLROW    GetMarkRowRanges( SCCOLROW* pRanges );

    BOOL        IsColumnMarked( SCCOL nCol ) const;
    BOOL        IsRowMarked( SCROW nRow ) const;
    BOOL        IsAllMarked( const ScRange& rRange ) const;     // Multi

                /// May return -1
    SCsROW      GetNextMarked( SCCOL nCol, SCsROW nRow, BOOL bUp ) const;
    BOOL        HasMultiMarks( SCCOL nCol ) const;
    BOOL        HasAnyMultiMarks() const;

    //  Tabellen-Markierungen anpassen:
    void        InsertTab( SCTAB nTab );
    void        DeleteTab( SCTAB nTab );
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
