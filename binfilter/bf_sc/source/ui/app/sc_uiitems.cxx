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


#include "userlist.hxx"
#include "uiitems.hxx"
#include "dpsave.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

/*N*/ TYPEINIT1(ScSortItem,           SfxPoolItem);
/*N*/ TYPEINIT1(ScQueryItem,          SfxPoolItem);
/*N*/ TYPEINIT1(ScSubTotalItem,       SfxPoolItem);
/*N*/ TYPEINIT1(ScUserListItem,       SfxPoolItem);
/*N*/ TYPEINIT1(ScConsolidateItem,    SfxPoolItem);
/*N*/ TYPEINIT1(ScPivotItem,		    SfxPoolItem);
/*N*/ TYPEINIT1(ScSolveItem,		    SfxPoolItem);

/*N*/ TYPEINIT1(ScTablesHint,			SfxHint);
/*N*/ TYPEINIT1(ScEditViewHint,		SfxHint);
/*N*/ TYPEINIT1(ScIndexHint,			SfxHint);


// -----------------------------------------------------------------------
//	ScTablesHint - Views anpassen, wenn Tabellen eingefuegt / geloescht
// -----------------------------------------------------------------------

/*N*/ ScTablesHint::ScTablesHint(USHORT nNewId, USHORT nTable1, USHORT nTable2) :
/*N*/ 	nId( nNewId ),
/*N*/ 	nTab1( nTable1 ),
/*N*/ 	nTab2( nTable2 )
/*N*/ {
/*N*/ }

/*N*/ ScTablesHint::~ScTablesHint()
/*N*/ {
/*N*/ }



/*N*/ ScIndexHint::~ScIndexHint()
/*N*/ {
/*N*/ }


// -----------------------------------------------------------------------
//      ScEditViewHint - neue EditView fuer Cursorposition anlegen
// -----------------------------------------------------------------------

/*N*/ ScEditViewHint::~ScEditViewHint()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------
//      ScSortItem - Daten fuer den Sortierdialog
// -----------------------------------------------------------------------

/*N*/ ScSortItem::ScSortItem( USHORT              nWhich,
/*N*/ 						const ScSortParam*  pSortData ) :
/*N*/ 		SfxPoolItem ( nWhich ),
/*N*/ 		pViewData   ( NULL )
/*N*/ {
/*N*/ 	if ( pSortData ) theSortData = *pSortData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSortItem::ScSortItem( const ScSortItem& rItem ) :
/*N*/ 		SfxPoolItem ( rItem ),
/*N*/ 		pViewData   ( rItem.pViewData ),
/*N*/ 		theSortData ( rItem.theSortData )
/*N*/ {
/*N*/ }

/*N*/ __EXPORT ScSortItem::~ScSortItem()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


// -----------------------------------------------------------------------
//      ScQueryItem - Daten fuer den Filterdialog
// -----------------------------------------------------------------------

/*N*/ ScQueryItem::ScQueryItem( USHORT                nWhich,
/*N*/ 						  ScViewData*           ptrViewData,
/*N*/ 						  const ScQueryParam*   pQueryData ) :
/*N*/ 		SfxPoolItem ( nWhich ),
/*N*/ 		bIsAdvanced	( FALSE ),
/*N*/ 		pViewData   ( ptrViewData )
/*N*/ {
/*N*/ 	if ( pQueryData ) theQueryData = *pQueryData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScQueryItem::ScQueryItem( const ScQueryItem& rItem ) :
/*N*/ 		SfxPoolItem ( rItem ),
/*N*/ 		bIsAdvanced	( rItem.bIsAdvanced ),
/*N*/ 		aAdvSource	( rItem.aAdvSource ),
/*N*/ 		pViewData   ( rItem.pViewData ),
/*N*/ 		theQueryData( rItem.theQueryData )
/*N*/ {
/*N*/ }

/*N*/ __EXPORT ScQueryItem::~ScQueryItem()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------



//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


// -----------------------------------------------------------------------
//      ScSubTotalItem - Daten fuer den Zwischenergebnisdialog
// -----------------------------------------------------------------------

/*N*/ ScSubTotalItem::ScSubTotalItem( USHORT               	nWhich,
/*N*/ 								ScViewData*           	ptrViewData,
/*N*/ 								const ScSubTotalParam*	pSubTotalData ) :
/*N*/ 		SfxPoolItem ( nWhich ),
/*N*/ 		pViewData   ( ptrViewData )
/*N*/ {
/*N*/ 	if ( pSubTotalData ) theSubTotalData = *pSubTotalData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSubTotalItem::ScSubTotalItem( const ScSubTotalItem& rItem ) :
/*N*/ 		SfxPoolItem		( rItem ),
/*N*/ 		pViewData		( rItem.pViewData ),
/*N*/ 		theSubTotalData	( rItem.theSubTotalData )
/*N*/ {
/*N*/ }

/*N*/ __EXPORT ScSubTotalItem::~ScSubTotalItem()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


// -----------------------------------------------------------------------
//      ScUserListItem - Transporter fuer den Benutzerlisten-TabPage
// -----------------------------------------------------------------------

/*N*/ ScUserListItem::ScUserListItem( USHORT nWhich )
/*N*/ 	:   SfxPoolItem ( nWhich ),
/*N*/ 		pUserList	( NULL )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScUserListItem::ScUserListItem( const ScUserListItem& rItem )
/*N*/ 	:	SfxPoolItem	( rItem )
/*N*/ {
/*N*/ 	if ( rItem.pUserList )
/*N*/ 		pUserList = new ScUserList( *(rItem.pUserList) );
/*N*/ 	else
/*N*/ 		pUserList = NULL;
/*N*/ }

/*N*/ __EXPORT ScUserListItem::~ScUserListItem()
/*N*/ {
/*N*/ 	delete pUserList;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------



//------------------------------------------------------------------------


//------------------------------------------------------------------------


// -----------------------------------------------------------------------
//      ScConsolidateItem - Daten fuer den Konsolidieren-Dialog
// -----------------------------------------------------------------------

/*N*/ ScConsolidateItem::ScConsolidateItem(
/*N*/ 							USHORT						nWhich,
/*N*/ 							const ScConsolidateParam*	pConsolidateData ) :
/*N*/ 		SfxPoolItem ( nWhich )
/*N*/ {
/*N*/ 	if ( pConsolidateData ) theConsData = *pConsolidateData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScConsolidateItem::ScConsolidateItem( const ScConsolidateItem& rItem ) :
/*N*/ 		SfxPoolItem	( rItem ),
/*N*/ 		theConsData	( rItem.theConsData )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScConsolidateItem::~ScConsolidateItem()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------



// -----------------------------------------------------------------------
//      ScPivotItem - Daten fuer den Pivot-Dialog
// -----------------------------------------------------------------------

/*N*/ ScPivotItem::ScPivotItem( USHORT nWhich, const ScDPSaveData* pData,
/*N*/ 							 const ScRange* pRange, BOOL bNew ) :
/*N*/ 		SfxPoolItem ( nWhich )
/*N*/ {
/*N*/ 	//	pSaveData must always exist
/*N*/ 	if ( pData )
/*?*/ 		pSaveData = new ScDPSaveData(*pData);
/*N*/ 	else
/*N*/ 		pSaveData = new ScDPSaveData;
/*N*/ 	if ( pRange ) aDestRange = *pRange;
/*N*/ 	bNewSheet = bNew;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScPivotItem::ScPivotItem( const ScPivotItem& rItem ) :
/*N*/ 		SfxPoolItem	( rItem ),
/*N*/ 		aDestRange	( rItem.aDestRange ),
/*N*/ 		bNewSheet	( rItem.bNewSheet )
/*N*/ {
/*N*/ 	DBG_ASSERT(rItem.pSaveData, "pSaveData");
/*N*/ 	pSaveData = new ScDPSaveData(*rItem.pSaveData);
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScPivotItem::~ScPivotItem()
/*N*/ {
/*N*/ 	delete pSaveData;
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------



// -----------------------------------------------------------------------
//      ScSolveItem - Daten fuer den Solver-Dialog
// -----------------------------------------------------------------------

/*N*/ ScSolveItem::ScSolveItem( USHORT				nWhich,
/*N*/ 						  const ScSolveParam*	pSolveData )
/*N*/ 	:	SfxPoolItem ( nWhich )
/*N*/ {
/*N*/ 	if ( pSolveData ) theSolveData = *pSolveData;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ ScSolveItem::ScSolveItem( const ScSolveItem& rItem )
/*N*/ 	:   SfxPoolItem		( rItem ),
/*N*/ 		theSolveData	( rItem.theSolveData )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------

/*N*/ __EXPORT ScSolveItem::~ScSolveItem()
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------

}
