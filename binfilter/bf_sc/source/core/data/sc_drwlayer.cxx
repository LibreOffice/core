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


#include "scitems.hxx"
#include <bf_svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef _SVX_FRMDIRITEM_HXX
#include <bf_svx/frmdiritem.hxx>
#endif
#include <bf_svx/objfac3d.hxx>
#include <bf_svx/svdoutl.hxx>
#include <bf_svx/svditer.hxx>
#include <bf_svx/svdoedge.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdundo.hxx>
#include <bf_svx/drawitem.hxx>
#ifndef _SVX_FHGTITEM_HXX
#include <bf_svx/fhgtitem.hxx>
#endif
#include <bf_sfx2/docfile.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_offmgr/app.hxx>

#include "drwlayer.hxx"
#include "drawpage.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "userdat.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
namespace binfilter {

#define DET_ARROW_OFFSET	1000

//	Abstand zur naechsten Zelle beim Loeschen (bShrink), damit der Anker
//	immer an der richtigen Zelle angezeigt wird
//#define SHRINK_DIST		3
//	und noch etwas mehr, damit das Objekt auch sichtbar in der Zelle liegt
#define SHRINK_DIST		25

#define SHRINK_DIST_TWIPS	15

// -----------------------------------------------------------------------
//
//	Das Anpassen der Detektiv-UserData muss zusammen mit den Draw-Undo's
//	in der SdrUndoGroup liegen, darum von SdrUndoAction abgeleitet:


// -----------------------------------------------------------------------

// STATIC DATA -----------------------------------------------------------

/*N*/ TYPEINIT1(ScTabDeletedHint, SfxHint);
/*N*/ TYPEINIT1(ScTabSizeChangedHint, SfxHint);

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static USHORT nInst = 0;

SvPersist* ScDrawLayer::pGlobalDrawPersist = NULL;

BOOL bDrawIsInUndo = FALSE;			//! Member

// -----------------------------------------------------------------------

/*N*/ __EXPORT ScTabDeletedHint::~ScTabDeletedHint()
/*N*/ {
/*N*/ }

/*N*/ ScTabSizeChangedHint::ScTabSizeChangedHint( USHORT nTabNo ) :
/*N*/ 	nTab( nTabNo )
/*N*/ {
/*N*/ }

/*N*/ __EXPORT ScTabSizeChangedHint::~ScTabSizeChangedHint()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

#define MAXMM	10000000

/*N*/ inline void TwipsToMM( long& nVal )
/*N*/ {
/*N*/ 	nVal = (long) ( nVal * HMM_PER_TWIPS );
/*N*/ }

/*N*/ inline void ReverseTwipsToMM( long& nVal )
/*N*/ {
/*N*/ 	//	reverse the effect of TwipsToMM - round up here (add 1)
/*N*/ 
/*N*/ 	nVal = ((long) ( nVal / HMM_PER_TWIPS )) + 1;
/*N*/ }

/*N*/ void lcl_TwipsToMM( Point& rPoint )
/*N*/ {
/*N*/ 	TwipsToMM( rPoint.X() );
/*N*/ 	TwipsToMM( rPoint.Y() );
/*N*/ }

/*N*/ void lcl_ReverseTwipsToMM( Point& rPoint )
/*N*/ {
/*N*/ 	ReverseTwipsToMM( rPoint.X() );
/*N*/ 	ReverseTwipsToMM( rPoint.Y() );
/*N*/ }

/*N*/ void lcl_ReverseTwipsToMM( Rectangle& rRect )
/*N*/ {
/*N*/ 	ReverseTwipsToMM( rRect.Left() );
/*N*/ 	ReverseTwipsToMM( rRect.Right() );
/*N*/ 	ReverseTwipsToMM( rRect.Top() );
/*N*/ 	ReverseTwipsToMM( rRect.Bottom() );
/*N*/ }

// -----------------------------------------------------------------------


/*N*/ ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const String& rName ) :
/*N*/ 	FmFormModel( SvtPathOptions().GetPalettePath(),
/*N*/ 				 NULL, 							// SfxItemPool* Pool
/*N*/ 				 pGlobalDrawPersist ?
/*N*/ 				 	pGlobalDrawPersist :
/*N*/ 				 	( pDocument ? pDocument->GetDocumentShell() : NULL ),
/*N*/ 				 TRUE ),		// bUseExtColorTable (is set below)
/*N*/ 	aName( rName ),
/*N*/ 	pDoc( pDocument ),
/*N*/ 	pUndoGroup( NULL ),
/*N*/ 	bRecording( FALSE ),
/*N*/ 	bAdjustEnabled( TRUE ),
/*N*/ 	bHyphenatorSet( FALSE )
/*N*/ {
/*N*/ 	pGlobalDrawPersist = NULL;			// nur einmal benutzen
/*N*/ 
/*N*/ 	SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : NULL;
/*N*/ 	if ( pObjSh )
/*N*/ 	{
/*N*/ 		SetObjectShell( pObjSh );
/*N*/ 
/*N*/ 		// set color table
/*N*/ 		SvxColorTableItem* pColItem = (SvxColorTableItem*) pObjSh->GetItem( ITEMID_COLOR_TABLE );
/*N*/ 		XColorTable* pXCol = pColItem ? pColItem->GetColorTable() : OFF_APP()->GetStdColorTable();
/*N*/ 		SetColorTable( pXCol );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		SetColorTable( OFF_APP()->GetStdColorTable() );
/*N*/ 
/*N*/ 	SetSwapGraphics(TRUE);
/*N*/ //	SetSwapAsynchron(TRUE);		// an der View
/*N*/ 
/*N*/ 	SetScaleUnit(MAP_100TH_MM);
/*N*/ 	SfxItemPool& rPool = GetItemPool();
/*N*/ 	rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
/*N*/ 	SvxFrameDirectionItem aModeItem( FRMDIR_ENVIRONMENT, EE_PARA_WRITINGDIR );
/*N*/ 	rPool.SetPoolDefaultItem( aModeItem );
/*N*/ 	rPool.FreezeIdRanges();							// der Pool wird auch direkt verwendet
/*N*/ 
/*N*/ 	SdrLayerAdmin& rAdmin = GetLayerAdmin();
/*N*/ 	rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("vorne")),    SC_LAYER_FRONT);
/*N*/ 	rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("hinten")),   SC_LAYER_BACK);
/*N*/ 	rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("intern")),   SC_LAYER_INTERN);
/*N*/ 	rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Controls")), SC_LAYER_CONTROLS);
/*N*/ 	// "Controls" is new - must also be created when loading
/*N*/ 
/*N*/ 	//	Link fuer URL-Fields setzen
/*N*/ 	ScModule* pScMod = SC_MOD();
/*N*/ 	Outliner& rOutliner = GetDrawOutliner();
/*N*/ 	rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );
/*N*/ 
/*N*/ 	Outliner& rHitOutliner = GetHitTestOutliner();
/*N*/ 	rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );
/*N*/ 
/*N*/     // #95129# SJ: set FontHeight pool defaults without changing static SdrEngineDefaults
/*N*/     SfxItemPool* pOutlinerPool = rOutliner.GetEditTextObjectPool();
/*N*/     if ( pOutlinerPool )
/*N*/  	    pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));           // 12Pt
/*N*/     SfxItemPool* pHitOutlinerPool = rHitOutliner.GetEditTextObjectPool();
/*N*/     if ( pHitOutlinerPool )
/*N*/  	    pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));    // 12Pt
/*N*/ 
/*N*/ 	//	URL-Buttons haben keinen Handler mehr, machen alles selber
/*N*/ 
/*N*/ 	if( !nInst++ )
/*N*/ 	{
/*N*/ 		pFac = new ScDrawObjFactory;
/*N*/ 		pF3d = new E3dObjFactory;
/*N*/ 	}
/*N*/ }

/*N*/ __EXPORT ScDrawLayer::~ScDrawLayer()
/*N*/ {
/*N*/ 	Broadcast(SdrHint(HINT_MODELCLEARED));
/*N*/ 
/*N*/ 	Clear();
/*N*/ 
/*N*/ 	delete pUndoGroup;
/*N*/ 	if( !--nInst )
/*N*/ 	{
/*N*/ 		delete pFac, pFac = NULL;
/*N*/ 		delete pF3d, pF3d = NULL;
/*N*/ 	}
/*N*/ }


/*N*/ SdrPage* __EXPORT ScDrawLayer::AllocPage(FASTBOOL bMasterPage)
/*N*/ {
/*N*/ 	//	don't create basic until it is needed
/*N*/ 	StarBASIC* pBasic = NULL;
/*N*/ 	ScDrawPage* pPage = new ScDrawPage( *this, pBasic, bMasterPage );
/*N*/ 	return pPage;
/*N*/ }


/*N*/ void ScDrawLayer::UpdateBasic()
/*N*/ {
/*N*/ 	//	don't create basic until it is needed
/*N*/ 	//!	remove this method?
/*N*/ }



/*N*/ void ScDrawLayer::ScAddPage( USHORT nTab )
/*N*/ {
/*N*/ 	if (bDrawIsInUndo)
/*N*/ 		return;
/*N*/ 
/*N*/ 	ScDrawPage* pPage = (ScDrawPage*)AllocPage( FALSE );
/*N*/ 	InsertPage(pPage, nTab);
/*N*/ 	if (bRecording)
/*N*/ 		AddCalcUndo(new SdrUndoNewPage(*pPage));
/*N*/ }


/*N*/ void ScDrawLayer::ScRenamePage( USHORT nTab, const String& rNewName )
/*N*/ {
/*N*/ 	ScDrawPage* pPage = (ScDrawPage*) GetPage(nTab);
/*N*/ 	if (pPage)
/*N*/ 		pPage->SetName(rNewName);
/*N*/ }





/*N*/ void ScDrawLayer::SetPageSize( USHORT nPageNo, const Size& rSize )
/*N*/ {
/*N*/ 	SdrPage* pPage = GetPage(nPageNo);
/*N*/ 	if (pPage)
/*N*/ 	{
/*N*/ 		if ( rSize != pPage->GetSize() )
/*N*/ 		{
/*N*/ 			pPage->SetSize( rSize );
/*N*/ 			Broadcast( ScTabSizeChangedHint( nPageNo ) );	// SetWorkArea() an den Views
/*N*/ 		}
/*N*/ 
/*N*/ 		// Detektivlinien umsetzen (an neue Hoehen/Breiten anpassen)
/*N*/ 		//	auch wenn Groesse gleich geblieben ist
/*N*/ 		//	(einzelne Zeilen/Spalten koennen geaendert sein)
/*N*/ 
/*N*/ 		ULONG nCount = pPage->GetObjCount();
/*N*/ 		for ( ULONG i = 0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			SdrObject* pObj = pPage->GetObj( i );
/*N*/ 			ScDrawObjData* pData = GetObjData( pObj );
/*N*/ 			if( pData )
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				RecalcPos( pObj, pData );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void ScDrawLayer::Load( SvStream& rStream )
/*N*/ {
/*N*/ 	bRecording = FALSE;
/*N*/ 	DELETEZ(pUndoGroup);
/*N*/ 
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 	while (aHdr.BytesLeft())
/*N*/ 	{
/*N*/ 		USHORT nID;
/*N*/ 		rStream >> nID;
/*N*/ 		switch (nID)
/*N*/ 		{
/*N*/ 			case SCID_DRAWPOOL:
/*N*/ 				{
/*N*/ 					ScReadHeader aPoolHdr( rStream );
/*N*/ 					GetItemPool().Load( rStream );				//! in Pool-Stream ?
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SCID_DRAWMODEL:
/*N*/ 				{
/*N*/ 					ScReadHeader aDrawHdr( rStream );
/*N*/ 					rStream >> *this;
/*N*/ 
/*N*/ 					//	Control-Layer ist nicht in alten Dateien
/*N*/ 					SdrLayerAdmin& rAdmin = GetLayerAdmin();
/*N*/ 					const SdrLayer* pLayer = rAdmin.GetLayerPerID(SC_LAYER_CONTROLS);
/*N*/ 					if (!pLayer)
/*N*/ 						rAdmin.NewLayer(
/*N*/ 							String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Controls")),
/*N*/ 							SC_LAYER_CONTROLS);
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				{
/*N*/ 					DBG_ERROR("unbekannter Sub-Record in ScDrawLayer::Load");
/*N*/ 					ScReadHeader aDummyHdr( rStream );
/*N*/ 				}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	GetItemPool().LoadCompleted();
/*N*/ }

/*N*/ void ScDrawLayer::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	ScWriteHeader aHdr( rStream );
/*N*/ 
/*N*/ //-/	const_cast<ScDrawLayer*>(this)->PrepareStore();		// non-const
/*N*/ 	const_cast<ScDrawLayer*>(this)->PreSave();		// non-const
/*N*/ 
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_DRAWPOOL;
/*N*/ 		ScWriteHeader aPoolHdr( rStream );
/*N*/ 		GetItemPool().Store( rStream ); 			//! in Pool-Stream ?
/*N*/ 	}
/*N*/ 
/*N*/ 	{
/*N*/ 		rStream << (USHORT) SCID_DRAWMODEL;
/*N*/ 		ScWriteHeader aDrawHdr( rStream );
/*N*/ 		rStream << *this;
/*N*/ 	}
/*N*/ 
/*N*/ 	const_cast<ScDrawLayer*>(this)->PostSave();		// non-const
/*N*/ }

/*N*/ BOOL ScDrawLayer::GetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( pDoc, "ScDrawLayer::GetPrintArea without document" );
/*N*/ 	if ( !pDoc )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	USHORT nTab = rRange.aStart.Tab();
/*N*/ 	DBG_ASSERT( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab unterschiedlich" );
/*N*/ 
/*N*/ 	BOOL bAny = FALSE;
/*N*/ 	long nEndX = 0;
/*N*/ 	long nEndY = 0;
/*N*/ 	long nStartX = LONG_MAX;
/*N*/ 	long nStartY = LONG_MAX;
/*N*/ 	USHORT i;
/*N*/ 
/*N*/ 	// Grenzen ausrechnen
/*N*/ 
/*N*/ 	if (!bSetHor)
/*N*/ 	{
/*?*/ 		nStartX = 0;
/*?*/ 		USHORT nStartCol = rRange.aStart.Col();
/*?*/ 		for (i=0; i<nStartCol; i++)
/*?*/ 			nStartX +=pDoc->GetColWidth(i,nTab);
/*?*/ 		nEndX = nStartX;
/*?*/ 		USHORT nEndCol = rRange.aEnd.Col();
/*?*/ 		for (i=nStartCol; i<=nEndCol; i++)
/*?*/ 			nEndX += pDoc->GetColWidth(i,nTab);
/*?*/ 		nStartX = (long)(nStartX * HMM_PER_TWIPS);
/*?*/ 		nEndX   = (long)(nEndX   * HMM_PER_TWIPS);
/*N*/ 	}
/*N*/ 	if (!bSetVer)
/*N*/ 	{
/*?*/ 		nStartY = 0;
/*?*/ 		USHORT nStartRow = rRange.aStart.Row();
/*?*/ 		for (i=0; i<nStartRow; i++)
/*?*/ 			nStartY +=pDoc->FastGetRowHeight(i,nTab);
/*?*/ 		nEndY = nStartY;
/*?*/ 		USHORT nEndRow = rRange.aEnd.Row();
/*?*/ 		for (i=nStartRow; i<=nEndRow; i++)
/*?*/ 			nEndY += pDoc->FastGetRowHeight(i,nTab);
/*?*/ 		nStartY = (long)(nStartY * HMM_PER_TWIPS);
/*?*/ 		nEndY   = (long)(nEndY   * HMM_PER_TWIPS);
/*N*/ 	}
/*N*/ 
/*N*/ 	const SdrPage* pPage = GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page nicht gefunden");
/*N*/ 	if (pPage)
/*N*/ 	{
/*N*/ 		SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 		SdrObject* pObject = aIter.Next();
/*N*/ 		while (pObject)
/*N*/ 		{
/*N*/ 							//! Flags (ausgeblendet?) testen
/*N*/ 
/*N*/ 			Rectangle aObjRect = pObject->GetBoundRect();
/*N*/ 			BOOL bFit = TRUE;
/*N*/ 			if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
/*N*/ 				bFit = FALSE;
/*N*/ 			if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
/*N*/ 				bFit = FALSE;
/*N*/ 			if ( bFit )
/*N*/ 			{
/*N*/ 				if (bSetHor)
/*N*/ 				{
/*N*/ 					if (aObjRect.Left() < nStartX) nStartX = aObjRect.Left();
/*N*/ 					if (aObjRect.Right()  > nEndX) nEndX = aObjRect.Right();
/*N*/ 				}
/*N*/ 				if (bSetVer)
/*N*/ 				{
/*N*/ 					if (aObjRect.Top()  < nStartY) nStartY = aObjRect.Top();
/*N*/ 					if (aObjRect.Bottom() > nEndY) nEndY = aObjRect.Bottom();
/*N*/ 				}
/*N*/ 				bAny = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/ 			pObject = aIter.Next();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bAny)
/*N*/ 	{
/*N*/ 		DBG_ASSERT( nStartX<=nEndX && nStartY<=nEndY, "Start/End falsch in ScDrawLayer::GetPrintArea" );
/*N*/ 
/*N*/ 		if (bSetHor)
/*N*/ 		{
/*N*/ 			nStartX = (long) (nStartX / HMM_PER_TWIPS);
/*N*/ 			nEndX = (long) (nEndX / HMM_PER_TWIPS);
/*N*/ 			long nWidth;
/*N*/ 
/*N*/ 			nWidth = 0;
/*N*/ 			for (i=0; i<MAXCOL && nWidth<=nStartX; i++)
/*N*/ 				nWidth += pDoc->GetColWidth(i,nTab);
/*N*/ 			rRange.aStart.SetCol( i ? (i-1) : 0 );
/*N*/ 
/*N*/ 			nWidth = 0;
/*N*/ 			for (i=0; i<MAXCOL && nWidth<=nEndX; i++)			//! bei Start anfangen
/*N*/ 				nWidth += pDoc->GetColWidth(i,nTab);
/*N*/ 			rRange.aEnd.SetCol( i ? (i-1) : 0 );
/*N*/ 		}
/*N*/ 
/*N*/ 		if (bSetVer)
/*N*/ 		{
/*N*/ 			nStartY = (long) (nStartY / HMM_PER_TWIPS);
/*N*/ 			nEndY = (long) (nEndY / HMM_PER_TWIPS);
/*N*/ 			long nHeight;
/*N*/ 
/*N*/ 			nHeight = 0;
/*N*/ 			for (i=0; i<MAXROW && nHeight<=nStartY; i++)
/*N*/ 				nHeight += pDoc->FastGetRowHeight(i,nTab);
/*N*/ 			rRange.aStart.SetRow( i ? (i-1) : 0 );
/*N*/ 
/*N*/ 			nHeight = 0;
/*N*/ 			for (i=0; i<MAXROW && nHeight<=nEndY; i++)
/*N*/ 				nHeight += pDoc->FastGetRowHeight(i,nTab);
/*N*/ 			rRange.aEnd.SetRow( i ? (i-1) : 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (bSetHor)
/*N*/ 		{
/*N*/ 			rRange.aStart.SetCol(0);
/*N*/ 			rRange.aEnd.SetCol(0);
/*N*/ 		}
/*N*/ 		if (bSetVer)
/*N*/ 		{
/*N*/ 			rRange.aStart.SetRow(0);
/*N*/ 			rRange.aEnd.SetRow(0);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bAny;
/*N*/ }

/*N*/ void ScDrawLayer::AddCalcUndo( SdrUndoAction* pUndo )
/*N*/ {
/*N*/ 	if (bRecording)
/*N*/ 	{
/*N*/ 		if (!pUndoGroup)
/*N*/ 			pUndoGroup = new SdrUndoGroup(*this);
/*N*/ 
/*N*/ 		pUndoGroup->AddAction( pUndo );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		delete pUndo;
/*N*/ }

//	MoveAreaTwips: all measures are kept in twips
/*N*/ void ScDrawLayer::MoveAreaTwips( USHORT nTab, const Rectangle& rArea,
/*N*/ 		const Point& rMove, const Point& rTopLeft )
/*N*/ {
/*N*/ 	if (!rMove.X() && !rMove.Y())
/*N*/ 		return; 									// nix
/*N*/ 
/*N*/ 	SdrPage* pPage = GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page nicht gefunden");
/*N*/ 	if (!pPage)
/*N*/ 		return;
/*N*/ 
/*N*/ 	// fuer Shrinking!
/*N*/ 	Rectangle aNew( rArea );
/*N*/ 	BOOL bShrink = FALSE;
/*N*/ 	if ( rMove.X() < 0 || rMove.Y() < 0 )		// verkleinern
/*N*/ 	{
/*N*/ 		if ( rTopLeft != rArea.TopLeft() )		// sind gleich beim Verschieben von Zellen
/*N*/ 		{
/*N*/ 			bShrink = TRUE;
/*N*/ 			aNew.Left() = rTopLeft.X();
/*N*/ 			aNew.Top() = rTopLeft.Y();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SdrObjListIter aIter( *pPage, IM_FLAT );
/*N*/ 	SdrObject* pObject = aIter.Next();
/*N*/ 	while (pObject)
/*N*/ 	{
/*N*/ 		if( GetAnchor( pObject ) == SCA_CELL )
/*N*/ 		{
/*N*/ 			if ( GetObjData( pObject ) )					// Detektiv-Pfeil ?
/*N*/ 			{
/*N*/ 				// hier nichts
/*N*/ 			}
/*N*/ 			else if ( pObject->ISA( SdrEdgeObj ) )			// Verbinder?
/*N*/ 			{
/*N*/ 				//	hier auch nichts
/*N*/ 				//!	nicht verbundene Enden wie bei Linien (s.u.) behandeln?
/*N*/ 			}
/*N*/ 			else if ( pObject->IsPolyObj() && pObject->GetPointCount()==2 )
/*N*/ 			{
/*N*/ 				for (USHORT i=0; i<2; i++)
/*N*/ 				{
/*N*/ 					BOOL bMoved = FALSE;
/*N*/ 					Point aPoint = pObject->GetPoint(i);
/*N*/ 					lcl_ReverseTwipsToMM( aPoint );
/*N*/ 					if (rArea.IsInside(aPoint))
/*N*/ 					{
/*N*/ 						aPoint += rMove; bMoved = TRUE;
/*N*/ 					}
/*N*/ 					else if (bShrink && aNew.IsInside(aPoint))
/*N*/ 					{
/*?*/ 						//	Punkt ist in betroffener Zelle - Test auf geloeschten Bereich
/*?*/ 						if ( rMove.X() && aPoint.X() >= rArea.Left() + rMove.X() )
/*?*/ 						{
/*?*/ 							aPoint.X() = rArea.Left() + rMove.X() - SHRINK_DIST_TWIPS;
/*?*/ 							if ( aPoint.X() < 0 ) aPoint.X() = 0;
/*?*/ 							bMoved = TRUE;
/*?*/ 						}
/*?*/ 						if ( rMove.Y() && aPoint.Y() >= rArea.Top() + rMove.Y() )
/*?*/ 						{
/*?*/ 							aPoint.Y() = rArea.Top() + rMove.Y() - SHRINK_DIST_TWIPS;
/*?*/ 							if ( aPoint.Y() < 0 ) aPoint.Y() = 0;
/*?*/ 							bMoved = TRUE;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					if( bMoved )
/*N*/ 					{
/*N*/ 						AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
/*N*/ 						lcl_TwipsToMM( aPoint );
/*N*/ 						pObject->SetPoint( aPoint, i );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				Rectangle aObjRect = pObject->GetLogicRect();
/*N*/ 				Point aOldMMPos = aObjRect.TopLeft();			// not converted, millimeters
/*N*/ 				lcl_ReverseTwipsToMM( aObjRect );
/*N*/ 				Point aTopLeft = aObjRect.TopLeft();
/*N*/ 				Size aMoveSize;
/*N*/ 				BOOL bDoMove = FALSE;
/*N*/ 				if (rArea.IsInside(aTopLeft))
/*N*/ 				{
/*N*/ 					aMoveSize = Size(rMove.X(),rMove.Y());
/*N*/ 					bDoMove = TRUE;
/*N*/ 				}
/*N*/ 				else if (bShrink && aNew.IsInside(aTopLeft))
/*N*/ 				{
/*?*/ 					//	Position ist in betroffener Zelle - Test auf geloeschten Bereich
/*?*/ 					if ( rMove.X() && aTopLeft.X() >= rArea.Left() + rMove.X() )
/*?*/ 					{
/*?*/ 						aMoveSize.Width() = rArea.Left() + rMove.X() - SHRINK_DIST - aTopLeft.X();
/*?*/ 						bDoMove = TRUE;
/*?*/ 					}
/*?*/ 					if ( rMove.Y() && aTopLeft.Y() >= rArea.Top() + rMove.Y() )
/*?*/ 					{
/*?*/ 						aMoveSize.Height() = rArea.Top() + rMove.Y() - SHRINK_DIST - aTopLeft.Y();
/*?*/ 						bDoMove = TRUE;
/*?*/ 					}
/*N*/ 				}
/*N*/ 				if ( bDoMove )
/*N*/ 				{
/*N*/ 					if ( aTopLeft.X() + aMoveSize.Width() < 0 )
/*?*/ 						aMoveSize.Width() = -aTopLeft.X();
/*N*/ 					if ( aTopLeft.Y() + aMoveSize.Height() < 0 )
/*?*/ 						aMoveSize.Height() = -aTopLeft.Y();
/*N*/ 
/*N*/ 					//	get corresponding move size in millimeters:
/*N*/ 					Point aNewPos( aTopLeft.X() + aMoveSize.Width(), aTopLeft.Y() + aMoveSize.Height() );
/*N*/ 					lcl_TwipsToMM( aNewPos );
/*N*/ 					aMoveSize = Size( aNewPos.X() - aOldMMPos.X(), aNewPos.Y() - aOldMMPos.Y() );	// millimeters
/*N*/ 
/*N*/ 					AddCalcUndo( new SdrUndoMoveObj( *pObject, aMoveSize ) );
/*N*/ 					pObject->Move( aMoveSize );
/*N*/ 				}
/*N*/ 				else if ( rArea.IsInside(aObjRect.BottomRight()) &&
/*N*/ 							!pObject->IsResizeProtect() )
/*N*/ 				{
/*N*/ 					//	geschuetzte Groessen werden nicht veraendert
/*N*/ 					//	(Positionen schon, weil sie ja an der Zelle "verankert" sind)
/*N*/ 					AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
/*N*/ 					long nOldSizeX = aObjRect.Right() - aObjRect.Left() + 1;
/*N*/ 					long nOldSizeY = aObjRect.Bottom() - aObjRect.Top() + 1;
/*N*/ 					pObject->Resize( aOldMMPos, Fraction( nOldSizeX+rMove.X(), nOldSizeX ),
/*N*/ 												Fraction( nOldSizeY+rMove.Y(), nOldSizeY ) );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pObject = aIter.Next();
/*N*/ 	}
/*N*/ }


/*N*/ void ScDrawLayer::WidthChanged( USHORT nTab, USHORT nCol, long nDifTwips )
/*N*/ {
/*N*/ 	DBG_ASSERT( pDoc, "ScDrawLayer::WidthChanged without document" );
/*N*/ 	if ( !pDoc )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (!bAdjustEnabled)
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	Rectangle aRect;
/*N*/ 	Point aTopLeft;
/*N*/ 
/*N*/ 	for (i=0; i<nCol; i++)
/*N*/ 		aRect.Left() += pDoc->GetColWidth(i,nTab);
/*N*/ 	aTopLeft.X() = aRect.Left();
/*N*/ 	aRect.Left() += pDoc->GetColWidth(nCol,nTab);
/*N*/ 
/*N*/ 	aRect.Right() = MAXMM;
/*N*/ 	aRect.Top() = 0;
/*N*/ 	aRect.Bottom() = MAXMM;
/*N*/ 
/*N*/ 	//!	aTopLeft ist falsch, wenn mehrere Spalten auf einmal ausgeblendet werden
/*N*/ 
/*N*/ 	MoveAreaTwips( nTab, aRect, Point( nDifTwips,0 ), aTopLeft );
/*N*/ }

/*N*/ void ScDrawLayer::HeightChanged( USHORT nTab, USHORT nRow, long nDifTwips )
/*N*/ {
/*N*/ 	DBG_ASSERT( pDoc, "ScDrawLayer::HeightChanged without document" );
/*N*/ 	if ( !pDoc )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if (!bAdjustEnabled)
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	Rectangle aRect;
/*N*/ 	Point aTopLeft;
/*N*/ 
/*N*/ 	for (i=0; i<nRow; i++)
/*N*/ 		aRect.Top() += pDoc->FastGetRowHeight(i,nTab);
/*N*/ 	aTopLeft.Y() = aRect.Top();
/*N*/ 	aRect.Top() += pDoc->FastGetRowHeight(nRow,nTab);
/*N*/ 
/*N*/ 	aRect.Bottom() = MAXMM;
/*N*/ 	aRect.Left() = 0;
/*N*/ 	aRect.Right() = MAXMM;
/*N*/ 
/*N*/ 	//!	aTopLeft ist falsch, wenn mehrere Zeilen auf einmal ausgeblendet werden
/*N*/ 
/*N*/ 	MoveAreaTwips( nTab, aRect, Point( 0,nDifTwips ), aTopLeft );
/*N*/ }

/*N*/ BOOL ScDrawLayer::HasObjectsInRows( USHORT nTab, USHORT nStartRow, USHORT nEndRow )
/*N*/ {
/*N*/ 	DBG_ASSERT( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
/*N*/ 	if ( !pDoc )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	Rectangle aTestRect;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nStartRow; i++)
/*N*/ 		aTestRect.Top() += pDoc->FastGetRowHeight(i,nTab);
/*N*/ 
/*N*/ 	if (nEndRow==MAXROW)
/*N*/ 		aTestRect.Bottom() = MAXMM;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aTestRect.Bottom() = aTestRect.Top();
/*N*/ 		for (i=nStartRow; i<=nEndRow; i++)
/*N*/ 		{
/*N*/ 			aTestRect.Bottom() += pDoc->FastGetRowHeight(i,nTab);
/*N*/ 		}
/*N*/ 		TwipsToMM( aTestRect.Bottom() );
/*N*/ 	}
/*N*/ 
/*N*/ 	TwipsToMM( aTestRect.Top() );
/*N*/ 
/*N*/ 	aTestRect.Left()  = 0;
/*N*/ 	aTestRect.Right() = MAXMM;
/*N*/ 
/*N*/ 	SdrPage* pPage = GetPage(nTab);
/*N*/ 	DBG_ASSERT(pPage,"Page nicht gefunden");
/*N*/ 	if (!pPage)
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	BOOL bFound = FALSE;
/*N*/ 
/*N*/ 	Rectangle aObjRect;
/*N*/ 	SdrObjListIter aIter( *pPage );
/*N*/ 	SdrObject* pObject = aIter.Next();
/*N*/ 	while ( pObject && !bFound )
/*N*/ 	{
/*N*/ 		aObjRect = pObject->GetSnapRect();	//! GetLogicRect ?
/*N*/ 		if (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft()))
/*N*/ 			bFound = TRUE;
/*N*/ 
/*N*/ 		pObject = aIter.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bFound;
/*N*/ }








// static

/*N*/ inline BOOL IsNamedObject( SdrObject* pObj, const String& rName )
/*N*/ {
/*N*/ 	//	TRUE if rName is the object's Name or PersistName
/*N*/ 	//	(used to find a named object)
/*N*/ 
/*N*/ 	return ( pObj->GetName() == rName ||
/*N*/ 			( pObj->GetObjIdentifier() == OBJ_OLE2 &&
/*N*/ 			  static_cast<SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
/*N*/ }

/*N*/ SdrObject* ScDrawLayer::GetNamedObject( const String& rName, USHORT nId, USHORT& rFoundTab ) const
/*N*/ {
/*N*/ 	USHORT nTabCount = GetPageCount();
/*N*/ 	for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 	{
/*N*/ 		const SdrPage* pPage = GetPage(nTab);
/*N*/ 		DBG_ASSERT(pPage,"Page ?");
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
/*N*/ 			SdrObject* pObject = aIter.Next();
/*N*/ 			while (pObject)
/*N*/ 			{
/*N*/ 				if ( nId == 0 || pObject->GetObjIdentifier() == nId )
/*N*/ 					if ( IsNamedObject( pObject, rName ) )
/*N*/ 					{
/*N*/ 						rFoundTab = nTab;
/*N*/ 						return pObject;
/*N*/ 					}
/*N*/ 
/*N*/ 				pObject = aIter.Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return NULL;
/*N*/ }




/*N*/ void ScDrawLayer::SetAnchor( SdrObject* pObj, ScAnchorType eType )
/*N*/ {
/*N*/ 	// Ein an der Seite verankertes Objekt zeichnet sich durch eine Anker-Pos
/*N*/ 	// von (0,1) aus. Das ist ein shabby Trick, der aber funktioniert!
/*N*/ 	Point aAnchor( 0, eType == SCA_PAGE ? 1 : 0 );
/*N*/ 	pObj->SetAnchorPos( aAnchor );
/*N*/ }

/*N*/ ScAnchorType ScDrawLayer::GetAnchor( const SdrObject* pObj )
/*N*/ {
/*N*/ 	Point aAnchor( pObj->GetAnchorPos() );
/*N*/ 	return ( aAnchor.Y() != 0 ) ? SCA_PAGE : SCA_CELL;
/*N*/ }

/*N*/ ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, BOOL bCreate )		// static
/*N*/ {
/*N*/ 	USHORT nCount = pObj->GetUserDataCount();
/*N*/ 	for( USHORT i = 0; i < nCount; i++ )
/*N*/ 	{
/*?*/ 		SdrObjUserData* pData = pObj->GetUserData( i );
/*?*/ 		if( pData && pData->GetInventor() == SC_DRAWLAYER
/*?*/ 					&& pData->GetId() == SC_UD_OBJDATA )
/*?*/ 			return (ScDrawObjData*) pData;
/*N*/ 	}
/*N*/ 	if( bCreate )
/*N*/ 	{
/*N*/ 		ScDrawObjData* pData = new ScDrawObjData;
/*N*/ 		pObj->InsertUserData( pData, 0 );
/*N*/ 		return pData;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ ScIMapInfo* ScDrawLayer::GetIMapInfo( SdrObject* pObj )				// static
/*N*/ {
/*N*/ 	USHORT nCount = pObj->GetUserDataCount();
/*N*/ 	for( USHORT i = 0; i < nCount; i++ )
/*N*/ 	{
/*?*/ 		SdrObjUserData* pData = pObj->GetUserData( i );
/*?*/ 		if( pData && pData->GetInventor() == SC_DRAWLAYER
/*?*/ 					&& pData->GetId() == SC_UD_IMAPDATA )
/*?*/ 			return (ScIMapInfo*) pData;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }


// static:


/*N*/ void __EXPORT ScDrawLayer::SetChanged( FASTBOOL bFlg /* =TRUE */ )
/*N*/ {
/*N*/ 	if ( bFlg && pDoc )
/*N*/ 		pDoc->SetChartListenerCollectionNeedsUpdate( TRUE );
/*N*/ 	FmFormModel::SetChanged( bFlg );
/*N*/ }

/*N*/ SvStream* __EXPORT ScDrawLayer::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
/*N*/ {
/*N*/ 	DBG_ASSERT( pDoc, "ScDrawLayer::GetDocumentStream without document" );
/*N*/ 	if ( !pDoc )
/*N*/ 		return NULL;
/*N*/ 
/*N*/ 	SvStorage*	pStor = pDoc->GetDocumentShell() ? pDoc->GetDocumentShell()->GetMedium()->GetStorage() : NULL;
/*N*/ 	SvStream*	pRet = NULL;
/*N*/ 
/*N*/ 	if( pStor )
/*N*/ 	{
/*N*/ 		if( rStreamInfo.maUserData.Len() &&
/*N*/ 			( rStreamInfo.maUserData.GetToken( 0, ':' ) ==
/*N*/ 			  String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
/*N*/ 		{
/*?*/ 			const String aPicturePath( rStreamInfo.maUserData.GetToken( 1, ':' ) );
/*?*/ 
/*?*/ 			// graphic from picture stream in picture storage in XML package
/*?*/ 			if( aPicturePath.GetTokenCount( '/' ) == 2 )
/*?*/ 			{
/*?*/ 				const String aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );
/*?*/ 
/*?*/ 				if( !xPictureStorage.Is() )
/*?*/ 				{
/*?*/ 					const String aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );
/*?*/ 
/*?*/ 					if( pStor->IsContained( aPictureStorageName ) &&
/*?*/ 						pStor->IsStorage( aPictureStorageName )  )
/*?*/ 					{
/*?*/ 						( (ScDrawLayer*) this )->xPictureStorage = pStor->OpenUCBStorage( aPictureStorageName, STREAM_READ | STREAM_WRITE );
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				if( xPictureStorage.Is() &&
/*?*/ 					xPictureStorage->IsContained( aPictureStreamName ) &&
/*?*/ 					xPictureStorage->IsStream( aPictureStreamName ) )
/*?*/ 				{
/*?*/ 					pRet = xPictureStorage->OpenStream( aPictureStreamName );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pRet = pStor->OpenStream( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_SCSTREAM)),
/*N*/ 									  STREAM_READ | STREAM_WRITE | STREAM_TRUNC );
/*N*/ 
/*N*/ 			if( pRet )
/*N*/ 			{
/*N*/ 				pRet->SetVersion( pStor->GetVersion() );
/*N*/ 				pRet->SetKey( pStor->GetKey() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		rStreamInfo.mbDeleteAfterUse = ( pRet != NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void ScDrawLayer::ReleasePictureStorage()
/*N*/ {
/*N*/ 	xPictureStorage.Clear();
/*N*/ }




}
