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

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "prnsave.hxx"
#include "global.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------

//
//		Daten pro Tabelle
//

/*N*/ ScPrintSaverTab::ScPrintSaverTab() :
/*N*/ 	nPrintCount(0),
/*N*/ 	pPrintRanges(NULL),
/*N*/ 	pRepeatCol(NULL),
/*N*/ 	pRepeatRow(NULL)
/*N*/ {
/*N*/ }

/*N*/ ScPrintSaverTab::~ScPrintSaverTab()
/*N*/ {
/*N*/ 	delete[] pPrintRanges;
/*N*/ 	delete pRepeatCol;
/*N*/ 	delete pRepeatRow;
/*N*/ }

/*N*/ void ScPrintSaverTab::SetAreas( USHORT nCount, const ScRange* pRanges )
/*N*/ {
/*N*/ 	delete[] pPrintRanges;
/*N*/ 	if (nCount && pRanges)
/*N*/ 	{
/*N*/ 		nPrintCount = nCount;
/*N*/ 		pPrintRanges = new ScRange[nCount];
/*N*/ 		for (USHORT i=0; i<nCount; i++)
/*N*/ 			pPrintRanges[i] = pRanges[i];
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nPrintCount = 0;
/*N*/ 		pPrintRanges = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ void ScPrintSaverTab::SetRepeat( const ScRange* pCol, const ScRange* pRow )
/*N*/ {
/*N*/ 	delete pRepeatCol;
/*N*/ 	pRepeatCol = pCol ? new ScRange(*pCol) : NULL;
/*N*/ 	delete pRepeatRow;
/*N*/ 	pRepeatRow = pRow ? new ScRange(*pRow) : NULL;
/*N*/ }



//
//		Daten fuer das ganze Dokument
//

/*N*/ ScPrintRangeSaver::ScPrintRangeSaver( USHORT nCount ) :
/*N*/ 	nTabCount( nCount )
/*N*/ {
/*N*/ 	if (nCount)
/*N*/ 		pData = new ScPrintSaverTab[nCount];
/*N*/ 	else
/*N*/ 		pData = NULL;
/*N*/ }

/*N*/ ScPrintRangeSaver::~ScPrintRangeSaver()
/*N*/ {
/*N*/ 	delete[] pData;
/*N*/ }

/*N*/ ScPrintSaverTab& ScPrintRangeSaver::GetTabData(USHORT nTab)
/*N*/ {
/*N*/ 	DBG_ASSERT(nTab<nTabCount,"ScPrintRangeSaver Tab zu gross");
/*N*/ 	return pData[nTab];
/*N*/ }






}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
