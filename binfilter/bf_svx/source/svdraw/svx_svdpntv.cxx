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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>

#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svdvmark.hxx"
#include "svdio.hxx"
#include "svdview.hxx"
#include "svdograf.hxx"
#include "svdouno.hxx"



#include <bf_svtools/whiter.hxx>



#include <bf_svtools/style.hxx>




#include "xoutx.hxx"

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ FrameAnimator::FrameAnimator(SdrView& rNewView):
/*N*/ 	rView(rNewView)
/*N*/ {
/*N*/ 	aTim.SetTimeoutHdl(LINK(this,FrameAnimator,Hdl));
/*N*/ 	aTim.SetTimeout(50);
/*N*/ 	pOut=NULL;
/*N*/ }



/*N*/ IMPL_LINK(FrameAnimator,Hdl,AutoTimer*,pTim)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////
//#define	TEST_IAO

/*N*/ SdrViewWinRec::SdrViewWinRec(OutputDevice* pW)
/*N*/ :	pWin(pW),
/*N*/ //	pVDev(NULL),
//STRIP012/*N*/ 	pIAOManager(NULL),
/*N*/ 	bXorVisible(FALSE)
/*N*/ {
/*N*/ 	// is it a window?
/*N*/ 	if(pW && pW->GetOutDevType() == OUTDEV_WINDOW)
/*N*/ 	{
/*N*/ 		// create B2dIAOManager for this window
//STRIP012/*N*/ 		pIAOManager = new B2dIAOManager((Window*)pW);
//STRIP012#ifdef TEST_IAO
//STRIP012/*?*/ 		if(pIAOManager)
//STRIP012/*?*/ 		{
//STRIP012/*?*/ 			// create some demo shapes
//STRIP012/*?*/ 			B2dIAOLine* pLine;
//STRIP012/*?*/ 			B2dIAOMarker* pMarker;
//STRIP012/*?*/ 			B2dIAOBitmapObj* pBitmap;
//STRIP012/*?*/ 			B2dIAOTriangle* pTriangle;
//STRIP012/*?*/ 
//STRIP012/*?*/ 			static BOOL bCreateLines = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateMarkers = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateField = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateSingleMarker = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateBitmap = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateBitmapField = FALSE;
//STRIP012/*?*/ 			static BOOL bCreateTriangle = FALSE;
//STRIP012/*?*/ 
//STRIP012/*?*/ 			if(bCreateLines)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, Point(5000, 5200), Point(5000, 7000));
//STRIP012/*?*/ 				pLine->SetBaseColor(Color(COL_CYAN));
//STRIP012/*?*/ 				pLine = new B2dIAOLineStriped(pIAOManager, Point(5100, 5200), Point(5100, 7000));
//STRIP012/*?*/ 				pLine->SetBaseColor(Color(COL_BLUE));
//STRIP012/*?*/ 				B2dIAOLineTwoColor* p2Line = new B2dIAOLineTwoColor(pIAOManager, Point(5200, 5200), Point(5200, 7000));
//STRIP012/*?*/ 				p2Line->SetBaseColor(Color(COL_YELLOW));
//STRIP012/*?*/ 				p2Line->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 				B2dIAOLineTwoColorAnim* p3Line = new B2dIAOLineTwoColorAnim(pIAOManager, Point(5300, 5200), Point(5300, 7000));
//STRIP012/*?*/ 				p3Line->SetBaseColor(Color(COL_YELLOW));
//STRIP012/*?*/ 				p3Line->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 
//STRIP012/*?*/ 			if(bCreateMarkers)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5000), B2D_IAO_MARKER_POINT);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTRED));
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5100, 5000), B2D_IAO_MARKER_PLUS);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5200, 5000), B2D_IAO_MARKER_CROSS);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTBLUE));
//STRIP012/*?*/ 
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5100), B2D_IAO_MARKER_RECT_5X5);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
//STRIP012/*?*/ 				pMarker->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5100, 5100), B2D_IAO_MARKER_RECT_7X7);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
//STRIP012/*?*/ 				pMarker->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5200, 5100), B2D_IAO_MARKER_RECT_9X9);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
//STRIP012/*?*/ 				pMarker->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 
//STRIP012/*?*/ 			if(bCreateField || bCreateBitmapField)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				static UINT16 nNumX = 10;
//STRIP012/*?*/ 				static UINT16 nNumY = 10;
//STRIP012/*?*/ 				static UINT16 nStart = 2000;
//STRIP012/*?*/ 				static UINT16 nEnd = 16000;
//STRIP012/*?*/ 
//STRIP012/*?*/ 				for(UINT16 a=nStart;a<nEnd;a+=(nEnd-nStart)/nNumX)
//STRIP012/*?*/ 				{
//STRIP012/*?*/ 					for(UINT16 b=nStart;b<nEnd;b+=(nEnd-nStart)/nNumY)
//STRIP012/*?*/ 					{
//STRIP012/*?*/ 						if(bCreateField)
//STRIP012/*?*/ 						{
//STRIP012/*?*/ 							pMarker = new B2dIAOMarker(pIAOManager, Point(a, b), B2D_IAO_MARKER_RECT_7X7);
//STRIP012/*?*/ 							pMarker->SetBaseColor(Color(
//STRIP012/*?*/ 								(((a-nStart)*256L)/(nEnd-nStart)),
//STRIP012/*?*/ 								(((b-nStart)*256L)/(nEnd-nStart)),
//STRIP012/*?*/ 								0x80));
//STRIP012/*?*/ 							pMarker->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 						}
//STRIP012/*?*/ 						if(bCreateBitmapField)
//STRIP012/*?*/ 						{
//STRIP012/*?*/ 							Bitmap aBitmap(Size(10, 10), 8);
//STRIP012/*?*/ 							pBitmap = new B2dIAOBitmapObj(pIAOManager, Point(a, b), aBitmap);
//STRIP012/*?*/ 						}
//STRIP012/*?*/ 					}
//STRIP012/*?*/ 				}
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 			if(bCreateSingleMarker)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				pMarker = new B2dIAOMarker(pIAOManager, Point(5000, 5000), B2D_IAO_MARKER_RECT_7X7);
//STRIP012/*?*/ 				pMarker->SetBaseColor(Color(COL_LIGHTGREEN));
//STRIP012/*?*/ 				pMarker->Set2ndColor(Color(COL_BLACK));
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 			if(bCreateBitmap)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				Bitmap aBitmap(Size(10, 10), 8);
//STRIP012/*?*/ 				pBitmap = new B2dIAOBitmapObj(pIAOManager, Point(6000, 6000), aBitmap);
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 			if(bCreateTriangle)
//STRIP012/*?*/ 			{
//STRIP012/*?*/ 				pTriangle = new B2dIAOTriangle(pIAOManager, Point(5000, 5000), Point(7000, 5000), Point(6000, 7000), Color(COL_YELLOW));
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->GetBasePosition(), pTriangle->Get2ndPosition());
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->Get2ndPosition(), pTriangle->Get3rdPosition());
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->Get3rdPosition(), pTriangle->GetBasePosition());
//STRIP012/*?*/ 
//STRIP012/*?*/ 				pTriangle = new B2dIAOBitmapTriangle(pIAOManager, Point(8000, 5000), Point(10000, 5000), Point(9000, 7000), Color(COL_RED));
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->GetBasePosition(), pTriangle->Get2ndPosition());
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->Get2ndPosition(), pTriangle->Get3rdPosition());
//STRIP012/*?*/ 				pLine = new B2dIAOLine(pIAOManager, pTriangle->Get3rdPosition(), pTriangle->GetBasePosition());
//STRIP012/*?*/ 			}
//STRIP012/*?*/ 		}
//STRIP012#endif
/*N*/ 	}
/*N*/ }

/*N*/ SdrViewWinRec::~SdrViewWinRec()
/*N*/ {
/*N*/ //	if (pVDev!=NULL)
/*N*/ //		delete pVDev;
/*N*/ 
/*N*/ 	// cleanup IAOManager for this window
//STRIP012/*N*/ 	if(pIAOManager)
//STRIP012/*N*/ 		delete pIAOManager;
//STRIP012/*N*/ 	pIAOManager = NULL;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrViewWinList::Clear()
/*N*/ {
/*N*/ 	USHORT nAnz=GetCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		delete GetObject(i);
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ }

/*N*/ USHORT SdrViewWinList::Find(OutputDevice* pW) const
/*N*/ {
/*N*/ 	USHORT nAnz=GetCount();
/*N*/ 	USHORT nRet=SDRVIEWWIN_NOTFOUND;
/*N*/ 	for (USHORT nNum=0; nNum<nAnz && nRet==SDRVIEWWIN_NOTFOUND; nNum++) {
/*N*/ 		if (GetObject(nNum)->pWin==pW) nRet=nNum;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ TYPEINIT1( SvxViewHint, SfxSimpleHint );
/*N*/ 
/*N*/ SvxViewHint::SvxViewHint( ULONG _nId ) :
/*N*/     SfxSimpleHint( _nId )
/*N*/ {
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	@@@@@	@@@@  @@ @@  @@ @@@@@@	@@ @@ @@ @@@@@ @@	@@
//	@@	@@ @@  @@ @@ @@@ @@   @@	@@ @@ @@ @@    @@	@@
//	@@	@@ @@  @@ @@ @@@@@@   @@	@@ @@ @@ @@    @@ @ @@
//	@@@@@  @@@@@@ @@ @@@@@@   @@	@@@@@ @@ @@@@  @@@@@@@
//	@@	   @@  @@ @@ @@ @@@   @@	 @@@  @@ @@    @@@@@@@
//	@@	   @@  @@ @@ @@  @@   @@	 @@@  @@ @@    @@@ @@@
//	@@	   @@  @@ @@ @@  @@   @@	  @   @@ @@@@@ @@	@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT2(SdrPaintView,SfxListener,SfxRepeatTarget);

/*N*/ DBG_NAME(SdrPaintView)

/*N*/ void SdrPaintView::ImpClearVars()
/*N*/ {
/*N*/ 	pXOut=NULL;
/*N*/ 	bForeignXOut=FALSE;
/*N*/ 	pItemBrowser=NULL;
/*N*/ 	bLayerSortedRedraw=FALSE;
/*N*/ 	bPageVisible=TRUE;
/*N*/ 	bPageBorderVisible=TRUE;
/*N*/ 	bBordVisible=TRUE;
/*N*/ 	bGridVisible=TRUE;
/*N*/ 	bGridFront	=FALSE;
/*N*/ 	bHlplVisible=TRUE;
/*N*/ 	bHlplFront	=TRUE;
/*N*/ 	bGlueVisible=FALSE;
/*N*/ 	bGlueVisible2=FALSE;
/*N*/ 	bGlueVisible3=FALSE;
/*N*/ 	bGlueVisible4=FALSE;
/*N*/ 	bSwapAsynchron=FALSE;
/*N*/ 	bPrintPreview=FALSE;
/*N*/ 	bLineDraft=FALSE;
/*N*/ 	bFillDraft=FALSE;
/*N*/ 	bGrafDraft=FALSE;
/*N*/ 	bHideGrafDraft=FALSE;
/*N*/ 	bTextDraft=FALSE;
/*N*/ 	bLineDraftPrn=FALSE;
/*N*/ 	bFillDraftPrn=FALSE;
/*N*/ 	bGrafDraftPrn=FALSE;
/*N*/ 	bTextDraftPrn=FALSE;
/*N*/ 
/*N*/ 	bObjectPaintIgnoresClipping=FALSE;
/*N*/ 
/*N*/ 	eAnimationMode = SDR_ANIMATION_ANIMATE;
/*N*/     bAnimationPause = FALSE;
/*N*/ 
/*N*/ 	nHitTolPix=2;
/*N*/ 	nMinMovPix=3;
/*N*/ 	nHitTolLog=0;
/*N*/ 	nMinMovLog=0;
/*N*/ 	pActualOutDev=NULL;
/*N*/ 
/*N*/ 	bSaveHiddenPages=FALSE;
/*N*/ 	bPageTwice=FALSE;
/*N*/ 	pDragWin=NULL;
/*N*/ 	bRestoreColors=TRUE;
/*N*/ 	pDisabledAttr=NULL;
/*N*/ 	pDefaultStyleSheet=NULL;
/*N*/ 	bEncircle=FALSE;
/*N*/ 	bSomeObjChgdFlag=FALSE;
/*N*/ 
/*N*/ 	bMasterBmp=FALSE;
/*N*/ 	pMasterBmp=NULL;
/*N*/ 	nMasterCacheMode = SDR_MASTERPAGECACHE_DEFAULT;
/*N*/ 
/*N*/     nGraphicManagerDrawMode = GRFMGR_DRAW_STANDARD;
/*N*/ 
/*N*/ 	aComeBackTimer.SetTimeout(1);
/*N*/ 	aComeBackTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpComeBackHdl));
/*N*/ 	aAfterPaintTimer.SetTimeout(1);
/*N*/ 	aAfterPaintTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpAfterPaintHdl));
/*N*/ 	aUserMarkerAnimator.SetTimeout(50);
/*N*/ 	aUserMarkerAnimator.SetTimeoutHdl(LINK(this,SdrPaintView,ImpUserMarkerAnimatorHdl));
/*N*/ 
/*N*/ 	String aNam;	// System::GetUserName() just return an empty string
/*N*/ 
/*N*/ 	if (pMod)
/*N*/ 		SetDefaultStyleSheet(pMod->GetDefaultStyleSheet(), TRUE);
/*N*/ 
/*N*/ 	aNam.ToUpperAscii();
/*N*/ 
/*N*/ 	maGridColor = Color( COL_BLACK );
/*N*/ }


/*N*/ SdrPaintView::SdrPaintView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	aPagV(1024,16,16),
/*N*/ 	aPagHide(1024,16,16),
/*N*/ 	aAni(*(SdrView*)this),
/*N*/ 	aDefaultAttr(pModel1->GetItemPool()),
/*N*/ 	aUserMarkers(1024,16,16)
/*N*/ {
/*N*/ 	DBG_CTOR(SdrPaintView,NULL);
/*N*/ 	pMod=pModel1;
/*N*/ 	ImpClearVars();
/*N*/ 	if (pOut!=NULL) AddWin(pOut);
/*N*/ 	pXOut=new ExtOutputDevice(pOut);
/*N*/ 
/*N*/ 	// Flag zur Visualisierung von Gruppen
/*N*/ 	bVisualizeEnteredGroup = TRUE;
/*N*/ 
/*N*/ 	StartListening( maColorConfig );
/*N*/ 	onChangeColorConfig();
/*N*/ }

/*N*/ SdrPaintView::~SdrPaintView()
/*N*/ {
/*N*/ 	DBG_DTOR(SdrPaintView,NULL);
/*N*/ 	aAfterPaintTimer.Stop();
/*N*/ 
/*N*/ 	EndListening( maColorConfig );
/*N*/ 
/*N*/ 	ClearAll();
/*N*/ 	if (!bForeignXOut && pXOut!=NULL) {
/*N*/ 		delete pXOut;
/*N*/ 	}
/*N*/ 	if (pDisabledAttr!=NULL) {
/*?*/ 		delete pDisabledAttr;
/*N*/ 	}
/*N*/ 	if (pMasterBmp!=NULL) {
/*?*/ 		delete pMasterBmp;
/*N*/ 	}
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if (pItemBrowser!=NULL) {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	delete pItemBrowser;
/*N*/ 	}
/*N*/ #endif
/*N*/ 	USHORT nAnz=ImpGetUserMarkerCount();
/*N*/ 	for (USHORT nNum=0; nNum<nAnz; nNum++) {
/*?*/ 		SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*?*/ 		pUM->pView=NULL; // Weil's mich gleich nichtmehr gibt.
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void __EXPORT SdrPaintView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	BOOL bObjChg=!bSomeObjChgdFlag; // TRUE= auswerten fuer ComeBack-Timer
/*N*/ 	BOOL bMaster=pMasterBmp!=NULL;	// TRUE= auswerten fuer MasterPagePaintCache
/*N*/ 	if (bObjChg || bMaster) {
/*N*/ 		SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
/*N*/ 		if (pSdrHint!=NULL) {
/*N*/ 			SdrHintKind eKind=pSdrHint->GetKind();
/*N*/ 			if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
/*N*/ 				if (bObjChg) {
/*N*/ 					bSomeObjChgdFlag=TRUE;
/*N*/ 					aComeBackTimer.Start();
/*N*/ 				}
/*N*/ 				if (bMaster) {
/*?*/ 					const SdrPage* pPg=pSdrHint->GetPage();
/*?*/ 					if (pPg!=NULL && pPg->IsMasterPage() && pPg->GetPageNum()==pMasterBmp->GetMasterPageNum()) {
/*?*/ 						ReleaseMasterPagePaintCache();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if (eKind==HINT_PAGEORDERCHG) {
/*N*/ 				const SdrPage* pPg=pSdrHint->GetPage();
/*N*/ 				if (!pPg->IsInserted()) { // aha, Seite geloescht: also hiden
/*N*/ 					USHORT nAnz=GetPageViewCount();
/*N*/ 					USHORT nv;
/*N*/ 					for (nv=nAnz; nv>0;) {
/*?*/ 						nv--;
/*?*/ 						SdrPageView* pPV=GetPageViewPvNum(nv);
/*?*/ 						if (pPV->GetPage()==pPg) {
/*?*/ 							HidePagePvNum(nv);
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if (bMaster) ReleaseMasterPagePaintCache();
/*N*/ 			}
/*N*/ 			if (eKind==HINT_PAGECHG) {
/*N*/ 				if (bMaster) ReleaseMasterPagePaintCache();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( rHint.ISA( SfxSimpleHint ) && ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_COLORS_CHANGED )
/*N*/ 	{
/*?*/ 		onChangeColorConfig();
/*?*/ 		InvalidateAllWin();
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ IMPL_LINK_INLINE_START(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)
/*N*/ {
/*N*/ 	if (bSomeObjChgdFlag) {
/*N*/ 		bSomeObjChgdFlag=FALSE;
/*N*/ 		ModelHasChanged();
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK_INLINE_END(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)

////////////////////////////////////////////////////////////////////////////////////////////////////



/*?*/ void SdrPaintView::ImpAsyncPaintDone( const SdrObject* pObj )
/*?*/ {{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 
/*?*/ }

/*N*/ IMPL_LINK(SdrPaintView,ImpAfterPaintHdl,Timer*,pTimer)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }

/*N*/ void SdrPaintView::ModelHasChanged()
/*N*/ {
/*N*/ 	// Auch alle PageViews benachrichtigen
/*N*/ 	USHORT nv;
/*N*/ 	USHORT nAnz;
/*N*/ 	nAnz=GetPageViewCount();
/*N*/ 	for (nv=nAnz; nv>0;) {
/*N*/ 		nv--;
/*N*/ 		SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/ 		if (!pPV->GetPage()->IsInserted()) {
/*?*/ 			HidePage(pPV);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nAnz=GetPageViewCount();
/*N*/ 	for (nv=0; nv<nAnz; nv++) {
/*N*/ 		SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/ 		pPV->ModelHasChanged();
/*N*/ 	}
/*N*/ 	nAnz=GetPageHideCount();
/*N*/ 	for (nv=0; nv<nAnz; nv++) {
/*?*/ 		SdrPageView* pPV=GetPageHidePvNum(nv);
/*?*/ 		pPV->ModelHasChanged();
/*N*/ 	}
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if (pItemBrowser!=NULL) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pItemBrowser->SetDirty();
/*N*/ #endif
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ IMPL_LINK_INLINE_START(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,pTimer)
/*N*/ {
/*N*/ 	USHORT nAnz=ImpGetUserMarkerCount();
/*N*/ 	for (USHORT nNum=0; nNum<nAnz; nNum++) {
/*N*/ 		SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*N*/ 		if (pUM->IsAnimate() && pUM->IsVisible()) {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pUM->DoAnimateOneStep();
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK_INLINE_END(SdrPaintView,ImpUserMarkerAnimatorHdl,AutoTimer*,pTimer)

/*N*/ void SdrPaintView::ImpInsertUserMarker(SdrViewUserMarker* pMarker)
/*N*/ {
/*N*/ 	aUserMarkers.Insert(pMarker);
/*N*/ 	ImpCheckMarkerAnimator();
/*N*/ }
/*N*/ 
/*N*/ void SdrPaintView::ImpRemoveUserMarker(SdrViewUserMarker* pMarker)
/*N*/ {
/*N*/ 	aUserMarkers.Remove(pMarker);
/*N*/ 	ImpCheckMarkerAnimator();
/*N*/ }
/*N*/ 
/*N*/ void SdrPaintView::ImpCheckMarkerAnimator()
/*N*/ {
/*N*/ 	BOOL bNeed=FALSE;
/*N*/ 	USHORT nAnz=ImpGetUserMarkerCount();
/*N*/ 	for (USHORT nNum=0; nNum<nAnz && !bNeed; nNum++) {
/*N*/ 		SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*N*/ 		bNeed=pUM->IsAnimate();
/*N*/ 	}
/*N*/ 	if (bNeed) aUserMarkerAnimator.Start();
/*N*/ 	else aUserMarkerAnimator.Stop();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ BOOL SdrPaintView::IsAction() const
/*N*/ {
/*N*/ 	return IsEncirclement();
/*N*/ }




/*N*/ void SdrPaintView::BrkAction()
/*N*/ {
/*N*/ 	BrkEncirclement();
/*N*/ }


/*N*/ void SdrPaintView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
/*N*/ {
/*N*/ 	if (IsEncirclement() && aDragStat.IsShown()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	USHORT nAnz=ImpGetUserMarkerCount();
/*N*/ 	for (USHORT nNum=0; nNum<nAnz; nNum++) {
/*N*/ 		SdrViewUserMarker* pUM=ImpGetUserMarker(nNum);
/*N*/ 		if (pUM->IsVisible()) {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pUM->Draw(pOut,FALSE,!bRestoreColors); // den 3. Parameter hier noch richtig setzen !!!!!
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::BrkEncirclement()
/*N*/ {
/*N*/ 	if (IsEncirclement()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ }




////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::ShowShownXor(OutputDevice* pOut, BOOL bShow)
/*N*/ {
/*N*/ 	USHORT nAnz=GetWinCount();
/*N*/ 	USHORT nw=0;
/*N*/ 	BOOL bWeiter=TRUE;
/*N*/ 	do {
/*N*/ 		if (pOut!=NULL) {
/*N*/ 			nw=aWinList.Find(pOut);
/*N*/ 			bWeiter=FALSE;
/*N*/ 		}
/*N*/ 		if (nw<nAnz && nw!=SDRVIEWWIN_NOTFOUND) {
/*N*/ 			BOOL bOn=IsShownXorVisibleWinNum(nw);
/*N*/ 			if (bOn!=bShow) {
/*N*/ 				ToggleShownXor(GetWin(nw),NULL);
/*N*/ 				SetShownXorVisible(nw,bShow);
/*N*/ 			}
/*N*/ 		} else bWeiter=FALSE;
/*N*/ 	} while (bWeiter);
/*N*/ }

/*N*/ BOOL SdrPaintView::IsShownXorVisible(OutputDevice* pOut) const
/*N*/ {
/*N*/ 	USHORT nPos=aWinList.Find(pOut);
/*N*/ 	if (nPos!=SDRVIEWWIN_NOTFOUND) {
/*N*/ 		return IsShownXorVisibleWinNum(nPos);
/*N*/ 	} else {
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::ClearPageViews()
/*N*/ {
/*N*/ 	BrkAction();
/*N*/ 	for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
/*N*/ 		SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/ 		InvalidateAllWin(pPV->GetPageRect());
/*N*/ 		delete pPV;
/*N*/ 	}
/*N*/ 	aPagV.Clear();
/*N*/ }

/*N*/ void SdrPaintView::ClearHideViews()
/*N*/ {
/*N*/ 	for (USHORT nh=0; nh<GetPageHideCount(); nh++) {
/*?*/ 		SdrPageView* pPV=GetPageHidePvNum(nh);
/*?*/ 		delete pPV;
/*N*/ 	}
/*N*/ 	aPagHide.Clear();
/*N*/ }

/*N*/ void SdrPaintView::Clear()
/*N*/ {
/*N*/ 	ClearPageViews();
/*N*/ 	ClearHideViews();
/*N*/ }

/*N*/ void SdrPaintView::ClearAll()
/*N*/ {
/*N*/ 	for( void* p = aAsyncPaintList.First(); p; p = aAsyncPaintList.Next() )
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 delete (ImpAsyncStruct*) p;
/*N*/ 
/*N*/ 	aAsyncPaintList.Clear();
/*N*/ 	ClearPageViews();
/*N*/ 	ClearHideViews();
/*N*/     ImpForceSwapOut();
/*N*/ }

/*N*/ SdrPageView* SdrPaintView::ShowPage(SdrPage* pPage, const Point& rOffs)
/*N*/ {
/*N*/ 	SdrPageView* pPV=NULL;
/*N*/ 	if (pPage!=NULL) {
/*N*/ 		SdrPageView* pTmpPV=NULL;
/*N*/ 		if (!bPageTwice) pTmpPV=GetPageView(pPage); // Evtl. jede Seite nur einmal!
/*N*/ 		if (pTmpPV==NULL) {
/*N*/ 			USHORT nPos=GetHiddenPV(pPage);   // War die schon mal da?
/*N*/ 			if (nPos<GetPageHideCount()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 			} else {
/*N*/ 				pPV=new SdrPageView(pPage,rOffs,*((SdrView*)this));
/*N*/ 			}
/*N*/ 			if (pPV!=NULL) {
/*N*/ 				aPagV.Insert(pPV,CONTAINER_APPEND);
/*N*/ 				pPV->Show();
/*N*/ 
/*N*/                 // #110290# Swap out graphics when switching pages
/*N*/                 ImpForceSwapOut();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pPV;
/*N*/ }

/*N*/ void SdrPaintView::HidePage(SdrPageView* pPV)
/*N*/ {
/*N*/ 	if (pPV!=NULL) {
/*N*/ 		ULONG nPos=aPagV.GetPos(pPV);
/*N*/ 		if (nPos!=CONTAINER_ENTRY_NOTFOUND) {
/*N*/ 			aPagV.Remove(nPos);
/*N*/ 			pPV->Hide();
/*N*/ 			if (bSaveHiddenPages) {
/*?*/ 				aPagHide.Insert(pPV,CONTAINER_APPEND);
/*N*/ 			} else {
/*N*/ 				delete pPV;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SdrPaintView::HideAllPages()
/*N*/ {
/*N*/ 	while (GetPageViewCount()>0) HidePagePvNum(0);
/*N*/ }



/*N*/ SdrPageView* SdrPaintView::GetPageView(const SdrPage* pPage) const
/*N*/ {
/*N*/ 	if (pPage==NULL) return NULL;
/*N*/ 	BOOL bWeiter=TRUE;
/*N*/ 	SdrPageView* pPV=NULL;
/*N*/ 	for (USHORT i=0; i<GetPageViewCount() && bWeiter; i++) {
/*?*/ 		pPV=GetPageViewPvNum(i);
/*?*/ 		bWeiter=(pPV->GetPage()!=pPage);
/*N*/ 	}
/*N*/ 	if (bWeiter) return NULL;
/*N*/ 	else return pPV;
/*N*/ }





/*N*/ USHORT SdrPaintView::GetHiddenPV(const SdrPage* pPage) const
/*N*/ {
/*N*/ 	BOOL bWeiter=TRUE;
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<GetPageHideCount() && bWeiter;) {
/*?*/ 		SdrPageView* pPV=GetPageHidePvNum(i);
/*?*/ 		bWeiter=(pPV->GetPage()!=pPage);
/*?*/ 		if (bWeiter) i++;
/*N*/ 	}
/*N*/ 	return i;
/*N*/ }


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::AddWin(OutputDevice* pWin1)
/*N*/ {
/*N*/ 	SdrViewWinRec* pWinRec=new SdrViewWinRec(pWin1);
/*N*/ 	pWinRec->bXorVisible=TRUE; // Normalerweise ist Xor nicht unterdrueckt
/*N*/ 	aWinList.Insert(pWinRec);
/*N*/ 	for (USHORT i=0; i<GetPageViewCount(); i++) {
/*?*/ 		GetPageViewPvNum(i)->AddWin(pWin1);
/*N*/ 	}
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if (pItemBrowser!=NULL) {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pItemBrowser->ForceParent();
/*N*/ #endif
/*N*/ }

/*N*/ void SdrPaintView::DelWin(OutputDevice* pWin1)
/*N*/ {
/*N*/ 	USHORT nPos=aWinList.Find(pWin1);
/*N*/ 	if (nPos!=SDRVIEWWIN_NOTFOUND) {
/*N*/ 		for (USHORT i=0; i<GetPageViewCount(); i++) {
/*N*/ 			GetPageViewPvNum(i)->DelWin(pWin1);
/*N*/ 		}
/*N*/ 		aWinList.Delete(nPos);
/*N*/ 	}
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if (pItemBrowser!=NULL) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pItemBrowser->ForceParent();
/*N*/ #endif
/*N*/ }

/*N*/ Rectangle SdrPaintView::GetVisibleArea( USHORT nNum )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return Rectangle();//STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::InitRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void SdrPaintView::GlueInvalidate() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP");
/*N*/ }

/*N*/ void SdrPaintView::InvalidateAllWin()
/*N*/ {
/*N*/ 	for (USHORT i=0; i<GetWinCount(); i++) {
/*N*/ 		OutputDevice* pOut=GetWin(i);
/*N*/ 		if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
/*N*/ 			InvalidateOneWin(*(Window*)pOut);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix)
/*N*/ {
/*N*/ 	USHORT nWinAnz=GetWinCount();
/*N*/ 	for (USHORT i=0; i<nWinAnz; i++) {
/*N*/ 		OutputDevice* pOut=GetWin(i);
/*N*/ 		if (pOut->GetOutDevType()==OUTDEV_WINDOW) {
/*N*/ 			Rectangle aRect(rRect);
/*N*/ 			if (bPlus1Pix) {
/*N*/ 				Size aPixSiz(1,1);
/*N*/ 				Size aSiz(pOut->PixelToLogic(aPixSiz));
/*N*/ 				aRect.Left	()-=aSiz.Width();
/*N*/ 				aRect.Top	()-=aSiz.Height();
/*N*/ 				aRect.Right ()+=aSiz.Width();
/*N*/ 				aRect.Bottom()+=aSiz.Height();
/*N*/ 			}
/*N*/ 			Point aOrg(pOut->GetMapMode().GetOrigin());
/*N*/ 			aOrg.X()=-aOrg.X(); aOrg.Y()=-aOrg.Y();
/*N*/ 			Rectangle aOutRect(aOrg,pOut->GetOutputSize());
/*N*/ 			if (aRect.IsOver(aOutRect)) {
/*N*/ 				InvalidateOneWin(*(Window*)pOut,aRect);
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPaintView::InvalidateOneWin(Window& rWin)
/*N*/ {
/*N*/ 	rWin.Invalidate();
/*N*/ }

/*N*/ void SdrPaintView::InvalidateOneWin(Window& rWin, const Rectangle& rRect)
/*N*/ {
/*N*/ 	rWin.Invalidate(rRect);
/*N*/ }


/*N*/ void SdrPaintView::LeaveAllGroup()
/*N*/ {
/*N*/ 	for (USHORT nv=0; nv<GetPageViewCount(); nv++) {
/*N*/ 		SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/ 		pPV->LeaveAllGroup();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPaintView::SetMasterPagePaintCaching( BOOL bOn, ULONG nCacheMode )
/*N*/ {
/*N*/ 	bMasterBmp = bOn;
/*N*/ 
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		if( SDR_MASTERPAGECACHE_DEFAULT == nCacheMode || SDR_MASTERPAGECACHE_NONE == nCacheMode )
/*N*/ 			nMasterCacheMode = SDR_MASTERPAGECACHE_FULL;
/*N*/ 		else
/*N*/ 			nMasterCacheMode = nCacheMode;
/*N*/ 
/*N*/ 		ReleaseMasterPagePaintCache();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nMasterCacheMode = SDR_MASTERPAGECACHE_NONE;
/*N*/ }

// z.B. rufen, wenn Obj der MPg geaendert
/*N*/ void SdrPaintView::ReleaseMasterPagePaintCache()
/*N*/ {
/*N*/ 	if (pMasterBmp!=NULL) {
/*?*/ 		delete pMasterBmp;
/*?*/ 		pMasterBmp=NULL;
/*N*/ 	}
/*N*/ }




/*N*/ void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	pDefaultStyleSheet=pStyleSheet;
/*N*/ 	if (pStyleSheet!=NULL && !bDontRemoveHardAttr) {
/*?*/ 		SfxWhichIter aIter(pStyleSheet->GetItemSet());
/*?*/ 		USHORT nWhich=aIter.FirstWhich();
/*?*/ 		while (nWhich!=0) {
/*?*/ 			if (pStyleSheet->GetItemSet().GetItemState(nWhich,TRUE)==SFX_ITEM_SET) {
/*?*/ 				aDefaultAttr.ClearItem(nWhich);
/*?*/ 			}
/*?*/ 			nWhich=aIter.NextWhich();
/*?*/ 		}
/*N*/ 	}
#ifndef SVX_LIGHT
/*N*/ 	if (pItemBrowser!=NULL) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pItemBrowser->SetDirty();
#endif
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrPaintView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/ 	{
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWPAGEVIEWS);
/*N*/ 		USHORT nv;
/*N*/ 		for (nv=0; nv<GetPageViewCount(); nv++) {
/*N*/ 			SdrPageView* pPV=GetPageViewPvNum(nv);
/*N*/ 			if (pPV->GetPage()->IsInserted()) {
/*?*/ 				rOut<<*pPV;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		for (nv=0; nv<GetPageHideCount(); nv++) {
/*?*/ 			SdrPageView* pPV=GetPageHidePvNum(nv);
/*?*/ 			if (pPV->GetPage()->IsInserted()) {
/*?*/ 				rOut<<*pPV;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWVISIELEM);
/*N*/ 		rOut<<BOOL(bLayerSortedRedraw);
/*N*/ 		rOut<<BOOL(bPageVisible);
/*N*/ 		rOut<<BOOL(bBordVisible);
/*N*/ 		rOut<<BOOL(bGridVisible);
/*N*/ 		rOut<<BOOL(bGridFront);
/*N*/ 		rOut<<BOOL(bHlplVisible);
/*N*/ 		rOut<<BOOL(bHlplFront);
/*N*/ 		rOut<<BOOL(bGlueVisible);
/*N*/ 		rOut<<aGridBig;
/*N*/ 		rOut<<aGridFin;
/*N*/ 		rOut<<aGridWdtX;
/*N*/ 		rOut<<aGridWdtY;
/*N*/ 		rOut<<aGridSubdiv;
/*N*/ 	} {
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWAKTLAYER);
/*N*/ 
/*N*/ 		// UNICODE: rOut << aAktLayer;
/*N*/ 		rOut.WriteByteString(aAktLayer);
/*N*/ 
/*N*/ 		// UNICODE: rOut << aMeasureLayer;
/*N*/ 		rOut.WriteByteString(aMeasureLayer);
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ BOOL SdrPaintView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/ 	const SdrNamedSubRecord& rSubHead,
/*N*/ 	SvStream& rIn)
/*N*/ {
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	if (rSubHead.GetInventor()==SdrInventor) {
/*N*/ 		bRet=TRUE;
/*N*/ 		switch (rSubHead.GetIdentifier()) {
/*N*/ 			case SDRIORECNAME_VIEWPAGEVIEWS: {
/*N*/ 				while (rSubHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
/*N*/ 					SdrPageView* pPV=new SdrPageView(NULL,Point(),*(SdrView*)this);
/*N*/ 					rIn>>*pPV;
/*N*/ 					if (pPV->GetPage()!=NULL) {
/*N*/ 						if (pPV->IsVisible()) {
/*N*/ 							aPagV.Insert(pPV,CONTAINER_APPEND);
/*N*/ 						} else aPagHide.Insert(pPV,CONTAINER_APPEND);
/*N*/ 					} else {
/*N*/ 						DBG_ERROR("SdrPaintView::ReadRecord(): Seite der PageView nicht gefunden");
/*N*/ 						delete pPV;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			} break;
/*N*/ 
/*N*/ 			case SDRIORECNAME_VIEWVISIELEM:
/*N*/ 			{
/*N*/ 				BOOL bTemp;
/*N*/ 
/*N*/ 				rIn>>bTemp; bLayerSortedRedraw=bTemp;
/*N*/ 				rIn>>bTemp; bPageVisible	  =bTemp;
/*N*/ 				rIn>>bTemp; bBordVisible	  =bTemp;
/*N*/ 				rIn>>bTemp; bGridVisible	  =bTemp;
/*N*/ 				rIn>>bTemp; bGridFront		  =bTemp;
/*N*/ 				rIn>>bTemp; bHlplVisible	  =bTemp;
/*N*/ 				rIn>>bTemp; bHlplFront		  =bTemp;
/*N*/ 				rIn>>bTemp; bGlueVisible	  =bTemp;
/*N*/ 				rIn>>aGridBig;
/*N*/ 				rIn>>aGridFin;
/*N*/ 
/*N*/ 				if(rSubHead.GetBytesLeft() > 0)
/*N*/ 				{
/*N*/ 					rIn >> aGridWdtX;
/*N*/ 					rIn >> aGridWdtY;
/*N*/ 					rIn >> aGridSubdiv;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case SDRIORECNAME_VIEWAKTLAYER:
/*N*/ 			{
/*N*/ 				// UNICODE: rIn >> aAktLayer;
/*N*/ 				rIn.ReadByteString(aAktLayer);
/*N*/ 
/*N*/ 				if(rSubHead.GetBytesLeft() > 0)
/*N*/ 				{
/*N*/ 					// UNICODE: rIn >> aMeasureLayer;
/*N*/ 					rIn.ReadByteString(aMeasureLayer);
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 
/*N*/ 			default:
/*N*/ 				bRet = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void SdrPaintView::onChangeColorConfig()
/*N*/ {
/*N*/     SetGridColor( Color( maColorConfig.GetColorValue( DRAWGRID ).nColor ) );
/*N*/ }

/*N*/ void SdrPaintView::SetGridColor( Color aColor )
/*N*/ {
/*N*/ 	maGridColor = aColor;
/*N*/ }


// #103834# Set background color for svx at SdrPageViews

/*N*/ void SdrPaintView::ImpForceSwapOut()
/*N*/ {
/*N*/     // #110290# Force swap out all graphics on this page. There might be 
/*N*/ 	// some left, since every graphic that has not received a Draw yet, 
/*N*/     // but is swapped in, has its swapout handler disabled.
/*N*/     while( !maSwappedInGraphicsStack.empty() )
/*N*/     {
/*N*/         maSwappedInGraphicsStack.front()->ForceSwapOut();
/*N*/         maSwappedInGraphicsStack.pop_front();
/*N*/     }
/*N*/ }

// eof
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
