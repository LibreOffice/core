/*************************************************************************
 *
 *  $RCSfile: svdorect.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-15 16:11:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdorect.hxx"
#include <math.h>
#include <stdlib.h>
#include "xpool.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdattr.hxx"
#include "svdpool.hxx"
#include "svdtouch.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svddrag.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdocapt.hxx" // fuer Import von SdrFileVersion 2
#include "svdpagv.hxx" // fuer
#include "svdview.hxx" // das
#include "svdundo.hxx" // Macro-Beispiel
#include "svdopath.hxx"
#include "svdglob.hxx"  // Stringcache
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

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrRectObj,SdrTextObj);

SdrRectObj::SdrRectObj():
    pXPoly(NULL),
    bXPolyIsLine(FALSE)
{
    bClosedObj=TRUE;
}

SdrRectObj::SdrRectObj(const Rectangle& rRect):
    SdrTextObj(rRect),
    pXPoly(NULL),
    bXPolyIsLine(FALSE)
{
    bClosedObj=TRUE;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind):
    SdrTextObj(eNewTextKind),
    pXPoly(NULL),
    bXPolyIsLine(FALSE)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind) ist nur fuer Textrahmen gedacht");
    bClosedObj=TRUE;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rRect):
    SdrTextObj(eNewTextKind,rRect),
    pXPoly(NULL),
    bXPolyIsLine(FALSE)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) ist nur fuer Textrahmen gedacht");
    bClosedObj=TRUE;
}

SdrRectObj::SdrRectObj(SdrObjKind eNewTextKind, const Rectangle& rNewRect, SvStream& rInput, USHORT eFormat):
    SdrTextObj(eNewTextKind,rNewRect,rInput,eFormat),
    pXPoly(NULL),
    bXPolyIsLine(FALSE)
{
    DBG_ASSERT(eTextKind==OBJ_TEXT || eTextKind==OBJ_TEXTEXT ||
               eTextKind==OBJ_OUTLINETEXT || eTextKind==OBJ_TITLETEXT,
               "SdrRectObj::SdrRectObj(SdrObjKind,...) ist nur fuer Textrahmen gedacht");
    bClosedObj=TRUE;
}

SdrRectObj::~SdrRectObj()
{
    if (pXPoly!=NULL) delete pXPoly;
}

void SdrRectObj::SetXPolyDirty()
{
    if (pXPoly!=NULL) {
        delete pXPoly;
        pXPoly=NULL;
    }
}

FASTBOOL SdrRectObj::PaintNeedsXPoly(long nEckRad) const
{
    FASTBOOL bNeed=aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || nEckRad!=0;
    return bNeed;
}

XPolygon SdrRectObj::ImpCalcXPoly(const Rectangle& rRect1, long nRad1, FASTBOOL bContour) const
{
    bContour=TRUE; // am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
    XPolygon aXPoly(rRect1,nRad1,nRad1);
    if (bContour) {
        unsigned nPointAnz=aXPoly.GetPointCount();
        XPolygon aNeuPoly(nPointAnz+1);
        unsigned nShift=nPointAnz-2;
        if (nRad1!=0) nShift=nPointAnz-5;
        unsigned j=nShift;
        for (unsigned i=1; i<nPointAnz; i++) {
            aNeuPoly[i]=aXPoly[j];
            aNeuPoly.SetFlags(i,aXPoly.GetFlags(j));
            j++;
            if (j>=nPointAnz) j=1;
        }
        aNeuPoly[0]=rRect1.BottomCenter();
        aNeuPoly[nPointAnz]=aNeuPoly[0];
        aXPoly=aNeuPoly;
    }
    // Die Winkelangaben beziehen sich immer auf die linke obere Ecke von !aRect!
    if (aGeo.nShearWink!=0) ShearXPoly(aXPoly,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotateXPoly(aXPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    return aXPoly;
}

void SdrRectObj::RecalcXPoly()
{
    pXPoly=new XPolygon(ImpCalcXPoly(aRect,GetEckenradius()));
}

const XPolygon& SdrRectObj::GetXPoly() const
{
    if (pXPoly==NULL) ((SdrRectObj*)this)->RecalcXPoly();
    return *pXPoly;
}

void SdrRectObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    FASTBOOL bNoTextFrame=!IsTextFrame();
    rInfo.bResizeFreeAllowed=bNoTextFrame || aGeo.nDrehWink%9000==0;
    rInfo.bResizePropAllowed=TRUE;
    rInfo.bRotateFreeAllowed=TRUE;
    rInfo.bRotate90Allowed  =TRUE;
    rInfo.bMirrorFreeAllowed=bNoTextFrame;
    rInfo.bMirror45Allowed  =bNoTextFrame;
    rInfo.bMirror90Allowed  =bNoTextFrame;

    // allow transparence
    rInfo.bTransparenceAllowed = TRUE;

    // gradient depends on fillstyle
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    rInfo.bShearAllowed     =bNoTextFrame;
    rInfo.bEdgeRadiusAllowed=TRUE;

    FASTBOOL bCanConv=!HasText() || ImpCanConvTextToCurve();
    if (bCanConv && !bNoTextFrame && !HasText()) {
        bCanConv=HasFill() || HasLine();
    }
    rInfo.bCanConvToPath    =bCanConv;
    rInfo.bCanConvToPoly    =bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

UINT16 SdrRectObj::GetObjIdentifier() const
{
    if (IsTextFrame()) return UINT16(eTextKind);
    else return UINT16(OBJ_RECT);
}

void SdrRectObj::RecalcBoundRect()
{
    aOutRect=GetSnapRect();
    long nLineWdt=ImpGetLineWdt();
    nLineWdt++; nLineWdt/=2;
    if (nLineWdt!=0) {
        long a=nLineWdt;
        if ((aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) && GetEckenradius()==0) {
            a*=2; // doppelt, wegen evtl. spitzen Ecken
        }
        aOutRect.Left  ()-=a;
        aOutRect.Top   ()-=a;
        aOutRect.Right ()+=a;
        aOutRect.Bottom()+=a;
    }
    ImpAddShadowToBoundRect();
    ImpAddTextToBoundRect();
}

void SdrRectObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect=aRect;
    if (aGeo.nShearWink!=0) {
        long nDst=Round((aRect.Bottom()-aRect.Top())*aGeo.nTan);
        if (aGeo.nShearWink>0) {
            Point aRef(rRect.TopLeft());
            rRect.Left()-=nDst;
            Point aTmpPt(rRect.TopLeft());
            RotatePoint(aTmpPt,aRef,aGeo.nSin,aGeo.nCos);
            aTmpPt-=rRect.TopLeft();
            rRect.Move(aTmpPt.X(),aTmpPt.Y());
        } else {
            rRect.Right()-=nDst;
        }
    }
}

FASTBOOL SdrRectObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    // Im Graustufenmodus soll die Hintergrundseite NICHT angezeigt werden
    ULONG nMode = rXOut.GetOutDev()->GetDrawMode();
    FASTBOOL bGrayscale =
       nMode == (DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL |
                 DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT);
    if( bGrayscale && pPage && pPage->IsMasterPage() )
    {
        Size aPageSize = pPage->GetSize();
        long aRectWidth = aRect.GetSize().Width() - 1;
        long aRectHeight = aRect.GetSize().Height() - 1;

        // Objekt so gross wie Seite ? -> Hintergrund
        if( aRectWidth == aPageSize.Width() &&
            aRectHeight == aPageSize.Height()  )
        {
            return TRUE;
        }
        // oder so gross wie Seite abzueglich der Raender
        if( aRectWidth == aPageSize.Width() -
                pPage->GetLftBorder() - pPage->GetRgtBorder() &&
            aRectHeight == aPageSize.Height() -
                pPage->GetUppBorder() - pPage->GetLwrBorder() )
        {
            return TRUE;
        }

    }

    if (bTextFrame && aGeo.nShearWink!=0) {
        DBG_WARNING("Shearwinkel vom TextFrame innerhalb von SdrRectObj::Paint() auf 0 gesetzt");
        ((SdrRectObj*)this)->ImpCheckShear();
        ((SdrRectObj*)this)->SetRectsDirty();
    }
    FASTBOOL bOk=TRUE;
    BOOL bHideContour(IsHideContour());
    sal_Int32 nEckRad(GetEckenradius());
    BOOL bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));
    BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));

    // prepare ItemSet of this object
    const SfxItemSet& rSet = GetItemSet();

    // perepare ItemSet to avoid old XOut line drawing
    SfxItemSet aEmptySet(*rSet.GetPool());
    aEmptySet.Put(XLineStyleItem(XLINE_NONE));
    aEmptySet.Put(XFillStyleItem(XFILL_NONE));

    // prepare line geometry
    ImpLineGeometry* pLineGeometry = ImpPrepareLineGeometry(rXOut, rSet, bIsLineDraft);

    // Shadows
    if (!bHideContour && ImpSetShadowAttributes(rXOut,FALSE))
    {
        UINT32 nXDist=((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        UINT32 nYDist=((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();

        // avoid shadow line drawing in XOut
        rXOut.SetLineAttr(aEmptySet);

        if (PaintNeedsXPoly(nEckRad)) {
            XPolygon aX(GetXPoly());
            aX.Move(nXDist,nYDist);
            rXOut.DrawXPolygon(aX);
        } else {
            Rectangle aR(aRect);
            aR.Move(nXDist,nYDist);
            rXOut.DrawRect(aR,USHORT(2*nEckRad),USHORT(2*nEckRad));
        }

        // new shadow line drawing
        if(pLineGeometry)
        {
            // draw the line geometry
            ImpDrawShadowLineGeometry(rXOut, rSet, *pLineGeometry);
        }
    }

    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
    rXOut.SetLineAttr(aEmptySet);

    if(bIsFillDraft)
    {
        // perepare ItemSet to avoid XOut filling
        rXOut.SetFillAttr(aEmptySet);
    }
    else
    {
        rXOut.SetFillAttr(rSet);
    }

    if (!bHideContour) {
        if (PaintNeedsXPoly(nEckRad)) {
            rXOut.DrawXPolygon(GetXPoly());
        } else {
            DBG_ASSERT(nEckRad==0,"SdrRectObj::Paint(): XOut.DrawRect() unterstuetz kein Eckenradius!");
            rXOut.DrawRect(aRect/*,USHORT(2*nEckRad),USHORT(2*nEckRad)*/);
        }
    }

    DBG_ASSERT(aRect.GetWidth()>1 && aRect.GetHeight()>1,"SdrRectObj::Paint(): Rect hat Nullgroesse (oder negativ)!");

    // Own line drawing
    if(!bHideContour && pLineGeometry)
    {
        // draw the line geometry
        ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
    }

    if (HasText()) {
        bOk=SdrTextObj::Paint(rXOut,rInfoRec);
    }
    if (bOk && (rInfoRec.nPaintMode & SDRPAINTMODE_GLUEPOINTS) !=0) {
        bOk=PaintGluePoints(rXOut,rInfoRec);
    }

    // throw away line geometry
    if(pLineGeometry)
        delete pLineGeometry;

    return bOk;
}

SdrObject* SdrRectObj::ImpCheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bForceFilled, FASTBOOL bForceTol) const
{
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    INT32 nMyTol=nTol;
    FASTBOOL bFilled=bForceFilled || HasFill();
    FASTBOOL bPickThrough=pModel!=NULL && pModel->IsPickThroughTransparentTextFrames();
    if (bTextFrame && !bPickThrough) bFilled=TRUE;
    FASTBOOL bLine=HasLine();

    INT32 nWdt=bLine ? ImpGetLineWdt()/2 :0; // Halbe Strichstaerke
    long nBoundWdt=aRect.GetWidth()-1;
    long nBoundHgt=aRect.GetHeight()-1;
    if (bFilled && nBoundWdt>short(nTol) && nBoundHgt>short(nTol) && Abs(aGeo.nShearWink)<=4500) {
        if (!bForceTol && !bTextFrame ) nMyTol=0; // Keine Toleranz noetig hier
    }
    if (nWdt>nMyTol && (!bTextFrame || pEdtOutl==NULL)) nMyTol=nWdt; // Bei dicker Umrandung keine Toleranz noetig, ausser wenn bei TextEdit
    Rectangle aR(aRect);
    if (nMyTol!=0 && bFilled) {
        aR.Left  ()-=nMyTol;
        aR.Top   ()-=nMyTol;
        aR.Right ()+=nMyTol;
        aR.Bottom()+=nMyTol;
    }

    if (bFilled || bLine || bTextFrame) { // Bei TextFrame so tun, alsob Linie da
        unsigned nCnt=0;
        INT32 nXShad=0,nYShad=0;
        long nEckRad=/*bTextFrame ? 0 :*/ GetEckenradius();
        do { // 1 Durchlauf, bei Schatten 2 Durchlaeufe.
            if (nCnt!=0) aR.Move(nXShad,nYShad);
            if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0 || nEckRad!=0 || !bFilled) {
                Polygon aPol(aR);
                if (nEckRad!=0) {
                    INT32 nRad=nEckRad;
                    if (bFilled) nRad+=nMyTol; // um korrekt zu sein ...
                    XPolygon aXPoly(ImpCalcXPoly(aR,nRad));
                    aPol=XOutCreatePolygon(aXPoly,NULL);
                } else {
                    if (aGeo.nShearWink!=0) ShearPoly(aPol,aRect.TopLeft(),aGeo.nTan);
                    if (aGeo.nDrehWink!=0) RotatePoly(aPol,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
                }
                if (bFilled) {
                    if (IsPointInsidePoly(aPol,rPnt)) return (SdrObject*)this;
                } else {
                    Rectangle aTouchRect(rPnt.X()-nMyTol,rPnt.Y()-nMyTol,rPnt.X()+nMyTol,rPnt.Y()+nMyTol);
                    if (IsRectTouchesLine(aPol,aTouchRect)) return (SdrObject*)this;
                }
            } else {
                if (aR.IsInside(rPnt)) return (SdrObject*)this;
            }
        } while (nCnt++==0 && ImpGetShadowDist(nXShad,nYShad));
    }
    FASTBOOL bCheckText=TRUE;
    if (bCheckText && HasText() && (!bTextFrame || bPickThrough)) {
        return SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer);
    }
    return NULL;
}

SdrObject* SdrRectObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt,nTol,pVisiLayer,FALSE/*,bTextFrame*/);
}

void SdrRectObj::TakeObjNameSingul(XubString& rName) const
{
    if (IsTextFrame()) SdrTextObj::TakeObjNameSingul(rName);
    else {
        USHORT nResId=STR_ObjNameSingulRECT;
        if (aGeo.nShearWink!=0) {
            nResId+=4;  // Parallelogramm oder Raute
            // Raute ist nicht, weil Shear die vertikalen Kanten verlaengert!
            // Wenn Zeit ist, werde ich das mal berechnen.
        } else {
            if (aRect.GetWidth()==aRect.GetHeight()) nResId+=2; // Quadrat
        }
        if (GetEckenradius()!=0) nResId+=8; // abgerundet
        rName=ImpGetResStr(nResId);
    }
}

void SdrRectObj::TakeObjNamePlural(XubString& rName) const
{
    if (IsTextFrame()) SdrTextObj::TakeObjNamePlural(rName);
    else {
        USHORT nResId=STR_ObjNamePluralRECT;
        if (aGeo.nShearWink!=0) {
            nResId+=4;  // Parallelogramm oder Raute
        } else {
            if (aRect.GetWidth()==aRect.GetHeight()) nResId+=2; // Quadrat
        }
        if (GetEckenradius()!=0) nResId+=8; // abgerundet
        rName=ImpGetResStr(nResId);
    }
}

void SdrRectObj::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=(rObj);
}

void SdrRectObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    rPoly=XPolyPolygon(ImpCalcXPoly(aRect,GetEckenradius()));
}

void SdrRectObj::TakeContour(XPolyPolygon& rPoly) const
{
    SdrTextObj::TakeContour(rPoly);
}

void SdrRectObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

void SdrRectObj::RecalcSnapRect()
{
    long nEckRad=GetEckenradius();
    if ((aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) && nEckRad!=0) {
        maSnapRect=GetXPoly().GetBoundRect();
    } else {
        SdrTextObj::RecalcSnapRect();
    }
}

void SdrRectObj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrTextObj::NbcSetSnapRect(rRect);
    SetXPolyDirty();
}

void SdrRectObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrTextObj::NbcSetLogicRect(rRect);
    SetXPolyDirty();
}

USHORT SdrRectObj::GetHdlCount() const
{
    return 9;
}

SdrHdl* SdrRectObj::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pH=NULL;
    Point aPnt;
    SdrHdlKind eKind=HDL_MOVE;
    switch (nHdlNum) {
        case 0: {
            long a=GetEckenradius();
            long b=Max(aRect.GetWidth(),aRect.GetHeight())/2; // Wird aufgerundet, da GetWidth() eins draufaddiert
            if (a>b) a=b;
            if (a<0) a=0;
            aPnt=aRect.TopLeft();
            aPnt.X()+=a;
            eKind=HDL_CIRC;
        } break; // Eckenradius
        case 1: aPnt=aRect.TopLeft();      eKind=HDL_UPLFT; break; // Oben links
        case 2: aPnt=aRect.TopCenter();    eKind=HDL_UPPER; break; // Oben
        case 3: aPnt=aRect.TopRight();     eKind=HDL_UPRGT; break; // Oben rechts
        case 4: aPnt=aRect.LeftCenter();   eKind=HDL_LEFT ; break; // Links
        case 5: aPnt=aRect.RightCenter();  eKind=HDL_RIGHT; break; // Rechts
        case 6: aPnt=aRect.BottomLeft();   eKind=HDL_LWLFT; break; // Unten links
        case 7: aPnt=aRect.BottomCenter(); eKind=HDL_LOWER; break; // Unten
        case 8: aPnt=aRect.BottomRight();  eKind=HDL_LWRGT; break; // Unten rechts
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPnt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPnt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    if (eKind!=HDL_MOVE) {
        pH=new SdrHdl(aPnt,eKind);
        pH->SetObj((SdrObject*)this);
        pH->SetDrehWink(aGeo.nDrehWink);
    }
    return pH;
}

FASTBOOL SdrRectObj::HasSpecialDrag() const
{
    return TRUE;
}

FASTBOOL SdrRectObj::BegDrag(SdrDragStat& rDrag) const
{
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    if (bRad) {
        rDrag.SetEndDragChangesAttributes(TRUE);
        return TRUE;
    } else {
        return SdrTextObj::BegDrag(rDrag);
    }
}

FASTBOOL SdrRectObj::MovDrag(SdrDragStat& rDrag) const
{
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    if (bRad) {
        return TRUE;
    } else {
        return SdrTextObj::MovDrag(rDrag);
    }
}

FASTBOOL SdrRectObj::EndDrag(SdrDragStat& rDrag)
{
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    if (bRad) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        Point aPt(rDrag.GetNow());
        if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin fuer Umkehrung
        // Shear nicht noetig, da Pt auf einer Linie mit dem RefPt (LiOb Ecke des Rect)
        long nRad=aPt.X()-aRect.Left();
        if (nRad<0) nRad=0;
        long nAltRad=GetEckenradius();
        if (nRad!=nAltRad) NbcSetEckenradius(nRad);
        SetChanged();
        SetRectsDirty();
        SetXPolyDirty();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        return TRUE;
    } else {
        return SdrTextObj::EndDrag(rDrag);
    }
}

void SdrRectObj::BrkDrag(SdrDragStat& rDrag) const
{
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    if (bRad) {
    } else {
        SdrTextObj::BrkDrag(rDrag);
    }
}

XubString SdrRectObj::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
{
    if(bCreateComment)
        return String();

    BOOL bRad(rDrag.GetHdl() && rDrag.GetHdl()->GetKind() == HDL_CIRC);

    if(bRad)
    {
        Point aPt(rDrag.GetNow());

        // -sin fuer Umkehrung
        if(aGeo.nDrehWink)
            RotatePoint(aPt, aRect.TopLeft(), -aGeo.nSin, aGeo.nCos);

        INT32 nRad(aPt.X() - aRect.Left());

        if(nRad < 0)
            nRad = 0;

        XubString aStr;

        ImpTakeDescriptionStr(STR_DragRectEckRad, aStr);
        aStr.AppendAscii(" (");
        aStr += GetMetrStr(nRad);
        aStr += sal_Unicode(')');

        return aStr;
    }
    else
    {
        return SdrTextObj::GetDragComment(rDrag, bUndoDragComment, FALSE);
    }
}

void SdrRectObj::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    FASTBOOL bRectSiz=!bRad;
    if (bRad) {
        Point aPt(rDrag.GetNow());
        if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),-aGeo.nSin,aGeo.nCos); // -sin fuer Umkehrung
        // Shear nicht noetig, da Pt auf einer Linie mit dem RefPt (LiOb Ecke des Rect)
        long nRad=aPt.X()-aRect.Left();
        if (nRad<0) nRad=0;
        rXPP.Insert(ImpCalcXPoly(aRect,nRad));
    } else {
        rXPP.Insert(ImpCalcXPoly(ImpDragCalcRect(rDrag),GetEckenradius()));
    }
}

void SdrRectObj::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();
    rXPP=XPolyPolygon(ImpCalcXPoly(aRect1,GetEckenradius()));
}

Pointer SdrRectObj::GetCreatePointer() const
{
    if (IsTextFrame()) return Pointer(POINTER_DRAW_TEXT);
    return Pointer(POINTER_DRAW_RECT);
}

void SdrRectObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    SetXPolyDirty();
}

void SdrRectObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    SetXPolyDirty();
}

void SdrRectObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
    SetXPolyDirty();
}

void SdrRectObj::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SdrTextObj::NbcShear(rRef,nWink,tn,bVShear);
    SetXPolyDirty();
}

void SdrRectObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SdrTextObj::NbcMirror(rRef1,rRef2);
    SetXPolyDirty();
}

FASTBOOL SdrRectObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return SdrTextObj::DoMacro(rRec);
}

XubString SdrRectObj::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    return SdrTextObj::GetMacroPopupComment(rRec);
}

SdrGluePoint SdrRectObj::GetVertexGluePoint(USHORT nPosNum) const
{
    INT32 nWdt = ((XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
    nWdt++;
    nWdt /= 2;

    Point aPt;
    switch (nPosNum) {
        case 0: aPt=aRect.TopCenter();    aPt.Y()-=nWdt; break;
        case 1: aPt=aRect.RightCenter();  aPt.X()+=nWdt; break;
        case 2: aPt=aRect.BottomCenter(); aPt.Y()+=nWdt; break;
        case 3: aPt=aRect.LeftCenter();   aPt.X()-=nWdt; break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(FALSE);
    return aGP;
}

SdrGluePoint SdrRectObj::GetCornerGluePoint(USHORT nPosNum) const
{
    INT32 nWdt = ((XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
    nWdt++;
    nWdt /= 2;

    Point aPt;
    switch (nPosNum) {
        case 0: aPt=aRect.TopLeft();     aPt.X()-=nWdt; aPt.Y()-=nWdt; break;
        case 1: aPt=aRect.TopRight();    aPt.X()+=nWdt; aPt.Y()-=nWdt; break;
        case 2: aPt=aRect.BottomRight(); aPt.X()+=nWdt; aPt.Y()+=nWdt; break;
        case 3: aPt=aRect.BottomLeft();  aPt.X()-=nWdt; aPt.Y()+=nWdt; break;
    }
    if (aGeo.nShearWink!=0) ShearPoint(aPt,aRect.TopLeft(),aGeo.nTan);
    if (aGeo.nDrehWink!=0) RotatePoint(aPt,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(FALSE);
    return aGP;
}

SdrObject* SdrRectObj::DoConvertToPolyObj(BOOL bBezier) const
{
    XPolygon aXP(ImpCalcXPoly(aRect,GetEckenradius()));
    { // #40608# Nur Uebergangsweise bis zum neuen TakeContour()
        aXP.Remove(0,1);
        aXP[aXP.GetPointCount()-1]=aXP[0];
    }
    SdrObject* pRet=NULL;
    if (!IsTextFrame() || HasFill() || HasLine()) {
        pRet=ImpConvertMakeObj(XPolyPolygon(aXP),TRUE,bBezier);
    }
    pRet=ImpConvertAddText(pRet,bBezier);
    return pRet;
}

void SdrRectObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
    SetXPolyDirty(); // wg. Eckenradius
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
void SdrRectObj::ItemSetChanged()
{
    // call parent
    SdrTextObj::ItemSetChanged();

    // local changes
    SetXPolyDirty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrRectObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
    SetXPolyDirty(); // wg. Eckenradius
}

SdrObjGeoData* SdrRectObj::NewGeoData() const
{ // etwas umstaendlicher, damit's vielleicht unter Chicago durchgeht
    SdrObjGeoData* pGeo=new SdrRectObjGeoData;
    return pGeo;
}

void SdrRectObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrRectObjGeoData& rRGeo=(SdrRectObjGeoData&)rGeo;
    rRGeo.nEckRad=GetEckenradius();
}

void SdrRectObj::RestGeoData(const SdrObjGeoData& rGeo)
{ // RectsDirty wird von SdrObject gerufen
    SdrTextObj::RestGeoData(rGeo);
    SdrRectObjGeoData& rRGeo=(SdrRectObjGeoData&)rGeo;
    long nAltRad=GetEckenradius();
    if (rRGeo.nEckRad!=nAltRad) NbcSetEckenradius(rRGeo.nEckRad);
    SetXPolyDirty();
}

void SdrRectObj::WriteData(SvStream& rOut) const
{
    SdrTextObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrRectObj");
#endif
}

void SdrRectObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrTextObj::ReadData(rHead,rIn);
    if (IsTextFrame() && rHead.GetVersion()<3 && !HAS_BASE(SdrCaptionObj,this)) {
        // Bis einschl. Version 2 wurden Textrahmen mit SdrTextObj dargestellt, ausser CaptionObj
        SfxItemPool* pPool=GetItemPool();
        if (pPool!=NULL) {
            // Umrandung und Hintergrund des importierten Textrahmens ausschalten
            SfxItemSet aSet(*pPool);
            aSet.Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
            aSet.Put(XFillStyleItem(XFILL_NONE));
            aSet.Put(XLineColorItem(String(),Color(COL_BLACK))); // Falls einer auf Solid umschaltet
            aSet.Put(XLineStyleItem(XLINE_NONE));

            SetItemSet(aSet);
        }
    } else {
        SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
        aCompat.SetID("SdrRectObj");
#endif
        if (rHead.GetVersion()<=5) {
            long nEckRad;
            rIn>>nEckRad;
            long nAltRad=GetEckenradius();
            if (nEckRad!=nAltRad) NbcSetEckenradius(nEckRad);
        }
    }
    SetXPolyDirty();
}

