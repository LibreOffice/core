/*************************************************************************
 *
 *  $RCSfile: svdocapt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:11:37 $
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

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#include "svdocapt.hxx"
#include "xpool.hxx"
#include "xpoly.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svdxout.hxx"
#include "svdetc.hxx"
#include "svdtrans.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdhdl.hxx"
#include "svddrag.hxx"
#include "svdmodel.hxx"
#include "svdview.hxx"   // fuer RectSnap
#include "svdpagv.hxx"   // fuer GetOffset bei BegDrag()
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname
#include "svdogrp.hxx"
#include "svdpage.hxx"

#ifndef _SVX_XFLHTIT_HXX
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif

#ifndef _SVX_XFLTRIT_HXX
#include <xfltrit.hxx>
#endif

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

enum EscDir {LKS,RTS,OBN,UNT};

class ImpCaptParams
{
public:
    SdrCaptionType              eType;
    long                        nAngle;
    long                        nGap;
    long                        nEscRel;
    long                        nEscAbs;
    long                        nLineLen;
    SdrCaptionEscDir            eEscDir;
    FASTBOOL                    bFitLineLen;
    FASTBOOL                    bEscRel;
    FASTBOOL                    bFixedAngle;

public:
    ImpCaptParams()
    {
        eType      =SDRCAPT_TYPE3;
        bFixedAngle=FALSE;
        nAngle     =4500;
        nGap       =0;
        eEscDir    =SDRCAPT_ESCHORIZONTAL;
        bEscRel    =TRUE;
        nEscRel    =5000;
        nEscAbs    =0;
        nLineLen   =0;
        bFitLineLen=TRUE;
    }
    void CalcEscPos(const Point& rTail, const Rectangle& rRect, Point& rPt, EscDir& rDir) const;
};

void ImpCaptParams::CalcEscPos(const Point& rTailPt, const Rectangle& rRect, Point& rPt, EscDir& rDir) const
{
    Point aTl(rTailPt); // lokal kopieren wg. Performance
    long nX,nY;
    if (bEscRel) {
        nX=rRect.Right()-rRect.Left();
        nX=BigMulDiv(nX,nEscRel,10000);
        nY=rRect.Bottom()-rRect.Top();
        nY=BigMulDiv(nY,nEscRel,10000);
    } else {
        nX=nEscAbs;
        nY=nEscAbs;
    }
    nX+=rRect.Left();
    nY+=rRect.Top();
    Point  aBestPt;
    EscDir eBestDir=LKS;
    FASTBOOL bTryH=eEscDir==SDRCAPT_ESCBESTFIT;
    if (!bTryH) {
        if (eType!=SDRCAPT_TYPE1) {
            bTryH=eEscDir==SDRCAPT_ESCHORIZONTAL;
        } else {
            bTryH=eEscDir==SDRCAPT_ESCVERTICAL;
        }
    }
    FASTBOOL bTryV=eEscDir==SDRCAPT_ESCBESTFIT;
    if (!bTryV) {
        if (eType!=SDRCAPT_TYPE1) {
            bTryV=eEscDir==SDRCAPT_ESCVERTICAL;
        } else {
            bTryV=eEscDir==SDRCAPT_ESCHORIZONTAL;
        }
    }

    if (bTryH) {
        Point aLft(rRect.Left()-nGap,nY);
        Point aRgt(rRect.Right()+nGap,nY);
        FASTBOOL bLft=(aTl.X()-aLft.X()<aRgt.X()-aTl.X());
        if (bLft) {
            eBestDir=LKS;
            aBestPt=aLft;
        } else {
            eBestDir=RTS;
            aBestPt=aRgt;
        }
    }
    if (bTryV) {
        Point aTop(nX,rRect.Top()-nGap);
        Point aBtm(nX,rRect.Bottom()+nGap);
        FASTBOOL bTop=(aTl.Y()-aTop.Y()<aBtm.Y()-aTl.Y());
        Point aBest2;
        EscDir eBest2;
        if (bTop) {
            eBest2=OBN;
            aBest2=aTop;
        } else {
            eBest2=UNT;
            aBest2=aBtm;
        }
        FASTBOOL bTakeIt=eEscDir!=SDRCAPT_ESCBESTFIT;
        if (!bTakeIt) {
            BigInt aHorX(aBestPt.X()-aTl.X()); aHorX*=aHorX;
            BigInt aHorY(aBestPt.Y()-aTl.Y()); aHorY*=aHorY;
            BigInt aVerX(aBest2.X()-aTl.X());  aVerX*=aVerX;
            BigInt aVerY(aBest2.Y()-aTl.Y());  aVerY*=aVerY;
            if (eType!=SDRCAPT_TYPE1) {
                bTakeIt=aVerX+aVerY<aHorX+aHorY;
            } else {
                bTakeIt=aVerX+aVerY>=aHorX+aHorY;
            }
        }
        if (bTakeIt) {
            aBestPt=aBest2;
            eBestDir=eBest2;
        }
    }
    rPt=aBestPt;
    rDir=eBestDir;
}

TYPEINIT1(SdrCaptionObj,SdrRectObj);

SdrCaptionObj::SdrCaptionObj():
    SdrRectObj(OBJ_TEXT),
    aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
    mbSpecialTextBoxShadow(FALSE)
{
//-/    pCaptAttr=NULL;
}

SdrCaptionObj::SdrCaptionObj(const Rectangle& rRect):
    SdrRectObj(OBJ_TEXT,rRect),
    aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
    mbSpecialTextBoxShadow(FALSE)
{
//-/    pCaptAttr=NULL;
}

SdrCaptionObj::SdrCaptionObj(const Rectangle& rRect, const Point& rTail):
    SdrRectObj(OBJ_TEXT,rRect),
    aTailPoly(3),  // Default Groesse: 3 Punkte = 2 Linien
    mbSpecialTextBoxShadow(FALSE)
{
    aTailPoly[0]=rTail;
//-/    pCaptAttr=NULL;
}

SdrCaptionObj::~SdrCaptionObj()
{
    // Attr entfernen (oder macht das SdrAttrObj noch nicht automatisch)
//-/    pCaptAttr=(SdrCaptionSetItem*)ImpSetNewAttr(pCaptAttr,NULL,FALSE);
}

FASTBOOL SdrCaptionObj::Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    // special shadow paint for calc
    if(mbSpecialTextBoxShadow)
    {
        const SfxItemSet& rSet = GetItemSet();
        sal_uInt32 nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        sal_uInt32 nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
        const SdrShadowColorItem& rShadColItem = ((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
        Color aShadCol(rShadColItem.GetValue());
        sal_uInt16 nTransp = ((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
        XFillStyle eStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

//-/        XFillAttrSetItem aF(rSet.GetPool());
        SfxItemSet aSet(rSet);
//-/        aF.GetItemSet().Put(rSet);

        if(eStyle == XFILL_HATCH) // #41666#
        {
            XHatch aHatch = ((XFillHatchItem&)(rSet.Get(XATTR_FILLHATCH))).GetValue();
            aHatch.SetColor(aShadCol);
            aSet.Put(XFillHatchItem(String(),aHatch));
        }
        else
        {
            if(eStyle != XFILL_NONE && eStyle != XFILL_SOLID)
            {
                // also fuer Gradient und Bitmap
                aSet.Put(XFillStyleItem(XFILL_SOLID));
            }

            aSet.Put(XFillColorItem(String(),aShadCol));
            aSet.Put(XFillTransparenceItem(nTransp));
        }

        rOut.SetFillAttr(aSet);

        sal_Int32 nEckRad(GetEckenradius());
        if(PaintNeedsXPoly(nEckRad))
        {
            XPolygon aX(GetXPoly());
            aX.Move(nXDist,nYDist);
            rOut.DrawXPolygon(aX);
        }
        else
        {
            Rectangle aR(aRect);
            aR.Move(nXDist,nYDist);
            rOut.DrawRect(aR,USHORT(2*nEckRad),USHORT(2*nEckRad));
        }
    }

    // call parent for normal paint
    return SdrRectObj::Paint(rOut, rInfoRec);
}

void SdrCaptionObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bCanConvToPath    =TRUE;
    rInfo.bCanConvToPoly    =TRUE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

UINT16 SdrCaptionObj::GetObjIdentifier() const
{
    return UINT16(OBJ_CAPTION);
}

void SdrCaptionObj::RecalcBoundRect()
{
    aOutRect=GetSnapRect();
    long nLineWdt=ImpGetLineWdt();
    nLineWdt++; nLineWdt/=2;
    long nLEndWdt=ImpGetLineEndAdd();
    if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
    if (nLineWdt!=0) {
        aOutRect.Left  ()-=nLineWdt;
        aOutRect.Top   ()-=nLineWdt;
        aOutRect.Right ()+=nLineWdt;
        aOutRect.Bottom()+=nLineWdt;
    }
    ImpAddShadowToBoundRect();
    ImpAddTextToBoundRect();
}

SdrObject* SdrCaptionObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    FASTBOOL bHit=SdrRectObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
    if (!bHit) {
        INT32 nMyTol=nTol;
//-/        INT32 nWdt=0;

//-/        if(mpObjectItemSet)
//-/        {
        INT32 nWdt = ((XLineWidthItem&)(GetItem(XATTR_LINEWIDTH))).GetValue();
        nWdt++;
        nWdt /= 2;
//-/        }

        if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
        Rectangle aR(rPnt,rPnt);
        aR.Left()  -=nMyTol;
        aR.Right() +=nMyTol;
        aR.Top()   -=nMyTol;
        aR.Bottom()+=nMyTol;
        bHit=IsRectTouchesLine(aTailPoly,aR);
    }
    return bHit ? (SdrObject*)this : NULL;
}

void SdrCaptionObj::operator=(const SdrObject& rObj)
{
    SdrRectObj::operator=(rObj);
    aTailPoly=((SdrCaptionObj&)rObj).aTailPoly;
//-/    pCaptAttr=(SdrCaptionSetItem*)ImpSetNewAttr(pCaptAttr,((SdrCaptionObj&)rObj).pCaptAttr);
}

void SdrCaptionObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulCAPTION);
}

void SdrCaptionObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralCAPTION);
}

void SdrCaptionObj::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const
{
    SdrRectObj::TakeXorPoly(rPoly,bDetail);
    rPoly.Insert(XPolygon(aTailPoly));
}

void SdrCaptionObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

USHORT SdrCaptionObj::GetHdlCount() const
{
    USHORT nAnz1=SdrRectObj::GetHdlCount();
    USHORT nAnz2=aTailPoly.GetSize();
    // Derzeit ist nur das Draggen des Schwanzendes implementiert
    return nAnz1+1;
}

SdrHdl* SdrCaptionObj::GetHdl(USHORT nHdlNum) const
{
    USHORT nRectHdlAnz=SdrRectObj::GetHdlCount();
    if (nHdlNum<nRectHdlAnz) {
        return SdrRectObj::GetHdl(nHdlNum);
    } else {
        USHORT nPntNum=nHdlNum;
        nPntNum-=nRectHdlAnz;
        if (nPntNum<aTailPoly.GetSize()) {
            SdrHdl* pHdl=new SdrHdl(aTailPoly.GetPoint(nPntNum),HDL_POLY);
            pHdl->SetPolyNum(1);
            pHdl->SetPointNum(nPntNum);
            return pHdl;
        } else return NULL;
    }
}

FASTBOOL SdrCaptionObj::HasSpecialDrag() const
{
    return TRUE;
}

FASTBOOL SdrCaptionObj::BegDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL && pHdl->GetPolyNum()==0) {
        return SdrRectObj::BegDrag(rDrag);
    } else {
        rDrag.SetOrtho8Possible(TRUE);
        if (pHdl==NULL) {
            if (bMovProt) return FALSE; // Position geschuetzt
            rDrag.SetNoSnap(TRUE); // Snap mache ich in diesem Fall selbst (RectSnap)
            rDrag.SetActionRect(aRect);
            Point aHit(rDrag.GetStart());
            if (rDrag.GetPageView()!=NULL) { // Hitposition bei versetzter PageView korregieren
                aHit-=rDrag.GetPageView()->GetOffset();
            }
            if (SdrRectObj::CheckHit(aHit,0,NULL)!=NULL) return TRUE;
            else return FALSE;
        } else {
            return (pHdl->GetPolyNum()==1) && (pHdl->GetPointNum()==0);
        }
    }
}

FASTBOOL SdrCaptionObj::MovDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL && pHdl->GetPolyNum()==0) {
        return SdrRectObj::MovDrag(rDrag);
    } else {
        SdrView* pView=rDrag.GetView();
        SdrPageView* pPV=rDrag.GetPageView();
        Rectangle aR(aRect);
        aR.Move(rDrag.GetDX(),rDrag.GetDY());
        if (pView!=NULL && pPV!=NULL && pView->IsSnapEnabled()) { // RectSnap
            long nDX=0,nDY=0;
            pView->SnapRect(aR,pPV,nDX,nDY);
            rDrag.Now().X()+=nDX;
            rDrag.Now().Y()+=nDY;
            aR.Move(nDX,nDY);
        }
        rDrag.SetActionRect(aR);
        return TRUE;
    }
}

FASTBOOL SdrCaptionObj::EndDrag(SdrDragStat& rDrag)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL && pHdl->GetPolyNum()==0) {
        FASTBOOL bRet=SdrRectObj::EndDrag(rDrag);
        ImpRecalcTail();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        return bRet;
    } else {
        SendRepaintBroadcast();
        Point aDelt(rDrag.GetNow()-rDrag.GetStart());
        if (pHdl==NULL) { // Rect verschoben
            aRect.Move(aDelt.X(),aDelt.Y());
        } else {          // Schwanz verschoben
            aTailPoly[0]+=aDelt;
        }
        ImpRecalcTail();
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        return TRUE;
    }
}

void SdrCaptionObj::BrkDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL && pHdl->GetPolyNum()==0) {
        SdrRectObj::BrkDrag(rDrag);
    } else {
    }
}

XubString SdrCaptionObj::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
{
    if (bCreateComment) return String();
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl!=NULL && pHdl->GetPolyNum()==0) {
        return SdrRectObj::GetDragComment(rDrag,bUndoDragComment,FALSE);
    } else {
        XubString aStr;
        if (pHdl==NULL) {
            ImpTakeDescriptionStr(STR_DragCaptFram,aStr);
        } else {
            ImpTakeDescriptionStr(STR_DragCaptTail,aStr);
        }
        return aStr;
    }
}

void SdrCaptionObj::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    const SdrHdl* pHdl=rDrag.GetHdl();
    FASTBOOL bRad=rDrag.GetHdl()!=NULL && rDrag.GetHdl()->GetKind()==HDL_CIRC;
    FASTBOOL bRectSiz=(pHdl!=NULL && pHdl->GetPolyNum()==0);
    if (bRad) {
        SdrRectObj::TakeDragPoly(rDrag,rXPP);
    } else {
        Point aDelt(rDrag.GetNow()-rDrag.GetStart());
        Polygon aTmpPoly(aTailPoly);
        Rectangle aTmpRect;
        if (bRectSiz) aTmpRect=ImpDragCalcRect(rDrag);
        else aTmpRect=aRect;
        ImpCaptParams aPara;
        ImpGetCaptParams(aPara);
        if (!bRectSiz) {
            if (pHdl==NULL) { // Rect verschieben
                aTmpRect.Move(aDelt.X(),aDelt.Y());
            } else {          // Schwanz verschieben
                aTmpPoly[0]+=aDelt;
            }
        }
        ImpCalcTail(aPara,aTmpPoly,aTmpRect);
        rXPP.Insert(ImpCalcXPoly(aTmpRect,GetEckenradius()));
        rXPP.Insert(XPolygon(aTmpPoly));
    }
}

void SdrCaptionObj::ImpGetCaptParams(ImpCaptParams& rPara) const
{
//-/    if(mpObjectItemSet)
//-/    {
    const SfxItemSet& rSet = GetItemSet();
    rPara.eType      =((SdrCaptionTypeItem&)      (rSet.Get(SDRATTR_CAPTIONTYPE      ))).GetValue();
    rPara.bFixedAngle=((SdrCaptionFixedAngleItem&)(rSet.Get(SDRATTR_CAPTIONANGLE     ))).GetValue();
    rPara.nAngle     =((SdrCaptionAngleItem&)     (rSet.Get(SDRATTR_CAPTIONFIXEDANGLE))).GetValue();
    rPara.nGap       =((SdrCaptionGapItem&)       (rSet.Get(SDRATTR_CAPTIONGAP       ))).GetValue();
    rPara.eEscDir    =((SdrCaptionEscDirItem&)    (rSet.Get(SDRATTR_CAPTIONESCDIR    ))).GetValue();
    rPara.bEscRel    =((SdrCaptionEscIsRelItem&)  (rSet.Get(SDRATTR_CAPTIONESCISREL  ))).GetValue();
    rPara.nEscRel    =((SdrCaptionEscRelItem&)    (rSet.Get(SDRATTR_CAPTIONESCREL    ))).GetValue();
    rPara.nEscAbs    =((SdrCaptionEscAbsItem&)    (rSet.Get(SDRATTR_CAPTIONESCABS    ))).GetValue();
    rPara.nLineLen   =((SdrCaptionLineLenItem&)   (rSet.Get(SDRATTR_CAPTIONLINELEN   ))).GetValue();
    rPara.bFitLineLen=((SdrCaptionFitLineLenItem&)(rSet.Get(SDRATTR_CAPTIONFITLINELEN))).GetValue();
//-/    }
}

void SdrCaptionObj::ImpRecalcTail()
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    ImpCalcTail(aPara,aTailPoly,aRect);
    SetRectsDirty();
    SetXPolyDirty();
}

void SdrCaptionObj::ImpCalcTail1(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{ // Gap/EscDir/EscPos
    Polygon aPol(2);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;
    aPol[1]=aTl;
    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    if (eEscDir==LKS || eEscDir==RTS) {
        long dx=aTl.X()-aEscPos.X();
        rRect.Move(dx,0);
        aPol[1].Y()=aEscPos.Y();
    } else {
        long dy=aTl.Y()-aEscPos.Y();
        rRect.Move(0,dy);
        aPol[1].X()=aEscPos.X();
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail2(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{ // Gap/EscDir/EscPos/Angle
    Polygon aPol(2);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;

    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    aPol[1]=aEscPos;

    if (!rPara.bFixedAngle) {
        // fehlende Implementation
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail3(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{ // Gap/EscDir/EscPos/Angle/LineLen
    Polygon aPol(3);
    Point aTl(rPoly[0]);
    aPol[0]=aTl;

    EscDir eEscDir;
    Point aEscPos;
    rPara.CalcEscPos(aTl,rRect,aEscPos,eEscDir);
    aPol[1]=aEscPos;
    aPol[2]=aEscPos;

    if (eEscDir==LKS || eEscDir==RTS) {
        if (rPara.bFitLineLen) {
            aPol[1].X()=(aTl.X()+aEscPos.X())/2;
        } else {
            if (eEscDir==LKS) aPol[1].X()-=rPara.nLineLen;
            else aPol[1].X()+=rPara.nLineLen;
        }
    } else {
        if (rPara.bFitLineLen) {
            aPol[1].Y()=(aTl.Y()+aEscPos.Y())/2;
        } else {
            if (eEscDir==OBN) aPol[1].Y()-=rPara.nLineLen;
            else aPol[1].Y()+=rPara.nLineLen;
        }
    }
    if (!rPara.bFixedAngle) {
        // fehlende Implementation
    }
    rPoly=aPol;
}

void SdrCaptionObj::ImpCalcTail4(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{
    ImpCalcTail3(rPara,rPoly,rRect);
}

void SdrCaptionObj::ImpCalcTail(const ImpCaptParams& rPara, Polygon& rPoly, Rectangle& rRect) const
{
    switch (rPara.eType) {
        case SDRCAPT_TYPE1: ImpCalcTail1(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE2: ImpCalcTail2(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE3: ImpCalcTail3(rPara,rPoly,rRect); break;
        case SDRCAPT_TYPE4: ImpCalcTail4(rPara,rPoly,rRect); break;
    }
}

FASTBOOL SdrCaptionObj::BegCreate(SdrDragStat& rStat)
{
    if (aRect.IsEmpty()) return FALSE; // Create z.Zt. nur mit vorgegebenen Rect

    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    aTailPoly[0]=rStat.GetStart();
    ImpCalcTail(aPara,aTailPoly,aRect);
    rStat.SetActionRect(aRect);
    return TRUE;
}

FASTBOOL SdrCaptionObj::MovCreate(SdrDragStat& rStat)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,aRect);
    rStat.SetActionRect(aRect);
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    return TRUE;
}

FASTBOOL SdrCaptionObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpCaptParams aPara;
    ImpGetCaptParams(aPara);
    aRect.SetPos(rStat.GetNow());
    ImpCalcTail(aPara,aTailPoly,aRect);
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

FASTBOOL SdrCaptionObj::BckCreate(SdrDragStat& rStat)
{
    return FALSE;
}

void SdrCaptionObj::BrkCreate(SdrDragStat& rStat)
{
}

void SdrCaptionObj::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    rXPP.Insert(XPolygon(aRect));
    rXPP.Insert(XPolygon(aTailPoly));
}

Pointer SdrCaptionObj::GetCreatePointer() const
{
    return Pointer(POINTER_DRAW_CAPTION);
}

void SdrCaptionObj::NbcMove(const Size& rSiz)
{
    SdrRectObj::NbcMove(rSiz);
    MovePoly(aTailPoly,rSiz);
}

void SdrCaptionObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);
    ResizePoly(aTailPoly,rRef,xFact,yFact);
    ImpRecalcTail();
}

void SdrCaptionObj::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(aTailPoly.GetPoint(0)-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

Point SdrCaptionObj::GetRelativePos() const
{
    return aTailPoly.GetPoint(0)-aAnchor;
}

void SdrCaptionObj::NbcSetAnchorPos(const Point& rPnt)
{
    SdrRectObj::NbcSetAnchorPos(rPnt);
    // !!!!! fehlende Impl.
}

const Point& SdrCaptionObj::GetAnchorPos() const
{
    // !!!!! fehlende Impl.
    return SdrRectObj::GetAnchorPos();
}

void SdrCaptionObj::RecalcSnapRect()
{
    SdrRectObj::RecalcSnapRect();
    maSnapRect.Union(aTailPoly.GetBoundRect());
    // !!!!! fehlende Impl.
}

const Rectangle& SdrCaptionObj::GetSnapRect() const
{
    return SdrRectObj::GetSnapRect();
}

void SdrCaptionObj::NbcSetSnapRect(const Rectangle& rRect)
{
    // !!!!! fehlende Impl.
}

const Rectangle& SdrCaptionObj::GetLogicRect() const
{
    return aRect;
}

void SdrCaptionObj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    ImpRecalcTail();
}

const Point& SdrCaptionObj::GetTailPos() const
{
    return aTailPoly[0];
}

void SdrCaptionObj::SetTailPos(const Point& rPos)
{
    if (aTailPoly.GetSize()==0 || aTailPoly[0]!=rPos) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcSetTailPos(rPos);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrCaptionObj::NbcSetTailPos(const Point& rPos)
{
    aTailPoly[0]=rPos;
    ImpRecalcTail();
}

USHORT SdrCaptionObj::GetSnapPointCount() const
{
    // !!!!! fehlende Impl.
    return 0;
}

Point SdrCaptionObj::GetSnapPoint(USHORT i) const
{
    // !!!!! fehlende Impl.
    return Point(0,0);
}

void SdrCaptionObj::SetModel(SdrModel* pNewModel)
{
    SdrRectObj::SetModel(pNewModel);
    ImpRecalcTail();
}

//-/void SdrCaptionObj::ForceDefaultAttr(SfxItemPool* pPool)
//-/{
//-/    FASTBOOL bLineMerk=pLineAttr==NULL;
//-/    SdrRectObj::ForceDefaultAttr(pPool);
//-/    if (pPool!=NULL) {
//-/        if (pCaptAttr==NULL) {
//-/            SdrCaptionSetItem aSetItem(pPool);
//-/            pCaptAttr=(SdrCaptionSetItem*)ImpSetNewAttr(pCaptAttr,&aSetItem,FALSE);
//-/        }
//-/        if (bLineMerk && pLineAttr!=NULL) {
//-/            XLineAttrSetItem aSetItem(*pLineAttr);
//-/            aSetItem.GetItemSet().Put(XLineStyleItem(XLINE_SOLID));
//-/            pLineAttr=(XLineAttrSetItem*)ImpSetNewAttr(pLineAttr,&aSetItem,FALSE);
//-/        }
//-/    }
//-/}

//-/void SdrCaptionObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    SdrRectObj::NbcSetAttributes(rAttr,bReplaceAll);
//-/    ImpRecalcTail();
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCaptionObj::SetItem( const SfxPoolItem& rItem )
{
    ImpRecalcTail();
    SdrRectObj::SetItem(rItem);
}

void SdrCaptionObj::ClearItem( USHORT nWhich )
{
    ImpRecalcTail();
    SdrRectObj::ClearItem(nWhich);
}

void SdrCaptionObj::SetItemSet( const SfxItemSet& rSet )
{
    ImpRecalcTail();
    SdrRectObj::SetItemSet(rSet);
}

SfxItemSet* SdrCaptionObj::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items, 2D and 3D
    return new SfxItemSet(rPool,
        // ranges from SdrAttrObj
        SDRATTR_START, SDRATTRSET_SHADOW,
        SDRATTRSET_OUTLINER, SDRATTRSET_MISC,

        // caption attributes
        SDRATTR_CAPTION_FIRST, SDRATTRSET_CAPTION,

        // outliner and end
        EE_ITEMS_START, EE_ITEMS_END,
        0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCaptionObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    SdrRectObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
    ImpRecalcTail();
}

void SdrCaptionObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrRectObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
    ImpRecalcTail();
}

//-/USHORT SdrCaptionObj::GetSetItemCount() const
//-/{
//-/    return 1+SdrRectObj::GetSetItemCount();
//-/}

//-/const SfxSetItem* SdrCaptionObj::GetSetItem(USHORT nNum) const
//-/{
//-/    if (nNum==0) return pCaptAttr;
//-/    nNum--;
//-/    return SdrRectObj::GetSetItem(nNum);
//-/}

//-/void SdrCaptionObj::SetSetItem(USHORT nNum, const SfxSetItem* pAttr)
//-/{
//-/    if (nNum==0) pCaptAttr=(const SdrCaptionSetItem*)pAttr;
//-/    else {
//-/        nNum--;
//-/        SdrRectObj::SetSetItem(nNum,pAttr);
//-/    }
//-/}

//-/SfxSetItem* SdrCaptionObj::MakeNewSetItem(USHORT nNum, FASTBOOL bClone) const
//-/{
//-/    if (nNum==0) {
//-/        if (bClone) return new SdrCaptionSetItem(*pCaptAttr);
//-/        else return new SdrCaptionSetItem(GetItemPool());
//-/    } else {
//-/        nNum--;
//-/        return SdrRectObj::MakeNewSetItem(nNum,bClone);
//-/    }
//-/}

SdrObjGeoData* SdrCaptionObj::NewGeoData() const
{
    return new SdrCaptObjGeoData;
}

void SdrCaptionObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrRectObj::SaveGeoData(rGeo);
    SdrCaptObjGeoData& rCGeo=(SdrCaptObjGeoData&)rGeo;
    rCGeo.aTailPoly=aTailPoly;
}

void SdrCaptionObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::RestGeoData(rGeo);
    SdrCaptObjGeoData& rCGeo=(SdrCaptObjGeoData&)rGeo;
    aTailPoly=rCGeo.aTailPoly;
}

SdrObject* SdrCaptionObj::DoConvertToPolyObj(BOOL bBezier) const
{ // #42334# - Convert implementiert
    SdrObject* pRect=SdrRectObj::DoConvertToPolyObj(bBezier);
    SdrObject* pTail=ImpConvertMakeObj(XPolyPolygon(XPolygon(aTailPoly)),FALSE,bBezier);
    SdrObject* pRet=(pTail!=NULL) ? pTail : pRect;
    if (pTail!=NULL && pRect!=NULL) {
        FASTBOOL bInsRect=TRUE;
        FASTBOOL bInsTail=TRUE;
        SdrObjList* pOL=pTail->GetSubList();
        if (pOL!=NULL) { pRet=pRect; bInsTail=FALSE; }
        if (pOL==NULL) pOL=pRect->GetSubList();
        if (pOL!=NULL) { pRet=pRect; bInsRect=FALSE; }
        if (pOL==NULL) {
            SdrObjGroup* pGrp=new SdrObjGroup;
            pOL=pGrp->GetSubList();
            pRet=pGrp;
        }
        if (bInsRect) pOL->NbcInsertObject(pRect);
        if (bInsTail) pOL->NbcInsertObject(pTail,0);
    }
    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrCaptionObj::PreSave()
{
    // call parent
    SdrRectObj::PreSave();

    // prepare SetItems for storage
    const SfxItemSet& rSet = GetItemSet();
    const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
    SdrCaptionSetItem aCaptAttr(rSet.GetPool());
    aCaptAttr.GetItemSet().Put(rSet);
    aCaptAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aCaptAttr);
}

void SdrCaptionObj::PostSave()
{
    // call parent
    SdrRectObj::PostSave();

    // remove SetItems from local itemset
    mpObjectItemSet->ClearItem(SDRATTRSET_CAPTION);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrCaptionObj::WriteData(SvStream& rOut) const
{
    SdrRectObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrCaptionObj");
#endif

    rOut << aTailPoly;
    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_CAPTION));



//-/        SdrCaptionSetItem aCaptAttr(pPool);
//-/        aCaptAttr.GetItemSet().Put(GetItemSet());
//-/        const SfxPoolItem& rCaptAttr = pPool->Put(aCaptAttr);
//-/        pPool->StoreSurrogate(rOut, &rCaptAttr);
//-/        pPool->StoreSurrogate(rOut,pCaptAttr);
    }
    else
    {
        rOut << UINT16(SFX_ITEMS_NULL);
    }
}

void SdrCaptionObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if(rIn.GetError())
        return;

    SdrRectObj::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrCaptionObj");
#endif

    rIn >> aTailPoly;

    if(rHead.GetVersion() < 11) { sal_uInt16 nWhichDum; rIn >> nWhichDum; } // ab V11 keine WhichId mehr

    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        sal_uInt16 nSetID = SDRATTRSET_CAPTION;
        const SdrCaptionSetItem* pCaptAttr = (const SdrCaptionSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
        if(pCaptAttr)
            SetItemSet(pCaptAttr->GetItemSet());
//-/        pCaptAttr=(const SdrCaptionSetItem*)ImpSetNewAttr(pCaptAttr,NULL);     // ggf altes rauswerfen
//-/        USHORT nWhichRef=SDRATTRSET_CAPTION;
//-/        pCaptAttr=(const SdrCaptionSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);
//-/        if (pStyleSheet!=NULL && pCaptAttr!=NULL) {
//-/            ((SfxItemSet*)&pCaptAttr->GetItemSet())->SetParent(&pStyleSheet->GetItemSet());
//-/        }
    }
    else
    {
        sal_uInt16 nSuroDum;
        rIn >> nSuroDum;
    }
}

