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
    sal_Bool            bTabMarked[MAXTABCOUNT];// Tabelle selektiert
    sal_Bool            bMarked;                // Rechteck markiert
    sal_Bool            bMultiMarked;           // mehrfach markiert

    sal_Bool            bMarking;               // Bereich wird aufgezogen -> kein MarkToMulti
    sal_Bool            bMarkIsNeg;             // Aufheben bei Mehrfachselektion

public:
                ScMarkData();
                ScMarkData(const ScMarkData& rData);
                ~ScMarkData();

    ScMarkData& operator=(const ScMarkData& rData);

    void        ResetMark();
    void        SetMarkArea( const ScRange& rRange );

    void        SetMultiMarkArea( const ScRange& rRange, sal_Bool bMark = sal_True );

    void        MarkToMulti();
    void        MarkToSimple();

    sal_Bool        IsMarked() const                { return bMarked; }
    sal_Bool        IsMultiMarked() const           { return bMultiMarked; }

    void        GetMarkArea( ScRange& rRange ) const;
    void        GetMultiMarkArea( ScRange& rRange ) const;

    void        SetAreaTab( SCTAB nTab );

    void        SelectTable( SCTAB nTab, sal_Bool bNew )        { bTabMarked[nTab] = bNew; }
    sal_Bool        GetTableSelect( SCTAB nTab ) const          { return bTabMarked[nTab]; }

    void        SelectOneTable( SCTAB nTab );
    SCTAB       GetSelectCount() const;
    SCTAB       GetFirstSelected() const;

    void        SetMarkNegative( sal_Bool bFlag )   { bMarkIsNeg = bFlag; }
    sal_Bool        IsMarkNegative() const          { return bMarkIsNeg;  }
    void        SetMarking( sal_Bool bFlag )        { bMarking = bFlag;   }
    sal_Bool        GetMarkingFlag() const          { return bMarking;    }

    //  fuer FillInfo / Document etc.
    const ScMarkArray* GetArray() const         { return pMultiSel; }

    sal_Bool        IsCellMarked( SCCOL nCol, SCROW nRow, sal_Bool bNoSimple = sal_False ) const;
    void        FillRangeListWithMarks( ScRangeList* pList, sal_Bool bClear ) const;
    void        ExtendRangeListTables( ScRangeList* pList ) const;

    void        MarkFromRangeList( const ScRangeList& rList, sal_Bool bReset );

    SCCOLROW    GetMarkColumnRanges( SCCOLROW* pRanges );
    SCCOLROW    GetMarkRowRanges( SCCOLROW* pRanges );

    sal_Bool        IsColumnMarked( SCCOL nCol ) const;
    sal_Bool        IsRowMarked( SCROW nRow ) const;
    sal_Bool        IsAllMarked( const ScRange& rRange ) const;     // Multi

                /// May return -1
    SCsROW      GetNextMarked( SCCOL nCol, SCsROW nRow, sal_Bool bUp ) const;
    sal_Bool        HasMultiMarks( SCCOL nCol ) const;
    sal_Bool        HasAnyMultiMarks() const;

    //  Tabellen-Markierungen anpassen:
    void        InsertTab( SCTAB nTab );
    void        DeleteTab( SCTAB nTab );
};



#endif


