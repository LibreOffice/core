/*************************************************************************
 *
 *  $RCSfile: svdopath.cxx,v $
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

#include <tools/bigint.hxx>
#include "svdopath.hxx"
#include <math.h>
#include "xpool.hxx"
#include "xpoly.hxx"
#include "xoutx.hxx"
#include "svdxout.hxx"
#include "svdattr.hxx"
#include "svdtouch.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdetc.hxx"
#include "svddrag.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdhdl.hxx"
//#include "svdosmrt.hxx"
#include "svdview.hxx"  // fuer MovCreate bei Freihandlinien
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname

#ifdef _MSC_VER
#pragma optimize ("",off)
#endif

#include "xlnwtit.hxx"
#include "xlnclit.hxx"
#include "xflclit.hxx"
#include "svdogrp.hxx"

#ifndef _SVX_XLNTRIT_HXX
#include "xlntrit.hxx"
#endif

/*************************************************************************/

#define SVDOPATH_INITSIZE   20
#define SVDOPATH_RESIZE     20

/*************************************************************************/

SdrPathObjGeoData::SdrPathObjGeoData()
{
}

SdrPathObjGeoData::~SdrPathObjGeoData()
{
}

TYPEINIT1(SdrPathObj,SdrTextObj);

SdrPathObj::SdrPathObj(SdrObjKind eNewKind)
{
    eKind=eNewKind;
    bClosedObj=IsClosed();
    bCreating=FALSE;
}

SdrPathObj::SdrPathObj(SdrObjKind eNewKind, const XPolyPolygon& rPathPoly):
    aPathPolygon(rPathPoly)
{
    eKind=eNewKind;
    bClosedObj=IsClosed();
    bCreating=FALSE;
    ImpForceKind();
}

SdrPathObj::SdrPathObj(const Point& rPt1, const Point& rPt2)
{
    eKind=OBJ_LINE;
    XPolygon aXP(2);
    aXP[0]=rPt1;
    aXP[1]=rPt2;
    aPathPolygon.Insert(aXP);
    bClosedObj=FALSE;
    bCreating=FALSE;
    ImpForceLineWink();
}

SdrPathObj::~SdrPathObj()
{
}

FASTBOOL SdrPathObj::FindPolyPnt(USHORT nAbsPnt, USHORT& rPolyNum,
                             USHORT& rPointNum, FASTBOOL bAllPoints) const
{
    USHORT nPolyCnt=aPathPolygon.Count();
    USHORT nPoly=0;
    FASTBOOL bClosed=IsClosed();
    nAbsPnt+=1;

    while (nPoly<nPolyCnt) {
        const XPolygon& rXPoly = aPathPolygon[nPoly];
        USHORT nPnt=0;
        USHORT nCnt=0;
        USHORT nPntCnt=rXPoly.GetPointCount();
        // geschlossen? Dann Endpunkt=Startpunkt
        if (bClosed && nPntCnt>1) nPntCnt--;

        while (nPnt<nPntCnt) {
            if (bAllPoints || rXPoly.GetFlags(nPnt)!=XPOLY_CONTROL) nCnt++;
            if (nCnt==nAbsPnt) {
                rPolyNum=nPoly;
                rPointNum=nPnt;
                return TRUE;
            }
            nPnt++;
        }
        nAbsPnt-=nCnt;
        nPoly++;
    }
    return FALSE;
}

void SdrPathObj::ImpForceLineWink()
{
    if (aPathPolygon.Count()==1 && aPathPolygon[0].GetPointCount()==2 && eKind==OBJ_LINE) {
        Point aDelt(aPathPolygon[0][1]);
        aDelt-=aPathPolygon[0][0];
        aGeo.nDrehWink=GetAngle(aDelt);
        aGeo.nShearWink=0;
        aGeo.RecalcSinCos();
        aGeo.RecalcTan();
    }
}

void SdrPathObj::ImpForceKind()
{
    if (eKind==OBJ_PATHPLIN) eKind=OBJ_PLIN;
    if (eKind==OBJ_PATHPOLY) eKind=OBJ_POLY;

    USHORT nPolyAnz=aPathPolygon.Count();
    USHORT nPoly1PointAnz=nPolyAnz==0 ? 0 : aPathPolygon[0].GetPointCount();
    FASTBOOL bHasCtrl=FALSE;
    FASTBOOL bHasLine=FALSE; // gemischt wird jedoch z.Zt. nicht in eKind festgehalten
    for (USHORT nPolyNum=0; nPolyNum<nPolyAnz && (!bHasCtrl || !bHasLine); nPolyNum++) {
        const XPolygon& rPoly=aPathPolygon[nPolyNum];
        USHORT nPointAnz=rPoly.GetPointCount();
        for (USHORT nPointNum=0; nPointNum<nPointAnz && (!bHasCtrl || !bHasLine); nPointNum++) {
            if (rPoly.IsControl(nPointNum)) bHasCtrl=TRUE;
            if (nPointNum+1<nPointAnz && !rPoly.IsControl(nPointNum) && !rPoly.IsControl(nPointNum+1)) bHasLine=TRUE;
        }
    }
    if (!bHasCtrl) {
        switch (eKind) {
            case OBJ_PATHLINE: eKind=OBJ_PLIN; break;
            case OBJ_FREELINE: eKind=OBJ_PLIN; break;
            case OBJ_PATHFILL: eKind=OBJ_POLY; break;
            case OBJ_FREEFILL: eKind=OBJ_POLY; break;
        }
    } else {
        switch (eKind) {
            case OBJ_LINE: eKind=OBJ_PATHLINE; break;
            case OBJ_PLIN: eKind=OBJ_PATHLINE; break;
            case OBJ_POLY: eKind=OBJ_PATHFILL; break;
        }
    }

    if (eKind==OBJ_LINE && (nPolyAnz!=1 || nPoly1PointAnz!=2)) eKind=OBJ_PLIN;
    if (eKind==OBJ_PLIN && (nPolyAnz==1 && nPoly1PointAnz==2)) eKind=OBJ_LINE;

    bClosedObj=IsClosed();
    if (eKind==OBJ_LINE) ImpForceLineWink();
}

void SdrPathObj::ImpSetClosed(FASTBOOL bClose)
{
    if (bClose) {
        switch (eKind) {
            case OBJ_LINE    : eKind=OBJ_POLY;     break;
            case OBJ_PLIN    : eKind=OBJ_POLY;     break;
            case OBJ_PATHLINE: eKind=OBJ_PATHFILL; break;
            case OBJ_FREELINE: eKind=OBJ_FREEFILL; break;
            case OBJ_SPLNLINE: eKind=OBJ_SPLNFILL; break;
        }
        // Nun das Poly noch schliessen
        USHORT nPolyAnz=aPathPolygon.Count();
        for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
             XPolygon& rXP=aPathPolygon[nPolyNum];
             USHORT nPntMax=rXP.GetPointCount();
             if (nPntMax!=0) {
                 nPntMax--;
                 if (rXP[0]!=rXP[nPntMax]) {
                     rXP[nPntMax+1]=Point(rXP[0]);
                 }
             }
        }
        bClosedObj=TRUE;
    } else {
        switch (eKind) {
            case OBJ_POLY    : eKind=OBJ_PLIN;     break;
            case OBJ_PATHFILL: eKind=OBJ_PATHLINE; break;
            case OBJ_FREEFILL: eKind=OBJ_FREELINE; break;
            case OBJ_SPLNFILL: eKind=OBJ_SPLNLINE; break;
        }
        bClosedObj=FALSE;
    }
    ImpForceKind();
}

void SdrPathObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bNoContortion=FALSE;

    FASTBOOL bCanConv = !HasText() || ImpCanConvTextToCurve();
    FASTBOOL bIsPath = IsBezier() || IsSpline();

    rInfo.bCanConvToPath = bCanConv && !bIsPath;
    rInfo.bCanConvToPoly = bCanConv && bIsPath;
    rInfo.bCanConvToContour = !IsFontwork() && (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

UINT16 SdrPathObj::GetObjIdentifier() const
{
    return USHORT(eKind);
}

void SdrPathObj::RecalcBoundRect()
{
    aOutRect=GetSnapRect();
    long nLineWdt=ImpGetLineWdt();
    if (!IsClosed()) { // ggf. Linienenden beruecksichtigen
        long nLEndWdt=ImpGetLineEndAdd();
        if (nLEndWdt>nLineWdt) nLineWdt=nLEndWdt;
    }
    if (nLineWdt!=0) {
        aOutRect.Left  ()-=nLineWdt;
        aOutRect.Top   ()-=nLineWdt;
        aOutRect.Right ()+=nLineWdt;
        aOutRect.Bottom()+=nLineWdt;
    }
    ImpAddShadowToBoundRect();
    ImpAddTextToBoundRect();
}

FASTBOOL SdrPathObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    BOOL bHideContour(IsHideContour());
    BOOL bIsFillDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTFILL));
    BOOL bIsLineDraft(0 != (rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTLINE));

    // prepare ItemSet of this object
    const SfxItemSet& rSet = GetItemSet();
//-/    SfxItemSet aSet((SfxItemPool&)(*GetItemPool()));
//-/    TakeAttributes(aSet, FALSE, TRUE);

    // perepare ItemSet to avoid old XOut line drawing
//-/    XLineAttrSetItem aXLSet(rSet.GetPool());
    SfxItemSet aEmptySet(*rSet.GetPool());
    aEmptySet.Put(XLineStyleItem(XLINE_NONE));
    aEmptySet.Put(XFillStyleItem(XFILL_NONE));

    // prepare line geometry
    ImpLineGeometry* pLineGeometry = ImpPrepareLineGeometry(rXOut, rSet, bIsLineDraft);

    // Shadows
    if (!bHideContour && ImpSetShadowAttributes(rXOut,!IsClosed()))
    {
        UINT32 nXDist=((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        UINT32 nYDist=((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
        XPolyPolygon aTmpXPoly(aPathPolygon);
        aTmpXPoly.Move(nXDist,nYDist);

        // avoid shadow line drawing in XOut
        rXOut.SetLineAttr(aEmptySet);

        if (!IsClosed()) {
            USHORT nPolyAnz=aTmpXPoly.Count();
            for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
                rXOut.DrawXPolyLine(aTmpXPoly.GetObject(nPolyNum));
            }
        } else {
            rXOut.DrawXPolyPolygon(aTmpXPoly);
        }

        // new shadow line drawing
        if(pLineGeometry)
        {
            // draw the line geometry
            ImpDrawShadowLineGeometry(rXOut, rSet, *pLineGeometry);
        }
    }

    // Before here the LineAttr were set: if(pLineAttr) rXOut.SetLineAttr(*pLineAttr);
    // avoid line drawing in XOut
    rXOut.SetLineAttr(aEmptySet);

    if(bIsFillDraft)
    {
        // perepare ItemSet to avoid XOut filling
        rXOut.SetFillAttr(aEmptySet);
    }
    else
    {
        if(IsClosed() || bHideContour)
        {
            rXOut.SetFillAttr(rSet);
        }
    }

    if (!bHideContour) {
        if (!IsClosed()) {
            USHORT nPolyAnz=aPathPolygon.Count();
            for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
                rXOut.DrawXPolyLine(aPathPolygon.GetObject(nPolyNum));
            }
        } else {
            rXOut.DrawXPolyPolygon(aPathPolygon);
        }
    }

    // Own line drawing
    if(!bHideContour && pLineGeometry)
    {
        // draw the line geometry
        ImpDrawColorLineGeometry(rXOut, rSet, *pLineGeometry);
    }

    FASTBOOL bOk=TRUE;
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

SdrObject* SdrPathObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    INT32 nMyTol=nTol;
    FASTBOOL bFilled=IsClosed() && (bTextFrame || HasFill());

    INT32 nWdt=ImpGetLineWdt()/2; // Halbe Strichstaerke
    if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
    Rectangle aR(rPnt,rPnt);
    aR.Left()  -=nMyTol;
    aR.Right() +=nMyTol;
    aR.Top()   -=nMyTol;
    aR.Bottom()+=nMyTol;

    FASTBOOL bHit=FALSE;
    unsigned nPolyAnz=aPathPolygon.Count();
    if (bFilled) {
        PolyPolygon aPP;
        for (unsigned nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
            aPP.Insert(XOutCreatePolygon(aPathPolygon[nPolyNum],NULL));
        }
        bHit=IsRectTouchesPoly(aPP,aR);
    } else {
        for (unsigned nPolyNum=0; nPolyNum<nPolyAnz && !bHit; nPolyNum++) {
            Polygon aPoly(XOutCreatePolygon(aPathPolygon[nPolyNum],NULL));
            bHit=IsRectTouchesLine(aPoly,aR);
        }
    }
    if (!bHit && !IsTextFrame() && HasText()) {
        bHit=SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
    }
    return bHit ? (SdrObject*)this : NULL;
}

void SdrPathObj::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=(rObj);
    SdrPathObj& rPath=(SdrPathObj&)rObj;
    aPathPolygon=rPath.aPathPolygon;
}

void SdrPathObj::TakeObjNameSingul(XubString& rName) const
{
    if (eKind==OBJ_LINE) {
        USHORT nId=STR_ObjNameSingulLINE;
        if (aPathPolygon.Count()==1 && aPathPolygon[0].GetPointCount()==2) {
            Point aP1(aPathPolygon[0][0]);
            Point aP2(aPathPolygon[0][1]);
            if (aP1!=aP2) {
                if (aP1.Y()==aP2.Y()) {
                    nId=STR_ObjNameSingulLINE_Hori;
                } else if (aP1.X()==aP2.X()) {
                    nId=STR_ObjNameSingulLINE_Vert;
                } else {
                    long dx=aP1.X()-aP2.X(); dx=Abs(dx);
                    long dy=aP1.Y()-aP2.Y(); dy=Abs(dy);
                    if (dx==dy) {
                        nId=STR_ObjNameSingulLINE_Diag;
                    }
                }
            }
        }
        rName=ImpGetResStr(nId);
    } else if (eKind==OBJ_PLIN || eKind==OBJ_POLY) {
        FASTBOOL bClosed=eKind==OBJ_POLY;
        USHORT nId=0;
        if (bCreating) { // z.Zt. Create
            if (bClosed) {
                nId=STR_ObjNameSingulPOLY;
            } else {
                nId=STR_ObjNameSingulPLIN;
            }
            rName=ImpGetResStr(nId);
        } else {                // sonst Punkteanzahl bestimmen
            ULONG nPntAnz=0;
            USHORT nPolyAnz=aPathPolygon.Count();
            for (USHORT nPoly=0; nPoly<nPolyAnz; nPoly++) {
                USHORT n=aPathPolygon[nPoly].GetPointCount();
                if (n>1 && bClosed) n--;
                nPntAnz+=n;
            }
            if (bClosed) {
                nId=STR_ObjNameSingulPOLY_PntAnz;
            } else {
                nId=STR_ObjNameSingulPLIN_PntAnz;
            }

            rName = ImpGetResStr(nId);
            UINT16 nPos = rName.SearchAscii("%N");

            if(nPos != STRING_NOTFOUND)
            {
                rName.Erase(nPos, 2);
                rName.Insert(UniString::CreateFromInt32(nPntAnz), nPos);
            }
        }
    } else {
        switch (eKind) {
            case OBJ_PATHLINE: rName=ImpGetResStr(STR_ObjNameSingulPATHLINE); break;
            case OBJ_FREELINE: rName=ImpGetResStr(STR_ObjNameSingulFREELINE); break;
            case OBJ_SPLNLINE: rName=ImpGetResStr(STR_ObjNameSingulNATSPLN); break;
            case OBJ_PATHFILL: rName=ImpGetResStr(STR_ObjNameSingulPATHFILL); break;
            case OBJ_FREEFILL: rName=ImpGetResStr(STR_ObjNameSingulFREEFILL); break;
            case OBJ_SPLNFILL: rName=ImpGetResStr(STR_ObjNameSingulPERSPLN); break;
        }
    }
}

void SdrPathObj::TakeObjNamePlural(XubString& rName) const
{
    switch (eKind) {
        case OBJ_LINE    : rName=ImpGetResStr(STR_ObjNamePluralLINE    ); break;
        case OBJ_PLIN    : rName=ImpGetResStr(STR_ObjNamePluralPLIN    ); break;
        case OBJ_POLY    : rName=ImpGetResStr(STR_ObjNamePluralPOLY    ); break;
        case OBJ_PATHLINE: rName=ImpGetResStr(STR_ObjNamePluralPATHLINE); break;
        case OBJ_FREELINE: rName=ImpGetResStr(STR_ObjNamePluralFREELINE); break;
        case OBJ_SPLNLINE: rName=ImpGetResStr(STR_ObjNamePluralNATSPLN); break;
        case OBJ_PATHFILL: rName=ImpGetResStr(STR_ObjNamePluralPATHFILL); break;
        case OBJ_FREEFILL: rName=ImpGetResStr(STR_ObjNamePluralFREEFILL); break;
        case OBJ_SPLNFILL: rName=ImpGetResStr(STR_ObjNamePluralPERSPLN); break;
    }
}

void SdrPathObj::TakeXorPoly(XPolyPolygon& rXPolyPoly, FASTBOOL bDetail) const
{
    rXPolyPoly=aPathPolygon;
}

void SdrPathObj::TakeContour(XPolyPolygon& rPoly) const
{
    // am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
    SdrTextObj::TakeContour(rPoly);
}

void SdrPathObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

USHORT SdrPathObj::GetHdlCount() const
{
    USHORT i,j;
    USHORT nCnt=0;
    USHORT nPolyCnt=aPathPolygon.Count();
    FASTBOOL bClosed=IsClosed();

    for (i=0; i<nPolyCnt; i++) {
        const XPolygon& rXPoly=aPathPolygon[i];
        USHORT nPntCnt=rXPoly.GetPointCount();
        // Polygon geschlossen? Dann Endpunkt = Startpunkt
        if (bClosed && nPntCnt>1) nPntCnt--;

        for (j=0; j<nPntCnt; j++)
            if (rXPoly.GetFlags(j)!=XPOLY_CONTROL) nCnt++;
    }
    return nCnt;
}

SdrHdl* SdrPathObj::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pHdl=NULL;
    USHORT  nPoly,nPnt;

    if (FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE)) {
        pHdl=new SdrHdl(aPathPolygon[nPoly][nPnt],HDL_POLY);
        pHdl->SetPolyNum(nPoly);
        pHdl->SetPointNum(nPnt);
        pHdl->SetSourceHdlNum(nHdlNum);
        pHdl->Set1PixMore(nPnt==0);
    }
    return pHdl;
}

void SdrPathObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    USHORT nCnt=GetHdlCount();
    USHORT nPolyCnt=aPathPolygon.Count();
    FASTBOOL bClosed=IsClosed();
    USHORT nIdx=0;

    for (USHORT i=0; i<nPolyCnt; i++) {
        const XPolygon& rXPoly=aPathPolygon.GetObject(i);
        USHORT nPntCnt=rXPoly.GetPointCount();
        if (bClosed && nPntCnt>1) nPntCnt--;

        for (USHORT j=0; j<nPntCnt; j++) {
            if (rXPoly.GetFlags(j)!=XPOLY_CONTROL) {
                const Point& rPnt=rXPoly[j];
                SdrHdl* pHdl=new SdrHdl(rPnt,HDL_POLY);
                pHdl->SetPolyNum(i);
                pHdl->SetPointNum(j);
                pHdl->Set1PixMore(j==0);
                pHdl->SetSourceHdlNum(nIdx);
                nIdx++;
                rHdlList.AddHdl(pHdl);
            }
        }
    }
}

USHORT SdrPathObj::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    USHORT nCnt=0;
    USHORT nPnt=rHdl.GetPointNum();
    USHORT nPolyNum=rHdl.GetPolyNum();
    if (nPolyNum<aPathPolygon.Count()) {
        const XPolygon& rXPoly=aPathPolygon[nPolyNum];
        USHORT nPntMax=rXPoly.GetPointCount();
        if (nPntMax>0) {
            nPntMax--;
            if (nPnt<=nPntMax) {
                if (rXPoly.GetFlags(nPnt)!=XPOLY_CONTROL) {
                    if (nPnt==0 && IsClosed()) nPnt=nPntMax;
                    if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL) nCnt++;
                    if (nPnt==nPntMax && IsClosed()) nPnt=0;
                    if (nPnt<nPntMax && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL) nCnt++;
                }
            }
        }
    }
    return nCnt;
}

SdrHdl* SdrPathObj::GetPlusHdl(const SdrHdl& rHdl, USHORT nPlusNum) const
{
    SdrHdl* pHdl=NULL;
    USHORT nPnt=rHdl.GetPointNum();
    USHORT nPolyNum=rHdl.GetPolyNum();
    if (nPolyNum<aPathPolygon.Count()) {
        const XPolygon& rXPoly=aPathPolygon[nPolyNum];
        USHORT nPntMax=rXPoly.GetPointCount();
        if (nPntMax>0) {
            nPntMax--;
            if (nPnt<=nPntMax) {
                pHdl=new SdrHdlBezWgt(&rHdl);
                pHdl->SetPolyNum(rHdl.GetPolyNum());

                if (nPnt==0 && IsClosed()) nPnt=nPntMax;
                if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL && nPlusNum==0) {
                    pHdl->SetPos(rXPoly[nPnt-1]);
                    pHdl->SetPointNum(nPnt-1);
                } else {
                    if (nPnt==nPntMax && IsClosed()) nPnt=0;
                    if (nPnt<rXPoly.GetPointCount()-1 && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL) {
                        pHdl->SetPos(rXPoly[nPnt+1]);
                        pHdl->SetPointNum(nPnt+1);
                    }
                }

                pHdl->SetSourceHdlNum(rHdl.GetSourceHdlNum());
                pHdl->SetPlusHdl(TRUE);
            }
        }
    }
    return pHdl;
}

FASTBOOL SdrPathObj::HasSpecialDrag() const
{
    return TRUE;
}

inline USHORT GetPrevPnt(USHORT nPnt, USHORT nPntMax, FASTBOOL bClosed)
{
    if (nPnt>0) {
        nPnt--;
    } else {
        nPnt=nPntMax;
        if (bClosed) nPnt--;
    }
    return nPnt;
}

inline USHORT GetNextPnt(USHORT nPnt, USHORT nPntMax, FASTBOOL bClosed)
{
    nPnt++;
    if (nPnt>nPntMax || (bClosed && nPnt>=nPntMax)) nPnt=0;
    return nPnt;
}

class ImpSdrPathDragData
{
public:
    XPolygon                    aXP;            // Ausschnitt aud dem Originalpolygon
    FASTBOOL                    bValid;         // FALSE = zu wenig Punkte
    FASTBOOL                    bClosed;        // geschlossenes Objekt?
    USHORT                      nPoly;          // Nummer des Polygons im PolyPolygon
    USHORT                      nPnt;           // Punktnummer innerhalb des obigen Polygons
    USHORT                      nPntAnz;        // Punktanzahl des Polygons
    USHORT                      nPntMax;        // Maximaler Index
    FASTBOOL                    bBegPnt;        // Gedraggter Punkt ist der Anfangspunkt einer Polyline
    FASTBOOL                    bEndPnt;        // Gedraggter Punkt ist der Endpunkt einer Polyline
    USHORT                      nPrevPnt;       // Index des vorherigen Punkts
    USHORT                      nNextPnt;       // Index des naechsten Punkts
    FASTBOOL                    bPrevIsBegPnt;  // Vorheriger Punkt ist Anfangspunkt einer Polyline
    FASTBOOL                    bNextIsEndPnt;  // Folgepunkt ist Endpunkt einer Polyline
    USHORT                      nPrevPrevPnt;   // Index des vorvorherigen Punkts
    USHORT                      nNextNextPnt;   // Index des uebernaechsten Punkts
    FASTBOOL                    bControl;       // Punkt ist ein Kontrollpunkt
    FASTBOOL                    bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
    FASTBOOL                    bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
    FASTBOOL                    bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
    FASTBOOL                    bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
    USHORT                      nPrevPrevPnt0;
    USHORT                      nPrevPnt0;
    USHORT                      nPnt0;
    USHORT                      nNextPnt0;
    USHORT                      nNextNextPnt0;
    FASTBOOL                    bEliminate;     // Punkt loeschen? (wird von MovDrag gesetzt)

public:
    ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl);
    void ResetPoly(const SdrPathObj& rPO);
};

ImpSdrPathDragData::ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl):
    aXP(5)
{
    bValid=FALSE;
    bClosed=rPO.IsClosed();          // geschlossenes Objekt?
    nPoly=rHdl.GetPolyNum();            // Nummer des Polygons im PolyPolygon
    nPnt=rHdl.GetPointNum();            // Punktnummer innerhalb des obigen Polygons
    const XPolygon& rXP=rPO.aPathPolygon[nPoly];     // Referenz auf das Polygon
    nPntAnz=rXP.GetPointCount();        // Punktanzahl des Polygons
    if (nPntAnz==0 || (bClosed && nPntAnz==1)) return; // min. 1Pt bei Line, min. 2 bei Polygon
    nPntMax=nPntAnz-1;                  // Maximaler Index
    bBegPnt=!bClosed && nPnt==0;        // Gedraggter Punkt ist der Anfangspunkt einer Polyline
    bEndPnt=!bClosed && nPnt==nPntMax;  // Gedraggter Punkt ist der Endpunkt einer Polyline
    if (bClosed && nPntAnz<=3) {        // Falls Polygon auch nur eine Linie ist
        bBegPnt=(nPntAnz<3) || nPnt==0;
        bEndPnt=(nPntAnz<3) || nPnt==nPntMax-1;
    }
    nPrevPnt=nPnt;                      // Index des vorherigen Punkts
    nNextPnt=nPnt;                      // Index des naechsten Punkts
    if (!bBegPnt) nPrevPnt=GetPrevPnt(nPnt,nPntMax,bClosed);
    if (!bEndPnt) nNextPnt=GetNextPnt(nPnt,nPntMax,bClosed);
    bPrevIsBegPnt=bBegPnt || (!bClosed && nPrevPnt==0);
    bNextIsEndPnt=bEndPnt || (!bClosed && nNextPnt==nPntMax);
    nPrevPrevPnt=nPnt;                  // Index des vorvorherigen Punkts
    nNextNextPnt=nPnt;                  // Index des uebernaechsten Punkts
    if (!bPrevIsBegPnt) nPrevPrevPnt=GetPrevPnt(nPrevPnt,nPntMax,bClosed);
    if (!bNextIsEndPnt) nNextNextPnt=GetNextPnt(nNextPnt,nPntMax,bClosed);
    bControl=rHdl.IsPlusHdl();          // Punkt ist ein Kontrollpunkt
    bIsPrevControl=FALSE;               // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
    bIsNextControl=FALSE;               // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
    bPrevIsControl=FALSE;               // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
    bNextIsControl=FALSE;               // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
    if (bControl) {
        bIsPrevControl=rXP.IsControl(nPrevPnt);
        bIsNextControl=!bIsPrevControl;
    } else {
        bPrevIsControl=!bBegPnt && !bPrevIsBegPnt && rXP.GetFlags(nPrevPnt)==XPOLY_CONTROL;
        bNextIsControl=!bEndPnt && !bNextIsEndPnt && rXP.GetFlags(nNextPnt)==XPOLY_CONTROL;
    }
    nPrevPrevPnt0=nPrevPrevPnt;
    nPrevPnt0    =nPrevPnt;
    nPnt0        =nPnt;
    nNextPnt0    =nNextPnt;
    nNextNextPnt0=nNextNextPnt;
    nPrevPrevPnt=0;
    nPrevPnt=1;
    nPnt=2;
    nNextPnt=3;
    nNextNextPnt=4;
    bEliminate=FALSE;
    ResetPoly(rPO);
    bValid=TRUE;
}

void ImpSdrPathDragData::ResetPoly(const SdrPathObj& rPO)
{
    const XPolygon& rXP=rPO.aPathPolygon[nPoly];     // Referenz auf das Polygon
    aXP[0]=rXP[nPrevPrevPnt0];  aXP.SetFlags(0,rXP.GetFlags(nPrevPrevPnt0));
    aXP[1]=rXP[nPrevPnt0];      aXP.SetFlags(1,rXP.GetFlags(nPrevPnt0));
    aXP[2]=rXP[nPnt0];          aXP.SetFlags(2,rXP.GetFlags(nPnt0));
    aXP[3]=rXP[nNextPnt0];      aXP.SetFlags(3,rXP.GetFlags(nNextPnt0));
    aXP[4]=rXP[nNextNextPnt0];  aXP.SetFlags(4,rXP.GetFlags(nNextNextPnt0));
}

class ImpPathCreateUser
{
public:
    Point                   aBezControl0;
    Point                   aBezStart;
    Point                   aBezCtrl1;
    Point                   aBezCtrl2;
    Point                   aBezEnd;
    Point                   aCircStart;
    Point                   aCircEnd;
    Point                   aCircCenter;
    Point                   aLineStart;
    Point                   aLineEnd;
    Point                   aRectP1;
    Point                   aRectP2;
    Point                   aRectP3;
    long                    nCircRadius;
    long                    nCircStWink;
    long                    nCircRelWink;
    FASTBOOL                bBezier;
    FASTBOOL                bBezHasCtrl0;
    FASTBOOL                bCurve;
    FASTBOOL                bCircle;
    FASTBOOL                bAngleSnap;
    FASTBOOL                bLine;
    FASTBOOL                bLine90;
    FASTBOOL                bRect;
    FASTBOOL                bMixedCreate;
    USHORT                  nBezierStartPoint;
    SdrObjKind              eStartKind;
    SdrObjKind              eAktKind;

public:
    ImpPathCreateUser(): nCircRadius(0),nCircStWink(0),nCircRelWink(0),
        bBezier(FALSE),bBezHasCtrl0(FALSE),bCurve(FALSE),bCircle(FALSE),bAngleSnap(FALSE),bLine(FALSE),bLine90(FALSE),bRect(FALSE),
        bMixedCreate(FALSE),nBezierStartPoint(0),eStartKind(OBJ_NONE),eAktKind(OBJ_NONE) { }

    void ResetFormFlags() { bBezier=FALSE; bCurve=FALSE; bCircle=FALSE; bLine=FALSE; bRect=FALSE; }
    FASTBOOL IsFormFlag() const { return bBezier || bCurve || bCircle || bLine || bRect; }
    XPolygon GetFormPoly() const;
    FASTBOOL CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, FASTBOOL bMouseDown, SdrView* pView);
    XPolygon GetBezierPoly() const;
    FASTBOOL CalcCurve(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView) { return FALSE; }
    XPolygon GetCurvePoly() const { return XPolygon(); }
    FASTBOOL CalcCircle(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    XPolygon GetCirclePoly() const;
    FASTBOOL CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    Point    CalcLine(const Point& rCsr, long nDirX, long nDirY, SdrView* pView) const;
    XPolygon GetLinePoly() const;
    FASTBOOL CalcRect(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    XPolygon GetRectPoly() const;
};

XPolygon ImpPathCreateUser::GetFormPoly() const
{
    if (bBezier) return GetBezierPoly();
    if (bCurve)  return GetCurvePoly();
    if (bCircle) return GetCirclePoly();
    if (bLine)   return GetLinePoly();
    if (bRect)   return GetRectPoly();
    return XPolygon();
}

FASTBOOL ImpPathCreateUser::CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, FASTBOOL bMouseDown, SdrView* pView)
{
    FASTBOOL bRet=TRUE;
    aBezStart=rP1;
    aBezCtrl1=rP1+rDir;
    aBezCtrl2=rP2;
    if (!bMouseDown) aBezEnd=rP2;
    bBezier=bRet;
    return bRet;
}

XPolygon ImpPathCreateUser::GetBezierPoly() const
{
    XPolygon aXP(4);
    aXP[0]=aBezStart; aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aBezCtrl1; aXP.SetFlags(1,XPOLY_CONTROL);
    aXP[2]=aBezCtrl2; aXP.SetFlags(2,XPOLY_CONTROL);
    aXP[3]=aBezEnd;
    return aXP;
}

FASTBOOL ImpPathCreateUser::CalcCircle(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    long nTangAngle=GetAngle(rDir);
    aCircStart=rP1;
    aCircEnd=rP2;
    aCircCenter=rP1;
    long dx=rP2.X()-rP1.X();
    long dy=rP2.Y()-rP1.Y();
    long dAngle=GetAngle(Point(dx,dy))-nTangAngle;
    dAngle=NormAngle360(dAngle);
    long nTmpAngle=NormAngle360(9000-dAngle);
    FASTBOOL bRet=nTmpAngle!=9000 && nTmpAngle!=27000;
    long nRad=0;
    if (bRet) {
        double cs=cos(nTmpAngle*nPi180);
        double nR=(double)GetLen(Point(dx,dy))/cs/2;
        nRad=Abs(Round(nR));
    }
    if (dAngle<18000) {
        nCircStWink=NormAngle360(nTangAngle-9000);
        nCircRelWink=NormAngle360(2*dAngle);
        aCircCenter.X()+=Round(nRad*cos((nTangAngle+9000)*nPi180));
        aCircCenter.Y()-=Round(nRad*sin((nTangAngle+9000)*nPi180));
    } else {
        nCircStWink=NormAngle360(nTangAngle+9000);
        nCircRelWink=-NormAngle360(36000-2*dAngle);
        aCircCenter.X()+=Round(nRad*cos((nTangAngle-9000)*nPi180));
        aCircCenter.Y()-=Round(nRad*sin((nTangAngle-9000)*nPi180));
    }
    bAngleSnap=pView!=NULL && pView->IsAngleSnapEnabled();
    if (bAngleSnap) {
        long nSA=pView->GetSnapAngle();
        if (nSA!=0) { // Winkelfang
            FASTBOOL bNeg=nCircRelWink<0;
            if (bNeg) nCircRelWink=-nCircRelWink;
            nCircRelWink+=nSA/2;
            nCircRelWink/=nSA;
            nCircRelWink*=nSA;
            nCircRelWink=NormAngle360(nCircRelWink);
            if (bNeg) nCircRelWink=-nCircRelWink;
        }
    }
    nCircRadius=nRad;
    if (nRad==0 || Abs(nCircRelWink)<5) bRet=FALSE;
    bCircle=bRet;
    return bRet;
}

XPolygon ImpPathCreateUser::GetCirclePoly() const
{
    if (nCircRelWink>=0) {
        XPolygon aXP(aCircCenter,nCircRadius,nCircRadius,
                     USHORT((nCircStWink+5)/10),USHORT((nCircStWink+nCircRelWink+5)/10),FALSE);
        aXP[0]=aCircStart; aXP.SetFlags(0,XPOLY_SMOOTH);
        if (!bAngleSnap) aXP[aXP.GetPointCount()-1]=aCircEnd;
        return aXP;
    } else {
        XPolygon aXP(aCircCenter,nCircRadius,nCircRadius,
                     USHORT(NormAngle360(nCircStWink+nCircRelWink+5)/10),USHORT((nCircStWink+5)/10),FALSE);
        USHORT nAnz=aXP.GetPointCount();
        for (USHORT nNum=nAnz/2; nNum>0;) {
            nNum--; // XPoly Punktreihenfolge umkehren
            USHORT n2=nAnz-nNum-1;
            Point aPt(aXP[nNum]);
            aXP[nNum]=aXP[n2];
            aXP[n2]=aPt;
        }
        aXP[0]=aCircStart; aXP.SetFlags(0,XPOLY_SMOOTH);
        if (!bAngleSnap) aXP[aXP.GetPointCount()-1]=aCircEnd;
        return aXP;
    }
}

Point ImpPathCreateUser::CalcLine(const Point& aCsr, long nDirX, long nDirY, SdrView* pView) const
{
    long x=aCsr.X(),x1=x,x2=x;
    long y=aCsr.Y(),y1=y,y2=y;
    FASTBOOL bHLin=nDirY==0;
    FASTBOOL bVLin=nDirX==0;
    if (bHLin) y=0;
    else if (bVLin) x=0;
    else {
        x1=BigMulDiv(y,nDirX,nDirY);
        y2=BigMulDiv(x,nDirY,nDirX);
        long l1=Abs(x1)+Abs(y1);
        long l2=Abs(x2)+Abs(y2);
        if (l1<=l2 !=(pView!=NULL && pView->IsBigOrtho())) {
            x=x1; y=y1;
        } else {
            x=x2; y=y2;
        }
    }
    return Point(x,y);
}

FASTBOOL ImpPathCreateUser::CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    aLineStart=rP1;
    aLineEnd=rP2;
    bLine90=FALSE;
    if (rP1==rP2 || (rDir.X()==0 && rDir.Y()==0)) { bLine=FALSE; return FALSE; }
    Point aTmpPt(rP2-rP1);
    long nDirX=rDir.X();
    long nDirY=rDir.Y();
    Point aP1(CalcLine(aTmpPt, nDirX, nDirY,pView)); aP1-=aTmpPt; long nQ1=Abs(aP1.X())+Abs(aP1.Y());
    Point aP2(CalcLine(aTmpPt, nDirY,-nDirX,pView)); aP2-=aTmpPt; long nQ2=Abs(aP2.X())+Abs(aP2.Y());
    if (pView!=NULL && pView->IsOrtho()) nQ1=0; // Ortho schaltet rechtwinklig aus
    bLine90=nQ1>2*nQ2;
    if (!bLine90) { // glatter Uebergang
        aLineEnd+=aP1;
    } else {          // rechtwinkliger Uebergang
        aLineEnd+=aP2;
    }
    bLine=TRUE;
    return TRUE;
}

XPolygon ImpPathCreateUser::GetLinePoly() const
{
    XPolygon aXP(2);
    aXP[0]=aLineStart; if (!bLine90) aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aLineEnd;
    return aXP;
}

FASTBOOL ImpPathCreateUser::CalcRect(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    aRectP1=rP1;
    aRectP2=rP1;
    aRectP3=rP2;
    if (rP1==rP2 || (rDir.X()==0 && rDir.Y()==0)) { bRect=FALSE; return FALSE; }
    Point aTmpPt(rP2-rP1);
    long nDirX=rDir.X();
    long nDirY=rDir.Y();
    long x=aTmpPt.X();
    long y=aTmpPt.Y();
    FASTBOOL bHLin=nDirY==0;
    FASTBOOL bVLin=nDirX==0;
    if (bHLin) y=0;
    else if (bVLin) x=0;
    else {
        y=BigMulDiv(x,nDirY,nDirX);
        long nHypLen=aTmpPt.Y()-y;
        long nTangAngle=-GetAngle(rDir);
        // sin=g/h, g=h*sin
        double a=nTangAngle*nPi180;
        double sn=sin(a);
        double cs=cos(a);
        double nGKathLen=nHypLen*sn;
        y+=Round(nGKathLen*sn);
        x+=Round(nGKathLen*cs);
    }
    aRectP2.X()+=x;
    aRectP2.Y()+=y;
    if (pView!=NULL && pView->IsOrtho()) {
        long dx1=aRectP2.X()-aRectP1.X(); long dx1a=Abs(dx1);
        long dy1=aRectP2.Y()-aRectP1.Y(); long dy1a=Abs(dy1);
        long dx2=aRectP3.X()-aRectP2.X(); long dx2a=Abs(dx2);
        long dy2=aRectP3.Y()-aRectP2.Y(); long dy2a=Abs(dy2);
        FASTBOOL b1MoreThan2=dx1a+dy1a>dx2a+dy2a;
        if (b1MoreThan2 != pView->IsBigOrtho()) {
            long xtemp=dy2a-dx1a; if (dx1<0) xtemp=-xtemp;
            long ytemp=dx2a-dy1a; if (dy1<0) ytemp=-ytemp;
            aRectP2.X()+=xtemp;
            aRectP2.Y()+=ytemp;
            aRectP3.X()+=xtemp;
            aRectP3.Y()+=ytemp;
        } else {
            long xtemp=dy1a-dx2a; if (dx2<0) xtemp=-xtemp;
            long ytemp=dx1a-dy2a; if (dy2<0) ytemp=-ytemp;
            aRectP3.X()+=xtemp;
            aRectP3.Y()+=ytemp;
        }
    }
    bRect=TRUE;
    return TRUE;
}

XPolygon ImpPathCreateUser::GetRectPoly() const
{
    XPolygon aXP(3);
    aXP[0]=aRectP1; aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aRectP2;
    if (aRectP3!=aRectP2) aXP[2]=aRectP3;
    return aXP;
}

FASTBOOL SdrPathObj::BegDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pHdl==NULL) return FALSE;
    ImpSdrPathDragData* pID=new ImpSdrPathDragData(*this,*pHdl);
    if (!pID->bValid) {
        DBG_ERROR("SdrPathObj::BegDrag(): ImpSdrPathDragData ist ungueltig");
        delete pID;
        return FALSE;
    }
    rDrag.SetUser(pID);
    return TRUE;
}

FASTBOOL SdrPathObj::MovDrag(SdrDragStat& rDrag) const
{
    ImpSdrPathDragData* pID=(ImpSdrPathDragData*)rDrag.GetUser();
    if (pID==NULL || !pID->bValid) {
        DBG_ERROR("SdrPathObj::MovDrag(): ImpSdrPathDragData ist ungueltig");
        return FALSE;
    }
    pID->ResetPoly(*this);

    // Div. Daten lokal Kopieren fuer weniger Code und schnelleren Zugriff
    FASTBOOL bClosed       =pID->bClosed       ; // geschlossenes Objekt?
    USHORT   nPnt          =pID->nPnt          ; // Punktnummer innerhalb des obigen Polygons
    FASTBOOL bBegPnt       =pID->bBegPnt       ; // Gedraggter Punkt ist der Anfangspunkt einer Polyline
    FASTBOOL bEndPnt       =pID->bEndPnt       ; // Gedraggter Punkt ist der Endpunkt einer Polyline
    USHORT   nPrevPnt      =pID->nPrevPnt      ; // Index des vorherigen Punkts
    USHORT   nNextPnt      =pID->nNextPnt      ; // Index des naechsten Punkts
    FASTBOOL bPrevIsBegPnt =pID->bPrevIsBegPnt ; // Vorheriger Punkt ist Anfangspunkt einer Polyline
    FASTBOOL bNextIsEndPnt =pID->bNextIsEndPnt ; // Folgepunkt ist Endpunkt einer Polyline
    USHORT   nPrevPrevPnt  =pID->nPrevPrevPnt  ; // Index des vorvorherigen Punkts
    USHORT   nNextNextPnt  =pID->nNextNextPnt  ; // Index des uebernaechsten Punkts
    FASTBOOL bControl      =pID->bControl      ; // Punkt ist ein Kontrollpunkt
    FASTBOOL bIsPrevControl=pID->bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
    FASTBOOL bIsNextControl=pID->bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
    FASTBOOL bPrevIsControl=pID->bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
    FASTBOOL bNextIsControl=pID->bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt

    // Ortho bei Linien/Polygonen = Winkel beibehalten
    if (!bControl && rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho()) {
        FASTBOOL bBigOrtho=rDrag.GetView()->IsBigOrtho();
        Point  aPos(rDrag.GetNow());      // die aktuelle Position
        Point  aPnt(pID->aXP[nPnt]);      // der gedraggte Punkt
        USHORT nPnt1=0xFFFF,nPnt2=0xFFFF; // seine Nachbarpunkte
        Point  aNeuPos1,aNeuPos2;         // die neuen Alternativen fuer aPos
        FASTBOOL bPnt1=FALSE,bPnt2=FALSE; // die neuen Alternativen gueltig?
        if (!bClosed && pID->nPntAnz>=2) { // Mind. 2 Pt bei Linien
            if (!bBegPnt) nPnt1=nPrevPnt;
            if (!bEndPnt) nPnt2=nNextPnt;
        }
        if (bClosed && pID->nPntAnz>=3) { // Mind. 3 Pt bei Polygon
            nPnt1=nPrevPnt;
            nPnt2=nNextPnt;
        }
        if (nPnt1!=0xFFFF && !bPrevIsControl) {
            Point aPnt1=pID->aXP[nPnt1];
            long ndx0=aPnt.X()-aPnt1.X();
            long ndy0=aPnt.Y()-aPnt1.Y();
            FASTBOOL bHLin=ndy0==0;
            FASTBOOL bVLin=ndx0==0;
            if (!bHLin || !bVLin) {
                long ndx=aPos.X()-aPnt1.X();
                long ndy=aPos.Y()-aPnt1.Y();
                bPnt1=TRUE;
                double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                FASTBOOL bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                FASTBOOL bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                if (bHor) ndy=long(ndy0*nXFact);
                if (bVer) ndx=long(ndx0*nYFact);
                aNeuPos1=aPnt1;
                aNeuPos1.X()+=ndx;
                aNeuPos1.Y()+=ndy;
            }
        }
        if (nPnt2!=0xFFFF && !bNextIsControl) {
            Point aPnt2=pID->aXP[nPnt2];
            long ndx0=aPnt.X()-aPnt2.X();
            long ndy0=aPnt.Y()-aPnt2.Y();
            FASTBOOL bHLin=ndy0==0;
            FASTBOOL bVLin=ndx0==0;
            if (!bHLin || !bVLin) {
                long ndx=aPos.X()-aPnt2.X();
                long ndy=aPos.Y()-aPnt2.Y();
                bPnt2=TRUE;
                double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                FASTBOOL bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                FASTBOOL bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                if (bHor) ndy=long(ndy0*nXFact);
                if (bVer) ndx=long(ndx0*nYFact);
                aNeuPos2=aPnt2;
                aNeuPos2.X()+=ndx;
                aNeuPos2.Y()+=ndy;
            }
        }
        if (bPnt1 && bPnt2) { // beide Alternativen vorhanden (Konkurenz)
            BigInt nX1(aNeuPos1.X()-aPos.X()); nX1*=nX1;
            BigInt nY1(aNeuPos1.Y()-aPos.Y()); nY1*=nY1;
            BigInt nX2(aNeuPos2.X()-aPos.X()); nX2*=nX2;
            BigInt nY2(aNeuPos2.Y()-aPos.Y()); nY2*=nY2;
            nX1+=nY1; // Korrekturabstand zum Quadrat
            nX2+=nY2; // Korrekturabstand zum Quadrat
            // Die Alternative mit dem geringeren Korrekturbedarf gewinnt
            if (nX1<nX2) bPnt2=FALSE; else bPnt1=FALSE;
        }
        if (bPnt1) rDrag.Now()=aNeuPos1;
        if (bPnt2) rDrag.Now()=aNeuPos2;
    }
    rDrag.SetActionRect(Rectangle(rDrag.GetNow(),rDrag.GetNow()));

    // IBM Special: Punkte eliminieren, wenn die beiden angrenzenden
    //              Linien eh' fast 180 deg sind.
    if (!bControl && rDrag.GetView()!=NULL && rDrag.GetView()->IsEliminatePolyPoints() &&
        !bBegPnt && !bEndPnt && !bPrevIsControl && !bNextIsControl)
    {
        Point aPt(pID->aXP[nNextPnt]);
        aPt-=rDrag.GetNow();
        long nWink1=GetAngle(aPt);
        aPt=rDrag.GetNow();
        aPt-=pID->aXP[nPrevPnt];
        long nWink2=GetAngle(aPt);
        long nDiff=nWink1-nWink2;
        nDiff=Abs(nDiff);
        pID->bEliminate=nDiff<=rDrag.GetView()->GetEliminatePolyPointLimitAngle();
        if (pID->bEliminate) { // Position anpassen, damit Smooth an den Enden stimmt
            aPt=pID->aXP[nNextPnt];
            aPt+=pID->aXP[nPrevPnt];
            aPt/=2;
            rDrag.Now()=aPt;
        }
    }

    // Um diese Entfernung wurde insgesamt gedraggd
    Point aDiff(rDrag.GetNow()); aDiff-=pID->aXP[nPnt];

    // Insgesamt sind 8 Faelle moeglich:
    //    X      1. Weder rechts noch links Ctrl.
    // o--X--o   2. Rechts und links Ctrl, gedraggd wird St.
    // o--X      3. Nur links Ctrl, gedraggd wird St.
    //    X--o   4. Nur rechts Ctrl, gedraggd wird St.
    // x--O--o   5. Rechts und links Ctrl, gedraggd wird links.
    // x--O      6. Nur links Ctrl, gedraggd wird links.
    // o--O--x   7. Rechts und links Ctrl, gedraggd wird rechts.
    //    O--x   8. Nur rechts Ctrl, gedraggd wird rechts.
    // Zusaetzlich ist zu beachten, dass das Veraendern einer Linie (keine Kurve)
    // eine evtl. Kurve am anderen Ende der Linie bewirkt, falls dort Smooth
    // gesetzt ist (Kontrollpunktausrichtung an Gerade).

    pID->aXP[nPnt]+=aDiff; // <<<<<<<<<<

    // Nun symmetrische PlusHandles etc. checken
    if (bControl) { // Faelle 5,6,7,8
        USHORT   nSt=nPnt;   // der zugehoerige Stuetzpunkt
        USHORT   nFix=nPnt;  // der gegenueberliegende Kontrollpunkt
        if (bIsNextControl) { // Wenn der naechste ein Kontrollpunkt ist, muss der vorh. der Stuetzpunkt sein
            nSt=nPrevPnt;
            nFix=nPrevPrevPnt;
        } else {
            nSt=nNextPnt;
            nFix=nNextNextPnt;
        }
        if (pID->aXP.IsSmooth(nSt)) {
            pID->aXP.CalcSmoothJoin(nSt,nPnt,nFix); // <<<<<<<<<<
        }
    }

    if (!bControl) { // Faelle 1,2,3,4 wobei bei 1 nix passiert und bei 3+4 unten noch mehr folgt
        // die beiden Kontrollpunkte mit verschieben
        if (bPrevIsControl) pID->aXP[nPrevPnt]+=aDiff; // <<<<<<<<<<
        if (bNextIsControl) pID->aXP[nNextPnt]+=aDiff; // <<<<<<<<<<
        // Kontrollpunkt ggf. an Gerade ausrichten
        if (pID->aXP.IsSmooth(nPnt)) {
            if (bPrevIsControl && !bNextIsControl && !bEndPnt) { // Fall 3
                pID->aXP.CalcSmoothJoin(nPnt,nNextPnt,nPrevPnt); // <<<<<<<<<<
            }
            if (bNextIsControl && !bPrevIsControl && !bBegPnt) { // Fall 4
                pID->aXP.CalcSmoothJoin(nPnt,nPrevPnt,nNextPnt); // <<<<<<<<<<
            }
        }
        // Und nun noch die anderen Enden der Strecken ueberpruefen (nPnt+-1).
        // Ist dort eine Kurve (IsControl(nPnt+-2)) mit SmoothJoin (nPnt+-1),
        // so muss der entsprechende Kontrollpunkt (nPnt+-2) angepasst werden.
        if (!bBegPnt && !bPrevIsControl && !bPrevIsBegPnt && pID->aXP.IsSmooth(nPrevPnt)) {
            if (pID->aXP.IsControl(nPrevPrevPnt)) {
                pID->aXP.CalcSmoothJoin(nPrevPnt,nPnt,nPrevPrevPnt); // <<<<<<<<<<
            }
        }
        if (!bEndPnt && !bNextIsControl && !bNextIsEndPnt && pID->aXP.IsSmooth(nNextPnt)) {
            if (pID->aXP.IsControl(nNextNextPnt)) {
                pID->aXP.CalcSmoothJoin(nNextPnt,nPnt,nNextNextPnt); // <<<<<<<<<<
            }
        }
    }

    return TRUE;
}

FASTBOOL SdrPathObj::EndDrag(SdrDragStat& rDrag)
{
    Point aLinePt1;
    Point aLinePt2;
    FASTBOOL bLineGlueMirror=eKind==OBJ_LINE;
    if (bLineGlueMirror) { // #40549#
        XPolygon& rXP=aPathPolygon[0];
        aLinePt1=rXP[0];
        aLinePt2=rXP[1];
    }
    ImpSdrPathDragData* pID=(ImpSdrPathDragData*)rDrag.GetUser();
    const SdrHdl* pHdl=rDrag.GetHdl();
    if (pID==NULL || !pID->bValid) {
        DBG_ERROR("SdrPathObj::EndDrag(): ImpSdrPathDragData ist ungueltig");
        return FALSE;
    }
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    // Referenz auf das Polygon
    XPolygon& rXP=aPathPolygon[pHdl->GetPolyNum()];

    // Die 5 Punkte die sich evtl. geaendert haben
    if (!pID->bPrevIsBegPnt) rXP[pID->nPrevPrevPnt0]=pID->aXP[pID->nPrevPrevPnt];
    if (!pID->bNextIsEndPnt) rXP[pID->nNextNextPnt0]=pID->aXP[pID->nNextNextPnt];
    if (!pID->bBegPnt)       rXP[pID->nPrevPnt0]    =pID->aXP[pID->nPrevPnt];
    if (!pID->bEndPnt)       rXP[pID->nNextPnt0]    =pID->aXP[pID->nNextPnt];
                             rXP[pID->nPnt0]        =pID->aXP[pID->nPnt];

    // Letzter Punkt muss beim Geschlossenen immer gleich dem Ersten sein
    if (pID->bClosed) rXP[rXP.GetPointCount()-1]=rXP[0];
    if (pID->bEliminate) {
        NbcDelPoint(rDrag.GetHdl()->GetSourceHdlNum());
    }
    ImpForceKind(); // Wg. impliziter Punktloeschung evtl. von PolyLine nach Line
    // Winkel anpassen fuer Text an einfacher Linie
    SetRectsDirty();
    if (bLineGlueMirror) { // #40549#
        XPolygon& rXP=aPathPolygon[0];
        Point aLinePt1_(rXP[0]);
        Point aLinePt2_(rXP[1]);
        FASTBOOL bXMirr=(aLinePt1_.X()>aLinePt2_.X())!=(aLinePt1.X()>aLinePt2.X());
        FASTBOOL bYMirr=(aLinePt1_.Y()>aLinePt2_.Y())!=(aLinePt1.Y()>aLinePt2.Y());
        if (bXMirr || bYMirr) {
            Point aRef1(GetSnapRect().Center());
            if (bXMirr) {
                Point aRef2(aRef1);
                aRef2.Y()++;
                NbcMirrorGluePoints(aRef1,aRef2);
            }
            if (bYMirr) {
                Point aRef2(aRef1);
                aRef2.X()++;
                NbcMirrorGluePoints(aRef1,aRef2);
            }
        }
    }
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    delete pID;
    rDrag.SetUser(NULL);
    return TRUE;
}

void SdrPathObj::BrkDrag(SdrDragStat& rDrag) const
{
    ImpSdrPathDragData* pID=(ImpSdrPathDragData*)rDrag.GetUser();
    if (pID!=NULL) {
        delete pID;
        rDrag.SetUser(NULL);
    }
}

XubString SdrPathObj::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
{
    ImpSdrPathDragData* pID = (ImpSdrPathDragData*)rDrag.GetUser();

    if(!pID || !pID->bValid)
        return String();

    // Hier auch mal pID verwenden !!!
    XubString aStr;

    if(!bCreateComment)
    {
        const SdrHdl* pHdl = rDrag.GetHdl();

        if(bUndoDragComment || !pModel || !pHdl)
        {
            ImpTakeDescriptionStr(STR_DragPathObj, aStr);
        }
        else
        {
            if(pID->bEliminate)
            {
                // Punkt von ...
                ImpTakeDescriptionStr(STR_ViewMarkedPoint, aStr);

                // %O loeschen
                XubString aStr2(ImpGetResStr(STR_EditDelete));

                // UNICODE: Punkt von ... loeschen
                aStr2.SearchAndReplaceAscii("%O", aStr);

                return aStr2;
            }

            // dx=0.00 dy=0.00                // Beide Seiten Bezier
            // dx=0.00 dy=0.00  l=0.00 0.00ø  // Anfang oder Ende oder eine Seite Bezier bzw. Hebel
            // dx=0.00 dy=0.00  l=0.00 0.00ø / l=0.00 0.00ø   // Mittendrin
            XubString aMetr;
            Point aBeg(rDrag.GetStart());
            Point aNow(rDrag.GetNow());

            aStr = String();
            aStr.AppendAscii("dx=");
            pModel->TakeMetricStr(aNow.X() - aBeg.X(), aMetr, TRUE);
            aStr += aMetr;

            aStr.AppendAscii(" dy=");
            pModel->TakeMetricStr(aNow.Y() - aBeg.Y(), aMetr, TRUE);
            aStr += aMetr;

            UINT16 nPntNum(pHdl->GetPointNum());
            const XPolygon& rXPoly = aPathPolygon[rDrag.GetHdl()->GetPolyNum()];
            UINT16 nPntAnz(rXPoly.GetPointCount());
            BOOL bClose(IsClosed());

            if(bClose)
                nPntAnz--;

            if(pHdl->IsPlusHdl())
            {
                // Hebel
                UINT16 nRef(nPntNum);

                if(rXPoly.IsControl(nPntNum + 1))
                    nRef--;
                else
                    nRef++;

                aNow -= rXPoly[nRef];

                INT32 nLen(GetLen(aNow));
                aStr.AppendAscii("  l=");
                pModel->TakeMetricStr(nLen, aMetr, TRUE);
                aStr += aMetr;

                INT32 nWink(GetAngle(aNow));
                aStr += sal_Unicode(' ');
                pModel->TakeWinkStr(nWink, aMetr);
                aStr += aMetr;
            }
            else if(nPntAnz > 1)
            {
                UINT16 nPntMax(nPntAnz - 1);
                Point aPt1,aPt2;
                BOOL bClose(IsClosed());
                BOOL bPt1(nPntNum > 0);
                BOOL bPt2(nPntNum < nPntMax);

                if(bClose && nPntAnz > 2)
                {
                    bPt1 = TRUE;
                    bPt2 = TRUE;
                }

                UINT16 nPt1,nPt2;

                if(nPntNum > 0)
                    nPt1 = nPntNum - 1;
                else
                    nPt1 = nPntMax;

                if(nPntNum < nPntMax)
                    nPt2 = nPntNum + 1;
                else
                    nPt2 = 0;

                if(bPt1 && rXPoly.IsControl(nPt1))
                    bPt1 = FALSE; // Keine Anzeige

                if(bPt2 && rXPoly.IsControl(nPt2))
                    bPt2 = FALSE; // von Bezierdaten

                if(bPt1)
                {
                    Point aPt(aNow);
                    aPt -= rXPoly[nPt1];

                    INT32 nLen(GetLen(aPt));
                    aStr.AppendAscii("  l=");
                    pModel->TakeMetricStr(nLen, aMetr, TRUE);
                    aStr += aMetr;

                    INT32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    pModel->TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }

                if(bPt2)
                {
                    if(bPt1)
                        aStr.AppendAscii(" / ");
                    else
                        aStr.AppendAscii("  ");

                    Point aPt(aNow);
                    aPt -= rXPoly[nPt2];

                    INT32 nLen(GetLen(aPt));
                    aStr.AppendAscii("l=");
                    pModel->TakeMetricStr(nLen, aMetr, TRUE);
                    aStr += aMetr;

                    INT32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    pModel->TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }
            }
        }
    }
    else if(pModel)
    {
        // Ansonsten CreateComment
        ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();
        SdrObjKind eKindMerk = eKind;

        // fuer Description bei Mixed das Aktuelle...
        ((SdrPathObj*)this)->eKind = pU->eAktKind;
        ImpTakeDescriptionStr(STR_ViewCreateObj, aStr);
        ((SdrPathObj*)this)->eKind = eKindMerk;

        Point aPrev(rDrag.GetPrev());
        Point aNow(rDrag.GetNow());

        if(pU->bLine)
            aNow = pU->aLineEnd;

        aNow -= aPrev;
        aStr.AppendAscii(" (");

        XubString aMetr;

        if(pU->bCircle)
        {
            pModel->TakeWinkStr(Abs(pU->nCircRelWink), aMetr);
            aStr += aMetr;
            aStr.AppendAscii(" r=");
            pModel->TakeMetricStr(pU->nCircRadius, aMetr, TRUE);
            aStr += aMetr;
        }

        aStr.AppendAscii("dx=");
        pModel->TakeMetricStr(aNow.X(), aMetr, TRUE);
        aStr += aMetr;

        aStr.AppendAscii(" dy=");
        pModel->TakeMetricStr(aNow.Y(), aMetr, TRUE);
        aStr += aMetr;

        if(!IsFreeHand())
        {
            INT32 nLen(GetLen(aNow));
            aStr.AppendAscii("  l=");
            pModel->TakeMetricStr(nLen, aMetr, TRUE);
            aStr += aMetr;

            INT32 nWink(GetAngle(aNow));
            aStr += sal_Unicode(' ');
            pModel->TakeWinkStr(nWink, aMetr);
            aStr += aMetr;
        }

        aStr += sal_Unicode(')');
    }
    return aStr;
}

void lcl_CopyBezier(const XPolygon& rSrc, USHORT nSPos, XPolygon& rDst, USHORT nDPos)
{
    rDst[nDPos++] = rSrc[nSPos++];
    rDst.SetFlags(nDPos, XPOLY_CONTROL);
    rDst[nDPos++] = rSrc[nSPos++];
    rDst.SetFlags(nDPos, XPOLY_CONTROL);
    rDst[nDPos++] = rSrc[nSPos++];
    rDst[nDPos] = rSrc[nSPos];
}

void SdrPathObj::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    ImpSdrPathDragData* pID=(ImpSdrPathDragData*)rDrag.GetUser();
    const XPolygon& rXP=aPathPolygon[rDrag.GetHdl()->GetPolyNum()];
    if (rXP.GetPointCount()<=2 /*|| rXPoly.GetFlags(1)==XPOLY_CONTROL && rXPoly.GetPointCount()<=4*/) {
        XPolygon aXPoly(rXP);
        aXPoly[rDrag.GetHdl()->GetPointNum()]=rDrag.GetNow();
        rXPP.Insert(aXPoly);
        return;
    }
    // Div. Daten lokal Kopieren fuer weniger Code und schnelleren Zugriff
    FASTBOOL bClosed       =pID->bClosed       ; // geschlossenes Objekt?
    USHORT   nPntAnz       =pID->nPntAnz       ; // Punktanzahl
    USHORT   nPnt          =pID->nPnt          ; // Punktnummer innerhalb des Polygons
    FASTBOOL bBegPnt       =pID->bBegPnt       ; // Gedraggter Punkt ist der Anfangspunkt einer Polyline
    FASTBOOL bEndPnt       =pID->bEndPnt       ; // Gedraggter Punkt ist der Endpunkt einer Polyline
    USHORT   nPrevPnt      =pID->nPrevPnt      ; // Index des vorherigen Punkts
    USHORT   nNextPnt      =pID->nNextPnt      ; // Index des naechsten Punkts
    FASTBOOL bPrevIsBegPnt =pID->bPrevIsBegPnt ; // Vorheriger Punkt ist Anfangspunkt einer Polyline
    FASTBOOL bNextIsEndPnt =pID->bNextIsEndPnt ; // Folgepunkt ist Endpunkt einer Polyline
    USHORT   nPrevPrevPnt  =pID->nPrevPrevPnt  ; // Index des vorvorherigen Punkts
    USHORT   nNextNextPnt  =pID->nNextNextPnt  ; // Index des uebernaechsten Punkts
    FASTBOOL bControl      =pID->bControl      ; // Punkt ist ein Kontrollpunkt
    FASTBOOL bIsPrevControl=pID->bIsPrevControl; // Punkt ist Kontrollpunkt vor einem Stuetzpunkt
    FASTBOOL bIsNextControl=pID->bIsNextControl; // Punkt ist Kontrollpunkt hinter einem Stuetzpunkt
    FASTBOOL bPrevIsControl=pID->bPrevIsControl; // Falls nPnt ein StPnt: Davor ist ein Kontrollpunkt
    FASTBOOL bNextIsControl=pID->bNextIsControl; // Falls nPnt ein StPnt: Dahinter ist ein Kontrollpunkt
    XPolygon aXPoly(pID->aXP);
    XPolygon aLine1(2);
    XPolygon aLine2(2);
    XPolygon aLine3(2);
    XPolygon aLine4(2);
    if (bControl) {
        aLine1[1]=pID->aXP[nPnt];
        if (bIsNextControl) { // bin ich Kontrollpunkt hinter der Stuetzstelle?
            aLine1[0]=pID->aXP[nPrevPnt];
            aLine2[0]=pID->aXP[nNextNextPnt];
            aLine2[1]=pID->aXP[nNextPnt];
            if (pID->aXP.IsSmooth(nPrevPnt) && !bPrevIsBegPnt && pID->aXP.IsControl(nPrevPrevPnt)) {
                aXPoly.Insert(0,rXP[pID->nPrevPrevPnt0-1],XPOLY_CONTROL);
                aXPoly.Insert(0,rXP[pID->nPrevPrevPnt0-2],XPOLY_NORMAL);
                // Hebellienien fuer das gegenueberliegende Kurvensegment
                aLine3[0]=pID->aXP[nPrevPnt];
                aLine3[1]=pID->aXP[nPrevPrevPnt];
                aLine4[0]=rXP[pID->nPrevPrevPnt0-2];
                aLine4[1]=rXP[pID->nPrevPrevPnt0-1];
            } else {
                aXPoly.Remove(0,1);
            }
        } else { // ansonsten bin ich Kontrollpunkt vor der Stuetzstelle
            aLine1[0]=pID->aXP[nNextPnt];
            aLine2[0]=pID->aXP[nPrevPrevPnt];
            aLine2[1]=pID->aXP[nPrevPnt];
            if (pID->aXP.IsSmooth(nNextPnt) && !bNextIsEndPnt && pID->aXP.IsControl(nNextNextPnt)) {
                aXPoly.Insert(XPOLY_APPEND,rXP[pID->nNextNextPnt0+1],XPOLY_CONTROL);
                aXPoly.Insert(XPOLY_APPEND,rXP[pID->nNextNextPnt0+2],XPOLY_NORMAL);
                // Hebellinien fuer das gegenueberliegende Kurvensegment
                aLine3[0]=pID->aXP[nNextPnt];
                aLine3[1]=pID->aXP[nNextNextPnt];
                aLine4[0]=rXP[pID->nNextNextPnt0+2];
                aLine4[1]=rXP[pID->nNextNextPnt0+1];
            } else {
                aXPoly.Remove(aXPoly.GetPointCount()-1,1);
            }
        }
    } else { // ansonsten kein Kontrollpunkt
        if (pID->bEliminate) {
            aXPoly.Remove(2,1);
        }
        if (bPrevIsControl) aXPoly.Insert(0,rXP[pID->nPrevPrevPnt0-1],XPOLY_NORMAL);
        else if (!bBegPnt && !bPrevIsBegPnt && pID->aXP.IsControl(nPrevPrevPnt)) {
            aXPoly.Insert(0,rXP[pID->nPrevPrevPnt0-1],XPOLY_CONTROL);
            aXPoly.Insert(0,rXP[pID->nPrevPrevPnt0-2],XPOLY_NORMAL);
        } else {
            aXPoly.Remove(0,1);
            if (bBegPnt) aXPoly.Remove(0,1);
        }
        if (bNextIsControl) aXPoly.Insert(XPOLY_APPEND,rXP[pID->nNextNextPnt0+1],XPOLY_NORMAL);
        else if (!bEndPnt && !bNextIsEndPnt && pID->aXP.IsControl(nNextNextPnt)) {
            aXPoly.Insert(XPOLY_APPEND,rXP[pID->nNextNextPnt0+1],XPOLY_CONTROL);
            aXPoly.Insert(XPOLY_APPEND,rXP[pID->nNextNextPnt0+2],XPOLY_NORMAL);
        } else {
            aXPoly.Remove(aXPoly.GetPointCount()-1,1);
            if (bEndPnt) aXPoly.Remove(aXPoly.GetPointCount()-1,1);
        }
        if (bClosed) { // "Birnenproblem": 2 Linien, 1 Kurve, alles Smooth, Punkt zw. beiden Linien wird gedraggt
            if (aXPoly.GetPointCount()>nPntAnz && aXPoly.IsControl(1)) {
                USHORT a=aXPoly.GetPointCount();
                aXPoly[a-2]=aXPoly[2]; aXPoly.SetFlags(a-2,aXPoly.GetFlags(2));
                aXPoly[a-1]=aXPoly[3]; aXPoly.SetFlags(a-1,aXPoly.GetFlags(3));
                aXPoly.Remove(0,3);
            }
        }
    }
    rXPP.Insert(aXPoly);
    if (aLine1.GetPointCount()>1) rXPP.Insert(aLine1);
    if (aLine2.GetPointCount()>1) rXPP.Insert(aLine2);
    if (aLine3.GetPointCount()>1) rXPP.Insert(aLine3);
    if (aLine4.GetPointCount()>1) rXPP.Insert(aLine4);
}

FASTBOOL SdrPathObj::BegCreate(SdrDragStat& rStat)
{
    FASTBOOL bFreeHand=IsFreeHand();
    rStat.SetNoSnap(bFreeHand);
    rStat.SetOrtho8Possible();
    aPathPolygon.Clear();
    bCreating=TRUE;
    FASTBOOL bMakeStartPoint=TRUE;
    SdrView* pView=rStat.GetView();
    if (pView!=NULL && pView->IsUseIncompatiblePathCreateInterface() &&
        (eKind==OBJ_POLY || eKind==OBJ_PLIN || eKind==OBJ_PATHLINE || eKind==OBJ_PATHFILL)) {
        bMakeStartPoint=FALSE;
    }
    aPathPolygon.Insert(XPolygon());
    aPathPolygon[0][0]=rStat.GetStart();
    if (bMakeStartPoint) {
        aPathPolygon[0][1]=rStat.GetNow();
    }
    ImpPathCreateUser* pU=new ImpPathCreateUser;
    pU->eStartKind=eKind;
    pU->eAktKind=eKind;
    rStat.SetUser(pU);
    return TRUE;
}

FASTBOOL SdrPathObj::MovCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    SdrView* pView=rStat.GetView();
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    if (pView!=NULL && pView->IsCreateMode()) {
        // ggf. auf anderes CreateTool umschalten
        UINT16 nIdent;
        UINT32 nInvent;
        pView->TakeCurrentObj(nIdent,nInvent);
        if (nInvent==SdrInventor && pU->eAktKind!=(SdrObjKind)nIdent) {
            SdrObjKind eNewKind=(SdrObjKind)nIdent;
            switch (eNewKind) {
                case OBJ_CARC: case OBJ_CIRC: case OBJ_CCUT: case OBJ_SECT: eNewKind=OBJ_CARC;
                case OBJ_RECT:
                case OBJ_LINE: case OBJ_PLIN: case OBJ_POLY:
                case OBJ_PATHLINE: case OBJ_PATHFILL:
                case OBJ_FREELINE: case OBJ_FREEFILL:
                case OBJ_SPLNLINE: case OBJ_SPLNFILL: {
                    pU->eAktKind=eNewKind;
                    pU->bMixedCreate=TRUE;
                    pU->nBezierStartPoint=rXPoly.GetPointCount();
                    if (pU->nBezierStartPoint>0) pU->nBezierStartPoint--;
                } break;
            } // switch
        }
    }
    USHORT nActPoint=rXPoly.GetPointCount();
    if (aPathPolygon.Count()>1 && rStat.IsMouseDown() && nActPoint<2) {
        rXPoly[0]=rStat.GetPos0();
        rXPoly[1]=rStat.GetNow();
        nActPoint=2;
    }
    if (nActPoint==0) {
        rXPoly[0]=rStat.GetPos0();
    } else nActPoint--;
    FASTBOOL bFreeHand=IsFreeHand(pU->eAktKind);
    rStat.SetNoSnap(bFreeHand /*|| (pU->bMixed && pU->eAktKind==OBJ_LINE)*/);
    rStat.SetOrtho8Possible(pU->eAktKind!=OBJ_CARC && pU->eAktKind!=OBJ_RECT && (!pU->bMixedCreate || pU->eAktKind!=OBJ_LINE));
    Point aActMerk(rXPoly[nActPoint]);
    rXPoly[nActPoint]=rStat.Now();
    if (!pU->bMixedCreate && pU->eStartKind==OBJ_LINE && rXPoly.GetPointCount()>=1) {
        Point aPt(rStat.Start());
        if (pView!=NULL && pView->IsCreate1stPointAsCenter()) {
            aPt+=aPt;
            aPt-=rStat.Now();
        }
        rXPoly[0]=aPt;
    }
    OutputDevice* pOut=pView==NULL ? NULL : pView->GetWin(0);
    if (bFreeHand) {
        if (pU->nBezierStartPoint>nActPoint) pU->nBezierStartPoint=nActPoint;
        if (rStat.IsMouseDown() && nActPoint>0) {
            // keine aufeinanderfolgenden Punkte an zu Nahe gelegenen Positionen zulassen
            long nMinDist=1;
            if (pView!=NULL) nMinDist=pView->GetFreeHandMinDistPix();
            if (pOut!=NULL) nMinDist=pOut->PixelToLogic(Size(nMinDist,0)).Width();
            if (nMinDist<1) nMinDist=1;

            Point aPt0(rXPoly[nActPoint-1]);
            Point aPt1(rStat.Now());
            long dx=aPt0.X()-aPt1.X(); if (dx<0) dx=-dx;
            long dy=aPt0.Y()-aPt1.Y(); if (dy<0) dy=-dy;
            if (dx<nMinDist && dy<nMinDist) return FALSE;

            // folgendes ist aus EndCreate kopiert (nur kleine Modifikationen)
            // und sollte dann mal in eine Methode zusammengefasst werden:

            if (nActPoint-pU->nBezierStartPoint>=3 && ((nActPoint-pU->nBezierStartPoint)%3)==0) {
                rXPoly.PointsToBezier(nActPoint-3);
                rXPoly.SetFlags(nActPoint-1,XPOLY_CONTROL);
                rXPoly.SetFlags(nActPoint-2,XPOLY_CONTROL);

                if (nActPoint>=6 && rXPoly.IsControl(nActPoint-4)) {
                    rXPoly.CalcTangent(nActPoint-3,nActPoint-4,nActPoint-2);
                    rXPoly.SetFlags(nActPoint-3,XPOLY_SMOOTH);
                }
            }
            rXPoly[nActPoint+1]=rStat.Now();
            rStat.NextPoint();
        } else {
            pU->nBezierStartPoint=nActPoint;
        }
    }

    pU->ResetFormFlags();
    if (IsBezier(pU->eAktKind)) {
        if (nActPoint>=2) {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],rStat.IsMouseDown(),pView);
        } else if (pU->bBezHasCtrl0) {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],pU->aBezControl0-rXPoly[nActPoint-1],rStat.IsMouseDown(),pView);
        }
    }
    if (pU->eAktKind==OBJ_CARC && nActPoint>=2) {
        pU->CalcCircle(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }
    if (pU->eAktKind==OBJ_LINE && nActPoint>=2) {
        pU->CalcLine(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }
    if (pU->eAktKind==OBJ_RECT && nActPoint>=2) {
        pU->CalcRect(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }

    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    ImpForceKind();
    if (pU->eStartKind!=OBJ_LINE) { aGeo.nDrehWink=0; aGeo.RecalcSinCos(); }
    //ImpForceLineWink();
    return TRUE;
}

FASTBOOL SdrPathObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    FASTBOOL bRet=FALSE;
    SdrView* pView=rStat.GetView();
    FASTBOOL bIncomp=pView!=NULL && pView->IsUseIncompatiblePathCreateInterface();
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    USHORT nActPoint=rXPoly.GetPointCount()-1;
    Point aAktMerk(rXPoly[nActPoint]);
    rXPoly[nActPoint]=rStat.Now();
    if (!pU->bMixedCreate && pU->eStartKind==OBJ_LINE) {
        if (rStat.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        bRet=eCmd==SDRCREATE_FORCEEND;
        if (bRet) {
            bCreating=FALSE;
            delete pU;
            rStat.SetUser(NULL);
        }
        ImpForceKind();
        return bRet;
    }

    OutputDevice* pOut=pView==NULL ? NULL : pView->GetWin(0);
    long nCloseDist=0;
    FASTBOOL bCloseOnEnd=FALSE;
    if (pView!=NULL && pOut!=NULL && pView->IsAutoClosePolys() && !bIncomp) {
        nCloseDist=pView->GetAutoCloseDistPix();
        nCloseDist=pOut->PixelToLogic(Size(nCloseDist,0)).Width();
        if (nCloseDist<1) nCloseDist=1;
        Point aPt0(rStat.GetStart());
        Point aPt1(rStat.GetNow());
        long dx=aPt0.X()-aPt1.X(); if (dx<0) dx=-dx;
        long dy=aPt0.Y()-aPt1.Y(); if (dy<0) dy=-dy;
        bCloseOnEnd=dx<=nCloseDist && dy<=nCloseDist;
    }

    if (!pU->bMixedCreate && IsFreeHand(pU->eStartKind)) {
        if (rStat.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        if (eCmd==SDRCREATE_FORCEEND && (bCloseOnEnd || IsClosed())) {
            ImpSetClosed(TRUE);
        }
        bRet=eCmd==SDRCREATE_FORCEEND;
        if (bRet) {
            bCreating=FALSE;
            delete pU;
            rStat.SetUser(NULL);
        }
        ImpForceKind();
        aGeo.nDrehWink=0; aGeo.RecalcSinCos();
        return bRet;
    }
    if (eCmd==SDRCREATE_NEXTPOINT || eCmd==SDRCREATE_NEXTOBJECT) {
        // keine aufeinanderfolgenden Punkte an identischer Position zulassen
        if (nActPoint==0 || rStat.Now()!=rXPoly[nActPoint-1]) {
            if (bIncomp) {
                if (pU->nBezierStartPoint>nActPoint) pU->nBezierStartPoint=nActPoint;
                if (IsBezier(pU->eAktKind) && nActPoint-pU->nBezierStartPoint>=3 && ((nActPoint-pU->nBezierStartPoint)%3)==0) {
                    rXPoly.PointsToBezier(nActPoint-3);
                    rXPoly.SetFlags(nActPoint-1,XPOLY_CONTROL);
                    rXPoly.SetFlags(nActPoint-2,XPOLY_CONTROL);

                    if (nActPoint>=6 && rXPoly.IsControl(nActPoint-4)) {
                        rXPoly.CalcTangent(nActPoint-3,nActPoint-4,nActPoint-2);
                        rXPoly.SetFlags(nActPoint-3,XPOLY_SMOOTH);
                    }
                }
            } else {
                if (nActPoint==1 && IsBezier(pU->eAktKind) && !pU->bBezHasCtrl0) {
                    pU->aBezControl0=rStat.GetNow();;
                    pU->bBezHasCtrl0=TRUE;
                    nActPoint--;
                }
                if (pU->IsFormFlag()) {
                    USHORT nPtAnz0=rXPoly.GetPointCount();
                    rXPoly.Remove(nActPoint-1,2); // die letzten beiden Punkte entfernen und durch die Form ersetzen
                    rXPoly.Insert(XPOLY_APPEND,pU->GetFormPoly());
                    USHORT nPtAnz1=rXPoly.GetPointCount();
                    for (USHORT i=nPtAnz0+1; i<nPtAnz1-1; i++) { // Damit BckAction richtig funktioniert
                        if (!rXPoly.IsControl(i)) rStat.NextPoint();
                    }
                    nActPoint=rXPoly.GetPointCount()-1;
                }
            }
            nActPoint++;
            rXPoly[nActPoint]=rStat.GetNow();
        }
        if (eCmd==SDRCREATE_NEXTOBJECT) {
            if (rXPoly.GetPointCount()>=2) {
                pU->bBezHasCtrl0=FALSE;
                // nur einzelnes Polygon kann offen sein, deshalb schliessen
                rXPoly[nActPoint]=rXPoly[0];
                ImpSetClosed(TRUE);
                XPolygon aXP;
                aXP[0]=rStat.GetNow();
                aPathPolygon.Insert(aXP);
            }
        }
    }

    USHORT nPolyAnz=aPathPolygon.Count();
    if (nPolyAnz!=0) {
        // den letzten Punkt ggf. wieder loeschen
        if (eCmd==SDRCREATE_FORCEEND) {
            XPolygon& rXP=aPathPolygon[nPolyAnz-1];
            USHORT nPtAnz=rXP.GetPointCount();
            if (nPtAnz>=2) {
                if (!rXP.IsControl(nPtAnz-2)) {
                    if (rXP[nPtAnz-1]==rXP[nPtAnz-2]) {
                        rXP.Remove(nPtAnz-1,1);
                    }
                } else {
                    if (rXP[nPtAnz-3]==rXP[nPtAnz-2]) {
                        rXP.Remove(nPtAnz-3,3);
                    }
                }
            }
        }
        for (USHORT nPolyNum=nPolyAnz; nPolyNum>0;) {
            nPolyNum--;
            XPolygon& rXP=aPathPolygon[nPolyNum];
            USHORT nPtAnz=rXP.GetPointCount();
            // Polygone mit zu wenig Punkten werden geloescht
            if (nPolyNum<nPolyAnz-1 || eCmd==SDRCREATE_FORCEEND) {
                if (nPtAnz<2) aPathPolygon.Remove(nPolyNum);
            }
        }
    }
    pU->ResetFormFlags();
    bRet=eCmd==SDRCREATE_FORCEEND;
    if (bRet) {
        bCreating=FALSE;
        delete pU;
        rStat.SetUser(NULL);
        aRect=aPathPolygon.GetBoundRect(); // fuer SdrTextObj
        if (bCloseOnEnd || IsClosed()) ImpSetClosed(TRUE);
        ImpForceKind();
        SetRectsDirty();
    }
    return bRet;
}

FASTBOOL SdrPathObj::BckCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    if (aPathPolygon.Count()>0) {
        XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
        USHORT nActPoint=rXPoly.GetPointCount();
        if (nActPoint>0) {
            nActPoint--;
            // Das letzte Stueck einer Bezierkurve wird erstmal zu 'ner Linie
            rXPoly.Remove(nActPoint,1);
            if (nActPoint>=3 && rXPoly.IsControl(nActPoint-1)) {
                // Beziersegment am Ende sollte zwar nicht vorkommen, aber falls doch ...
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }
        nActPoint=rXPoly.GetPointCount();
        if (nActPoint>=4) { // Kein Beziersegment am Ende
            nActPoint--;
            if (rXPoly.IsControl(nActPoint-1)) {
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }
        if (rXPoly.GetPointCount()<2) {
            aPathPolygon.Remove(aPathPolygon.Count()-1);
        }
        if (aPathPolygon.Count()>0) {
            XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
            USHORT nActPoint=rXPoly.GetPointCount();
            if (nActPoint>0) {
                nActPoint--;
                rXPoly[nActPoint]=rStat.Now();
            }
        }
    }
    pU->ResetFormFlags();
    return aPathPolygon.Count()!=0;
}

void SdrPathObj::BrkCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    aPathPolygon.Clear();
    bCreating=FALSE;
    delete pU;
    rStat.SetUser(NULL);
}

void SdrPathObj::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP=aPathPolygon;
    SdrView* pView=rDrag.GetView();
    if (pView!=NULL && pView->IsUseIncompatiblePathCreateInterface()) return;

    ImpPathCreateUser* pU=(ImpPathCreateUser*)rDrag.GetUser();
    XPolygon& rXP=rXPP[rXPP.Count()-1];
    USHORT nPtAnz=rXP.GetPointCount();
    if (pU->IsFormFlag()) { // Letztes Polylinesegment entfernen und durch Form ersetzen
        rXP.Remove(nPtAnz-2,2);
        nPtAnz=rXP.GetPointCount();
        rXP.Insert(nPtAnz,pU->GetFormPoly());
    }
    if (pU->bBezier && rDrag.IsMouseDown()) { // Dragging des Hebels
        // Den Hebel etwas gestrichelt darstellen:
        // erst -- -- -- -- -- -- -- und das letzte 1/4 frei
        // -> 1/25 Raster
        XPolygon aXP(2);
        Point aP1(pU->aBezCtrl2);
        Point aP2(pU->aBezEnd);
        long dx=aP2.X()-aP1.X();
        long dy=aP2.Y()-aP1.Y();
        for (long i=0; i<7; i++) {
            aXP[0].X()=aP1.X()+(i*3)*dx/25;
            aXP[0].Y()=aP1.Y()+(i*3)*dy/25;
            aXP[1].X()=aP1.X()+(2+i*3)*dx/25;
            aXP[1].Y()=aP1.Y()+(2+i*3)*dy/25;
            rXPP.Insert(aXP);
        }
    }
}

Pointer SdrPathObj::GetCreatePointer() const
{
    switch (eKind) {
        case OBJ_LINE    : return Pointer(POINTER_DRAW_LINE);
        case OBJ_POLY    : return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PLIN    : return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PATHLINE: return Pointer(POINTER_DRAW_BEZIER);
        case OBJ_PATHFILL: return Pointer(POINTER_DRAW_BEZIER);
        case OBJ_FREELINE: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_FREEFILL: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_SPLNLINE: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_SPLNFILL: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_PATHPOLY: return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PATHPLIN: return Pointer(POINTER_DRAW_POLYGON);
    } // switch
    return Pointer(POINTER_CROSS);
}

void SdrPathObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    MoveXPoly(aPathPolygon,rSiz);
}

void SdrPathObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrTextObj::NbcResize(rRef,xFact,yFact);
    ResizeXPoly(aPathPolygon,rRef,xFact,yFact);
}

void SdrPathObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
    RotateXPoly(aPathPolygon,rRef,sn,cs);
}

void SdrPathObj::NbcShear(const Point& rRefPnt, long nAngle, double fTan, FASTBOOL bVShear)
{
    SdrTextObj::NbcShear(rRefPnt,nAngle,fTan,bVShear);
    ShearXPoly(aPathPolygon,rRefPnt,fTan,bVShear);
}

void SdrPathObj::NbcMirror(const Point& rRefPnt1, const Point& rRefPnt2)
{
    SdrTextObj::NbcMirror(rRefPnt1,rRefPnt2);
    MirrorXPoly(aPathPolygon,rRefPnt1,rRefPnt2);
}

void SdrPathObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    if (aGeo.nDrehWink==0) {
        rRect=GetSnapRect();
    } else {
        XPolyPolygon aXPP(aPathPolygon);
        RotateXPoly(aXPP,Point(),-aGeo.nSin,aGeo.nCos);
        rRect=aXPP.GetBoundRect();
        Point aTmp(rRect.TopLeft());
        RotatePoint(aTmp,Point(),aGeo.nSin,aGeo.nCos);
        aTmp-=rRect.TopLeft();
        rRect.Move(aTmp.X(),aTmp.Y());
    }
}

void SdrPathObj::RecalcSnapRect()
{
    maSnapRect=aPathPolygon.GetBoundRect();
}

void SdrPathObj::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aOld(GetSnapRect());
    long nMulX = rRect.Right()  - rRect.Left();
    long nDivX = aOld.Right()   - aOld.Left();
    long nMulY = rRect.Bottom() - rRect.Top();
    long nDivY = aOld.Bottom()  - aOld.Top();
    if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
    if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
    Fraction aX(nMulX,nDivX);
    Fraction aY(nMulY,nDivY);
    NbcResize(aOld.TopLeft(), aX, aY);
    NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
}

USHORT SdrPathObj::GetSnapPointCount() const
{
    return GetHdlCount();
}

Point SdrPathObj::GetSnapPoint(USHORT nSnapPnt) const
{
    USHORT nPoly,nPnt;
    if (!FindPolyPnt(nSnapPnt,nPoly,nPnt,FALSE)) {
        DBG_ASSERT(FALSE,"SdrPathObj::GetSnapPoint: Punkt nSnapPnt nicht vorhanden!");
    }
    return Point( aPathPolygon[nPoly][nPnt] );
}

FASTBOOL SdrPathObj::IsPolyObj() const
{
    return TRUE;
}

USHORT SdrPathObj::GetPointCount() const
{
    USHORT nPolyCnt = aPathPolygon.Count();
    USHORT nPntCnt = 0;

    for (USHORT i = 0; i < nPolyCnt; i++)
        nPntCnt += aPathPolygon[i].GetPointCount();

    return nPntCnt;
}

const Point& SdrPathObj::GetPoint(USHORT nHdlNum) const
{
    USHORT nPoly,nPnt;
    FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE);
    return aPathPolygon[nPoly][nPnt];
}

void SdrPathObj::NbcSetPoint(const Point& rPnt, USHORT nHdlNum)
{
    USHORT nPoly,nPnt;

    if (FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE)) {
        XPolygon& rXP=aPathPolygon[nPoly];
        rXP[nPnt]=rPnt;
        if (IsClosed() && nPnt==0) rXP[rXP.GetPointCount()-1]=rXP[0];
        if (eKind==OBJ_LINE) ImpForceLineWink();
        SetRectsDirty();
    }
}

/*************************************************************************/
// Abstand des Punktes Pt zur Strecke AB. Hat der Punkt Pt keine Senkrechte
// zur Strecke AB, so ist der Abstand zum naechstliegenden Punkt verwendet;
// dazu wird nocheinmal der einfache Abstand parallel zu AB draufaddiert
// (als Winkelersatz) (=> groesserer Abstand=unguenstigerer Winkel).

BigInt GetLineQDist(Point aPt, Point aA, Point aB)
{
    aPt-=aA; // Nullpunkt zum Linienanfang verschieben
    aB-=aA;
    aA=Point(0,0);
    // Nun auf eine waagerechte Strecke transformieren
    // Ergebnis:  aA Ã-------´ aB
    long nWink=GetAngle(aB);
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    RotatePoint(aPt,aA,-nSin,nCos);
    RotatePoint(aB,aA,-nSin,nCos);
    //
    if (aPt.X()>=aA.X() && aPt.X()<=aB.X()) { // Hat Pt eine Senkrechte auf AB?
        BigInt nRet=aPt.Y();
        return nRet*nRet;
    }
    if (aPt.X()<aA.X()) { // Pt liegt links der Strecke
        BigInt nDX=aA.X()-aPt.X();
        BigInt nDY=aA.Y()-aPt.Y();
        return nDX*nDX+nDY*nDY+nDX; // Den horizontalen Abstand nochmal als Winkelersatz dazu
    }
    // Pt liegt rechts der Strecke
    BigInt nDX=aPt.X()-aB.X();
    BigInt nDY=aPt.Y()-aB.Y();
    return nDX*nDX+nDY*nDY+nDX; // Den horizontalen Abstand nochmal als Winkelersatz dazu
}

BigInt GetQDist(const Point& rP1, const Point& rP2)
{
    BigInt nDX(rP2.X()-rP1.X());
    BigInt nDY(rP2.Y()-rP1.Y());
    return nDX*nDX+nDY*nDY;
}

USHORT SdrPathObj::NbcInsPoint(const Point& rPos, FASTBOOL bNewObj, FASTBOOL bHideHim,
    FASTBOOL& rInsNextAfter)
{
    USHORT nNewHdl;

    if (bNewObj) {
        rInsNextAfter=TRUE;
        nNewHdl=NbcInsPoint(0,rPos,FALSE,bNewObj,bHideHim);
    } else {
        VirtualDevice   aVDev;
        Polygon         aPoly, aStart(2), aEnd(2);
        Point           aBestPnt[2];
        BigInt          nBestDst(0x7FFFFFFF);
        USHORT          nBestPoly = 0;
        USHORT          nBestPnt = 0xFFFF;
        USHORT          nPolyCnt = aPathPolygon.Count();
        USHORT          nPoly;
        USHORT          nPnt;
        USHORT          nPntMax;
        FASTBOOL        bAppend = FALSE;
        FASTBOOL        bTestEnd = FALSE;

        MapMode aMap = aVDev.GetMapMode();
        aMap.SetMapUnit(pModel->GetScaleUnit());
        aMap.SetScaleX(pModel->GetScaleFraction());
        aMap.SetScaleY(pModel->GetScaleFraction());
        aVDev.SetMapMode(aMap);

        for (nPoly = 0; nPoly < nPolyCnt; nPoly++)
        {
            XPolygon aXPolyPart(4);
            XPolygon& rXPoly = aPathPolygon[nPoly];
            nPnt=0;

            nPntMax=rXPoly.GetPointCount();
            if (nPntMax>0) {
                nPntMax--;

                while (nPnt<nPntMax) {
                    USHORT nNextPartPos;

                    if (rXPoly.IsControl(nPnt+1)) {
                        lcl_CopyBezier(rXPoly,nPnt,aXPolyPart,0);
                        aPoly=XOutCreatePolygon(aXPolyPart,&aVDev);
                        nNextPartPos=3;
                    } else {
                        aPoly = Polygon(2);
                        aPoly[0] = rXPoly[nPnt];
                        aPoly[1] = rXPoly[nPnt+1];
                        nNextPartPos = 1;
                    }
                    USHORT nPartMax=aPoly.GetSize()-1;

                    for (USHORT i=0; i<nPartMax; i++) {
                        BigInt nDist(GetLineQDist(rPos, aPoly[i], aPoly[i+1]));

                        if (nDist<nBestDst) {
                            nBestDst =nDist;
                            nBestPoly=nPoly;
                            nBestPnt =nPnt;
                            aBestPnt[0]=rXPoly[nPnt];
                            aBestPnt[1]=rXPoly[nPnt+1];
                            if (nPnt+nNextPartPos>=nPntMax)
                                bTestEnd=TRUE;
                        }
                    }
                    // Start und Endlinien sichern fuer nachfolgenden Test
                    if (nPnt==0) {
                        aStart[0]=aPoly[0];
                        aStart[1]=aPoly[1];
                    }
                    nPnt+=nNextPartPos;

                    if (nPnt>=nPartMax) {
                        aEnd[1]=aPoly[nPartMax-1];
                        aEnd[0]=aPoly[nPartMax];
                    }
                }
            }
        }
        // und nun noch fuer Linienanfang und -ende
        if ( !IsClosed() )
        {
            if ( nBestPnt == 0 )
            {
                aStart[1] = aStart[1] - aStart[0];
                aStart[0] = rPos - aStart[0];
                // Skalarprodukt fuer Winkelbestimmung
                long nDotProd = aStart[0].X() * aStart[1].X() +
                                aStart[0].Y() * aStart[1].Y();
                // Cosinus des Winkels <= 0? neue Linie am Anfang
                if ( nDotProd <= 0 )
                {
                    nBestPnt = 0;
                    bAppend = TRUE;
                }
            }
            if ( bTestEnd )
            {
                aEnd[1] = aEnd[1] - aEnd[0];
                aEnd[0] = rPos - aEnd[0];
                // Skalarprodukt fuer Winkelbestimmung
                long nDotProd = aEnd[0].X() * aEnd[1].X() +
                                aEnd[0].Y() * aEnd[1].Y();
                // Cosinus des Winkels <= 0? neue Linie am Ende
                if (nDotProd<=0 && (!bAppend || GetQDist(rPos,aEnd[1])<GetQDist(rPos,aStart[0]))) {
                    nBestPnt=nPntMax;
                    bAppend=TRUE;
                }
            }
        }
        USHORT nIdx = 0;
        nPnt = 1;

        // Handle-Nummer aus Punktindex berechnen
        for (nPoly = 0; nPoly <= nBestPoly; nPoly++)
        {
            /*const*/ XPolygon& rXPoly = aPathPolygon[nPoly];
            long nMax = rXPoly.GetPointCount() - 1;

            if ( nPoly == nBestPoly )   nMax = nBestPnt;
            else if ( IsClosed() )   nMax -= 1;

            while ( (long) nPnt <= nMax )
                if ( ! rXPoly.IsControl(nPnt++) )
                    nIdx += 1;
            nPnt = 0;
        }
        // berechnen, wo die naechsten Punkte eingefuegt werden sollen
        if (bAppend) {
            rInsNextAfter=(nBestPnt==nPntMax);
        } else {
            rInsNextAfter=GetQDist(rPos,aBestPnt[0]) < GetQDist(rPos,aBestPnt[1]);
        }
        nNewHdl=NbcInsPoint(nIdx,rPos,(!bAppend || nBestPnt==nPntMax),bNewObj,bHideHim);
    }
    ImpForceKind();
    return nNewHdl;
}

USHORT SdrPathObj::NbcInsPoint(USHORT nHdl, const Point& rPos, FASTBOOL bInsAfter,
    FASTBOOL bNewObj, FASTBOOL bHideHim)
{
    USHORT nHdlCnt=GetHdlCount();
    USHORT nNewHdl;
    Point aPnt(rPos);

    if (bNewObj) {
        XPolygon aNewPoly(SVDOPATH_INITSIZE,SVDOPATH_RESIZE);
        aNewPoly[0]=aPnt;
        aNewPoly[1]=aPnt;
        aNewPoly[2]=aPnt;
        aPathPolygon.Insert(aNewPoly);
        if (!IsClosed() && aPathPolygon.Count()>1) {
             ImpSetClosed(TRUE);
        }
        SetRectsDirty();
        nNewHdl=nHdlCnt;
    } else {
        if (nHdl>nHdlCnt) {
            nHdl=nHdlCnt;
        }

        USHORT nPoly, nPnt;
        if (!FindPolyPnt(nHdl,nPoly,nPnt,FALSE)) {
            DBG_ASSERT(FALSE,"SdrPathObj::NbcInsPoint() ungueltiger Index.");
            return 0xFFFF;
        }
        // Einfuegen des Punktes in das Polygon...
        XPolygon& rXPoly=aPathPolygon[nPoly];
        if (bHideHim && rXPoly.GetPointCount()!=0) {
            aPnt=rXPoly[nPnt];
        }
        USHORT nPntCnt=rXPoly.GetPointCount();
        nNewHdl=nHdl;
        // ggf. dahinter einfuegen
        if (bInsAfter) {
            nPnt++;
            if (nPnt<nPntCnt && rXPoly.IsControl(nPnt))
                nPnt+=2;
            nNewHdl++;
        }
        if (nPnt>0 && rXPoly.IsControl(nPnt-1)) {
            Point aDiff;
            USHORT nInsPos=nPnt-1;

            aDiff=(rXPoly[nPnt]-rPos) /3;
            rXPoly.Insert(nInsPos,rPos+aDiff,XPOLY_CONTROL);
            rXPoly.Insert(nInsPos,rPos,XPOLY_SMOOTH);
            aDiff=(rXPoly[nPnt-3]-rPos) /3;
            rXPoly.Insert(nInsPos,rPos+aDiff,XPOLY_CONTROL);

            rXPoly.CalcTangent(nInsPos+1,nInsPos,nInsPos+2);
        } else {
            rXPoly.Insert(nPnt,aPnt,XPOLY_NORMAL);
        }
    }
    ImpForceKind();
    return nNewHdl;
}

FASTBOOL SdrPathObj::NbcDelPoint(USHORT nHdlNum)
{
    USHORT nPoly,nPnt;

    if (FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE)) {
        XPolygon& rXPoly=aPathPolygon[nPoly];
        FASTBOOL bClosed=IsClosed();

        if (rXPoly.GetPointCount()>1) {
            USHORT nPntMax=USHORT(rXPoly.GetPointCount()-1);
            FASTBOOL bFrst=nPnt==0;
            FASTBOOL bLast=nPnt==nPntMax;
            FASTBOOL bPrevIsBez=!bFrst && rXPoly.IsControl(USHORT(nPnt-1));
            FASTBOOL bNextIsBez=!bLast && rXPoly.IsControl(nPnt+1);
            if (bClosed && bFrst) bPrevIsBez=rXPoly.IsControl(USHORT(nPntMax-1));
            if (bClosed && bLast) bNextIsBez=rXPoly.IsControl(1);
            USHORT nDelOfs=nPnt;
            USHORT nDelAnz=0;

            if (bPrevIsBez && bNextIsBez) { // Bezierpunkt mittendrin
                if (bFrst || bLast) { // Bezierflaeche 1. Punkt
                   nDelAnz=3;
                   rXPoly[nPntMax-1]=rXPoly[2];
                } else { // Bezierflaeche oder -linie mittendrin
                    nDelOfs--;
                    nDelAnz=3;
                }
            } else if (!bPrevIsBez && !bNextIsBez) { // Polygonpunkt oder Polylinepunkt (inkl. Anfangs-/Endpunkt)
                nDelAnz=1;
            } else if (!bClosed && bFrst && bNextIsBez) {
                nDelAnz=3;  // Bezierpunkt am Anfang der Bezierlinie
            } else if (bClosed && bLast && bPrevIsBez) {
                nDelOfs-=2; // Bezierpunkt am Ende der Bezierlinie
                nDelAnz=3;
            } else if (bPrevIsBez && !bNextIsBez) {
                if (bFrst) {
                    nDelAnz=1;  // Uebergang Kurve nach Linie
                    rXPoly.Remove(USHORT(nPntMax-2),2);
                } else {
                    nDelAnz=3;  // Uebergang Kurve nach Linie
                    nDelOfs-=2;
                }
            } else if (!bPrevIsBez && bNextIsBez) {
                nDelAnz=3;  // Uebergang Kurve nach Linie
#ifdef DBG_UTIL
            } else {
                ByteString aStr("SdrPathObj::NbcDelPoint(USHORT(");
                aStr += nHdlNum;
                aStr += ")): Unerlaubt im else-Zweig";
                DBG_ERROR(aStr.GetBuffer());
#endif
            }
            if (nDelAnz!=0) rXPoly.Remove(nDelOfs,nDelAnz);
            if (bClosed) { // letzten Punkt auf den Ersten setzen
                USHORT nPntMax=rXPoly.GetPointCount();
                if (nPntMax>0) {
                    nPntMax--;
                    rXPoly[nPntMax]=rXPoly[0];
                    rXPoly.SetFlags(nPntMax,rXPoly.GetFlags(0));
                }
            }
        }
        if ((bClosed && rXPoly.GetPointCount()<3) || rXPoly.GetPointCount()<2) {
            aPathPolygon.Remove(nPoly);
        }
        SetRectsDirty();
    }
    ImpForceKind();
    return (aPathPolygon.Count()>0);
}

SdrObject* SdrPathObj::NbcRipPoint(USHORT nHdlNum, USHORT& rNewPt0Index)
{
    SdrPathObj* pNewObj=NULL;
    USHORT      nPoly,nPnt;

    if (FindPolyPnt(nHdlNum,nPoly,nPnt,FALSE)) {
        if (nPoly>0) return NULL;

        const XPolygon& rXPoly=aPathPolygon.GetObject(nPoly);
        USHORT nPntAnz=rXPoly.GetPointCount();
        if (nPntAnz<=1) return NULL;
        USHORT nPntMax=USHORT(nPntAnz-1);

        if (IsClosed() && nPntAnz>=1) {
            XPolygon aNeuP(rXPoly);
            USHORT nSrcCnt=nPnt;
            USHORT nIdxCnt=0;
            for (USHORT i=0; i<nPntMax; i++) {
                if (nSrcCnt==0) rNewPt0Index=nIdxCnt; // Mitteilung an den Aufrufer: PtArray ist verschoben
                aNeuP[i]=rXPoly[nSrcCnt];
                aNeuP.SetFlags(i,rXPoly.GetFlags(nSrcCnt));
                nSrcCnt++;
                if (nSrcCnt>=nPntMax) nSrcCnt=0;
                if (!rXPoly.IsControl(i)) nIdxCnt++;
            }
            aNeuP[nPntMax]=aNeuP[0];
            aNeuP.SetFlags(nPntMax,aNeuP.GetFlags(0));
            aPathPolygon.Replace(aNeuP,nPoly);
            ImpSetClosed(FALSE);
        } else if (nPntMax>=2 && nPnt>0 && nPnt<nPntMax) {
            // Aus offener PolyLine zwei PolyLines erzeugen
            pNewObj=(SdrPathObj*)Clone();
            aPathPolygon[0].Remove(nPnt+1,nPntMax-nPnt);
            SetRectsDirty();
            (pNewObj->aPathPolygon)[0].Remove(0,nPnt);
            pNewObj->SetRectsDirty();
        }
    }
    ImpForceKind();
    return pNewObj;
}

void SdrPathObj::NbcShut()
{
}

SdrObject* SdrPathObj::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pRet = ImpConvertMakeObj(aPathPolygon, IsClosed(), bBezier);
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pRet);
    if(pPath)
        pPath->ConvertAllSegments(bBezier ? SDRPATH_CURVE : SDRPATH_LINE);
    pRet = ImpConvertAddText(pRet, bBezier);
    return pRet;
}

SdrObjGeoData* SdrPathObj::NewGeoData() const
{
    return new SdrPathObjGeoData;
}

void SdrPathObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrPathObjGeoData& rPGeo = (SdrPathObjGeoData&) rGeo;
    rPGeo.aPathPolygon=aPathPolygon;
    rPGeo.eKind=eKind;
}

void SdrPathObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrPathObjGeoData& rPGeo=(SdrPathObjGeoData&)rGeo;
    aPathPolygon=rPGeo.aPathPolygon;
    eKind=rPGeo.eKind;
    ImpForceKind(); // damit u.a. bClosed gesetzt wird
}

void SdrPathObj::WriteData(SvStream& rOut) const
{
    SdrTextObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPathObj");
#endif
    {
        SdrDownCompat aPathCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
#ifdef DBG_UTIL
        aPathCompat.SetID("SdrPathObj(PathPolygon)");
#endif
        rOut<<aPathPolygon;
    }
}

void SdrPathObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrTextObj::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPathObj");
#endif
    aPathPolygon.Clear();
    if (rHead.GetVersion()<=6 && (rHead.nIdentifier==OBJ_LINE || rHead.nIdentifier==OBJ_POLY || rHead.nIdentifier==OBJ_PLIN)) {
        // SdrPolyObj importieren
        switch (eKind) {
            case OBJ_LINE: {
                Polygon aP(2);
                rIn>>aP[0];
                rIn>>aP[1];
                aPathPolygon=XPolyPolygon(PolyPolygon(aP));
            } break;
            case OBJ_PLIN: {
                Polygon aP;
                rIn>>aP;
                aPathPolygon=XPolyPolygon(PolyPolygon(aP));
            } break;
            default: {
                PolyPolygon aPoly;
                rIn>>aPoly;
                aPathPolygon=XPolyPolygon(aPoly);
                // und nun die Polygone ggf. durch einfuegen eines weiteren Punktes schliessen
                USHORT nPolyAnz=aPathPolygon.Count();
                for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
                    const XPolygon& rPoly=aPathPolygon[nPolyNum];
                    USHORT nPointAnz=rPoly.GetPointCount();
                    aPathPolygon[nPolyNum].GetPointCount();
                    if (nPointAnz>=2 && rPoly[0]!=rPoly[USHORT(nPointAnz-1)]) {
                        Point aPt(rPoly[0]);
                        aPathPolygon[nPolyNum][nPointAnz]=aPt;
                    }
                }
            }
        }
    } else {
        if (rHead.GetVersion()>=11) { // ab V11 ist das eingepackt
            SdrDownCompat aPathCompat(rIn,STREAM_READ);
#ifdef DBG_UTIL
            aPathCompat.SetID("SdrPathObj(PathPolygon)");
#endif
            rIn>>aPathPolygon;
        } else {
            rIn>>aPathPolygon;
        }
    }
    ImpForceKind(); // ggf. den richtigen Identifier herstellen.
}

void SdrPathObj::NbcSetPathPoly(const XPolyPolygon& rPathPoly)
{
    aPathPolygon=rPathPoly;
    ImpForceKind();
    if (IsClosed()) {
        USHORT nPolyAnz=aPathPolygon.Count();
        for (USHORT nPolyNum=nPolyAnz; nPolyNum>0;) {
            nPolyNum--;
            const XPolygon& rConstXP=aPathPolygon[nPolyNum];
            USHORT nPointAnz=rConstXP.GetPointCount();
            if (nPointAnz!=0) {
                Point aStartPt(rConstXP[0]);
                if (rConstXP[nPointAnz-1]!=aStartPt) {
                    // Polygon schliessen (wird dabei um einen Punkt erweitert)
                    aPathPolygon[nPolyNum][nPointAnz]=aStartPt;
                }
            } else {
                aPathPolygon.Remove(nPolyNum); // leere Polygone raus
            }

        }
    }
    SetRectsDirty();
}

void SdrPathObj::SetPathPoly(const XPolyPolygon& rPathPoly)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetPathPoly(rPathPoly);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrPathObj::ToggleClosed(long nOpenDistance)
{
    Rectangle aBoundRect0;

    if(pUserCall != NULL)
        aBoundRect0 = GetBoundRect();

    FASTBOOL bClosed = IsClosed();
    FASTBOOL bBCFlag = FALSE;

    USHORT nPolyAnz = aPathPolygon.Count();
    for(USHORT nPolyNum = 0; nPolyNum < nPolyAnz; nPolyNum++)
    {
        XPolygon& rXPoly = aPathPolygon[nPolyNum];
        USHORT nPntAnz = rXPoly.GetPointCount();

        if(nPntAnz >= 3)
        {
            USHORT nPntMax = nPntAnz-1;
            if(!bBCFlag)
            {
                SendRepaintBroadcast();
                bBCFlag = TRUE;
            }
            if(bClosed)
            {
                // Oeffnen
                double fDist = rXPoly.CalcDistance(nPntMax, nPntMax-1);

                if(fDist == 0)
                    fDist = 1;

                double fRatio = (double)nOpenDistance / fDist;
                Point aDiff = rXPoly[nPntMax-1] - rXPoly[nPntMax];
                aDiff.X() = (long) (fRatio * aDiff.X());
                aDiff.Y() = (long) (fRatio * aDiff.Y());
                rXPoly[nPntMax] += aDiff;
            }
            else
            {
                // Schliessen
                INT32 nDist0 = (INT32)(rXPoly.CalcDistance(nPntMax, 0) + 0.5);

                if(nDist0 > nOpenDistance)
                {
                    // Punkt hinzufuegen zum schliessen
                    nPntMax += 1;
                }

                rXPoly[nPntMax] = rXPoly[0];
                ImpSetClosed(TRUE);
                rXPoly.SetFlags(nPntMax, rXPoly.GetFlags(0));
                if (rXPoly.IsSmooth(0))
                    rXPoly.CalcSmoothJoin(0, 1, nPntMax-1);
            }
        }
    }
    if(bBCFlag)
    {
        ImpSetClosed(!bClosed); // neuen ObjKind setzen
        ImpForceKind(); // wg. Line->Poly->PolyLine statt Line->Poly->Line
        SetRectsDirty();
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
    }
}

XPolyFlags SdrPathObj::GetSmoothFlag(const SdrHdl* pHdl) const
{
    XPolyFlags eRet=XPOLY_NORMAL;
    if (pHdl!=NULL) {
        USHORT nPnt=pHdl->GetPointNum();
        const XPolygon& rXPoly=aPathPolygon[pHdl->GetPolyNum()];
        eRet=rXPoly.GetFlags(nPnt);
    }
    return eRet;
}

void SdrPathObj::ImpSetSmoothFlag(USHORT nPolyNum, USHORT nPointNum, XPolyFlags eFlag)
{
    if (eFlag==XPOLY_NORMAL || eFlag==XPOLY_SMOOTH || eFlag==XPOLY_SYMMTR) {
        FASTBOOL bClosed=IsClosed();
        USHORT nPnt=nPointNum;
        XPolygon& rXPoly=aPathPolygon[nPolyNum];
        USHORT nPntMax=rXPoly.GetPointCount();
        if (nPntMax==0) return;
        nPntMax--;
        rXPoly.SetFlags(nPnt,eFlag);
        if (bClosed && nPnt==0) rXPoly.SetFlags(nPntMax,eFlag);

        if (eFlag!=XPOLY_NORMAL) {
            USHORT nPrev=nPnt;
            USHORT nNext=nPnt+1;

            if (nPrev==0 && bClosed) nPrev=nPntMax;
            if (nNext>nPntMax && bClosed) nNext=1;

            if (nPrev>0 && nNext<=nPntMax) {
                nPrev--;
                FASTBOOL bPrevIsBez=rXPoly.IsControl(nPrev);
                FASTBOOL bNextIsBez=rXPoly.IsControl(nNext);

                if (bPrevIsBez || bNextIsBez) {
                    if (bPrevIsBez && bNextIsBez) {
                        rXPoly.CalcTangent(nPnt,nPrev,nNext);
                    } else {
                        rXPoly.CalcSmoothJoin(nPnt,nPrev,nNext);
                    }
                    if (bClosed) {
                        if (nPnt==0) rXPoly.SetFlags(nPntMax,eFlag);
                        else if (nPnt==nPntMax) rXPoly.SetFlags(0,eFlag);
                    }
                }
            }
        }
    }
}

void SdrPathObj::NbcSetSmoothFlag(const SdrHdl* pHdl, XPolyFlags eFlag)
{
    if (pHdl!=NULL) {
        ImpSetSmoothFlag(pHdl->GetPolyNum(),pHdl->GetPointNum(),eFlag);
        ImpForceKind();
        SetRectsDirty();
    }
}

void SdrPathObj::SetSmoothFlag(const SdrHdl* pHdl, XPolyFlags eFlag)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetSmoothFlag(pHdl,eFlag);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

SdrPathType SdrPathObj::CanConvertSegment(const SdrHdl* pHdl) const
{
    SdrPathType ePathType=SDRPATH_NONE;
    if (pHdl!=NULL) {
        const XPolygon& rXPoly=aPathPolygon[pHdl->GetPolyNum()];
        USHORT nPnt=pHdl->GetPointNum();
        USHORT nPntMax=rXPoly.GetPointCount();
        if (nPntMax>0) {
            nPntMax--;
            if (nPnt<nPntMax) {
                if (rXPoly.IsControl(nPnt+1)) {
                    ePathType=SDRPATH_LINE;
                } else {
                    ePathType=SDRPATH_CURVE;
                }
            }
        }
    }
    return ePathType;
}

SdrPathType SdrPathObj::GetSegmentType(const SdrHdl* pHdl) const
{
    SdrPathType ePathType=SDRPATH_NONE;
    if (pHdl!=NULL) {
        const XPolygon& rXPoly=aPathPolygon[pHdl->GetPolyNum()];
        USHORT nPnt=pHdl->GetPointNum();
        USHORT nPntMax=rXPoly.GetPointCount();
        if (nPntMax>0) {
            nPntMax--;
            if (nPnt<nPntMax) {
                if (rXPoly.IsControl(nPnt+1)) {
                    ePathType=SDRPATH_CURVE;
                } else {
                    ePathType=SDRPATH_LINE;
                }
            }
        }
    }
    return ePathType;
}

void SdrPathObj::ConvertSegment(const SdrHdl* pHdl)
{
    if (pHdl==NULL) return;
    XPolygon& rXPoly = aPathPolygon[pHdl->GetPolyNum()];
    USHORT nP1=pHdl->GetPointNum();
    USHORT nPntMax=rXPoly.GetPointCount();
    if (nPntMax==0) return;
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    nPntMax--;
    if (nP1<nPntMax) {
        USHORT nP2=nP1+1;

        SendRepaintBroadcast();

        if (rXPoly.IsControl(nP2)) {
            rXPoly.Remove(nP2, 2);
            nPntMax -= 2;
        } else {
            Point aDiff = (rXPoly[nP2] - rXPoly[nP1]) / 3;
            rXPoly.Insert(nP2, rXPoly[nP1] + aDiff, XPOLY_CONTROL);
            aDiff *= 2;
            rXPoly.Insert(nP2+1, rXPoly[nP1] + aDiff, XPOLY_CONTROL);
            nP2 = nP1 + 3;
            nPntMax += 2;
        }

        if (rXPoly.IsSmooth(nP1)) {
            USHORT nPrev = nP1;
            if (nP1>0)           nPrev--;
            else if (IsClosed()) nPrev=nPntMax-1;
            if (nPrev!=nP1 && (rXPoly.IsControl(nPrev) || rXPoly.IsControl(nP1+1))) {
                if (rXPoly.IsControl(nPrev) && rXPoly.IsControl(nP1+1)) {
                    rXPoly.CalcTangent(nP1,nPrev,nP1+1);
                } else {
                    rXPoly.CalcSmoothJoin(nP1,nPrev,nP1+1);
                }
            }
        }
        if (rXPoly.IsSmooth(nP2)) {
            USHORT nNext=nP2;
            if (nP2<nPntMax) nNext++;
            else if (IsClosed()) nNext=1;
            if (nNext!=nP2 && (rXPoly.IsControl(nP2-1) || rXPoly.IsControl(nNext))) {
                if (rXPoly.IsControl(nP2-1) && rXPoly.IsControl(nNext)) {
                    rXPoly.CalcTangent(nP2,nP2-1,nNext);
                } else {
                    rXPoly.CalcSmoothJoin(nP2,nP2-1,nNext);
                }
            }
        }
        ImpForceKind();
        SetRectsDirty();
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrPathObj::ImpConvertSegment(USHORT nPolyNum, USHORT nPointNum, SdrPathType ePathType, FASTBOOL bIgnoreSmooth)
{
    XPolygon& rXPoly=aPathPolygon[nPolyNum];
    USHORT nP1=nPointNum;
    USHORT nPntMax=rXPoly.GetPointCount();
    if (nPntMax==0) return;
    nPntMax--;
    if (nP1<nPntMax) {
        USHORT nP2=nP1+1;
        FASTBOOL bIsCurve=rXPoly.IsControl(nP2);
        if (ePathType!=SDRPATH_NONE) {
            if ((bIsCurve && ePathType==SDRPATH_CURVE) ||
                (!bIsCurve && ePathType==SDRPATH_LINE)) return; // Wandlung nicht erforderlich
        }
        if (bIsCurve) {
            rXPoly.Remove(nP2,2);
            nPntMax-=2;
        } else {
            Point aDiff=(rXPoly[nP2]-rXPoly[nP1])/3;
            rXPoly.Insert(nP2,rXPoly[nP1]+aDiff,XPOLY_CONTROL);
            aDiff*=2;
            rXPoly.Insert(nP2+1,rXPoly[nP1]+aDiff,XPOLY_CONTROL);
            nP2=nP1+3;
            nPntMax+=2;
        }
        if (!bIgnoreSmooth) {
            if (rXPoly.IsSmooth(nP1)) {
                USHORT nPrev=nP1;
                if (nP1>0) nPrev--;
                else if (IsClosed()) nPrev=nPntMax-1;
                if (nPrev!=nP1 && (rXPoly.IsControl(nPrev) || rXPoly.IsControl(nP1+1))) {
                    if (rXPoly.IsControl(nPrev) && rXPoly.IsControl(nP1+1)) {
                        rXPoly.CalcTangent(nP1, nPrev, nP1+1);
                    } else {
                        rXPoly.CalcSmoothJoin(nP1,nPrev,nP1+1);
                    }
                }
            }
            if (rXPoly.IsSmooth(nP2)) {
                USHORT nNext=nP2;
                if (nP2<nPntMax) nNext++;
                else if (IsClosed()) nNext=1;
                if (nNext!=nP2 && (rXPoly.IsControl(nP2-1) || rXPoly.IsControl(nNext))) {
                    if (rXPoly.IsControl(nP2-1) && rXPoly.IsControl(nNext)) {
                        rXPoly.CalcTangent(nP2,nP2-1,nNext);
                    } else {
                        rXPoly.CalcSmoothJoin(nP2,nP2-1,nNext);
                    }
                }
            }
        }
    }
}

void SdrPathObj::NbcConvertSegment(const SdrHdl* pHdl, SdrPathType ePathType, FASTBOOL bIgnoreSmooth)
{
    if (pHdl!=NULL) {
        ImpConvertSegment(pHdl->GetPolyNum(),pHdl->GetPointNum(),ePathType,bIgnoreSmooth);
        ImpForceKind(); // ebenso impl. an der SdrPolyEditView
        SetRectsDirty();
    }
}

void SdrPathObj::ConvertSegment(const SdrHdl* pHdl, SdrPathType ePathType, FASTBOOL bIgnoreSmooth)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcConvertSegment(pHdl,ePathType,bIgnoreSmooth);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrPathObj::ConvertAllSegments(SdrPathType ePathType)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    FASTBOOL bBroadcastFlg=FALSE;
    FASTBOOL bClosed=IsClosed();
    // von hinten anfangen, da evtl. Punkte geloescht oder eingefuegt werden
    USHORT nPoly=aPathPolygon.Count();
    while (nPoly>0) {
        nPoly--;
        XPolygon& rXPoly=aPathPolygon[nPoly];
        USHORT nPnt=rXPoly.GetPointCount()-1;
        FASTBOOL bSmoothFlg=FALSE;

        while (nPnt>0) {
            if (rXPoly.IsControl(nPnt-1)) {
                if (nPnt<3) {
                    nPnt=0; // enddeckt durch #35912#. Nun Sicherheitsabfrage
                    DBG_ERROR("SdrPathObj::ConvertAllSegments(): Ungueltige Kontrollpunktanordnung endeckt!");
                } else {
                    nPnt-=3;
                }
                if (ePathType==SDRPATH_LINE || ePathType==SDRPATH_NONE) {
                    if (!bBroadcastFlg) { SendRepaintBroadcast(); bBroadcastFlg=TRUE; }
                    bSmoothFlg=TRUE;
                    ImpConvertSegment(nPoly,nPnt,ePathType,TRUE);
                }
            } else {
                nPnt--;
                if (ePathType==SDRPATH_CURVE || ePathType==SDRPATH_NONE) {
                    if (!bBroadcastFlg) { SendRepaintBroadcast(); bBroadcastFlg=TRUE; }
                    bSmoothFlg=TRUE;
                    ImpConvertSegment(nPoly,nPnt,ePathType,TRUE);
                }
            }
        }
        if (bSmoothFlg) { // und nun die Kontrollpunkte nach Smoothbedingung korregieren
            nPnt=rXPoly.GetPointCount();
            FASTBOOL bLast=TRUE;
            FASTBOOL bLastIsCurve=FALSE;
            while (nPnt>0) {
                FASTBOOL bCurve=rXPoly.IsControl(nPnt-1);
                if (bCurve) {
                    if (nPnt<3) {
                        nPnt=0; // enddeckt durch #35912#. Nun Sicherheitsabfrage
                        DBG_ERROR("SdrPathObj::ConvertAllSegments(): Ungueltige Kontrollpunktanordnung endeckt!");
                    } else {
                        nPnt-=3;
                    }
                } else nPnt--;
                if (bLast) {
                    bLast=FALSE; // den Letzten Punkt ignorieren, denn da hat Smooth eh keinen Einfluss
                    bLastIsCurve=bCurve;
                } else {
                    if (rXPoly.IsSmooth(nPnt) &&
                        (bCurve ||                                   // dahinter eine Kurve
                         (nPnt>0 && rXPoly.IsControl(nPnt-1)) ||     // oder davor eine Kurve
                          (bClosed && nPnt==0 && bLastIsCurve)))
                    {
                        XPolyFlags eSmooth=rXPoly.GetFlags(nPnt);
                        rXPoly.SetFlags(nPnt,XPOLY_NORMAL); // damit ImpSetSmoothFlag() was tut
                        ImpSetSmoothFlag(nPoly,nPnt,eSmooth);
                    }
                }
            }
        }
    }
    if (bBroadcastFlg) {
        ImpForceKind();
        SetRectsDirty();
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

