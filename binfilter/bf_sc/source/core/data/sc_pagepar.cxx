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

// INCLUDE ---------------------------------------------------------------

// System - Includes -----------------------------------------------------

#ifdef PCH
#endif


#include <string.h>

#include "pagepar.hxx"
namespace binfilter {


//========================================================================
// struct ScPageTableParam:

/*N*/ ScPageTableParam::ScPageTableParam()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScPageTableParam::ScPageTableParam( const ScPageTableParam& r )
/*N*/ {
/*N*/ 	*this = r;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScPageTableParam::~ScPageTableParam()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScPageTableParam::Reset()
/*N*/ {
/*N*/ 	bNotes=bGrid=bHeaders=bDrawings=
/*N*/ 	bLeftRight=bScaleAll=bScalePageNum=
/*N*/ 	bFormulas=bNullVals=bSkipEmpty			= FALSE;
/*N*/ 	bTopDown=bScaleNone=bCharts=bObjects	= TRUE;
/*N*/ 	nScaleAll		= 100;
/*N*/ 	nScalePageNum	= 0;
/*N*/ 	nFirstPageNo	= 1;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//========================================================================
// struct ScPageAreaParam:

/*N*/ ScPageAreaParam::ScPageAreaParam()
/*N*/ {
/*N*/ 	Reset();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScPageAreaParam::ScPageAreaParam( const ScPageAreaParam& r )
/*N*/ {
/*N*/ 	*this = r;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScPageAreaParam::~ScPageAreaParam()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void __EXPORT ScPageAreaParam::Reset()
/*N*/ {
/*N*/ 	bPrintArea = bRepeatRow = bRepeatCol = FALSE;
/*N*/ 
/*N*/ 	memset( &aPrintArea, 0, sizeof(ScRange) );
/*N*/ 	memset( &aRepeatRow, 0, sizeof(ScRange) );
/*N*/ 	memset( &aRepeatCol, 0, sizeof(ScRange) );
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

}
