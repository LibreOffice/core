/*************************************************************************
 *
 *  $RCSfile: svdobj.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-19 09:52:30 $
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

#include <math.h>
#include <vcl/metaact.hxx>   // fuer TakeContour
#include <vcl/cvtsvm.hxx>
#include <tools/bigint.hxx>
#include <vector>
#include "svdobj.hxx"
#include "xpoly.hxx"
#include "svdxout.hxx"
#include "svdetc.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdhdl.hxx"
#include "svddrag.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdovirt.hxx"  // Fuer Add/Del Ref
#include "svdpagv.hxx"   // fuer PaintGluePoints
#include "svdview.hxx"   // fuer Dragging (Ortho abfragen)
#include "svdscrol.hxx"
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname
#include "svdogrp.hxx"   // Factory
#include "svdopath.hxx"  // Factory
#include "svdoedge.hxx"  // Factory
#include "svdorect.hxx"  // Factory
#include "svdocirc.hxx"  // Factory
#include "svdotext.hxx"  // Factory
#include "svdomeas.hxx"  // Factory
#include "svdograf.hxx"  // Factory
#include "svdoole2.hxx"  // Factory
#include "svdocapt.hxx"  // Factory
#include "svdopage.hxx"  // Factory
#include "svdouno.hxx"   // Factory
#include "svdattrx.hxx" // NotPersistItems

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "xlnwtit.hxx"
#include "xlnstwit.hxx"
#include "xlnedwit.hxx"
#include "xlnstit.hxx"
#include "xlnedit.hxx"
#include "xlnstcit.hxx"
#include "xlnedcit.hxx"
#include "xlndsit.hxx"
#include "xlnclit.hxx"
#include "xflclit.hxx"
#include "svditer.hxx"
#include "xlntrit.hxx"
#include "xfltrit.hxx"
#include "xlinjoit.hxx"
#include "unopage.hxx"

#ifndef _SVDPOOL_HXX
#include "svdpool.hxx"
#endif

#ifndef _MyEDITENG_HXX
#include "editeng.hxx"
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

using namespace ::com::sun::star;

/***********************************************************************
* Macros fuer Umrechnung Twips<->100tel mm                             *
***********************************************************************/
#define TWIPS_TO_MM(val) ((val * 127 + 36) / 72)
#define MM_TO_TWIPS(val) ((val * 72 + 63) / 127)

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserCall);

SdrObjUserCall::~SdrObjUserCall()
{
}

void SdrObjUserCall::Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserData);

void SdrObjUserData::operator=(const SdrObjUserData& rData)    // nicht implementiert
{
}

FASTBOOL SdrObjUserData::operator==(const SdrObjUserData& rData) const // nicht implementiert
{
    return FALSE;
}

FASTBOOL SdrObjUserData::operator!=(const SdrObjUserData& rData) const // nicht implementiert
{
    return FALSE;
}

SdrObjUserData::~SdrObjUserData()
{
}

void SdrObjUserData::WriteData(SvStream& rOut)
{
    rOut<<nInventor;
    rOut<<nIdentifier;
    rOut<<nVersion;
}

void SdrObjUserData::ReadData(SvStream& rIn)
{
    //Inventor und Identifier wurden bereits von Aufrufer gelesen,
    //sonst haette er mich ja nicht erzeugen koennen (kein SeekBack!).
    rIn>>nVersion; // Miniatur-Versionsverwaltung.
}

void SdrObjUserData::AfterRead()
{
}

FASTBOOL SdrObjUserData::HasMacro(const SdrObject* pObj) const
{
    return FALSE;
}

SdrObject* SdrObjUserData::CheckMacroHit(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    if (pObj==NULL) return NULL;
    return pObj->CheckHit(rRec.aPos,rRec.nTol,rRec.pVisiLayer);
}

Pointer SdrObjUserData::GetMacroPointer(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    return Pointer(POINTER_REFHAND);
}

void SdrObjUserData::PaintMacro(ExtOutputDevice& rXOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    if (pObj==NULL) return;
    Color aBlackColor( COL_BLACK );
    Color aTranspColor( COL_TRANSPARENT );
    rXOut.OverrideLineColor( aBlackColor );
    rXOut.OverrideFillColor( aTranspColor );
    RasterOp eRop0=rXOut.GetRasterOp();
    rXOut.SetRasterOp(ROP_INVERT);
    XPolyPolygon aXPP;
    pObj->TakeXorPoly(aXPP,TRUE);
    USHORT nAnz=aXPP.Count();
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        rXOut.DrawXPolyLine(aXPP[nNum]);
    }
    rXOut.SetRasterOp(eRop0);
}

FASTBOOL SdrObjUserData::DoMacro(const SdrObjMacroHitRec& rRec, SdrObject* pObj)
{
    return FALSE;
}

XubString SdrObjUserData::GetMacroPopupComment(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    return String();
}

void SdrObjUserDataList::Clear()
{
    USHORT nAnz=GetUserDataCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetUserData(i);
    }
    aList.Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObjGeoData);

SdrObjGeoData::SdrObjGeoData():
    pGPL(NULL),
    bMovProt(FALSE),
    bSizProt(FALSE),
    bNoPrint(FALSE),
    bClosedObj(FALSE),
    nLayerId(0)
{
    DBG_CTOR(SdrObjGeoData,NULL);
}

SdrObjGeoData::~SdrObjGeoData()
{
    DBG_DTOR(SdrObjGeoData,NULL);
    delete pGPL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjPlusData);

SdrObjPlusData::SdrObjPlusData():
    pBroadcast(NULL),
    pUserDataList(NULL),
    pGluePoints(NULL),
    pAnimator(NULL),
    pAutoTimer(NULL)
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    if (pBroadcast   !=NULL) delete pBroadcast;
    if (pUserDataList!=NULL) delete pUserDataList;
    if (pGluePoints  !=NULL) delete pGluePoints;
    if (pAnimator    !=NULL) delete pAnimator;
    if (pAutoTimer   !=NULL) delete pAutoTimer;
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNeuPlusData=new SdrObjPlusData;
    if (pUserDataList!=NULL) {
        USHORT nAnz=pUserDataList->GetUserDataCount();
        if (nAnz!=0) {
            pNeuPlusData->pUserDataList=new SdrObjUserDataList;
            for (USHORT i=0; i<nAnz; i++) {
                SdrObjUserData* pNeuUserData=pUserDataList->GetUserData(i)->Clone(pObj1);
                if (pNeuUserData!=NULL) {
                    pNeuPlusData->pUserDataList->InsertUserData(pNeuUserData);
                } else {
                    DBG_ERROR("SdrObjPlusData::Clone(): UserData.Clone() liefert NULL");
                }
            }
        }
    }
    if (pGluePoints!=NULL) pNeuPlusData->pGluePoints=new SdrGluePointList(*pGluePoints);
    // MtfAnimator wird auch nicht mitkopiert
    pNeuPlusData->aObjName=aObjName;
    if (pAutoTimer!=NULL) {
        pNeuPlusData->pAutoTimer=new AutoTimer;
        // Handler, etc. nicht mitkopieren!
    }
    return pNeuPlusData;
}

///////////////////////////////////////////////////////////////////////////////

static double SMALLEST_DASH_WIDTH(26.95);

LineStyleParameterPack::LineStyleParameterPack(const SfxItemSet& rSet,
    BOOL bForceHair, OutputDevice& rOut)
:   mrOut(rOut),
    rStartPolygon(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue()),
    rEndPolygon(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue()),
    bForceNoArrowsLeft(FALSE),
    bForceNoArrowsRight(FALSE)
{
    nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
    if(bForceHair)
        nLineWidth = 0L;
    eLineStyle = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();

    nStartWidth = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
    if(nStartWidth < 0)
        nStartWidth = -nLineWidth * nStartWidth / 100;

    nEndWidth = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
    if(nEndWidth < 0)
        nEndWidth = -nLineWidth * nEndWidth / 100;

    bStartCentered = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    bEndCentered = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();

    fDegreeStepWidth = 10.0;
    eLineJoint = ((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue();

    aDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetValue();

    // fill local dash info
    nNumDotDashArray = (GetDots() + GetDashes()) * 2;
    pDotDashArray = new double[nNumDotDashArray];
    UINT16 a;
    UINT16 nIns = 0;
    double fDashDotDistance = (double)GetDashDistance();
    double fSingleDashLen = (double)GetDashLen();
    double fSingleDotLen = (double)GetDotLen();
    double fLineWidth = (double)GetLineWidth();

    if(GetDashStyle() == XDASH_RECTRELATIVE || GetDashStyle() == XDASH_ROUNDRELATIVE)
    {
        if(GetLineWidth())
        {
            double fFactor = fLineWidth / 100.0;

            if(GetDashes())
            {
                if(GetDashLen())
                {
                    // is a dash
                    fSingleDashLen *= fFactor;
                }
                else
                {
                    // is a dot
                    fSingleDashLen = fLineWidth;
                }
            }

            if(GetDots())
            {
                if(GetDotLen())
                {
                    // is a dash
                    fSingleDotLen *= fFactor;
                }
                else
                {
                    // is a dot
                    fSingleDotLen = fLineWidth;
                }
            }

            if(GetDashes() || GetDots())
            {
                if(GetDashDistance())
                    fDashDotDistance *= fFactor;
                else
                    fDashDotDistance = fLineWidth;
            }
        }
        else
        {
            if(GetDashes())
            {
                if(GetDashLen())
                {
                    // is a dash
                    fSingleDashLen = (SMALLEST_DASH_WIDTH * fSingleDashLen) / 100.0;
                }
                else
                {
                    // is a dot
                    fSingleDashLen = SMALLEST_DASH_WIDTH;
                }
            }

            if(GetDots())
            {
                if(GetDotLen())
                {
                    // is a dash
                    fSingleDotLen = (SMALLEST_DASH_WIDTH * fSingleDotLen) / 100.0;
                }
                else
                {
                    // is a dot
                    fSingleDotLen = SMALLEST_DASH_WIDTH;
                }
            }

            if(GetDashes() || GetDots())
            {
                if(GetDashDistance())
                {
                    // dash as distance
                    fDashDotDistance = (SMALLEST_DASH_WIDTH * fDashDotDistance) / 100.0;
                }
                else
                {
                    // dot as distance
                    fDashDotDistance = SMALLEST_DASH_WIDTH;
                }
            }
        }
    }
    else
    {
        // smallest dot size compare value
        double fDotCompVal(GetLineWidth() ? fLineWidth : SMALLEST_DASH_WIDTH);

        // absolute values
        if(GetDashes())
        {
            if(GetDashLen())
            {
                // is a dash
                if(fSingleDashLen < SMALLEST_DASH_WIDTH)
                    fSingleDashLen = SMALLEST_DASH_WIDTH;
            }
            else
            {
                // is a dot
                if(fSingleDashLen < fDotCompVal)
                    fSingleDashLen = fDotCompVal;
            }
        }

        if(GetDots())
        {
            if(GetDotLen())
            {
                // is a dash
                if(fSingleDotLen < SMALLEST_DASH_WIDTH)
                    fSingleDotLen = SMALLEST_DASH_WIDTH;
            }
            else
            {
                // is a dot
                if(fSingleDotLen < fDotCompVal)
                    fSingleDotLen = fDotCompVal;
            }
        }

        if(GetDashes() || GetDots())
        {
            if(GetDashDistance())
            {
                // dash as distance
                if(fDashDotDistance < SMALLEST_DASH_WIDTH)
                    fDashDotDistance = SMALLEST_DASH_WIDTH;
            }
            else
            {
                // dot as distance
                if(fDashDotDistance < fDotCompVal)
                    fDashDotDistance = fDotCompVal;
            }
        }
    }

    fFullDashDotLen = 0.0;

    for(a=0;a<GetDots();a++)
    {
        pDotDashArray[nIns++] = fSingleDotLen;
        fFullDashDotLen += fSingleDotLen;
        pDotDashArray[nIns++] = fDashDotDistance;
        fFullDashDotLen += fDashDotDistance;
    }

    for(a=0;a<GetDashes();a++)
    {
        pDotDashArray[nIns++] = fSingleDashLen;
        fFullDashDotLen += fSingleDashLen;
        pDotDashArray[nIns++] = fDashDotDistance;
        fFullDashDotLen += fDashDotDistance;
    }
}

LineStyleParameterPack::~LineStyleParameterPack()
{
    if(pDotDashArray)
        delete pDotDashArray;
}

UINT16 LineStyleParameterPack::GetFirstDashDotIndex(double fPos, double& rfDist) const
{
    double fIndPos = fPos - (fFullDashDotLen * (double)((UINT32)(fPos / fFullDashDotLen)));
    UINT16 nPos = 0;

    while(fIndPos && fIndPos - pDotDashArray[nPos] > -SMALL_DVALUE)
    {
        fIndPos -= pDotDashArray[nPos];
        nPos = (nPos + 1 == nNumDotDashArray) ? 0 : nPos + 1;
    }

    rfDist = pDotDashArray[nPos] - fIndPos;
    nPos = (nPos + 1 == nNumDotDashArray) ? 0 : nPos + 1;

    return nPos;
}

UINT16 LineStyleParameterPack::GetNextDashDotIndex(UINT16 nPos, double& rfDist) const
{
    rfDist = pDotDashArray[nPos];
    nPos = (nPos + 1 == nNumDotDashArray) ? 0 : nPos + 1;
    return nPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double LineGeometryCreator::ImpSimpleFindCutPoint(
    const Vector3D& rEdge1Start, const Vector3D& rEdge1Delta,
    const Vector3D& rEdge2Start, const Vector3D& rEdge2Delta)
{
    double fZwi = (rEdge1Delta.X() * rEdge2Delta.Y()) - (rEdge1Delta.Y() * rEdge2Delta.X());
    double fRetval = 0.0;

    if(fabs(fZwi) > SMALL_DVALUE)
    {
        fRetval = (rEdge2Delta.Y() * (rEdge2Start.X() - rEdge1Start.X())
            + rEdge2Delta.X() * (rEdge1Start.Y() - rEdge2Start.Y())) / fZwi;
    }
    return fRetval;
}

void LineGeometryCreator::ImpCreateLineSegment(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext)
{
    if(mrLineAttr.GetLineWidth())
    {
        double fHalfLineWidth((double)mrLineAttr.GetLineWidth() / 2.0);
        Vector3D aEdge = *pRight - *pLeft;
        Vector3D aPerpend = aEdge.GetPerpendicular2D();
        XLineJoint eJoint = mrLineAttr.GetLineJoint();

        // joints need eventually not be done
        if((eJoint == XLINEJOINT_MIDDLE || eJoint == XLINEJOINT_MITER) && (!pPrev && !pNext))
            eJoint = XLINEJOINT_NONE;

        switch(eJoint)
        {
            case XLINEJOINT_NONE:       // no rounding
            {
                Polygon3D aNewPoly(4);

                aPerpend *= fHalfLineWidth;
                aNewPoly[0] = *pLeft + aPerpend;
                aNewPoly[1] = *pRight + aPerpend;
                aNewPoly[2] = *pRight - aPerpend;
                aNewPoly[3] = *pLeft - aPerpend;

                aNewPoly.SetClosed(TRUE);
                mrPolyPoly3D.Insert(aNewPoly);

                break;
            }
            case XLINEJOINT_MIDDLE:     // calc middle value between joints
            {
                Polygon3D aNewPoly(4);
                Vector3D aPerpendLeft(aPerpend);
                Vector3D aPerpendRight(aPerpend);

                if(pPrev)
                {
                    aPerpendLeft = *pLeft - *pPrev;
                    aPerpendLeft = aPerpendLeft.GetPerpendicular2D();
                }

                if(pNext)
                {
                    aPerpendRight = *pNext - *pRight;
                    aPerpendRight = aPerpendRight.GetPerpendicular2D();
                }

                aPerpendLeft = (aPerpend + aPerpendLeft) * (fHalfLineWidth / 2.0);
                aPerpendRight = (aPerpend + aPerpendRight) * (fHalfLineWidth / 2.0);

                aNewPoly[0] = *pLeft + aPerpendLeft;
                aNewPoly[1] = *pRight + aPerpendRight;
                aNewPoly[2] = *pRight - aPerpendRight;
                aNewPoly[3] = *pLeft - aPerpendLeft;

                aNewPoly.SetClosed(TRUE);
                mrPolyPoly3D.Insert(aNewPoly);

                break;
            }
            case XLINEJOINT_BEVEL:      // join edges with line
            default: // #73428# case XLINEJOINT_ROUND:      // create arc
            {
                Vector3D aPerpendRight(aPerpend);
                BOOL bCreateSimplePart(TRUE);

                if(pNext)
                {
                    aPerpendRight = *pNext - *pRight;
                    aPerpendRight = aPerpendRight.GetPerpendicular2D();
                    double fAngle = atan2(aPerpend.Y(), aPerpend.X());
                    double fRightAngle = atan2(aPerpendRight.Y(), aPerpendRight.X());
                    double fAngleDiff = fAngle - fRightAngle;
                    double fDegreeStepWidth = mrLineAttr.GetDegreeStepWidth() * F_PI180;

                    // go to range [0.0..2*F_PI[
                    while(fAngleDiff < 0.0)
                        fAngleDiff += (F_PI * 2.0);
                    while(fAngleDiff >= (F_PI * 2.0))
                        fAngleDiff -= (F_PI * 2.0);

                    if((fAngleDiff > fDegreeStepWidth) && (fAngleDiff < ((F_PI * 2.0) - fDegreeStepWidth)))
                    {
                        bCreateSimplePart = FALSE;
                        aPerpend *= fHalfLineWidth;
                        aPerpendRight *= fHalfLineWidth;

                        if(eJoint == XLINEJOINT_BEVEL)
                        {
                            UINT16 nPolyPoints(pPrev ? 7 : 6);
                            Polygon3D aNewPoly(nPolyPoints);

                            aNewPoly[0] = *pLeft + aPerpend;
                            aNewPoly[1] = *pRight + aPerpend;
                            aNewPoly[4] = *pRight - aPerpend;
                            aNewPoly[5] = *pLeft - aPerpend;

                            if(pPrev)
                                aNewPoly[6] = *pLeft;

                            if(fAngleDiff > F_PI)
                            {
                                // lower side
                                aNewPoly[2] = *pRight;
                                aNewPoly[3] = *pRight - aPerpendRight;
                            }
                            else
                            {
                                // upper side
                                aNewPoly[2] = *pRight + aPerpendRight;
                                aNewPoly[3] = *pRight;
                            }

                            aNewPoly.SetClosed(TRUE);
                            mrPolyPoly3D.Insert(aNewPoly);
                        }
                        else
                        {
                            BOOL bUseLowerSide(fAngleDiff > F_PI);
                            UINT16 nSegments;

                            if(bUseLowerSide)
                            {
                                fAngleDiff = (F_PI * 2.0) - fAngleDiff;
                                nSegments = (UINT16)(fAngleDiff / fDegreeStepWidth);
                            }
                            else
                            {
                                nSegments = (UINT16)(fAngleDiff / fDegreeStepWidth);
                            }

                            UINT16 nPolyPoints(pPrev ? 7 : 6);
                            Polygon3D aNewPoly(nPolyPoints + nSegments);

                            aNewPoly[0] = *pLeft + aPerpend;
                            aNewPoly[1] = *pRight + aPerpend;
                            aNewPoly[4 + nSegments] = *pRight - aPerpend;
                            aNewPoly[5 + nSegments] = *pLeft - aPerpend;

                            if(pPrev)
                                aNewPoly[6 + nSegments] = *pLeft;

                            fAngleDiff /= (double)(nSegments + 1);

                            if(bUseLowerSide)
                            {
                                // lower side
                                aNewPoly[2] = *pRight;
                                aNewPoly[3] = *pRight - aPerpendRight;

                                for(UINT16 a=0;a<nSegments;a++)
                                {
                                    double fDegree = fRightAngle - (double)a * fAngleDiff;
                                    Vector3D aNewPos(
                                        pRight->X() - (cos(fDegree) * fHalfLineWidth),
                                        pRight->Y() - (sin(fDegree) * fHalfLineWidth));
                                    aNewPoly[4 + a] = aNewPos;
                                }
                            }
                            else
                            {
                                // upper side
                                aNewPoly[2 + nSegments] = *pRight + aPerpendRight;
                                aNewPoly[3 + nSegments] = *pRight;

                                for(UINT16 a=0;a<nSegments;a++)
                                {
                                    double fDegree = fAngle - (double)a * fAngleDiff;
                                    Vector3D aNewPos(
                                        pRight->X() + (cos(fDegree) * fHalfLineWidth),
                                        pRight->Y() + (sin(fDegree) * fHalfLineWidth));
                                    aNewPoly[2 + a] = aNewPos;
                                }
                            }

                            aNewPoly.SetClosed(TRUE);
                            mrPolyPoly3D.Insert(aNewPoly);
                        }
                    }
                }

                if(bCreateSimplePart)
                {
                    // angle smaller DegreeStepWidth, create simple segment
                    UINT16 nNumPnt(4);

                    if(pPrev)
                        nNumPnt++;

                    if(pNext)
                        nNumPnt++;

                    Polygon3D aNewPoly(nNumPnt);

                    aPerpend *= fHalfLineWidth;
                    aPerpendRight *= fHalfLineWidth;
                    nNumPnt = 0;

                    if(pPrev)
                        aNewPoly[nNumPnt++] = *pLeft;

                    aNewPoly[nNumPnt++] = *pLeft + aPerpend;
                    aNewPoly[nNumPnt++] = *pRight + aPerpendRight;

                    if(pNext)
                        aNewPoly[nNumPnt++] = *pRight;

                    aNewPoly[nNumPnt++] = *pRight - aPerpendRight;
                    aNewPoly[nNumPnt++] = *pLeft - aPerpend;

                    aNewPoly.SetClosed(TRUE);
                    mrPolyPoly3D.Insert(aNewPoly);
                }

                break;
            }
            case XLINEJOINT_MITER:      // extend till cut
            {
                Polygon3D aNewPoly(4);
                aPerpend *= fHalfLineWidth;
                BOOL bLeftSolved(FALSE);
                BOOL bRightSolved(FALSE);

                if(pPrev)
                {
                    Vector3D aLeftVec(*pLeft - *pPrev);
                    Vector3D aPerpendLeft = aLeftVec.GetPerpendicular2D();
                    aPerpendLeft *= fHalfLineWidth;
                    double fUpperCut = ImpSimpleFindCutPoint(*pPrev + aPerpendLeft, aLeftVec, *pRight + aPerpend, -aEdge);

                    if(fUpperCut != 0.0 && fUpperCut < mrLineAttr.GetLinejointMiterUpperBound())
                    {
                        double fLowerCut = ImpSimpleFindCutPoint(*pPrev - aPerpendLeft, aLeftVec, *pRight - aPerpend, -aEdge);

                        if(fLowerCut < mrLineAttr.GetLinejointMiterUpperBound())
                        {
                            Vector3D aParam1 = *pPrev + aPerpendLeft;
                            Vector3D aParam2 = *pLeft + aPerpendLeft;
                            aNewPoly[0].CalcInBetween(aParam1, aParam2, fUpperCut);
                            aParam1 = *pPrev - aPerpendLeft;
                            aParam2 = *pLeft - aPerpendLeft;
                            aNewPoly[3].CalcInBetween(aParam1, aParam2, fLowerCut);
                            bLeftSolved = TRUE;
                        }
                    }
                }
                if(!bLeftSolved)
                {
                    aNewPoly[0] = *pLeft + aPerpend;
                    aNewPoly[3] = *pLeft - aPerpend;
                }

                if(pNext)
                {
                    Vector3D aRightVec(*pRight - *pNext);
                    Vector3D aPerpendRight = -aRightVec;
                    aPerpendRight = aPerpendRight.GetPerpendicular2D();
                    aPerpendRight *= fHalfLineWidth;
                    double fUpperCut = ImpSimpleFindCutPoint(*pNext + aPerpendRight, aRightVec, *pRight + aPerpend, aEdge);

                    if(fUpperCut != 0.0 && fUpperCut < mrLineAttr.GetLinejointMiterUpperBound())
                    {
                        double fLowerCut = ImpSimpleFindCutPoint(*pNext - aPerpendRight, aRightVec, *pRight - aPerpend, aEdge);

                        if(fLowerCut < mrLineAttr.GetLinejointMiterUpperBound())
                        {
                            Vector3D aParam1 = *pNext + aPerpendRight;
                            Vector3D aParam2 = *pRight + aPerpendRight;
                            aNewPoly[1].CalcInBetween(aParam1, aParam2, fUpperCut);
                            aParam1 = *pNext - aPerpendRight;
                            aParam2 = *pRight - aPerpendRight;
                            aNewPoly[2].CalcInBetween(aParam1, aParam2, fLowerCut);
                            bRightSolved = TRUE;
                        }
                    }
                }
                if(!bRightSolved)
                {
                    aNewPoly[1] = *pRight + aPerpend;
                    aNewPoly[2] = *pRight - aPerpend;
                }

                aNewPoly.SetClosed(TRUE);
                mrPolyPoly3D.Insert(aNewPoly);

                break;
            }
        }
    }
    else
    {
        Polygon3D aNewPoly(2);

        aNewPoly[0] = *pLeft;
        aNewPoly[1] = *pRight;

        aNewPoly.SetClosed(FALSE);
        mrPolyLine3D.Insert(aNewPoly);
    }
}

void LineGeometryCreator::ImpCreateSegmentsForLine(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext, double fPolyPos)
{
    Vector3D aEdge(*pRight - *pLeft);
    double fLen = aEdge.GetLength();
    double fPos = 0.0;
    double fDist;
    BOOL bFirst(TRUE);
    BOOL bLast(FALSE);
    UINT16 nInd = mrLineAttr.GetFirstDashDotIndex(fPolyPos, fDist);

    do {
        // nInd right edge, fDist to it
        if((nInd % 2) && fDist > SMALL_DVALUE)
        {
            // left is fpos, get right
            double fRight = fPos + fDist;

            if(fRight > fLen)
            {
                fRight = fLen;
                bLast = TRUE;
            }

            // create segment from fPos to fRight
            Vector3D aLeft(*pLeft);
            Vector3D aRight(*pRight);

            if(!bFirst)
                aLeft.CalcInBetween(*pLeft, *pRight, fPos / fLen);
            if(!bLast)
                aRight.CalcInBetween(*pLeft, *pRight, fRight / fLen);

            ImpCreateLineSegment(bFirst ? pPrev : 0L, &aLeft, &aRight, bLast ? pNext : 0L);
        }

        bFirst = FALSE;
        fPos += fDist;
        nInd = mrLineAttr.GetNextDashDotIndex(nInd, fDist);
    } while(fPos < fLen);
}

double LineGeometryCreator::ImpCreateLineStartEnd(Polygon3D& rArrowPoly, const Polygon3D& rSourcePoly, BOOL bFront, double fWantedWidth, BOOL bCentered)
{
    double fRetval(0.0);
    double fOffset(0.0);
    Volume3D aPolySize(rArrowPoly.GetPolySize());
    double fScaleValue(fWantedWidth / aPolySize.GetWidth());
    Matrix4D aTrans;
    Vector3D aCenter;

    if(bCentered)
    {
        aCenter = Vector3D(
            (aPolySize.MinVec().X() + aPolySize.MaxVec().X()) / 2.0,
            (aPolySize.MinVec().Y() + aPolySize.MaxVec().Y()) / 2.0, 0.0);
    }
    else
    {
        aCenter = Vector3D(rArrowPoly.GetMiddle());
    }

    aTrans.Translate(-aCenter);
    aTrans.Scale(fScaleValue, fScaleValue, fScaleValue);

    if(bCentered)
    {
        Vector3D aLowerCenter(aCenter.X(), aPolySize.MinVec().Y(), 0.0);

        aLowerCenter *= aTrans;
        aCenter *= aTrans;
        fOffset = (aCenter - aLowerCenter).GetLength();
        fRetval = fOffset / 2.0;
    }
    else
    {
        Vector3D aLowerCenter(aCenter.X(), aPolySize.MinVec().Y(), 0.0);
        Vector3D aUpperCenter(aCenter.X(), aPolySize.MaxVec().Y(), 0.0);

        aUpperCenter *= aTrans;
        aLowerCenter *= aTrans;
        fOffset = (aUpperCenter - aLowerCenter).GetLength();
        fRetval = fOffset * 0.8;
    }

    Vector3D aHead = (bFront) ? rSourcePoly[0] : rSourcePoly[rSourcePoly.GetPointCount() - 1];
    Vector3D aTail = (bFront) ? rSourcePoly[1] : rSourcePoly[rSourcePoly.GetPointCount() - 2];

    if(fOffset != 0.0)
    {
        if(!bFront)
            fOffset = rSourcePoly.GetLength() - fOffset;
        aTail = rSourcePoly.GetPosition(fOffset);
    }

    Vector3D aDirection = aHead - aTail;
    aDirection.Normalize();
    double fRotation = atan2(aDirection.Y(), aDirection.X()) - (90.0 * F_PI180);

    aTrans.RotateZ(fRotation);
    aTrans.Translate(aHead);

    if(!bCentered)
    {
        Vector3D aUpperCenter(aCenter.X(), aPolySize.MaxVec().Y(), 0.0);

        aUpperCenter *= aTrans;
        aCenter *= aTrans;
        aTrans.Translate(aCenter - aUpperCenter);
    }

    rArrowPoly.Transform(aTrans);
    rArrowPoly.SetClosed(TRUE);

    return fRetval;
}

void LineGeometryCreator::ImpCreateLineGeometry(const Polygon3D& rSourcePoly)
{
    UINT16 nPntCnt = rSourcePoly.GetPointCount();

    if(nPntCnt > 1)
    {
        BOOL bClosed = rSourcePoly.IsClosed();
        UINT16 nCount = nPntCnt;
        Polygon3D aPoly = rSourcePoly;

        if(!bClosed)
        {
            nCount = nPntCnt-1;
            double fPolyLength = rSourcePoly.GetLength();
            double fStart = 0.0;
            double fEnd = fPolyLength;

            if(mrLineAttr.IsStartActive())
            {
                // create line start polygon and move line end
                Polygon3D aArrowPoly(XOutCreatePolygon(mrLineAttr.GetStartPolygon(), &mrLineAttr.GetOutDev()));
                fStart = ImpCreateLineStartEnd(
                    aArrowPoly, rSourcePoly, TRUE,
                    (double)mrLineAttr.GetStartWidth(), mrLineAttr.IsStartCentered());
                mrPolyPoly3D.Insert(aArrowPoly);
            }

            if(mrLineAttr.IsEndActive())
            {
                // create line end polygon and move line end
                Polygon3D aArrowPoly(XOutCreatePolygon(mrLineAttr.GetEndPolygon(), &mrLineAttr.GetOutDev()));
                fEnd = fPolyLength - ImpCreateLineStartEnd(
                    aArrowPoly, rSourcePoly, FALSE,
                    (double)mrLineAttr.GetEndWidth(), mrLineAttr.IsEndCentered());
                mrPolyPoly3D.Insert(aArrowPoly);
            }

            if(fStart != 0.0 || fEnd != fPolyLength)
            {
                // build new poly, consume something from old poly
                aPoly = Polygon3D(nCount);
                UINT16 nInsPos(0);
                double fPolyPos = 0.0;

                for(UINT16 a=0;a<nCount;a++)
                {
                    Vector3D aEdge = rSourcePoly[a+1] - rSourcePoly[a];
                    double fLength = aEdge.GetLength();

                    if(fStart != 0.0)
                    {
                        if(fStart - fLength > -SMALL_DVALUE)
                        {
                            fStart -= fLength;
                        }
                        else
                        {
                            Vector3D aNewPos;
                            aNewPos.CalcInBetween(rSourcePoly[a], rSourcePoly[a+1], fStart / fLength);
                            aPoly[nInsPos++] = aNewPos;
                            fStart = 0.0;
                        }
                    }
                    else
                    {
                        aPoly[nInsPos++] = rSourcePoly[a];
                    }

                    if((fPolyPos + fLength) - fEnd > -SMALL_DVALUE)
                    {
                        Vector3D aNewPos;
                        aNewPos.CalcInBetween(rSourcePoly[a], rSourcePoly[a+1], (fEnd - fPolyPos) / fLength);
                        aPoly[nInsPos++] = aNewPos;
                        a = nCount;
                    }

                    // next PolyPos
                    fPolyPos += fLength;
                }

                nCount = aPoly.GetPointCount() - 1;
            }
        }

        if(nCount)
        {
            if(!mrLineAttr.GetLineWidth()
                && (mbLineDraft || mrLineAttr.GetLineStyle() == XLINE_SOLID))
            {
                // LineWidth zero, solid line -> add directly to linePoly
                mrPolyLine3D.Insert(aPoly);
            }
            else
            {
                const Vector3D* pPrev = NULL;
                const Vector3D* pLeft = NULL;
                const Vector3D* pRight = NULL;
                const Vector3D* pNext = NULL;
                double fPolyPos = 0.0;

                for(UINT16 a=0;a<nCount;a++)
                {
                    BOOL bStart(!a);
                    BOOL bEnd(a+1 == nCount);

                    // get left, right positions
                    pLeft = &aPoly[a];

                    // get length
                    if(bClosed)
                    {
                        pRight = &aPoly[(a+1) % nCount];
                        pPrev = &aPoly[(a+nCount-1) % nCount];
                        pNext = &aPoly[(a+2) % nCount];
                    }
                    else
                    {
                        pRight = &aPoly[a+1];

                        if(bStart)
                        {
                            pPrev = NULL;
                        }
                        else
                        {
                            pPrev = &aPoly[a-1];
                        }

                        if(bEnd)
                        {
                            pNext = NULL;
                        }
                        else
                        {
                            pNext = &aPoly[a+2];
                        }
                    }

                    // positions are in pPrev, pLeft, pRight and pNext.
                    if(!mbLineDraft && mrLineAttr.GetLineStyle() == XLINE_DASH)
                        ImpCreateSegmentsForLine(pPrev, pLeft, pRight, pNext, fPolyPos);
                    else
                        ImpCreateLineSegment(pPrev, pLeft, pRight, pNext);

                    // increment PolyPos
                    Vector3D aEdge = *pRight - *pLeft;
                    fPolyPos += aEdge.GetLength();
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions

SdrBroadcastItemChange::SdrBroadcastItemChange(const SdrObject& rObj)
{
    if(rObj.ISA(SdrObjGroup))
    {
        SdrObjListIter aIter((const SdrObjGroup&)rObj, IM_DEEPNOGROUPS);
        mpData = new List();

        while(aIter.IsMore())
        {
            SdrObject* pObj = aIter.Next();
            if(pObj)
                ((List*)mpData)->Insert(new Rectangle(pObj->GetBoundRect()), LIST_APPEND);
        }

        mnCount = ((List*)mpData)->Count();
    }
    else
    {
        mnCount = 1;
        mpData = new Rectangle(rObj.GetBoundRect());
    }
}

SdrBroadcastItemChange::~SdrBroadcastItemChange()
{
    if(mnCount > 1)
    {
        for(sal_uInt32 a(0); a < mnCount;a++)
            delete ((Rectangle*)((List*)mpData)->GetObject(a));
        delete ((List*)mpData);
    }
    else
        delete ((Rectangle*)mpData);
}

const Rectangle& SdrBroadcastItemChange::GetRectangle(sal_uInt32 nIndex) const
{
    if(mnCount > 1)
        return *((Rectangle*)((List*)mpData)->GetObject(nIndex));
    else
        return *((Rectangle*)mpData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@ @@@@@  @@@@  @@@@@@
//  @@  @@ @@  @@     @@ @@    @@  @@   @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@@@@      @@ @@@@  @@       @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@  @@ @@  @@ @@    @@  @@   @@
//   @@@@  @@@@@   @@@@  @@@@@  @@@@    @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObject);
TYPEINIT1(SdrObject,SfxListener);

SdrObject::SdrObject():
    pObjList(NULL),
    pPage(NULL),
    pModel(NULL),
    pUserCall(NULL),
    pPlusData(NULL),
    nOrdNum(0),
    nLayerId(0)
{
    DBG_CTOR(SdrObject,NULL);
    bVirtObj         =FALSE;
    bBoundRectDirty  =TRUE;
    bSnapRectDirty   =TRUE;
    bNetLock         =FALSE;
    bInserted        =FALSE;
    bGrouped         =FALSE;
    bMovProt         =FALSE;
    bSizProt         =FALSE;
    bNoPrint         =FALSE;
    bEmptyPresObj    =FALSE;
    bNotPersistent   =FALSE;
    bNeedColorRestore=FALSE;
    bNotVisibleAsMaster=FALSE;
    bClosedObj       =FALSE;
    bWriterFlyFrame  =FALSE;
    bNotMasterCachable=FALSE;
    bIsEdge=FALSE;
    bIs3DObj=FALSE;
    bMarkProt=FALSE;
    bIsUnoObj=FALSE;
}

SdrObject::~SdrObject()
{
    DBG_DTOR(SdrObject,NULL);
    SendUserCall(SDRUSERCALL_DELETE,GetBoundRect());
    if (pPlusData!=NULL) delete pPlusData;
}

SdrObjPlusData* SdrObject::NewPlusData() const
{
    return new SdrObjPlusData;
}

void SdrObject::SetRectsDirty(FASTBOOL bNotMyself)
{
    if (!bNotMyself) {
        bBoundRectDirty=TRUE;
        bSnapRectDirty=TRUE;
    }
    if (pObjList!=NULL) {
        pObjList->SetRectsDirty();
    }
}

void SdrObject::SetModel(SdrModel* pNewModel)
{
    if(pNewModel && pPage)
    {
        if(pPage->GetModel() != pNewModel)
        {
            pPage = NULL;
        }
    }

    pModel = pNewModel;
}

void SdrObject::SetObjList(SdrObjList* pNewObjList)
{
    pObjList=pNewObjList;
}

void SdrObject::SetPage(SdrPage* pNewPage)
{
    pPage=pNewPage;
    if (pPage!=NULL) {
        SdrModel* pMod=pPage->GetModel();
        if (pMod!=pModel && pMod!=NULL) {
            SetModel(pMod);
        }
    }
}

// init global static itempool
SdrItemPool* SdrObject::mpGlobalItemPool = NULL;

SdrItemPool* SdrObject::GetGlobalDrawObjectItemPool()
{
    if(!mpGlobalItemPool)
    {
        mpGlobalItemPool = new SdrItemPool(SDRATTR_START, SDRATTR_END);
        SfxItemPool* pGlobalOutlPool = EditEngine::CreatePool();
        mpGlobalItemPool->SetSecondaryPool(pGlobalOutlPool);
        mpGlobalItemPool->SetDefaultMetric((SfxMapUnit)SdrEngineDefaults::GetMapUnit());
        mpGlobalItemPool->FreezeIdRanges();
    }

    return mpGlobalItemPool;
}

void SdrObject::FreeGlobalDrawObjectItemPool()
{
    // code for deletion of GlobalItemPool
    if(mpGlobalItemPool)
    {
        SfxItemPool* pGlobalOutlPool = mpGlobalItemPool->GetSecondaryPool();
        delete mpGlobalItemPool;
        delete pGlobalOutlPool;
    }
}

SdrItemPool* SdrObject::GetItemPool() const
{
    if(pModel)
        return (SdrItemPool*)(&pModel->GetItemPool());

    // use a static global default pool
    return SdrObject::GetGlobalDrawObjectItemPool();
}

UINT32 SdrObject::GetObjInventor()   const
{
    return SdrInventor;
}

UINT16 SdrObject::GetObjIdentifier() const
{
    return UINT16(OBJ_NONE);
}

void SdrObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToContour = FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
}

SdrLayerID SdrObject::GetLayer() const
{
    return SdrLayerID(nLayerId);
}

void SdrObject::GetLayer(SetOfByte& rSet) const
{
    rSet.Set((BYTE)nLayerId);
    SdrObjList* pOL=GetSubList();
    if (pOL!=NULL) {
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            pOL->GetObj(nObjNum)->GetLayer(rSet);
        }
    }
}

void SdrObject::NbcSetLayer(SdrLayerID nLayer)
{
    nLayerId=nLayer;
}

void SdrObject::SetLayer(SdrLayerID nLayer)
{
    NbcSetLayer(nLayer);
    SetChanged();
    SendRepaintBroadcast();
}

void SdrObject::AddListener(SfxListener& rListener)
{
    ImpForcePlusData();
    if (pPlusData->pBroadcast==NULL) pPlusData->pBroadcast=new SfxBroadcaster;
    rListener.StartListening(*pPlusData->pBroadcast);
}

void SdrObject::RemoveListener(SfxListener& rListener)
{
    if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
        rListener.EndListening(*pPlusData->pBroadcast);
        if (!pPlusData->pBroadcast->HasListeners()) {
            delete pPlusData->pBroadcast;
            pPlusData->pBroadcast=NULL;
        }
    }
}

void SdrObject::AddReference(SdrVirtObj& rVrtObj)
{
    AddListener(rVrtObj);
}

void SdrObject::DelReference(SdrVirtObj& rVrtObj)
{
    RemoveListener(rVrtObj);
}

ImpSdrMtfAnimator* SdrObject::ImpForceMtfAnimator()
{
    ImpForcePlusData();
    if (pPlusData->pAnimator==NULL) pPlusData->pAnimator=new ImpSdrMtfAnimator;
    return pPlusData->pAnimator;
}

AutoTimer* SdrObject::ForceAutoTimer()
{
    ImpForcePlusData();
    if (pPlusData->pAutoTimer==NULL) pPlusData->pAutoTimer=new AutoTimer;
    return pPlusData->pAutoTimer;
}

FASTBOOL SdrObject::HasRefPoint() const
{
    return FALSE;
}

Point SdrObject::GetRefPoint() const
{
    return GetBoundRect().Center();
}

void SdrObject::SetRefPoint(const Point& /*rPnt*/)
{
}

SdrObjList* SdrObject::GetSubList() const
{
    return NULL;
}

SdrObject* SdrObject::GetUpGroup() const
{
    return pObjList!=NULL ? pObjList->GetOwnerObj() : NULL;
}

FASTBOOL SdrObject::HasSetName() const
{
    return TRUE;
}

void SdrObject::SetName(const XubString& rStr)
{
    if(rStr.Len())
    {
        ImpForcePlusData();
        pPlusData->aObjName = rStr;
    }
    else
    {
        if(pPlusData)
        {
            pPlusData->aObjName = rStr;
        }
    }
}

XubString SdrObject::GetName() const
{
    SdrObjPlusData* pPlus=pPlusData;
    if (pPlus!=NULL) {
        return pPlus->aObjName;
    }
    return String();
}

UINT32 SdrObject::GetOrdNum() const
{
    if (pObjList!=NULL) {
        if (pObjList->IsObjOrdNumsDirty()) {
            pObjList->RecalcObjOrdNums();
        }
    } else ((SdrObject*)this)->nOrdNum=0;
    return nOrdNum;
}

const Rectangle& SdrObject::GetBoundRect() const
{
    if (bBoundRectDirty) {
        ((SdrObject*)this)->RecalcBoundRect();
        ((SdrObject*)this)->bBoundRectDirty=FALSE;
    }
    return aOutRect;
}

void SdrObject::RecalcBoundRect()
{
}

void SdrObject::SendRepaintBroadcast(const Rectangle& rRect) const
{
    BOOL bBrd(pPlusData && pPlusData->pBroadcast);
    BOOL bPnt(bInserted && pModel);

    if(bPnt || bBrd)
    {
        SdrHint aHint(*this, rRect);

        if(bBrd)
            pPlusData->pBroadcast->Broadcast(aHint);

        if(bPnt)
            pModel->Broadcast(aHint);

        // alle Animationen wegen Obj-Aenderung neustarten
        RestartAnimation(NULL);
    }
}

void SdrObject::SendRepaintBroadcast(BOOL bNoPaintNeeded) const
{
    BOOL bBrd(pPlusData && pPlusData->pBroadcast);
    BOOL bPnt(bInserted && pModel);

    if(bPnt || bBrd)
    {
        SdrHint aHint(*this);
        aHint.SetNeedRepaint(!bNoPaintNeeded);

        if(bBrd)
            pPlusData->pBroadcast->Broadcast(aHint);

        if(bPnt)
            pModel->Broadcast(aHint);

        // alle Animationen wegen Obj-Aenderung neustarten
        RestartAnimation(NULL);
    }
}

void SdrObject::SetChanged()
{
    if (bInserted && pModel!=NULL) pModel->SetChanged();
}

FASTBOOL SdrObject::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& /*rInfoRec*/) const
{
    Color aRedColor( COL_RED );
    Color aYellowColor( COL_YELLOW );
    rXOut.OverrideLineColor( aRedColor );
    rXOut.OverrideFillColor( aYellowColor );
    rXOut.DrawRect(GetBoundRect());

    return TRUE;
}

void SdrObject::CreateLinePoly(PolyPolygon3D& rPolyPolygon, PolyPolygon3D& rPolyLine, OutputDevice& rOut,
    BOOL bForceHair, BOOL bIsLineDraft) const
{
    // get XOR Poly as base
    XPolyPolygon aTmpPolyPolygon;
    TakeXorPoly(aTmpPolyPolygon, TRUE);

    // get LineStyleParameterPack
    LineStyleParameterPack aLineAttr(GetItemSet(), bForceHair || bIsLineDraft, rOut);
    LineGeometryCreator aLineCreator(aLineAttr, rPolyPolygon, rPolyLine, bIsLineDraft);

    // compute single lines
    for(UINT16 a=0;a<aTmpPolyPolygon.Count();a++)
    {
        // expand splines into polygons and convert to double
        Polygon3D aPoly3D(XOutCreatePolygon(aTmpPolyPolygon[a], &rOut));
        aPoly3D.RemoveDoublePoints();

        // convert line to single Polygons; make sure the part
        // polygons are all clockwise oriented
        aLineCreator.AddPolygon3D(aPoly3D);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//#define TEST_SKELETON
#ifdef TEST_SKELETON

static OutputDevice* pImpTestOut = 0L;

class ImpSkeleton;

class ImpSkeletonNode
{
    const ImpSkeleton&          mrParent;
    ImpSkeletonNode*            mpNext;
    ImpSkeletonNode*            mpPrev;
    ImpSkeletonNode*            mpUp;
    ImpSkeletonNode*            mpDown;

    Vector3D                    maPosition;
    Vector3D                    maDirection;
    double                      mfDistance;
    BOOL                        mbDirection;

public:
    ImpSkeletonNode(const ImpSkeleton& rPrnt, const Vector3D& rPos, const Vector3D& rDir, BOOL bDir);
    ~ImpSkeletonNode();

    ImpSkeletonNode* GetNext() const { return mpNext; }
    void SetNext(ImpSkeletonNode* pNew) { mpNext = pNew; }

    ImpSkeletonNode* GetPrev() const { return mpPrev; }
    void SetPrev(ImpSkeletonNode* pNew) { mpPrev = pNew; }

    ImpSkeletonNode* GetUp() const { return mpUp; }
    void SetUp(ImpSkeletonNode* pNew) { mpUp = pNew; }

    ImpSkeletonNode* GetDown() const { return mpDown; }
    void SetDown(ImpSkeletonNode* pNew) { mpDown = pNew; }

    const Vector3D& GetPosition() const { return maPosition; }
    const Vector3D& GetDirection() const { return maDirection; }
    BOOL GetOrientation() const { return mbDirection; }

    double GetDistance() const { return mfDistance; }
    void SetDistance(double fNew) { mfDistance = fNew; }

    void Paint(Color aCol);
};

///////////////////////////////////////////////////////////////////////////////

DECLARE_LIST(ImpSkeletonNodeList, ImpSkeletonNode*);

///////////////////////////////////////////////////////////////////////////////

class ImpSkeleton
{
    ImpSkeletonNodeList         maList;
    ImpSkeletonNode*            mpList;

public:
    ImpSkeleton(const Polygon3D& rPoly);
    ~ImpSkeleton();

    BOOL ImpSimpleFindCutPoint(
        const Vector3D& rEdge1Start, const Vector3D& rEdge1Delta,
        const Vector3D& rEdge2Start, const Vector3D& rEdge2Delta,
        double& rCutA, double& rCutB) const;

    void PaintTree(ImpSkeletonNode* pNode, Color aCol);
};

///////////////////////////////////////////////////////////////////////////////

ImpSkeletonNode::ImpSkeletonNode(const ImpSkeleton& rPrnt, const Vector3D& rPos, const Vector3D& rDir, BOOL bDir)
:   mrParent(rPrnt),
    mpNext(this),
    mpPrev(this),
    mpUp(0L),
    mpDown(0L),
    maPosition(rPos),
    maDirection(rDir),
    mfDistance(0.0),
    mbDirection(bDir)
{
}

ImpSkeletonNode::~ImpSkeletonNode()
{
}

void ImpSkeletonNode::Paint(Color aCol)
{
    if(pImpTestOut)
    {
        Vector3D aP1 = GetPosition();
        Vector3D aP2 = aP1 + (GetDirection() * 5000.0);
        Vector3D aP3 = aP1 - (GetDirection() * 1000.0);

        Point aPn1((INT32)aP1.X(), (INT32)-aP1.Y());
        Point aPn2((INT32)aP2.X(), (INT32)-aP2.Y());
        Point aPn3((INT32)aP3.X(), (INT32)-aP3.Y());

        pImpTestOut->SetLineColor(Color(aCol));
        pImpTestOut->DrawLine(aPn1, aPn2);
        pImpTestOut->SetLineColor(Color(COL_BLACK));
        pImpTestOut->DrawLine(aPn1, aPn3);
    }
}

///////////////////////////////////////////////////////////////////////////////

ImpSkeleton::ImpSkeleton(const Polygon3D& rPoly)
:   mpList(0L)
{
    UINT16 nCnt(rPoly.GetPointCount());

    // build base list
    for(UINT16 a=0;a<nCnt;a++)
    {
        Vector3D aPrevDir = rPoly[(a + nCnt - 1) % nCnt] - rPoly[a];
        Vector3D aNextDir = rPoly[(a + 1) % nCnt] - rPoly[a];

        aPrevDir.Normalize();
        aNextDir.Normalize();

        Vector3D aMidDir = aPrevDir + aNextDir;

        aMidDir.Normalize();

        BOOL bDirection((aPrevDir.Y() * aNextDir.X() - aPrevDir.X() * aNextDir.Y()) > -SMALL_DVALUE);
        ImpSkeletonNode* pNewNode = new ImpSkeletonNode(*this, rPoly[a], aMidDir, bDirection);
        maList.Insert(pNewNode);

        if(mpList)
        {
            pNewNode->SetNext(mpList->GetNext());
            mpList->SetNext(pNewNode);
            pNewNode->GetNext()->SetPrev(pNewNode);
            pNewNode->SetPrev(mpList);
        }

        mpList = pNewNode;

        pNewNode->Paint(COL_YELLOW);
    }

    // iterate
    while(nCnt > 1)
    {
        // make step
        double fLowestDist(DBL_MAX);
        double fLowestCut;
        ImpSkeletonNode* pCand = NULL;
        ImpSkeletonNode* pCurr = mpList;

        do {
            ImpSkeletonNode* pNext = pCurr->GetNext();
            double fCutA, fCutB;

            pCurr->Paint(COL_RED);
            pNext->Paint(COL_LIGHTRED);

            if(ImpSimpleFindCutPoint(pCurr->GetPosition(), pCurr->GetDirection(),
                pNext->GetPosition(), pNext->GetDirection(), fCutA, fCutB))
            {
                if((!pCurr->GetOrientation() && fCutA > 0.0) || (pCurr->GetOrientation() && fCutA < 0.0))
                {
                    Vector3D aCutPos;
                    Vector3D aMidPos = (pCurr->GetPosition() + pNext->GetPosition()) / 2.0;

                    aCutPos.CalcInBetween(
                        pCurr->GetPosition(),
                        pCurr->GetPosition() + pCurr->GetDirection(), fCutA);

                    aCutPos -= aMidPos;

                    double fComp = aCutPos.GetLength() + pCurr->GetDistance() + pNext->GetDistance();

                    if(fComp < fLowestDist)
                    {
                        fLowestDist = fComp;
                        fLowestCut = fCutA;
                        pCand = pCurr;
                    }
                }
            }

            pCurr->Paint(COL_YELLOW);
            pNext->Paint(COL_YELLOW);

            pCurr = pNext;
        } while(pCurr != mpList);

        if(pCand)
        {
            // create cut entry and re-arrange tree
            Vector3D aCutPos;
            aCutPos.CalcInBetween(pCand->GetPosition(), pCand->GetPosition() + pCand->GetDirection(), fLowestCut);

            ImpSkeletonNode* pLeft = pCand;
            ImpSkeletonNode* pRight = pCand->GetNext();

            pLeft->Paint(COL_RED);
            pRight->Paint(COL_LIGHTRED);

            Vector3D aPrevDir = pLeft->GetPrev()->GetPosition() - pLeft->GetPosition();
            Vector3D aNextDir = pRight->GetNext()->GetPosition() - pRight->GetPosition();

            aPrevDir.Normalize();
            aNextDir.Normalize();

            Vector3D aMidDir = aPrevDir + aNextDir;

            aMidDir.Normalize();

            BOOL bDirection((aPrevDir.Y() * aNextDir.X() - aPrevDir.X() * aNextDir.Y()) > -SMALL_DVALUE);
            ImpSkeletonNode* pNewNode = new ImpSkeletonNode(*this, aCutPos, aMidDir, bDirection);
            maList.Insert(pNewNode);

            // set distance
            Vector3D aMidPoint = (pLeft->GetPosition() + pRight->GetPosition()) / 2.0;
            pNewNode->SetDistance(aMidPoint.GetLength());

            // add new node to old chain
            pNewNode->SetPrev(pLeft->GetPrev());
            pLeft->GetPrev()->SetNext(pNewNode);
            pNewNode->SetNext(pRight->GetNext());
            pRight->GetNext()->SetPrev(pNewNode);

            // Set Up-Down chaining
            pNewNode->SetDown(pLeft);
            pLeft->SetUp(pNewNode);
            pRight->SetUp(pNewNode);

            // close lower chain
            pLeft->SetPrev(pRight);
            pRight->SetNext(pLeft);

            nCnt--;
            mpList = pNewNode;

            pLeft->Paint(COL_BLUE);
            pRight->Paint(COL_LIGHTBLUE);

            pNewNode->Paint(COL_LIGHTGREEN);
        }
        else
        {
            // last lines are all parallel, group together
            pCurr = mpList;
            Vector3D aMidPos;
            UINT16 nNumber(0);

            do {
                aMidPos += pCurr->GetPosition();
                nNumber++;
                pCurr = pCurr->GetNext();
            } while(pCurr != mpList);

            aMidPos /= (double)nNumber;

            ImpSkeletonNode* pNewNode = new ImpSkeletonNode(*this, aMidPos, Vector3D(), FALSE);
            maList.Insert(pNewNode);

            // Set Up-Down chaining
            pNewNode->SetDown(mpList);
            pCurr = mpList;

            do {
                pCurr->SetUp(pNewNode);
                pCurr->Paint(COL_BLUE);
                pCurr = pCurr->GetNext();
            } while(pCurr != mpList);

            mpList = pNewNode;

            nCnt = 1;
        }
    }

    // show found centers
    PaintTree(mpList, COL_LIGHTRED);
}

ImpSkeleton::~ImpSkeleton()
{
    while(maList.Count())
        delete maList.Remove((UINT32)0L);
}

BOOL ImpSkeleton::ImpSimpleFindCutPoint(
    const Vector3D& rEdge1Start, const Vector3D& rEdge1Delta,
    const Vector3D& rEdge2Start, const Vector3D& rEdge2Delta,
    double& rCutA, double& rCutB) const
{
    double fZwi = (rEdge1Delta.X() * rEdge2Delta.Y()) - (rEdge1Delta.Y() * rEdge2Delta.X());
    rCutA = 0.0;

    if(fabs(fZwi) > SMALL_DVALUE)
    {
        rCutA = (rEdge2Delta.Y() * (rEdge2Start.X() - rEdge1Start.X()) + rEdge2Delta.X() * (rEdge1Start.Y() - rEdge2Start.Y())) / fZwi;

        if(fabs(rEdge2Delta.X()) > fabs(rEdge2Delta.Y()))
            rCutB = (rEdge1Start.X() + rCutA * rEdge1Delta.X() - rEdge2Start.X()) / rEdge2Delta.X();
        else
            rCutB = (rEdge1Start.Y() + rCutA * rEdge1Delta.Y() - rEdge2Start.Y()) / rEdge2Delta.Y();

        return TRUE;
    }

    return FALSE;
}

void ImpSkeleton::PaintTree(ImpSkeletonNode* pNode, Color aCol)
{
    if(pImpTestOut)
    {
        Vector3D aP1 = pNode->GetPosition();
        Point aPn1((INT32)aP1.X(), (INT32)-aP1.Y());
        ImpSkeletonNode* pDown = pNode->GetDown();

        if(pDown)
        {
            do {
                Vector3D aP2 = pDown->GetPosition();
                Point aPn2((INT32)aP2.X(), (INT32)-aP2.Y());

                pImpTestOut->SetLineColor(Color(aCol));
                pImpTestOut->DrawLine(aPn1, aPn2);

                PaintTree(pDown, aCol);

                pDown = pDown->GetNext();
            } while(pDown != pNode->GetDown());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void PolyPolygon3D_BuildSkeletonsAndGrow(const PolyPolygon3D& rPolyPoly)
{
    for(UINT16 a=0;a<rPolyPoly.Count();a++)
    {
        const Polygon3D& rPoly = rPolyPoly[a];
        ImpSkeleton aSkeleton(rPoly);
    }
}

#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ImpLineGeometry* SdrObject::ImpPrepareLineGeometry(ExtOutputDevice& rXOut, const SfxItemSet& rSet,
    BOOL bIsLineDraft) const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();
    if(eXLS != XLINE_NONE)
    {
        PolyPolygon3D aPolyPoly3D;
        PolyPolygon3D aLinePoly3D;

        // need to force single point line?
        INT32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        Size aSize(nLineWidth, nLineWidth);
        aSize = rXOut.GetOutDev()->LogicToPixel(aSize);
        BOOL bForceOnePixel(aSize.Width() <= 1 || aSize.Height() <= 1);
        BOOL bForceTwoPixel(!bForceOnePixel && (aSize.Width() <= 2 || aSize.Height() <= 2));

        // no force to two pixel when connected to MetaFile, so that not
        // four lines instead of one is recorded (e.g.)
        if(bForceTwoPixel && rXOut.GetOutDev()->GetConnectMetaFile())
            bForceTwoPixel = FALSE;

        // #78210# switch off bForceTwoPixel when line draft mode
        if(bForceTwoPixel && bIsLineDraft)
        {
            bForceTwoPixel = FALSE;
            bForceOnePixel = TRUE;
        }

        // create line geometry
        CreateLinePoly(aPolyPoly3D, aLinePoly3D, *rXOut.GetOutDev(),
            bForceOnePixel || bForceTwoPixel, bIsLineDraft);

        if(aPolyPoly3D.Count() || aLinePoly3D.Count())
            return new ImpLineGeometry(aPolyPoly3D, aLinePoly3D, bForceOnePixel, bForceTwoPixel);
    }

    return 0L;
}

void SdrObject::ImpDrawShadowLineGeometry(
    ExtOutputDevice& rXOut, const SfxItemSet& rSet, ImpLineGeometry& rLineGeometry) const
{
    sal_uInt32 nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
    sal_uInt32 nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
    const SdrShadowColorItem& rShadColItem = ((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
    Color aColor(rShadColItem.GetValue());
    sal_uInt16 nTrans = ((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();

    if(nXDist || nYDist)
    {
        // transformation necessary
        PolyPolygon3D aRememberPolyPoly = rLineGeometry.GetPolyPoly3D();
        PolyPolygon3D aRememberLinePoly = rLineGeometry.GetLinePoly3D();
        Matrix4D aTrans;

        aTrans.Translate((double)nXDist, -(double)nYDist, 0.0);
        rLineGeometry.GetPolyPoly3D().Transform(aTrans);
        rLineGeometry.GetLinePoly3D().Transform(aTrans);

        // draw the line geometry
        ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry);

        // reset line geometry to old values
        rLineGeometry.GetPolyPoly3D() = aRememberPolyPoly;
        rLineGeometry.GetLinePoly3D() = aRememberLinePoly;
    }
    else
    {
        // draw shadow line geometry
        ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry);
    }
}

void SdrObject::ImpDrawColorLineGeometry(
    ExtOutputDevice& rXOut, const SfxItemSet& rSet, ImpLineGeometry& rLineGeometry) const
{
    Color aColor = ((XLineColorItem&)rSet.Get(XATTR_LINECOLOR)).GetValue();
    sal_uInt16 nTrans = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();

    // draw the line geometry
    ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry);
}

void SdrObject::ImpDrawLineGeometry(
    ExtOutputDevice& rXOut,
    Color& rColor,
    sal_uInt16 nTransparence,
    ImpLineGeometry& rLineGeometry) const
{
    // #72796# black/white option active?
    UINT32 nOldDrawMode(rXOut.GetOutDev()->GetDrawMode());
    BOOL bDrawModePatched(FALSE);

    // #72796# if yes, force to DRAWMODE_BLACKFILL for these are LINES to be drawn
    // as polygons
    if((nOldDrawMode & DRAWMODE_WHITEFILL) && (nOldDrawMode && DRAWMODE_BLACKLINE))
    {
        rXOut.GetOutDev()->SetDrawMode(nOldDrawMode|DRAWMODE_BLACKFILL);
        bDrawModePatched = TRUE;
    }

    if(nTransparence)
    {
        if(nTransparence != 100)
        {
            // to be shown line has transparence, output via MetaFile
            UINT8 nScaledTrans((UINT8)((nTransparence * 255)/100));
            Color aTransColor(nScaledTrans, nScaledTrans, nScaledTrans);
            Gradient aGradient(GRADIENT_LINEAR, aTransColor, aTransColor);
            GDIMetaFile aMetaFile;
            VirtualDevice aVDev;
            Volume3D aVolume;
            MapMode aMap(rXOut.GetOutDev()->GetMapMode());

            // StepCount to someting small
            aGradient.SetSteps(3);

            // get bounds of geometry
            if(rLineGeometry.GetPolyPoly3D().Count())
                aVolume.Union(rLineGeometry.GetPolyPoly3D().GetPolySize());
            if(rLineGeometry.GetLinePoly3D().Count())
                aVolume.Union(rLineGeometry.GetLinePoly3D().GetPolySize());

            // get pixel size in logic coor for 1,2 pixel cases
            Size aSizeSinglePixel(1, 1);

            if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
                aSizeSinglePixel = rXOut.GetOutDev()->PixelToLogic(aSizeSinglePixel);

            // create BoundRectangle
            Rectangle aBound(
                (INT32)aVolume.MinVec().X(),
                (INT32)-aVolume.MaxVec().Y(),
                (INT32)aVolume.MaxVec().X(),
                (INT32)-aVolume.MinVec().Y());

            if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
            {
                // enlarge aBound
                if(rLineGeometry.DoForceTwoPixel())
                {
                    aBound.Right() += 2 * (aSizeSinglePixel.Width() - 1);
                    aBound.Bottom() += 2 * (aSizeSinglePixel.Height() - 1);
                }
                else
                {
                    aBound.Right() += (aSizeSinglePixel.Width() - 1);
                    aBound.Bottom() += (aSizeSinglePixel.Height() - 1);
                }
            }

            // prepare VDev and MetaFile
            aVDev.EnableOutput(FALSE);
            aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
            aMetaFile.Record(&aVDev);
            aVDev.SetLineColor(rColor);
            aVDev.SetFillColor(rColor);
            aVDev.SetFont(rXOut.GetOutDev()->GetFont());
            aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
            aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());

            // create output
            if(rLineGeometry.GetPolyPoly3D().Count())
            {
                PolyPolygon aPolyPoly = rLineGeometry.GetPolyPoly3D().GetPolyPolygon();

                for(UINT16 a=0;a<aPolyPoly.Count();a++)
                    aMetaFile.AddAction(new MetaPolygonAction(aPolyPoly[a]));
            }

            if(rLineGeometry.GetLinePoly3D().Count())
            {
                PolyPolygon aLinePoly = rLineGeometry.GetLinePoly3D().GetPolyPolygon();

                if(rLineGeometry.DoForceTwoPixel())
                {
                    UINT16 a;

                    for(a=0;a<aLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aLinePoly[a]));

                    aLinePoly.Move(aSizeSinglePixel.Width() - 1, 0);

                    for(a=0;a<aLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aLinePoly[a]));

                    aLinePoly.Move(0, aSizeSinglePixel.Height() - 1);

                    for(a=0;a<aLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aLinePoly[a]));

                    aLinePoly.Move(-aSizeSinglePixel.Width() - 1, 0);

                    for(a=0;a<aLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aLinePoly[a]));
                }
                else
                {
                    for(UINT16 a=0;a<aLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aLinePoly[a]));
                }
            }

            // draw metafile
            aMetaFile.Stop();
            aMetaFile.WindStart();
            aMap.SetOrigin(aBound.TopLeft());
            aMetaFile.SetPrefMapMode(aMap);
            aMetaFile.SetPrefSize(aBound.GetSize());
            rXOut.GetOutDev()->DrawTransparent(aMetaFile, aBound.TopLeft(), aBound.GetSize(), aGradient);
        }
    }
    else
    {
        // no transparence, simple output
        if(rLineGeometry.GetPolyPoly3D().Count())
        {
            PolyPolygon aPolyPoly = rLineGeometry.GetPolyPoly3D().GetPolyPolygon();

            rXOut.GetOutDev()->SetLineColor();
            rXOut.GetOutDev()->SetFillColor(rColor);

            for(UINT16 a=0;a<aPolyPoly.Count();a++)
                rXOut.GetOutDev()->DrawPolygon(aPolyPoly[a]);
        }

        if(rLineGeometry.GetLinePoly3D().Count())
        {
            PolyPolygon aLinePoly = rLineGeometry.GetLinePoly3D().GetPolyPolygon();

            rXOut.GetOutDev()->SetLineColor(rColor);
            rXOut.GetOutDev()->SetFillColor();

            if(rLineGeometry.DoForceTwoPixel())
            {
                PolyPolygon aPolyPolyPixel = rXOut.GetOutDev()->LogicToPixel(aLinePoly);
                BOOL bWasEnabled = rXOut.GetOutDev()->IsMapModeEnabled();
                rXOut.GetOutDev()->EnableMapMode(FALSE);
                UINT16 a;

                for(a=0;a<aLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(1,0);

                for(a=0;a<aLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(0,1);

                for(a=0;a<aLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(-1,0);

                for(a=0;a<aLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                rXOut.GetOutDev()->EnableMapMode(bWasEnabled);
            }
            else
            {
                for(UINT16 a=0;a<aLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aLinePoly[a]);
            }
        }
    }

    // #72796# restore DrawMode
    if(bDrawModePatched)
        rXOut.GetOutDev()->SetDrawMode(nOldDrawMode);
}

BOOL SdrObject::LineGeometryUsageIsNecessary() const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)GetItem(XATTR_LINESTYLE)).GetValue();
    return (eXLS != XLINE_NONE);
}

///////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrObject::PaintGluePoints(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    // Nur Klebepunkte der aktuellen ObjList Painten
    if (rInfoRec.pPV==NULL || rInfoRec.pPV->GetObjList()==pObjList) {
        const SdrGluePointList* pGPL=GetGluePointList();
        if (pGPL!=NULL) {
            OutputDevice* pOut=rXOut.GetOutDev();
            pGPL->DrawAll(*pOut,this);
        }
    }
    return TRUE;
}

SdrObject* SdrObject::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if (pVisiLayer!=NULL && !pVisiLayer->IsSet(nLayerId)) return NULL;
    Rectangle aO(GetBoundRect());
    aO.Left()-=nTol; aO.Top()-=nTol; aO.Right()+=nTol; aO.Bottom()+=nTol;
    FASTBOOL bRet=aO.IsInside(rPnt);
    return bRet ? (SdrObject*)this : NULL;
}

SdrObject* SdrObject::Clone() const
{
    SdrObject* pObj=SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL);
    if (pObj!=NULL) {
        pObj->pModel=pModel;
        pObj->pPage=pPage;
        *pObj=*this;
    }
    return pObj;
}

SdrObject* SdrObject::Clone(SdrPage* pNewPage, SdrModel* pNewModel) const
{
    SdrObject* pObj=SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL);
    if (pObj!=NULL) {
        pObj->pModel=pNewModel;
        pObj->pPage=pNewPage;
        *pObj=*this;
    }
    return pObj;
}

void SdrObject::operator=(const SdrObject& rObj)
{
    pModel  =rObj.pModel;
    aOutRect=rObj.GetBoundRect();
    nLayerId=rObj.GetLayer();
    aAnchor =rObj.aAnchor;
    bVirtObj=rObj.bVirtObj;
    bSizProt=rObj.bSizProt;
    bMovProt=rObj.bMovProt;
    bNoPrint=rObj.bNoPrint;
    //EmptyPresObj wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bEmptyPresObj =rObj.bEmptyPresObj;
    //NotVisibleAsMaster wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bNotVisibleAsMaster=rObj.bNotVisibleAsMaster;

    bBoundRectDirty=rObj.bBoundRectDirty;
    bSnapRectDirty=TRUE; //rObj.bSnapRectDirty;
    bNotMasterCachable=rObj.bNotMasterCachable;
    if (pPlusData!=NULL) { delete pPlusData; pPlusData=NULL; }
    if (rObj.pPlusData!=NULL) {
        pPlusData=rObj.pPlusData->Clone(this);
    }
    if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
        delete pPlusData->pBroadcast; // der Broadcaster wird nicht mitkopiert
        pPlusData->pBroadcast=NULL;
    }
}

void SdrObject::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulNONE);
}

void SdrObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralNONE);
}

void SdrObject::ImpTakeDescriptionStr(USHORT nStrCacheID, XubString& rStr, USHORT nVal) const
{
    rStr = ImpGetResStr(nStrCacheID);

    sal_Char aSearchText1[] = "%O";
    sal_Char aSearchText2[] = "%N";
    xub_StrLen nPos = rStr.SearchAscii(aSearchText1);

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);

        XubString aObjName;

        TakeObjNameSingul(aObjName);
        rStr.Insert(aObjName, nPos);
    }

    nPos = rStr.SearchAscii(aSearchText2);

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);
        rStr.Insert(UniString::CreateFromInt32(nVal), nPos);
    }
}

XubString SdrObject::GetWinkStr(long nWink, FASTBOOL bNoDegChar) const
{
    XubString aStr;
    if (pModel!=NULL) {
        pModel->TakeWinkStr(nWink,aStr,bNoDegChar);
    }
    return aStr;
}

XubString SdrObject::GetMetrStr(long nVal, MapUnit eWantMap, FASTBOOL bNoUnitChars) const
{
    XubString aStr;
    if (pModel!=NULL) {
        pModel->TakeMetricStr(nVal,aStr,bNoUnitChars);
    }
    return aStr;
}

void SdrObject::TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL /*bDetail*/) const
{
    rPoly=XPolyPolygon(XPolygon(GetBoundRect()));
}

void SdrObject::TakeContour( XPolyPolygon& rPoly ) const
{
    VirtualDevice   aBlackHole;
    GDIMetaFile     aMtf;
    SdrPaintInfoRec aPaintInfo;
    XPolygon        aXPoly;

    aBlackHole.EnableOutput( FALSE );
    aBlackHole.SetDrawMode( DRAWMODE_NOFILL );

    ExtOutputDevice aXOut( &aBlackHole );
    SdrObject*      pClone = Clone();
    pClone->SetItem(XLineStyleItem(XLINE_SOLID));
    pClone->SetItem(XLineColorItem(String(), Color(COL_BLACK)));
    pClone->SetItem(XFillStyleItem(XFILL_NONE));

    aMtf.Record( &aBlackHole );
    aPaintInfo.nPaintMode = SDRPAINTMODE_DRAFTTEXT | SDRPAINTMODE_DRAFTGRAF;
    pClone->Paint( aXOut, aPaintInfo );
    delete pClone;
    aMtf.Stop();
    aMtf.WindStart();
    rPoly.Clear();

    for( ULONG nActionNum = 0, nActionAnz = aMtf.GetActionCount(); nActionNum < nActionAnz; nActionNum++ )
    {
        const MetaAction&   rAct = *aMtf.GetAction( nActionNum );
        BOOL                bXPoly = FALSE;

        switch( rAct.GetType() )
        {
            case META_RECT_ACTION:
            {
                const Rectangle& rRect = ( (const MetaRectAction&) rAct ).GetRect();

                if( rRect.GetWidth() && rRect.GetHeight() )
                {
                    aXPoly = rRect;
                    bXPoly = TRUE;
                }
            }
            break;

            case META_ELLIPSE_ACTION:
            {
                const Rectangle& rRect = ( (const MetaEllipseAction&) rAct ).GetRect();

                if( rRect.GetWidth() && rRect.GetHeight() )
                {
                    aXPoly = XPolygon( rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1 );
                    bXPoly = TRUE;
                }
            }
            break;

            case META_POLYGON_ACTION:
            {
                const Polygon& rPoly = ( (const MetaPolygonAction&) rAct ).GetPolygon();

                if( rPoly.GetSize() > 2 )
                {
                    aXPoly = rPoly;
                    bXPoly = TRUE;
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const PolyPolygon& rPolyPoly = ( (const MetaPolyPolygonAction&) rAct ).GetPolyPolygon();

                if( rPolyPoly.Count() && ( rPolyPoly[ 0 ].GetSize() > 2 ) )
                    rPoly.Insert( rPolyPoly );
            }
            break;

            case META_POLYLINE_ACTION:
            {
                const Polygon& rPoly = ( (const MetaPolyLineAction&) rAct ).GetPolygon();

                if( rPoly.GetSize() > 1 )
                {
                    aXPoly = rPoly;
                    bXPoly = TRUE;
                }
            }
            break;

            case META_LINE_ACTION:
            {
                aXPoly = XPolygon( 2 );
                aXPoly[ 0 ] = ( (const MetaLineAction&) rAct ).GetStartPoint();
                aXPoly[ 1 ] = ( (const MetaLineAction&) rAct ).GetEndPoint();
                bXPoly = TRUE;
            }
            break;

            default:
            break;
        }

        if( bXPoly )
            rPoly.Insert( aXPoly );
    }

    // if we only have the outline of the object, we have _no_ contouir
    if( rPoly.Count() == 1 )
        rPoly.Clear();
}

void SdrObject::TakeContour(XPolyPolygon& rXPoly, SdrContourType eType) const
{
}

// Handles

USHORT SdrObject::GetHdlCount() const
{
    return 8;
}

SdrHdl* SdrObject::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pH=NULL;
    const Rectangle& rR=GetSnapRect();
    switch (nHdlNum) {
        case 0: pH=new SdrHdl(rR.TopLeft(),     HDL_UPLFT); break; // Oben links
        case 1: pH=new SdrHdl(rR.TopCenter(),   HDL_UPPER); break; // Oben
        case 2: pH=new SdrHdl(rR.TopRight(),    HDL_UPRGT); break; // Oben rechts
        case 3: pH=new SdrHdl(rR.LeftCenter(),  HDL_LEFT ); break; // Links
        case 4: pH=new SdrHdl(rR.RightCenter(), HDL_RIGHT); break; // Rechts
        case 5: pH=new SdrHdl(rR.BottomLeft(),  HDL_LWLFT); break; // Unten links
        case 6: pH=new SdrHdl(rR.BottomCenter(),HDL_LOWER); break; // Unten
        case 7: pH=new SdrHdl(rR.BottomRight(), HDL_LWRGT); break; // Unten rechts
    }
    return pH;
}

USHORT SdrObject::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    return 0;
}

SdrHdl* SdrObject::GetPlusHdl(const SdrHdl& rHdl, USHORT nPlNum) const
{
    return NULL;
}

void SdrObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    USHORT nAnz=GetHdlCount();
    for (USHORT i=0; i<nAnz; i++) {
        SdrHdl* pHdl=GetHdl(i);
        if (pHdl!=NULL) {
            rHdlList.AddHdl(pHdl);
        }
    }
}

// Drag

Rectangle SdrObject::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    Rectangle aTmpRect(GetSnapRect());
    Rectangle aRect(aTmpRect);
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    FASTBOOL bEcke=(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
    FASTBOOL bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    FASTBOOL bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    FASTBOOL bLft=(eHdl==HDL_UPLFT || eHdl==HDL_LEFT  || eHdl==HDL_LWLFT);
    FASTBOOL bRgt=(eHdl==HDL_UPRGT || eHdl==HDL_RIGHT || eHdl==HDL_LWRGT);
    FASTBOOL bTop=(eHdl==HDL_UPRGT || eHdl==HDL_UPPER || eHdl==HDL_UPLFT);
    FASTBOOL bBtm=(eHdl==HDL_LWRGT || eHdl==HDL_LOWER || eHdl==HDL_LWLFT);
    if (bLft) aTmpRect.Left()  =aPos.X();
    if (bRgt) aTmpRect.Right() =aPos.X();
    if (bTop) aTmpRect.Top()   =aPos.Y();
    if (bBtm) aTmpRect.Bottom()=aPos.Y();
    if (bOrtho) { // Ortho
        long nWdt0=aRect.Right() -aRect.Left();
        long nHgt0=aRect.Bottom()-aRect.Top();
        long nXMul=aTmpRect.Right() -aTmpRect.Left();
        long nYMul=aTmpRect.Bottom()-aTmpRect.Top();
        long nXDiv=nWdt0;
        long nYDiv=nHgt0;
        FASTBOOL bXNeg=(nXMul<0)!=(nXDiv<0);
        FASTBOOL bYNeg=(nYMul<0)!=(nYDiv<0);
        nXMul=Abs(nXMul);
        nYMul=Abs(nYMul);
        nXDiv=Abs(nXDiv);
        nYDiv=Abs(nYDiv);
        Fraction aXFact(nXMul,nXDiv); // Fractions zum kuerzen
        Fraction aYFact(nYMul,nYDiv); // und zum vergleichen
        nXMul=aXFact.GetNumerator();
        nYMul=aYFact.GetNumerator();
        nXDiv=aXFact.GetDenominator();
        nYDiv=aYFact.GetDenominator();
        if (bEcke) { // Eckpunkthandles
            FASTBOOL bUseX=(aXFact<aYFact) != bBigOrtho;
            if (bUseX) {
                long nNeed=long(BigInt(nHgt0)*BigInt(nXMul)/BigInt(nXDiv));
                if (bYNeg) nNeed=-nNeed;
                if (bTop) aTmpRect.Top()=aTmpRect.Bottom()-nNeed;
                if (bBtm) aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            } else {
                long nNeed=long(BigInt(nWdt0)*BigInt(nYMul)/BigInt(nYDiv));
                if (bXNeg) nNeed=-nNeed;
                if (bLft) aTmpRect.Left()=aTmpRect.Right()-nNeed;
                if (bRgt) aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        } else { // Scheitelpunkthandles
            if ((bLft || bRgt) && nXDiv!=0) {
                long nHgt0=aRect.Bottom()-aRect.Top();
                long nNeed=long(BigInt(nHgt0)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.Top()-=(nNeed-nHgt0)/2;
                aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                long nWdt0=aRect.Right()-aRect.Left();
                long nNeed=long(BigInt(nWdt0)*BigInt(nYMul)/BigInt(nYDiv));
                aTmpRect.Left()-=(nNeed-nWdt0)/2;
                aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        }
    }
    aTmpRect.Justify();
    return aTmpRect;
}

FASTBOOL SdrObject::HasSpecialDrag() const
{
    return FALSE; // noch nicht ganz fertig ...
}

FASTBOOL SdrObject::BegDrag(SdrDragStat& rDrag) const
{
    if (bSizProt) return FALSE; // Groesse geschuetzt
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    if (eHdl==HDL_UPLFT || eHdl==HDL_UPPER || eHdl==HDL_UPRGT ||
        eHdl==HDL_LEFT  ||                    eHdl==HDL_RIGHT ||
        eHdl==HDL_LWLFT || eHdl==HDL_LOWER || eHdl==HDL_LWRGT) return TRUE;
    return FALSE;
}

FASTBOOL SdrObject::MovDrag(SdrDragStat& rDrag) const
{
    return TRUE;
}

FASTBOOL SdrObject::EndDrag(SdrDragStat& rDrag)
{
    Rectangle aNewRect(ImpDragCalcRect(rDrag));
    if (aNewRect!=GetSnapRect()) {
        SetSnapRect(aNewRect);
    }
    return TRUE;
}

void SdrObject::BrkDrag(SdrDragStat& rDrag) const
{
}

XubString SdrObject::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bDragUndoComment, FASTBOOL bCreateComment) const
{
    return String();
}

void SdrObject::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    Rectangle aTmpRect(ImpDragCalcRect(rDrag));
    rXPP.Insert(XPolygon(aTmpRect));
}

// Create
FASTBOOL SdrObject::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    return TRUE;
}

FASTBOOL SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(aOutRect);
    rStat.SetActionRect(aOutRect);
    aOutRect.Justify();
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    return TRUE;
}

FASTBOOL SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aOutRect);
    aOutRect.Justify();
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrObject::BrkCreate(SdrDragStat& rStat)
{
}

FASTBOOL SdrObject::BckCreate(SdrDragStat& rStat)
{
    return FALSE;
}

void SdrObject::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();
    rXPP=XPolyPolygon(XPolygon(aRect1));
}

Pointer SdrObject::GetCreatePointer() const
{
    return Pointer(POINTER_CROSS);
}

// Transformationen
void SdrObject::NbcMove(const Size& rSiz)
{
    MoveRect(aOutRect,rSiz);
    SetRectsDirty();
}

void SdrObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
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
    ResizeRect(aOutRect,rRef,xFact,yFact);
    SetRectsDirty();
}

void SdrObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SetGlueReallyAbsolute(TRUE);
    aOutRect.Move(-rRef.X(),-rRef.Y());
    Rectangle R(aOutRect);
    if (sn==1.0 && cs==0.0) { // 90ø
        aOutRect.Left()  =-R.Bottom();
        aOutRect.Right() =-R.Top();
        aOutRect.Top()   =R.Left();
        aOutRect.Bottom()=R.Right();
    } else if (sn==0.0 && cs==-1.0) { // 180ø
        aOutRect.Left()  =-R.Right();
        aOutRect.Right() =-R.Left();
        aOutRect.Top()   =-R.Bottom();
        aOutRect.Bottom()=-R.Top();
    } else if (sn==-1.0 && cs==0.0) { // 270ø
        aOutRect.Left()  =R.Top();
        aOutRect.Right() =R.Bottom();
        aOutRect.Top()   =-R.Right();
        aOutRect.Bottom()=-R.Left();
    }
    aOutRect.Move(rRef.X(),rRef.Y());
    aOutRect.Justify(); // Sicherheitshalber
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nWink,sn,cs);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    aOutRect.Move(-rRef1.X(),-rRef1.Y());
    Rectangle R(aOutRect);
    long dx=rRef2.X()-rRef1.X();
    long dy=rRef2.Y()-rRef1.Y();
    if (dx==0) {          // Vertikale Achse
        aOutRect.Left() =-R.Right();
        aOutRect.Right()=-R.Left();
    } else if (dy==0) {   // Horizontale Achse
        aOutRect.Top()   =-R.Bottom();
        aOutRect.Bottom()=-R.Top();
    } else if (dx==dy) {  /* 45 Grad Achse \ */
        aOutRect.Left()  =R.Top();
        aOutRect.Right() =R.Bottom();
        aOutRect.Top()   =R.Left();
        aOutRect.Bottom()=R.Right();
    } else if (dx==-dy) { // 45 Grad Achse /
        aOutRect.Left()  =-R.Bottom();
        aOutRect.Right() =-R.Top();
        aOutRect.Top()   =-R.Right();
        aOutRect.Bottom()=-R.Left();
    }
    aOutRect.Move(rRef1.X(),rRef1.Y());
    aOutRect.Justify(); // Sicherheitshalber
    SetRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    SetGlueReallyAbsolute(TRUE);
    NbcShearGluePoints(rRef,nWink,tn,bVShear);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcMove(rSiz);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

void SdrObject::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcResize(rRef,xFact,yFact);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if (nWink!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcRotate(rRef,nWink,sn,cs);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::Mirror(const Point& rRef1, const Point& rRef2)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcMirror(rRef1,rRef2);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    if (nWink!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcShear(rRef,nWink,tn,bVShear);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

void SdrObject::SetRelativePos(const Point& rPnt)
{
    if (rPnt!=GetRelativePos()) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcSetRelativePos(rPnt);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

Point SdrObject::GetRelativePos() const
{
    return GetSnapRect().TopLeft()-aAnchor;
}

void SdrObject::NbcSetAnchorPos(const Point& rPnt)
{
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    aAnchor=rPnt;
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

void SdrObject::SetAnchorPos(const Point& rPnt)
{
    if (rPnt!=aAnchor) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        NbcSetAnchorPos(rPnt);
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

const Point& SdrObject::GetAnchorPos() const
{
    return aAnchor;
}

void SdrObject::RecalcSnapRect()
{
}

const Rectangle& SdrObject::GetSnapRect() const
{
    return aOutRect;
}

void SdrObject::NbcSetSnapRect(const Rectangle& rRect)
{
    aOutRect=rRect;
}

const Rectangle& SdrObject::GetLogicRect() const
{
    return GetSnapRect();
}

void SdrObject::NbcSetLogicRect(const Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}

void SdrObject::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetSnapRect(rRect);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetLogicRect(rRect);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

long SdrObject::GetRotateAngle() const
{
    return 0;
}

long SdrObject::GetShearAngle(FASTBOOL bVertical) const
{
    return 0;
}

USHORT SdrObject::GetSnapPointCount() const
{
    return GetPointCount();
}

Point  SdrObject::GetSnapPoint(USHORT i) const
{
    return GetPoint(i);
}

FASTBOOL SdrObject::IsPolyObj() const
{
    return FALSE;
}

USHORT SdrObject::GetPointCount() const
{
    return 0;
}

const Point& SdrObject::GetPoint(USHORT i) const
{
    return *((Point*)NULL);
}

void SdrObject::SetPoint(const Point& rPnt, USHORT i)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetPoint(rPnt,i);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::NbcSetPoint(const Point& rPnt, USHORT i)
{
}

USHORT SdrObject::InsPoint(const Point& rPos, FASTBOOL bNewObj, FASTBOOL& rInsNextAfter)
{
    USHORT nRet=0xFFFF;
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    nRet=NbcInsPoint(rPos,bNewObj,FALSE,rInsNextAfter);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return nRet;
}

USHORT SdrObject::NbcInsPoint(const Point& rPos, FASTBOOL bNewObj, FASTBOOL bHideHim, FASTBOOL& rInsNextAfter)
{
    return 0xFFFF;
}

USHORT SdrObject::InsPoint(USHORT i, const Point& rPos, FASTBOOL bInsAfter, FASTBOOL bNewObj)
{
    USHORT nRet=0xFFFF;
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    nRet=NbcInsPoint(i,rPos,bInsAfter,bNewObj,FALSE);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return nRet;
}

USHORT SdrObject::NbcInsPoint(USHORT i, const Point& rPos, FASTBOOL bInsAfter, FASTBOOL bNewObj, FASTBOOL bHideHim)
{
    return 0xFFFF;
}

FASTBOOL SdrObject::DelPoint(USHORT i)
{
    FASTBOOL bRet=TRUE;
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    bRet=NbcDelPoint(i);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return bRet;
}

FASTBOOL SdrObject::NbcDelPoint(USHORT i)
{
    return TRUE;
}

SdrObject* SdrObject::RipPoint(USHORT i, USHORT& rNewPt0Index)
{
    SdrObject* pRet=NULL;
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    pRet=NbcRipPoint(i,rNewPt0Index);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    return pRet;
}

SdrObject* SdrObject::NbcRipPoint(USHORT i, USHORT& rNewPt0Index)
{
    return NULL;
}

void SdrObject::Shut()
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcShut();
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::NbcShut()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrObject::HasTextEdit() const
{
    return FALSE;
}

SdrObject* SdrObject::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return CheckHit(rPnt,nTol,pVisiLayer);
}

FASTBOOL SdrObject::BegTextEdit(SdrOutliner& rOutl)
{
    return FALSE;
}

void SdrObject::EndTextEdit(SdrOutliner& rOutl)
{
}

void SdrObject::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetOutlinerParaObject(pTextObject);
    SetChanged();
    SendRepaintBroadcast();
    if (GetBoundRect()!=aBoundRect0) {
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
}

OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
{
    return NULL;
}

void SdrObject::NbcReformatText()
{
}

void SdrObject::ReformatText()
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcReformatText();
    SetChanged();
    SendRepaintBroadcast();
    if (GetBoundRect()!=aBoundRect0) {
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
{
}

void SdrObject::RestartAnimation(SdrPageView* pPageView) const
{
}

#define Imp2ndKennung (0x434F4D43)
SdrObjUserData* SdrObject::ImpGetMacroUserData() const
{
    SdrObjUserData* pData=NULL;
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=GetUserData(nNum);
        if (!pData->HasMacro(this)) pData=NULL;
    }
    return pData;
}

FASTBOOL SdrObject::HasMacro() const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    return pData!=NULL ? pData->HasMacro(this) : FALSE;
}

SdrObject* SdrObject::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->CheckMacroHit(rRec,this);
    }
    return CheckHit(rRec.aPos,rRec.nTol,rRec.pVisiLayer);
}

Pointer SdrObject::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPointer(rRec,this);
    }
    return Pointer(POINTER_REFHAND);
}

void SdrObject::PaintMacro(ExtOutputDevice& rXOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        pData->PaintMacro(rXOut,rDirtyRect,rRec,this);
    } else {
        Color aBlackColor( COL_BLACK );
        Color aTranspColor( COL_TRANSPARENT );
        rXOut.OverrideLineColor( aBlackColor );
        rXOut.OverrideFillColor( aTranspColor );
        RasterOp eRop0=rXOut.GetRasterOp();
        rXOut.SetRasterOp(ROP_INVERT);
        XPolyPolygon aXPP;
        TakeXorPoly(aXPP,TRUE);
        USHORT nAnz=aXPP.Count();
        for (USHORT nNum=0; nNum<nAnz; nNum++) {
            rXOut.DrawXPolyLine(aXPP[nNum]);
        }
        rXOut.SetRasterOp(eRop0);
    }
}

FASTBOOL SdrObject::DoMacro(const SdrObjMacroHitRec& rRec)
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->DoMacro(rRec,this);
    }
    return FALSE;
}

XubString SdrObject::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPopupComment(rRec,this);
    }
    return String();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjGeoData* SdrObject::NewGeoData() const
{
    return new SdrObjGeoData;
}

void SdrObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rGeo.aBoundRect    =GetBoundRect();
    rGeo.aAnchor       =aAnchor       ;
    rGeo.bMovProt      =bMovProt      ;
    rGeo.bSizProt      =bSizProt      ;
    rGeo.bNoPrint      =bNoPrint      ;
    rGeo.bClosedObj    =bClosedObj    ;
    rGeo.nLayerId      =nLayerId      ;

    // Benutzerdefinierte Klebepunkte
    if (pPlusData!=NULL && pPlusData->pGluePoints!=NULL) {
        if (rGeo.pGPL!=NULL) {
            *rGeo.pGPL=*pPlusData->pGluePoints;
        } else {
            rGeo.pGPL=new SdrGluePointList(*pPlusData->pGluePoints);
        }
    } else {
        if (rGeo.pGPL!=NULL) {
            delete rGeo.pGPL;
            rGeo.pGPL=NULL;
        }
    }
}

void SdrObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    SetRectsDirty();
    aOutRect      =rGeo.aBoundRect    ;
    aAnchor       =rGeo.aAnchor       ;
    bMovProt      =rGeo.bMovProt      ;
    bSizProt      =rGeo.bSizProt      ;
    bNoPrint      =rGeo.bNoPrint      ;
    bClosedObj    =rGeo.bClosedObj    ;
    nLayerId      =rGeo.nLayerId      ;

    // Benutzerdefinierte Klebepunkte
    if (rGeo.pGPL!=NULL) {
        ImpForcePlusData();
        if (pPlusData->pGluePoints!=NULL) {
            *pPlusData->pGluePoints=*rGeo.pGPL;
        } else {
            pPlusData->pGluePoints=new SdrGluePointList(*rGeo.pGPL);
        }
    } else {
        if (pPlusData!=NULL && pPlusData->pGluePoints!=NULL) {
            delete pPlusData->pGluePoints;
            pPlusData->pGluePoints=NULL;
        }
    }
}

SdrObjGeoData* SdrObject::GetGeoData() const
{
    SdrObjGeoData* pGeo=NewGeoData();
    SaveGeoData(*pGeo);
    return pGeo;
}

void SdrObject::SetGeoData(const SdrObjGeoData& rGeo)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    RestGeoData(rGeo);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

SfxItemSet* SdrObject::mpEmptyItemSet = 0L;
const SfxItemSet& SdrObject::GetItemSet() const
{
    if(!mpEmptyItemSet)
        mpEmptyItemSet = ((SdrObject*)this)->CreateNewItemSet((SfxItemPool&)(*GetItemPool()));
    DBG_ASSERT(mpEmptyItemSet, "Could not create an SfxItemSet(!)");
    DBG_ASSERT(FALSE,"SdrObject::GetItemSet() should never be called, SdrObject has no Items");
    return *mpEmptyItemSet;
}

SfxItemSet* SdrObject::CreateNewItemSet(SfxItemPool& rPool)
{
    // Basic implementation; Basic object has NO attributes
    DBG_ASSERT(FALSE,"SdrObject::CreateNewItemSet() should never be called, SdrObject has no Items");
    return new SfxItemSet(rPool, SDRATTR_START, SDRATTR_END);
}

void SdrObject::SetItem( const SfxPoolItem& rItem )
{
    sal_uInt16 nWhichID(rItem.Which());

    if(AllowItemChange(nWhichID, &rItem))
    {
        ItemChange(nWhichID, &rItem);
        PostItemChange(nWhichID);
        ItemSetChanged();
    }
}

void SdrObject::ClearItem( const sal_uInt16 nWhich )
{
    if(AllowItemChange(nWhich))
    {
        ItemChange(nWhich);
        PostItemChange(nWhich);
        ItemSetChanged();
    }
}

void SdrObject::SetItemSet( const SfxItemSet& rSet )
{
    SfxWhichIter aWhichIter(rSet);
    sal_uInt16 nWhich(aWhichIter.FirstWhich());
    const SfxPoolItem *pPoolItem;
    std::vector< sal_uInt16 > aPostItemChangeList;
    BOOL bDidChange(FALSE);

    while(nWhich)
    {
        if(SFX_ITEM_SET == rSet.GetItemState(nWhich, FALSE, &pPoolItem))
        {
            if(AllowItemChange(nWhich, pPoolItem))
            {
                bDidChange = TRUE;
                ItemChange(nWhich, pPoolItem);
                aPostItemChangeList.push_back( nWhich );
            }
        }
        nWhich = aWhichIter.NextWhich();
    }

    if(bDidChange)
    {
        std::vector< sal_uInt16 >::iterator aIter = aPostItemChangeList.begin();
        const std::vector< sal_uInt16 >::iterator aEnd = aPostItemChangeList.end();
        while( aIter != aEnd )
        {
            PostItemChange((*aIter));
            aIter++;
        }

        ItemSetChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet was changed, maybe user wants to react

void SdrObject::ItemSetChanged()
{
}

void SdrObject::BroadcastItemChange(const SdrBroadcastItemChange& rChange)
{
    sal_uInt32 nCount(rChange.GetRectangleCount());

    for(sal_uInt32 a(0); a < nCount; a++)
        SendRepaintBroadcast(rChange.GetRectangle(a));

    if(ISA(SdrObjGroup))
    {
        SdrObjListIter aIter(*((SdrObjGroup*)this), IM_DEEPNOGROUPS);
        while(aIter.IsMore())
        {
            SdrObject* pObj = aIter.Next();
            SendRepaintBroadcast(pObj->GetBoundRect());
        }
    }
    else
        SendRepaintBroadcast(GetBoundRect());

    for(a = 0; a < nCount; a++)
        SendUserCall(SDRUSERCALL_CHGATTR, rChange.GetRectangle(a));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// syntactical sugar for ItemSet accesses

void SdrObject::SetItemAndBroadcast(const SfxPoolItem& rItem)
{
    SdrBroadcastItemChange aC(*this);
    SetItem(rItem);
    BroadcastItemChange(aC);
}

void SdrObject::ClearItemAndBroadcast(const sal_uInt16 nWhich)
{
    SdrBroadcastItemChange aC(*this);
    ClearItem(nWhich);
    BroadcastItemChange(aC);
}

void SdrObject::SetItemSetAndBroadcast(const SfxItemSet& rSet)
{
    SdrBroadcastItemChange aC(*this);
    SetItemSet(rSet);
    BroadcastItemChange(aC);
}

const SfxPoolItem& SdrObject::GetItem(const sal_uInt16 nWhich) const
{
    return GetItemSet().Get(nWhich);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// private support routines for ItemSet access

BOOL SdrObject::AllowItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem) const
{
    return TRUE;
}

void SdrObject::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
{
    DBG_ASSERT(FALSE,"SdrObject::ItemChange() should never be called, SdrObject has no Items");
}

void SdrObject::PostItemChange(const sal_uInt16 nWhich)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// pre- and postprocessing for objects for saving

void SdrObject::PreSave()
{
}

void SdrObject::PostSave()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObject::ApplyNotPersistAttr(const SfxItemSet& rAttr)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcApplyNotPersistAttr(rAttr);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::NbcApplyNotPersistAttr(const SfxItemSet& rAttr)
{
    const Rectangle& rSnap=GetSnapRect();
    const Rectangle& rLogic=GetLogicRect();
    Point aRef1(rSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.X()=((const SdrTransformRef1XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.Y()=((const SdrTransformRef1YItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.X()=((const SdrTransformRef2XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.Y()=((const SdrTransformRef2YItem*)pPoolItem)->GetValue();
    }

    Rectangle aNewSnap(rSnap);
    if (rAttr.GetItemState(SDRATTR_MOVEX,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }
    if (rAttr.GetItemState(SDRATTR_MOVEY,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONX,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONY,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEWIDTH,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOneSizeWidthItem*)pPoolItem)->GetValue();
        aNewSnap.Right()=aNewSnap.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOneSizeHeightItem*)pPoolItem)->GetValue();
        aNewSnap.Bottom()=aNewSnap.Top()+n;
    }
    if (aNewSnap!=rSnap) {
        if (aNewSnap.GetSize()==rSnap.GetSize()) {
            NbcMove(Size(aNewSnap.Left()-rSnap.Left(),aNewSnap.Top()-rSnap.Top()));
        } else {
            NbcSetSnapRect(aNewSnap);
        }
    }

    if (rAttr.GetItemState(SDRATTR_SHEARANGLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrShearAngleItem*)pPoolItem)->GetValue();
        n-=GetShearAngle();
        if (n!=0) {
            double nTan=tan(n*nPi180);
            NbcShear(aRef1,n,nTan,FALSE);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEANGLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateAngleItem*)pPoolItem)->GetValue();
        n-=GetRotateAngle();
        if (n!=0) {
            double nSin=sin(n*nPi180);
            double nCos=cos(n*nPi180);
            NbcRotate(aRef1,n,nSin,nCos);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateOneItem*)pPoolItem)->GetValue();
        double nSin=sin(n*nPi180);
        double nCos=cos(n*nPi180);
        NbcRotate(aRef1,n,nSin,nCos);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrHorzShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,FALSE);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrVertShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,TRUE);
    }

    if (rAttr.GetItemState(SDRATTR_OBJMOVEPROTECT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        FASTBOOL b=((const SdrObjMoveProtectItem*)pPoolItem)->GetValue();
        SetMoveProtect(b);
    }
    if (rAttr.GetItemState(SDRATTR_OBJSIZEPROTECT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        FASTBOOL b=((const SdrObjSizeProtectItem*)pPoolItem)->GetValue();
        SetResizeProtect(b);
    }
    if (rAttr.GetItemState(SDRATTR_OBJPRINTABLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        FASTBOOL b=((const SdrObjPrintableItem*)pPoolItem)->GetValue();
        SetPrintable(b);
    }

    SdrLayerID nLayer=SDRLAYER_NOTFOUND;
    if (rAttr.GetItemState(SDRATTR_LAYERID,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nLayer=((const SdrLayerIdItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,TRUE,&pPoolItem)==SFX_ITEM_SET && pModel!=NULL) {
        XubString aLayerName=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        const SdrLayerAdmin* pLayAd=pPage!=NULL ? &pPage->GetLayerAdmin() : pModel!=NULL ? &pModel->GetLayerAdmin() : NULL;
        if (pLayAd!=NULL) {
            const SdrLayer* pLayer=pLayAd->GetLayer(aLayerName, TRUE);
            if (pLayer!=NULL) {
                nLayer=pLayer->GetID();
            }
        }

    }
    if (nLayer!=SDRLAYER_NOTFOUND) {
        NbcSetLayer(nLayer);
    }

    if (rAttr.GetItemState(SDRATTR_OBJECTNAME,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        XubString aName=((const SdrObjectNameItem*)pPoolItem)->GetValue();
        SetName(aName);
    }
    Rectangle aNewLogic(rLogic);
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEWIDTH,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeWidthItem*)pPoolItem)->GetValue();
        aNewLogic.Right()=aNewLogic.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeHeightItem*)pPoolItem)->GetValue();
        aNewLogic.Bottom()=aNewLogic.Top()+n;
    }
    if (aNewLogic!=rLogic) {
        NbcSetLogicRect(aNewLogic);
    }
    Fraction aResizeX(1,1);
    Fraction aResizeY(1,1);
    if (rAttr.GetItemState(SDRATTR_RESIZEXONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aResizeX*=((const SdrResizeXOneItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aResizeY*=((const SdrResizeYOneItem*)pPoolItem)->GetValue();
    }
    if (aResizeX!=Fraction(1,1) || aResizeY!=Fraction(1,1)) {
        NbcResize(aRef1,aResizeX,aResizeY);
    }
}

void lcl_SetItem(SfxItemSet& rAttr, FASTBOOL bMerge, const SfxPoolItem& rItem)
{
    if (bMerge) rAttr.MergeValue(rItem,TRUE);
    else rAttr.Put(rItem);
}

void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr, FASTBOOL bMerge) const
{
    const Rectangle& rSnap=GetSnapRect();
    const Rectangle& rLogic=GetLogicRect();
    lcl_SetItem(rAttr,bMerge,SdrObjMoveProtectItem(IsMoveProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjSizeProtectItem(IsResizeProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjPrintableItem(IsPrintable()));
    lcl_SetItem(rAttr,bMerge,SdrRotateAngleItem(GetRotateAngle()));
    lcl_SetItem(rAttr,bMerge,SdrShearAngleItem(GetShearAngle()));
    lcl_SetItem(rAttr,bMerge,SdrOneSizeWidthItem(rSnap.GetWidth()-1));
    lcl_SetItem(rAttr,bMerge,SdrOneSizeHeightItem(rSnap.GetHeight()-1));
    lcl_SetItem(rAttr,bMerge,SdrOnePositionXItem(rSnap.Left()));
    lcl_SetItem(rAttr,bMerge,SdrOnePositionYItem(rSnap.Top()));
    if (rLogic.GetWidth()!=rSnap.GetWidth()) {
        lcl_SetItem(rAttr,bMerge,SdrLogicSizeWidthItem(rLogic.GetWidth()-1));
    }
    if (rLogic.GetHeight()!=rSnap.GetHeight()) {
        lcl_SetItem(rAttr,bMerge,SdrLogicSizeHeightItem(rLogic.GetHeight()-1));
    }
    if (HasSetName()) {
        XubString aName(GetName());

        if(aName.Len())
        {
            lcl_SetItem(rAttr, bMerge, SdrObjectNameItem(aName));
        }
    }

    lcl_SetItem(rAttr,bMerge,SdrLayerIdItem(nLayerId));
    const SdrLayerAdmin* pLayAd=pPage!=NULL ? &pPage->GetLayerAdmin() : pModel!=NULL ? &pModel->GetLayerAdmin() : NULL;
    if (pLayAd!=NULL) {
        const SdrLayer* pLayer=pLayAd->GetLayerPerID(nLayerId);
        if (pLayer!=NULL) {
            lcl_SetItem(rAttr,bMerge,SdrLayerNameItem(pLayer->GetName()));
        }
    }
    Point aRef1(rSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;
    lcl_SetItem(rAttr,bMerge,SdrTransformRef1XItem(aRef1.X()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef1YItem(aRef1.Y()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef2XItem(aRef2.X()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef2YItem(aRef2.Y()));
}

SfxStyleSheet* SdrObject::GetStyleSheet() const
{
    // Das hier ist ein Hack:
    return NULL;
}

void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
}

void SdrObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
}

// Das Broadcasting beim Setzen der Attribute wird vom AttrObj gemanagt
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrObject::IsNode() const
{
    return TRUE;
}

SdrGluePoint SdrObject::GetVertexGluePoint(USHORT nPosNum) const
{
    Rectangle aR(GetBoundRect());
    Point aPt;
    switch (nPosNum) {
        case 0 : aPt=aR.TopCenter();    break;
        case 1 : aPt=aR.RightCenter();  break;
        case 2 : aPt=aR.BottomCenter(); break;
        case 3 : aPt=aR.LeftCenter();   break;
    }
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(FALSE);
    return aGP;
}

SdrGluePoint SdrObject::GetCornerGluePoint(USHORT nPosNum) const
{
    Rectangle aR(GetBoundRect());
    Point aPt;
    switch (nPosNum) {
        case 0 : aPt=aR.TopLeft();     break;
        case 1 : aPt=aR.TopRight();    break;
        case 2 : aPt=aR.BottomRight(); break;
        case 3 : aPt=aR.BottomLeft();  break;
    }
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(FALSE);
    return aGP;
}

const SdrGluePointList* SdrObject::GetGluePointList() const
{
    if (pPlusData!=NULL) return pPlusData->pGluePoints;
    return NULL;
}

SdrGluePointList* SdrObject::GetGluePointList()
{
    if (pPlusData!=NULL) return pPlusData->pGluePoints;
    return NULL;
}

SdrGluePointList* SdrObject::ForceGluePointList()
{
    ImpForcePlusData();
    if (pPlusData->pGluePoints==NULL) {
        pPlusData->pGluePoints=new SdrGluePointList;
    }
    return pPlusData->pGluePoints;
}

void SdrObject::SetGlueReallyAbsolute(FASTBOOL bOn)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->SetReallyAbsolute(bOn,*this);
    }
}

void SdrObject::NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Rotate(rRef,nWink,sn,cs,this);
    }
}

void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Mirror(rRef1,rRef2,this);
    }
}

void SdrObject::NbcShearGluePoints(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Shear(rRef,nWink,tn,bVShear,this);
    }
}

FASTBOOL SdrObject::IsEdge() const
{
    return FALSE;
}

void SdrObject::ToggleEdgeXor(const SdrDragStat& rDrag, ExtOutputDevice& rXOut, FASTBOOL bTail1, FASTBOOL bTail2, FASTBOOL bDetail) const
{
    Color aBlackColor( COL_BLACK );
    Color aTranspColor( COL_TRANSPARENT );
    rXOut.OverrideLineColor( aBlackColor );
    rXOut.OverrideFillColor( aTranspColor );
    RasterOp eRop0=rXOut.GetRasterOp();
    rXOut.SetRasterOp(ROP_INVERT);
    NspToggleEdgeXor(rDrag,rXOut,bTail1,bTail2,bDetail);
    rXOut.SetRasterOp(eRop0);
}

void SdrObject::NspToggleEdgeXor(const SdrDragStat& rDrag, ExtOutputDevice& rXOut, FASTBOOL bTail1, FASTBOOL bTail2, FASTBOOL bDetail) const
{
}

void SdrObject::ConnectToNode(FASTBOOL bTail1, SdrObject* pObj)
{
}

void SdrObject::DisconnectFromNode(FASTBOOL bTail1)
{
}

SdrObject* SdrObject::GetConnectedNode(FASTBOOL bTail1) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::ImpConvertToContourObj(SdrObject* pRet, BOOL bForceLineDash) const
{
    BOOL bNoChange(TRUE);

    if(pRet->LineGeometryUsageIsNecessary())
    {
        // Polygon aus Bezierkurve interpolieren
        VirtualDevice aVDev;
        MapMode aMap = aVDev.GetMapMode();
        aMap.SetMapUnit(pModel->GetScaleUnit());
        aMap.SetScaleX(pModel->GetScaleFraction());
        aMap.SetScaleY(pModel->GetScaleFraction());
        aVDev.SetMapMode(aMap);

        PolyPolygon3D aPolyPoly3D;
        PolyPolygon3D aLinePoly3D;
        pRet->CreateLinePoly(aPolyPoly3D, aLinePoly3D, aVDev, FALSE, FALSE);

        //  || aLinePoly3D.Count() removed; the conversion is ONLY
        // useful when new closed filled polygons are created
        if(aPolyPoly3D.Count() || (bForceLineDash && aLinePoly3D.Count()))
        {
            SfxItemSet aSet(pRet->GetItemSet());
            XFillStyle eOldFillStyle = ((const XFillStyleItem&)(aSet.Get(XATTR_FILLSTYLE))).GetValue();
            SdrPathObj* aLinePolygonPart = NULL;
            SdrPathObj* aLineLinePart = NULL;
            BOOL bBuildGroup(FALSE);

            aPolyPoly3D.Merge(TRUE);

            if(aPolyPoly3D.Count())
            {
                aLinePolygonPart = new SdrPathObj(OBJ_PATHFILL, aPolyPoly3D.GetXPolyPolygon());
                aLinePolygonPart->SetModel(pRet->GetModel());

                aSet.Put(XLineWidthItem(0L));
                Color aColorLine = ((const XLineColorItem&)(aSet.Get(XATTR_LINECOLOR))).GetValue();
                UINT16 nTransLine = ((const XLineTransparenceItem&)(aSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
                aSet.Put(XFillColorItem(XubString(), aColorLine));
                aSet.Put(XFillStyleItem(XFILL_SOLID));
                aSet.Put(XLineStyleItem(XLINE_NONE));
                aSet.Put(XFillTransparenceItem(nTransLine));

                aLinePolygonPart->SetItemSet(aSet);
            }

            if(aLinePoly3D.Count())
            {
                aLineLinePart = new SdrPathObj(OBJ_PATHFILL, aLinePoly3D.GetXPolyPolygon());
                aLineLinePart->SetModel(pRet->GetModel());

                aSet.Put(XLineWidthItem(0L));
                aSet.Put(XFillStyleItem(XFILL_NONE));
                aSet.Put(XLineStyleItem(XLINE_SOLID));

                aLineLinePart->SetItemSet(aSet);

                if(aLinePolygonPart)
                    bBuildGroup = TRUE;
            }

            if(!bBuildGroup)
            {
                SdrPathObj* pPath = PTR_CAST(SdrPathObj, pRet);
                if(pPath && pPath->IsClosed())
                {
                    if(eOldFillStyle != XFILL_NONE)
                        bBuildGroup = TRUE;
                }
            }

            if(bBuildGroup)
            {
                SdrObject* pGroup = new SdrObjGroup;
                pGroup->SetModel(pRet->GetModel());

                aSet.ClearItem();

                aSet.Put(pRet->GetItemSet());

                aSet.Put(XLineStyleItem(XLINE_NONE));
                aSet.Put(XLineWidthItem(0L));

                SdrObject* pClone = pRet->Clone();
                pClone->SetModel(pRet->GetModel());

                pClone->SetItemSet(aSet);

                pGroup->GetSubList()->NbcInsertObject( pClone );

                if(aLinePolygonPart)
                    pGroup->GetSubList()->NbcInsertObject(aLinePolygonPart);

                if(aLineLinePart)
                    pGroup->GetSubList()->NbcInsertObject(aLineLinePart);

                pRet = pGroup;
            }
            else
            {
                if(aLinePolygonPart)
                    pRet = aLinePolygonPart;
                else if(aLineLinePart)
                    pRet = aLineLinePart;
            }

            bNoChange = FALSE;
        }
    }

    if(bNoChange)
    {
        SdrObject* pClone = pRet->Clone();
        pClone->SetModel(pRet->GetModel());
        pRet = pClone;
    }

    return pRet;
}

// convert this path object to contour object, even when it is a group
SdrObject* SdrObject::ConvertToContourObj(SdrObject* pRet, BOOL bForceLineDash) const
{
    if(pRet->ISA(SdrObjGroup))
    {
        SdrObjList* pObjList = pRet->GetSubList();
        SdrObject* pGroup = new SdrObjGroup;
        pGroup->SetModel(pRet->GetModel());

        for(UINT32 a=0;a<pObjList->GetObjCount();a++)
        {
            SdrObject* pIterObj = pObjList->GetObj(a);
            pGroup->GetSubList()->NbcInsertObject(ConvertToContourObj(pIterObj, bForceLineDash));
        }

        pRet = pGroup;
    }
    else
    {
        pRet = ImpConvertToContourObj(pRet, bForceLineDash);
    }

    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::ConvertToPolyObj(BOOL bBezier, BOOL bLineToArea) const
{
    SdrObject* pRet = DoConvertToPolyObj(bBezier);

    if(pRet && bLineToArea)
    {
        SdrObject* pNewRet = ConvertToContourObj(pRet);
        delete pRet;
        pRet = pNewRet;
    }

    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::DoConvertToPolyObj(BOOL bBezier) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Streams

void SdrObject::AfterRead()
{
    USHORT nAnz=GetUserDataCount();
    for (USHORT i=0; i<nAnz; i++) {
        GetUserData(i)->AfterRead();
    }
}

void SdrObject::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrObject");
#endif
    rIn>>aOutRect;
    rIn>>nLayerId;
    rIn>>aAnchor;
    BOOL bTemp;
    rIn>>bTemp; bMovProt=bTemp;
    rIn>>bTemp; bSizProt=bTemp;
    rIn>>bTemp; bNoPrint=bTemp;
    rIn>>bTemp; bMarkProt=bTemp; // war ganz frueher mal bTextToContour
    rIn>>bTemp; bEmptyPresObj=bTemp;
    if (rHead.GetVersion()>=4) {
        rIn>>bTemp; bNotVisibleAsMaster=bTemp;
    }
    if (rHead.GetVersion()>=11) { // ab V11 sind Connectors in DownCompat gefasst (mit Flag davor)
        rIn>>bTemp;
        if (bTemp) {
            SdrDownCompat aGluePointsCompat(rIn,STREAM_READ);
#ifdef DBG_UTIL
            aGluePointsCompat.SetID("SdrObject(Klebepunkte)");
#endif
            if (aGluePointsCompat.GetBytesLeft()!=0) {
                ImpForcePlusData();
                if (pPlusData->pGluePoints==NULL) pPlusData->pGluePoints=new SdrGluePointList;
                rIn>>*pPlusData->pGluePoints;
            }
        }
    } else {
        Polygon aTmpPoly;
        rIn>>aTmpPoly; // aConnectors; ehemals Konnektoren
    }

    if (pPlusData!=NULL && pPlusData->pUserDataList!=NULL) {
        delete pPlusData->pUserDataList;
        pPlusData->pUserDataList=NULL;
    }
    FASTBOOL bReadUserDataList=FALSE;
    SdrDownCompat* pUserDataListCompat=NULL;
    if (rHead.GetVersion()>=11) { // ab V11 ist die UserDataList in DownCompat gefasst (mit Flag davor)
        rIn>>bTemp;
        bReadUserDataList=bTemp;
        if (bTemp) {
            pUserDataListCompat=new SdrDownCompat(rIn,STREAM_READ); // Record fuer UserDataList oeffnen
#ifdef DBG_UTIL
            pUserDataListCompat->SetID("SdrObject(UserDataList)");
#endif
        }
    } else {
        bReadUserDataList=TRUE;
    }
    if (bReadUserDataList) {
        USHORT nUserDataAnz;
        rIn>>nUserDataAnz;
        if (nUserDataAnz!=0) {
            ImpForcePlusData();
            pPlusData->pUserDataList=new SdrObjUserDataList;
            for (USHORT i=0; i<nUserDataAnz; i++) {
                SdrDownCompat* pUserDataCompat=NULL;
                if (rHead.GetVersion()>=11) { // ab V11 sind UserData in DownCompat gefasst
                    //SdrDownCompat aUserDataCompat(rIn,STREAM_READ); // Record fuer UserData oeffnen (seit V11)
                    pUserDataCompat=new SdrDownCompat(rIn,STREAM_READ); // Record fuer UserData oeffnen (seit V11)
#ifdef DBG_UTIL
                    pUserDataCompat->SetID("SdrObject(UserData)");
#endif
                }
                UINT32 nInvent;
                UINT16 nIdent;
                rIn>>nInvent;
                rIn>>nIdent;
                SdrObjUserData* pData=SdrObjFactory::MakeNewObjUserData(nInvent,nIdent,this);
                if (pData!=NULL)
                {
                    pData->ReadData(rIn);
                    pPlusData->pUserDataList->InsertUserData(pData);
                } else {
                    // Wenn UserDataFactory nicht gesetzt ist, kann auch keiner
                    // etwas mit diesen Daten anfangen; durch Compat werden sie
                    // eh ueberlesen, daher ist diese Assertion überflüssig (KA)
                    // DBG_ERROR("SdrObject::ReadData(): ObjFactory kann UserData nicht erzeugen");
                }
                if (pUserDataCompat!=NULL) { // Aha, UserData war eingepackt. Record nun schliessen
                    delete pUserDataCompat;
                    pUserDataCompat=NULL;
                }
            }
        }
        if (pUserDataListCompat!=NULL) { // Aha, UserDataList war eingepackt. Record nun schliessen
            delete pUserDataListCompat;
            pUserDataListCompat=NULL;
        }
    }
}

void SdrObject::WriteData(SvStream& rOut) const
{
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrObject");
#endif
    rOut<<GetBoundRect();
    rOut<<nLayerId;
    rOut<<aAnchor;
    BOOL bTemp;
    bTemp=bMovProt;       rOut<<bTemp;
    bTemp=bSizProt;       rOut<<bTemp;
    bTemp=bNoPrint;       rOut<<bTemp;
    bTemp=bMarkProt;      rOut<<bTemp;
    bTemp=bEmptyPresObj;  rOut<<bTemp;
    bTemp=bNotVisibleAsMaster; rOut<<bTemp;

    // Konnektoren
    bTemp=pPlusData!=NULL && pPlusData->pGluePoints!=NULL && pPlusData->pGluePoints->GetCount()!=0;
    rOut<<bTemp; // Flag fuer GluePointList vorhanden
    if (bTemp) {
        SdrDownCompat aConnectorsCompat(rOut,STREAM_WRITE); // ab V11 Konnektoren einpacken
#ifdef DBG_UTIL
        aConnectorsCompat.SetID("SdrObject(Klebepunkte)");
#endif
        rOut<<*pPlusData->pGluePoints;
    }

    // UserData
    USHORT nUserDataAnz=GetUserDataCount();
    bTemp=nUserDataAnz!=0;
    rOut<<bTemp;
    if (bTemp) {
        SdrDownCompat aUserDataListCompat(rOut,STREAM_WRITE); // Record fuer UserDataList oeffnen (seit V11)
#ifdef DBG_UTIL
        aUserDataListCompat.SetID("SdrObject(UserDataList)");
#endif
        rOut<<nUserDataAnz;
        for (USHORT i=0; i<nUserDataAnz; i++) {
            SdrDownCompat aUserDataCompat(rOut,STREAM_WRITE); // Record fuer UserData oeffnen (seit V11)
#ifdef DBG_UTIL
            aUserDataCompat.SetID("SdrObject(UserData)");
#endif
            pPlusData->pUserDataList->GetUserData(i)->WriteData(rOut);
        }
    }

}

SvStream& operator>>(SvStream& rIn, SdrObject& rObj)
{
    DBG_ASSERT(!rObj.IsNotPersistent(),"operator>>(SdrObject): Ein nicht persistentes Zeichenobjekts wird gestreamt");
    SdrObjIOHeader aHead(rIn,STREAM_READ,&rObj);
    rObj.ReadData(aHead,rIn);
    return rIn;
}

SvStream& operator<<(SvStream& rOut, const SdrObject& rObj)
{
    DBG_ASSERT(!rObj.IsNotPersistent(),"operator<<(SdrObject): Ein nicht persistentes Zeichenobjekts wird gestreamt");
    SdrObjIOHeader aHead(rOut,STREAM_WRITE,&rObj);
    rObj.WriteData(rOut);
    return rOut;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObject::SetInserted(FASTBOOL bIns)
{
    if (bIns!=bInserted) {
        bInserted=bIns;
        Rectangle aBoundRect0(GetBoundRect());
        if (bIns) SendUserCall(SDRUSERCALL_INSERTED,aBoundRect0);
        else SendUserCall(SDRUSERCALL_REMOVED,aBoundRect0);

        if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) { // #42522#
            SdrHint aHint(*this);
            aHint.SetKind(bIns?HINT_OBJINSERTED:HINT_OBJREMOVED);
            pPlusData->pBroadcast->Broadcast(aHint);
        }
    }
}

void SdrObject::SetMoveProtect(FASTBOOL bProt)
{
    bMovProt=bProt;
    SetChanged();
    if (bInserted && pModel!=NULL) {
        SdrHint aHint(*this);
        aHint.SetNeedRepaint(FALSE);
        pModel->Broadcast(aHint);
    }
}

void SdrObject::SetResizeProtect(FASTBOOL bProt)
{
    bSizProt=bProt;
    SetChanged();
    if (bInserted && pModel!=NULL) {
        SdrHint aHint(*this);
        aHint.SetNeedRepaint(FALSE);
        pModel->Broadcast(aHint);
    }
}

void SdrObject::SetPrintable(FASTBOOL bPrn)
{
    bNoPrint=!bPrn;
    SetChanged();
    if (bInserted && pModel!=NULL) {
        SdrHint aHint(*this);
        aHint.SetNeedRepaint(FALSE);
        pModel->Broadcast(aHint);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

USHORT SdrObject::GetUserDataCount() const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return 0;
    return pPlusData->pUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(USHORT nNum) const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return NULL;
    return pPlusData->pUserDataList->GetUserData(nNum);
}

void SdrObject::InsertUserData(SdrObjUserData* pData, USHORT nPos)
{
    if (pData!=NULL) {
        ImpForcePlusData();
        if (pPlusData->pUserDataList==NULL) pPlusData->pUserDataList=new SdrObjUserDataList;
        pPlusData->pUserDataList->InsertUserData(pData,nPos);
    } else {
        DBG_ERROR("SdrObject::InsertUserData(): pData ist NULL-Pointer");
    }
}

void SdrObject::DeleteUserData(USHORT nNum)
{
    USHORT nAnz=GetUserDataCount();
    if (nNum<nAnz) {
        pPlusData->pUserDataList->DeleteUserData(nNum);
        if (nAnz==1)  {
            delete pPlusData->pUserDataList;
            pPlusData->pUserDataList=NULL;
        }
    } else {
        DBG_ERROR("SdrObject::DeleteUserData(): ungueltiger Index");
    }
}

void SdrObject::SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect)
{
    SdrObjGroup* pGroup = NULL;

    if( pObjList && pObjList->GetListKind() == SDROBJLIST_GROUPOBJ )
        pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();

    if ( pUserCall )
    {
        // UserCall ausfuehren
        pUserCall->Changed( *this, eUserCall, rBoundRect );
    }

    while( pGroup )
    {
        // Gruppe benachrichtigen
        if( pGroup->GetUserCall() )
        {
            SdrUserCallType eChildUserType = SDRUSERCALL_CHILD_CHGATTR;

            switch( eUserCall )
            {
                case SDRUSERCALL_MOVEONLY:
                    eChildUserType = SDRUSERCALL_CHILD_MOVEONLY;
                break;

                case SDRUSERCALL_RESIZE:
                    eChildUserType = SDRUSERCALL_CHILD_RESIZE;
                break;

                case SDRUSERCALL_CHGATTR:
                    eChildUserType = SDRUSERCALL_CHILD_CHGATTR;
                break;

                case SDRUSERCALL_DELETE:
                    eChildUserType = SDRUSERCALL_CHILD_DELETE;
                break;

                case SDRUSERCALL_COPY:
                    eChildUserType = SDRUSERCALL_CHILD_COPY;
                break;

                case SDRUSERCALL_INSERTED:
                    eChildUserType = SDRUSERCALL_CHILD_INSERTED;
                break;

                case SDRUSERCALL_REMOVED:
                    eChildUserType = SDRUSERCALL_CHILD_REMOVED;
                break;
            }

            pGroup->GetUserCall()->Changed( *this, eChildUserType, rBoundRect );
        }

        if( pGroup->GetObjList()                                       &&
            pGroup->GetObjList()->GetListKind() == SDROBJLIST_GROUPOBJ &&
            pGroup != (SdrObjGroup*) pObjList->GetOwnerObj() )
            pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();
        else
            pGroup = NULL;
    }
}

// ItemPool fuer dieses Objekt wechseln
void SdrObject::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool)
{
    // Hier passiert erst was in SdrAttrObj und in SdrObjGroup
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShape( mxUnoShape );

    if( !xShape.is() && pPage )
    {
        uno::Reference< uno::XInterface > xPage( pPage->getUnoPage() );
        if( xPage.is() )
        {
            SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation(xPage);
            if( pDrawPage )
            {
                // create one
                xShape = pDrawPage->_CreateShape( this );
                mxUnoShape = xShape;
            }
        }
    }

    return xShape;
}

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
BOOL SdrObject::TRGetBaseGeometry(Matrix3D& rMat, XPolyPolygon& rPolyPolygon) const
{
    // any kind of SdrObject, just use SnapRect
    Rectangle aRectangle(GetSnapRect());

    // convert to transformation values
    Vector2D aScale((double)aRectangle.GetWidth(), (double)aRectangle.GetHeight());
    Vector2D aTranslate((double)aRectangle.Left(), (double)aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
        aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());

    // force MapUnit to 100th mm
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // postion
                aTranslate.X() = TWIPS_TO_MM(aTranslate.X());
                aTranslate.Y() = TWIPS_TO_MM(aTranslate.Y());

                // size
                aScale.X() = TWIPS_TO_MM(aScale.X());
                aScale.Y() = TWIPS_TO_MM(aScale.Y());

                break;
            }
            default:
            {
                DBG_ERROR("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build matrix
    rMat.Identity();
    if(aScale.X() != 1.0 || aScale.Y() != 1.0)
        rMat.Scale(aScale.X(), aScale.Y());
    if(aTranslate.X() != 0.0 || aTranslate.Y() != 0.0)
        rMat.Translate(aTranslate.X(), aTranslate.Y());

    return FALSE;
}

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
void SdrObject::TRSetBaseGeometry(const Matrix3D& rMat, const XPolyPolygon& rPolyPolygon)
{
    // break up matrix
    Vector2D aScale, aTranslate;
    double fShear, fRotate;
    rMat.DecomposeAndCorrect(aScale, fShear, fRotate, aTranslate);

    // force metric to pool metric
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.X() = MM_TO_TWIPS(aTranslate.X());
                aTranslate.Y() = MM_TO_TWIPS(aTranslate.Y());

                // size
                aScale.X() = MM_TO_TWIPS(aScale.X());
                aScale.Y() = MM_TO_TWIPS(aScale.Y());

                break;
            }
            default:
            {
                DBG_ERROR("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if(GetAnchorPos().X() != 0 || GetAnchorPos().Y() != 0)
        aTranslate -= Vector2D(GetAnchorPos().X(), GetAnchorPos().Y());

    // build BaseRect
    Point aPoint(FRound(aTranslate.X()), FRound(aTranslate.Y()));
    Rectangle aBaseRect(aPoint, Size(FRound(aScale.X()), FRound(aScale.Y())));

    // set BaseRect
    SetSnapRect(aBaseRect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@  @@@@@  @@@@   @@@@  @@@@@@  @@@@  @@@@@  @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@@@  @@@@@@ @@       @@   @@  @@ @@@@@   @@@@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@   @@
//  @@  @@ @@  @@ @@  @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@   @@
//   @@@@  @@@@@   @@@@   @@    @@  @@  @@@@    @@    @@@@  @@  @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObjFactory::MakeNewObject(UINT32 nInvent, UINT16 nIdent, SdrPage* pPage, SdrModel* pModel)
{
    if(pModel == NULL && pPage != NULL)
        pModel = pPage->GetModel();
    SdrObject* pObj = NULL;

    if(nInvent == SdrInventor)
    {
        switch (nIdent)
        {
            case USHORT(OBJ_NONE       ): pObj=new SdrObject;                   break;
            case USHORT(OBJ_GRUP       ): pObj=new SdrObjGroup;                 break;
            case USHORT(OBJ_LINE       ): pObj=new SdrPathObj(OBJ_LINE       ); break;
            case USHORT(OBJ_POLY       ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case USHORT(OBJ_PLIN       ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case USHORT(OBJ_PATHLINE   ): pObj=new SdrPathObj(OBJ_PATHLINE   ); break;
            case USHORT(OBJ_PATHFILL   ): pObj=new SdrPathObj(OBJ_PATHFILL   ); break;
            case USHORT(OBJ_FREELINE   ): pObj=new SdrPathObj(OBJ_FREELINE   ); break;
            case USHORT(OBJ_FREEFILL   ): pObj=new SdrPathObj(OBJ_FREEFILL   ); break;
            case USHORT(OBJ_PATHPOLY   ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case USHORT(OBJ_PATHPLIN   ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case USHORT(OBJ_EDGE       ): pObj=new SdrEdgeObj;                  break;
            case USHORT(OBJ_RECT       ): pObj=new SdrRectObj;                  break;
            case USHORT(OBJ_CIRC       ): pObj=new SdrCircObj(OBJ_CIRC       ); break;
            case USHORT(OBJ_SECT       ): pObj=new SdrCircObj(OBJ_SECT       ); break;
            case USHORT(OBJ_CARC       ): pObj=new SdrCircObj(OBJ_CARC       ); break;
            case USHORT(OBJ_CCUT       ): pObj=new SdrCircObj(OBJ_CCUT       ); break;
            case USHORT(OBJ_TEXT       ): pObj=new SdrRectObj(OBJ_TEXT       ); break;
            case USHORT(OBJ_TEXTEXT    ): pObj=new SdrRectObj(OBJ_TEXTEXT    ); break;
            case USHORT(OBJ_TITLETEXT  ): pObj=new SdrRectObj(OBJ_TITLETEXT  ); break;
            case USHORT(OBJ_OUTLINETEXT): pObj=new SdrRectObj(OBJ_OUTLINETEXT); break;
            case USHORT(OBJ_MEASURE    ): pObj=new SdrMeasureObj;               break;
            case USHORT(OBJ_GRAF       ): pObj=new SdrGrafObj;                  break;
            case USHORT(OBJ_OLE2       ): pObj=new SdrOle2Obj;                  break;
            case USHORT(OBJ_FRAME      ): pObj=new SdrOle2Obj(TRUE);            break;
            case USHORT(OBJ_CAPTION    ): pObj=new SdrCaptionObj;               break;
            case USHORT(OBJ_PAGE       ): pObj=new SdrPageObj;                  break;
            case USHORT(OBJ_UNO        ): pObj=new SdrUnoObj(String());    break;
        }
    }

    if(pObj == NULL)
    {
        SdrObjFactory* pFact=new SdrObjFactory(nInvent,nIdent,pPage,pModel);
        SdrLinkList& rLL=ImpGetUserMakeObjHdl();
        unsigned nAnz=rLL.GetLinkCount();
        unsigned i=0;
        while (i<nAnz && pObj==NULL) {
            rLL.GetLink(i).Call((void*)pFact);
            pObj=pFact->pNewObj;
            i++;
        }
        delete pFact;
    }

    if(pObj == NULL)
    {
        // Na wenn's denn keiner will ...
    }

    if(pObj != NULL)
    {
        if(pPage != NULL)
            pObj->SetPage(pPage);
        else if(pModel != NULL)
            pObj->SetModel(pModel);
    }

    return pObj;
}

SdrObjUserData* SdrObjFactory::MakeNewObjUserData(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj1)
{
    SdrObjUserData* pData=NULL;
    if (nInvent==SdrInventor) {
        switch (nIdent) {
            case USHORT(SDRUSERDATA_OBJGROUPLINK): pData=new ImpSdrObjGroupLinkUserData(pObj1); break;
            case USHORT(SDRUSERDATA_OBJTEXTLINK) : pData=new ImpSdrObjTextLinkUserData((SdrTextObj*)pObj1); break;
        }
    }
    if (pData==NULL) {
        SdrObjFactory aFact(nInvent,nIdent,pObj1);
        SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
        unsigned nAnz=rLL.GetLinkCount();
        unsigned i=0;
        while (i<nAnz && pData==NULL) {
            rLL.GetLink(i).Call((void*)&aFact);
            pData=aFact.pNewData;
            i++;
        }
    }
    return pData;
}

void SdrObjFactory::InsertMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjHdl();
    rLL.RemoveLink(rLink);
}

void SdrObjFactory::InsertMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
    rLL.RemoveLink(rLink);
}

