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


