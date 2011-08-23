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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "detdata.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
namespace binfilter {

//------------------------------------------------------------------------

/*N*/ SV_IMPL_PTRARR( ScDetOpArr_Impl, ScDetOpDataPtr );

//------------------------------------------------------------------------

/*N*/ ScDetOpList::ScDetOpList(const ScDetOpList& rList) :
/*N*/ 	bHasAddError( FALSE )
/*N*/ {
/*N*/ 	USHORT nCount = rList.Count();
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 		Append( new ScDetOpData(*rList[i]) );
/*N*/ }

/*N*/ void ScDetOpList::UpdateReference( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
/*N*/ 								const ScRange& rRange, short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	USHORT nCount = Count();
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		ScAddress aPos = (*this)[i]->GetPos();
/*N*/ 		USHORT nCol1 = aPos.Col();
/*N*/ 		USHORT nRow1 = aPos.Row();
/*N*/ 		USHORT nTab1 = aPos.Tab();
/*N*/ 		USHORT nCol2 = nCol1;
/*N*/ 		USHORT nRow2 = nRow1;
/*N*/ 		USHORT nTab2 = nTab1;
/*N*/ 
/*N*/ 		ScRefUpdateRes eRes =
/*N*/ 			ScRefUpdate::Update( pDoc, eUpdateRefMode,
/*N*/ 				rRange.aStart.Col(), rRange.aStart.Row(), rRange.aStart.Tab(),
/*N*/ 				rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aEnd.Tab(), nDx, nDy, nDz,
/*N*/ 				nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 		if ( eRes != UR_NOTHING )
/*N*/ 			(*this)[i]->SetPos( ScAddress( nCol1, nRow1, nTab1 ) );
/*N*/ 	}
/*N*/ }

/*N*/ void ScDetOpList::Append( ScDetOpData* pData )
/*N*/ {
/*N*/ 	if ( pData->GetOperation() == SCDETOP_ADDERROR )
/*N*/ 		bHasAddError = TRUE;
/*N*/ 
/*N*/ 	Insert( pData, Count() );
/*N*/ }



/*N*/ void ScDetOpList::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nNewCount;
/*N*/ 	rStream >> nNewCount;
/*N*/ 
/*N*/ 	ScAddress aPos;
/*N*/ 	USHORT nOper;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nNewCount; i++)
/*N*/ 	{
/*N*/ 		//	1) Position (ScAddress)
/*N*/ 		//	2) Operation (USHORT)
/*N*/ 
/*N*/ 		aHdr.StartEntry();
/*N*/ 
/*N*/ 		rStream >> aPos;
/*N*/ 		rStream >> nOper;
/*N*/ 		Append( new ScDetOpData( aPos, (ScDetOpType) nOper ) );
/*N*/ 
/*N*/ 		aHdr.EndEntry();
/*N*/ 	}
/*N*/ }

/*N*/ void ScDetOpList::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	ScMultipleWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ 	USHORT nCount = Count();
/*N*/ 	rStream << nCount;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nCount; i++)
/*N*/ 	{
/*N*/ 		//	1) Position (ScAddress)
/*N*/ 		//	2) Operation (USHORT)
/*N*/ 
/*N*/ 		aHdr.StartEntry();
/*N*/ 
/*N*/ 		ScDetOpData* pData = (*this)[i];
/*N*/ 		rStream << pData->GetPos();
/*N*/ 		rStream << (USHORT) pData->GetOperation();
/*N*/ 
/*N*/ 		aHdr.EndEntry();
/*N*/ 	}
/*N*/ }



}
