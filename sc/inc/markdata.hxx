/*************************************************************************
 *
 *  $RCSfile: markdata.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:11:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_MARKDATA_HXX
#define SC_MARKDATA_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class ScMarkArray;
class ScRangeList;

//!     todo:
//!     Es muss auch die Moeglichkeit geben, MarkArrays pro Tabelle zu halten,
//!     damit "alle suchen" ueber mehrere Tabellen wieder funktioniert!


class ScMarkData
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


