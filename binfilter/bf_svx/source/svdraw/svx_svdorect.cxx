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

//#include <math.h>
#include <stdlib.h>
#include "xpool.hxx"
#include "svdpool.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdpage.hxx"
#include "svdocapt.hxx" // fuer Import von SdrFileVersion 2
#include "svdview.hxx" // das
#include "svdstr.hrc"   // Objektname

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SVX_RECTENUM_HXX
#include "rectenum.hxx"
#endif

#ifndef _SVX_SVDOIMP_HXX
#include "svdoimp.hxx"
#endif

#ifndef _XOUTX_HXX
#include "xoutx.hxx"
#endif

namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrRectObj,SdrTextObj);

/*N*/ SdrRectObj::SdrRectObj():
/*N*/ 	pXPoly(NULL),
/*N*/ 	bXPolyIsLine(FALSE)
/*N*/ {
/*N*/ 	bClosedObj=TRUE;
/*N*/ }

/*N*/ SdrRectObj::SdrRectObj(const Rectangle& rRect):
/*N*/ 	SdrTextObj(rRect),
/*N*/ 	pXPoly(NULL),
/*N*/ 	bXPolyIsLine(FALSE)
/*N*/ {
/*N*/ 	bClosedObj=TRUE;
/*N*/ }

/*N*/ SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind):
/*N*/ 	SdrTextObj(eNewTextKind),
/*N*/ 	pXPoly(NULL),
/*N*/ 	bXPolyIsLine(FALSE)
/*N*/ {
/*N*/ 	DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
/*N*/ 			   eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
/*N*/ 			   "SdrRectObj::SdrRectObj(SdrObjKind) ist nur fuer Textrahmen gedacht");
/*N*/ 	bClosedObj=TRUE;
/*N*/ }

/*N*/ SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect):
/*N*/ 	SdrTextObj(eNewTextKind,rRect),
/*N*/ 	pXPoly(NULL),
/*N*/ 	bXPolyIsLine(FALSE)
/*N*/ {
/*N*/ 	DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
/*N*/ 			   eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
/*N*/ 			   "SdrRectObj::SdrRectObj(SdrObjKind,...) ist nur fuer Textrahmen gedacht");
/*N*/ 	bClosedObj=TRUE;
/*N*/ }

/*N*/ SdrRectObj::~SdrRectObj()
/*N*/ {
/*N*/ 	if (pXPoly!=NULL) delete pXPoly;
/*N*/ }

/*N*/ void SdrRectObj::SetXPolyDirty()
/*N*/ {
/*N*/ 	if (pXPoly!=NULL) {
/*N*/ 		delete pXPoly;
/*N*/ 		pXPoly=NULL;
/*N*/ 	}
/*N*/ }

/*N*/ FASTBOOL SdrRectObj::PaintNeedsXPoly(long nEckRad) const
/*N*/ {
/*N*/ 	FASTBOOL bNeed=aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || nEckRad!=0;
/*N*/ 	return bNeed;
/*N*/ }

/*N*/ XPolygon SdrRectObj::ImpCalcXPoly(const Rectangle& rRect1, long nRad1, FASTBOOL bContour) const
/*N*/ {
/*N*/ 	bContour=TRUE; // am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
/*N*/ 	XPolygon aXPoly(rRect1,nRad1,nRad1);
/*N*/ 	if (bContour) {
/*N*/ 		unsigned nPointAnz=aXPoly.GetPointCount();
/*N*/ 		XPolygon aNeuPoly(nPointAnz+1);
/*N*/ 		unsigned nShift=nPointAnz-2;
/*N*/ 		if (nRad1!=0) nShift=nPointAnz-5;
/*N*/ 		unsigned j=nShift;
/*N*/ 		for (unsigned i=1; i<nPointAnz; i++) {
/*N*/ 			aNeuPoly[i]=aXPoly[j];
/*N*/ 			aNeuPoly.SetFlags(i,aXPoly.GetFlags(j));
/*N*/ 			j++;
/*N*/ 			if (j>=nPointAnz) j=1;
/*N*/ 		}
/*N*/ 		aNeuPoly[0]=rRect1.BottomCenter();
/*N*/ 		aNeuPoly[nPointAnz]=aNeuPoly[0];
/*N*/ 		aXPoly=aNeuPoly;
/*N*/ 	}
/*N*/ 	// Die Winkelangaben beziehen sich immer auf die linke obere Ecke von !aRect!
/*N*/ 	if (aGeo.nShearWink!=0) ShearXPoly(aXPoly,aRect.TopLeft(),aGeo.nTan);
/*N*/ 	if (aGeo.nDrehWink!=0) RotateXPoly(aXPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 	return aXPoly;
/*N*/ }

/*N*/ void SdrRectObj::RecalcXPoly()
/*N*/ { // #i37639# Needed for calc import
/*N*/	pXPoly=new XPolygon(ImpCalcXPoly(aRect,GetEckenradius()));
/*N*/ }

/*N*/ const XPolygon& SdrRectObj::GetXPoly() const
/*N*/ {
/*N*/ 	if (pXPoly==NULL) ((SdrRectObj*)this)->RecalcXPoly();
/*N*/ 	return *pXPoly;
/*N*/ }


/*N*/ UINT16 SdrRectObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	if (IsTextFrame()) return UINT16(eTextKind);
/*N*/ 	else return UINT16(OBJ_RECT);
/*N*/ }

/*N*/ void SdrRectObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	aOutRect=GetSnapRect();
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	nLineWdt++; nLineWdt/=2;
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		long a=nLineWdt;
/*N*/ 		if ((aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) && GetEckenradius()==0) {
/*N*/ 			a*=2; // doppelt, wegen evtl. spitzen Ecken
/*N*/ 		}
/*N*/ 		aOutRect.Left  ()-=a;
/*N*/ 		aOutRect.Top   ()-=a;
/*N*/ 		aOutRect.Right ()+=a;
/*N*/ 		aOutRect.Bottom()+=a;
/*N*/ 	}
/*N*/ 	ImpAddShadowToBoundRect();
/*N*/ 	ImpAddTextToBoundRect();
/*N*/ }

/*N*/ void SdrRectObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	rRect=aRect;
/*N*/ 	if (aGeo.nShearWink!=0) {
/*N*/ 		long nDst=Round((aRect.Bottom()-aRect.Top())*aGeo.nTan);
/*N*/ 		if (aGeo.nShearWink>0) {
/*?*/ 			Point aRef(rRect.TopLeft());
/*?*/ 			rRect.Left()-=nDst;
/*?*/ 			Point aTmpPt(rRect.TopLeft());
/*?*/ 			RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
/*?*/ 			aTmpPt-=rRect.TopLeft();
/*?*/ 			rRect.Move(aTmpPt.X(),aTmpPt.Y());
/*N*/ 		} else {
/*N*/ 			rRect.Right()-=nDst;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ FASTBOOL SdrRectObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
/*N*/ {
/*N*/ 	// Hidden objects on masterpages, draw nothing
/*N*/ 	if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	// Im Graustufenmodus/Kontrastmodus soll die Hintergrundseite NICHT angezeigt werden
/*N*/ 	ULONG nMode = rXOut.GetOutDev()->GetDrawMode();
/*N*/ 	FASTBOOL bGrayscaleMode = ( nMode == (DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
/*N*/ 	FASTBOOL bSettingsMode = ( nMode == (DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT ) );
/*N*/ 	
/*N*/     if( ( bGrayscaleMode || bSettingsMode ) && pPage && pPage->IsMasterPage() )
/*N*/ 	{
/*?*/ 		Size aPageSize = pPage->GetSize();
/*?*/ 		long aRectWidth = aRect.GetSize().Width() - 1;
/*?*/ 		long aRectHeight = aRect.GetSize().Height() - 1;
/*?*/ 
/*?*/ 		// Objekt so gross wie Seite ? -> Hintergrund
/*?*/ 		if( aRectWidth == aPageSize.Width() &&
/*?*/ 			aRectHeight == aPageSize.Height()  )
/*?*/ 		{
/*?*/ 			return TRUE;
/*?*/ 		}
/*?*/ 		// oder so gross wie Seite abzueglich der Raender
/*?*/ 		if( aRectWidth == aPageSize.Width() -
/*?*/ 				pPage->GetLftBorder() - pPage->GetRgtBorder() &&
/*?*/ 			aRectHeight == aPageSize.Height() -
/*?*/ 				pPage->GetUppBorder() - pPage->GetLwrBorder() )
/*?*/ 		{
/*?*/ 			return TRUE;
/*?*/ 		}
/*?*/ 
/*N*/ 	}

/*N*/ 	if (bTextFrame && aGeo.nShearWink!=0) {
/*?*/ 		DBG_WARNING("Shearwinkel vom TextFrame innerhalb von SdrRectObj::Paint() auf 0 gesetzt");
/*?*/ 		((SdrRectObj*)this)->ImpCheckShear();
/*?*/ 		((SdrRectObj*)this)->SetRectsDirty();
/*N*/ 	}
/*N*/ 	FASTBOOL bOk=TRUE;
/*N*/ 	BOOL bHideContour(IsHideContour());
/*N*/ 	sal_Int32 nEckRad(GetEckenradius());
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
/*?*/         if( bIsFillDraft )
/*?*/             rXOut.SetFillAttr(aEmptySet);
/*?*/         else
/*?*/             rXOut.SetFillAttr(aShadowSet);
/*?*/ 
/*?*/ 		UINT32 nXDist=((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
/*?*/ 		UINT32 nYDist=((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
/*?*/ 
/*?*/ 		// avoid shadow line drawing in XOut
/*?*/ 		rXOut.SetLineAttr(aEmptySet);
/*?*/ 
/*?*/         {
/*?*/             // #100127# Output original geometry for metafiles
/*?*/             ImpGraphicFill aFill( *this, rXOut, aShadowSet, true );
/*?*/ 
/*?*/             if (PaintNeedsXPoly(nEckRad)) {
/*?*/                 XPolygon aX(GetXPoly());
/*?*/                 aX.Move(nXDist,nYDist);
/*?*/                 rXOut.DrawXPolygon(aX);
/*?*/             } else {
/*?*/                 Rectangle aR(aRect);
/*?*/                 aR.Move(nXDist,nYDist);
/*?*/                 rXOut.DrawRect(aR,USHORT(2*nEckRad),USHORT(2*nEckRad));
/*?*/             }
/*?*/         }
/*?*/ 
/*?*/ 		// new shadow line drawing
/*?*/ 		if( pLineGeometry.get() )
/*?*/ 		{
/*?*/ 			// draw the line geometry
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ImpDrawShadowLineGeometry(rXOut, rSet, *pLineGeometry);
/*?*/ 		}
/*N*/ 	}

    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
/*N*/ 	rXOut.SetLineAttr(aEmptySet);
/*N*/ 
/*N*/     rXOut.SetFillAttr( bIsFillDraft ? aEmptySet : rSet );
/*N*/ 
/*N*/ 	if (!bHideContour) {
/*N*/         // #100127# Output original geometry for metafiles
/*N*/         ImpGraphicFill aFill( *this, rXOut, bIsFillDraft ? aEmptySet : rSet );
/*N*/         
/*N*/ 		if (PaintNeedsXPoly(nEckRad)) {
/*?*/ 			rXOut.DrawXPolygon(GetXPoly());
/*N*/ 		} else {
/*N*/ 			DBG_ASSERT(nEckRad==0,"SdrRectObj::Paint(): XOut.DrawRect() unterstuetz kein Eckenradius!");
/*N*/ 			rXOut.DrawRect(aRect/*,USHORT(2*nEckRad),USHORT(2*nEckRad)*/);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	DBG_ASSERT(aRect.GetWidth()>1 && aRect.GetHeight()>1,"SdrRectObj::Paint(): Rect hat Nullgroesse (oder negativ)!");
/*N*/ 
/*N*/ 	// Own line drawing
/*N*/ 	if( !bHideContour && pLineGeometry.get() )
/*N*/ 	{
/*N*/ 		// draw the line geometry
/*N*/ 		ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
/*N*/ 	}
/*N*/ 
/*N*/ 	if (HasText()) {
/*?*/ 		bOk=SdrTextObj::Paint(rXOut,rInfoRec);
/*N*/ 	}
/*N*/ 	if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 bOk=PaintGluePoints(rXOut,rInfoRec);
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }

/*N*/ SdrObject* SdrRectObj::ImpCheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bForceFilled, FASTBOOL bForceTol) const
/*N*/ {
/*N*/ 	if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
/*N*/ 	INT32 nMyTol=nTol;
/*N*/ 	FASTBOOL bFilled=bForceFilled || HasFill();
/*N*/ 	FASTBOOL bPickThrough=pModel!=NULL && pModel->IsPickThroughTransparentTextFrames();
/*N*/ 	if (bTextFrame && !bPickThrough) bFilled=TRUE;
/*N*/ 	FASTBOOL bLine=HasLine();
/*N*/ 
/*N*/ 	INT32 nWdt=bLine ? ImpGetLineWdt()/2 :0; // Halbe Strichstaerke
/*N*/ 	long nBoundWdt=aRect.GetWidth()-1;
/*N*/ 	long nBoundHgt=aRect.GetHeight()-1;
/*N*/ 	if (bFilled && nBoundWdt>short(nTol) && nBoundHgt>short(nTol) && Abs(aGeo.nShearWink)<=4500) {
/*N*/ 		if (!bForceTol && !bTextFrame ) nMyTol=0; // Keine Toleranz noetig hier
/*N*/ 	}
/*N*/ 	if (nWdt>nMyTol && (!bTextFrame || pEdtOutl==NULL)) nMyTol=nWdt; // Bei dicker Umrandung keine Toleranz noetig, ausser wenn bei TextEdit
/*N*/ 	Rectangle aR(aRect);
/*N*/ 	if (nMyTol!=0 && bFilled) {
/*N*/ 		aR.Left  ()-=nMyTol;
/*N*/ 		aR.Top   ()-=nMyTol;
/*N*/ 		aR.Right ()+=nMyTol;
/*N*/ 		aR.Bottom()+=nMyTol;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (bFilled || bLine || bTextFrame) { // Bei TextFrame so tun, alsob Linie da
/*N*/ 		unsigned nCnt=0;
/*N*/ 		INT32 nXShad=0,nYShad=0;
/*N*/ 		long nEckRad=/*bTextFrame ? 0 :*/ GetEckenradius();
/*N*/ 		do { // 1 Durchlauf, bei Schatten 2 Durchlaeufe.
/*N*/ 			if (nCnt!=0) aR.Move(nXShad,nYShad);
/*N*/ 			if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || nEckRad!=0 || !bFilled) {
/*N*/ 				Polygon aPol(aR);
/*N*/ 				if (nEckRad!=0) {
/*N*/ 					INT32 nRad=nEckRad;
/*N*/ 					if (bFilled) nRad+=nMyTol; // um korrekt zu sein ...
/*N*/ 					XPolygon aXPoly(ImpCalcXPoly(aR,nRad));
/*N*/ 					aPol=XOutCreatePolygon(aXPoly,NULL);
/*N*/ 				} else {
/*N*/ 					if (aGeo.nShearWink!=0) ShearPoly(aPol,aRect.TopLeft(),aGeo.nTan);
/*N*/ 					if (aGeo.nDrehWink!=0) RotatePoly(aPol,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 				}
/*N*/ 				if (bFilled) {
/*N*/ 					if (IsPointInsidePoly(aPol,rPnt)) return (SdrObject*)this;
/*N*/ 				} else {
/*N*/ 					Rectangle aTouchRect(rPnt.X()-nMyTol,rPnt.Y()-nMyTol,rPnt.X()+nMyTol,rPnt.Y()+nMyTol);
/*N*/ 					if (IsRectTouchesLine(aPol,aTouchRect)) return (SdrObject*)this;
/*N*/ 				}
/*N*/ 			} else {
/*N*/ 				if (aR.IsInside(rPnt)) return (SdrObject*)this;
/*N*/ 			}
/*N*/ 		} while (nCnt++==0 && ImpGetShadowDist(nXShad,nYShad));
/*N*/ 	}
/*N*/ 	FASTBOOL bCheckText=TRUE;
/*N*/ 	if (bCheckText && HasText() && (!bTextFrame || bPickThrough)) {
/*N*/ 		return SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer);
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ SdrObject* SdrRectObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
/*N*/ {
/*N*/ 	return ImpCheckHit(rPnt,nTol,pVisiLayer,FALSE/*,bTextFrame*/);
/*N*/ }



/*N*/ void SdrRectObj::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	SdrTextObj::operator=(rObj);
/*N*/ }

/*N*/ void SdrRectObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
/*N*/ {
/*N*/ 	rPoly=XPolyPolygon(ImpCalcXPoly(aRect,GetEckenradius()));
/*N*/ }

/*N*/ void SdrRectObj::TakeContour(XPolyPolygon& rPoly) const
/*N*/ {
/*N*/ 	SdrTextObj::TakeContour(rPoly);
/*N*/ }


/*N*/ void SdrRectObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	long nEckRad=GetEckenradius();
/*N*/ 	if ((aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) && nEckRad!=0) {
/*?*/ 		maSnapRect=GetXPoly().GetBoundRect();
/*N*/ 	} else {
/*N*/ 		SdrTextObj::RecalcSnapRect();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrRectObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrTextObj::NbcSetSnapRect(rRect);
/*N*/ 	SetXPolyDirty();
/*N*/ }

/*N*/ void SdrRectObj::NbcSetLogicRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	SdrTextObj::NbcSetLogicRect(rRect);
/*N*/ 	SetXPolyDirty();
/*N*/ }











/*N*/ Pointer SdrRectObj::GetCreatePointer() const
/*N*/ {
/*N*/ 	if (IsTextFrame()) return Pointer(POINTER_DRAW_TEXT);
/*N*/ 	return Pointer(POINTER_DRAW_RECT);
/*N*/ }

/*N*/ void SdrRectObj::NbcMove(const Size& rSiz)
/*N*/ {
/*N*/ 	SdrTextObj::NbcMove(rSiz);
/*N*/ 	SetXPolyDirty();
/*N*/ }

/*N*/ void SdrRectObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	SdrTextObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	SetXPolyDirty();
/*N*/ }

/*N*/ void SdrRectObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
/*N*/ {
/*N*/ 	SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
/*N*/ 	SetXPolyDirty();
/*N*/ }

/*N*/ void SdrRectObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
/*N*/ 	SetXPolyDirty();
/*N*/ }




/*N*/ SdrGluePoint SdrRectObj::GetVertexGluePoint(USHORT nPosNum) const
/*N*/ {
/*N*/ 	INT32 nWdt = ((XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
/*N*/ 	nWdt++;
/*N*/ 	nWdt /= 2;
/*N*/ 
/*N*/ 	Point aPt;
/*N*/ 	switch (nPosNum) {
/*N*/ 		case 0: aPt=aRect.TopCenter();    aPt.Y()-=nWdt; break;
/*N*/ 		case 1: aPt=aRect.RightCenter();  aPt.X()+=nWdt; break;
/*N*/ 		case 2: aPt=aRect.BottomCenter(); aPt.Y()+=nWdt; break;
/*N*/ 		case 3: aPt=aRect.LeftCenter();   aPt.X()-=nWdt; break;
/*N*/ 	}
/*N*/ 	if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
/*N*/ 	if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 	aPt-=GetSnapRect().Center();
/*N*/ 	SdrGluePoint aGP(aPt);
/*N*/ 	aGP.SetPercent(FALSE);
/*N*/ 	return aGP;
/*N*/ }



/*N*/ void SdrRectObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
/*N*/ 	SetXPolyDirty(); // wg. Eckenradius
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
/*N*/ void SdrRectObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrTextObj::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	SetXPolyDirty();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrRectObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ 	SetXPolyDirty(); // wg. Eckenradius
/*N*/ }

/*N*/ SdrObjGeoData* SdrRectObj::NewGeoData() const
/*N*/ { // etwas umstaendlicher, damit's vielleicht unter Chicago durchgeht
/*N*/ 	SdrObjGeoData* pGeo=new SdrRectObjGeoData;
/*N*/ 	return pGeo;
/*N*/ }

/*N*/ void SdrRectObj::SaveGeoData(SdrObjGeoData& rGeo) const
/*N*/ {
/*N*/ 	SdrTextObj::SaveGeoData(rGeo);
/*N*/ 	SdrRectObjGeoData& rRGeo=(SdrRectObjGeoData&)rGeo;
/*N*/ 	rRGeo.nEckRad=GetEckenradius();
/*N*/ }

/*?*/ void SdrRectObj::RestGeoData(const SdrObjGeoData& rGeo)
/*?*/ { // RectsDirty wird von SdrObject gerufen
/*?*/ 	SdrTextObj::RestGeoData(rGeo);
/*?*/ 	SdrRectObjGeoData& rRGeo=(SdrRectObjGeoData&)rGeo;
/*?*/ 	long nAltRad=GetEckenradius();
/*?*/ 	if (rRGeo.nEckRad!=nAltRad) NbcSetEckenradius(rRGeo.nEckRad);
/*?*/ 	SetXPolyDirty();
/*?*/ }

/*N*/ void SdrRectObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrTextObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrRectObj");
/*N*/ #endif
/*N*/ }

/*N*/ void SdrRectObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return;
/*N*/ 	SdrTextObj::ReadData(rHead,rIn);
/*N*/ 	if (IsTextFrame() && rHead.GetVersion()<3 && !HAS_BASE(SdrCaptionObj,this)) {
/*N*/ 		// Bis einschl. Version 2 wurden Textrahmen mit SdrTextObj dargestellt, ausser CaptionObj
/*?*/ 		SfxItemPool* pPool=GetItemPool();
/*?*/ 		if (pPool!=NULL) {
/*?*/ 			// Umrandung und Hintergrund des importierten Textrahmens ausschalten
/*?*/ 			SfxItemSet aSet(*pPool);
/*?*/ 			aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
/*?*/ 			aSet.Put(XFillStyleItem(XFILL_NONE));
/*?*/ 			aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // Falls einer auf Solid umschaltet
/*?*/ 			aSet.Put(XLineStyleItem(XLINE_NONE));
/*?*/ 
/*?*/ 			SetItemSet(aSet);
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aCompat.SetID("SdrRectObj");
/*N*/ #endif
/*N*/ 		if (rHead.GetVersion()<=5) {
/*N*/ 			long nEckRad;
/*?*/ 			rIn>>nEckRad;
/*?*/ 			long nAltRad=GetEckenradius();
/*?*/ 			if (nEckRad!=nAltRad) NbcSetEckenradius(nEckRad);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetXPolyDirty();
/*N*/ }

}
