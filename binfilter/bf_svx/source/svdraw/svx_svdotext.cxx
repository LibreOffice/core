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

#include "svditext.hxx"
#include "svdview.hxx"  // Objekt gerade editiert wird
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svdoutl.hxx"
#include "svdstr.hrc"   // Objektname
#include "svdtxhdl.hxx"  // DrawTextToPath
#include "writingmodeitem.hxx"



#ifndef _EDITSTAT_HXX //autogen
#include <editstat.hxx>
#endif

#ifndef _OUTLOBJ_HXX //autogen
#include <outlobj.hxx>
#endif

#ifndef _EDITOBJ_HXX //autogen
#include <editobj.hxx>
#endif


#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <bf_svtools/itempool.hxx>
#endif

#ifndef _SVX_ADJITEM_HXX //autogen
#include "adjitem.hxx"
#endif


#ifndef _SVX_ITEMDATA_HXX
#include "itemdata.hxx"
#endif

#include "flditem.hxx"


#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>		// FRound
#endif

#ifndef _SVX_XFLGRIT_HXX
#include "xflgrit.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

#ifndef _B2D_MATRIX3D_HXX
#include <bf_goodies/matrix3d.hxx>
#endif

namespace binfilter {

// #104018# replace macros above with type-safe methods
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrTextObj,SdrAttrObj);

/*N*/ SdrTextObj::SdrTextObj():
/*N*/ 	eTextKind(OBJ_TEXT),
/*N*/ 	pOutlinerParaObject(NULL),
/*N*/ 	pEdtOutl(NULL),
/*N*/ 	pFormTextBoundRect(NULL)
/*N*/ {
/*N*/ 	bTextSizeDirty=FALSE;
/*N*/ 	bPortionInfoChecked=FALSE;
/*N*/ 	bTextFrame=FALSE;
/*N*/ 	bNoShear=FALSE;
/*N*/ 	bNoRotate=FALSE;
/*N*/ 	bNoMirror=FALSE;
/*N*/ 	bDisableAutoWidthOnDragging=FALSE;
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	mbInEditMode = FALSE;
/*N*/ 
/*N*/ 	// #108784#
/*N*/ 	maTextEditOffset = Point(0, 0);
/*N*/ }

/*N*/ SdrTextObj::SdrTextObj(const Rectangle& rNewRect):
/*N*/ 	aRect(rNewRect),
/*N*/ 	eTextKind(OBJ_TEXT),
/*N*/ 	pOutlinerParaObject(NULL),
/*N*/ 	pEdtOutl(NULL),
/*N*/ 	pFormTextBoundRect(NULL)
/*N*/ {
/*N*/ 	bTextSizeDirty=FALSE;
/*N*/ 	bPortionInfoChecked=FALSE;
/*N*/ 	bTextFrame=FALSE;
/*N*/ 	bNoShear=FALSE;
/*N*/ 	bNoRotate=FALSE;
/*N*/ 	bNoMirror=FALSE;
/*N*/ 	bDisableAutoWidthOnDragging=FALSE;
/*N*/ 	ImpJustifyRect(aRect);
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	mbInEditMode = FALSE;
/*N*/ 
/*N*/ 	// #108784#
/*N*/ 	maTextEditOffset = Point(0, 0);
/*N*/ }

/*N*/ SdrTextObj::SdrTextObj(SdrObjKind eNewTextKind):
/*N*/ 	eTextKind(eNewTextKind),
/*N*/ 	pOutlinerParaObject(NULL),
/*N*/ 	pEdtOutl(NULL),
/*N*/ 	pFormTextBoundRect(NULL)
/*N*/ {
/*N*/ 	bTextSizeDirty=FALSE;
/*N*/ 	bTextFrame=TRUE;
/*N*/ 	bNoShear=TRUE;
/*N*/ 	bNoRotate=FALSE;
/*N*/ 	bNoMirror=TRUE;
/*N*/ 	bPortionInfoChecked=FALSE;
/*N*/ 	bDisableAutoWidthOnDragging=FALSE;
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	mbInEditMode = FALSE;
/*N*/ 
/*N*/ 	// #108784#
/*N*/ 	maTextEditOffset = Point(0, 0);
/*N*/ }

/*N*/ SdrTextObj::SdrTextObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect):
/*N*/ 	aRect(rNewRect),
/*N*/ 	eTextKind(eNewTextKind),
/*N*/ 	pOutlinerParaObject(NULL),
/*N*/ 	pEdtOutl(NULL),
/*N*/ 	pFormTextBoundRect(NULL)
/*N*/ {
/*N*/ 	bTextSizeDirty=FALSE;
/*N*/ 	bTextFrame=TRUE;
/*N*/ 	bNoShear=TRUE;
/*N*/ 	bNoRotate=FALSE;
/*N*/ 	bNoMirror=TRUE;
/*N*/ 	bPortionInfoChecked=FALSE;
/*N*/ 	bDisableAutoWidthOnDragging=FALSE;
/*N*/ 	ImpJustifyRect(aRect);
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	mbInEditMode = FALSE;
/*N*/ 
/*N*/ 	// #108784#
/*N*/ 	maTextEditOffset = Point(0, 0);
/*N*/ }

/*N*/ SdrTextObj::~SdrTextObj()
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		SdrOutliner& rOutl = pModel->GetHitTestOutliner();
/*N*/ 		if( rOutl.GetTextObj() == this )
/*N*/ 			rOutl.SetTextObj( NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*N*/ 		delete pOutlinerParaObject;
/*N*/ 	}
/*N*/ 	if (pFormTextBoundRect!=NULL) {
/*N*/ 		delete pFormTextBoundRect;
/*N*/ 	}
/*N*/ 	ImpLinkAbmeldung();
/*N*/ }

/*N*/ const Size& SdrTextObj::GetTextSize() const
/*N*/ {
/*N*/ 	if (bTextSizeDirty) {
/*N*/ 		Size aSiz;
/*N*/ 		if (pOutlinerParaObject!=NULL) {
/*N*/ 			SdrOutliner& rOutliner=ImpGetDrawOutliner();
/*N*/ 			rOutliner.SetText(*pOutlinerParaObject);
/*N*/ 			rOutliner.SetUpdateMode(TRUE);
/*N*/ 			aSiz=rOutliner.CalcTextSize();
/*N*/ 			rOutliner.Clear();
/*N*/ 		}
/*N*/ 		// 2x casting auf nonconst
/*N*/ 		((SdrTextObj*)this)->aTextSize=aSiz;
/*N*/ 		((SdrTextObj*)this)->bTextSizeDirty=FALSE;
/*N*/ 	}
/*N*/ 	return aTextSize;
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::IsAutoGrowHeight() const
/*N*/ {
/*N*/ 	if(!bTextFrame)
/*N*/ 		return FALSE; // AutoGrow nur bei TextFrames
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	BOOL bRet = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
/*N*/ 
/*N*/ 	if(bRet)
/*N*/ 	{
/*N*/ 		SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();
/*N*/ 
/*N*/ 		if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
/*N*/ 		{
/*N*/ 			SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
/*N*/ 
/*N*/ 			if(eDirection == SDRTEXTANI_UP || eDirection == SDRTEXTANI_DOWN)
/*N*/ 			{
/*N*/ 				bRet = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::IsAutoGrowWidth() const
/*N*/ {
/*N*/ 	if(!bTextFrame)
/*N*/ 		return FALSE; // AutoGrow nur bei TextFrames
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	BOOL bRet = ((SdrTextAutoGrowHeightItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue();
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	BOOL bInEditMOde = IsInEditMode();
/*N*/ 
/*N*/ 	if(!bInEditMOde && bRet)
/*N*/ 	{
/*N*/ 		SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();
/*N*/ 
/*N*/ 		if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
/*N*/ 		{
/*?*/ 			SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
/*?*/ 
/*?*/ 			if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
/*?*/ 			{
/*?*/ 				bRet = FALSE;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ SdrTextHorzAdjust SdrTextObj::GetTextHorizontalAdjust() const
/*N*/ {
/*N*/ 	if(IsContourTextFrame())
/*?*/ 		return SDRTEXTHORZADJUST_BLOCK;
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrTextHorzAdjust eRet = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
/*N*/ 
/*N*/ 	// #101684#
/*N*/ 	BOOL bInEditMode = IsInEditMode();
/*N*/ 
/*N*/ 	if(!bInEditMode && eRet == SDRTEXTHORZADJUST_BLOCK)
/*N*/ 	{
/*N*/ 		SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();
/*N*/ 
/*N*/ 		if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
/*N*/ 		{
/*?*/ 			SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
/*?*/ 
/*?*/ 			if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
/*?*/ 			{
/*?*/ 				eRet = SDRTEXTHORZADJUST_LEFT;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return eRet;
/*N*/ } // defaults: BLOCK fuer Textrahmen, CENTER fuer beschriftete Grafikobjekte

/*N*/ SdrTextVertAdjust SdrTextObj::GetTextVerticalAdjust() const
/*N*/ {
/*N*/ 	if(IsContourTextFrame())
/*?*/ 		return SDRTEXTVERTADJUST_TOP;
/*N*/ 
/*N*/ 	// #103516# Take care for vertical text animation here
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrTextVertAdjust eRet = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();
/*N*/ 	BOOL bInEditMode = IsInEditMode();
/*N*/ 
/*N*/ 	// #103516# Take care for vertical text animation here
/*N*/ 	if(!bInEditMode && eRet == SDRTEXTVERTADJUST_BLOCK)
/*N*/ 	{
/*N*/ 		SdrTextAniKind eAniKind = ((SdrTextAniKindItem&)(rSet.Get(SDRATTR_TEXT_ANIKIND))).GetValue();
/*N*/ 
/*N*/ 		if(eAniKind == SDRTEXTANI_SCROLL || eAniKind == SDRTEXTANI_ALTERNATE || eAniKind == SDRTEXTANI_SLIDE)
/*N*/ 		{
/*N*/ 			SdrTextAniDirection eDirection = ((SdrTextAniDirectionItem&)(rSet.Get(SDRATTR_TEXT_ANIDIRECTION))).GetValue();
/*N*/ 
/*N*/ 			if(eDirection == SDRTEXTANI_LEFT || eDirection == SDRTEXTANI_RIGHT)
/*N*/ 			{
/*N*/ 				eRet = SDRTEXTVERTADJUST_TOP;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return eRet;
/*N*/ } // defaults: TOP fuer Textrahmen, CENTER fuer beschriftete Grafikobjekte

/*N*/ void SdrTextObj::ImpJustifyRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	if (!rRect.IsEmpty()) {
/*N*/ 		rRect.Justify();
/*N*/ 		if (rRect.Left()==rRect.Right()) rRect.Right()++;
/*N*/ 		if (rRect.Top()==rRect.Bottom()) rRect.Bottom()++;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrTextObj::ImpCheckShear()
/*N*/ {
/*N*/ 	if (bNoShear && aGeo.nShearWink!=0) {
/*N*/ 		aGeo.nShearWink=0;
/*N*/ 		aGeo.nTan=0;
/*N*/ 	}
/*N*/ }



/*N*/ FASTBOOL SdrTextObj::HasEditText() const
/*N*/ {
/*N*/ 	FASTBOOL bRet=FALSE;
/*N*/ 	if (pEdtOutl!=NULL) {
/*N*/ 		Paragraph* p1stPara=pEdtOutl->GetParagraph( 0 );
/*N*/ 		ULONG nParaAnz=pEdtOutl->GetParagraphCount();
/*N*/ 		if (p1stPara==NULL) nParaAnz=0;
/*N*/ 		if (nParaAnz==1) { // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
/*N*/ 			XubString aStr(pEdtOutl->GetText(p1stPara));
/*N*/ 
/*N*/ 			// Aha, steht nix drin!
/*N*/ 			if(!aStr.Len())
/*N*/ 				nParaAnz = 0;
/*N*/ 		}
/*N*/ 		bRet=nParaAnz!=0;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void SdrTextObj::SetPage(SdrPage* pNewPage)
/*N*/ {
/*N*/ 	FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
/*N*/ 	FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;
/*N*/ 	FASTBOOL bLinked=IsLinkedText();
/*N*/ 
/*N*/ 	if (bLinked && bRemove) {
/*?*/ 		ImpLinkAbmeldung();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrAttrObj::SetPage(pNewPage);
/*N*/ 
/*N*/ 	if (bLinked && bInsert) {
/*?*/ 		ImpLinkAnmeldung();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrTextObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrModel* pOldModel=pModel;
/*N*/ 	BOOL bLinked=IsLinkedText();
/*N*/ 	BOOL bChg=pNewModel!=pModel;
/*N*/ 	BOOL bHgtSet = rSet.GetItemState(EE_CHAR_FONTHEIGHT, TRUE) == SFX_ITEM_SET;
/*N*/ 	if (bLinked && bChg) {
/*?*/ 		ImpLinkAbmeldung();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrAttrObj::SetModel(pNewModel);
/*N*/ 
/*N*/ 	if (bChg && pOutlinerParaObject!=NULL && pOldModel!=NULL && pNewModel!=NULL) {
/*?*/ 		MapUnit aOldUnit(pOldModel->GetScaleUnit());
/*?*/ 		MapUnit aNewUnit(pNewModel->GetScaleUnit());
/*?*/ 		FASTBOOL bScaleUnitChanged=aNewUnit!=aOldUnit;
/*?*/ 		SetTextSizeDirty();
/*?*/ 		// und nun dem OutlinerParaObject einen neuen Pool verpassen
/*?*/ 		// !!! Hier muss noch DefTab und RefDevice der beiden Models
/*?*/ 		// !!! verglichen werden und dann ggf. AutoGrow zuschlagen !!!
/*?*/ 		// !!! fehlende Implementation !!!
/*?*/ 		ULONG nOldFontHgt=pOldModel->GetDefaultFontHeight();
/*?*/ 		ULONG nNewFontHgt=pNewModel->GetDefaultFontHeight();
/*?*/ 		BOOL bDefHgtChanged=nNewFontHgt!=nOldFontHgt;
/*?*/ 		BOOL bSetHgtItem=bDefHgtChanged && !bHgtSet;
/*?*/ 		if (bSetHgtItem) { // #32665#
/*?*/ 			// zunaechst das HeightItem festklopfen, damit
/*?*/ 			// 1. Es eben bestehen bleibt und
/*?*/ 			// 2. DoStretchChars vom richtigen Wert ausgeht
/*?*/ 			SetItem(SvxFontHeightItem(nOldFontHgt));
/*?*/ 		}
/*?*/ 		// erst jetzt den Outliner holen, etc. damit obiges SetAttr auch wirkt
/*?*/ 		SdrOutliner& rOutliner=ImpGetDrawOutliner();
/*?*/ 		rOutliner.SetText(*pOutlinerParaObject);
/*?*/ 		delete pOutlinerParaObject;
/*?*/ 		pOutlinerParaObject=NULL;
/*?*/ 		if (bScaleUnitChanged) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 		}
/*?*/ 		SetOutlinerParaObject(rOutliner.CreateParaObject()); // #34494#
/*?*/ 		pOutlinerParaObject->ClearPortionInfo();
/*?*/ 		bPortionInfoChecked=FALSE;
/*?*/ 		rOutliner.Clear();
/*?*/ 		//ImpSetTextStyleSheetListeners();
/*N*/ 	}

/*N*/ 	if (bLinked && bChg) {
/*?*/ 		ImpLinkAnmeldung();
/*N*/ 	}
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::NbcSetEckenradius(long nRad)
/*N*/ {
/*N*/ 	SetItem(SdrEckenradiusItem(nRad));
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::NbcSetAutoGrowHeight(FASTBOOL bAuto)
/*N*/ {
/*N*/ 	if(bTextFrame)
/*N*/ 	{
/*N*/ 		SetItem(SdrTextAutoGrowHeightItem(bAuto));
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ FASTBOOL SdrTextObj::NbcSetMinTextFrameHeight(long nHgt)
/*N*/ {
/*N*/ 	if(bTextFrame)
/*N*/ 	{
/*N*/ 		SetItem(SdrTextMinFrameHeightItem(nHgt));
/*N*/ 
/*N*/ 		// #84974# use bDisableAutoWidthOnDragging as
/*N*/ 		// bDisableAutoHeightOnDragging if vertical.
/*N*/ 		if(IsVerticalWriting() && bDisableAutoWidthOnDragging)
/*N*/ 		{
/*?*/ 			bDisableAutoWidthOnDragging = FALSE;
/*?*/ 			SetItem(SdrTextAutoGrowHeightItem(FALSE));
/*N*/ 		}
/*N*/ 
/*N*/ 		return TRUE;
/*N*/ 	}
/*?*/ 	return FALSE;
/*N*/ }



/*N*/ FASTBOOL SdrTextObj::NbcSetMinTextFrameWidth(long nWdt)
/*N*/ {
/*N*/ 	if(bTextFrame)
/*N*/ 	{
/*N*/ 		SetItem(SdrTextMinFrameWidthItem(nWdt));
/*N*/ 
/*N*/ 		// #84974# use bDisableAutoWidthOnDragging only
/*N*/ 		// when not vertical.
/*N*/ 		if(!IsVerticalWriting() && bDisableAutoWidthOnDragging)
/*N*/ 		{
/*?*/ 			bDisableAutoWidthOnDragging = FALSE;
/*?*/ 			SetItem(SdrTextAutoGrowWidthItem(FALSE));
/*N*/ 		}
/*N*/ 
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }




/*N*/ void SdrTextObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	rRect=aRect;
/*N*/ }

/*N*/ void SdrTextObj::TakeTextAnchorRect(Rectangle& rAnchorRect) const
/*N*/ {
/*N*/ 	long nLeftDist=GetTextLeftDistance();
/*N*/ 	long nRightDist=GetTextRightDistance();
/*N*/ 	long nUpperDist=GetTextUpperDistance();
/*N*/ 	long nLowerDist=GetTextLowerDistance();
/*N*/ 	Rectangle aAnkRect(aRect); // Rect innerhalb dem geankert wird
/*N*/ 	FASTBOOL bFrame=IsTextFrame();
/*N*/ 	if (!bFrame) {
/*N*/ 		TakeUnrotatedSnapRect(aAnkRect);
/*N*/ 	}
/*N*/ 	Point aRotateRef(aAnkRect.TopLeft());
/*N*/ 	aAnkRect.Left()+=nLeftDist;
/*N*/ 	aAnkRect.Top()+=nUpperDist;
/*N*/ 	aAnkRect.Right()-=nRightDist;
/*N*/ 	aAnkRect.Bottom()-=nLowerDist;
/*N*/ 
/*N*/ 	// #108816#
/*N*/ 	// Since sizes may be bigger than the object bounds it is necessary to
/*N*/ 	// justify the rect now.
/*N*/ 	ImpJustifyRect(aAnkRect);
/*N*/ 
/*N*/ 	if (bFrame) {
/*N*/ 		// !!! hier noch etwas verfeinern !!!
/*N*/ 		if (aAnkRect.GetWidth()<2) aAnkRect.Right()=aAnkRect.Left()+1;   // Mindestgroesse 2
/*N*/ 		if (aAnkRect.GetHeight()<2) aAnkRect.Bottom()=aAnkRect.Top()+1;  // Mindestgroesse 2
/*N*/ 	}
/*N*/ 	if (aGeo.nDrehWink!=0) {
/*N*/ 		Point aTmpPt(aAnkRect.TopLeft());
/*N*/ 		RotatePoint(aTmpPt,aRotateRef,aGeo.nSin,aGeo.nCos);
/*N*/ 		aTmpPt-=aAnkRect.TopLeft();
/*N*/ 		aAnkRect.Move(aTmpPt.X(),aTmpPt.Y());
/*N*/ 	}
/*N*/ 	rAnchorRect=aAnkRect;
/*N*/ }

/*N*/ void SdrTextObj::TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText,
/*N*/ 	                           Rectangle* pAnchorRect, BOOL bLineWidth ) const
/*N*/ {
/*N*/ 	Rectangle aAnkRect; // Rect innerhalb dem geankert wird
/*N*/ 	TakeTextAnchorRect(aAnkRect);
/*N*/ 	SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
/*N*/ 	SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
/*N*/ 	SdrTextAniKind      eAniKind=GetTextAniKind();
/*N*/ 	SdrTextAniDirection eAniDirection=GetTextAniDirection();
/*N*/ 
/*N*/ 	SdrFitToSizeType eFit=GetFitToSize();
/*N*/ 	FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
/*N*/ 	FASTBOOL bContourFrame=IsContourTextFrame();
/*N*/ 
/*N*/ 	FASTBOOL bFrame=IsTextFrame();
/*N*/ 	ULONG nStat0=rOutliner.GetControlWord();
/*N*/ 	Size aNullSize;
/*N*/ 	if (!bContourFrame)
/*N*/ 	{
/*N*/ 		rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
/*N*/ 		rOutliner.SetMinAutoPaperSize(aNullSize);
/*N*/ 		rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));
/*N*/ 	}
/*N*/ 
/*N*/ 	if (!bFitToSize && !bContourFrame)
/*N*/ 	{
/*N*/ 		long nAnkWdt=aAnkRect.GetWidth();
/*N*/ 		long nAnkHgt=aAnkRect.GetHeight();
/*N*/ 		if (bFrame)
/*N*/ 		{
/*N*/ 			long nWdt=nAnkWdt;
/*N*/ 			long nHgt=nAnkHgt;
/*N*/ 
/*N*/ 			// #101684#
/*N*/ 			BOOL bInEditMode = IsInEditMode();
/*N*/ 
/*N*/ 			if (!bInEditMode && (eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE))
/*N*/ 			{
/*?*/ 				// Grenzenlose Papiergroesse fuer Laufschrift
/*?*/ 				if (eAniDirection==SDRTEXTANI_LEFT || eAniDirection==SDRTEXTANI_RIGHT) nWdt=1000000;
/*?*/ 				if (eAniDirection==SDRTEXTANI_UP || eAniDirection==SDRTEXTANI_DOWN) nHgt=1000000;
/*N*/ 			}
/*N*/ 			rOutliner.SetMaxAutoPaperSize(Size(nWdt,nHgt));
/*N*/ 		}
/*N*/ 
/*N*/ 		// #103516# New try with _BLOCK for hor and ver after completely
/*N*/ 		// supporting full width for vertical text.
/*N*/ 		if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
/*N*/ 		{
/*N*/ 			rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
/*N*/ 		}
/*N*/ 
/*N*/ 		if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
/*N*/ 		{
/*N*/ 			rOutliner.SetMinAutoPaperSize(Size(0, nAnkHgt));
/*N*/ 		}
/*N*/ 
/*N*/ 		// #103335# back to old solution, thus #100801# will be back and needs to be solved in
/*N*/ 		// another way.
//		if (eHAdj==SDRTEXTHORZADJUST_BLOCK)
//		{
//			if(IsVerticalWriting())
//				rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, nAnkHgt));
//			else
//				rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
//		}

//		// #100801# MinAutoPaperSize needs always to be set completely
//		// when Verical
//		if(IsVerticalWriting())
//		{
//			rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, nAnkHgt));
//		}
//
//		if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
//		{
//			// #89459#
//			if(!IsVerticalWriting())
//			{
//				rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
//			}
//		}
/*N*/ 	}
/*N*/ 
/*N*/ 	rOutliner.SetPaperSize(aNullSize);
/*N*/ 	if (bContourFrame)
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ImpSetContourPolygon( rOutliner, aAnkRect, bLineWidth );
/*N*/ 
/*N*/ 	// Text in den Outliner stecken - ggf. den aus dem EditOutliner
/*N*/ 	OutlinerParaObject* pPara=pOutlinerParaObject;
/*N*/ 	if (pEdtOutl && !bNoEditText)
/*?*/ 		pPara=pEdtOutl->CreateParaObject();
/*N*/ 
/*N*/ 	if (pPara)
/*N*/ 	{
/*N*/ 		BOOL bHitTest = FALSE;
/*N*/ 		if( pModel )
/*N*/ 			bHitTest = &pModel->GetHitTestOutliner() == &rOutliner;
/*N*/ 
/*N*/ 		const SdrTextObj* pTestObj = rOutliner.GetTextObj();
/*N*/ 		if( !pTestObj || !bHitTest || pTestObj != this ||
/*N*/ 		    pTestObj->GetOutlinerParaObject() != pOutlinerParaObject )
/*N*/ 		{
/*N*/ 			if( bHitTest )
/*N*/ 				rOutliner.SetTextObj( this );
/*N*/ 
/*N*/ 			rOutliner.SetUpdateMode(TRUE);
/*N*/ 			rOutliner.SetText(*pPara);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ //		rOutliner.Clear();
/*?*/ 		rOutliner.SetTextObj( NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pEdtOutl && !bNoEditText && pPara)
/*?*/ 		delete pPara;
/*N*/ 
/*N*/ 	rOutliner.SetUpdateMode(TRUE);
/*N*/ 	rOutliner.SetControlWord(nStat0);
/*N*/ 
/*N*/ 	if (!bPortionInfoChecked)
/*N*/ 	{
/*N*/ 		// Optimierung: ggf. BigTextObject erzeugen
/*N*/ 		((SdrTextObj*)this)->bPortionInfoChecked=TRUE;
/*N*/ 		if (pOutlinerParaObject!=NULL && rOutliner.ShouldCreateBigTextObject())
/*?*/ 			((SdrTextObj*)this)->pOutlinerParaObject=rOutliner.CreateParaObject();
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aTextPos(aAnkRect.TopLeft());
/*N*/ 	Size aTextSiz(rOutliner.GetPaperSize()); // GetPaperSize() hat etwas Toleranz drauf, oder?
/*N*/ 
/*N*/ 	// #106653#
/*N*/ 	// For draw objects containing text correct hor/ver alignment if text is bigger
/*N*/ 	// than the object itself. Without that correction, the text would always be
/*N*/ 	// formatted to the left edge (or top edge when vertical) of the draw object.
/*N*/ 	if(!IsTextFrame())
/*N*/ 	{
/*N*/ 		if(aAnkRect.GetWidth() < aTextSiz.Width() && !IsVerticalWriting())
/*N*/ 		{
/*N*/ 			// #110129#
/*N*/ 			// Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
/*N*/ 			// else the alignment is wanted.
/*N*/ 			if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
/*N*/ 			{
/*N*/ 				eHAdj = SDRTEXTHORZADJUST_CENTER;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aAnkRect.GetHeight() < aTextSiz.Height() && IsVerticalWriting())
/*N*/ 		{
/*N*/ 			// #110129#
/*N*/ 			// Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
/*N*/ 			// else the alignment is wanted.
/*N*/ 			if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
/*N*/ 			{
/*N*/ 				eVAdj = SDRTEXTVERTADJUST_CENTER;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (eHAdj==SDRTEXTHORZADJUST_CENTER || eHAdj==SDRTEXTHORZADJUST_RIGHT)
/*N*/ 	{
/*N*/ 		long nFreeWdt=aAnkRect.GetWidth()-aTextSiz.Width();
/*N*/ 		if (eHAdj==SDRTEXTHORZADJUST_CENTER)
/*N*/ 			aTextPos.X()+=nFreeWdt/2;
/*N*/ 		if (eHAdj==SDRTEXTHORZADJUST_RIGHT)
/*N*/ 			aTextPos.X()+=nFreeWdt;
/*N*/ 	}
/*N*/ 	if (eVAdj==SDRTEXTVERTADJUST_CENTER || eVAdj==SDRTEXTVERTADJUST_BOTTOM)
/*N*/ 	{
/*N*/ 		long nFreeHgt=aAnkRect.GetHeight()-aTextSiz.Height();
/*N*/ 		if (eVAdj==SDRTEXTVERTADJUST_CENTER)
/*N*/ 			aTextPos.Y()+=nFreeHgt/2;
/*N*/ 		if (eVAdj==SDRTEXTVERTADJUST_BOTTOM)
/*N*/ 			aTextPos.Y()+=nFreeHgt;
/*N*/ 	}
/*N*/ 	if (aGeo.nDrehWink!=0)
/*N*/ 		RotatePoint(aTextPos,aAnkRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 
/*N*/ 	if (pAnchorRect)
/*N*/ 		*pAnchorRect=aAnkRect;
/*N*/ 
/*N*/ 	// rTextRect ist bei ContourFrame in einigen Faellen nicht korrekt
/*N*/ 	rTextRect=Rectangle(aTextPos,aTextSiz);
/*N*/ 	if (bContourFrame)
/*?*/ 		rTextRect=aAnkRect;
/*N*/ }

/*N*/ OutlinerParaObject* SdrTextObj::GetEditOutlinerParaObject() const
/*N*/ {
/*N*/ 	OutlinerParaObject* pPara=NULL;
/*N*/ 	if (pEdtOutl!=NULL) { // Wird gerade editiert, also das ParaObject aus dem aktiven Editor verwenden
/*?*/ 		Paragraph* p1stPara=pEdtOutl->GetParagraph( 0 );
/*?*/ 		ULONG nParaAnz=pEdtOutl->GetParagraphCount();
/*?*/ 		if (nParaAnz==1 && p1stPara!=NULL) { // bei nur einem Para nachsehen ob da ueberhaupt was drin steht
/*?*/ 			XubString aStr(pEdtOutl->GetText(p1stPara));
/*?*/ 
/*?*/ 			// Aha, steht nix drin!
/*?*/ 			if(!aStr.Len())
/*?*/ 				nParaAnz = 0;
/*?*/ 		}
/*?*/ 		if (p1stPara!=NULL && nParaAnz!=0) {
/*?*/ 			pPara = pEdtOutl->CreateParaObject(0, (sal_uInt16)nParaAnz);
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return pPara;
/*N*/ }



// Geht z.Zt. nur wenn das Obj schon wenigstens einmal gepaintet wurde
// Denn dann ist der MtfAnimator initiallisiert




/*N*/ void SdrTextObj::ImpAddTextToBoundRect()
/*N*/ {
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*N*/ 		if (IsContourTextFrame()) return;
/*N*/ 		if (IsFontwork()) {
/*N*/ 			if (pModel!=NULL) {
/*N*/ 				VirtualDevice aVD;
/*N*/ 				ExtOutputDevice aXOut(&aVD);
/*N*/ 				SdrOutliner& rOutl=ImpGetDrawOutliner();
/*N*/ 				rOutl.SetUpdateMode(TRUE);
/*N*/ 				ImpTextPortionHandler aTPHandler(rOutl,*this);
/*N*/ 
/*N*/ 				aXOut.SetTextAttr(GetItemSet());
/*N*/ 
/*N*/ 				aTPHandler.DrawTextToPath(aXOut,FALSE);
/*N*/ 				if (pFormTextBoundRect==NULL) pFormTextBoundRect=new Rectangle;
/*N*/ 				*pFormTextBoundRect=aTPHandler.GetFormTextBoundRect();
/*N*/ 				aOutRect.Union(*pFormTextBoundRect);
/*N*/ 			}
/*N*/ 		} else { // Ansonsten Text im Zeichenobjekt zentriert
/*N*/ 			if (pFormTextBoundRect!=NULL) {
/*?*/ 				delete pFormTextBoundRect;
/*?*/ 				pFormTextBoundRect=NULL;
/*N*/ 			}
/*N*/ 			FASTBOOL bCheckText=TRUE;
/*N*/ 			if (bTextFrame) {
/*N*/ 				bCheckText=GetTextLeftDistance ()<0 ||
/*N*/ 						   GetTextRightDistance()<0 ||
/*N*/ 						   GetTextUpperDistance()<0 ||
/*N*/ 						   GetTextLowerDistance()<0 ||
/*N*/ 						   (GetEckenradius()>0 && aGeo.nDrehWink!=0);
/*N*/ 			}
/*N*/ 			if (bCheckText) {
/*N*/ 				SdrOutliner& rOutliner=ImpGetDrawOutliner();
/*N*/ 				Rectangle aTextRect;
/*N*/ 				Rectangle aAnchorRect;
/*N*/ 				TakeTextRect(rOutliner,aTextRect,TRUE,&aAnchorRect); // EditText ignorieren!
/*N*/ 				SdrFitToSizeType eFit=GetFitToSize();
/*N*/ 				FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
/*N*/ 				if (bFitToSize) aTextRect=aAnchorRect;
/*N*/ 				rOutliner.Clear();
/*N*/ 				if (aGeo.nDrehWink!=0) {
/*N*/ 					Polygon aPol(aTextRect);
/*N*/ 					if (aGeo.nDrehWink!=0) RotatePoly(aPol,aTextRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 					aOutRect.Union(aPol.GetBoundRect());
/*N*/ 				} else {
/*N*/ 					aOutRect.Union(aTextRect);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SdrObject* SdrTextObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	if (!bTextFrame && pOutlinerParaObject==NULL) return NULL;
/*N*/ 	if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
/*N*/ 	INT32 nMyTol=nTol;
/*N*/ 	FASTBOOL bFontwork=IsFontwork();
/*N*/ 	SdrFitToSizeType eFit=GetFitToSize();
/*N*/ 	FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
/*N*/ 	Rectangle aR(aRect);
/*N*/ 	Rectangle aAnchor(aR);
/*N*/ 	Rectangle aTextRect(aR);
/*N*/ 	SdrOutliner* pOutliner = NULL;
/*N*/ 	pOutliner = &pModel->GetHitTestOutliner();
/*N*/ 
/*N*/ 	if (bFontwork) {
/*?*/ 		if (pFormTextBoundRect!=NULL) aR=*pFormTextBoundRect;
/*?*/ 		else aR=GetBoundRect();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		TakeTextRect( *pOutliner, aTextRect, FALSE, &aAnchor, FALSE ); // EditText nicht mehr ignorieren! TRUE); // EditText ignorieren!
/*N*/ 
/*N*/ 		if (bFitToSize)
/*?*/ 			aR=aAnchor;
/*N*/ 		else
/*N*/ 			aR=aTextRect;
/*N*/ 	}
/*N*/ 	if (aR.GetWidth()-1>short(nTol) && aR.GetHeight()-1>short(nTol)) nMyTol=0; // Keine Toleranz noetig hier
/*N*/ 	if (nMyTol!=0) {
/*?*/ 		aR.Left  ()-=nMyTol;
/*?*/ 		aR.Top   ()-=nMyTol;
/*?*/ 		aR.Right ()+=nMyTol;
/*?*/ 		aR.Bottom()+=nMyTol;
/*N*/ 	}
/*N*/ 	FASTBOOL bRet=FALSE;
/*N*/ 
/*N*/ 	if(bFontwork)
/*N*/ 	{
/*N*/ 		bRet = aR.IsInside(rPnt);
/*N*/ 
/*N*/ 		// #105130# Include aRect here in measurements to be able to hit a
/*N*/ 		// fontwork object on its border
/*N*/ 		if(!bRet)
/*N*/ 		{
/*N*/ 			const Rectangle aSnapRect = GetSnapRect();
/*N*/ 
/*N*/ 			if(	(rPnt.X() >= aSnapRect.Left() - nTol && rPnt.X() <= aSnapRect.Left() + nTol)
/*N*/ 			 || (rPnt.X() >= aSnapRect.Right() - nTol && rPnt.X() <= aSnapRect.Right() + nTol)
/*N*/ 			 || (rPnt.Y() >= aSnapRect.Top() - nTol && rPnt.Y() <= aSnapRect.Top() + nTol)
/*N*/ 			 || (rPnt.Y() >= aSnapRect.Bottom() - nTol && rPnt.Y() <= aSnapRect.Bottom() + nTol))
/*N*/ 			{
/*N*/ 				bRet = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (aGeo.nDrehWink!=0) {
/*N*/ 			Polygon aPol(aR);
/*N*/ 			RotatePoly(aPol,aR.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 			bRet=IsPointInsidePoly(aPol,rPnt);
/*N*/ 		} else {
/*N*/ 			bRet=aR.IsInside(rPnt);
/*N*/ 		}
/*N*/ 		if (bRet) { // und nun noch checken, ob wirklich Buchstaben getroffen sind
/*N*/ 			// Featurewunsch zur 4.0
/*N*/ 			// Zunaechst meine Dok-Koordinaten in EE-Dok-Koordinaten umwandeln.
/*N*/ 			Point aPt(rPnt); aPt-=aR.TopLeft();
/*N*/ 			if (bFitToSize) { // #38214#: FitToSize berueksichtigen
/*?*/ 				Fraction aX(aTextRect.GetWidth()-1,aAnchor.GetWidth()-1);
/*?*/ 				Fraction aY(aTextRect.GetHeight()-1,aAnchor.GetHeight()-1);
/*?*/ 				ResizePoint(aPt,Point(),aX,aY);
/*N*/ 			}
/*N*/ 			if (aGeo.nDrehWink!=0) RotatePoint(aPt,Point(),-aGeo.nSin,aGeo.nCos); // -sin fuer Unrotate
/*N*/ 			// Und nun im EE-Dok auf Buchstabensuche gehen
/*N*/ 			long nHitTol = 2000;
/*N*/ 			OutputDevice* pRef = pOutliner->GetRefDevice();
/*N*/ 			if( pRef )
/*N*/ 				nHitTol = pRef->LogicToLogic( nHitTol, MAP_100TH_MM, pRef->GetMapMode().GetMapUnit() );
/*N*/ 
/*N*/ 			bRet = pOutliner->IsTextPos( aPt, (sal_uInt16)nHitTol );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet ? (SdrObject*)this : NULL;
/*N*/ }



/*N*/ void SdrTextObj::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	SdrAttrObj::operator=(rObj);
/*N*/ 	const SdrTextObj* pText=PTR_CAST(SdrTextObj,&rObj);
/*N*/ 	if (pText!=NULL) {
/*N*/ 		aRect     =pText->aRect;
/*N*/ 		aGeo      =pText->aGeo;
/*N*/ 		eTextKind =pText->eTextKind;
/*N*/ 		bTextFrame=pText->bTextFrame;
/*N*/ 		aTextSize=pText->aTextSize;
/*N*/ 		bTextSizeDirty=pText->bTextSizeDirty;
/*N*/ 
/*N*/ 		// #101776# Not all of the necessary parameters were copied yet.
/*N*/ 		bNoShear = pText->bNoShear;
/*N*/ 		bNoRotate = pText->bNoRotate;
/*N*/ 		bNoMirror = pText->bNoMirror;
/*N*/ 		bDisableAutoWidthOnDragging = pText->bDisableAutoWidthOnDragging;
/*N*/ 
/*N*/ 		if (pOutlinerParaObject!=NULL) delete pOutlinerParaObject;
/*N*/ 		if (pText->HasText()) {
/*N*/ 			const Outliner* pEO=pText->pEdtOutl;
/*N*/ 			if (pEO!=NULL) {
/*?*/ 				pOutlinerParaObject=pEO->CreateParaObject();
/*N*/ 			} else {
/*N*/ 				pOutlinerParaObject=pText->pOutlinerParaObject->Clone();
/*N*/ 			}
/*N*/ 		} else {
/*N*/ 			pOutlinerParaObject=NULL;
/*N*/ 		}
/*N*/ 		ImpSetTextStyleSheetListeners();
/*N*/ 	}
/*N*/ }


/*N*/ void SdrTextObj::TakeContour(XPolyPolygon& rPoly) const
/*N*/ {
/*N*/ 	SdrAttrObj::TakeContour(rPoly);
/*N*/ 
/*N*/ 	// #80328# using Clone()-Paint() strategy inside TakeContour() leaves a destroyed
/*N*/ 	// SdrObject as pointer in DrawOutliner. Set *this again in fetching the outliner
/*N*/ 	// in every case
/*N*/ 	SdrOutliner& rOutliner=ImpGetDrawOutliner();
/*N*/ 
/*N*/ 	// und nun noch ggf. das BoundRect des Textes dazu
/*N*/ 	if (pOutlinerParaObject!=NULL && !IsFontwork() && !IsContourTextFrame()) {
/*?*/ 		Rectangle aAnchor;
/*?*/ 		Rectangle aR;
/*?*/ 		TakeTextRect(rOutliner,aR,FALSE,&aAnchor);
/*?*/ 		rOutliner.Clear();
/*?*/ 		SdrFitToSizeType eFit=GetFitToSize();
/*?*/ 		FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
/*?*/ 		if (bFitToSize) aR=aAnchor;
/*?*/ 		Polygon aPol(aR);
/*?*/ 		if (aGeo.nDrehWink!=0) RotatePoly(aPol,aR.TopLeft(),aGeo.nSin,aGeo.nCos);
/*?*/ 		rPoly.Insert(XPolygon(aPol));
/*N*/ 	}
/*N*/ }


/*N*/ void SdrTextObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) {
/*N*/ 		Polygon aPol(aRect);
/*N*/ 		if (aGeo.nShearWink!=0) ShearPoly(aPol,aRect.TopLeft(),aGeo.nTan);
/*N*/ 		if (aGeo.nDrehWink!=0) RotatePoly(aPol,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 		maSnapRect=aPol.GetBoundRect();
/*N*/ 	} else {
/*N*/ 		maSnapRect=aRect;
/*N*/ 	}
/*N*/ }



/*N*/ void SdrTextObj::ImpCheckMasterCachable()
/*N*/ {
/*N*/ 	bNotMasterCachable=FALSE;
/*N*/ 	if (!bNotVisibleAsMaster && pOutlinerParaObject!=NULL && pOutlinerParaObject->IsEditDoc()) {
/*N*/ 		const EditTextObject& rText=pOutlinerParaObject->GetTextObject();
/*N*/ 		bNotMasterCachable=rText.HasField(SvxPageField::StaticType());
/*N*/ 	}
/*N*/ }

// #101029#: Extracted from ImpGetDrawOutliner()
/*N*/ void SdrTextObj::ImpInitDrawOutliner( SdrOutliner& rOutl ) const
/*N*/ {
/*N*/ 	rOutl.SetUpdateMode(FALSE);
/*N*/ 	USHORT nOutlinerMode = OUTLINERMODE_OUTLINEOBJECT;
/*N*/ 	if ( !IsOutlText() )
/*N*/ 		nOutlinerMode = OUTLINERMODE_TEXTOBJECT;
/*N*/ 	rOutl.Init( nOutlinerMode );
/*N*/ 
/*N*/ 	rOutl.SetGlobalCharStretching(100,100);
/*N*/ 	ULONG nStat=rOutl.GetControlWord();
/*N*/ 	nStat&=~(EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE);
/*N*/ 	rOutl.SetControlWord(nStat);
/*N*/ 	Size aNullSize;
/*N*/ 	Size aMaxSize(100000,100000);
/*N*/ 	rOutl.SetMinAutoPaperSize(aNullSize);
/*N*/ 	rOutl.SetMaxAutoPaperSize(aMaxSize);
/*N*/ 	rOutl.SetPaperSize(aMaxSize);
/*N*/ 	rOutl.ClearPolygon();
/*N*/ }

/*N*/ SdrOutliner& SdrTextObj::ImpGetDrawOutliner() const
/*N*/ {
/*N*/ 	SdrOutliner& rOutl=pModel->GetDrawOutliner(this);
/*N*/ 
/*N*/     // #101029#: Code extracted to ImpInitDrawOutliner()
/*N*/     ImpInitDrawOutliner( rOutl );
/*N*/ 
/*N*/ 	return rOutl;
/*N*/ }

// #101029#: Extracted from Paint()

/*N*/ void SdrTextObj::SetupOutlinerFormatting( SdrOutliner& rOutl, Rectangle& rPaintRect ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrTextObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrAttrObj::PreSave();
/*N*/ 
/*N*/ 	// Prepare OutlinerParaObjects for storing
/*N*/ 	OutlinerParaObject* pParaObj = GetOutlinerParaObject();
/*N*/ 	if(pParaObj && GetModel())
/*N*/ 		pParaObj->PrepareStore((SfxStyleSheetPool*)GetModel()->GetStyleSheetPool());
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ OutlinerParaObject* SdrTextObj::GetOutlinerParaObject() const
/*N*/ {
/*N*/ 	return pOutlinerParaObject;
/*N*/ }

/*N*/ void SdrTextObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		// Update HitTestOutliner
/*N*/ 		const SdrTextObj* pTestObj = pModel->GetHitTestOutliner().GetTextObj();
/*N*/ 		if( pTestObj && pTestObj->GetOutlinerParaObject() == pOutlinerParaObject )
/*?*/ 			pModel->GetHitTestOutliner().SetTextObj( NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*N*/ 		delete pOutlinerParaObject;
/*N*/ 		pOutlinerParaObject=NULL;
/*N*/ 	}
/*N*/ 	pOutlinerParaObject=pTextObject;
/*N*/ 
/*N*/ 	if( pOutlinerParaObject )
/*N*/ 	{
/*N*/ 		ImpForceItemSet();
/*N*/ 		mpObjectItemSet->Put( SvxWritingModeItem( pOutlinerParaObject->IsVertical() ? ::com::sun::star::text::WritingMode_TB_RL : ::com::sun::star::text::WritingMode_LR_TB ) );
/*N*/ 	}
/*N*/ 
/*N*/ 	SetTextSizeDirty();
/*N*/ 	bPortionInfoChecked=FALSE;
/*N*/ 	if (IsTextFrame() && (IsAutoGrowHeight() || IsAutoGrowWidth())) { // Textrahmen anpassen!
/*N*/ 		NbcAdjustTextFrameWidthAndHeight();
/*N*/ 	}
/*N*/ 	if (!IsTextFrame()) {
/*N*/ 		// Das SnapRect behaelt seine Groesse bei
/*N*/ 		bBoundRectDirty=TRUE;
/*N*/ 		SetRectsDirty(TRUE);
/*N*/ 	}
/*N*/ 	ImpSetTextStyleSheetListeners();
/*N*/ 	ImpCheckMasterCachable();
/*N*/ }

/*N*/ void SdrTextObj::NbcReformatText()
/*N*/ {
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*N*/ 		bPortionInfoChecked=FALSE;
/*N*/ 		pOutlinerParaObject->ClearPortionInfo();
/*N*/ 		if (bTextFrame) {
/*N*/ 			NbcAdjustTextFrameWidthAndHeight();
/*N*/ 		} else {
/*N*/ 			// Das SnapRect behaelt seine Groesse bei
/*N*/ 			bBoundRectDirty=TRUE;
/*N*/ 			SetRectsDirty(TRUE);
/*N*/ 		}
/*N*/ 		SetTextSizeDirty();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrTextObj::ReformatText()
/*N*/ {
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcReformatText();
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		if (GetBoundRect()!=aBoundRect0) {
/*N*/ 			SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrTextObj::RestartAnimation(SdrPageView* pPageView) const
/*N*/ {
///*N*/ 	FASTBOOL bAnimated=GetTextAniKind()!=SDRTEXTANI_NONE;
///*N*/ 	if (bAnimated) {
///*N*/ 		ImpSdrMtfAnimator* pAnimator=((SdrTextObj*)this)->ImpGetMtfAnimator();
///*N*/ 		if (pAnimator!=NULL) {
///*?*/ 			if (pPageView==NULL) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
////STRIP001 /*?*/ 				pAnimator->Stop();
///*?*/ 			} else {
///*?*/ 				for (ULONG nInfoNum=pAnimator->GetInfoCount(); nInfoNum>0;) {
///*?*/ 					nInfoNum--;
///*?*/ 					ImpMtfAnimationInfo* pInfo=pAnimator->GetInfo(nInfoNum);
///*?*/ 					if (pInfo->pPageView==pPageView) {
///*?*/ 						pAnimator->RemoveInfo(nInfoNum);
///*?*/ 					}
///*?*/ 				}
///*?*/ 			}
///*N*/ 		}
///*N*/ 	}
/*N*/ }


/*N*/ void SdrTextObj::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	SdrAttrObj::SaveGeoData(rGeo);
/*N*/ 	SdrTextObjGeoData& rTGeo=(SdrTextObjGeoData&)rGeo;
/*N*/ 	rTGeo.aRect  =aRect;
/*N*/ 	rTGeo.aGeo   =aGeo;
/*N*/ }

/*N*/ void SdrTextObj::RestGeoData(const SdrObjGeoData& rGeo)
/*N*/ { // RectsDirty wird von SdrObject gerufen
/*N*/ 	SdrAttrObj::RestGeoData(rGeo);
/*N*/ 	SdrTextObjGeoData& rTGeo=(SdrTextObjGeoData&)rGeo;
/*N*/ 	aRect  =rTGeo.aRect;
/*N*/ 	aGeo   =rTGeo.aGeo;
/*N*/ 	SetTextSizeDirty();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// I/O
////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrTextObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrAttrObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrTextObj");
/*N*/ #endif
/*N*/ 	rOut<<BYTE(eTextKind);
/*N*/ 	rOut<<aRect;
/*N*/ 	rOut<<INT32(aGeo.nDrehWink);
/*N*/ 	rOut<<INT32(aGeo.nShearWink);
/*N*/ 
/*N*/ 	// Wird gerade editiert, also das ParaObject aus dem aktiven Editor verwenden
/*N*/ 	// Das war frueher. Jetzt wird beim Speichern sowas aehnliches wie EndTextEdit gemacht! #43095#
/*N*/ 	if (pEdtOutl!=NULL) {
/*?*/ 		// #43095#
/*?*/ 		OutlinerParaObject* pPara=GetEditOutlinerParaObject();
/*?*/ 		// casting auf nicht-const
/*?*/ 		((SdrTextObj*)this)->SetOutlinerParaObject(pPara);
/*?*/ 
/*?*/ 		// #91254# put text to object and set EmptyPresObj to FALSE
/*?*/ 		if(pPara && IsEmptyPresObj())
/*?*/ 			((SdrTextObj*)this)->SetEmptyPresObj(FALSE);
/*N*/ 	}
/*N*/ 	OutlinerParaObject* pPara=pOutlinerParaObject;
/*N*/ 
/*N*/ 	BOOL bOutlinerParaObjectValid=pPara!=NULL;
/*N*/ 	rOut<<bOutlinerParaObjectValid;
/*N*/ 
/*N*/ 	if (bOutlinerParaObjectValid)
/*N*/ 	{
/*N*/ 		SdrDownCompat aTextCompat(rOut,STREAM_WRITE); // Ab V11 eingepackt
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aTextCompat.SetID("SdrTextObj(OutlinerParaObject)");
/*N*/ #endif
/*N*/ 		pPara->Store(rOut); // neues Store am Outliner ab SV303
/*N*/ 		pPara->FinishStore();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ab FileVersion 10 wird das TextBoundRect gestreamt
/*N*/ 	BOOL bFormTextBoundRectValid=pFormTextBoundRect!=NULL;
/*N*/ 	rOut<<bFormTextBoundRectValid;
/*N*/ 	if (bFormTextBoundRectValid) {
/*N*/ 		rOut<<*pFormTextBoundRect;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrTextObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	if (pOutlinerParaObject!=NULL) {
/*?*/ 		delete pOutlinerParaObject;
/*?*/ 		pOutlinerParaObject=NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrAttrObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrTextObj");
/*N*/ #endif
/*N*/ 	BYTE nTmp;
/*N*/ 	rIn>>nTmp;
/*N*/ 	eTextKind=SdrObjKind(nTmp);
/*N*/ 	rIn>>aRect;
/*N*/ 	INT32 n32;
/*N*/ 	rIn>>n32; aGeo.nDrehWink=n32;
/*N*/ 	rIn>>n32; aGeo.nShearWink=n32;
/*N*/ 	aGeo.RecalcSinCos();
/*N*/ 	aGeo.RecalcTan();
/*N*/ 	//rIn>>aText;
/*N*/ 	if (rHead.GetVersion()<=5 && IsOutlText()) { // Das war bis zu diesem Zeitpunkt nicht gespeichert
/*?*/ 		NbcSetAutoGrowHeight(FALSE);
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bOutlinerParaObjectValid=FALSE;
/*N*/ 	rIn>>bOutlinerParaObjectValid;
/*N*/ 	if (bOutlinerParaObjectValid)
/*N*/ 	{
/*N*/ 		SfxItemPool* pOutlPool=pModel!=NULL ? &pModel->GetItemPool() : NULL;
/*N*/ 		if (rHead.GetVersion()>=11) {
/*N*/ 			SdrDownCompat aTextCompat(rIn,STREAM_READ); // ab V11 eingepackt
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aTextCompat.SetID("SdrTextObj(OutlinerParaObject)");
/*N*/ #endif
/*N*/ 			pOutlinerParaObject=OutlinerParaObject::Create(rIn,pOutlPool);
/*N*/ 		} else {
/*N*/ 			pOutlinerParaObject=OutlinerParaObject::Create(rIn,pOutlPool);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOutlinerParaObject )
/*N*/ 	{
/*N*/ 		if( pOutlinerParaObject->GetOutlinerMode() == OUTLINERMODE_DONTKNOW )
/*N*/ 		{
/*N*/ 			if( eTextKind == OBJ_TITLETEXT )
/*N*/ 				pOutlinerParaObject->SetOutlinerMode( OUTLINERMODE_TITLEOBJECT );
/*N*/ 			else if( eTextKind == OBJ_OUTLINETEXT )
/*N*/ 				pOutlinerParaObject->SetOutlinerMode( OUTLINERMODE_OUTLINEOBJECT );
/*N*/ 			else
/*N*/ 				pOutlinerParaObject->SetOutlinerMode( OUTLINERMODE_TEXTOBJECT );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pOutlinerParaObject->IsVertical() )
/*N*/ 		{
/*?*/ 			ImpForceItemSet();
/*?*/ 			mpObjectItemSet->Put( SvxWritingModeItem( ::com::sun::star::text::WritingMode_TB_RL ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (rHead.GetVersion()>=10) {
/*N*/ 		// Ab FileVersion 10 wird das TextBoundRect gestreamt
/*N*/ 		BOOL bFormTextBoundRectValid=FALSE;
/*N*/ 		rIn>>bFormTextBoundRectValid;
/*N*/ 		if (bFormTextBoundRectValid) {
/*N*/ 			if (pFormTextBoundRect==NULL) pFormTextBoundRect=new Rectangle;
/*N*/ 			rIn>>*pFormTextBoundRect;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if(rHead.GetVersion() < 12 && !bTextFrame)
/*N*/ 	{
/*N*/ 		mpObjectItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
/*N*/ 		mpObjectItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
/*N*/ 		mpObjectItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bTextFrame && pOutlinerParaObject!=NULL)
/*N*/ 		NbcAdjustTextFrameWidthAndHeight();
/*N*/ 
/*N*/ 	if ( pOutlinerParaObject &&
/*N*/ 	     pOutlinerParaObject->GetTextObject().GetVersion() < 500 &&
/*N*/ 		 !pOutlinerParaObject->IsEditDoc() )
/*N*/ 	{
/*N*/ 		pOutlinerParaObject->MergeParaAttribs( GetItemSet() );
/*N*/ 	}
/*N*/ 
/*N*/ 	// #84529# correct gradient rotation for 5.2 and earlier
/*N*/ 	if(aGeo.nDrehWink != 0 && rHead.GetVersion() <= 16)
/*N*/ 	{
/*N*/ 		XFillStyle eStyle = ((const XFillStyleItem&)GetItem(XATTR_FILLSTYLE)).GetValue();
/*N*/ 		if(XFILL_GRADIENT == eStyle)
/*N*/ 		{
/*N*/ 			XFillGradientItem aItem = (XFillGradientItem&)GetItem(XATTR_FILLGRADIENT);
/*N*/ 			XGradient aGradient = aItem.GetValue();
/*N*/ 
/*N*/ 			// calc new angle. aGeo.nDrehWink is 1/100th degree, aGradient.GetAngle()
/*N*/ 			// is 1/10th degree. Match this.
/*N*/ 			sal_Int32 nNewAngle = ((aGeo.nDrehWink + (aGradient.GetAngle() * 10)) + 5) / 10;
/*N*/ 
/*N*/ 			while(nNewAngle < 0)
/*?*/ 				nNewAngle += 3600;
/*N*/ 
/*N*/ 			while(nNewAngle >= 3600)
/*N*/ 				nNewAngle -= 3600;
/*N*/ 
/*N*/ 			// create new item and set
/*N*/ 			aGradient.SetAngle(nNewAngle);
/*N*/ 			aItem.SetValue(aGradient);
/*N*/ 			SetItem(aItem);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	ImpSetTextStyleSheetListeners();
/*N*/ 	SetTextSizeDirty();
/*N*/ 	ImpCheckMasterCachable();
/*N*/ }



/*N*/ SdrFitToSizeType SdrTextObj::GetFitToSize() const
/*N*/ {
/*N*/ 	SdrFitToSizeType eType = SDRTEXTFIT_NONE;
/*N*/ 
/*N*/ 	if(!IsAutoGrowWidth())
/*N*/ 		eType = ((SdrTextFitToSizeTypeItem&)(GetItem(SDRATTR_TEXT_FITTOSIZE))).GetValue();
/*N*/ 
/*N*/ 	return eType;
/*N*/ }

/*N*/ void SdrTextObj::ForceOutlinerParaObject()
/*N*/ {
/*N*/ 	if( pOutlinerParaObject == NULL )
/*N*/ 	{
/*?*/ 		USHORT nOutlMode = OUTLINERMODE_TEXTOBJECT;
/*?*/ 		if( IsTextFrame() && eTextKind == OBJ_OUTLINETEXT )
/*?*/ 			nOutlMode = OUTLINERMODE_OUTLINEOBJECT;
/*?*/ 
/*?*/ 		Outliner* pOutliner = SdrMakeOutliner( nOutlMode, pModel );
/*?*/ 		if( pOutliner )
/*?*/ 		{
/*?*/ 			Outliner& aDrawOutliner = pModel->GetDrawOutliner();
/*?*/ 			pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
/*?*/ 
/*?*/ 			pOutliner->SetStyleSheet( 0, GetStyleSheet());
/*?*/ 			OutlinerParaObject* pOutlinerParaObject = pOutliner->CreateParaObject();
/*?*/ 			SetOutlinerParaObject( pOutlinerParaObject );
/*?*/ 
/*?*/ 			delete pOutliner;
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SdrTextObj::IsVerticalWriting() const
/*N*/ {
/*N*/ 	// #89459#
/*N*/ 	if(pOutlinerParaObject)
/*N*/ 		return pOutlinerParaObject->IsVertical();
/*N*/ 	if(pEdtOutl)
/*?*/ 		return pEdtOutl->IsVertical();
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SdrTextObj::SetVerticalWriting( BOOL bVertical )
/*N*/ {
/*N*/ 	ForceOutlinerParaObject();
/*N*/ 
/*N*/ 	DBG_ASSERT( pOutlinerParaObject, "SdrTextObj::SetVerticalWriting() without OutlinerParaObject!" );
/*N*/ 	if( pOutlinerParaObject )
/*N*/ 	{
/*N*/ 		if(pOutlinerParaObject->IsVertical() != bVertical)
/*N*/ 		{
/*?*/ 			// get item settings
/*?*/ 			const SfxItemSet& rSet = GetItemSet();
/*?*/ 			sal_Bool bAutoGrowWidth = ((SdrTextAutoGrowWidthItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWWIDTH)).GetValue();
/*?*/ 			sal_Bool bAutoGrowHeight = ((SdrTextAutoGrowHeightItem&)rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT)).GetValue();
/*?*/ 
/*?*/ 			// #103516# Also exchange hor/ver adjust items
/*?*/ 			SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
/*?*/ 			SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();
/*?*/ 
/*?*/ 			// rescue object size
/*?*/ 			Rectangle aObjectRect = GetSnapRect();
/*?*/ 
/*?*/ 			// prepare ItemSet to set exchanged width and height items
/*?*/ 			SfxItemSet aNewSet(*rSet.GetPool(),
/*?*/ 				SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
/*?*/ 				// #103516# Expanded item ranges to also support hor and ver adjust.
/*?*/ 				SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
/*?*/ 				SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
/*?*/ 				0, 0);
/*?*/ 
/*?*/ 			aNewSet.Put(rSet);
/*?*/ 			aNewSet.Put(SdrTextAutoGrowWidthItem(bAutoGrowHeight));
/*?*/ 			aNewSet.Put(SdrTextAutoGrowHeightItem(bAutoGrowWidth));
/*?*/ 
/*?*/ 			// #103516# Exchange horz and vert adjusts
/*?*/ 			switch(eVert)
/*?*/ 			{
/*?*/ 				case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
/*?*/ 				case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
/*?*/ 				case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
/*?*/ 				case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
/*?*/ 			}
/*?*/ 			switch(eHorz)
/*?*/ 			{
/*?*/ 				case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
/*?*/ 				case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
/*?*/ 				case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
/*?*/ 				case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
/*?*/ 			}
/*?*/ 
/*?*/ 			SetItemSet(aNewSet);
/*?*/ 
/*?*/ 			// set ParaObject orientation accordingly
/*?*/ 			pOutlinerParaObject->SetVertical(bVertical);
/*?*/ 
/*?*/ 			// restore object size
/*?*/ 			SetSnapRect(aObjectRect);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// transformation interface for StarOfficeAPI. This implements support for
// homogen 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
/*N*/ BOOL SdrTextObj::TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const
/*N*/ {
/*N*/ 	// get turn and shear
/*N*/ 	double fRotate = (aGeo.nDrehWink / 100.0) * F_PI180;
/*N*/ 	double fShear = (aGeo.nShearWink / 100.0) * F_PI180;
/*N*/ 
/*N*/ 	// get aRect, this is the unrotated snaprect
/*N*/ 	Rectangle aRectangle(aRect);
/*N*/ 
/*N*/ 	// fill other values
/*N*/ 	Vector2D aScale((double)aRectangle.GetWidth(), (double)aRectangle.GetHeight());
/*N*/ 	Vector2D aTranslate((double)aRectangle.Left(), (double)aRectangle.Top());
/*N*/ 
/*N*/ 	// position maybe relative to anchorpos, convert
/*N*/ 	if( pModel->IsWriter() )
/*N*/ 	{
/*N*/ 		if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
/*N*/ 			aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
/*N*/ 	}
/*N*/ 
/*N*/ 	// force MapUnit to 100th mm
/*N*/ 	SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
/*N*/ 	if(eMapUnit != SFX_MAPUNIT_100TH_MM)
/*N*/ 	{
/*N*/ 		switch(eMapUnit)
/*N*/ 		{
/*N*/ 			case SFX_MAPUNIT_TWIP :
/*N*/ 			{
/*N*/ 				// position
/*N*/ 				// #104018#
/*N*/ 				aTranslate.X() = ImplTwipsToMM(aTranslate.X());
/*N*/ 				aTranslate.Y() = ImplTwipsToMM(aTranslate.Y());
/*N*/ 
/*N*/ 				// size
/*N*/ 				// #104018#
/*N*/ 				aScale.X() = ImplTwipsToMM(aScale.X());
/*N*/ 				aScale.Y() = ImplTwipsToMM(aScale.Y());
/*N*/ 
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			default:
/*N*/ 			{
/*N*/ 				DBG_ERROR("TRGetBaseGeometry: Missing unit translation to 100th mm!");
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// build matrix
/*N*/ 	rMat.Identity();
/*N*/ 	if(aScale.X() != 1.0 || aScale.Y() != 1.0)
/*N*/ 		rMat.Scale(aScale.X(), aScale.Y());
/*N*/ 	if(fShear != 0.0)
/*N*/ 		rMat.ShearX(tan(fShear));
/*N*/ 	if(fRotate != 0.0)
/*N*/ 		rMat.Rotate(fRotate);
/*N*/ 	if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
/*N*/ 		rMat.Translate(aTranslate.X(), aTranslate.Y());
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
/*N*/ void SdrTextObj::TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon)
/*N*/ {
/*N*/ 	// break up matrix
/*N*/ 	Vector2D aScale, aTranslate;
/*N*/ 	double fShear, fRotate;
/*N*/ 	rMat.DecomposeAndCorrect(aScale, fShear, fRotate, aTranslate);
/*N*/ 
/*N*/ 	// reset object shear and rotations
/*N*/ 	aGeo.nDrehWink = 0;
/*N*/ 	aGeo.RecalcSinCos();
/*N*/ 	aGeo.nShearWink = 0;
/*N*/ 	aGeo.RecalcTan();
/*N*/ 
/*N*/ 	// force metric to pool metric
/*N*/ 	SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
/*N*/ 	if(eMapUnit != SFX_MAPUNIT_100TH_MM)
/*N*/ 	{
/*N*/ 		switch(eMapUnit)
/*N*/ 		{
/*N*/ 			case SFX_MAPUNIT_TWIP :
/*N*/ 			{
/*N*/ 				// position
/*N*/ 				// #104018#
/*N*/ 				aTranslate.X() = ImplMMToTwips(aTranslate.X());
/*N*/ 				aTranslate.Y() = ImplMMToTwips(aTranslate.Y());
/*N*/ 
/*N*/ 				// size
/*N*/ 				// #104018#
/*N*/ 				aScale.X() = ImplMMToTwips(aScale.X());
/*N*/ 				aScale.Y() = ImplMMToTwips(aScale.Y());
/*N*/ 
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			default:
/*N*/ 			{
/*N*/ 				DBG_ERROR("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// if anchor is used, make position relative to it
/*N*/ 	if( pModel->IsWriter() )
/*N*/ 	{
/*N*/ 		if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
/*N*/ 			aTranslate += Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
/*N*/ 	}
/*N*/ 
/*N*/ 	// build and set BaseRect (use scale)
/*N*/ 	Point aPoint = Point();
/*N*/ 	Size  aSize(FRound(aScale.X()), FRound(aScale.Y()));
/*N*/ 	Rectangle aBaseRect(aPoint, aSize);
/*N*/ 	SetSnapRect(aBaseRect);
/*N*/ 
/*N*/ 	// shear?
/*N*/ 	if(fShear != 0.0)
/*N*/ 	{
/*N*/ 		GeoStat aGeoStat;
/*N*/ 		aGeoStat.nShearWink = FRound((atan(fShear) / F_PI180) * 100.0);
/*N*/ 		aGeoStat.RecalcTan();
/*N*/ 		Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, FALSE);
/*N*/ 	}
/*N*/ 
/*N*/ 	// rotation?
/*N*/ 	if(fRotate != 0.0)
/*N*/ 	{
/*N*/ 		GeoStat aGeoStat;
/*N*/ 		aGeoStat.nDrehWink = FRound((fRotate / F_PI180) * 100.0);
/*N*/ 		aGeoStat.RecalcSinCos();
/*N*/ 		Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
/*N*/ 	}
/*N*/ 
/*N*/ 	// translate?
/*N*/ 	if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
/*N*/ 	{
/*N*/ 		Move(Size(
/*N*/ 			(sal_Int32)FRound(aTranslate.X()),
/*N*/ 			(sal_Int32)FRound(aTranslate.Y())));
/*N*/ 	}
/*N*/ }


/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Konzept des TextObjekts:
// ~~~~~~~~~~~~~~~~~~~~~~~~
// Attribute/Varianten:
// - BOOL Textrahmen / beschriftetes Zeichenobjekt
// - BOOL FontWork                 (wenn nicht Textrahmen und nicht ContourTextFrame)
// - BOOL ContourTextFrame         (wenn nicht Textrahmen und nicht Fontwork)
// - long Drehwinkel               (wenn nicht FontWork)
// - long Textrahmenabstaende      (wenn nicht FontWork)
// - BOOL FitToSize                (wenn nicht FontWork)
// - BOOL AutoGrowingWidth/Height  (wenn nicht FitToSize und nicht FontWork)
// - long Min/MaxFrameWidth/Height (wenn AutoGrowingWidth/Height)
// - enum Horizontale Textverankerung Links,Mitte,Rechts,Block,Stretch(ni)
// - enum Vertikale Textverankerung Oben,Mitte,Unten,Block,Stretch(ni)
// - enum Laufschrift              (wenn nicht FontWork)
//
// Jedes abgeleitete Objekt ist entweder ein Textrahmen (bTextFrame=TRUE)
// oder ein beschriftetes Zeichenobjekt (bTextFrame=FALSE).
//
// Defaultverankerung von Textrahmen:
//   SDRTEXTHORZADJUST_BLOCK, SDRTEXTVERTADJUST_TOP
//   = statische Pooldefaults
// Defaultverankerung von beschrifteten Zeichenobjekten:
//   SDRTEXTHORZADJUST_CENTER, SDRTEXTVERTADJUST_CENTER
//   durch harte Attributierung von SdrAttrObj
//
// Jedes vom SdrTextObj abgeleitete Objekt muss ein "UnrotatedSnapRect"
// (->TakeUnrotatedSnapRect()) liefern (Drehreferenz ist TopLeft dieses
// Rechtecks (aGeo.nDrehWink)), welches die Grundlage der Textverankerung
// bildet. Von diesem werden dann ringsum die Textrahmenabstaende abgezogen;
// das Ergebnis ist der Ankerbereich (->TakeTextAnchorRect()). Innerhalb
// dieses Bereichs wird dann in Abhaengigkeit von der horizontalen und
// vertikalen Ausrichtung (SdrTextVertAdjust,SdrTextHorzAdjust) der Ankerpunkt
// sowie der Ausgabebereich bestimmt. Bei beschrifteten Grafikobjekten kann
// der Ausgabebereich durchaus groesser als der Ankerbereich werden, bei
// Textrahmen ist er stets kleiner oder gleich (ausser bei negativen Textrahmen-
// abstaenden).
//
// FitToSize hat Prioritaet vor Textverankerung und AutoGrowHeight/Width. Der
// Ausgabebereich ist bei FitToSize immer genau der Ankerbereich. Weiterhin
// gibt es bei FitToSize keinen automatischen Zeilenumbruch.
//
// ContourTextFrame:
// - long Drehwinkel
// - long Textrahmenabstaende         spaeter vielleicht
// - BOOL FitToSize                   spaeter vielleicht
// - BOOL AutoGrowingWidth/Height     viel spaeter vielleicht
// - long Min/MaxFrameWidth/Height    viel spaeter vielleicht
// - enum Horizontale Textverankerung spaeter vielleicht, erstmal Links, Absatz zentr.
// - enum Vertikale Textverankerung   spaeter vielleicht, erstmal oben
// - enum Laufschrift                 spaeter vielleicht (evtl. sogar mit korrektem Clipping)
//
// Bei Aenderungen zu beachten:
// - Paint
// - HitTest
// - RecalcBoundRect
// - ConvertToPoly
// - Edit
// - Drucken,Speichern, Paint in Nachbarview waerend Edit
// - ModelChanged (z.B. durch NachbarView oder Lineale) waerend Edit
// - FillColorChanged waerend Edit
// - uvm...
//
/////////////////////////////////////////////////////////////////////////////////////////////////

}
