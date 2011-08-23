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

// System - Includes -----------------------------------------------------

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifdef _MSC_VER
#pragma optimize("",off)
#endif
                                        // sonst Absturz Win beim Fuellen

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
//#include <math.h>

#include "globstr.hrc"
#include "document.hxx"
#include "autoform.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

#define _D_MAX_LONG_  (double) 0x7fffffff

extern USHORT nScFillModeMouseModifier;		// global.cxx

// -----------------------------------------------------------------------














#define LF_LEFT         1
#define LF_TOP          2
#define LF_RIGHT        4
#define LF_BOTTOM       8
#define LF_ALL          (LF_LEFT | LF_TOP | LF_RIGHT | LF_BOTTOM)



/*N*/ void ScTable::SetError( USHORT nCol, USHORT nRow, USHORT nError)
/*N*/ {
/*N*/ 	if (ValidColRow(nCol, nRow))
/*N*/ 		aCol[nCol].SetError( nRow, nError );
/*N*/ }





/*N*/ BOOL ScTable::TestTabRefAbs(USHORT nTable)
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	for (USHORT i=0; i <= MAXCOL; i++)
/*N*/ 		if (aCol[i].TestTabRefAbs(nTable))
/*N*/ 			bRet = TRUE;
/*N*/ 	return bRet;
/*N*/ }


/*N*/ void ScTable::CompileDBFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++) aCol[i].CompileDBFormula( bCreateFormulaString );
/*N*/ }

/*N*/ void ScTable::CompileNameFormula( BOOL bCreateFormulaString )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++) aCol[i].CompileNameFormula( bCreateFormulaString );
/*N*/ }

/*N*/ void ScTable::CompileColRowNameFormula()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<=MAXCOL; i++) aCol[i].CompileColRowNameFormula();
/*N*/ }






}
