/*************************************************************************
 *
 *  $RCSfile: svdoedge.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: aw $ $Date: 2001-03-09 17:07:18 $
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

#include "svdoedge.hxx"
#include "xpool.hxx"
#include "xpoly.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdpagv.hxx"
#include "svdview.hxx"
#include "svdxout.hxx"
#include "svddrag.hxx"
#include "svddrgv.hxx"
#include "svddrgm1.hxx"
#include "svdhdl.hxx"
#include "svdtouch.hxx"
#include "svdtrans.hxx"
#include "svdetc.hxx"
#include "svdio.hxx"
#include "svdsuro.hxx"
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjConnection::~SdrObjConnection()
{
    if (pSuro!=NULL) delete pSuro;
}

void SdrObjConnection::ResetVars()
{
    pSuro=NULL;
    pObj=NULL;
    nConId=0;
    nXDist=0;
    nYDist=0;
    bBestConn=TRUE;
    bBestVertex=TRUE;
    bXDistOvr=FALSE;
    bYDistOvr=FALSE;
    bAutoVertex=FALSE;
    bAutoCorner=FALSE;
}

FASTBOOL SdrObjConnection::TakeGluePoint(SdrGluePoint& rGP, FASTBOOL bSetAbsPos) const
{
    FASTBOOL bRet=FALSE;
    if (pObj!=NULL) { // Ein Obj muss schon angedockt sein!
        if (bAutoVertex) {
            rGP=pObj->GetVertexGluePoint(nConId);
            bRet=TRUE;
        } else if (bAutoCorner) {
            rGP=pObj->GetCornerGluePoint(nConId);
            bRet=TRUE;
        } else {
            const SdrGluePointList* pGPL=pObj->GetGluePointList();
            if (pGPL!=NULL) {
                USHORT nNum=pGPL->FindGluePoint(nConId);
                if (nNum!=SDRGLUEPOINT_NOTFOUND) {
                    rGP=(*pGPL)[nNum];
                    bRet=TRUE;
                }
            }
        }
    }
    if (bRet && bSetAbsPos) {
        Point aPt(rGP.GetAbsolutePos(*pObj));
        aPt+=aObjOfs;
        rGP.SetPos(aPt);
    }
    return bRet;
}

void SdrObjConnection::Write(SvStream& rOut, const SdrObject* pEdgeObj) const
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOConnID); // ab V11 eingepackt
    SdrObjSurrogate aSuro(pObj,pEdgeObj);
    rOut<<aSuro;
    rOut<<nConId;
    rOut<<nXDist;
    rOut<<nYDist;
    BOOL bTmp;
    bTmp=bBestConn;   rOut<<bTmp;
    bTmp=bBestVertex; rOut<<bTmp;
    bTmp=bXDistOvr;   rOut<<bTmp;
    bTmp=bYDistOvr;   rOut<<bTmp;
    bTmp=bAutoVertex; rOut<<bTmp;
    bTmp=bAutoCorner; rOut<<bTmp;
    UINT32 nReserve=0;
    rOut<<nReserve;
    rOut<<nReserve;
}

void SdrObjConnection::Read(SvStream& rIn, const SdrObject* pEdgeObj)
{
    if (rIn.GetError()!=0) return;
    SdrIOHeader aHead(rIn,STREAM_READ,SdrIOConnID); // ab V11 eingepackt
    pSuro=new SdrObjSurrogate(*pEdgeObj,rIn);
    BOOL bBit;
    rIn>>nConId;
    rIn>>nXDist;
    rIn>>nYDist;
    rIn>>bBit; bBestConn  =bBit;
    rIn>>bBit; bBestVertex=bBit;
    rIn>>bBit; bXDistOvr  =bBit;
    rIn>>bBit; bYDistOvr  =bBit;
    rIn>>bBit; bAutoVertex=bBit;
    rIn>>bBit; bAutoCorner=bBit;
    UINT32 nReserve;
    rIn>>nReserve;
    rIn>>nReserve;
}

void SdrObjConnection::ReadTilV10(SvStream& rIn, const SdrObject* pEdgeObj)
{
    if (rIn.GetError()!=0) return;
    pSuro=new SdrObjSurrogate(*pEdgeObj,rIn);
    BOOL bBit;
    rIn>>nConId;
    rIn>>nXDist;
    rIn>>nYDist;
    rIn>>bBit; bBestConn  =bBit;
    rIn>>bBit; bBestVertex=bBit;
    rIn>>bBit; bXDistOvr  =bBit;
    rIn>>bBit; bYDistOvr  =bBit;
    rIn>>bBit; bAutoVertex=bBit;
    rIn>>bBit; bAutoCorner=bBit;
    UINT32 nReserve;
    rIn>>nReserve;
    rIn>>nReserve;
}

void SdrObjConnection::AfterRead(const SdrObject* pEdgeObj)
{
    if (pSuro!=NULL) {
        pObj=pSuro->GetObject();
        delete pSuro;
        pSuro=NULL;
    }
}

Point& SdrEdgeInfoRec::ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode)
{
    switch (eLineCode) {
        case OBJ1LINE2 : return aObj1Line2;
        case OBJ1LINE3 : return aObj1Line3;
        case OBJ2LINE2 : return aObj2Line2;
        case OBJ2LINE3 : return aObj2Line3;
        case MIDDLELINE: return aMiddleLine;
    } // switch
    return aMiddleLine;
}

USHORT SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    switch (eLineCode) {
        case OBJ1LINE2 : return 1;
        case OBJ1LINE3 : return 2;
        case OBJ2LINE2 : return rXP.GetPointCount()-3;
        case OBJ2LINE3 : return rXP.GetPointCount()-4;
        case MIDDLELINE: return nMiddleLine;
    } // switch
    return 0;
}

FASTBOOL SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    USHORT nIdx=ImpGetPolyIdx(eLineCode,rXP);
    FASTBOOL bHorz=nAngle1==0 || nAngle1==18000;
    if (eLineCode==OBJ2LINE2 || eLineCode==OBJ2LINE3) {
        nIdx=rXP.GetPointCount()-nIdx; // #36314#
        bHorz=nAngle2==0 || nAngle2==18000; // #52000#
    }
    if ((nIdx & 1)==1) bHorz=!bHorz;
    return bHorz;
}

void SdrEdgeInfoRec::ImpSetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP, long nVal)
{
    Point& rPt=ImpGetLineVersatzPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP)) rPt.Y()=nVal;
    else rPt.X()=nVal;
}

long SdrEdgeInfoRec::ImpGetLineVersatz(SdrEdgeLineCode eLineCode, const XPolygon& rXP) const
{
    const Point& rPt=ImpGetLineVersatzPoint(eLineCode);
    if (ImpIsHorzLine(eLineCode,rXP)) return rPt.Y();
    else return rPt.X();
}

SvStream& operator<<(SvStream& rOut, const SdrEdgeInfoRec& rEI)
{
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrEdgeInfoRec");
#endif
    rOut<<rEI.aObj1Line2;
    rOut<<rEI.aObj1Line3;
    rOut<<rEI.aObj2Line2;
    rOut<<rEI.aObj2Line3;
    rOut<<rEI.aMiddleLine;
    rOut<<rEI.nAngle1;
    rOut<<rEI.nAngle2;
    rOut<<rEI.nObj1Lines;
    rOut<<rEI.nObj2Lines;
    rOut<<rEI.nMiddleLine;
    rOut<<rEI.cOrthoForm;
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrEdgeInfoRec& rEI)
{
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrEdgeInfoRec");
#endif
    rIn>>rEI.aObj1Line2;
    rIn>>rEI.aObj1Line3;
    rIn>>rEI.aObj2Line2;
    rIn>>rEI.aObj2Line3;
    rIn>>rEI.aMiddleLine;
    rIn>>rEI.nAngle1;
    rIn>>rEI.nAngle2;
    rIn>>rEI.nObj1Lines;
    rIn>>rEI.nObj2Lines;
    rIn>>rEI.nMiddleLine;
    rIn>>rEI.cOrthoForm;
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrEdgeObj,SdrTextObj);

SdrEdgeObj::SdrEdgeObj():
    SdrTextObj()
{
    bClosedObj=FALSE;
    bIsEdge=TRUE;
    bEdgeTrackDirty=FALSE;
    nNotifyingCount=0;
    pEdgeTrack=new XPolygon;
}

SdrEdgeObj::~SdrEdgeObj()
{
    DisconnectFromNode(TRUE);
    DisconnectFromNode(FALSE);
    delete pEdgeTrack;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

SfxItemSet* SdrEdgeObj::CreateNewItemSet(SfxItemPool& rPool)
{
    // include ALL items, 2D and 3D
    return new SfxItemSet(rPool,
        // ranges from SdrAttrObj
        SDRATTR_START, SDRATTRSET_SHADOW,
        SDRATTRSET_OUTLINER, SDRATTRSET_MISC,

        // edge attributes
        SDRATTR_EDGE_FIRST, SDRATTRSET_EDGE,

        // outliner and end
        EE_ITEMS_START, EE_ITEMS_END,
        0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access
void SdrEdgeObj::ItemSetChanged()
{
    // call parent
    SdrTextObj::ItemSetChanged();

    // local changes
    ImpSetAttrToEdgeInfo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEdgeObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    SdrTextObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
    ImpSetAttrToEdgeInfo(); // Werte vom Pool nach aEdgeInfo kopieren
}

void SdrEdgeObj::ImpSetAttrToEdgeInfo()
{
    const SfxItemSet& rSet = GetItemSet();
    SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
        sal_uInt16 n = 0;

        if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ1LINE2, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ1LINE3, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(MIDDLELINE, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ2LINE3, *pEdgeTrack, nVals[n]);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
        {
            aEdgeInfo.ImpSetLineVersatz(OBJ2LINE2, *pEdgeTrack, nVals[n]);
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        BOOL bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
        BOOL bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;

        if(bHor1)
        {
            aEdgeInfo.aObj1Line2.X() = nVal1;
        }
        else
        {
            aEdgeInfo.aObj1Line2.Y() = nVal1;
        }

        if(bHor2)
        {
            aEdgeInfo.aObj2Line2.X() = nVal2;
        }
        else
        {
            aEdgeInfo.aObj2Line2.Y() = nVal2;
        }
    }

    // #84649#
    bEdgeTrackDirty = TRUE;
}

void SdrEdgeObj::ImpSetEdgeInfoToAttr()
{
    const SfxItemSet& rSet = GetItemSet();
    SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
    sal_Int32 nValAnz = ((SdrEdgeLineDeltaAnzItem&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();
    sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
    sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
    sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
    sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
    sal_uInt16 n = 0;

    if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
    {
        if(aEdgeInfo.nObj1Lines >= 2 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE2, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj1Lines >= 3 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ1LINE3, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(MIDDLELINE, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 3 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE3, *pEdgeTrack);
            n++;
        }

        if(aEdgeInfo.nObj2Lines >= 2 && n < 3)
        {
            nVals[n] = aEdgeInfo.ImpGetLineVersatz(OBJ2LINE2, *pEdgeTrack);
            n++;
        }
    }
    else if(eKind == SDREDGE_THREELINES)
    {
        BOOL bHor1 = aEdgeInfo.nAngle1 == 0 || aEdgeInfo.nAngle1 == 18000;
        BOOL bHor2 = aEdgeInfo.nAngle2 == 0 || aEdgeInfo.nAngle2 == 18000;

        n = 2;
        nVals[0] = bHor1 ? aEdgeInfo.aObj1Line2.X() : aEdgeInfo.aObj1Line2.Y();
        nVals[1] = bHor2 ? aEdgeInfo.aObj2Line2.X() : aEdgeInfo.aObj2Line2.Y();
    }

    if(n != nValAnz || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3)
    {
        if(n != nValAnz)
            SetItem(SdrEdgeLineDeltaAnzItem(n));

        if(nVals[0] != nVal1)
            SetItem(SdrEdgeLine1DeltaItem(nVals[0]));

        if(nVals[1] != nVal2)
            SetItem(SdrEdgeLine2DeltaItem(nVals[1]));

        if(nVals[2] != nVal3)
            SetItem(SdrEdgeLine3DeltaItem(nVals[2]));

        if(n < 3)
            ClearItem(SDRATTR_EDGELINE3DELTA);

        if(n < 2)
            ClearItem(SDRATTR_EDGELINE2DELTA);

        if(n < 1)
            ClearItem(SDRATTR_EDGELINE1DELTA);
    }
}

void SdrEdgeObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    FASTBOOL bCanConv=!HasText() || ImpCanConvTextToCurve();
    rInfo.bCanConvToPath=bCanConv;
    rInfo.bCanConvToPoly=bCanConv;
    rInfo.bCanConvToContour = (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

UINT16 SdrEdgeObj::GetObjIdentifier() const
{
    return UINT16(OBJ_EDGE);
}

const Rectangle& SdrEdgeObj::GetBoundRect() const
{
    if (bEdgeTrackDirty) {
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    }
    return SdrTextObj::GetBoundRect();
}

const Rectangle& SdrEdgeObj::GetSnapRect() const
{
    if (bEdgeTrackDirty) {
        ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    }
    return SdrTextObj::GetSnapRect();
}

void SdrEdgeObj::RecalcSnapRect()
{
    maSnapRect=pEdgeTrack->GetBoundRect();
}

void SdrEdgeObj::RecalcBoundRect()
{
    aOutRect=GetSnapRect();
    long nLineWdt=ImpGetLineWdt();
    // Linienenden beruecksichtigen
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

void SdrEdgeObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    rRect=GetSnapRect();
}

FASTBOOL SdrEdgeObj::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Hidden objects on masterpages, draw nothing
    if((rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE) && bNotVisibleAsMaster)
        return TRUE;

    BOOL bHideContour(IsHideContour());
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
    if(!bHideContour && ImpSetShadowAttributes(rXOut,TRUE))
    {
        UINT32 nXDist=((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
        UINT32 nYDist=((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
        XPolygon aXP(*pEdgeTrack);
        aXP.Move(nXDist,nYDist);

        // avoid shadow line drawing in XOut
        rXOut.SetLineAttr(aEmptySet);

        rXOut.DrawXPolyLine(aXP);

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
        if(bHideContour)
        {
            rXOut.SetFillAttr(rSet);
        }
    }

    if (!bHideContour) {
        FASTBOOL bDraw=TRUE;
        if (bDraw) rXOut.DrawXPolyLine(*pEdgeTrack);
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

SdrObject* SdrEdgeObj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    INT32 nMyTol=nTol;

    INT32 nWdt=ImpGetLineWdt()/2; // Halbe Strichstaerke
    if (nWdt>nMyTol) nMyTol=nWdt; // Bei dicker Linie keine Toleranz noetig
    Rectangle aR(rPnt,rPnt);
    aR.Left()  -=nMyTol;
    aR.Right() +=nMyTol;
    aR.Top()   -=nMyTol;
    aR.Bottom()+=nMyTol;

    FASTBOOL bHit=FALSE;

    Polygon aPoly=XOutCreatePolygon(*pEdgeTrack,NULL);
    bHit=IsRectTouchesLine(aPoly,aR);
    if (!bHit && HasText()) bHit=SdrTextObj::CheckHit(rPnt,nTol,pVisiLayer)!=NULL;
    return bHit ? (SdrObject*)this : NULL;
}

FASTBOOL SdrEdgeObj::IsNode() const
{
    return TRUE;
}

SdrGluePoint SdrEdgeObj::GetVertexGluePoint(USHORT nNum) const
{
    Point aPt;
    USHORT nPntAnz=pEdgeTrack->GetPointCount();
    if (nPntAnz>0) {
        Point aOfs(GetSnapRect().Center());
        if (nNum==2 && GetConnectedNode(TRUE)==NULL) aPt=(*pEdgeTrack)[0];
        else if (nNum==3 && GetConnectedNode(FALSE)==NULL) aPt=(*pEdgeTrack)[nPntAnz-1];
        else {
            if ((nPntAnz & 1) ==1) {
                aPt=(*pEdgeTrack)[nPntAnz/2];
            } else {
                Point aPt1((*pEdgeTrack)[nPntAnz/2-1]);
                Point aPt2((*pEdgeTrack)[nPntAnz/2]);
                aPt1+=aPt2;
                aPt1.X()/=2;
                aPt1.Y()/=2;
                aPt=aPt1;
            }
        }
        aPt-=aOfs;
    }
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(FALSE);
    return aGP;
}

SdrGluePoint SdrEdgeObj::GetCornerGluePoint(USHORT nNum) const
{
    return GetVertexGluePoint(nNum);
}

const SdrGluePointList* SdrEdgeObj::GetGluePointList() const
{
    return NULL; // Keine benutzerdefinierten Klebepunkte fuer Verbinder #31671#
}

SdrGluePointList* SdrEdgeObj::ForceGluePointList()
{
    return NULL; // Keine benutzerdefinierten Klebepunkte fuer Verbinder #31671#
}

FASTBOOL SdrEdgeObj::IsEdge() const
{
    return TRUE;
}

void SdrEdgeObj::ConnectToNode(FASTBOOL bTail1, SdrObject* pObj)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    DisconnectFromNode(bTail1);
    if (pObj!=NULL) {
        pObj->AddListener(*this);
        rCon.pObj=pObj;
        bEdgeTrackDirty=TRUE;
    }
}

void SdrEdgeObj::DisconnectFromNode(FASTBOOL bTail1)
{
    SdrObjConnection& rCon=GetConnection(bTail1);
    if (rCon.pObj!=NULL) {
        rCon.pObj->RemoveListener(*this);
        rCon.pObj=NULL;
    }
}

SdrObject* SdrEdgeObj::GetConnectedNode(FASTBOOL bTail1) const
{
    SdrObject* pObj=GetConnection(bTail1).pObj;
    if (pObj!=NULL && (pObj->GetPage()!=pPage || !pObj->IsInserted())) pObj=NULL;
    return pObj;
}

FASTBOOL SdrEdgeObj::CheckNodeConnection(FASTBOOL bTail1) const
{
    FASTBOOL bRet=FALSE;
    const SdrObjConnection& rCon=GetConnection(bTail1);
    USHORT nPtAnz=pEdgeTrack->GetPointCount();
    if (rCon.pObj!=NULL && rCon.pObj->GetPage()==pPage && nPtAnz!=0) {
        const SdrGluePointList* pGPL=rCon.pObj->GetGluePointList();
        USHORT nConAnz=pGPL==NULL ? 0 : pGPL->GetCount();
        USHORT nGesAnz=nConAnz+8;
        Point aTail(bTail1 ? (*pEdgeTrack)[0] : (*pEdgeTrack)[USHORT(nPtAnz-1)]);
        for (USHORT i=0; i<nGesAnz && !bRet; i++) {
            if (i<nConAnz) { // UserDefined
                bRet=aTail==(*pGPL)[i].GetAbsolutePos(*rCon.pObj);
            } else if (i<nConAnz+4) { // Vertex
                SdrGluePoint aPt(rCon.pObj->GetVertexGluePoint(i-nConAnz));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.pObj);
            } else {                  // Corner
                SdrGluePoint aPt(rCon.pObj->GetCornerGluePoint(i-nConAnz-4));
                bRet=aTail==aPt.GetAbsolutePos(*rCon.pObj);
            }
        }
    }
    return bRet;
}

void SdrEdgeObj::ImpSetTailPoint(FASTBOOL bTail1, const Point& rPt)
{
    USHORT nPtAnz=pEdgeTrack->GetPointCount();
    if (nPtAnz==0) {
        (*pEdgeTrack)[0]=rPt;
        (*pEdgeTrack)[1]=rPt;
    } else if (nPtAnz==1) {
        if (!bTail1) (*pEdgeTrack)[1]=rPt;
        else { (*pEdgeTrack)[1]=(*pEdgeTrack)[0]; (*pEdgeTrack)[0]=rPt; }
    } else {
        if (!bTail1) (*pEdgeTrack)[USHORT(nPtAnz-1)]=rPt;
        else (*pEdgeTrack)[0]=rPt;
    }
    ImpRecalcEdgeTrack();
    SetRectsDirty();
}

void SdrEdgeObj::ImpUndirtyEdgeTrack()
{
    if (bEdgeTrackDirty) {
        ImpRecalcEdgeTrack();
    }
}

void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=aOutRect; // war vorher =GetBoundRect() #36431#
    SetRectsDirty();
    if (!bEdgeTrackDirty) SendRepaintBroadcast();
    *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
    ImpSetEdgeInfoToAttr(); // Die Werte aus aEdgeInfo in den Pool kopieren
    bEdgeTrackDirty=FALSE;
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

USHORT SdrEdgeObj::ImpCalcEscAngle(SdrObject* pObj, const Point& rPt) const
{
    if (pObj==NULL) return SDRESC_ALL;
    Rectangle aR(pObj->GetSnapRect());
    long dxl=rPt.X()-aR.Left();
    long dyo=rPt.Y()-aR.Top();
    long dxr=aR.Right()-rPt.X();
    long dyu=aR.Bottom()-rPt.Y();
    FASTBOOL bxMitt=Abs(dxl-dxr)<2;
    FASTBOOL byMitt=Abs(dyo-dyu)<2;
    long dx=Min(dxl,dxr);
    long dy=Min(dyo,dyu);
    FASTBOOL bDiag=Abs(dx-dy)<2;
    if (bxMitt && byMitt) return SDRESC_ALL; // In der Mitte
    if (bDiag) {  // diagonal
        USHORT nRet=0;
        if (byMitt) nRet|=SDRESC_VERT;
        if (bxMitt) nRet|=SDRESC_HORZ;
        if (dxl<dxr) { // Links
            if (dyo<dyu) nRet|=SDRESC_LEFT | SDRESC_TOP;
            else nRet|=SDRESC_LEFT | SDRESC_BOTTOM;
        } else {       // Rechts
            if (dyo<dyu) nRet|=SDRESC_RIGHT | SDRESC_TOP;
            else nRet|=SDRESC_RIGHT | SDRESC_BOTTOM;
        }
        return nRet;
    }
    if (dx<dy) { // waagerecht
        if (bxMitt) return SDRESC_HORZ;
        if (dxl<dxr) return SDRESC_LEFT;
        else return SDRESC_RIGHT;
    } else {     // senkrecht
        if (byMitt) return SDRESC_VERT;
        if (dyo<dyu) return SDRESC_TOP;
        else return SDRESC_BOTTOM;
    }
}

FASTBOOL SdrEdgeObj::ImpStripPolyPoints(XPolygon& rXP) const
{
    // fehlende Implementation !!!
    return FALSE;
}

XPolygon SdrEdgeObj::ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rMeeting) const
{
    XPolygon aXP;
    aXP.Insert(XPOLY_APPEND,rStPt,XPOLY_NORMAL);
    FASTBOOL bRts=nEscAngle==0;
    FASTBOOL bObn=nEscAngle==9000;
    FASTBOOL bLks=nEscAngle==18000;
    FASTBOOL bUnt=nEscAngle==27000;
    FASTBOOL bHor=bLks || bRts;
    FASTBOOL bVer=bObn || bUnt;

    Point aP1(rStPt); // erstmal den Pflichtabstand
    if (bLks) aP1.X()=rRect.Left();
    if (bRts) aP1.X()=rRect.Right();
    if (bObn) aP1.Y()=rRect.Top();
    if (bUnt) aP1.Y()=rRect.Bottom();

    FASTBOOL bFinish=FALSE;
    if (!bFinish) {
        Point aP2(aP1); // Und nun den Pflichtabstand ggf. bis auf Meetinghoehe erweitern
        if (bLks && rMeeting.X()<=aP2.X()) aP2.X()=rMeeting.X();
        if (bRts && rMeeting.X()>=aP2.X()) aP2.X()=rMeeting.X();
        if (bObn && rMeeting.Y()<=aP2.Y()) aP2.Y()=rMeeting.Y();
        if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.Y()=rMeeting.Y();
        aXP.Insert(XPOLY_APPEND,aP2,XPOLY_NORMAL);

        Point aP3(aP2);
        if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // Aussenrum
            if (rMeeting.Y()<aP2.Y()) {
                aP3.Y()=rRect.Top();
                if (rMeeting.Y()<aP3.Y()) aP3.Y()=rMeeting.Y();
            } else {
                aP3.Y()=rRect.Bottom();
                if (rMeeting.Y()>aP3.Y()) aP3.Y()=rMeeting.Y();
            }
            aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
            if (aP3.Y()!=rMeeting.Y()) {
                aP3.X()=rMeeting.X();
                aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
            }
        }
        if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // Aussenrum
            if (rMeeting.X()<aP2.X()) {
                aP3.X()=rRect.Left();
                if (rMeeting.X()<aP3.X()) aP3.X()=rMeeting.X();
            } else {
                aP3.X()=rRect.Right();
                if (rMeeting.X()>aP3.X()) aP3.X()=rMeeting.X();
            }
            aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
            if (aP3.X()!=rMeeting.X()) {
                aP3.Y()=rMeeting.Y();
                aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
            }
        }
    }
#ifdef DBG_UTIL
    if (aXP.GetPointCount()>4) {
        DBG_ERROR("SdrEdgeObj::ImpCalcObjToCenter(): Polygon hat mehr als 4 Punkte!");
    }
#endif
    return aXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const XPolygon& rTrack0, SdrObjConnection& rCon1, SdrObjConnection& rCon2, SdrEdgeInfoRec* pInfo) const
{
    Point aPt1,aPt2;
    SdrGluePoint aGP1,aGP2;
    USHORT nEsc1=SDRESC_ALL,nEsc2=SDRESC_ALL;
    Rectangle aBoundRect1;
    Rectangle aBoundRect2;
    Rectangle aBewareRect1;
    Rectangle aBewareRect2;
    // Erstmal die alten Endpunkte wiederholen
    if (rTrack0.GetPointCount()!=0) {
        aPt1=rTrack0[0];
        USHORT nSiz=rTrack0.GetPointCount();
        nSiz--;
        aPt2=rTrack0[nSiz];
    } else {
        if (!aOutRect.IsEmpty()) {
            aPt1=aOutRect.TopLeft();
            aPt2=aOutRect.BottomRight();
        }
    }
    FASTBOOL bCon1=rCon1.pObj!=NULL && rCon1.pObj->GetPage()==pPage && rCon1.pObj->IsInserted();
    FASTBOOL bCon2=rCon2.pObj!=NULL && rCon2.pObj->GetPage()==pPage && rCon2.pObj->IsInserted();
    const SfxItemSet& rSet = GetItemSet();

    if (bCon1) {
        if (rCon1.pObj==(SdrObject*)this) { // sicherheitshalber Abfragen #44515#
            aBoundRect1=aOutRect;
        } else {
            aBoundRect1=rCon1.pObj->GetBoundRect();
        }
        aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
        aBewareRect1=aBoundRect1;

        sal_Int32 nH = ((SdrEdgeNode1HorzDistItem&)rSet.Get(SDRATTR_EDGENODE1HORZDIST)).GetValue();
        sal_Int32 nV = ((SdrEdgeNode1VertDistItem&)rSet.Get(SDRATTR_EDGENODE1VERTDIST)).GetValue();

        aBewareRect1.Left()-=nH;
        aBewareRect1.Right()+=nH;
        aBewareRect1.Top()-=nV;
        aBewareRect1.Bottom()+=nV;
    } else {
        aBoundRect1=Rectangle(aPt1,aPt1);
        aBoundRect1.Move(rCon1.aObjOfs.X(),rCon1.aObjOfs.Y());
        aBewareRect1=aBoundRect1;
    }
    if (bCon2) {
        if (rCon2.pObj==(SdrObject*)this) { // sicherheitshalber Abfragen #44515#
            aBoundRect2=aOutRect;
        } else {
            aBoundRect2=rCon2.pObj->GetBoundRect();
        }
        aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
        aBewareRect2=aBoundRect2;

        sal_Int32 nH = ((SdrEdgeNode2HorzDistItem&)rSet.Get(SDRATTR_EDGENODE2HORZDIST)).GetValue();
        sal_Int32 nV = ((SdrEdgeNode2VertDistItem&)rSet.Get(SDRATTR_EDGENODE2VERTDIST)).GetValue();

        aBewareRect2.Left()-=nH;
        aBewareRect2.Right()+=nH;
        aBewareRect2.Top()-=nV;
        aBewareRect2.Bottom()+=nV;
    } else {
        aBoundRect2=Rectangle(aPt2,aPt2);
        aBoundRect2.Move(rCon2.aObjOfs.X(),rCon2.aObjOfs.Y());
        aBewareRect2=aBoundRect2;
    }
    XPolygon aBestXP;
    ULONG nBestQual=0xFFFFFFFF;
    SdrEdgeInfoRec aBestInfo;
    FASTBOOL bAuto1=bCon1 && rCon1.bBestVertex;
    FASTBOOL bAuto2=bCon2 && rCon2.bBestVertex;
    if (bAuto1) rCon1.bAutoVertex=TRUE;
    if (bAuto2) rCon2.bAutoVertex=TRUE;
    USHORT nBestAuto1=0;
    USHORT nBestAuto2=0;
    USHORT nAnz1=bAuto1 ? 4 : 1;
    USHORT nAnz2=bAuto2 ? 4 : 1;
    for (USHORT nNum1=0; nNum1<nAnz1; nNum1++) {
        if (bAuto1) rCon1.nConId=nNum1;
        if (bCon1 && rCon1.TakeGluePoint(aGP1,TRUE)) {
            aPt1=aGP1.GetPos();
            nEsc1=aGP1.GetEscDir();
            if (nEsc1==SDRESC_SMART) nEsc1=ImpCalcEscAngle(rCon1.pObj,aPt1-rCon1.aObjOfs);
        }
        for (USHORT nNum2=0; nNum2<nAnz2; nNum2++) {
            if (bAuto2) rCon2.nConId=nNum2;
            if (bCon2 && rCon2.TakeGluePoint(aGP2,TRUE)) {
                aPt2=aGP2.GetPos();
                nEsc2=aGP2.GetEscDir();
                if (nEsc2==SDRESC_SMART) nEsc2=ImpCalcEscAngle(rCon2.pObj,aPt2-rCon2.aObjOfs);
            }
            for (long nA1=0; nA1<36000; nA1+=9000) {
                USHORT nE1=nA1==0 ? SDRESC_RIGHT : nA1==9000 ? SDRESC_TOP : nA1==18000 ? SDRESC_LEFT : nA1==27000 ? SDRESC_BOTTOM : 0;
                for (long nA2=0; nA2<36000; nA2+=9000) {
                    USHORT nE2=nA2==0 ? SDRESC_RIGHT : nA2==9000 ? SDRESC_TOP : nA2==18000 ? SDRESC_LEFT : nA2==27000 ? SDRESC_BOTTOM : 0;
                    if ((nEsc1&nE1)!=0 && (nEsc2&nE2)!=0) {
                        ULONG nQual=0;
                        SdrEdgeInfoRec aInfo;
                        if (pInfo!=NULL) aInfo=*pInfo;
                        XPolygon aXP(ImpCalcEdgeTrack(aPt1,nA1,aBoundRect1,aBewareRect1,aPt2,nA2,aBoundRect2,aBewareRect2,&nQual,&aInfo));
                        if (nQual<nBestQual) {
                            aBestXP=aXP;
                            nBestQual=nQual;
                            aBestInfo=aInfo;
                            nBestAuto1=nNum1;
                            nBestAuto2=nNum2;
                        }
                    }
                }
            }
        }
    }
    if (bAuto1) rCon1.nConId=nBestAuto1;
    if (bAuto2) rCon2.nConId=nBestAuto2;
    if (pInfo!=NULL) *pInfo=aBestInfo;
    return aBestXP;
}

XPolygon SdrEdgeObj::ImpCalcEdgeTrack(const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
    const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
    ULONG* pnQuality, SdrEdgeInfoRec* pInfo) const
{
    SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetItem(SDRATTR_EDGEKIND))).GetValue();
    FASTBOOL bRts1=nAngle1==0;
    FASTBOOL bObn1=nAngle1==9000;
    FASTBOOL bLks1=nAngle1==18000;
    FASTBOOL bUnt1=nAngle1==27000;
    FASTBOOL bHor1=bLks1 || bRts1;
    FASTBOOL bVer1=bObn1 || bUnt1;
    FASTBOOL bRts2=nAngle2==0;
    FASTBOOL bObn2=nAngle2==9000;
    FASTBOOL bLks2=nAngle2==18000;
    FASTBOOL bUnt2=nAngle2==27000;
    FASTBOOL bHor2=bLks2 || bRts2;
    FASTBOOL bVer2=bObn2 || bUnt2;
    FASTBOOL bInfo=pInfo!=NULL;
    if (bInfo) {
        pInfo->cOrthoForm=0;
        pInfo->nAngle1=nAngle1;
        pInfo->nAngle2=nAngle2;
        pInfo->nObj1Lines=1;
        pInfo->nObj2Lines=1;
        pInfo->nMiddleLine=0xFFFF;
    }
    Point aPt1(rPt1);
    Point aPt2(rPt2);
    Rectangle aBoundRect1 (rBoundRect1 );
    Rectangle aBoundRect2 (rBoundRect2 );
    Rectangle aBewareRect1(rBewareRect1);
    Rectangle aBewareRect2(rBewareRect2);
    Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
    FASTBOOL bMeetingXMid=TRUE;
    FASTBOOL bMeetingYMid=TRUE;
    if (eKind==SDREDGE_ONELINE) {
        XPolygon aXP(2);
        aXP[0]=rPt1;
        aXP[1]=rPt2;
        if (pnQuality!=NULL) {
            *pnQuality=Abs(rPt1.X()-rPt2.X())+Abs(rPt1.Y()-rPt2.Y());
        }
        return aXP;
    } else if (eKind==SDREDGE_THREELINES) {
        XPolygon aXP(4);
        aXP[0]=rPt1;
        aXP[1]=rPt1;
        aXP[2]=rPt2;
        aXP[3]=rPt2;
        if (bRts1) aXP[1].X()=aBewareRect1.Right();  //+=500;
        if (bObn1) aXP[1].Y()=aBewareRect1.Top();    //-=500;
        if (bLks1) aXP[1].X()=aBewareRect1.Left();   //-=500;
        if (bUnt1) aXP[1].Y()=aBewareRect1.Bottom(); //+=500;
        if (bRts2) aXP[2].X()=aBewareRect2.Right();  //+=500;
        if (bObn2) aXP[2].Y()=aBewareRect2.Top();    //-=500;
        if (bLks2) aXP[2].X()=aBewareRect2.Left();   //-=500;
        if (bUnt2) aXP[2].Y()=aBewareRect2.Bottom(); //+=500;
        if (pnQuality!=NULL) {
            long nQ=Abs(aXP[1].X()-aXP[0].X())+Abs(aXP[1].Y()-aXP[0].Y());
                nQ+=Abs(aXP[2].X()-aXP[1].X())+Abs(aXP[2].Y()-aXP[1].Y());
                nQ+=Abs(aXP[3].X()-aXP[2].X())+Abs(aXP[3].Y()-aXP[2].Y());
            *pnQuality=nQ;
        }
        USHORT n1=1;
        USHORT n2=1;
        if (bInfo) {
            pInfo->nObj1Lines=2;
            pInfo->nObj2Lines=2;
            if (bHor1) {
                aXP[1].X()+=pInfo->aObj1Line2.X();
            } else {
                aXP[1].Y()+=pInfo->aObj1Line2.Y();
            }
            if (bHor2) {
                aXP[2].X()+=pInfo->aObj2Line2.X();
            } else {
                aXP[2].Y()+=pInfo->aObj2Line2.Y();
            }
        }
        return aXP;
    }
    USHORT nIntersections=0;
    FASTBOOL bForceMeeting=FALSE; // Muss die Linie durch den MeetingPoint laufen?
    {
        Point aC1(aBewareRect1.Center());
        Point aC2(aBewareRect2.Center());
        if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
            // Ueberschneidung auf der X-Achse
            long n1=Max(aBewareRect1.Left(),aBewareRect2.Left());
            long n2=Min(aBewareRect1.Right(),aBewareRect2.Right());
            aMeeting.X()=(n1+n2+1)/2;
        } else {
            // Ansonsten den Mittelpunkt des Freiraums
            if (aC1.X()<aC2.X()) {
                aMeeting.X()=(aBewareRect1.Right()+aBewareRect2.Left()+1)/2;
            } else {
                aMeeting.X()=(aBewareRect1.Left()+aBewareRect2.Right()+1)/2;
            }
        }
        if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
            // Ueberschneidung auf der Y-Achse
            long n1=Max(aBewareRect1.Top(),aBewareRect2.Top());
            long n2=Min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
            aMeeting.Y()=(n1+n2+1)/2;
        } else {
            // Ansonsten den Mittelpunkt des Freiraums
            if (aC1.Y()<aC2.Y()) {
                aMeeting.Y()=(aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2;
            } else {
                aMeeting.Y()=(aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2;
            }
        }
        // Im Prinzip gibt es 3 zu unterscheidene Faelle:
        //   1. Beide in die selbe Richtung
        //   2. Beide in genau entgegengesetzte Richtungen
        //   3. Einer waagerecht und der andere senkrecht
        long nXMin=Min(aBewareRect1.Left(),aBewareRect2.Left());
        long nXMax=Max(aBewareRect1.Right(),aBewareRect2.Right());
        long nYMin=Min(aBewareRect1.Top(),aBewareRect2.Top());
        long nYMax=Max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
        FASTBOOL bBoundOverlap=aBoundRect1.Right()>aBoundRect2.Left() && aBoundRect1.Left()<aBoundRect2.Right() &&
                               aBoundRect1.Bottom()>aBoundRect2.Top() && aBoundRect1.Top()<aBoundRect2.Bottom();
        FASTBOOL bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
                                aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
        unsigned nMainCase=3;
        if (nAngle1==nAngle2) nMainCase=1;
        else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
        if (nMainCase==1) { // Fall 1: Beide in eine Richtung moeglich.
            if (bVer1) aMeeting.X()=(aPt1.X()+aPt2.X()+1)/2; // ist hier besser, als der
            if (bHor1) aMeeting.Y()=(aPt1.Y()+aPt2.Y()+1)/2; // Mittelpunkt des Freiraums
            // bX1Ok bedeutet, dass die Vertikale, die aus Obj1 austritt, keinen Konflikt mit Obj2 bildet, ...
            FASTBOOL bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
            FASTBOOL bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
            FASTBOOL bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
            FASTBOOL bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
            if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
                aMeeting.X()=nXMin;
                bMeetingXMid=FALSE;
            }
            if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
                aMeeting.X()=nXMax;
                bMeetingXMid=FALSE;
            }
            if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
                aMeeting.Y()=nYMin;
                bMeetingYMid=FALSE;
            }
            if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
                aMeeting.Y()=nYMax;
                bMeetingYMid=FALSE;
            }
        } else if (nMainCase==2) {
            // Fall 2:
            bForceMeeting=TRUE;
            if (bHor1) { // beide waagerecht
                // 9 Moeglichkeiten:                   ù ù ù
                //   2.1 Gegenueber, Ueberschneidung   Ã ´ ù
                //       nur auf der Y-Achse           ù ù ù
                //   2.2, 2.3 Gegenueber, vertikal versetzt. Ã ù ù   ù ù ù
                //            Ueberschneidung weder auf der  ù ´ ù   ù ´ ù
                //            X- noch auf der Y-Achse        ù ù ù   Ã ù ù
                //   2.4, 2.5 Untereinander,   ù Ã ù   ù ù ù
                //            Ueberschneidung  ù ´ ù   ù ´ ù
                //            nur auf X-Achse  ù ù ù   ù Ã ù
                //   2.6, 2.7 Gegeneinander, vertikal versetzt. ù ù Ã   ù ù ù
                //            Ueberschneidung weder auf der     ù ´ ù   ù ´ ù
                //            X- noch auf der Y-Achse.          ù ù ù   ù ù Ã
                //   2.8 Gegeneinander.       ù ù ù
                //       Ueberschneidung nur  ù ´ Ã
                //       auf der Y-Achse.     ù ù ù
                //   2.9 Die BewareRects der Objekte ueberschneiden
                //       sich auf X- und Y-Achse.
                // Die Faelle gelten entsprechend umgesetzt auch fuer
                // senkrechte Linienaustritte.
                // Die Faelle 2.1-2.7 werden mir dem Default-Meeting ausreichend
                // gut behandelt. Spezielle MeetingPoints werden hier also nur
                // fuer 2.8 und 2.9 bestimmt.

                // Normalisierung. aR1 soll der nach rechts und
                // aR2 der nach links austretende sein.
                Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
                Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
                Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
                Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
                    // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                    if (aBewR1.Right()>aBewR2.Left()) {
                        // Faelle 2.8, 2.9
                        // Fall 2.8 ist immer Aussenrumlauf (bDirect=FALSE).
                        // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                        // Ueberschneidung der BewareRects ohne Ueberschneidung der
                        // Boundrects wenn die Linienaustritte sonst das BewareRect
                        // des jeweils anderen Objekts verletzen wuerden.
                        FASTBOOL bCase29Direct=FALSE;
                        FASTBOOL bCase29=aBewR1.Right()>aBewR2.Left();
                        if (aBndR1.Right()<=aBndR2.Left()) { // Fall 2.9 und keine Boundrectueberschneidung
                            if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
                                (aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
                               bCase29Direct=TRUE;
                            }
                        }
                        if (!bCase29Direct) {
                            FASTBOOL bObenLang=Abs(nYMin-aMeeting.Y())<=Abs(nYMax-aMeeting.Y());
                            if (bObenLang) {
                                aMeeting.Y()=nYMin;
                            } else {
                                aMeeting.Y()=nYMax;
                            }
                            bMeetingYMid=FALSE;
                            if (bCase29) {
                                // und nun noch dafuer sorgen, dass das
                                // umzingelte Obj nicht durchquert wird
                                if (aBewR1.Center().Y()<aBewR2.Center().Y() != bObenLang) {
                                    aMeeting.X()=aBewR2.Right();
                                } else {
                                    aMeeting.X()=aBewR1.Left();
                                }
                                bMeetingXMid=FALSE;
                            }
                        } else {
                            // Direkte Verbindung (3-Linien Z-Verbindung), da
                            // Verletzung der BewareRects unvermeidlich ist.
                            // Via Dreisatz werden die BewareRects nun verkleinert.
                            long nWant1=aBewR1.Right()-aBndR1.Right(); // Abstand bei Obj1
                            long nWant2=aBndR2.Left()-aBewR2.Left();   // Abstand bei Obj2
                            long nSpace=aBndR2.Left()-aBndR1.Right(); // verfuegbarer Platz
                            long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            long nGet2=nSpace-nGet1;
                            if (bRts1) { // Normalisierung zurueckwandeln
                                aBewareRect1.Right()+=nGet1-nWant1;
                                aBewareRect2.Left()-=nGet2-nWant2;
                            } else {
                                aBewareRect2.Right()+=nGet1-nWant1;
                                aBewareRect1.Left()-=nGet2-nWant2;
                            }
                            nIntersections++; // Qualitaet herabsetzen
                        }
                    }
                }
            } else if (bVer1) { // beide senkrecht
                Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
                Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
                Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
                Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
                if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
                    // Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
                    if (aBewR1.Bottom()>aBewR2.Top()) {
                        // Faelle 2.8, 2.9
                        // Fall 2.8 ist immer Aussenrumlauf (bDirect=FALSE).
                        // Fall 2.9 kann auch Direktverbindung sein (bei geringer
                        // Ueberschneidung der BewareRects ohne Ueberschneidung der
                        // Boundrects wenn die Linienaustritte sonst das BewareRect
                        // des jeweils anderen Objekts verletzen wuerden.
                        FASTBOOL bCase29Direct=FALSE;
                        FASTBOOL bCase29=aBewR1.Bottom()>aBewR2.Top();
                        if (aBndR1.Bottom()<=aBndR2.Top()) { // Fall 2.9 und keine Boundrectueberschneidung
                            if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
                                (aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
                               bCase29Direct=TRUE;
                            }
                        }
                        if (!bCase29Direct) {
                            FASTBOOL bLinksLang=Abs(nXMin-aMeeting.X())<=Abs(nXMax-aMeeting.X());
                            if (bLinksLang) {
                                aMeeting.X()=nXMin;
                            } else {
                                aMeeting.X()=nXMax;
                            }
                            bMeetingXMid=FALSE;
                            if (bCase29) {
                                // und nun noch dafuer sorgen, dass das
                                // umzingelte Obj nicht durchquert wird
                                if (aBewR1.Center().X()<aBewR2.Center().X() != bLinksLang) {
                                    aMeeting.Y()=aBewR2.Bottom();
                                } else {
                                    aMeeting.Y()=aBewR1.Top();
                                }
                                bMeetingYMid=FALSE;
                            }
                        } else {
                            // Direkte Verbindung (3-Linien Z-Verbindung), da
                            // Verletzung der BewareRects unvermeidlich ist.
                            // Via Dreisatz werden die BewareRects nun verkleinert.
                            long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // Abstand bei Obj1
                            long nWant2=aBndR2.Top()-aBewR2.Top();   // Abstand bei Obj2
                            long nSpace=aBndR2.Top()-aBndR1.Bottom(); // verfuegbarer Platz
                            long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
                            long nGet2=nSpace-nGet1;
                            if (bUnt1) { // Normalisierung zurueckwandeln
                                aBewareRect1.Bottom()+=nGet1-nWant1;
                                aBewareRect2.Top()-=nGet2-nWant2;
                            } else {
                                aBewareRect2.Bottom()+=nGet1-nWant1;
                                aBewareRect1.Top()-=nGet2-nWant2;
                            }
                            nIntersections++; // Qualitaet herabsetzen
                        }
                    }
                }
            }
        } else if (nMainCase==3) { // Fall 3: Einer waagerecht und der andere senkrecht. Sehr viele Fallunterscheidungen
            // Kleine Legende: ù ú ù ú ù -> Ohne Ueberschneidung, maximal Beruehrung.
            //                 ú ú ú ú ú -> Ueberschneidung
            //                 ù ú Ã ú ù -> Selbe Hoehe
            //                 ú ú ú ú ú -> Ueberschneidung
            //                 ù ú ù ú ù -> Ohne Ueberschneidung, maximal Beruehrung.
            // Linienaustritte links ´, rechts Ã, oben Á und unten Â.
            // Insgesamt sind 96 Konstellationen moeglich, wobei einige nicht einmal
            // eindeutig einem Fall und damit einer Behandlungsmethode zugeordnet werden
            // koennen.
            // 3.1: Hierzu moegen alle Konstellationen zaehlen, die durch den
            //      Default-MeetingPoint zufriedenstellend abgedeckt sind (20+12).
            //   Â Â Â ú Á    Á ú Â Â Â   Diese 12  ù ú ù Â ù    ù ú ù ú ù    ù Â ù ú ù    ù ú ù ú ù
            //   ú ú ú ú Á    Á ú ú ú ú   Konstel.  ú ú ú ú ú    ú ú ú ú Â    ú ú ú ú ú    Â ú ú ú ú
            //   ù ú Ã ú ù    ù ú ´ ú ù   jedoch    ù ú Ã ú Á    ù ú Ã ú Â    Á ú ´ ú ù    Â ú ´ ú ù
            //   ú ú ú ú Â    Â ú ú ú ú   nur zum   ú ú ú ú Á    ú ú ú ú ú    Á ú ú ú ú    ú ú ú ú ú
            //   Á Á Á ú Â    Â ú Á Á Á   Teil:     ù ú ù Á ù    ù ú ù ú ù    ù Á ù ú ù    ù ú ù ú ù
            //   Letztere 16 Faelle scheiden aus, sobald sich die Objekte offen
            //   gegenueberstehen (siehe Fall 3.2).
            // 3.2: Die Objekte stehen sich offen gegenueber und somit ist eine
            //      Verbindung mit lediglich 2 Linien moeglich (4+20).
            //      Dieser Fall hat 1. Prioritaet.
            //   ù ú ù ú Â   Â ú ù ú ù   Diese 20  ù ú ù Â ù   ù Â ù ú ù   ù ú ù ú ù   ù ú ù ú ù
            //   ú ú ú ú ú   ú ú ú ú ú   Konstel.  ú ú ú Â Â   Â Â ú ú ú   ú ú ú ú ú   ú ú ú ú ú
            //   ù ú Ã ú ù   ù ú ´ ú ù   jedoch    ù ú Ã Á Á   Á Á ´ ú ù   ù ú Ã Â Â   Â Â ´ ú ù
            //   ú ú ú ú ú   ú ú ú ú ú   nur zum   ú ú ú Á Á   Á Á ú ú ú   ú ú ú ú ú   ú ú ú ú ú
            //   ù ú ù ú Á   Á ú ù ú ù   Teil:     ù ú ù Á ù   ù Á ù ú ù   ù ú ù ú ù   ù ú ù ú ù
            // 3.3: Die Linienaustritte zeigen vom anderen Objekt weg bzw. hinter
            //      dessen Ruecken vorbei (52+4).
            //   Á Á Á Á ù   ù Á Á Á Á   ù ú ú ú ù   ù ú ù ú ù   Diese 4   ù ú ù ú ù   ù ú ù ú ù
            //   Á Á Á Á ú   ú Á Á Á Á   Â Â Â ú ú   ú ú Â Â Â   Konstel.  ú ú ú Â ú   ú Â ú ú ú
            //   Á Á Ã ú ù   ù ú ´ Á Á   Â Â Ã ú ù   ù ú ´ Â Â   jedoch    ù ú Ã ú ù   ù ú ´ ú ù
            //   Á Á Á ú ú   ú ú Á Á Á   Â Â Â Â ú   ú Â Â Â Â   nur zum   ú ú ú Á ú   ú Á ú ú ú
            //   ù ú ù ú ù   ù ú ù ú ù   Â Â Â Â ù   ù Â Â Â Â   Teil:     ù ú ù ú ù   ù ú ù ú ù

            // Fall 3.2
            Rectangle aTmpR1(aBewareRect1);
            Rectangle aTmpR2(aBewareRect2);
            if (bBewareOverlap) {
                // Ueberschneidung der BewareRects: BoundRects fuer Check auf Fall 3.2 verwenden.
                aTmpR1=aBoundRect1;
                aTmpR2=aBoundRect2;
            }
            if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
                 ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
                (((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
                 ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
                // Fall 3.2 trifft zu: Verbindung mit lediglich 2 Linien
                bForceMeeting=TRUE;
                bMeetingXMid=FALSE;
                bMeetingYMid=FALSE;
                if (bHor1) {
                    aMeeting.X()=aPt2.X();
                    aMeeting.Y()=aPt1.Y();
                } else {
                    aMeeting.X()=aPt1.X();
                    aMeeting.Y()=aPt2.Y();
                }
                // Falls Ueberschneidung der BewareRects:
                aBewareRect1=aTmpR1;
                aBewareRect2=aTmpR2;
            } else if ((((bRts1 && aBewareRect1.Right ()>aBewareRect2.Left  ()) ||
                         (bLks1 && aBewareRect1.Left  ()<aBewareRect2.Right ())) &&
                        ((bUnt2 && aBewareRect2.Bottom()>aBewareRect1.Top   ()) ||
                         (bObn2 && aBewareRect2.Top   ()<aBewareRect1.Bottom()))) ||
                       (((bRts2 && aBewareRect2.Right ()>aBewareRect1.Left  ()) ||
                         (bLks2 && aBewareRect2.Left  ()<aBewareRect1.Right ())) &&
                        ((bUnt1 && aBewareRect1.Bottom()>aBewareRect2.Top   ()) ||
                         (bObn1 && aBewareRect1.Top   ()<aBewareRect2.Bottom())))) {
                // Fall 3.3
                bForceMeeting=TRUE;
                if (bRts1 || bRts2) { aMeeting.X()=nXMax; bMeetingXMid=FALSE; }
                if (bLks1 || bLks2) { aMeeting.X()=nXMin; bMeetingXMid=FALSE; }
                if (bUnt1 || bUnt2) { aMeeting.Y()=nYMax; bMeetingYMid=FALSE; }
                if (bObn1 || bObn2) { aMeeting.Y()=nYMin; bMeetingYMid=FALSE; }
            }
        }
    }

    XPolygon aXP1(ImpCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
    XPolygon aXP2(ImpCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
    USHORT nXP1Anz=aXP1.GetPointCount();
    USHORT nXP2Anz=aXP2.GetPointCount();
    if (bInfo) {
        pInfo->nObj1Lines=nXP1Anz; if (nXP1Anz>1) pInfo->nObj1Lines--;
        pInfo->nObj2Lines=nXP2Anz; if (nXP2Anz>1) pInfo->nObj2Lines--;
    }
    Point aEP1(aXP1[nXP1Anz-1]);
    Point aEP2(aXP2[nXP2Anz-1]);
    FASTBOOL bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
    FASTBOOL bHorzE1=aEP1.Y()==aXP1[nXP1Anz-2].Y(); // letzte Linie von XP1 horizontal?
    FASTBOOL bHorzE2=aEP2.Y()==aXP2[nXP2Anz-2].Y(); // letzte Linie von XP2 horizontal?
    if (aEP1==aEP2 && (bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X())) {
        // Sonderbehandlung fuer 'I'-Verbinder
        nXP1Anz--; aXP1.Remove(nXP1Anz,1);
        nXP2Anz--; aXP2.Remove(nXP2Anz,1);
        bMeetingXMid=FALSE;
        bMeetingYMid=FALSE;
    }
    if (bInsMeetingPoint) {
        aXP1.Insert(XPOLY_APPEND,aMeeting,XPOLY_NORMAL);
        if (bInfo) {
            // Durch einfuegen des MeetingPoints kommen 2 weitere Linie hinzu.
            // Evtl. wird eine von diesen die Mittellinie.
            if (pInfo->nObj1Lines==pInfo->nObj2Lines) {
                pInfo->nObj1Lines++;
                pInfo->nObj2Lines++;
            } else {
                if (pInfo->nObj1Lines>pInfo->nObj2Lines) {
                    pInfo->nObj2Lines++;
                    pInfo->nMiddleLine=nXP1Anz-1;
                } else {
                    pInfo->nObj1Lines++;
                    pInfo->nMiddleLine=nXP1Anz;
                }
            }
        }
    } else if (bInfo && aEP1!=aEP2 && nXP1Anz+nXP2Anz>=4) {
        // Durch Verbinden der beiden Enden kommt eine weitere Linie hinzu.
        // Dies wird die Mittellinie.
        pInfo->nMiddleLine=nXP1Anz-1;
    }
    USHORT nNum=aXP2.GetPointCount();
    if (aXP1[nXP1Anz-1]==aXP2[nXP2Anz-1] && nXP1Anz>1 && nXP2Anz>1) nNum--;
    while (nNum>0) {
        nNum--;
        aXP1.Insert(XPOLY_APPEND,aXP2[nNum],XPOLY_NORMAL);
    }
    USHORT nPntAnz=aXP1.GetPointCount();
    char cForm=0;
    if (bInfo || pnQuality!=NULL) {
        cForm='?';
        if (nPntAnz==2) cForm='I';
        else if (nPntAnz==3) cForm='L';
        else if (nPntAnz==4) { // Z oder U
            if (nAngle1==nAngle2) cForm='U';
            else cForm='Z';
        } else if (nPntAnz==4) { // Ú-¿  Ú-¿
            // ...                 -Ù     -Ù
        } else if (nPntAnz==6) { // S oder C oder ...
            if (nAngle1!=nAngle2) {
                // Fuer Typ S hat Linie2 dieselbe Richtung wie Linie4.
                // Bei Typ C sind die beiden genau entgegengesetzt.
                Point aP1(aXP1[1]);
                Point aP2(aXP1[2]);
                Point aP3(aXP1[3]);
                Point aP4(aXP1[4]);
                if (aP1.Y()==aP2.Y()) { // beide Linien Horz
                    if (aP1.X()<aP2.X()==aP3.X()<aP4.X()) cForm='S';
                    else cForm='C';
                } else { // sonst beide Linien Vert
                    if (aP1.Y()<aP2.Y()==aP3.Y()<aP4.Y()) cForm='S';
                    else cForm='C';
                }
            } else cForm='4'; // sonst der 3. Fall mit 5 Linien
        } else cForm='?';  //
        // Weitere Formen:
        if (bInfo) {
            pInfo->cOrthoForm=cForm;
            if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
                pInfo->nObj1Lines=1;
                pInfo->nObj2Lines=1;
                if (cForm=='Z' || cForm=='U') {
                    pInfo->nMiddleLine=1;
                } else {
                    pInfo->nMiddleLine=0xFFFF;
                }
            } else if (cForm=='S' || cForm=='C') {
                pInfo->nObj1Lines=2;
                pInfo->nObj2Lines=2;
                pInfo->nMiddleLine=2;
            }
        }
    }
    if (pnQuality!=NULL) {
        ULONG nQual=0;
        ULONG nQual0=nQual; // Ueberlaeufe vorbeugen
        FASTBOOL bOverflow=FALSE;
        Point aPt0(aXP1[0]);
        for (USHORT nPntNum=1; nPntNum<nPntAnz; nPntNum++) {
            Point aPt1(aXP1[nPntNum]);
            nQual+=Abs(aPt1.X()-aPt0.X())+Abs(aPt1.Y()-aPt0.Y());
            if (nQual<nQual0) bOverflow=TRUE;
            nQual0=nQual;
            aPt0=aPt1;
        }

        USHORT nTmp=nPntAnz;
        if (cForm=='Z') {
            nTmp=2; // Z-Form hat gute Qualitaet (nTmp=2 statt 4)
            ULONG n1=Abs(aXP1[1].X()-aXP1[0].X())+Abs(aXP1[1].Y()-aXP1[0].Y());
            ULONG n2=Abs(aXP1[2].X()-aXP1[1].X())+Abs(aXP1[2].Y()-aXP1[1].Y());
            ULONG n3=Abs(aXP1[3].X()-aXP1[2].X())+Abs(aXP1[3].Y()-aXP1[2].Y());
            // fuer moeglichst gleichlange Linien sorgen
            ULONG nBesser=0;
            n1+=n3;
            n3=n2/4;
            if (n1>=n2) nBesser=6;
            else if (n1>=3*n3) nBesser=4;
            else if (n1>=2*n3) nBesser=2;
            if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // Senkrechte Startlinie kriegt auch noch einen Pluspunkt (fuer H/V-Prio)
            if (nQual>nBesser) nQual-=nBesser; else nQual=0;
        }
        if (nTmp>=3) {
            nQual0=nQual;
            nQual+=(ULONG)nTmp*0x01000000;
            if (nQual<nQual0 || nTmp>15) bOverflow=TRUE;
        }
        if (nPntAnz>=2) { // Austrittswinkel nochmal pruefen
            Point aP1(aXP1[1]); aP1-=aXP1[0];
            Point aP2(aXP1[nPntAnz-2]); aP2-=aXP1[nPntAnz-1];
            long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
            if (aP1.Y()<0) nAng1=9000; if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // Schraeg!?!
            long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
            if (aP2.Y()<0) nAng2=9000; if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // Schraeg!?!
            if (nAng1!=nAngle1) nIntersections++;
            if (nAng2!=nAngle2) nIntersections++;
        }

        // Fuer den Qualitaetscheck wieder die Original-Rects verwenden und
        // gleichzeitig checken, ob eins fuer die Edge-Berechnung verkleinert
        // wurde (z.B. Fall 2.9)
        aBewareRect1=rBewareRect1;
        aBewareRect2=rBewareRect2;

        for (USHORT i=0; i<nPntAnz; i++) {
            Point aPt1(aXP1[i]);
            FASTBOOL b1=aPt1.X()>aBewareRect1.Left() && aPt1.X()<aBewareRect1.Right() &&
                        aPt1.Y()>aBewareRect1.Top() && aPt1.Y()<aBewareRect1.Bottom();
            FASTBOOL b2=aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right() &&
                        aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom();
            USHORT nInt0=nIntersections;
            if (i==0 || i==nPntAnz-1) {
                if (b1 && b2) nIntersections++;
            } else {
                if (b1) nIntersections++;
                if (b2) nIntersections++;
            }
            // und nun noch auf Ueberschneidungen checken
            if (i>0 && nInt0==nIntersections) {
                if (aPt0.Y()==aPt1.Y()) { // Horizontale Linie
                    if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
                        ((aPt0.X()<=aBewareRect1.Left() && aPt1.X()>=aBewareRect1.Right()) ||
                         (aPt1.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
                    if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
                        ((aPt0.X()<=aBewareRect2.Left() && aPt1.X()>=aBewareRect2.Right()) ||
                         (aPt1.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
                } else { // Vertikale Linie
                    if (aPt0.X()>aBewareRect1.Left() && aPt0.X()<aBewareRect1.Right() &&
                        ((aPt0.Y()<=aBewareRect1.Top() && aPt1.Y()>=aBewareRect1.Bottom()) ||
                         (aPt1.Y()<=aBewareRect1.Top() && aPt0.Y()>=aBewareRect1.Bottom()))) nIntersections++;
                    if (aPt0.X()>aBewareRect2.Left() && aPt0.X()<aBewareRect2.Right() &&
                        ((aPt0.Y()<=aBewareRect2.Top() && aPt1.Y()>=aBewareRect2.Bottom()) ||
                         (aPt1.Y()<=aBewareRect2.Top() && aPt0.Y()>=aBewareRect2.Bottom()))) nIntersections++;
                }
            }
            aPt0=aPt1;
        }
        if (nPntAnz<=1) nIntersections++;
        nQual0=nQual;
        nQual+=(ULONG)nIntersections*0x10000000;
        if (nQual<nQual0 || nIntersections>15) bOverflow=TRUE;

        if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
        *pnQuality=nQual;
    }
    if (bInfo) { // nun die Linienversaetze auf aXP1 anwenden
        if (pInfo->nMiddleLine!=0xFFFF) {
            USHORT nIdx=pInfo->ImpGetPolyIdx(MIDDLELINE,aXP1);
            if (pInfo->ImpIsHorzLine(MIDDLELINE,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aMiddleLine.Y();
                aXP1[nIdx+1].Y()+=pInfo->aMiddleLine.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aMiddleLine.X();
                aXP1[nIdx+1].X()+=pInfo->aMiddleLine.X();
            }
        }
        if (pInfo->nObj1Lines>=2) {
            USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE2,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ1LINE2,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj1Line2.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj1Line2.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj1Line2.X();
                aXP1[nIdx+1].X()+=pInfo->aObj1Line2.X();
            }
        }
        if (pInfo->nObj1Lines>=3) {
            USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ1LINE3,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ1LINE3,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj1Line3.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj1Line3.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj1Line3.X();
                aXP1[nIdx+1].X()+=pInfo->aObj1Line3.X();
            }
        }
        if (pInfo->nObj2Lines>=2) {
            USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE2,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ2LINE2,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj2Line2.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj2Line2.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj2Line2.X();
                aXP1[nIdx+1].X()+=pInfo->aObj2Line2.X();
            }
        }
        if (pInfo->nObj2Lines>=3) {
            USHORT nIdx=pInfo->ImpGetPolyIdx(OBJ2LINE3,aXP1);
            if (pInfo->ImpIsHorzLine(OBJ2LINE3,aXP1)) {
                aXP1[nIdx].Y()+=pInfo->aObj2Line3.Y();
                aXP1[nIdx+1].Y()+=pInfo->aObj2Line3.Y();
            } else {
                aXP1[nIdx].X()+=pInfo->aObj2Line3.X();
                aXP1[nIdx+1].X()+=pInfo->aObj2Line3.X();
            }
        }
    }
    // Nun mache ich ggf. aus dem Verbinder eine Bezierkurve
    if (eKind==SDREDGE_BEZIER && nPntAnz>2) {
        Point* pPt1=&aXP1[0];
        Point* pPt2=&aXP1[1];
        Point* pPt3=&aXP1[nPntAnz-2];
        Point* pPt4=&aXP1[nPntAnz-1];
        long dx1=pPt2->X()-pPt1->X();
        long dy1=pPt2->Y()-pPt1->Y();
        long dx2=pPt3->X()-pPt4->X();
        long dy2=pPt3->Y()-pPt4->Y();
        if (cForm=='L') { // nPntAnz==3
            aXP1.SetFlags(1,XPOLY_CONTROL);
            Point aPt3(*pPt2);
            aXP1.Insert(2,aPt3,XPOLY_CONTROL);
            nPntAnz=aXP1.GetPointCount();
            pPt1=&aXP1[0];
            pPt2=&aXP1[1];
            pPt3=&aXP1[nPntAnz-2];
            pPt4=&aXP1[nPntAnz-1];
            pPt2->X()-=dx1/3;
            pPt2->Y()-=dy1/3;
            pPt3->X()-=dx2/3;
            pPt3->Y()-=dy2/3;
        } else if (nPntAnz>=4 && nPntAnz<=6) { // Z oder U oder ...
            // fuer Alle Anderen werden die Endpunkte der Ausgangslinien
            // erstmal zu Kontrollpunkten. Bei nPntAnz>4 ist also noch
            // Nacharbeit erforderlich!
            aXP1.SetFlags(1,XPOLY_CONTROL);
            aXP1.SetFlags(nPntAnz-2,XPOLY_CONTROL);
            // Distanz x1.5
            pPt2->X()+=dx1/2;
            pPt2->Y()+=dy1/2;
            pPt3->X()+=dx2/2;
            pPt3->Y()+=dy2/2;
            if (nPntAnz==5) {
                // Vor und hinter dem Mittelpunkt jeweils
                // noch einen Kontrollpunkt einfuegen
                Point aCenter(aXP1[2]);
                long dx1=aCenter.X()-aXP1[1].X();
                long dy1=aCenter.Y()-aXP1[1].Y();
                long dx2=aCenter.X()-aXP1[3].X();
                long dy2=aCenter.Y()-aXP1[3].Y();
                aXP1.Insert(2,aCenter,XPOLY_CONTROL);
                aXP1.SetFlags(3,XPOLY_SYMMTR);
                aXP1.Insert(4,aCenter,XPOLY_CONTROL);
                aXP1[2].X()-=dx1/2;
                aXP1[2].Y()-=dy1/2;
                aXP1[3].X()-=(dx1+dx2)/4;
                aXP1[3].Y()-=(dy1+dy2)/4;
                aXP1[4].X()-=dx2/2;
                aXP1[4].Y()-=dy2/2;
            }
            if (nPntAnz==6) {
                Point aPt1(aXP1[2]);
                Point aPt2(aXP1[3]);
                aXP1.Insert(2,aPt1,XPOLY_CONTROL);
                aXP1.Insert(5,aPt2,XPOLY_CONTROL);
                long dx=aPt1.X()-aPt2.X();
                long dy=aPt1.Y()-aPt2.Y();
                aXP1[3].X()-=dx/2;
                aXP1[3].Y()-=dy/2;
                aXP1.SetFlags(3,XPOLY_SYMMTR);
                //aXP1[4].X()+=dx/2;
                //aXP1[4].Y()+=dy/2;
                aXP1.Remove(4,1); // weil identisch mit aXP1[3]
            }
        }
    }
    return aXP1;
}

/*
Nach einer einfachen Rechnung koennte es max. 64 unterschiedliche Verlaeufe mit
5 Linien, 32 mit 4 Linien, 16 mit 3, 8 mit 2 Linien und 4 mit 1 Linie geben=124.
Normalisiert auf 1. Austrittswinkel nach rechts bleiben dann noch 31.
Dann noch eine vertikale Spiegelung wegnormalisiert bleiben noch 16
characteristische Verlaufszuege mit 1-5 Linien:
Mit 1 Linie (Typ 'I'):  --
Mit 2 Linien (Typ 'L'): -Ù
Mit 3 Linien (Typ 'U'):  -¿  (Typ 'Z'):  Ú-
                         -Ù             -Ù
Mit 4 Linien: 1 ist nicht plausibel, 3 ist=2 (90deg Drehung). Verbleibt 2,4
     Ú-Ù  Ú¿  À¿  Ú¿                               Ú¿  Ú-¿
    -Ù   -Ù   -Ù  -Ù                              -Ù    -Ù
Mit 5 Linien: nicht plausibel sind 1,2,4,5. 7 ist identisch mit 3 (Richtungsumkehr)
              Bleibt also 3,6 und 8.              '4'  'S'  'C'
       ¿    Ú             -¿   Ú-  Ú-¿                  Ú-
     Ú-Ù  Ú-Ù  Ú-¿   Ú-¿   À¿  À¿ -Ù ³ Ú-¿         Ú-¿  À¿  Ú-¿
    -Ù   -Ù   -Ù Ù  -Ù À-  -Ù  -Ù  --Ù À Ù        -Ù Ù  -Ù  À Ù
Insgesamt sind also 9 Grundtypen zu unterscheiden die den 400 Konstellationen
aus Objektposition und Austrittswinkeln zuzuordnen sind.
4 der 9 Grundtypen haben eine 'Mittellinie'. Die Anzahl der zu Objektabstaende
je Objekt variiert von 0-3:
     Mi   O1   O2   Anmerkung
'I':  n   0    0
'L':  n   0    0
'U':  n  0-1  0-1
'Z':  j   0    0
4.1:  j   0    1    = U+1 bzw. 1+U
4.2:  n  0-2  0-2   = Z+1
'4':  j   0    2    = Z+2
'S':  j   1    1    = 1+Z+1
'C':  n  0-3  0-3   = 1+U+1
*/

void __EXPORT SdrEdgeObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId&, const SfxHint& rHint, const TypeId&)
{
    SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
    ULONG nId=pSimple==0 ? 0 : pSimple->GetId();
    FASTBOOL bDataChg=nId==SFX_HINT_DATACHANGED;
    FASTBOOL bDying=nId==SFX_HINT_DYING;
    FASTBOOL bObj1=aCon1.pObj!=NULL && aCon1.pObj->GetBroadcaster()==&rBC;
    FASTBOOL bObj2=aCon2.pObj!=NULL && aCon2.pObj->GetBroadcaster()==&rBC;
    if (bDying && (bObj1 || bObj2)) {
        // #35605# Dying vorher abfangen, damit AttrObj nicht
        // wg. vermeintlicher Vorlagenaenderung rumbroadcastet
        if (bObj1) aCon1.pObj=NULL;
        if (bObj2) aCon2.pObj=NULL;
        return; // Und mehr braucht hier nicht getan werden.
    }
    SdrTextObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
    if (nNotifyingCount==0) { // Hier nun auch ein VerriegelungsFlag
        ((SdrEdgeObj*)this)->nNotifyingCount++;
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (bDataChg) { // StyleSheet geaendert
            ImpSetAttrToEdgeInfo(); // Werte bei Vorlagenaenderung vom Pool nach aEdgeInfo kopieren
        }
        if (bDataChg                                ||
            (bObj1 && aCon1.pObj->GetPage()==pPage) ||
            (bObj2 && aCon2.pObj->GetPage()==pPage) ||
            (pSdrHint && pSdrHint->GetKind()==HINT_OBJREMOVED))
        {
            // Broadcasting nur, wenn auf der selben Page
            Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
            if (!bEdgeTrackDirty) SendRepaintBroadcast();
            bEdgeTrackDirty=TRUE;
            SendRepaintBroadcast();
            SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
        }
        ((SdrEdgeObj*)this)->nNotifyingCount--;
    }
}

void SdrEdgeObj::operator=(const SdrObject& rObj)
{
    SdrTextObj::operator=(rObj);
    *pEdgeTrack    =*((SdrEdgeObj&)rObj).pEdgeTrack;
    bEdgeTrackDirty=((SdrEdgeObj&)rObj).bEdgeTrackDirty;
    aCon1          =((SdrEdgeObj&)rObj).aCon1;
    aCon2          =((SdrEdgeObj&)rObj).aCon2;
    aCon1.pObj=NULL;
    aCon2.pObj=NULL;
    aEdgeInfo=((SdrEdgeObj&)rObj).aEdgeInfo;
}

void SdrEdgeObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulEDGE);
}

void SdrEdgeObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralEDGE);
}

void SdrEdgeObj::TakeXorPoly(XPolyPolygon& rXPolyPoly, FASTBOOL bDetail) const
{
    if (bEdgeTrackDirty) ((SdrEdgeObj*)this)->ImpRecalcEdgeTrack();
    rXPolyPoly=XPolyPolygon(*pEdgeTrack);
}

void SdrEdgeObj::TakeContour(XPolyPolygon& rPoly) const
{
    // am 14.1.97 wg. Umstellung TakeContour ueber Mtf und Paint. Joe.
    SdrTextObj::TakeContour(rPoly);
}

void SdrEdgeObj::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

USHORT SdrEdgeObj::GetHdlCount() const
{
    SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetItem(SDRATTR_EDGEKIND))).GetValue();
    USHORT nHdlAnz=0;
    USHORT nPntAnz=pEdgeTrack->GetPointCount();
    if (nPntAnz!=0) {
        nHdlAnz=2;
        if ((eKind==SDREDGE_ORTHOLINES || eKind==SDREDGE_BEZIER) && nPntAnz>=4) {
            USHORT nO1=aEdgeInfo.nObj1Lines>0 ? aEdgeInfo.nObj1Lines-1 : 0;
            USHORT nO2=aEdgeInfo.nObj2Lines>0 ? aEdgeInfo.nObj2Lines-1 : 0;
            USHORT nM=aEdgeInfo.nMiddleLine!=0xFFFF ? 1 : 0;
            nHdlAnz+=nO1+nO2+nM;
        } else if (eKind==SDREDGE_THREELINES && nPntAnz==4) {
            if (GetConnectedNode(TRUE)!=NULL) nHdlAnz++;
            if (GetConnectedNode(FALSE)!=NULL) nHdlAnz++;
        }
    }
    return nHdlAnz;
}

SdrHdl* SdrEdgeObj::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pHdl=NULL;
    USHORT nPntAnz=pEdgeTrack->GetPointCount();
    if (nPntAnz!=0) {
        if (nHdlNum==0) {
            pHdl=new ImpEdgeHdl((*pEdgeTrack)[0],HDL_POLY);
            if (aCon1.pObj!=NULL && aCon1.bBestVertex) pHdl->Set1PixMore(TRUE);
        } else if (nHdlNum==1) {
            pHdl=new ImpEdgeHdl((*pEdgeTrack)[USHORT(nPntAnz-1)],HDL_POLY);
            if (aCon2.pObj!=NULL && aCon2.bBestVertex) pHdl->Set1PixMore(TRUE);
        } else {
            SdrEdgeKind eKind=((SdrEdgeKindItem&)(GetItem(SDRATTR_EDGEKIND))).GetValue();
            if (eKind==SDREDGE_ORTHOLINES || eKind==SDREDGE_BEZIER) {
                USHORT nO1=aEdgeInfo.nObj1Lines>0 ? aEdgeInfo.nObj1Lines-1 : 0;
                USHORT nO2=aEdgeInfo.nObj2Lines>0 ? aEdgeInfo.nObj2Lines-1 : 0;
                USHORT nM=aEdgeInfo.nMiddleLine!=0xFFFF ? 1 : 0;
                USHORT nNum=nHdlNum-2;
                short nPt=0;
                pHdl=new ImpEdgeHdl(Point(),HDL_POLY);
                if (nNum<nO1) {
                    nPt=nNum+1;
                    if (nNum==0) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE2);
                    if (nNum==1) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE3);
                } else {
                    nNum-=nO1;
                    if (nNum<nO2) {
                        nPt=nPntAnz-3-nNum;
                        if (nNum==0) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE2);
                        if (nNum==1) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE3);
                    } else {
                        nNum-=nO2;
                        if (nNum<nM) {
                            nPt=aEdgeInfo.nMiddleLine;
                            ((ImpEdgeHdl*)pHdl)->SetLineCode(MIDDLELINE);
                        }
                    }
                }
                if (nPt>0) {
                    Point aPos((*pEdgeTrack)[nPt]);
                    aPos+=(*pEdgeTrack)[nPt+1];
                    aPos.X()/=2;
                    aPos.Y()/=2;
                    pHdl->SetPos(aPos);
                } else {
                    delete pHdl;
                    pHdl=NULL;
                }
            } else if (eKind==SDREDGE_THREELINES) {
                USHORT nNum=nHdlNum;
                if (GetConnectedNode(TRUE)==NULL) nNum++;
                Point aPos((*pEdgeTrack)[nNum-1]);
                pHdl=new ImpEdgeHdl(aPos,HDL_POLY);
                if (nNum==2) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ1LINE2);
                if (nNum==3) ((ImpEdgeHdl*)pHdl)->SetLineCode(OBJ2LINE2);
            }
        }
        if (pHdl!=NULL) {
            pHdl->SetPointNum(nHdlNum);
        }
    }
    return pHdl;
}

FASTBOOL SdrEdgeObj::HasSpecialDrag() const
{
    return TRUE;
}

class ImpEdgeUser
{
public:
    XPolygon          aXP;
    SdrObjConnection  aCon1;
    SdrObjConnection  aCon2;
    SdrObjConnection* pDragCon;
    SdrEdgeInfoRec    aInfo;
};

FASTBOOL SdrEdgeObj::BegDrag(SdrDragStat& rDragStat) const
{
    if (rDragStat.GetHdl()==NULL) return FALSE;
    rDragStat.SetEndDragChangesAttributes(TRUE);
    rDragStat.pUser=new ImpEdgeUser;
    ImpEdgeUser* pEdgeUser=(ImpEdgeUser*)rDragStat.pUser;
    pEdgeUser->aXP=(*pEdgeTrack);
    pEdgeUser->aInfo=aEdgeInfo;
    pEdgeUser->aCon1=aCon1;
    pEdgeUser->aCon2=aCon2;
    pEdgeUser->pDragCon=NULL;
    if (rDragStat.GetHdl()->GetPointNum()<2) {
        rDragStat.SetEndDragChangesGeoAndAttributes(TRUE);
        if (rDragStat.GetHdl()->GetPointNum()==0) pEdgeUser->pDragCon=&pEdgeUser->aCon1;
        if (rDragStat.GetHdl()->GetPointNum()==1) pEdgeUser->pDragCon=&pEdgeUser->aCon2;
        rDragStat.SetNoSnap(TRUE);
    }
    return TRUE;
}

FASTBOOL SdrEdgeObj::MovDrag(SdrDragStat& rDragStat) const
{
    Point aPt(rDragStat.GetNow());
    ImpEdgeUser* pEdgeUser=(ImpEdgeUser*)rDragStat.pUser;
    const SdrHdl* pHdl=rDragStat.GetHdl();
    const ImpEdgeHdl* pEdgeHdl=(ImpEdgeHdl*)pHdl;
    pEdgeUser->aXP=(*pEdgeTrack);
    pEdgeUser->aInfo=aEdgeInfo;
    if (pHdl->GetPointNum()<2) {
        pEdgeUser->pDragCon->pObj=NULL;
        if (rDragStat.GetPageView()!=NULL) {
            ImpFindConnector(aPt,*rDragStat.GetPageView(),*pEdgeUser->pDragCon,this);
            if (rDragStat.GetView()!=NULL) {
                rDragStat.GetView()->SetConnectMarker(*pEdgeUser->pDragCon,*rDragStat.GetPageView());
            }
        }
        if (pHdl->GetPointNum()==0) pEdgeUser->aXP[0]=aPt;
        else pEdgeUser->aXP[USHORT(pEdgeUser->aXP.GetPointCount()-1)]=aPt;
        pEdgeUser->aInfo.aObj1Line2=Point();
        pEdgeUser->aInfo.aObj1Line3=Point();
        pEdgeUser->aInfo.aObj2Line2=Point();
        pEdgeUser->aInfo.aObj2Line3=Point();
        pEdgeUser->aInfo.aMiddleLine=Point();
    } else { // Sonst Dragging eines Linienversatzes
        SdrEdgeLineCode eLineCode=pEdgeHdl->GetLineCode();
        Point aDist(rDragStat.GetNow()); aDist-=rDragStat.GetStart();
        long nDist=pEdgeHdl->IsHorzDrag() ? aDist.X() : aDist.Y();
        nDist+=pEdgeUser->aInfo.ImpGetLineVersatz(eLineCode,pEdgeUser->aXP);
        pEdgeUser->aInfo.ImpSetLineVersatz(eLineCode,pEdgeUser->aXP,nDist);
    }
    pEdgeUser->aXP=ImpCalcEdgeTrack(pEdgeUser->aXP,pEdgeUser->aCon1,pEdgeUser->aCon2,&pEdgeUser->aInfo);
    return TRUE;
}

FASTBOOL SdrEdgeObj::EndDrag(SdrDragStat& rDragStat)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    ImpEdgeUser* pEdgeUser=(ImpEdgeUser*)rDragStat.pUser;
    if (rDragStat.GetHdl()->GetPointNum()<2) {
        (*pEdgeTrack)=pEdgeUser->aXP;
        aEdgeInfo=pEdgeUser->aInfo;
        if (rDragStat.GetHdl()->GetPointNum()==0) {
            ConnectToNode(TRUE,pEdgeUser->aCon1.pObj);
            aCon1=pEdgeUser->aCon1;
        } else {
            ConnectToNode(FALSE,pEdgeUser->aCon2.pObj);
            aCon2=pEdgeUser->aCon2;
        }
    } else { // Sonst Dragging eines Linienversatzes
        (*pEdgeTrack)=pEdgeUser->aXP;
        aEdgeInfo=pEdgeUser->aInfo;
    }
    ImpSetEdgeInfoToAttr();
    delete (ImpEdgeUser*)rDragStat.pUser;
    rDragStat.pUser=NULL;
    SetChanged();
    SetRectsDirty();
    SendRepaintBroadcast();
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return TRUE;
}

void SdrEdgeObj::BrkDrag(SdrDragStat& rDragStat) const
{
    delete (ImpEdgeUser*)rDragStat.pUser;
    rDragStat.pUser=NULL;
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
}

XubString SdrEdgeObj::GetDragComment(const SdrDragStat& rDragStat, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
{
    XubString aStr;
    if (!bCreateComment) ImpTakeDescriptionStr(STR_DragEdgeTail,aStr);
    return aStr;
}

void SdrEdgeObj::TakeDragPoly(const SdrDragStat& rDragStat, XPolyPolygon& rXPP) const
{
    ImpEdgeUser* pEdgeUser=(ImpEdgeUser*)rDragStat.pUser;
    rXPP.Clear();
    rXPP.Insert(pEdgeUser->aXP);
}

void SdrEdgeObj::NspToggleEdgeXor(const SdrDragStat& rDragStat, ExtOutputDevice& rXOut, FASTBOOL bTail1, FASTBOOL bTail2, FASTBOOL bDetail) const
{
    SdrDragMethod* pDM=rDragStat.GetDragMethod();
    if (pDM!=NULL) {
        if (IS_TYPE(SdrDragObjOwn,pDM) || IS_TYPE(SdrDragMovHdl,pDM)) return;
        if (!pDM->IsMoveOnly()) bDetail=FALSE;
        if (bDetail) {
            SdrObjConnection aMyCon1(aCon1);
            SdrObjConnection aMyCon2(aCon2);
            if (bTail1) pDM->MovPoint(aMyCon1.aObjOfs,Point(0,0));
            if (bTail2) pDM->MovPoint(aMyCon2.aObjOfs,Point(0,0));
            SdrEdgeInfoRec aInfo(aEdgeInfo);
            XPolygon aXP(ImpCalcEdgeTrack(*pEdgeTrack,aMyCon1,aMyCon2,&aInfo));

            OutputDevice* pOut = rXOut.GetOutDev();

            if (pOut->GetOutDevType() == OUTDEV_WINDOW)
            {
                const Polygon aPolygon( XOutCreatePolygon(aXP, pOut) );
                ((Window*) pOut)->InvertTracking(aPolygon, SHOWTRACK_WINDOW);
            }
            else
            {
                rXOut.DrawXPolyLine(aXP);
            }
        } else {
            const XPolygon& rXP=*pEdgeTrack;
            Point aPt1(rXP[0]);
            Point aPt2(rXP[USHORT(rXP.GetPointCount()-1)]);
            if (aCon1.pObj!=NULL && (aCon1.bBestConn || aCon1.bBestVertex)) aPt1=aCon1.pObj->GetSnapRect().Center();
            if (aCon2.pObj!=NULL && (aCon2.bBestConn || aCon2.bBestVertex)) aPt2=aCon2.pObj->GetSnapRect().Center();
            if (bTail1) pDM->MovPoint(aPt1,Point(0,0));
            if (bTail2) pDM->MovPoint(aPt2,Point(0,0));
            rXOut.GetOutDev()->DrawLine(aPt1,aPt2);
        }
    }
}

FASTBOOL SdrEdgeObj::BegCreate(SdrDragStat& rDragStat)
{
    rDragStat.SetNoSnap(TRUE);
    pEdgeTrack->SetPointCount(2);
    (*pEdgeTrack)[0]=rDragStat.GetStart();
    (*pEdgeTrack)[1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=NULL) {
        ImpFindConnector(rDragStat.GetStart(),*rDragStat.GetPageView(),aCon1,this);
        ConnectToNode(TRUE,aCon1.pObj);
    }
    *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
    return TRUE;
}

FASTBOOL SdrEdgeObj::MovCreate(SdrDragStat& rDragStat)
{
    USHORT nMax=pEdgeTrack->GetPointCount();
    (*pEdgeTrack)[nMax-1]=rDragStat.GetNow();
    if (rDragStat.GetPageView()!=NULL) {
        ImpFindConnector(rDragStat.GetNow(),*rDragStat.GetPageView(),aCon2,this);
        rDragStat.GetView()->SetConnectMarker(aCon2,*rDragStat.GetPageView());
    }
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    ConnectToNode(FALSE,aCon2.pObj);
    *pEdgeTrack=ImpCalcEdgeTrack(*pEdgeTrack,aCon1,aCon2,&aEdgeInfo);
    bEdgeTrackDirty=FALSE;
    return TRUE;
}

FASTBOOL SdrEdgeObj::EndCreate(SdrDragStat& rDragStat, SdrCreateCmd eCmd)
{
    FASTBOOL bOk=(eCmd==SDRCREATE_FORCEEND || rDragStat.GetPointAnz()>=2);
    if (bOk) {
        ConnectToNode(TRUE,aCon1.pObj);
        ConnectToNode(FALSE,aCon2.pObj);
        if (rDragStat.GetView()!=NULL) {
            rDragStat.GetView()->HideConnectMarker();
        }
        ImpSetEdgeInfoToAttr(); // Die Werte aus aEdgeInfo in den Pool kopieren
    }
    SetRectsDirty();
    return bOk;
}

FASTBOOL SdrEdgeObj::BckCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
    return FALSE;
}

void SdrEdgeObj::BrkCreate(SdrDragStat& rDragStat)
{
    if (rDragStat.GetView()!=NULL) {
        rDragStat.GetView()->HideConnectMarker();
    }
}

void SdrEdgeObj::TakeCreatePoly(const SdrDragStat& rStatDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    rXPP.Insert(*pEdgeTrack);
}

Pointer SdrEdgeObj::GetCreatePointer() const
{
    return Pointer(POINTER_DRAW_CONNECT);
}

FASTBOOL SdrEdgeObj::ImpFindConnector(const Point& rPt, const SdrPageView& rPV, SdrObjConnection& rCon, const SdrEdgeObj* pThis, OutputDevice* pOut)
{
    rCon.ResetVars();
    if (pOut==NULL) pOut=rPV.GetView().GetWin(0);
    if (pOut==NULL) return FALSE;
    SdrObjList* pOL=rPV.GetObjList();
    const SetOfByte& rVisLayer=rPV.GetVisibleLayers();
    // Sensitiver Bereich der Konnektoren ist doppelt so gross wie die Handles:
    USHORT nMarkHdSiz=rPV.GetView().GetMarkHdlSizePixel();
    Size aHalfConSiz(nMarkHdSiz,nMarkHdSiz);
    aHalfConSiz=pOut->PixelToLogic(aHalfConSiz);
    Size aHalfCenterSiz(2*aHalfConSiz.Width(),2*aHalfConSiz.Height());
    Rectangle aMouseRect(rPt,rPt);
    aMouseRect.Left()  -=aHalfConSiz.Width();
    aMouseRect.Top()   -=aHalfConSiz.Height();
    aMouseRect.Right() +=aHalfConSiz.Width();
    aMouseRect.Bottom()+=aHalfConSiz.Height();
    USHORT nBoundHitTol=(USHORT)aHalfConSiz.Width()/2; if (nBoundHitTol==0) nBoundHitTol=1;
    ULONG no=pOL->GetObjCount();
    FASTBOOL bFnd=FALSE;
    SdrObjConnection aTestCon;
    SdrObjConnection aBestCon;
    FASTBOOL bTestBoundHit=FALSE;
    FASTBOOL bBestBoundHit=FALSE;

    while (no>0 && !bFnd) {
        // Problem: Gruppenobjekt mit verschiedenen Layern liefert LayerID 0 !!!!
        no--;
        SdrObject* pObj=pOL->GetObj(no);
        if (rVisLayer.IsSet(pObj->GetLayer()) &&       // nur sichtbare Objekte
            (pThis==NULL || pObj!=(SdrObject*)pThis) && // nicht an mich selbst connecten
            pObj->IsNode())
        {
            Rectangle aObjBound(pObj->GetBoundRect());
            if (aObjBound.IsOver(aMouseRect)) {
                aTestCon.ResetVars();
                bTestBoundHit=FALSE;
                FASTBOOL bEdge=HAS_BASE(SdrEdgeObj,pObj); // kein BestCon fuer Edge
                // Die Userdefined Konnektoren haben absolute Prioritaet.
                // Danach kommt Vertex, Corner und Mitte(Best) gleich priorisiert.
                // Zum Schluss kommt noch ein HitTest aufs Obj.
                const SdrGluePointList* pGPL=pObj->GetGluePointList();
                USHORT nConAnz=pGPL==NULL ? 0 : pGPL->GetCount();
                USHORT nGesAnz=nConAnz+9;
                FASTBOOL bUserFnd=FALSE;
                ULONG nBestDist=0xFFFFFFFF;
                for (USHORT i=0; i<nGesAnz; i++) {
                    FASTBOOL bUser=i<nConAnz;
                    FASTBOOL bVertex=i>=nConAnz+0 && i<nConAnz+4;
                    FASTBOOL bCorner=i>=nConAnz+4 && i<nConAnz+8;
                    FASTBOOL bCenter=i==nConAnz+8;
                    FASTBOOL bOk=FALSE;
                    Point aConPos;
                    USHORT nConNum=i;
                    if (bUser) {
                        const SdrGluePoint& rGP=(*pGPL)[nConNum];
                        aConPos=rGP.GetAbsolutePos(*pObj);
                        nConNum=rGP.GetId();
                        bOk=TRUE;
                    } else if (bVertex && !bUserFnd) {
                        nConNum-=nConAnz;
                        if (rPV.GetView().IsAutoVertexConnectors()) {
                            SdrGluePoint aPt(pObj->GetVertexGluePoint(nConNum));
                            aConPos=aPt.GetAbsolutePos(*pObj);
                            bOk=TRUE;
                        } else i+=3;
                    } else if (bCorner && !bUserFnd) {
                        nConNum-=nConAnz+4;
                        if (rPV.GetView().IsAutoCornerConnectors()) {
                            SdrGluePoint aPt(pObj->GetCornerGluePoint(nConNum));
                            aConPos=aPt.GetAbsolutePos(*pObj);
                            bOk=TRUE;
                        } else i+=3;
                    } else if (bCenter && !bUserFnd && !bEdge) { // Edges nicht!
                        nConNum=0;
                        aConPos=aObjBound.Center();
                        bOk=TRUE;
                    }
                    if (bOk && aMouseRect.IsInside(aConPos)) {
                        if (bUser) bUserFnd=TRUE;
                        bFnd=TRUE;
                        ULONG nDist=(ULONG)Abs(aConPos.X()-rPt.X())+(ULONG)Abs(aConPos.Y()-rPt.Y());
                        if (nDist<nBestDist) {
                            nBestDist=nDist;
                            aTestCon.pObj=pObj;
                            aTestCon.nConId=nConNum;
                            aTestCon.bAutoCorner=bCorner;
                            aTestCon.bAutoVertex=bVertex;
                            aTestCon.bBestConn=FALSE; // bCenter;
                            aTestCon.bBestVertex=bCenter;
                        }
                    }
                }
                // Falls kein Konnektor getroffen wird nochmal
                // HitTest versucht fuer BestConnector (=bCenter)
                if (!bFnd && !bEdge && pObj->IsHit(rPt,nBoundHitTol,&rVisLayer)) {
                    bFnd=TRUE;
                    aTestCon.pObj=pObj;
                    aTestCon.bBestConn=TRUE;
                }
                if (bFnd) {
                    Rectangle aMouseRect2(rPt,rPt);
                    aMouseRect.Left()  -=nBoundHitTol;
                    aMouseRect.Top()   -=nBoundHitTol;
                    aMouseRect.Right() +=nBoundHitTol;
                    aMouseRect.Bottom()+=nBoundHitTol;
                    bTestBoundHit=aObjBound.IsOver(aMouseRect2);
                }

            }
        }
    }
    rCon=aTestCon;
    return bFnd;
}

void SdrEdgeObj::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aOld(GetSnapRect());
    long nMulX = rRect.Right()  - rRect.Left();
    long nDivX = aOld.Right()   - aOld.Left();
    long nMulY = rRect.Bottom() - rRect.Top();
    long nDivY = aOld.Bottom()  - aOld.Top();
    if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
    if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
    Fraction aX(nMulX, nDivX);
    Fraction aY(nMulY, nDivY);
    NbcResize(aOld.TopLeft(), aX, aY);
    NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
}

void SdrEdgeObj::NbcMove(const Size& rSiz)
{
    SdrTextObj::NbcMove(rSiz);
    MoveXPoly(*pEdgeTrack,rSiz);
}

void SdrEdgeObj::NbcResize(const Point& rRefPnt, const Fraction& aXFact, const Fraction& aYFact)
{
    SdrTextObj::NbcResize(rRefPnt,aXFact,aXFact);
    ResizeXPoly(*pEdgeTrack,rRefPnt,aXFact,aYFact);

    // #75735#
    aEdgeInfo.aObj1Line2 = Point();
    aEdgeInfo.aObj1Line3 = Point();
    aEdgeInfo.aObj2Line2 = Point();
    aEdgeInfo.aObj2Line3 = Point();
    aEdgeInfo.aMiddleLine = Point();
}

SdrObject* SdrEdgeObj::DoConvertToPolyObj(BOOL bBezier) const
{
    SdrObject* pRet=ImpConvertMakeObj(XPolyPolygon(*pEdgeTrack),FALSE,bBezier);
    pRet=ImpConvertAddText(pRet,bBezier);
    return pRet;
}

USHORT SdrEdgeObj::GetSnapPointCount() const
{
    return 2;
}

Point SdrEdgeObj::GetSnapPoint(USHORT i) const
{
    ((SdrEdgeObj*)this)->ImpUndirtyEdgeTrack();
    USHORT nAnz=pEdgeTrack->GetPointCount();
    if (i==0) return (*pEdgeTrack)[0];
    else return (*pEdgeTrack)[nAnz-1];
}

FASTBOOL SdrEdgeObj::IsPolyObj() const
{
    return FALSE;
}

USHORT SdrEdgeObj::GetPointCount() const
{
    return 0;
}

const Point& SdrEdgeObj::GetPoint(USHORT i) const
{
    ((SdrEdgeObj*)this)->ImpUndirtyEdgeTrack();
    USHORT nAnz=pEdgeTrack->GetPointCount();
    if (i==0) return (*pEdgeTrack)[0];
    else return (*pEdgeTrack)[nAnz-1];
}

void SdrEdgeObj::NbcSetPoint(const Point& rPnt, USHORT i)
{
    // ToDo: Umconnekten fehlt noch
    ImpUndirtyEdgeTrack();
    USHORT nAnz=pEdgeTrack->GetPointCount();
    if (i==0) (*pEdgeTrack)[0]=rPnt;
    if (i==1) (*pEdgeTrack)[nAnz-1]=rPnt;
    SetEdgeTrackDirty();
    SetRectsDirty();
}

SdrEdgeObjGeoData::SdrEdgeObjGeoData()
{
    pEdgeTrack=new XPolygon;
}

SdrEdgeObjGeoData::~SdrEdgeObjGeoData()
{
    delete pEdgeTrack;
}

SdrObjGeoData* SdrEdgeObj::NewGeoData() const
{
    return new SdrEdgeObjGeoData;
}

void SdrEdgeObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrEdgeObjGeoData& rEGeo=(SdrEdgeObjGeoData&)rGeo;
    rEGeo.aCon1          =aCon1;
    rEGeo.aCon2          =aCon2;
    *rEGeo.pEdgeTrack    =*pEdgeTrack;
    rEGeo.bEdgeTrackDirty=bEdgeTrackDirty;
    rEGeo.aEdgeInfo      =aEdgeInfo;
}

void SdrEdgeObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrEdgeObjGeoData& rEGeo=(SdrEdgeObjGeoData&)rGeo;
    if (aCon1.pObj!=rEGeo.aCon1.pObj) {
        if (aCon1.pObj!=NULL) aCon1.pObj->RemoveListener(*this);
        aCon1=rEGeo.aCon1;
        if (aCon1.pObj!=NULL) aCon1.pObj->AddListener(*this);
    }
    if (aCon2.pObj!=rEGeo.aCon2.pObj) {
        if (aCon2.pObj!=NULL) aCon2.pObj->RemoveListener(*this);
        aCon2=rEGeo.aCon2;
        if (aCon2.pObj!=NULL) aCon2.pObj->AddListener(*this);
    }
    *pEdgeTrack    =*rEGeo.pEdgeTrack;
    bEdgeTrackDirty=rEGeo.bEdgeTrackDirty;
    aEdgeInfo      =rEGeo.aEdgeInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrEdgeObj::PreSave()
{
    // call parent
    SdrTextObj::PreSave();

    // prepare SetItems for storage
    const SfxItemSet& rSet = GetUnmergedItemSet();
    const SfxItemSet* pParent = GetStyleSheet() ? &GetStyleSheet()->GetItemSet() : 0L;
    SdrEdgeSetItem aEdgeAttr(rSet.GetPool());
    aEdgeAttr.GetItemSet().Put(rSet);
    aEdgeAttr.GetItemSet().SetParent(pParent);
    mpObjectItemSet->Put(aEdgeAttr);
}

void SdrEdgeObj::PostSave()
{
    // call parent
    SdrTextObj::PostSave();

    // remove SetItems from local itemset
    mpObjectItemSet->ClearItem(SDRATTRSET_EDGE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEdgeObj::WriteData(SvStream& rOut) const
{
    SdrTextObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrEdgeObj");
#endif

    {
        SdrDownCompat aTrackCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
#ifdef DBG_UTIL
        aTrackCompat.SetID("SdrEdgeObj(EdgeTrack)");
#endif
        rOut << *pEdgeTrack;
    }

    aCon1.Write(rOut, this); // Die Connections haben
    aCon2.Write(rOut, this); // ihren eigenen Header

    SfxItemPool* pPool = GetItemPool();

    if(pPool)
    {
        const SfxItemSet& rSet = GetUnmergedItemSet();

        pPool->StoreSurrogate(rOut, &rSet.Get(SDRATTRSET_EDGE));
    }
    else
    {
        rOut << UINT16(SFX_ITEMS_NULL);
    }

    rOut << aEdgeInfo;
}

void SdrEdgeObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrTextObj::ReadData(rHead,rIn);
    if (rHead.GetVersion()<2) { // frueher war EdgeObj von PathObj abgeleitet
        DBG_ERROR("SdrEdgeObj::ReadData(): Dateiversion<2 wird nicht mehr unterstuetzt");
        rIn.SetError(SVSTREAM_WRONGVERSION); // Format-Fehler, File zu alt
        return;
    }
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrEdgeObj");
#endif
    if (rHead.GetVersion()>=11) { // ab V11 ist alles eingepackt
        {
            SdrDownCompat aTrackCompat(rIn,STREAM_READ); // ab V11 eingepackt
#ifdef DBG_UTIL
            aTrackCompat.SetID("SdrEdgeObj(EdgeTrack)");
#endif
            rIn>>*pEdgeTrack;
        }
        aCon1.Read(rIn,this); // Die Connections haben
        aCon2.Read(rIn,this); // ihren eigenen Header.
    } else {
        rIn>>*pEdgeTrack;
        if (rHead.GetBytesLeft()>0) { // Aha, da ist noch mehr (Verbindungsdaten)
            aCon1.ReadTilV10(rIn,this); // Import der
            aCon2.ReadTilV10(rIn,this); // Connections
        }
    }

    if(aCompat.GetBytesLeft() > 0)
    {
        // ab 10-08-1996 (noch Vers 12) Items fuer Verbinder
        SfxItemPool* pPool = GetItemPool();
        if(pPool)
        {
            sal_uInt16 nSetID = SDRATTRSET_EDGE;
            const SdrEdgeSetItem* pEdgeAttr = (const SdrEdgeSetItem*)pPool->LoadSurrogate(rIn, nSetID, 0);
            if(pEdgeAttr)
                SetItemSet(pEdgeAttr->GetItemSet());
        }
        else
        {
            sal_uInt16 nSuroDum;
            rIn >> nSuroDum;
        }
    }

    if(aCompat.GetBytesLeft() > 0)
    {
        // ab 14-01-1997 (noch Vers 12) EdgeInfoRec
        rIn >> aEdgeInfo;
    }
}

void SdrEdgeObj::AfterRead()
{
    SdrTextObj::AfterRead();
    aCon1.AfterRead(this);
    aCon2.AfterRead(this);
    if (aCon1.pObj!=NULL) aCon1.pObj->AddListener(*this);
    if (aCon2.pObj!=NULL) aCon2.pObj->AddListener(*this);

    // #84026# always recalculate edgetrack after load
    bEdgeTrackDirty=TRUE;
}

Point SdrEdgeObj::GetTailPoint( BOOL bTail ) const
{
    if( pEdgeTrack && pEdgeTrack->GetPointCount()!=0)
    {
        const XPolygon& rTrack0 = *pEdgeTrack;
        if(bTail)
        {
            return rTrack0[0];
        }
        else
        {
            const USHORT nSiz = rTrack0.GetPointCount() - 1;
            return rTrack0[nSiz];
        }
    }
    else
    {
        if(bTail)
            return aOutRect.TopLeft();
        else
            return aOutRect.BottomRight();
    }

}

void SdrEdgeObj::SetTailPoint( BOOL bTail, const Point& rPt )
{
    ImpSetTailPoint( bTail, rPt );
    SetChanged();
}

/** this method is used by the api to set a glue point for a connection
    nId == -1 :     The best default point is automaticly choosen
    0 <= nId <= 3 : One of the default points is choosen
    nId >= 4 :      A user defined glue point is choosen
*/
void SdrEdgeObj::setGluePointIndex( sal_Bool bTail, sal_Int32 nIndex /* = -1 */ )
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();

    SdrObjConnection& rConn1 = GetConnection( bTail );

    rConn1.SetAutoVertex( nIndex >= 0 && nIndex <= 3 );
    rConn1.SetBestConnection( nIndex < 0 );
    rConn1.SetBestVertex( nIndex < 0 );

    if( nIndex > 3 )
    {
        nIndex -= 4;

        // for user defined glue points we have
        // to get the id for this index first
        const SdrGluePointList* pList = rConn1.GetObject() ? rConn1.GetObject()->GetGluePointList() : NULL;
        if( pList == NULL || nIndex >= pList->GetCount() )
            return;

        nIndex = (*pList)[(sal_uInt16)nIndex].GetId();
    }
    else if( nIndex < 0 )
    {
        nIndex = 0;
    }

    rConn1.SetConnectorId( (USHORT)nIndex );

    SetChanged();
    SetRectsDirty();
    ImpRecalcEdgeTrack();
    bEdgeTrackDirty=TRUE;
}

/** this method is used by the api to return a glue point id for a connection.
    See setGluePointId for possible return values */
sal_Int32 SdrEdgeObj::getGluePointIndex( sal_Bool bTail )
{
    SdrObjConnection& rConn1 = GetConnection( bTail );
    sal_Int32 nId = -1;
    if( !rConn1.IsBestConnection() )
    {
        nId = rConn1.GetConnectorId();
        if( !rConn1.IsAutoVertex() )
        {
            // for user defined glue points we have
            // to get the index for this id first
            const SdrGluePointList* pList = rConn1.GetObject() ? rConn1.GetObject()->GetGluePointList() : NULL;
            if( pList == NULL || nId >= pList->GetCount() )
                return -1;

            nId = pList->FindGluePoint((sal_uInt16)nId);
            nId += 4;
        }
    }
    return nId;
}