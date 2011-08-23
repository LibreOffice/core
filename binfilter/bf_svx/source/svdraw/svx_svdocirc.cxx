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

#include <bf_svtools/style.hxx>


#include <xlnwtit.hxx>

#include <xlnedwit.hxx>

#include <xlnstwit.hxx>

#include <xlnstit.hxx>

#include <xlnedit.hxx>

#include "svdocirc.hxx"
#include <math.h>
#include "svdpool.hxx"
#include "svdattrx.hxx"
#include "svdio.hxx"
#include "svdstr.hrc"    // Objektname

#include "eeitem.hxx"

#include "rectenum.hxx"

#include "svdoimp.hxx"

#include "xoutx.hxx"

namespace binfilter {

/*N*/ void SetWinkPnt(const Rectangle& rR, long nWink, Point& rPnt)
/*N*/ {
/*N*/ 	Point aCenter(rR.Center());
/*N*/ 	long nWdt=rR.Right()-rR.Left();
/*N*/ 	long nHgt=rR.Bottom()-rR.Top();
/*N*/ 	long nMaxRad=((nWdt>nHgt ? nWdt : nHgt)+1) /2;
/*N*/ 	double a;
/*N*/ 	a=nWink*nPi180;
/*N*/ 	rPnt=Point(Round(cos(a)*nMaxRad),-Round(sin(a)*nMaxRad));
/*N*/ 	if (nWdt==0) rPnt.X()=0;
/*N*/ 	if (nHgt==0) rPnt.Y()=0;
/*N*/ 	if (nWdt!=nHgt) {
/*N*/ 		if (nWdt>nHgt) {
/*N*/ 			if (nWdt!=0) {
/*N*/ 				// eventuelle Ueberlaeufe bei sehr grossen Objekten abfangen (Bug 23384)
/*N*/ 				if (Abs(nHgt)>32767 || Abs(rPnt.Y())>32767) {
/*N*/ 					rPnt.Y()=BigMulDiv(rPnt.Y(),nHgt,nWdt);
/*N*/ 				} else {
/*N*/ 					rPnt.Y()=rPnt.Y()*nHgt/nWdt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		} else {
/*N*/ 			if (nHgt!=0) {
/*N*/ 				// eventuelle Ueberlaeufe bei sehr grossen Objekten abfangen (Bug 23384)
/*N*/ 				if (Abs(nWdt)>32767 || Abs(rPnt.X())>32767) {
/*N*/ 					rPnt.X()=BigMulDiv(rPnt.X(),nWdt,nHgt);
/*N*/ 				} else {
/*N*/ 					rPnt.X()=rPnt.X()*nWdt/nHgt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rPnt+=aCenter;
/*N*/ }

/*N*/ TYPEINIT1(SdrCircObj,SdrRectObj);

/*N*/ SdrCircObj::SdrCircObj(SdrObjKind eNewKind)
/*N*/ {
/*N*/ 	nStartWink=0;
/*N*/ 	nEndWink=36000;
/*N*/ 	eKind=eNewKind;
/*N*/ 	bClosedObj=eNewKind!=OBJ_CARC;
/*N*/ }

/*N*/ SdrCircObj::SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect):
/*N*/ 	SdrRectObj(rRect)
/*N*/ {
/*N*/ 	nStartWink=0;
/*N*/ 	nEndWink=36000;
/*N*/ 	eKind=eNewKind;
/*N*/ 	bClosedObj=eNewKind!=OBJ_CARC;
/*N*/ }

/*N*/ SdrCircObj::SdrCircObj(SdrObjKind eNewKind, const Rectangle& rRect, long nNewStartWink, long nNewEndWink):
/*N*/ 	SdrRectObj(rRect)
/*N*/ {
/*N*/ 	long nWinkDif=nNewEndWink-nNewStartWink;
/*N*/ 	nStartWink=NormAngle360(nNewStartWink);
/*N*/ 	nEndWink=NormAngle360(nNewEndWink);
/*N*/ 	if (nWinkDif==36000) nEndWink+=nWinkDif; // Vollkreis
/*N*/ 	eKind=eNewKind;
/*N*/ 	bClosedObj=eNewKind!=OBJ_CARC;
/*N*/ }
/*N*/ 
/*N*/ SdrCircObj::~SdrCircObj()
/*N*/ {
/*N*/ }


/*N*/ UINT16 SdrCircObj::GetObjIdentifier() const
/*N*/ {
/*N*/ 	return UINT16(eKind);
/*N*/ }

/*N*/ FASTBOOL SdrCircObj::PaintNeedsXPoly() const
/*N*/ {
/*N*/ 	// XPoly ist notwendig fuer alle gedrehten Ellipsenobjekte,
/*N*/ 	// fuer alle Kreis- und Ellipsenabschnitte
/*N*/ 	// und wenn nicht WIN dann (erstmal) auch fuer Kreis-/Ellipsenausschnitte
/*N*/ 	// und Kreis-/Ellipsenboegen (wg. Genauigkeit)
/*N*/ 	FASTBOOL bNeed=aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || eKind==OBJ_CCUT;
/*N*/ #ifndef WIN
/*N*/ 	// Wenn nicht Win, dann fuer alle ausser Vollkreis (erstmal!!!)
/*N*/ 	if (eKind!=OBJ_CIRC) bNeed=TRUE;
/*N*/ #endif
/*N*/ 
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	if(!bNeed)
/*N*/ 	{
/*N*/ 		// XPoly ist notwendig fuer alles was nicht LineSolid oder LineNone ist
/*N*/ 		XLineStyle eLine = ((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
/*N*/ 		bNeed = eLine != XLINE_NONE && eLine != XLINE_SOLID;
/*N*/ 
/*N*/ 		// XPoly ist notwendig fuer dicke Linien
/*N*/ 		if(!bNeed && eLine != XLINE_NONE)
/*N*/ 			bNeed = ((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue() != 0;
/*N*/ 
/*N*/ 		// XPoly ist notwendig fuer Kreisboegen mit Linienenden
/*N*/ 		if(!bNeed && eKind == OBJ_CARC)
/*N*/ 		{
/*N*/ 			// Linienanfang ist da, wenn StartPolygon und StartWidth!=0
/*?*/ 			bNeed=((XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue().GetPointCount() != 0 &&
/*?*/ 				  ((XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue() != 0;
/*?*/ 
/*?*/ 			if(!bNeed)
/*?*/ 			{
/*?*/ 				// Linienende ist da, wenn EndPolygon und EndWidth!=0
/*?*/ 				bNeed = ((XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue().GetPointCount() != 0 &&
/*?*/ 						((XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue() != 0;
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// XPoly ist notwendig, wenn Fill !=None und !=Solid
/*N*/ 	if(!bNeed && eKind != OBJ_CARC)
/*N*/ 	{
/*N*/ 		XFillStyle eFill=((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();
/*N*/ 		bNeed = eFill != XFILL_NONE && eFill != XFILL_SOLID;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(!bNeed && eKind != OBJ_CIRC && nStartWink == nEndWink)
/*N*/ 		bNeed=TRUE; // Weil sonst Vollkreis gemalt wird
/*N*/ 
/*N*/ 	return bNeed;
/*N*/ }

/*N*/ XPolygon SdrCircObj::ImpCalcXPoly(const Rectangle& rRect1, long nStart, long nEnd, FASTBOOL bContour) const
/*N*/ {
/*N*/ 	bContour=TRUE; // am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
/*N*/ 	long rx=rRect1.GetWidth()/2;  // Da GetWidth()/GetHeight() jeweils 1
/*N*/ 	long ry=rRect1.GetHeight()/2; // draufaddieren wird korrekt gerundet.
/*N*/ 	long a=0,e=3600;
/*N*/ 	if (eKind!=OBJ_CIRC) {
/*N*/ 		a=nStart/10;
/*N*/ 		e=nEnd/10;
/*N*/ 		if (bContour) {
/*N*/ 			// Drehrichtung umkehren, damit Richtungssinn genauso wie Rechteck
/*N*/ 			rx=-rx;
/*N*/ 			a=1800-a; if (a<0) a+=3600;
/*N*/ 			e=1800-e; if (e<0) e+=3600;
/*N*/ 			long nTmp=a;
/*N*/ 			a=e;
/*N*/ 			e=nTmp;
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		if (bContour) {
/*N*/ 			long nTmp=rx;
/*N*/ 			rx=ry;
/*N*/ 			ry=nTmp;
/*N*/ 			// und auch noch die Drehrichtung aendern
/*N*/ 			ry=-ry;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	((SdrCircObj*)this)->bXPolyIsLine=eKind==OBJ_CARC;
/*N*/ 	FASTBOOL bClose=eKind==OBJ_CIRC /*|| eKind==OBJ_SECT*/;
/*N*/ 	XPolygon aXPoly(rRect1.Center(),rx,ry,USHORT(a),USHORT(e),bClose);
/*N*/ 	if (eKind!=OBJ_CIRC && nStart==nEnd) {
/*?*/ 		if (eKind==OBJ_SECT) {
/*?*/ 			Point aMerk(aXPoly[0]);
/*?*/ 			aXPoly=XPolygon(2);
/*?*/ 			aXPoly[0]=rRect1.Center();
/*?*/ 			aXPoly[1]=aMerk;
/*?*/ 		} else {
/*?*/ 			aXPoly=XPolygon();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if (eKind==OBJ_SECT) { // Der Sektor soll Start/Ende im Zentrum haben
/*N*/ 		// Polygon um einen Punkt rotieren (Punkte im Array verschieben)
/*N*/ 		unsigned nPointAnz=aXPoly.GetPointCount();
/*N*/ 		aXPoly.Insert(0,rRect1.Center(),XPOLY_NORMAL);
/*N*/ 		aXPoly[aXPoly.GetPointCount()]=rRect1.Center();
/*N*/ 	}
/*N*/ 	// Der Kreis soll Anfang und Ende im unteren Scheitelpunkt haben!
/*N*/ 	if (bContour && eKind==OBJ_CIRC) RotateXPoly(aXPoly,rRect1.Center(),-1.0,0.0);
/*N*/ 	// Die Winkelangaben beziehen sich immer auf die linke obere Ecke von !aRect!
/*N*/ 	if (aGeo.nShearWink!=0) ShearXPoly(aXPoly,aRect.TopLeft(),aGeo.nTan);
/*N*/ 	if (aGeo.nDrehWink!=0) RotateXPoly(aXPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
/*N*/ 	return aXPoly;
/*N*/ }

/*N*/ void SdrCircObj::RecalcXPoly()
/*N*/ {
/*N*/ 	pXPoly=new XPolygon(ImpCalcXPoly(aRect,nStartWink,nEndWink));
/*N*/ }

/*N*/ void SdrCircObj::RecalcBoundRect()
/*N*/ {
/*N*/ 	SetWinkPnt(aRect,nStartWink,aPnt1);
/*N*/ 	SetWinkPnt(aRect,nEndWink,aPnt2);
/*N*/ 	bBoundRectDirty=FALSE;
/*N*/ 	aOutRect=GetSnapRect();
/*N*/ 	long nLineWdt=ImpGetLineWdt();
/*N*/ 	nLineWdt++; nLineWdt/=2;
/*N*/ 	if (nLineWdt!=0) {
/*N*/ 		long nWink=nEndWink-nStartWink;
/*N*/ 		if (nWink<0) nWink+=36000;
/*N*/ 		if (eKind==OBJ_SECT && nWink<18000) nLineWdt*=2; // doppelt, wegen evtl. spitzen Ecken
/*N*/ 		if (eKind==OBJ_CCUT && nWink<18000) nLineWdt*=2; // doppelt, wegen evtl. spitzen Ecken
/*N*/ 	}
/*N*/ 	if (eKind==OBJ_CARC) { // ggf. Linienenden beruecksichtigen
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

/*N*/ FASTBOOL SdrCircObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
/*N*/ {
/*N*/ 	// Hidden objects on masterpages, draw nothing
/*N*/ 	if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	BOOL bHideContour(IsHideContour());
/*N*/ 	BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));
/*N*/ 	BOOL bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));
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
/*N*/ 	if(!bHideContour && ImpSetShadowAttributes(rSet, aShadowSet))
/*N*/ 	{
/*?*/         if( eKind==OBJ_CARC || bIsFillDraft )
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
/*?*/         if (PaintNeedsXPoly()) 
/*?*/         {
/*?*/             if( !bXPolyIsLine ) 
/*?*/             {
/*?*/                 XPolygon aX(GetXPoly()); // In dieser Reihenfolge, damit bXPolyIsLine gueltig ist.
/*?*/                 aX.Move(nXDist,nYDist);
/*?*/                 
/*?*/                 // #100127# Output original geometry for metafiles
/*?*/                 ImpGraphicFill aFill( *this, rXOut, aShadowSet, true );
/*?*/                 
/*?*/                 rXOut.DrawXPolygon(aX);
/*?*/             }
/*?*/         } else {
/*?*/             // #100127# Output original geometry for metafiles
/*?*/             ImpGraphicFill aFill( *this, rXOut, aShadowSet, true );
/*?*/             
/*?*/             Rectangle aR(aRect);
/*?*/             aR.Move(nXDist,nYDist);
/*?*/             if (eKind==OBJ_CIRC) {
/*?*/                 rXOut.DrawEllipse(aR);
/*?*/             } else {
/*?*/                 GetBoundRect(); // fuer aPnt1,aPnt2
/*?*/                 Point aTmpPt1(aPnt1);
/*?*/                 Point aTmpPt2(aPnt2);
/*?*/                 aTmpPt1.X()+=nXDist;
/*?*/                 aTmpPt1.Y()+=nYDist;
/*?*/                 aTmpPt2.X()+=nXDist;
/*?*/                 aTmpPt2.Y()+=nYDist;
/*?*/                 switch (eKind) {
/*?*/                     case OBJ_SECT: rXOut.DrawPie(aR,aTmpPt1,aTmpPt2); break;
/*?*/                     case OBJ_CARC: rXOut.DrawArc(aR,aTmpPt1,aTmpPt2); break;
/*?*/                     case OBJ_CCUT: DBG_ERROR("SdrCircObj::Paint(): ein Kreisabschnitt muss immer mit XPoly gepaintet werden"); break;
/*?*/                 }
/*?*/             }
/*?*/         }
/*?*/ 
/*?*/ 		// new shadow line drawing
/*?*/ 		if( pLineGeometry.get() )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ 			// draw the line geometry
/*?*/ 		}
/*N*/ 	}

    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
/*N*/ 	rXOut.SetLineAttr(aEmptySet);
/*N*/ 
/*N*/     rXOut.SetFillAttr( bIsFillDraft ? aEmptySet : rSet );
/*N*/ 
/*N*/ 	if (!bHideContour) {
/*N*/ 		if (PaintNeedsXPoly()) 
/*N*/         {
/*?*/ 			if( !bXPolyIsLine ) 
/*?*/             {
/*?*/                 const XPolygon& rXP=GetXPoly(); // In dieser Reihenfolge, damit bXPolyIsLine gueltig ist.
/*?*/ 
/*?*/                 // #100127# Output original geometry for metafiles
/*?*/                 ImpGraphicFill aFill( *this, rXOut, bIsFillDraft ? aEmptySet : rSet );
/*?*/ 
/*?*/ 				rXOut.DrawXPolygon(rXP);
/*N*/ 			}
/*N*/ 		} else {
/*N*/             // #100127# Output original geometry for metafiles
/*N*/             ImpGraphicFill aFill( *this, rXOut, bIsFillDraft ? aEmptySet : rSet );
/*N*/ 
/*N*/ 			if (eKind==OBJ_CIRC) {
/*N*/ 				rXOut.DrawEllipse(aRect);
/*N*/ 			} else {
/*?*/ 				GetBoundRect(); // fuer aPnt1,aPnt2
/*?*/ 				switch (eKind) {
/*?*/ 					case OBJ_SECT: rXOut.DrawPie(aRect,aPnt1,aPnt2); break;
/*?*/ 					case OBJ_CARC: rXOut.DrawArc(aRect,aPnt1,aPnt2); break;
/*?*/ 					case OBJ_CCUT: DBG_ERROR("SdrCircObj::Paint(): ein Kreisabschnitt muss immer mit XPoly gepaintet werden"); break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}

    // Own line drawing
/*N*/ 	if(!bHideContour && pLineGeometry.get() )
/*N*/ 	{
/*N*/ 		// draw the line geometry
/*N*/ 		ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
/*N*/ 	}
/*N*/ 
/*N*/ 	FASTBOOL bOk=TRUE;
/*N*/ 	if (HasText()) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 	return bOk;
/*N*/ }




/*N*/ void SdrCircObj::operator=(const SdrObject& rObj)
/*N*/ {
/*N*/ 	SdrRectObj::operator=(rObj);
/*N*/ 
/*N*/ 	nStartWink = ((SdrCircObj&)rObj).nStartWink;
/*N*/ 	nEndWink = ((SdrCircObj&)rObj).nEndWink;
/*N*/ 	aPnt1 = ((SdrCircObj&)rObj).aPnt1;
/*N*/ 	aPnt2 = ((SdrCircObj&)rObj).aPnt2;
/*N*/ }

/*N*/ void SdrCircObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
/*N*/ {
/*N*/ 	XPolygon aP(ImpCalcXPoly(aRect,nStartWink,nEndWink));
/*N*/ 	if (!bXPolyIsLine) { // Polygon schliessen
/*N*/ 		USHORT n=aP.GetPointCount();
/*N*/ 		Point aPnt(aP[0]);
/*N*/ 		aP[n]=aPnt;
/*N*/ 	}
/*N*/ 	rPoly=XPolyPolygon(aP);
/*N*/ }

/*N*/ void SdrCircObj::TakeContour(XPolyPolygon& rPoly) const
/*N*/ {
/*N*/ 	// am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
/*N*/ 	SdrRectObj::TakeContour(rPoly);
/*N*/ }





















/*N*/ void SdrCircObj::NbcMove(const Size& aSiz)
/*N*/ {
/*N*/ 	MoveRect(aRect,aSiz);
/*N*/ 	MoveRect(aOutRect,aSiz);
/*N*/ 	MoveRect(maSnapRect,aSiz);
/*N*/ 	MovePoint(aPnt1,aSiz);
/*N*/ 	MovePoint(aPnt2,aSiz);
/*N*/ 	SetXPolyDirty();
/*N*/ 	SetRectsDirty(TRUE);
/*N*/ }

/*N*/ void SdrCircObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	long nWink0=aGeo.nDrehWink;
/*N*/ 	FASTBOOL bNoShearRota=(aGeo.nDrehWink==0 && aGeo.nShearWink==0);
/*N*/ 	SdrTextObj::NbcResize(rRef,xFact,yFact);
/*N*/ 	bNoShearRota|=(aGeo.nDrehWink==0 && aGeo.nShearWink==0);
/*N*/ 	if (eKind!=OBJ_CIRC) {
/*N*/ 		FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
/*N*/ 		FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
/*N*/ 		if (bXMirr || bYMirr) {
/*N*/ 			// bei bXMirr!=bYMirr muessten eigentlich noch die beiden
/*N*/ 			// Linienende vertauscht werden. Das ist jedoch mal wieder
/*N*/ 			// schlecht (wg. zwangslaeufiger harter Formatierung).
/*N*/ 			// Alternativ koennte ein bMirrored-Flag eingefuehrt werden
/*N*/ 			// (Vielleicht ja mal grundsaetzlich, auch fuer gepiegelten Text, ...).
/*N*/ 			long nS0=nStartWink;
/*N*/ 			long nE0=nEndWink;
/*N*/ 			if (bNoShearRota) {
/*N*/ 				// Das RectObj spiegelt bei VMirror bereits durch durch 180deg Drehung.
/*N*/ 				if (! (bXMirr && bYMirr)) {
/*N*/ 					long nTmp=nS0;
/*N*/ 					nS0=18000-nE0;
/*N*/ 					nE0=18000-nTmp;
/*N*/ 				}
/*N*/ 			} else { // Spiegeln fuer verzerrte Ellipsen
/*N*/ 				if (bXMirr!=bYMirr) {
/*N*/ 					nS0+=nWink0;
/*N*/ 					nE0+=nWink0;
/*N*/ 					if (bXMirr) {
/*N*/ 						long nTmp=nS0;
/*N*/ 						nS0=18000-nE0;
/*N*/ 						nE0=18000-nTmp;
/*N*/ 					}
/*N*/ 					if (bYMirr) {
/*N*/ 						long nTmp=nS0;
/*N*/ 						nS0=-nE0;
/*N*/ 						nE0=-nTmp;
/*N*/ 					}
/*N*/ 					nS0-=aGeo.nDrehWink;
/*N*/ 					nE0-=aGeo.nDrehWink;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			long nWinkDif=nE0-nS0;
/*N*/ 			nStartWink=NormAngle360(nS0);
/*N*/ 			nEndWink  =NormAngle360(nE0);
/*N*/ 			if (nWinkDif==36000) nEndWink+=nWinkDif; // Vollkreis
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetXPolyDirty();
/*N*/ 	ImpSetCircInfoToAttr();
/*N*/ }

/*N*/ void SdrCircObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
/*N*/ 	SetXPolyDirty();
/*N*/ 	ImpSetCircInfoToAttr();
/*N*/ }





/*N*/ void Union(Rectangle& rR, const Point& rP)
/*N*/ {
/*N*/ 	if (rP.X()<rR.Left  ()) rR.Left  ()=rP.X();
/*N*/ 	if (rP.X()>rR.Right ()) rR.Right ()=rP.X();
/*N*/ 	if (rP.Y()<rR.Top   ()) rR.Top   ()=rP.Y();
/*N*/ 	if (rP.Y()>rR.Bottom()) rR.Bottom()=rP.Y();
/*N*/ }

/*N*/ void SdrCircObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
/*N*/ {
/*N*/ 	rRect=aRect;
/*N*/ 	if (eKind!=OBJ_CIRC) {
/*N*/ 		SetWinkPnt(rRect,nStartWink,((SdrCircObj*)(this))->aPnt1);
/*N*/ 		SetWinkPnt(rRect,nEndWink  ,((SdrCircObj*)(this))->aPnt2);
/*N*/ 		long a=nStartWink;
/*N*/ 		long e=nEndWink;
/*N*/ 		rRect.Left  ()=aRect.Right();
/*N*/ 		rRect.Right ()=aRect.Left();
/*N*/ 		rRect.Top   ()=aRect.Bottom();
/*N*/ 		rRect.Bottom()=aRect.Top();
/*N*/ 		Union(rRect,aPnt1);
/*N*/ 		Union(rRect,aPnt2);
/*N*/ 		if ((a<=18000 && e>=18000) || (a>e && (a<=18000 || e>=18000))) {
/*N*/ 			Union(rRect,aRect.LeftCenter());
/*N*/ 		}
/*N*/ 		if ((a<=27000 && e>=27000) || (a>e && (a<=27000 || e>=27000))) {
/*N*/ 			Union(rRect,aRect.BottomCenter());
/*N*/ 		}
/*N*/ 		if (a>e) {
/*N*/ 			Union(rRect,aRect.RightCenter());
/*N*/ 		}
/*N*/ 		if ((a<=9000 && e>=9000) || (a>e && (a<=9000 || e>=9000))) {
/*N*/ 			Union(rRect,aRect.TopCenter());
/*N*/ 		}
/*N*/ 		if (eKind==OBJ_SECT) {
/*N*/ 			Union(rRect,aRect.Center());
/*N*/ 		}
/*N*/ 		if (aGeo.nDrehWink!=0) {
/*?*/ 			Point aDst(rRect.TopLeft());
/*?*/ 			aDst-=aRect.TopLeft();
/*?*/ 			Point aDst0(aDst);
/*?*/ 			RotatePoint(aDst,Point(),aGeo.nSin,aGeo.nCos);
/*?*/ 			aDst-=aDst0;
/*?*/ 			rRect.Move(aDst.X(),aDst.Y());
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (aGeo.nShearWink!=0) {
/*N*/ 		long nDst=Round((rRect.Bottom()-rRect.Top())*aGeo.nTan);
/*N*/ 		if (aGeo.nShearWink>0) {
/*?*/ 			Point aRef(rRect.TopLeft());
/*?*/ 			rRect.Left()-=nDst;
/*?*/ 			Point aTmpPt(rRect.TopLeft());
/*?*/ 			RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
/*?*/ 			aTmpPt-=rRect.TopLeft();
/*?*/ 			rRect.Move(aTmpPt.X(),aTmpPt.Y());
/*?*/ 		} else {
/*?*/ 			rRect.Right()-=nDst;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrCircObj::RecalcSnapRect()
/*N*/ {
/*N*/ 	if (PaintNeedsXPoly()) {
/*N*/ 		maSnapRect=GetXPoly().GetBoundRect();
/*N*/ 	} else {
/*N*/ 		TakeUnrotatedSnapRect(maSnapRect);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrCircObj::NbcSetSnapRect(const Rectangle& rRect)
/*N*/ {
/*N*/ 	if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || eKind!=OBJ_CIRC) {
/*?*/ 		Rectangle aSR0(GetSnapRect());
/*?*/ 		long nWdt0=aSR0.Right()-aSR0.Left();
/*?*/ 		long nHgt0=aSR0.Bottom()-aSR0.Top();
/*?*/ 		long nWdt1=rRect.Right()-rRect.Left();
/*?*/ 		long nHgt1=rRect.Bottom()-rRect.Top();
/*?*/ 		NbcResize(maSnapRect.TopLeft(),Fraction(nWdt1,nWdt0),Fraction(nHgt1,nHgt0));
/*?*/ 		NbcMove(Size(rRect.Left()-aSR0.Left(),rRect.Top()-aSR0.Top()));
/*N*/ 	} else {
/*N*/ 		aRect=rRect;
/*N*/ 		ImpJustifyRect(aRect);
/*N*/ 	}
/*N*/ 	SetRectsDirty();
/*N*/ 	SetXPolyDirty();
/*N*/ 	ImpSetCircInfoToAttr();
/*N*/ }



/*N*/ void __EXPORT SdrCircObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	SetXPolyDirty();
/*N*/ 	SdrRectObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
/*N*/ 	ImpSetAttrToCircInfo();
/*N*/ }

/*N*/ void SdrCircObj::ForceDefaultAttr()
/*N*/ {
/*N*/ 	SdrCircKind eKindA = SDRCIRC_FULL;
/*N*/ 
/*N*/ 	if(eKind == OBJ_SECT)
/*N*/ 		eKindA = SDRCIRC_SECT;
/*N*/ 	else if(eKind == OBJ_CARC)
/*N*/ 		eKindA = SDRCIRC_ARC;
/*N*/ 	else if(eKind == OBJ_CCUT)
/*N*/ 		eKindA = SDRCIRC_CUT;
/*N*/ 
/*N*/ 	if(eKindA != SDRCIRC_FULL)
/*N*/ 	{
/*N*/ 		ImpForceItemSet();
/*N*/ 		mpObjectItemSet->Put(SdrCircKindItem(eKindA));
/*N*/ 
/*N*/ 		if(nStartWink)
/*N*/ 			mpObjectItemSet->Put(SdrCircStartAngleItem(nStartWink));
/*N*/ 
/*N*/ 		if(nEndWink != 36000)
/*N*/ 			mpObjectItemSet->Put(SdrCircEndAngleItem(nEndWink));
/*N*/ 	}
/*N*/ 
/*N*/ 	// call parent, after SetItem(SdrCircKindItem())
/*N*/ 	// because ForceDefaultAttr() will call
/*N*/ 	// ImpSetAttrToCircInfo() which needs a correct
/*N*/ 	// SdrCircKindItem
/*N*/ 	SdrRectObj::ForceDefaultAttr();
/*N*/ 
/*N*/ }

/*N*/ void SdrCircObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
/*N*/ {
/*N*/ 	SetXPolyDirty();
/*N*/ 	SdrRectObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
/*N*/ 	ImpSetAttrToCircInfo();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

/*N*/ SfxItemSet* SdrCircObj::CreateNewItemSet(SfxItemPool& rPool)
/*N*/ {
/*N*/ 	// include ALL items, 2D and 3D
/*N*/ 	return new SfxItemSet(rPool,
/*N*/ 		// ranges from SdrAttrObj
/*N*/ 		SDRATTR_START, SDRATTRSET_SHADOW,
/*N*/ 		SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
/*N*/ 		SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,
/*N*/ 
/*N*/ 		// circle attributes
/*N*/ 		SDRATTR_CIRC_FIRST, SDRATTRSET_CIRC,
/*N*/ 
/*N*/ 		// outliner and end
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END,
/*N*/ 		0, 0);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
/*N*/ void SdrCircObj::ItemSetChanged(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	// local changes
/*N*/ 	SetXPolyDirty();
/*N*/ 
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::ItemSetChanged(rSet);
/*N*/ 
/*N*/ 	// local changes
/*N*/ 	ImpSetAttrToCircInfo();
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrCircObj::ImpSetAttrToCircInfo()
/*N*/ {
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 	SdrCircKind eNewKindA = ((SdrCircKindItem&)rSet.Get(SDRATTR_CIRCKIND)).GetValue();
/*N*/ 	SdrObjKind eNewKind = eKind;
/*N*/ 
/*N*/ 	if(eNewKindA == SDRCIRC_FULL)
/*N*/ 		eNewKind = OBJ_CIRC;
/*N*/ 	else if(eNewKindA == SDRCIRC_SECT)
/*N*/ 		eNewKind = OBJ_SECT;
/*N*/ 	else if(eNewKindA == SDRCIRC_ARC)
/*N*/ 		eNewKind = OBJ_CARC;
/*N*/ 	else if(eNewKindA == SDRCIRC_CUT)
/*N*/ 		eNewKind = OBJ_CCUT;
/*N*/ 
/*N*/ 	sal_Int32 nNewStart = ((SdrCircStartAngleItem&)rSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue();
/*N*/ 	sal_Int32 nNewEnd = ((SdrCircEndAngleItem&)rSet.Get(SDRATTR_CIRCENDANGLE)).GetValue();
/*N*/ 
/*N*/ 	BOOL bKindChg = eKind != eNewKind;
/*N*/ 	BOOL bWinkChg = nNewStart != nStartWink || nNewEnd != nEndWink;
/*N*/ 
/*N*/ 	if(bKindChg || bWinkChg)
/*N*/ 	{
/*N*/ 		eKind = eNewKind;
/*N*/ 		nStartWink = nNewStart;
/*N*/ 		nEndWink = nNewEnd;
/*N*/ 
/*N*/ 		if(bKindChg || (eKind != OBJ_CIRC && bWinkChg))
/*N*/ 		{
/*N*/ 			SetXPolyDirty();
/*N*/ 			SetRectsDirty();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrCircObj::ImpSetCircInfoToAttr()
/*N*/ {
/*N*/ 	SdrCircKind eNewKindA = SDRCIRC_FULL;
/*N*/ 	const SfxItemSet& rSet = GetItemSet();
/*N*/ 
/*N*/ 	if(eKind == OBJ_SECT)
/*N*/ 		eNewKindA = SDRCIRC_SECT;
/*N*/ 	else if(eKind == OBJ_CARC)
/*N*/ 		eNewKindA = SDRCIRC_ARC;
/*N*/ 	else if(eKind == OBJ_CCUT)
/*N*/ 		eNewKindA = SDRCIRC_CUT;
/*N*/ 
/*N*/ 	SdrCircKind eOldKindA = ((SdrCircKindItem&)rSet.Get(SDRATTR_CIRCKIND)).GetValue();
/*N*/ 	sal_Int32 nOldStartWink = ((SdrCircStartAngleItem&)rSet.Get(SDRATTR_CIRCSTARTANGLE)).GetValue();
/*N*/ 	sal_Int32 nOldEndWink = ((SdrCircEndAngleItem&)rSet.Get(SDRATTR_CIRCENDANGLE)).GetValue();
/*N*/ 
/*N*/ 	if(eNewKindA != eOldKindA || nStartWink != nOldStartWink || nEndWink != nOldEndWink)
/*N*/ 	{
/*N*/ 		// #81921# since SetItem() implicitly calls ImpSetAttrToCircInfo()
/*N*/ 		// setting the item directly is necessary here.
/*?*/ 		ImpForceItemSet();
/*?*/ 
/*?*/ 		if(eNewKindA != eOldKindA)
/*?*/ 			mpObjectItemSet->Put(SdrCircKindItem(eNewKindA));
/*?*/ 
/*?*/ 		if(nStartWink != nOldStartWink)
/*?*/ 			mpObjectItemSet->Put(SdrCircStartAngleItem(nStartWink));
/*?*/ 
/*?*/ 		if(nEndWink != nOldEndWink)
/*?*/ 			mpObjectItemSet->Put(SdrCircEndAngleItem(nEndWink));
/*?*/ 
/*?*/ 		SetXPolyDirty();
/*?*/ 		ImpSetAttrToCircInfo();
/*N*/ 	}
/*N*/ }

/*N*/ SdrObject* SdrCircObj::DoConvertToPolyObj(BOOL bBezier) const
/*N*/ {
/*N*/ 	XPolygon aXP(ImpCalcXPoly(aRect,nStartWink,nEndWink));
/*N*/ 	SdrObjKind ePathKind=OBJ_PATHFILL;
/*N*/ 	FASTBOOL bFill=TRUE;
/*N*/ 	if (eKind==OBJ_CARC) bFill=FALSE;
/*N*/ 	SdrObject* pRet=ImpConvertMakeObj(XPolyPolygon(aXP),bFill,bBezier);
/*N*/ 	pRet=ImpConvertAddText(pRet,bBezier);
/*N*/ 	return pRet;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

/*N*/ void SdrCircObj::PreSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::PreSave();
/*N*/ 
/*N*/ 	// prepare SetItems for storage
/*N*/ 	const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 	const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
/*N*/ 	SdrCircSetItem aCircAttr(rSet.GetPool());
/*N*/ 	aCircAttr.GetItemSet().Put(rSet);
/*N*/ 	aCircAttr.GetItemSet().SetParent(pParent);
/*N*/ 	mpObjectItemSet->Put(aCircAttr);
/*N*/ }

/*N*/ void SdrCircObj::PostSave()
/*N*/ {
/*N*/ 	// call parent
/*N*/ 	SdrRectObj::PostSave();
/*N*/ 
/*N*/ 	// remove SetItems from local itemset
/*N*/ 	mpObjectItemSet->ClearItem(SDRATTRSET_CIRC);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrCircObj::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	SdrRectObj::WriteData(rOut);
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrCircObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	if(eKind != OBJ_CIRC)
/*N*/ 	{
/*N*/ 		rOut << nStartWink;
/*N*/ 		rOut << nEndWink;
/*N*/ 	}
/*N*/ 
/*N*/ 	SfxItemPool* pPool=GetItemPool();
/*N*/ 	if(pPool)
/*N*/ 	{
/*N*/ 		const SfxItemSet& rSet = GetUnmergedItemSet();
/*N*/ 
/*N*/ 		pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_CIRC));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rOut << UINT16(SFX_ITEMS_NULL);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrCircObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if(rIn.GetError())
/*N*/ 		return;
/*N*/ 
/*N*/ 	// #91764# remember eKind, it will be deleted during SdrRectObj::ReadData(...)
/*N*/ 	// but needs to be known to decide to jump over angles or not. Deletion happens
/*N*/ 	// cause of fix #89025# wich is necessary, too.
/*N*/ 	SdrObjKind eRememberedKind = eKind;
/*N*/ 
/*N*/ 	SdrRectObj::ReadData(rHead,rIn);
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrCircObj");
/*N*/ #endif
/*N*/ 
/*N*/ 	// #92309# at once restore the remembered eKind here.
/*N*/ 	eKind = eRememberedKind;
/*N*/ 
/*N*/ 	// #91764# use remembered eKind here
/*N*/ 	if(eRememberedKind != OBJ_CIRC)
/*N*/ 	{
/*N*/ 		rIn >> nStartWink;
/*N*/ 		rIn >> nEndWink;
/*N*/ 	}
/*N*/ 
/*N*/ 	if(aCompat.GetBytesLeft() > 0)
/*N*/ 	{
/*N*/ 		SfxItemPool* pPool = GetItemPool();
/*N*/ 
/*N*/ 		if(pPool)
/*N*/ 		{
/*N*/ 			sal_uInt16 nSetID = SDRATTRSET_CIRC;
/*N*/ 			const SdrCircSetItem* pCircAttr = (const SdrCircSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
/*N*/ 			if(pCircAttr)
/*N*/ 				SetItemSet(pCircAttr->GetItemSet());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			sal_uInt16 nSuroDum;
/*N*/ 			rIn >> nSuroDum;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// create pCircAttr for old Objects to let ImpSetCircInfoToAttr() do it's work
/*N*/ 		SdrCircKind eKindA(SDRCIRC_FULL);
/*N*/ 
/*N*/ 		if(eKind == OBJ_SECT)
/*N*/ 			eKindA = SDRCIRC_SECT;
/*N*/ 		else if(eKind == OBJ_CARC)
/*N*/ 			eKindA = SDRCIRC_ARC;
/*N*/ 		else if(eKind == OBJ_CCUT)
/*N*/ 			eKindA = SDRCIRC_CUT;
/*N*/ 
/*N*/ 		if(eKindA != SDRCIRC_FULL)
/*N*/ 		{
/*N*/ 			mpObjectItemSet->Put(SdrCircKindItem(eKindA));
/*N*/ 
/*N*/ 			if(nStartWink)
/*N*/ 				mpObjectItemSet->Put(SdrCircStartAngleItem(nStartWink));
/*N*/ 
/*N*/ 			if(nEndWink != 36000)
/*N*/ 				mpObjectItemSet->Put(SdrCircEndAngleItem(nEndWink));
/*N*/ 		}
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
