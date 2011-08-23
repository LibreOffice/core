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

#include "svdopath.hxx"
#include <math.h>
#include "xpool.hxx"
#include "xoutx.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdview.hxx"  // fuer MovCreate bei Freihandlinien
#include "svdstr.hrc"   // Objektname

#ifdef _MSC_VER
#pragma optimize ("",off)
#endif



#include <vcl/salbtype.hxx>		// FRound

#include "svdoimp.hxx"

#include <bf_goodies/matrix3d.hxx>

namespace binfilter {

// #104018# replace macros above with type-safe methods
inline sal_Int32 ImplTwipsToMM(sal_Int32 nVal) { return ((nVal * 127 + 36) / 72); }
inline sal_Int32 ImplMMToTwips(sal_Int32 nVal) { return ((nVal * 72 + 63) / 127); }
inline sal_Int64 ImplTwipsToMM(sal_Int64 nVal) { return ((nVal * 127 + 36) / 72); }
inline sal_Int64 ImplMMToTwips(sal_Int64 nVal) { return ((nVal * 72 + 63) / 127); }
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

/*************************************************************************/

#define SVDOPATH_INITSIZE   20
#define SVDOPATH_RESIZE     20

/*************************************************************************/

/*N*/ SdrPathObjGeoData::SdrPathObjGeoData()
/*N*/ {
/*N*/ }

/*N*/ SdrPathObjGeoData::~SdrPathObjGeoData()
/*N*/ {
/*N*/ }

/*N*/ TYPEINIT1(SdrPathObj,SdrTextObj);

/*N*/ SdrPathObj::SdrPathObj(SdrObjKind eNewKind)
/*N*/ {
/*N*/ 	eKind=eNewKind;
/*N*/ 	bClosedObj=IsClosed();
/*N*/ 	bCreating=FALSE;
/*N*/ }

/*N*/ SdrPathObj::SdrPathObj(SdrObjKind eNewKind, const XPolyPolygon& rPathPoly)
/*N*/ {
/*N*/ 	eKind=eNewKind;
/*N*/ 	bClosedObj=IsClosed();
/*N*/ 
/*N*/ 	// #104640# Set local XPolyPolygon with open/close correction
/*N*/ 	NbcSetPathPoly(rPathPoly);
/*N*/ 
/*N*/ 	bCreating=FALSE;
/*N*/ 	ImpForceKind();
/*N*/ }

/*?*/ SdrPathObj::SdrPathObj(const Point& rPt1, const Point& rPt2)
/*?*/ {
/*?*/ 	eKind=OBJ_LINE;
/*?*/ 	XPolygon aXP(2);
/*?*/ 	aXP[0]=rPt1;
/*?*/ 	aXP[1]=rPt2;
/*?*/ 	aPathPolygon.Insert(aXP);
/*?*/ 	bClosedObj=FALSE;
/*?*/ 	bCreating=FALSE;
/*?*/ 	ImpForceLineWink();
/*?*/ }

/*N*/ SdrPathObj::~SdrPathObj()
/*N*/ {
/*N*/ }

/*N*/ FASTBOOL SdrPathObj::FindPolyPnt(USHORT nAbsPnt, USHORT& rPolyNum,
/*N*/ 							 USHORT& rPointNum, FASTBOOL bAllPoints) const
/*N*/ {
/*N*/ 	USHORT nPolyCnt=aPathPolygon.Count();
/*N*/ 	USHORT nPoly=0;
/*N*/ 	FASTBOOL bClosed=IsClosed();
/*N*/ 	nAbsPnt+=1;
/*N*/ 
/*N*/ 	while (nPoly<nPolyCnt) {
/*N*/ 		const XPolygon& rXPoly = aPathPolygon[nPoly];
/*N*/ 		USHORT nPnt=0;
/*N*/ 		USHORT nCnt=0;
/*N*/ 		USHORT nPntCnt=rXPoly.GetPointCount();
/*N*/ 		// geschlossen? Dann Endpunkt=Startpunkt
/*N*/ 		if (bClosed && nPntCnt>1) nPntCnt--;
/*N*/ 
/*N*/ 		while (nPnt<nPntCnt) {
/*N*/ 			if (bAllPoints || rXPoly.GetFlags(nPnt)!=XPOLY_CONTROL) nCnt++;
/*N*/ 			if (nCnt==nAbsPnt) {
/*N*/ 				rPolyNum=nPoly;
/*N*/ 				rPointNum=nPnt;
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 			nPnt++;
/*N*/ 		}
/*?*/ 		nAbsPnt-=nCnt;
/*?*/ 		nPoly++;
/*?*/ 	}
/*?*/ 	return FALSE;
/*N*/ }

/*N*/ void SdrPathObj::ImpForceLineWink()
/*N*/ {
/*N*/ 	if (aPathPolygon.Count()==1 && aPathPolygon[0].GetPointCount()==2 && eKind==OBJ_LINE) {
/*N*/ 		Point aDelt(aPathPolygon[0][1]);
/*N*/ 		aDelt-=aPathPolygon[0][0];
/*N*/ 		aGeo.nDrehWink=GetAngle(aDelt);
/*N*/ 		aGeo.nShearWink=0;
/*N*/ 		aGeo.RecalcSinCos();
/*N*/ 		aGeo.RecalcTan();
/*N*/ 
/*N*/ 		// #101412# for SdrTextObj, keep aRect up to date
/*N*/ 		aRect = Rectangle(aPathPolygon[0][0], aPathPolygon[0][1]);
/*N*/ 		aRect.Justify();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPathObj::ImpForceKind()
/*N*/ {
/*N*/ 	if (eKind==OBJ_PATHPLIN) eKind=OBJ_PLIN;
/*N*/ 	if (eKind==OBJ_PATHPOLY) eKind=OBJ_POLY;
/*N*/ 
/*N*/ 	USHORT nPolyAnz=aPathPolygon.Count();
/*N*/ 	USHORT nPoly1PointAnz=nPolyAnz==0 ? 0 : aPathPolygon[0].GetPointCount();
/*N*/ 	FASTBOOL bHasCtrl=FALSE;
/*N*/ 	FASTBOOL bHasLine=FALSE; // gemischt wird jedoch z.Zt. nicht in eKind festgehalten
/*N*/ 	for (USHORT nPolyNum=0; nPolyNum<nPolyAnz && (!bHasCtrl || !bHasLine); nPolyNum++) {
/*N*/ 		const XPolygon& rPoly=aPathPolygon[nPolyNum];
/*N*/ 		USHORT nPointAnz=rPoly.GetPointCount();
/*N*/ 		for (USHORT nPointNum=0; nPointNum<nPointAnz && (!bHasCtrl || !bHasLine); nPointNum++) {
/*N*/ 			if (rPoly.IsControl(nPointNum)) bHasCtrl=TRUE;
/*N*/ 			if (nPointNum+1<nPointAnz && !rPoly.IsControl(nPointNum) && !rPoly.IsControl(nPointNum+1)) bHasLine=TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!bHasCtrl) {
/*N*/ 		switch (eKind) {
/*N*/ 			case OBJ_PATHLINE: eKind=OBJ_PLIN; break;
/*N*/ 			case OBJ_FREELINE: eKind=OBJ_PLIN; break;
/*N*/ 			case OBJ_PATHFILL: eKind=OBJ_POLY; break;
/*N*/ 			case OBJ_FREEFILL: eKind=OBJ_POLY; break;
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		switch (eKind) {
/*N*/ 			case OBJ_LINE: eKind=OBJ_PATHLINE; break;
/*N*/ 			case OBJ_PLIN: eKind=OBJ_PATHLINE; break;
/*N*/ 			case OBJ_POLY: eKind=OBJ_PATHFILL; break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if (eKind==OBJ_LINE && (nPolyAnz!=1 || nPoly1PointAnz!=2)) eKind=OBJ_PLIN;
/*N*/ 	if (eKind==OBJ_PLIN && (nPolyAnz==1 && nPoly1PointAnz==2)) eKind=OBJ_LINE;
/*N*/ 
/*N*/ 	bClosedObj=IsClosed();
/*N*/ 
/*N*/ 	if (eKind==OBJ_LINE) 
/*N*/ 	{
/*N*/ 		ImpForceLineWink();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// #i10659#, similar to #101412# but for polys with more than 2 points.
/*N*/ 		//
/*N*/ 		// Here i again need to fix something, because when Path-Polys are Copy-Pasted
/*N*/ 		// between Apps with different measurements (e.g. 100TH_MM and TWIPS) there is
/*N*/ 		// a scaling loop started from SdrExchangeView::Paste. This is principally nothing
/*N*/ 		// wrong, but aRect is wrong here and not even updated by RecalcSnapRect(). If
/*N*/ 		// this is the case, some size needs to be set here in aRect to avoid that the cyclus
/*N*/ 		// through Rect2Poly - Poly2Rect does something badly wrong since that cycle is
/*N*/ 		// BASED on aRect. That cycle is triggered in SdrTextObj::NbcResize() which is called
/*N*/ 		// from the local Resize() implementation.
/*N*/ 		//
/*N*/ 		// Basic problem is that the member aRect in SdrTextObj basically is a unrotated
/*N*/ 		// text rectangle for the text object itself and methods at SdrTextObj do handle it
/*N*/ 		// in that way. Many draw objects derived from SdrTextObj 'abuse' aRect as SnapRect
/*N*/ 		// which is basically wrong. To make the SdrText methods which deal with aRect directly
/*N*/ 		// work it is necessary to always keep aRect updated. This e.g. not done after a Clone()
/*N*/ 		// command for SdrPathObj. Since adding this update mechanism with #101412# to
/*N*/ 		// ImpForceLineWink() for lines was very successful, i add it to where ImpForceLineWink()
/*N*/ 		// was called, once here below and once on a 2nd place below.
/*N*/ 
/*N*/ 		// #i10659# for SdrTextObj, keep aRect up to date
/*N*/ 		aRect=aPathPolygon.GetBoundRect(); // fuer SdrTextObj
/*N*/ 	}
/*N*/ }



/*N*/ UINT16 SdrPathObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return USHORT(eKind);
/*N*/ }

/*N*/ void SdrPathObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	aOutRect=GetSnapRect();
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	if (!IsClosed()) { // ggf. Linienenden beruecksichtigen
/*N*/ 		long nLEndWdt=ImpGetLineEndAdd();
/*N*/ 		if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
/*N*/ 	}
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		aOutRect.Left  ()-=nLineWdt;
/*N*/ 		aOutRect.Top   ()-=nLineWdt;
/*N*/ 		aOutRect.Right ()+=nLineWdt;
/*N*/ 		aOutRect.Bottom()+=nLineWdt;
/*N*/ 	}
/*N*/ 	ImpAddShadowToBoundRect();
/*N*/ 	ImpAddTextToBoundRect();
/*N*/ }

/*N*/ FASTBOOL SdrPathObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
/*N*/ {
/*N*/ 	// Hidden objects on masterpages, draw nothing
/*N*/ 	if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
/*N*/ 		return TRUE; 
/*N*/ 
/*N*/ 	BOOL bHideContour(IsHideContour());
/*N*/ 	BOOL bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));
/*N*/ 	BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));
/*N*/ 
/*N*/ 	// prepare ItemSet of this object
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 
/*N*/ 	// perepare ItemSet to avoid old XOut line drawing
/*N*/ 	SfxItemSet aEmptySet(*rSet.GetPool());
/*N*/ 	aEmptySet.Put(XLineStyleItem(XLINE_NONE));
/*N*/ 	aEmptySet.Put(XFillStyleItem(XFILL_NONE));
/*N*/ 
/*N*/     // #103692# prepare ItemSet for shadow fill attributes
/*N*/     SfxItemSet aShadowSet(rSet);
/*N*/ 
/*N*/ 	// prepare line geometry
/*N*/ 	::std::auto_ptr< SdrLineGeometry > pLineGeometry( ImpPrepareLineGeometry(rXOut, rSet, bIsLineDraft) );
/*N*/ 
/*N*/ 	// Shadows
/*N*/ 	if (!bHideContour && ImpSetShadowAttributes(rSet, aShadowSet))
/*N*/ 	{
/*?*/         if( !IsClosed() || bIsFillDraft )
/*?*/             rXOut.SetFillAttr(aEmptySet);
/*?*/         else
/*?*/             rXOut.SetFillAttr(aShadowSet);
/*?*/ 
/*?*/ 		UINT32 nXDist=((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
/*?*/ 		UINT32 nYDist=((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
/*?*/ 		XPolyPolygon aTmpXPoly(aPathPolygon);
/*?*/ 		aTmpXPoly.Move(nXDist,nYDist);
/*?*/ 
/*?*/ 		// avoid shadow line drawing in XOut
/*?*/ 		rXOut.SetLineAttr(aEmptySet);
/*?*/ 		
/*?*/         if (!IsClosed()) {
/*?*/             USHORT nPolyAnz=aTmpXPoly.Count();
/*?*/             for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
/*?*/                 rXOut.DrawXPolyLine(aTmpXPoly.GetObject(nPolyNum));
/*?*/             }
/*?*/         } else {
/*?*/             // #100127# Output original geometry for metafiles
/*?*/             ImpGraphicFill aFill( *this, rXOut, aShadowSet, true );
/*?*/             
/*?*/             rXOut.DrawXPolyPolygon(aTmpXPoly);
/*?*/         }
/*?*/ 
/*?*/ 		// new shadow line drawing
/*?*/ 		if( pLineGeometry.get() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 			// draw the line geometry
/*?*/ 		}
/*N*/ 	}
    
    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
    // avoid line drawing in XOut
/*N*/ 	rXOut.SetLineAttr(aEmptySet);
/*N*/ 
/*N*/     rXOut.SetFillAttr( bIsFillDraft || !IsClosed() ? aEmptySet : rSet );
/*N*/ 
/*N*/ 	if( !bHideContour )
/*N*/     {
/*N*/         if( IsClosed() )
/*N*/         {
/*N*/             // #100127# Output original geometry for metafiles
/*?*/             ImpGraphicFill aFill( *this, rXOut, bIsFillDraft || !IsClosed() ? aEmptySet : rSet );
/*?*/ 
/*?*/             rXOut.DrawXPolyPolygon(aPathPolygon);
/*N*/         }
/*N*/ 
/*N*/         // Own line drawing
/*N*/         if( pLineGeometry.get() )
/*N*/         {
/*N*/             // draw the line geometry
/*N*/             ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	FASTBOOL bOk=TRUE;
/*N*/ 	if (HasText()) {
/*?*/ 		bOk=SdrTextObj::Paint(rXOut,rInfoRec);
/*N*/ 	}
/*N*/ 	if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bOk=PaintGluePoints(rXOut,rInfoRec);
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }

/*N*/ SdrObject* SdrPathObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
/*N*/ 	INT32 nMyTol=nTol;
/*N*/ 	FASTBOOL bFilled=IsClosed() && (bTextFrame || HasFill());
/*N*/ 
/*N*/ 	INT32 nWdt=ImpGetLineWdt()/2; // Halbe Strichstaerke
/*N*/ 	if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
/*N*/ 	Rectangle aR(rPnt,rPnt);
/*N*/ 	aR.Left()  -=nMyTol;
/*N*/ 	aR.Right() +=nMyTol;
/*N*/ 	aR.Top()   -=nMyTol;
/*N*/ 	aR.Bottom()+=nMyTol;
/*N*/ 
/*N*/ 	FASTBOOL bHit=FALSE;
/*N*/ 	unsigned nPolyAnz=aPathPolygon.Count();
/*N*/ 	if (bFilled) {
/*N*/ 		PolyPolygon aPP;
/*N*/ 		for (unsigned nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
/*N*/ 			aPP.Insert(XOutCreatePolygon(aPathPolygon[nPolyNum],NULL));
/*N*/ 		}
/*N*/ 		bHit=IsRectTouchesPoly(aPP,aR);
/*N*/ 	} else {
/*N*/ 		for (unsigned nPolyNum=0; nPolyNum<nPolyAnz && !bHit; nPolyNum++) {
/*N*/ 			Polygon aPoly(XOutCreatePolygon(aPathPolygon[nPolyNum],NULL));
/*N*/ 			bHit=IsRectTouchesLine(aPoly,aR);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!bHit && !IsTextFrame() && HasText()) {
/*N*/ 		bHit=SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
/*N*/ 	}
/*N*/ 	return bHit ? (SdrObject*)this : NULL;
/*N*/ }

/*N*/ void SdrPathObj::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	SdrTextObj::operator=(rObj);
/*N*/ 	SdrPathObj& rPath=(SdrPathObj&)rObj;
/*N*/ 	aPathPolygon=rPath.aPathPolygon;
/*N*/ }



/*N*/ void SdrPathObj::TakeXorPoly(XPolyPolygon& rXPolyPoly, FASTBOOL bDetail) const
/*N*/ {
/*N*/ 	rXPolyPoly=aPathPolygon;
/*N*/ }

/*N*/ void SdrPathObj::TakeContour(XPolyPolygon& rPoly) const
/*N*/ {
/*N*/ 	// am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
/*N*/ 	SdrTextObj::TakeContour(rPoly);
/*N*/ }








/*?*/ inline USHORT GetPrevPnt(USHORT nPnt, USHORT nPntMax, FASTBOOL bClosed)
/*?*/ {
/*?*/ 	if (nPnt>0) {
/*?*/ 		nPnt--;
/*?*/ 	} else {
/*?*/ 		nPnt=nPntMax;
/*?*/ 		if (bClosed) nPnt--;
/*?*/ 	}
/*?*/ 	return nPnt;
/*?*/ }

/*?*/ inline USHORT GetNextPnt(USHORT nPnt, USHORT nPntMax, FASTBOOL bClosed)
/*?*/ {
/*?*/ 	nPnt++;
/*?*/ 	if (nPnt>nPntMax || (bClosed && nPnt>=nPntMax)) nPnt=0;
/*?*/ 	return nPnt;
/*?*/ }





























/*N*/ void SdrPathObj::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	SdrTextObj::NbcMove(rSiz);
/*N*/ 	MoveXPoly(aPathPolygon,rSiz);
/*N*/ }

/*N*/ void SdrPathObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrTextObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	ResizeXPoly(aPathPolygon,rRef,xFact,yFact);
/*N*/ }

/*N*/ void SdrPathObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
/*N*/ 	RotateXPoly(aPathPolygon,rRef,sn,cs);
/*N*/ }

/*N*/ void SdrPathObj::NbcShear(const Point& rRefPnt, long nAngle, double fTan, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	SdrTextObj::NbcShear(rRefPnt,nAngle,fTan,bVShear);
/*N*/ 	ShearXPoly(aPathPolygon,rRefPnt,fTan,bVShear);
/*N*/ }


/*N*/ void SdrPathObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	if (aGeo.nDrehWink==0) {
/*N*/ 		rRect=GetSnapRect();
/*N*/ 	} else {
/*N*/ 		XPolyPolygon aXPP(aPathPolygon);
/*N*/ 		RotateXPoly(aXPP,Point(),-aGeo.nSin,aGeo.nCos);
/*N*/ 		rRect=aXPP.GetBoundRect();
/*N*/ 		Point aTmp(rRect.TopLeft());
/*N*/ 		RotatePoint(aTmp,Point(),aGeo.nSin,aGeo.nCos);
/*N*/ 		aTmp-=rRect.TopLeft();
/*N*/ 		rRect.Move(aTmp.X(),aTmp.Y());
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPathObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	maSnapRect=aPathPolygon.GetBoundRect();
/*N*/ }

/*N*/ void SdrPathObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	Rectangle aOld(GetSnapRect());
/*N*/ 
/*N*/ 	// #95736# Take RECT_EMPTY into account when calculating scale factors
/*N*/ 	long nMulX = (RECT_EMPTY == rRect.Right()) ? 0 : rRect.Right()  - rRect.Left();
/*N*/ 	
/*N*/ 	long nDivX = aOld.Right()   - aOld.Left();
/*N*/ 	
/*N*/ 	// #95736# Take RECT_EMPTY into account when calculating scale factors
/*N*/ 	long nMulY = (RECT_EMPTY == rRect.Bottom()) ? 0 : rRect.Bottom() - rRect.Top();
/*N*/ 	
/*N*/ 	long nDivY = aOld.Bottom()  - aOld.Top();
/*N*/ 	if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
/*N*/ 	if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
/*N*/ 	Fraction aX(nMulX,nDivX);
/*N*/ 	Fraction aY(nMulY,nDivY);
/*N*/ 	NbcResize(aOld.TopLeft(), aX, aY);
/*N*/ 	NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
/*N*/ }



/*N*/ FASTBOOL SdrPathObj::IsPolyObj() const
/*N*/ {
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ USHORT SdrPathObj::GetPointCount() const
/*N*/ {
/*N*/ 	USHORT nPolyCnt = aPathPolygon.Count();
/*N*/ 	USHORT nPntCnt = 0;
/*N*/ 
/*N*/ 	for (USHORT i = 0; i < nPolyCnt; i++)
/*N*/ 		nPntCnt += aPathPolygon[i].GetPointCount();
/*N*/ 
/*N*/ 	return nPntCnt;
/*N*/ }

/*N*/ const Point& SdrPathObj::GetPoint(USHORT nHdlNum) const
/*N*/ {
/*N*/ 	USHORT nPoly,nPnt;
/*N*/ 	FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE);
/*N*/ 	return aPathPolygon[nPoly][nPnt];
/*N*/ }

/*N*/ void SdrPathObj::NbcSetPoint(const Point& rPnt, USHORT nHdlNum)
/*N*/ {
/*N*/ 	USHORT nPoly,nPnt;
/*N*/ 
/*N*/ 	if (FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE)) {
/*N*/ 		XPolygon& rXP=aPathPolygon[nPoly];
/*N*/ 		rXP[nPnt]=rPnt;
/*N*/ 		if (IsClosed() && nPnt==0) rXP[rXP.GetPointCount()-1]=rXP[0];
/*N*/ 
/*N*/ 		if (eKind==OBJ_LINE) 
/*N*/ 		{
/*N*/ 			ImpForceLineWink();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// #i10659# for SdrTextObj, keep aRect up to date
/*N*/ 			aRect=aPathPolygon.GetBoundRect(); // fuer SdrTextObj
/*N*/ 		}
/*N*/ 
/*N*/ 		SetRectsDirty();
/*N*/ 	}
/*N*/ }

/*************************************************************************/
// Abstand des Punktes Pt zur Strecke AB. Hat der Punkt Pt keine Senkrechte
// zur Strecke AB, so ist der Abstand zum naechstliegenden Punkt verwendet;
// dazu wird nocheinmal der einfache Abstand parallel zu AB draufaddiert
// (als Winkelersatz) (=> groesserer Abstand=unguenstigerer Winkel).









/*N*/ SdrObjGeoData* SdrPathObj::NewGeoData() const
/*N*/ {
/*N*/ 	return new SdrPathObjGeoData;
/*N*/ }

/*N*/ void SdrPathObj::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	SdrTextObj::SaveGeoData(rGeo);
/*N*/ 	SdrPathObjGeoData& rPGeo = (SdrPathObjGeoData&) rGeo;
/*N*/ 	rPGeo.aPathPolygon=aPathPolygon;
/*N*/ 	rPGeo.eKind=eKind;
/*N*/ }


/*N*/ void SdrPathObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrTextObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrPathObj");
/*N*/ #endif
/*N*/ 	{
/*N*/ 		SdrDownCompat aPathCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aPathCompat.SetID("SdrPathObj(PathPolygon)");
/*N*/ #endif
/*N*/ 		rOut<<aPathPolygon;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrPathObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrTextObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrPathObj");
/*N*/ #endif
/*N*/ 	aPathPolygon.Clear();
/*N*/ 	if (rHead.GetVersion()<=6 && (rHead.nIdentifier==OBJ_LINE || rHead.nIdentifier==OBJ_POLY || rHead.nIdentifier==OBJ_PLIN)) {
/*?*/ 		// SdrPolyObj importieren
/*?*/ 		switch (eKind) {
/*?*/ 			case OBJ_LINE: {
/*?*/ 				Polygon aP(2);
/*?*/ 				rIn>>aP[0];
/*?*/ 				rIn>>aP[1];
/*?*/ 				aPathPolygon=XPolyPolygon(PolyPolygon(aP));
/*?*/ 			} break;
/*?*/ 			case OBJ_PLIN: {
/*?*/ 				Polygon aP;
/*?*/ 				rIn>>aP;
/*?*/ 				aPathPolygon=XPolyPolygon(PolyPolygon(aP));
/*?*/ 			} break;
/*?*/ 			default: {
/*?*/ 				PolyPolygon aPoly;
/*?*/ 				rIn>>aPoly;
/*?*/ 				aPathPolygon=XPolyPolygon(aPoly);
/*?*/ 				// und nun die Polygone ggf. durch einfuegen eines weiteren Punktes schliessen
/*?*/ 				USHORT nPolyAnz=aPathPolygon.Count();
/*?*/ 				for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
/*?*/ 					const XPolygon& rPoly=aPathPolygon[nPolyNum];
/*?*/ 					USHORT nPointAnz=rPoly.GetPointCount();
/*?*/ 					aPathPolygon[nPolyNum].GetPointCount();
/*?*/ 					if (nPointAnz>=2 && rPoly[0]!=rPoly[USHORT(nPointAnz-1)]) {
/*?*/ 						Point aPt(rPoly[0]);
/*?*/ 						aPathPolygon[nPolyNum][nPointAnz]=aPt;
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	} else {
/*N*/ 		if (rHead.GetVersion()>=11) { // ab V11 ist das eingepackt
/*N*/ 			SdrDownCompat aPathCompat(rIn,STREAM_READ);
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aPathCompat.SetID("SdrPathObj(PathPolygon)");
/*N*/ #endif
/*N*/ 			rIn>>aPathPolygon;
/*N*/ 		} else {
/*N*/ 			rIn>>aPathPolygon;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	ImpForceKind(); // ggf. den richtigen Identifier herstellen.
/*N*/ }

/*N*/ void SdrPathObj::NbcSetPathPoly(const XPolyPolygon& rPathPoly)
/*N*/ {
/*N*/ 	aPathPolygon=rPathPoly;
/*N*/ 	ImpForceKind();
/*N*/ 	if (IsClosed()) {
/*N*/ 		USHORT nPolyAnz=aPathPolygon.Count();
/*N*/ 		for (USHORT nPolyNum=nPolyAnz; nPolyNum>0;) {
/*N*/ 			nPolyNum--;
/*N*/ 			const XPolygon& rConstXP=aPathPolygon[nPolyNum];
/*N*/ 			USHORT nPointAnz=rConstXP.GetPointCount();
/*N*/ 			if (nPointAnz!=0) {
/*N*/ 				Point aStartPt(rConstXP[0]);
/*N*/ 				if (rConstXP[nPointAnz-1]!=aStartPt) {
/*N*/ 					// Polygon schliessen (wird dabei um einen Punkt erweitert)
/*N*/ 					aPathPolygon[nPolyNum][nPointAnz]=aStartPt;
/*N*/ 				}
/*N*/ 			} else {
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aPathPolygon.Remove(nPolyNum); // leere Polygone raus
/*N*/ 			}
/*N*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetRectsDirty();
/*N*/ }

/*N*/ void SdrPathObj::SetPathPoly(const XPolyPolygon& rPathPoly)
/*N*/ {
/*N*/ 	Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	NbcSetPathPoly(rPathPoly);
/*N*/ 	SetChanged();
/*N*/ 	SendRepaintBroadcast();
/*N*/ 	SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
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
/*N*/ BOOL SdrPathObj::TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const
/*N*/ {
/*N*/ 	double fRotate;
/*N*/ 	double fShear;
/*N*/ 	Rectangle aRectangle;
/*N*/ 
/*N*/ 	if(eKind==OBJ_LINE)
/*N*/ 	{
/*N*/ 		// #85920# special handling for single line mode (2 points)
/*N*/ 		XPolygon aLine(2);
/*N*/ 		aLine[0] = GetPoint(0);
/*N*/ 		aLine[1] = GetPoint(1);
/*N*/ 		rPolyPolygon.Clear();
/*N*/ 		rPolyPolygon.Insert(aLine);
/*N*/ 		aRectangle = rPolyPolygon.GetBoundRect();
/*N*/ 
/*N*/ 		// fill in values
/*N*/ 		fRotate = fShear = 0.0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// get turn and shear
/*N*/ 		fRotate = (aGeo.nDrehWink / 100.0) * F_PI180;
/*N*/ 		fShear = (aGeo.nShearWink / 100.0) * F_PI180;
/*N*/ 
/*N*/ 		// get path, remove rotate and shear
/*N*/ 		rPolyPolygon = GetPathPoly();
/*N*/ 		if(aGeo.nDrehWink)
/*N*/ 			RotateXPoly(rPolyPolygon, Point(), -aGeo.nSin, aGeo.nCos);
/*N*/ 		
/*N*/ 		aRectangle = rPolyPolygon.GetBoundRect();
/*N*/ 		Point aTmp(aRectangle.TopLeft());
/*N*/ 		
/*N*/ 		if(aGeo.nShearWink)
/*N*/ 		{
/*N*/ 			ShearXPoly(rPolyPolygon, aTmp, -aGeo.nTan, FALSE);
/*N*/ 			aRectangle = rPolyPolygon.GetBoundRect();
/*N*/ 			aTmp = aRectangle.TopLeft();
/*N*/ 		}
/*N*/ 		
/*N*/ 		RotatePoint(aTmp, Point(), aGeo.nSin, aGeo.nCos);
/*N*/ 		aTmp -= aRectangle.TopLeft();
/*N*/ 
/*N*/ 		// polygon to base position
/*N*/ 		rPolyPolygon.Move(aTmp.X(), aTmp.Y());
/*N*/ 
/*N*/ 		// get bound rect for values
/*N*/ 		aRectangle = rPolyPolygon.GetBoundRect();
/*N*/ 	}
/*N*/ 
/*N*/ 	// fill in values
/*N*/ 	Vector2D aScale((double)aRectangle.GetWidth(), (double)aRectangle.GetHeight());
/*N*/ 	Vector2D aTranslate((double)aRectangle.Left(), (double)aRectangle.Top());
/*N*/ 
/*N*/ 	// polygon to (0,0)
/*N*/ 	rPolyPolygon.Move(-aRectangle.Left(), -aRectangle.Top());
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
/*N*/ 				// polygon
/*N*/ 				for(sal_uInt16 a(0); a < rPolyPolygon.Count(); a++)
/*N*/ 				{
/*N*/ 					XPolygon& rPoly = rPolyPolygon[a];
/*N*/ 					for(sal_uInt16 b(0); b < rPoly.GetPointCount(); b++)
/*N*/ 					{
/*N*/ 						rPoly[b].X() = ImplTwipsToMM(rPoly[b].X());
/*N*/ 						rPoly[b].Y() = ImplTwipsToMM(rPoly[b].Y());
/*N*/ 					}
/*N*/ 				}
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
/*N*/ 	return TRUE;
/*N*/ }

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix. 
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has 
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
/*N*/ void SdrPathObj::TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon)
/*N*/ {
/*N*/ 	// break up matrix
/*N*/ 	Vector2D aScale, aTranslate;
/*N*/ 	double fShear, fRotate;
/*N*/ 	rMat.DecomposeAndCorrect(aScale, fShear, fRotate, aTranslate);
/*N*/ 
/*N*/ 	// copy poly
/*N*/ 	XPolyPolygon aNewPolyPolygon(rPolyPolygon);
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
/*N*/ 				// polygon
/*N*/ 				for(sal_uInt16 a(0); a < aNewPolyPolygon.Count(); a++)
/*N*/ 				{
/*N*/ 					XPolygon& rPoly = aNewPolyPolygon[a];
/*N*/ 					for(sal_uInt16 b(0); b < rPoly.GetPointCount(); b++)
/*N*/ 					{
/*N*/ 						rPoly[b].X() = ImplMMToTwips(rPoly[b].X());
/*N*/ 						rPoly[b].Y() = ImplMMToTwips(rPoly[b].Y());
/*N*/ 					}
/*N*/ 				}
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
/*N*/ 	if( pModel->IsWriter() )
/*N*/ 	{
/*N*/ 		// if anchor is used, make position relative to it
/*N*/ 		if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
/*N*/ 			aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());
/*N*/ 	}
/*N*/ 
/*N*/ 	// set PathPoly and get type
/*N*/ 	SetPathPoly(aNewPolyPolygon);
/*N*/ 
/*N*/ 	if(eKind==OBJ_LINE)
/*N*/ 	{
/*N*/ 		// #85920# special handling for single line mode (2 points)
/*N*/ 		Point aPoint1 = aNewPolyPolygon[0][0];
/*N*/ 		Point aPoint2 = aNewPolyPolygon[0][1];
/*N*/ 
/*N*/ 		// shear?
/*N*/ 		if(fShear != 0.0)
/*N*/ 		{
/*?*/ 			GeoStat aGeoStat;
/*?*/ 			aGeoStat.nShearWink = FRound((atan(fShear) / F_PI180) * 100.0);
/*?*/ 			aGeoStat.RecalcTan();
/*?*/ 			ShearPoint(aPoint1, Point(), aGeoStat.nTan, FALSE);
/*?*/ 			ShearPoint(aPoint2, Point(), aGeoStat.nTan, FALSE);
/*N*/ 		}
/*N*/ 
/*N*/ 		// rotation?
/*N*/ 		if(fRotate != 0.0)
/*N*/ 		{
/*?*/ 			GeoStat aGeoStat;
/*?*/ 			aGeoStat.nDrehWink = FRound((fRotate / F_PI180) * 100.0);
/*?*/ 			aGeoStat.RecalcSinCos();
/*?*/ 			RotatePoint(aPoint1, Point(), aGeoStat.nSin, aGeoStat.nCos);
/*?*/ 			RotatePoint(aPoint2, Point(), aGeoStat.nSin, aGeoStat.nCos);
/*N*/ 		}
/*N*/ 
/*N*/ 		// translate?
/*N*/ 		if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
/*N*/ 		{
/*N*/ 			Point aOffset((sal_Int32)FRound(aTranslate.X()), (sal_Int32)FRound(aTranslate.Y()));
/*N*/ 			aPoint1 += aOffset;
/*N*/ 			aPoint2 += aOffset;
/*N*/ 		}
/*N*/ 
/*N*/ 		// put points back to poly
/*N*/ 		aNewPolyPolygon[0][0] = aPoint1;
/*N*/ 		aNewPolyPolygon[0][1] = aPoint2;
/*N*/ 
/*N*/ 		// set PathPoly again; this sets all of JOEs old needed stati and values
/*N*/ 		SetPathPoly(aNewPolyPolygon);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// shear?
/*N*/ 		if(fShear != 0.0)
/*N*/ 		{
/*N*/ 			GeoStat aGeoStat;
/*N*/ 			aGeoStat.nShearWink = FRound((atan(fShear) / F_PI180) * 100.0);
/*N*/ 			aGeoStat.RecalcTan();
/*N*/ 			Shear(Point(), aGeoStat.nShearWink, aGeoStat.nTan, FALSE);
/*N*/ 		}
/*N*/ 
/*N*/ 		// rotation?
/*N*/ 		if(fRotate != 0.0)
/*N*/ 		{
/*N*/ 			GeoStat aGeoStat;
/*N*/ 			aGeoStat.nDrehWink = FRound((fRotate / F_PI180) * 100.0);
/*N*/ 			aGeoStat.RecalcSinCos();
/*N*/ 			Rotate(Point(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
/*N*/ 		}
/*N*/ 
/*N*/ 		// translate?
/*N*/ 		if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
/*N*/ 		{
/*N*/ 			Move(Size(
/*N*/ 				(sal_Int32)FRound(aTranslate.X()), 
/*N*/ 				(sal_Int32)FRound(aTranslate.Y())));
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// EOF
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
