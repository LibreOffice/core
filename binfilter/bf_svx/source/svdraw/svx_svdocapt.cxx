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

#include <tools/bigint.hxx>


#include <bf_svtools/style.hxx>

#include "svdocapt.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svdio.hxx"
#include "svdstr.hrc"    // Objektname




#include "eeitem.hxx"
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ enum EscDir {LKS,RTS,OBN,UNT};

/*N*/ class ImpCaptParams
/*N*/ {
/*N*/ public:
/*N*/ 	SdrCaptionType				eType;
/*N*/ 	long						nAngle;
/*N*/ 	long						nGap;
/*N*/ 	long						nEscRel;
/*N*/ 	long						nEscAbs;
/*N*/ 	long						nLineLen;
/*N*/ 	SdrCaptionEscDir			eEscDir;
/*N*/ 	FASTBOOL					bFitLineLen;
/*N*/ 	FASTBOOL					bEscRel;
/*N*/ 	FASTBOOL					bFixedAngle;
/*N*/ 
/*N*/ public:
/*N*/ 	ImpCaptParams()
/*N*/ 	{
/*N*/ 		eType      =SDRCAPT_TYPE3;
/*N*/ 		bFixedAngle=FALSE;
/*N*/ 		nAngle     =4500;
/*N*/ 		nGap       =0;
/*N*/ 		eEscDir    =SDRCAPT_ESCHORIZONTAL;
/*N*/ 		bEscRel    =TRUE;
/*N*/ 		nEscRel    =5000;
/*N*/ 		nEscAbs    =0;
/*N*/ 		nLineLen   =0;
/*N*/ 		bFitLineLen=TRUE;
/*N*/ 	}
/*N*/ 	void CalcEscPos(const Point& rTail, const Rectangle& rRect, Point& rPt, EscDir& rDir) const;
/*N*/ };

/*N*/ void ImpCaptParams::CalcEscPos(const Point& rTailPt, const Rectangle& rRect, Point& rPt, EscDir& rDir) const
/*N*/ {
/*N*/ 	Point aTl(rTailPt); // lokal kopieren wg. Performance
/*N*/ 	long nX,nY;
/*N*/ 	if (bEscRel) {
/*N*/ 		nX=rRect.Right()-rRect.Left();
/*N*/ 		nX=BigMulDiv(nX,nEscRel,10000);
/*N*/ 		nY=rRect.Bottom()-rRect.Top();
/*N*/ 		nY=BigMulDiv(nY,nEscRel,10000);
/*N*/ 	} else {
/*N*/ 		nX=nEscAbs;
/*N*/ 		nY=nEscAbs;
/*N*/ 	}
/*N*/ 	nX+=rRect.Left();
/*N*/ 	nY+=rRect.Top();
/*N*/ 	Point  aBestPt;
/*N*/ 	EscDir eBestDir=LKS;
/*N*/ 	FASTBOOL bTryH=eEscDir==SDRCAPT_ESCBESTFIT;
/*N*/ 	if (!bTryH) {
/*N*/ 		if (eType!=SDRCAPT_TYPE1) {
/*N*/ 			bTryH=eEscDir==SDRCAPT_ESCHORIZONTAL;
/*N*/ 		} else {
/*N*/ 			bTryH=eEscDir==SDRCAPT_ESCVERTICAL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	FASTBOOL bTryV=eEscDir==SDRCAPT_ESCBESTFIT;
/*N*/ 	if (!bTryV) {
/*N*/ 		if (eType!=SDRCAPT_TYPE1) {
/*N*/ 			bTryV=eEscDir==SDRCAPT_ESCVERTICAL;
/*N*/ 		} else {
/*N*/ 			bTryV=eEscDir==SDRCAPT_ESCHORIZONTAL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bTryH) {
/*N*/ 		Point aLft(rRect.Left()-nGap,nY);
/*N*/ 		Point aRgt(rRect.Right()+nGap,nY);
/*N*/ 		FASTBOOL bLft=(aTl.X()-aLft.X()<aRgt.X()-aTl.X());
/*N*/ 		if (bLft) {
/*N*/ 			eBestDir=LKS;
/*N*/ 			aBestPt=aLft;
/*N*/ 		} else {
/*N*/ 			eBestDir=RTS;
/*N*/ 			aBestPt=aRgt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (bTryV) {
/*?*/ 		Point aTop(nX,rRect.Top()-nGap);
/*?*/ 		Point aBtm(nX,rRect.Bottom()+nGap);
/*?*/ 		FASTBOOL bTop=(aTl.Y()-aTop.Y()<aBtm.Y()-aTl.Y());
/*?*/ 		Point aBest2;
/*?*/ 		EscDir eBest2;
/*?*/ 		if (bTop) {
/*?*/ 			eBest2=OBN;
/*?*/ 			aBest2=aTop;
/*?*/ 		} else {
/*?*/ 			eBest2=UNT;
/*?*/ 			aBest2=aBtm;
/*?*/ 		}
/*?*/ 		FASTBOOL bTakeIt=eEscDir!=SDRCAPT_ESCBESTFIT;
/*?*/ 		if (!bTakeIt) {
/*?*/ 			BigInt aHorX(aBestPt.X()-aTl.X()); aHorX*=aHorX;
/*?*/ 			BigInt aHorY(aBestPt.Y()-aTl.Y()); aHorY*=aHorY;
/*?*/ 			BigInt aVerX(aBest2.X()-aTl.X());  aVerX*=aVerX;
/*?*/ 			BigInt aVerY(aBest2.Y()-aTl.Y());  aVerY*=aVerY;
/*?*/ 			if (eType!=SDRCAPT_TYPE1) {
/*?*/ 				bTakeIt=aVerX+aVerY<aHorX+aHorY;
/*?*/ 			} else {
/*?*/ 				bTakeIt=aVerX+aVerY>=aHorX+aHorY;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if (bTakeIt) {
/*?*/ 			aBestPt=aBest2;
/*?*/ 			eBestDir=eBest2;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	rPt=aBestPt;
/*N*/ 	rDir=eBestDir;
/*N*/ }

/*N*/ TYPEINIT1(SdrCaptionObj,SdrRectObj);

/*N*/ SdrCaptionObj::SdrCaptionObj():
/*N*/ 	SdrRectObj(OBJ_TEXT),
/*N*/ 	aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
/*N*/ 	mbSpecialTextBoxShadow(FALSE)
/*N*/ {
/*N*/ }

/*N*/ SdrCaptionObj::~SdrCaptionObj()
/*N*/ {
/*N*/ }



/*N*/ UINT16 SdrCaptionObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16(OBJ_CAPTION);
/*N*/ }

/*N*/ void SdrCaptionObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	aOutRect=GetSnapRect();
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	nLineWdt++; nLineWdt/=2;
/*N*/ 	long nLEndWdt=ImpGetLineEndAdd();
/*N*/ 	if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		aOutRect.Left  ()-=nLineWdt;
/*N*/ 		aOutRect.Top   ()-=nLineWdt;
/*N*/ 		aOutRect.Right ()+=nLineWdt;
/*N*/ 		aOutRect.Bottom()+=nLineWdt;
/*N*/ 	}
/*N*/ 	ImpAddShadowToBoundRect();
/*N*/ 	ImpAddTextToBoundRect();
/*N*/ }
















/*N*/ void SdrCaptionObj::ImpGetCaptParams(ImpCaptParams& rPara) const
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	rPara.eType      =((SdrCaptionTypeItem&)      (rSet.Get(SDRATTR_CAPTIONTYPE      ))).GetValue();
/*N*/ 	rPara.bFixedAngle=((SdrCaptionFixedAngleItem&)(rSet.Get(SDRATTR_CAPTIONANGLE     ))).GetValue();
/*N*/ 	rPara.nAngle     =((SdrCaptionAngleItem&)     (rSet.Get(SDRATTR_CAPTIONFIXEDANGLE))).GetValue();
/*N*/ 	rPara.nGap       =((SdrCaptionGapItem&)       (rSet.Get(SDRATTR_CAPTIONGAP       ))).GetValue();
/*N*/ 	rPara.eEscDir    =((SdrCaptionEscDirItem&)    (rSet.Get(SDRATTR_CAPTIONESCDIR    ))).GetValue();
/*N*/ 	rPara.bEscRel    =((SdrCaptionEscIsRelItem&)  (rSet.Get(SDRATTR_CAPTIONESCISREL  ))).GetValue();
/*N*/ 	rPara.nEscRel    =((SdrCaptionEscRelItem&)    (rSet.Get(SDRATTR_CAPTIONESCREL    ))).GetValue();
/*N*/ 	rPara.nEscAbs    =((SdrCaptionEscAbsItem&)    (rSet.Get(SDRATTR_CAPTIONESCABS    ))).GetValue();
/*N*/ 	rPara.nLineLen   =((SdrCaptionLineLenItem&)   (rSet.Get(SDRATTR_CAPTIONLINELEN   ))).GetValue();
/*N*/ 	rPara.bFitLineLen=((SdrCaptionFitLineLenItem&)(rSet.Get(SDRATTR_CAPTIONFITLINELEN))).GetValue();
/*N*/ }

/*N*/ void SdrCaptionObj::ImpRecalcTail()
/*N*/ {
/*N*/ 	ImpCaptParams aPara;
/*N*/ 	ImpGetCaptParams(aPara);
/*N*/ 	ImpCalcTail(aPara,aTailPoly,aRect);
/*N*/ 	SetRectsDirty();
/*N*/ 	SetXPolyDirty();
/*N*/ }



/*N*/ void SdrCaptionObj::ImpCalcTail3(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
/*N*/ { // Gap/EscDir/EscPos/Angle/LineLen
/*N*/ 	Polygon aPol(3);
/*N*/ 	Point aTl(rPoly[0]);
/*N*/ 	aPol[0]=aTl;
/*N*/ 
/*N*/ 	EscDir eEscDir;
/*N*/ 	Point aEscPos;
/*N*/ 	rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
/*N*/ 	aPol[1]=aEscPos;
/*N*/ 	aPol[2]=aEscPos;
/*N*/ 
/*N*/ 	if (eEscDir==LKS || eEscDir==RTS) {
/*N*/ 		if (rPara.bFitLineLen) {
/*N*/ 			aPol[1].X()=(aTl.X()+aEscPos.X())/2;
/*N*/ 		} else {
/*N*/ 			if (eEscDir==LKS) aPol[1].X()-=rPara.nLineLen;
/*N*/ 			else aPol[1].X()+=rPara.nLineLen;
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		if (rPara.bFitLineLen) {
/*N*/ 			aPol[1].Y()=(aTl.Y()+aEscPos.Y())/2;
/*N*/ 		} else {
/*N*/ 			if (eEscDir==OBN) aPol[1].Y()-=rPara.nLineLen;
/*N*/ 			else aPol[1].Y()+=rPara.nLineLen;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!rPara.bFixedAngle) {
/*N*/ 		// fehlende Implementation
/*N*/ 	}
/*N*/ 	rPoly=aPol;
/*N*/ }


/*N*/ void SdrCaptionObj::ImpCalcTail(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
/*N*/ {
/*N*/ 	switch (rPara.eType) {
/*?*/ 		case SDRCAPT_TYPE1: DBG_BF_ASSERT(0, "STRIP"); break;//STRIP001 ImpCalcTail1(rPara,rPoly,rRect); break;
/*?*/ 		case SDRCAPT_TYPE2: DBG_BF_ASSERT(0, "STRIP"); break;//STRIP001 ImpCalcTail2(rPara,rPoly,rRect); break;
/*N*/ 		case SDRCAPT_TYPE3: ImpCalcTail3(rPara,rPoly,rRect); break;
/*?*/ 		case SDRCAPT_TYPE4: DBG_BF_ASSERT(0, "STRIP"); break;//STRIP001 ImpCalcTail4(rPara,rPoly,rRect); break;
/*N*/ 	}
/*N*/ }








/*N*/ void SdrCaptionObj::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	SdrRectObj::NbcMove(rSiz);
/*N*/ 	MovePoly(aTailPoly,rSiz);
/*N*/ }

/*N*/ void SdrCaptionObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrRectObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	ResizePoly(aTailPoly,rRef,xFact,yFact);
/*N*/ 	ImpRecalcTail();
/*N*/ }



/*N*/ void SdrCaptionObj::NbcSetAnchorPos(const Point& rPnt)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetAnchorPos(rPnt);
/*N*/ 	// !!!!! fehlende Impl.
/*N*/ }

/*N*/ const Point& SdrCaptionObj::GetAnchorPos() const
/*N*/ {
/*N*/ 	// !!!!! fehlende Impl.
/*N*/ 	return SdrRectObj::GetAnchorPos();
/*N*/ }

/*N*/ void SdrCaptionObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	SdrRectObj::RecalcSnapRect();
/*N*/ 	maSnapRect.Union(aTailPoly.GetBoundRect());
/*N*/ 	// !!!!! fehlende Impl.
/*N*/ }

/*N*/ const Rectangle& SdrCaptionObj::GetSnapRect() const
/*N*/ {
/*N*/ 	return SdrRectObj::GetSnapRect();
/*N*/ }

/*N*/ void SdrCaptionObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	// #109587#
/*N*/ 	//
/*N*/ 	// The new SnapRect contains the tail BoundRect, see
/*N*/ 	// RecalcSnapRect() above. Thus, the new to-be-setted
/*N*/ 	// SnapRect needs to be 'cleared' from that tail offsets
/*N*/ 	// before setting it as new SnapRect at the SdrRectObj.
/*N*/ 	//
/*N*/ 	// As base for 'clearing' the old text rect is taken from aRect
/*N*/ 	// using GetLogicRect(), see below. Second the outer tail point
/*N*/ 	// wich expanded that rect. Since the other end of the
/*N*/ 	// connection polygon always resides at one edge of the text rect
/*N*/ 	// this is sufficient information.
/*N*/ 	Rectangle aNewSnapRect(rRect);
/*N*/ 	const Rectangle aOriginalTextRect(GetLogicRect());
/*N*/ 	const Point aTailPoint = GetTailPos();
/*N*/ 
/*N*/ 	// #109992#
/*N*/ 	// This compares only make sense when aOriginalTextRect and the
/*N*/ 	// aTailPoint contain useful data. Thus, test it before usage.
/*N*/ 	if(!aOriginalTextRect.IsEmpty())
/*N*/ 	{
/*N*/ 		if(aTailPoint.X() < aOriginalTextRect.Left())
/*N*/ 		{
/*N*/ 			const sal_Int32 nDist = aOriginalTextRect.Left() - aTailPoint.X();
/*N*/ 			aNewSnapRect.Left() = aNewSnapRect.Left() + nDist;
/*N*/ 		}
/*N*/ 		else if(aTailPoint.X() > aOriginalTextRect.Right())
/*N*/ 		{
/*N*/ 			const sal_Int32 nDist = aTailPoint.X() - aOriginalTextRect.Right();
/*N*/ 			aNewSnapRect.Right() = aNewSnapRect.Right() - nDist;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(aTailPoint.Y() < aOriginalTextRect.Top())
/*N*/ 		{
/*N*/ 			const sal_Int32 nDist = aOriginalTextRect.Top() - aTailPoint.Y();
/*N*/ 			aNewSnapRect.Top() = aNewSnapRect.Top() + nDist;
/*N*/ 		}
/*N*/ 		else if(aTailPoint.Y() > aOriginalTextRect.Bottom())
/*N*/ 		{
/*N*/ 			const sal_Int32 nDist = aTailPoint.Y() - aOriginalTextRect.Bottom();
/*N*/ 			aNewSnapRect.Bottom() = aNewSnapRect.Bottom() - nDist;
/*N*/ 		}
/*N*/ 
/*N*/ 		// make sure rectangle is correctly defined
/*N*/ 		ImpJustifyRect(aNewSnapRect);
/*N*/ 
/*N*/ 		// #86616#
/*N*/ 		SdrRectObj::NbcSetSnapRect(aNewSnapRect);
/*N*/ 	}
/*N*/ }



/*N*/ const Point& SdrCaptionObj::GetTailPos() const
/*N*/ {
/*N*/ 	return aTailPoly[0];
/*N*/ }

/*N*/ void SdrCaptionObj::SetTailPos(const Point& rPos)
/*N*/ {
/*N*/ 	if (aTailPoly.GetSize()==0 || aTailPoly[0]!=rPos) {
/*N*/ 		Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		NbcSetTailPos(rPos);
/*N*/ 		SetChanged();
/*N*/ 		SendRepaintBroadcast();
/*N*/ 		SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrCaptionObj::NbcSetTailPos(const Point& rPos)
/*N*/ {
/*N*/ 	aTailPoly[0]=rPos;
/*N*/ 	ImpRecalcTail();
/*N*/ }



/*N*/ void SdrCaptionObj::SetModel(SdrModel* pNewModel)
/*N*/ {
/*N*/ 	SdrRectObj::SetModel(pNewModel);
/*N*/ 	ImpRecalcTail();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ SfxItemSet* SdrCaptionObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// caption attributes
/*N*/ 		SDRATTR_CAPTION_FIRST, SDRATTRSET_CAPTION,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
/*N*/ void SdrCaptionObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// local changes
/*N*/ 	ImpRecalcTail();
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::ItemSetChanged(rSet);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrCaptionObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ 	ImpRecalcTail();
/*N*/ }






////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*?*/ void SdrCaptionObj::PreSave()
/*?*/ {
/*?*/ 	// call parent
/*?*/ 	SdrRectObj::PreSave();
/*?*/ 
/*?*/ 	// prepare SetItems for storage
/*?*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*?*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*?*/ 	SdrCaptionSetItem aCaptAttr(rSet.GetPool());
/*?*/ 	aCaptAttr.GetItemSet().Put(rSet);
/*?*/ 	aCaptAttr.GetItemSet().SetParent(pParent);
/*?*/ 	mpObjectItemSet->Put(aCaptAttr);
/*?*/ }

/*?*/ void SdrCaptionObj::PostSave()
/*?*/ {
/*?*/ 	// call parent
/*?*/ 	SdrRectObj::PostSave();
/*?*/ 
/*?*/ 	// remove SetItems from local itemset
/*?*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_CAPTION);
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*?*/ void SdrCaptionObj::WriteData(SvStream& rOut) const
/*?*/ {
/*?*/ 	SdrRectObj::WriteData(rOut);
/*?*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*?*/ #ifdef DBG_UTIL
/*?*/ 	aCompat.SetID("SdrCaptionObj");
/*?*/ #endif
/*?*/ 
/*?*/ 	rOut << aTailPoly;
/*?*/ 	SfxItemPool* pPool = GetItemPool();
/*?*/ 
/*?*/ 	if(pPool)
/*?*/ 	{
/*?*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*?*/ 
/*?*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_CAPTION));
/*?*/ 	}
/*?*/ 	else
/*?*/ 	{
/*?*/ 		rOut << UINT16(SFX_ITEMS_NULL);
/*?*/ 	}
/*?*/ }

/*N*/ void SdrCaptionObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if(rIn.GetError())
/*N*/ 		return;
/*N*/ 
/*N*/ 	SdrRectObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrCaptionObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	rIn >> aTailPoly;
/*N*/ 
/*N*/ 	if(rHead.GetVersion() < 11) { sal_uInt16 nWhichDum; rIn >> nWhichDum; } // ab V11 keine WhichId mehr
/*N*/ 
/*N*/ 	SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		sal_uInt16 nSetID = SDRATTRSET_CAPTION;
/*N*/ 		const SdrCaptionSetItem* pCaptAttr = (const SdrCaptionSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 		if(pCaptAttr)
/*N*/ 			SetItemSet(pCaptAttr->GetItemSet());
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		sal_uInt16 nSuroDum;
/*N*/ 		rIn >> nSuroDum;
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
