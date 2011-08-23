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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class ScMarkArray;
class ScRangeList;

//!		todo:
//!		Es muss auch die Moeglichkeit geben, MarkArrays pro Tabelle zu halten,
//!		damit "alle suchen" ueber mehrere Tabellen wieder funktioniert!


class ScMarkData
{
private:
    ScRange			aMarkRange;				// Bereich
    ScRange			aMultiRange;			// maximaler Bereich insgesamt
    ScMarkArray*	pMultiSel;				// Mehrfachselektion
    BOOL			bTabMarked[MAXTAB+1];	// Tabelle selektiert
    BOOL			bMarked;				// Rechteck markiert
    BOOL			bMultiMarked;			// mehrfach markiert

    BOOL			bMarking;				// Bereich wird aufgezogen -> kein MarkToMulti
    BOOL			bMarkIsNeg;				// Aufheben bei Mehrfachselektion

public:
                ScMarkData();
                ScMarkData(const ScMarkData& rData);
                ~ScMarkData();


    void		ResetMark();
    void		SetMarkArea( const ScRange& rRange );

    void		SetMultiMarkArea( const ScRange& rRange, BOOL bMark = TRUE );

    void		MarkToMulti();
    void		MarkToSimple();

    BOOL		IsMarked() const				{ return bMarked; }
    BOOL		IsMultiMarked() const			{ return bMultiMarked; }

    void		GetMarkArea( ScRange& rRange ) const;
    void		GetMultiMarkArea( ScRange& rRange ) const;


    void		SelectTable( USHORT nTab, BOOL bNew )		{ bTabMarked[nTab] = bNew; }
    BOOL		GetTableSelect( USHORT nTab ) const			{ return bTabMarked[nTab]; }

    void		SelectOneTable( USHORT nTab );
    USHORT		GetSelectCount() const;
    void		SetMarkNegative( BOOL bFlag )	{ bMarkIsNeg = bFlag; }
    BOOL		IsMarkNegative() const			{ return bMarkIsNeg;  }
    void		SetMarking( BOOL bFlag )		{ bMarking = bFlag;   }
    BOOL		GetMarkingFlag() const			{ return bMarking;	  }

    //	fuer FillInfo / Document etc.
    const ScMarkArray* GetArray() const			{ return pMultiSel; }

    BOOL		IsCellMarked( USHORT nCol, USHORT nRow, BOOL bNoSimple = FALSE ) const;
    void		FillRangeListWithMarks( ScRangeList* pList, BOOL bClear ) const;

    void		MarkFromRangeList( const ScRangeList& rList, BOOL bReset );


    BOOL		IsAllMarked( const ScRange& rRange ) const;		// Multi

    BOOL		HasMultiMarks( USHORT nCol ) const;
    BOOL		HasAnyMultiMarks() const;
};



} //namespace binfilter
#endif


