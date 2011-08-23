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

#include <hintids.hxx>
#include <txtinet.hxx>
#include <txtatr.hxx>
#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>        // SwCharFmt, SwTxtNode
#include <hints.hxx>        // SwCharFmt, SwUpdateAttr
#include <poolfmt.hxx>		// RES_POOLCHR_INET_...

#include <horiornt.hxx>

#include <doc.hxx>			// SwDoc
namespace binfilter {


/*************************************************************************
 *						class SwTxtHardBlank
 *************************************************************************/



/*************************************************************************
 *						class SwTxtCharFmt
 *************************************************************************/

/*N*/ SwTxtCharFmt::SwTxtCharFmt( const SwFmtCharFmt& rAttr,
/*N*/ 					xub_StrLen nStartIn, xub_StrLen nEndIn )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStartIn, nEndIn ),
/*N*/ 	pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	((SwFmtCharFmt&)rAttr).pTxtAttr = this;
/*N*/ 	SetCharFmtAttr( TRUE );
/*N*/ }

/*N*/ SwTxtCharFmt::~SwTxtCharFmt( )
/*N*/ {
/*N*/ }

/*N*/ void SwTxtCharFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
/*N*/ 			&& (nWhich!=RES_OBJECTDYING)
/*N*/ 			&& (nWhich!=RES_ATTRSET_CHG)
/*N*/ 			&& (nWhich!=RES_FMT_CHG) )
/*N*/ 		ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
/*N*/ #endif
/*N*/ 
/*N*/ 	if( pMyTxtNd )
/*N*/ 	{
/*N*/ 		SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
/*N*/ 		pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
/*N*/ 	}
/*N*/ }

    // erfrage vom Modify Informationen
/*N*/ BOOL SwTxtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
/*N*/ 		&pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
 *						class SwTxtINetFmt
 *************************************************************************/

/*N*/ SwTxtINetFmt::SwTxtINetFmt( const SwFmtINetFmt& rAttr,
/*N*/ 							xub_StrLen nStartIn, xub_StrLen nEndIn )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStartIn, nEndIn ),
/*N*/ 	SwClient( 0 ),
/*N*/     pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	bValidVis = FALSE;
/*N*/ 	((SwFmtINetFmt&)rAttr).pTxtAttr  = this;
/*N*/ 	SetCharFmtAttr( TRUE );
/*N*/ }

/*N*/ SwTxtINetFmt::~SwTxtINetFmt( )
/*N*/ {
/*N*/ }

/*N*/ SwCharFmt* SwTxtINetFmt::GetCharFmt()
/*N*/ {
/*N*/ 	const SwFmtINetFmt& rFmt = SwTxtAttrEnd::GetINetFmt();
/*N*/ 	SwCharFmt* pRet = NULL;
/*N*/ 
/*N*/ 	if( rFmt.GetValue().Len() )
/*N*/ 	{
/*N*/ 		const SwDoc* pDoc = GetTxtNode().GetDoc();
/*N*/ 		if( !IsValidVis() )
/*N*/ 		{
/*N*/ 			SetVisited( pDoc->IsVisitedURL( rFmt.GetValue() ) );
/*N*/ 			SetValidVis( TRUE );
/*N*/ 		}
/*N*/ 		USHORT nId;
/*N*/ 		const String& rStr = IsVisited() ? rFmt.GetVisitedFmt()
/*N*/ 										   : rFmt.GetINetFmt();
/*N*/ 		if( rStr.Len() )
/*N*/ 			nId = IsVisited() ? rFmt.GetVisitedFmtId() : rFmt.GetINetFmtId();
/*N*/ 		else
/*N*/ 			nId = IsVisited() ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL;
/*N*/ 
/*N*/ 		// JP 10.02.2000, Bug 72806: dont modify the doc for getting the
/*N*/ 		//		correct charstyle.
/*N*/ 		BOOL bResetMod = !pDoc->IsModified();
/*N*/ 		Link aOle2Lnk;
/*N*/ 		if( bResetMod )
/*N*/ 		{
/*N*/ 			aOle2Lnk = pDoc->GetOle2Link();
/*N*/ 			((SwDoc*)pDoc)->SetOle2Link( Link() );
/*N*/ 		}
/*N*/ 
/*N*/ 		pRet = IsPoolUserFmt( nId )
/*N*/ 				? ((SwDoc*)pDoc)->FindCharFmtByName( rStr )
/*N*/ 				: ((SwDoc*)pDoc)->GetCharFmtFromPool( nId );
/*N*/ 
/*N*/ 		if( bResetMod )
/*N*/ 		{
/*N*/ 			((SwDoc*)pDoc)->ResetModified();
/*N*/ 			((SwDoc*)pDoc)->SetOle2Link( aOle2Lnk );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pRet )
/*N*/ 		pRet->Add( this );
/*N*/ 	else if( GetRegisteredIn() )
/*N*/ 		pRegisteredIn->Remove( this );
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void SwTxtINetFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if ( (nWhich<RES_CHRATR_BEGIN || nWhich>RES_CHRATR_END)
/*N*/ 			&& (nWhich!=RES_OBJECTDYING)
/*N*/ 			&& (nWhich!=RES_ATTRSET_CHG)
/*N*/ 			&& (nWhich!=RES_FMT_CHG) )
/*N*/ 		ASSERT(!this, "SwTxtCharFmt::Modify(): unbekanntes Modify!");
/*N*/ #endif
/*N*/ 
/*N*/ 	if( pMyTxtNd )
/*N*/ 	{
/*N*/ 		SwUpdateAttr aUpdateAttr( *GetStart(), *GetEnd(), nWhich );
/*N*/ 		pMyTxtNd->SwCntntNode::Modify( &aUpdateAttr, &aUpdateAttr );
/*N*/ 	}
/*N*/ }

    // erfrage vom Modify Informationen
/*N*/ BOOL SwTxtINetFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	if( RES_AUTOFMT_DOCNODE != rInfo.Which() || !pMyTxtNd ||
/*N*/ 		&pMyTxtNd->GetNodes() != ((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	((SwAutoFmtGetDocNode&)rInfo).pCntntNode = pMyTxtNd;
/*N*/ 	return FALSE;
/*N*/ }


// ATT_XNLCONTAINERITEM ******************************




// ******************************






// ******************************


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
