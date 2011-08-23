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


#include <horiornt.hxx>

#include <bf_sfx2/docfile.hxx>
#include <bf_svtools/inethist.hxx>

#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <doc.hxx>
#include <visiturl.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <viscrs.hxx>
#include <editsh.hxx>
#include <docsh.hxx>
namespace binfilter {


/*N*/ SwURLStateChanged::SwURLStateChanged( const SwDoc* pD )
/*N*/ 	: pDoc( pD )
/*N*/ {
/*N*/ 	StartListening( *INetURLHistory::GetOrCreate() );
/*N*/ }

/*N*/ SwURLStateChanged::~SwURLStateChanged()
/*N*/ {
/*N*/ 	EndListening( *INetURLHistory::GetOrCreate() );
/*N*/ }

/*N*/ void SwURLStateChanged::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*N*/ {
/*N*/ 	if( rHint.ISA( INetURLHistoryHint ) && pDoc->GetRootFrm() )
/*N*/ 	{
/*N*/ 		// diese URL wurde veraendert:
/*N*/ 		const INetURLObject* pIURL = ((INetURLHistoryHint&)rHint).GetObject();
/*N*/ 		String sURL( pIURL->GetMainURL( INetURLObject::NO_DECODE ) ), sBkmk;
/*N*/ 
/*N*/ 		SwEditShell* pESh = pDoc->GetEditShell();
/*N*/ 
/*N*/ 		if( pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() &&
/*N*/ 			// falls das unser Doc ist, kann es auch lokale Spruenge geben!
/*N*/ 			sURL == pDoc->GetDocShell()->GetMedium()->GetName() )
/*?*/ 			(sBkmk = pIURL->GetMark()).Insert( INET_MARK_TOKEN, 0 );
/*N*/ 
/*N*/ 		BOOL bAction = FALSE, bUnLockView = FALSE;
/*N*/ 		const SwFmtINetFmt* pItem;
/*N*/ 		const SwTxtINetFmt* pTxtAttr;
/*N*/ 		const SwTxtNode* pTxtNd;
/*N*/ 		USHORT n, nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_INETFMT );
/*N*/ 		for( n = 0; n < nMaxItems; ++n )
/*N*/ 			if( 0 != (pItem = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem(
/*N*/ 				RES_TXTATR_INETFMT, n ) ) &&
/*N*/ 				( pItem->GetValue() == sURL ||
/*N*/ 					( sBkmk.Len() && pItem->GetValue() == sBkmk )) &&
/*N*/ 				0 != ( pTxtAttr = pItem->GetTxtINetFmt()) &&
/*N*/ 				0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) )
/*N*/ 			{
/*?*/ 				if( !bAction && pESh )
/*?*/ 				{
/*?*/ 					pESh->StartAllAction();
/*?*/ 					bAction = TRUE;
/*?*/ 					bUnLockView = !pESh->IsViewLocked();
/*?*/ 					pESh->LockView( TRUE );
/*?*/ 				}
/*?*/ 				((SwTxtINetFmt*)pTxtAttr)->SetValidVis( FALSE );
/*?*/ 				const SwTxtAttr* pAttr = pTxtAttr;
/*?*/ 				SwUpdateAttr aUpdateAttr( *pAttr->GetStart(),
/*?*/ 										  *pAttr->GetEnd(),
/*?*/ 										  RES_FMT_CHG );
/*?*/ 				((SwTxtNode*)pTxtNd)->SwCntntNode::Modify( &aUpdateAttr,
/*?*/ 															&aUpdateAttr );
/*?*/ 			}
/*N*/ 
/*N*/ 		if( bAction )
/*?*/ 			pESh->EndAllAction();
/*N*/  		if( bUnLockView )
/*?*/      		pESh->LockView( FALSE );
/*N*/ 	}
/*N*/ }

    // erfrage ob die URL besucht war. Uebers Doc, falls nur ein Bookmark
    // angegeben ist. Dann muss der Doc. Name davor gesetzt werden!
/*N*/ BOOL SwDoc::IsVisitedURL( const String& rURL ) const
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	static long nTmp = 0;
/*N*/ 	++nTmp;
/*N*/ #endif
/*N*/ 
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if( rURL.Len() )
/*N*/ 	{
/*N*/ 		INetURLHistory *pHist = INetURLHistory::GetOrCreate();
/*N*/ 		if( '#' == rURL.GetChar( 0 ) && pDocShell && pDocShell->GetMedium() )
/*N*/ 		{
/*N*/ 			INetURLObject aIObj( pDocShell->GetMedium()->GetURLObject() );
/*N*/ 			aIObj.SetMark( rURL.Copy( 1 ) );
/*N*/ 			bRet = pHist->QueryUrl( aIObj );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = pHist->QueryUrl( rURL );
/*N*/ 
/*N*/ 		// dann  wollen wird auch ueber Statusaenderungen in der History
/*N*/ 		// informiert werden!
/*N*/ 		if( !pURLStateChgd )
/*N*/ 		{
/*N*/ 			SwDoc* pD = (SwDoc*)this;
/*N*/ 			pD->pURLStateChgd = new SwURLStateChanged( this );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
