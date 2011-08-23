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

#ifndef _PVPRTDAT_HXX
#define _PVPRTDAT_HXX


#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

class SwPagePreViewPrtData
{
    ULONG nLeftSpace, nRightSpace, nTopSpace, nBottomSpace,
            nHorzSpace, nVertSpace;
    BYTE nRow, nCol;
    BOOL bLandscape : 1;
    BOOL bStretch : 1;
public:
    SwPagePreViewPrtData()
        : nLeftSpace(0), nRightSpace(0), nTopSpace(0), nBottomSpace(0),
            nHorzSpace(0), nVertSpace(0), nRow(1), nCol(1),
            bLandscape(0),bStretch(0)
    {}

    ULONG GetLeftSpace() const 			{ return nLeftSpace; }
    void SetLeftSpace( ULONG n ) 		{ nLeftSpace = n; }

    ULONG GetRightSpace() const 		{ return nRightSpace; }
    void SetRightSpace( ULONG n ) 		{ nRightSpace = n; }

    ULONG GetTopSpace() const 			{ return nTopSpace; }
    void SetTopSpace( ULONG n ) 		{ nTopSpace = n; }

    ULONG GetBottomSpace() const 		{ return nBottomSpace; }
    void SetBottomSpace( ULONG n ) 		{ nBottomSpace = n; }

    ULONG GetHorzSpace() const 			{ return nHorzSpace; }
    void SetHorzSpace( ULONG n ) 		{ nHorzSpace = n; }

    ULONG GetVertSpace() const 			{ return nVertSpace; }
    void SetVertSpace( ULONG n ) 		{ nVertSpace = n; }

    BYTE GetRow() const 				{ return nRow; }
    void SetRow(BYTE n ) 				{ nRow = n; }

    BYTE GetCol() const 				{ return nCol; }
    void SetCol( BYTE n ) 				{ nCol = n; }

    BOOL GetLandscape() const 			{ return bLandscape; }
    void SetLandscape( BOOL b ) 		{ bLandscape = b; }

    // JP 19.08.98: wird zur Zeit nicht ausgewertet, weil der Font sich
    //				nicht enstprechend stretch laesst.
    BOOL GetStretch() const 			{ return bStretch; }
    void SetStretch( BOOL b ) 			{ bStretch = b; }
};


} //namespace binfilter
#endif


