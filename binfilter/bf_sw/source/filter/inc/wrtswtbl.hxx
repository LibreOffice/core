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

#ifndef  _WRTSWTBL_HXX
#define  _WRTSWTBL_HXX

#include <tools/solar.h>
#include <tools/color.hxx>
#include <bf_svtools/svarray.hxx>

#include <orntenum.hxx>
#include <horiornt.hxx>
class Color; 
namespace binfilter {


class SwTableBox;
class SwTableBoxes;
class SwTableLine;
class SwTableLines;
class SwTable;
class SwFrmFmt;
class SwHTMLTableLayout;
class SvxBrushItem;
class SvxBoxItem;
class SvxBorderLine;


//---------------------------------------------------------------------------
// 		 Code aus dem HTML-Filter fuers schreiben von Tabellen
//---------------------------------------------------------------------------

#define COLFUZZY 20
#define ROWFUZZY 20
#define COL_DFLT_WIDTH ((2*COLFUZZY)+1)
#define ROW_DFLT_HEIGHT (2*ROWFUZZY)+1


//-----------------------------------------------------------------------




//-----------------------------------------------------------------------






//-----------------------------------------------------------------------

class SwWriteTableCol
{
    USHORT nPos;						// End Position der Spalte

    USHORT nWidthOpt;

    BOOL bRelWidthOpt : 1;
    BOOL bOutWidth : 1;					// Spaltenbreite ausgeben?

public:
    BOOL bLeftBorder : 1;				// Welche Umrandungen sind da?
    BOOL bRightBorder : 1;

    SwWriteTableCol( USHORT nPosition );

    USHORT GetPos() const 						{ return nPos; }

    void SetLeftBorder( BOOL bBorder ) 			{ bLeftBorder = bBorder; }
    BOOL HasLeftBorder() const 					{ return bLeftBorder; }

    void SetRightBorder( BOOL bBorder )			{ bRightBorder = bBorder; }
    BOOL HasRightBorder() const					{ return bRightBorder; }

    void SetOutWidth( BOOL bSet ) 				{ bOutWidth = bSet; }
    BOOL GetOutWidth() const 					{ return bOutWidth; }

    inline int operator==( const SwWriteTableCol& rCol ) const;
    inline int operator<( const SwWriteTableCol& rCol ) const;

    void SetWidthOpt( USHORT nWidth, BOOL bRel )
    {
        nWidthOpt = nWidth; bRelWidthOpt = bRel;
    }
    USHORT GetWidthOpt() const					{ return nWidthOpt; }
    BOOL HasRelWidthOpt() const 				{ return bRelWidthOpt; }
};

inline int SwWriteTableCol::operator==( const SwWriteTableCol& rCol ) const
{
    // etwas Unschaerfe zulassen
    return (nPos >= rCol.nPos ? nPos - rCol.nPos
                                     : rCol.nPos - nPos ) <= COLFUZZY;
}

inline int SwWriteTableCol::operator<( const SwWriteTableCol& rCol ) const
{
    // Da wir hier nur die Wahrheits-Grade 0 und 1 kennen, lassen wir lieber
    // auch nicht zu, dass x==y und x<y gleichzeitig gilt ;-)
    return nPos < rCol.nPos - COLFUZZY;
}


typedef SwWriteTableCol *SwWriteTableColPtr;
SV_DECL_PTRARR_SORT_DEL( SwWriteTableCols, SwWriteTableColPtr, 5, 5 )

//-----------------------------------------------------------------------

class SwWriteTable
{








public:
    static long GetBoxWidth( const SwTableBox *pBox );






};




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
