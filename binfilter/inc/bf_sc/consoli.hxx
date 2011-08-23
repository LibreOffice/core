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

#ifndef SC_CONSOLI_HXX
#define SC_CONSOLI_HXX

#include "global.hxx"
namespace binfilter {

class ScDocument;

// -----------------------------------------------------------------------

struct ScReferenceEntry				// ohne Constructor !
{
    USHORT	nCol;
    USHORT	nRow;
    USHORT	nTab;
};


//!		Delta-Wert fuer Daten benutzen?

class ScReferenceList			// ohne Constructor !
{
private:
    USHORT				nCount;
    USHORT				nFullSize;			// inkl. Fuell-Eintraege
    ScReferenceEntry*	pData;

public:
    void					Init() 						{ nCount=0; nFullSize=0; pData=NULL; }
    void					Clear() 					{ delete[] pData; }

    USHORT					GetCount()					{ return nCount; }
    const ScReferenceEntry&	GetEntry( USHORT nPos )		{ return pData[nPos]; }
    void					SetFullSize( USHORT nNew )	{ nFullSize = nNew; }
};

// -----------------------------------------------------------------------


//
//	Reihenfolge:
//		1)	ScConsData anlegen
//		2)	Parameter (Size/Flags)
//		3)	AddFields fuer alle Bereiche (nur noetig bei bColByName oder bRowByName)
//		4)	DoneFields					 ( 		"						"		   )
//		5)	AddData fuer alle Bereiche
//			evtl. AddName nach jedem Bereich
//		6)	OutputToDocument
//

//!	ab bestimmter Groesse ScDocument Struktur benutzen?


class ScConsData
{
private:
    ScSubTotalFunc		eFunction;
    BOOL				bReference;
    BOOL				bColByName;
    BOOL				bRowByName;
    BOOL				bSubTitles;
    USHORT				nColCount;
    USHORT				nRowCount;
    BOOL**				ppUsed;
    double**			ppSum;
    double**			ppCount;
    double**			ppSumSqr;
    ScReferenceList**	ppRefs;
    String**			ppColHeaders;
    String**			ppRowHeaders;
    USHORT				nDataCount;
    USHORT				nTitleCount;
    String**			ppTitles;
    USHORT**			ppTitlePos;
    BOOL				bCornerUsed;
    String				aCornerText;		// nur bei bColByName && bRowByName

public:
                ScConsData();
                ~ScConsData();

    void		SetSize( USHORT nCols, USHORT nRows );
    void		SetFlags( ScSubTotalFunc eFunc, BOOL bColName, BOOL bRowName, BOOL bRef );

    void		InitData(BOOL bDelete=TRUE);
    void		DeleteData();

    void		AddFields( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
    void		DoneFields();

    void		AddData( ScDocument* pSrcDoc, USHORT nTab,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );
    void		AddName( const String& rName );

    void		OutputToDocument( ScDocument* pDestDoc, USHORT nCol, USHORT nRow, USHORT nTab );

    void		GetSize( USHORT& rCols, USHORT& rRows ) const;
};


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
