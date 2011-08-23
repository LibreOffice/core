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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "sortparam.hxx"

namespace binfilter {


//------------------------------------------------------------------------

/*N*/ ScSortParam::ScSortParam()
/*N*/ {
/*N*/ 	Clear();
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSortParam::ScSortParam( const ScSortParam& r ) :
/*N*/ 		nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),
/*N*/ 		bHasHeader(r.bHasHeader),bCaseSens(r.bCaseSens),
/*N*/ 		bByRow(r.bByRow),bUserDef(r.bUserDef),nUserIndex(r.nUserIndex),bIncludePattern(r.bIncludePattern),
/*N*/ 		bInplace(r.bInplace),
/*N*/ 		nDestTab(r.nDestTab),nDestCol(r.nDestCol),nDestRow(r.nDestRow),
/*N*/ 		aCollatorLocale( r.aCollatorLocale ), aCollatorAlgorithm( r.aCollatorAlgorithm )
/*N*/ {
/*N*/ 	for (USHORT i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		bDoSort[i]	  = r.bDoSort[i];
/*N*/ 		nField[i]	  = r.nField[i];
/*N*/ 		bAscending[i] = r.bAscending[i];
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void ScSortParam::Clear()
/*N*/ {
/*N*/ 	nCol1=nRow1=nCol2=nRow2=nDestTab=nDestCol=nDestRow=nUserIndex = 0;
/*N*/ 	bHasHeader=bCaseSens=bUserDef = FALSE;
/*N*/ 	bByRow=bIncludePattern=bInplace	= TRUE;
/*N*/ 	aCollatorLocale = ::com::sun::star::lang::Locale();
/*N*/ 	aCollatorAlgorithm.Erase();
/*N*/ 
/*N*/ 	for (USHORT i=0; i<MAXSORT; i++)
/*N*/ 	{
/*N*/ 		bDoSort[i]	  = FALSE;
/*N*/ 		nField[i]	  = 0;
/*N*/ 		bAscending[i] = TRUE;
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
