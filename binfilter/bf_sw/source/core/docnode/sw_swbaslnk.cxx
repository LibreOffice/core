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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#include <hintids.hxx>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <bf_svx/boxitem.hxx>
#include <bf_svx/svxids.hrc>		// fuer die EventIds
#include <bf_svx/linkmgr.hxx>

#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>

#include <errhdl.hxx>

#include <pam.hxx>
#include <editsh.hxx>
#include <swbaslnk.hxx>
#include <swserv.hxx>
#include <ndgrf.hxx>
#include <hints.hxx>
#include <cntfrm.hxx>
namespace binfilter {

/*N*/ BOOL SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd );

/*N*/ TYPEINIT1( SwBaseLink, ::binfilter::SvBaseLink );

/*N*/ SV_IMPL_REF( SwServerObject )

/*N*/ SwBaseLink::~SwBaseLink()
/*N*/ {
/*N*/ }

/*N*/ void lcl_CallModify( SwGrfNode& rGrfNd, SfxPoolItem& rItem )
/*N*/ {
/*N*/ 	//JP 4.7.2001: call fist all not SwNoTxtFrames, then the SwNoTxtFrames.
/*N*/ 	//				The reason is, that in the SwNoTxtFrames the Graphic
/*N*/ 	//				after a Paint will be swapped out! So all other "behind"
/*N*/ 	//				them havent't a loaded Graphic. - #86501#
/*N*/ 	rGrfNd.LockModify();
/*N*/ 
/*N*/ 	SwClientIter aIter( rGrfNd );
/*N*/ 	for( int n = 0; n < 2; ++n )
/*N*/ 	{
/*N*/ 		SwClient * pLast = aIter.GoStart();
/*N*/ 		if( pLast ) 	// konnte zum Anfang gesprungen werden ??
/*N*/ 		{
/*N*/ 			do {
/*N*/ 				if( (0 == n) ^ ( 0 != pLast->ISA( SwCntntFrm )) )
/*N*/ 					pLast->Modify( &rItem, &rItem );
/*N*/ 			} while( 0 != ( pLast = aIter++ ));
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rGrfNd.UnlockModify();
/*N*/ }


/*N*/ void SwBaseLink::DataChanged( const String& rMimeType,
/*N*/ 							const ::com::sun::star::uno::Any & rValue )
/*N*/ {
/*N*/ 	if( !pCntntNode )
/*N*/ 	{
/*?*/ 		ASSERT(!this, "DataChanged ohne ContentNode" );
/*?*/ 		return ;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDoc* pDoc = pCntntNode->GetDoc();
/*N*/ 	if( pDoc->IsInDtor() || ChkNoDataFlag() || bIgnoreDataChanged )
/*N*/ 	{
/*?*/ 		bIgnoreDataChanged = FALSE;
/*?*/ 		return ;
/*N*/ 	}
/*N*/ 
/*N*/ 	ULONG nFmt = SotExchange::GetFormatIdFromMimeType( rMimeType );
/*N*/ 
/*N*/ 	if( pCntntNode->IsNoTxtNode() &&
/*N*/ 		nFmt == SvxLinkManager::RegisterStatusInfoId() )
/*N*/ 	{
/*?*/ 		// nur eine Statusaenderung - Events bedienen ?
/*?*/ 		::rtl::OUString sState;
/*?*/ 		if( rValue.hasValue() && ( rValue >>= sState ))
/*?*/ 		{
/*?*/ 			USHORT nEvent = 0;
/*?*/ 			switch( sState.toInt32() )
/*?*/ 			{
/*?*/ 			case STATE_LOAD_OK:		nEvent = SVX_EVENT_IMAGE_LOAD;	break;
/*?*/ 			case STATE_LOAD_ERROR: 	nEvent = SVX_EVENT_IMAGE_ERROR;	break;
/*?*/ 			case STATE_LOAD_ABORT: 	nEvent = SVX_EVENT_IMAGE_ABORT;	break;
/*?*/ 			}
/*?*/ 
/*?*/ 			SwFrmFmt* pFmt;
/*?*/ 			if( nEvent && 0 != ( pFmt = pCntntNode->GetFlyFmt() ))
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwCallMouseEvent aCallEvent;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		return;			// das wars!
/*N*/ 	}
/*N*/ 
/*N*/ 	FASTBOOL bUpdate = FALSE;
/*N*/ 	FASTBOOL bGraphicArrived = FALSE;
/*N*/ 	FASTBOOL bGraphicPieceArrived = FALSE;
/*N*/ 	FASTBOOL bDontNotify = FALSE;
/*N*/ 	Size aGrfSz, aFrmFmtSz;
/*N*/ 
/*N*/ 	if( pCntntNode->IsGrfNode() )
/*N*/ 	{
/*N*/ 		BfGraphicObject& rGrfObj = ((SwGrfNode*)pCntntNode)->GetGrfObj();
/*N*/ 
/*N*/ 		bDontNotify = ((SwGrfNode*)pCntntNode)->IsFrameInPaint();
/*N*/ 
/*N*/ 		bGraphicArrived = GetObj()->IsDataComplete();
/*N*/ 		bGraphicPieceArrived = GetObj()->IsPending();
/*N*/ 		((SwGrfNode*)pCntntNode)->SetGrafikArrived( bGraphicArrived );
/*N*/ 
/*N*/ 		Graphic aGrf;
/*N*/ 		if( SvxLinkManager::GetGraphicFromAny( rMimeType, rValue, aGrf ) &&
/*N*/ 			( GRAPHIC_DEFAULT != aGrf.GetType() ||
/*N*/ 			  GRAPHIC_DEFAULT != rGrfObj.GetType() ) )
/*N*/ 		{
/*N*/ 			aGrfSz = ::binfilter::GetGraphicSizeTwip( aGrf, 0 );
/*N*/ 			if( static_cast< const SwGrfNode * >( pCntntNode )->IsChgTwipSizeFromPixel() )
/*N*/ 			{
/*N*/ 				const MapMode aMapTwip( MAP_TWIP );
/*N*/ 				aFrmFmtSz =
/*N*/ 					Application::GetDefaultDevice()->PixelToLogic(
/*N*/ 						aGrf.GetSizePixel(), aMapTwip );
/*N*/ 
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				aFrmFmtSz = aGrfSz;
/*N*/ 			}
/*N*/ 			Size aSz( ((SwGrfNode*)pCntntNode)->GetTwipSize() );
/*N*/ 
/*N*/ 			if( bGraphicPieceArrived && GRAPHIC_DEFAULT != aGrf.GetType() &&
/*N*/ 				( !aSz.Width() || !aSz.Height() ) )
/*N*/ 			{
/*N*/ 				// wenn nur ein Teil ankommt, aber die Groesse nicht
/*N*/ 				// gesetzt ist, dann muss "unten" der Teil von
/*N*/ 				// bGraphicArrived durchlaufen werden!
/*N*/ 				// (ansonten wird die Grafik in deft. Size gepaintet)
/*N*/ 				bGraphicArrived = TRUE;
/*N*/ 				bGraphicPieceArrived = FALSE;
/*N*/ 			}
/*N*/ 
/*N*/ 			rGrfObj.SetGraphic( aGrf, rGrfObj.GetLink() );
/*N*/ 			bUpdate = TRUE;
/*N*/ 
/*N*/ 			// Bug 33999: damit der Node den Transparent-Status
/*N*/ 			//		richtig gesetzt hat, ohne auf die Grafik
/*N*/ 			//		zugreifen zu muessen (sonst erfolgt ein SwapIn!).
/*N*/ 			if( bGraphicArrived )
/*N*/ 			{
/*N*/ 				// Bug #34735#: immer mit der korrekten Grafik-Size
/*N*/ 				//				arbeiten
/*N*/ 				if( aGrfSz.Height() && aGrfSz.Width() &&
/*N*/ 					aSz.Height() && aSz.Width() &&
/*N*/ 					aGrfSz != aSz )
/*?*/ 					((SwGrfNode*)pCntntNode)->SetTwipSize( aGrfSz );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bUpdate && !bGraphicArrived && !bGraphicPieceArrived )
/*?*/ 			((SwGrfNode*)pCntntNode)->SetTwipSize( Size(0,0) );
/*N*/ 	}
/*N*/ 	else if( pCntntNode->IsOLENode() )
/*?*/ 		bUpdate = TRUE;
/*N*/ 
/*N*/ 	ViewShell *pSh = 0;
/*N*/ 	SwEditShell* pESh = pDoc->GetEditShell( &pSh );
/*N*/ 
/*N*/ 	if ( bUpdate && bGraphicPieceArrived && !(bSwapIn || bDontNotify) )
/*N*/ 	{
/*?*/ 		//Hint ohne Actions verschicken, loest direktes Paint aus.
/*?*/ 		if ( (!pSh || !pSh->ActionPend()) && (!pESh || !pESh->ActionPend()) )
/*?*/ 		{
/*?*/ 			SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
/*?*/ 			pCntntNode->Modify( &aMsgHint, &aMsgHint );
/*?*/ 			bUpdate = FALSE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	static BOOL bInNotifyLinks = FALSE;
/*N*/ 	if( bUpdate && !bDontNotify && (!bSwapIn || bGraphicArrived) &&
/*N*/ 		!bInNotifyLinks)
/*N*/ 	{
/*N*/ 		BOOL bLockView = FALSE;
/*N*/ 		if( pSh )
/*N*/ 		{
/*N*/ 			bLockView = pSh->IsViewLocked();
/*N*/ 			pSh->LockView( TRUE );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pESh )
/*N*/ 			pESh->StartAllAction();
/*N*/ 		else if( pSh )
/*?*/ 			pSh->StartAction();
/*N*/ 
/*N*/ 		SwMsgPoolItem aMsgHint( bGraphicArrived ? RES_GRAPHIC_ARRIVED :
/*N*/ 												  RES_UPDATE_ATTR );
/*N*/ 
/*N*/ 		if ( bGraphicArrived )
/*N*/ 		{
/*N*/ 			//Alle benachrichtigen, die am gleichen Link horchen.
/*N*/ 			bInNotifyLinks = TRUE;
/*N*/ 
/*N*/ 			const ::binfilter::SvBaseLinks& rLnks = pDoc->GetLinkManager().GetLinks();
/*N*/ 			for( USHORT n = rLnks.Count(); n; )
/*N*/ 			{
/*N*/ 				::binfilter::SvBaseLink* pLnk = &(*rLnks[ --n ]);
/*N*/ 				if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
/*N*/ 					pLnk->ISA( SwBaseLink ) && pLnk->GetObj() == GetObj() )
/*N*/ 				{
/*N*/ 					SwBaseLink* pBLink = (SwBaseLink*)pLnk;
/*N*/ 					SwGrfNode* pGrfNd = (SwGrfNode*)pBLink->pCntntNode;
/*N*/ 
/*N*/ 					if( pBLink != this &&
/*N*/ 						( !bSwapIn ||
/*N*/ 							GRAPHIC_DEFAULT == pGrfNd->GetGrfObj().GetType()))
/*N*/ 					{
/*?*/ 						pBLink->bIgnoreDataChanged = FALSE;
/*?*/ 						pBLink->DataChanged( rMimeType, rValue );
/*?*/ 						pBLink->bIgnoreDataChanged = TRUE;
/*?*/ 
/*?*/ 						pGrfNd->SetGrafikArrived( ((SwGrfNode*)pCntntNode)->
/*?*/ 													IsGrafikArrived() );
/*?*/ 
/*?*/ 						// Fly der Grafik anpassen !
/*?*/ 						if( !::binfilter::SetGrfFlySize( aGrfSz, aFrmFmtSz, pGrfNd ) )
/*?*/ 							::binfilter::lcl_CallModify( *pGrfNd, aMsgHint );
/*N*/ 					}
/*N*/ 					else if( pBLink == this &&
/*N*/ 							!::binfilter::SetGrfFlySize( aGrfSz, aFrmFmtSz, pGrfNd ) )
/*N*/ 						// Fly der Grafik anpassen !
/*N*/ 						::binfilter::lcl_CallModify( *pGrfNd, aMsgHint );
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			bInNotifyLinks = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			pCntntNode->Modify( &aMsgHint, &aMsgHint );
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		if( pESh )
/*N*/ 		{
/*N*/ 			const BOOL bEndActionByVirDev = pESh->IsEndActionByVirDev();
/*N*/ 			pESh->SetEndActionByVirDev( TRUE );
/*N*/ 			pESh->EndAllAction();
/*N*/ 			pESh->SetEndActionByVirDev( bEndActionByVirDev );
/*N*/ 		}
/*N*/ 		else if( pSh )
/*?*/ 			pSh->EndAction();
/*N*/ 
/*N*/ 		if( pSh && !bLockView )
/*N*/ 			pSh->LockView( FALSE );
/*N*/ 	}
/*N*/ }

/*N*/ FASTBOOL SwBaseLink::IsShowQuickDrawBmp() const
/*N*/ {
/*N*/ 	return pCntntNode && pCntntNode->IsGrfNode() &&
/*N*/ #ifdef NEW_GRFOBJ
/*N*/ 			((SwGrfNode*)pCntntNode)->HasMagicId()
/*N*/ #else
/*N*/ 			// wie kommt man an die Info dran, das eine Grafik im Cache steht?
/*N*/ 			FALSE
/*N*/ #endif
/*N*/ 		;
/*N*/ }


/*N*/ BOOL SetGrfFlySize( const Size& rGrfSz, const Size& rFrmSz, SwGrfNode* pGrfNd )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	ViewShell *pSh;
/*N*/ 	CurrShell *pCurr = 0;
/*N*/ 	if ( pGrfNd->GetDoc()->GetEditShell( &pSh ) )
/*N*/ 		pCurr = new CurrShell( pSh );
/*N*/ 
/*N*/ 	Size aSz = pGrfNd->GetTwipSize();
/*N*/ 	if ( !(aSz.Width() && aSz.Height()) &&
/*N*/ 			rGrfSz.Width() && rGrfSz.Height() )
/*N*/ 	{
/*N*/ 		SwFrmFmt* pFmt;
/*N*/ 		if( pGrfNd->IsChgTwipSize() &&
/*N*/ 			0 != (pFmt = pGrfNd->GetFlyFmt()) )
/*N*/ 		{
/*?*/ 			Size aCalcSz( aSz );
/*?*/ 			if ( !aSz.Height() && aSz.Width() )
/*?*/ 				//passende Hoehe ausrechnen.
/*?*/ 				aCalcSz.Height() = rFrmSz.Height() *
/*?*/ 						aSz.Width() / rFrmSz.Width();
/*?*/ 			else if ( !aSz.Width() && aSz.Height() )
/*?*/ 				//passende Breite ausrechnen
/*?*/ 				aCalcSz.Width() = rFrmSz.Width() *
/*?*/ 						aSz.Height() / rFrmSz.Height();
/*?*/ 			else
/*?*/ 				//Hoehe und Breite uebernehmen
/*?*/ 				aCalcSz = rFrmSz;
/*?*/ 
/*?*/ 			const SvxBoxItem 	 &rBox = pFmt->GetBox();
/*?*/ 			aCalcSz.Width() += rBox.CalcLineSpace(BOX_LINE_LEFT) +
/*?*/ 							   rBox.CalcLineSpace(BOX_LINE_RIGHT);
/*?*/ 			aCalcSz.Height()+= rBox.CalcLineSpace(BOX_LINE_TOP) +
/*?*/ 							   rBox.CalcLineSpace(BOX_LINE_BOTTOM);
/*?*/ 			const SwFmtFrmSize& rOldAttr = pFmt->GetFrmSize();
/*?*/ 			if( rOldAttr.GetSize() != aCalcSz )
/*?*/ 			{
/*?*/ 				SwFmtFrmSize aAttr( rOldAttr  );
/*?*/ 				aAttr.SetSize( aCalcSz );
/*?*/ 				pFmt->SetAttr( aAttr );
/*?*/ 				bRet = TRUE;
/*?*/ 			}
/*?*/ 
/*?*/ 			if( !aSz.Width() )
/*?*/ 			{
/*?*/ 				// Wenn die Grafik in einer Tabelle verankert ist, muess
/*?*/ 				// die Tabellen-Spalten neu berechnet werden
/*?*/ 				const SwDoc *pDoc = pGrfNd->GetDoc();
/*?*/ 				const SwPosition* pAPos = pFmt->GetAnchor().GetCntntAnchor();
/*?*/ 				SwNode *pANd;
/*?*/ 				SwTableNode *pTblNd;
/*?*/ 				if( pAPos &&
/*?*/ 					0 != (pANd = pDoc->GetNodes()[pAPos->nNode]) &&
/*?*/ 					0 != (pTblNd = pANd->FindTableNode()) )
/*?*/ 				{
/*?*/ 					BOOL bLastGrf = !pTblNd->GetTable().DecGrfsThatResize();
/*?*/ 					SwHTMLTableLayout *pLayout =
/*?*/ 						pTblNd->GetTable().GetHTMLTableLayout();
/*?*/ 					if(	pLayout )
/*?*/ 					{
/*?*/ 						DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nBrowseWidth =
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// SetTwipSize skaliert ggf. eine ImageMap neu und
/*N*/ 		// braucht dazu das Frame-Format
/*N*/ 		pGrfNd->SetTwipSize( rGrfSz );
/*N*/ 	}
/*N*/ 
/*N*/ 	delete pCurr;
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SwBaseLink::SwapIn( BOOL bWaitForData, BOOL bNativFormat )
/*N*/ {
/*N*/ 	bSwapIn = TRUE;
/*N*/ 
/*N*/ 	FASTBOOL bRes;
/*N*/ 
/*N*/ 	if( !GetObj() && ( bNativFormat || ( !IsSynchron() && bWaitForData ) ))
/*N*/ 	{
/*?*/ 		AddNextRef();
/*?*/ 		_GetRealObject();
/*?*/ 		ReleaseRef();
/*N*/ 	}
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	{
/*N*/ 		String sGrfNm;
/*N*/ 		GetLinkManager()->GetDisplayNames( this, 0, &sGrfNm, 0, 0 );
/*N*/ 		int x = 0;
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if( GetObj() )
/*N*/ 	{
/*N*/ 		String aMimeType( SotExchange::GetFormatMimeType( GetContentType() ));
/*N*/ 
/*N*/ //!! ??? what have we here to do ????
/*N*/ //!!		if( bNativFormat )
/*N*/ //!!			aData.SetAspect( aData.GetAspect() | ASPECT_ICON );
/*N*/ 
/*N*/ 		::com::sun::star::uno::Any aValue;
/*N*/ 		GetObj()->GetData( aValue, aMimeType, !IsSynchron() && bWaitForData );
/*N*/ 
/*N*/ 		if( bWaitForData && !GetObj() )
/*N*/ 		{
/*?*/ 			ASSERT( !this, "das SvxFileObject wurde in einem GetData geloescht!" );
/*?*/ 			bRes = FALSE;
/*N*/ 		}
/*N*/ 		else if( 0 != ( bRes = aValue.hasValue() ) )
/*N*/ 		{
/*N*/ 			//JP 14.04.99: Bug 64820 - das Flag muss beim SwapIn natuerlich
/*N*/ 			//				zurueckgesetzt werden. Die Daten sollen ja neu
/*N*/ 			//				uebernommen werden
/*N*/ 			bIgnoreDataChanged = FALSE;
/*N*/ 			DataChanged( aMimeType, aValue );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( !IsSynchron() && bWaitForData )
/*N*/ 	{
/*?*/ 		SetSynchron( TRUE );
/*?*/ 		bRes = Update();
/*?*/ 		SetSynchron( FALSE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bRes = Update();
/*N*/ 
/*N*/ 	bSwapIn = FALSE;
/*N*/ 	return bRes;
/*N*/ }



/*N*/ BOOL SwBaseLink::IsRecursion( const SwBaseLink* pChkLnk ) const
/*N*/ {
/*N*/ 	SwServerObjectRef aRef( (SwServerObject*)GetObj() );
/*N*/ 	if( aRef.Is() )
/*N*/ 	{
/*N*/ 		// es ist ein ServerObject, also frage nach allen darin
/*N*/ 		// befindlichen Links, ob wir darin enthalten sind. Ansonsten
/*N*/ 		// handelt es sich um eine Rekursion.
/*N*/ 		return aRef->IsLinkInServer( pChkLnk );
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ BOOL SwBaseLink::IsInRange( ULONG, ULONG, xub_StrLen, xub_StrLen ) const
/*N*/ {
/*N*/ 	// Grafik oder OLE-Links nicht,
/*N*/ 	// Felder oder Sections haben eigene Ableitung!
/*N*/ 	return FALSE;
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
