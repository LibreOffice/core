/*************************************************************************
 *
 *  $RCSfile: svdotxdr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#include "svdotext.hxx"
#include "svdhdl.hxx"
#include "svddrag.hxx"
#include "svdview.hxx"
#include "svdorect.hxx" // fuer SetXPolyDirty in MovCreate bei SolidDragging
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

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
//  Dragging, Handles, Create
//
////////////////////////////////////////////////////////////////////////////////////////////////////

USHORT SdrTextObj::GetHdlCount() const
{
    return 8;
}

SdrHdl* SdrTextObj::GetHdl(USHORT nHdlNum) const
{
    SdrHdl* pH=NULL;
    Point aPnt;
    SdrHdlKind eKind=HDL_MOVE;
    switch (nHdlNum) {
        case 0: aPnt=aRect.TopLeft();      eKind=HDL_UPLFT; break; // Oben links
        case 1: aPnt=aRect.TopCenter();    eKind=HDL_UPPER; break; // Oben
        case 2: aPnt=aRect.TopRight();     eKind=HDL_UPRGT; break; // Oben rechts
        case 3: aPnt=aRect.LeftCenter();   eKind=HDL_LEFT ; break; // Links
        case 4: aPnt=aRect.RightCenter();  eKind=HDL_RIGHT; break; // Rechts
        case 5: aPnt=aRect.BottomLeft();   eKind=HDL_LWLFT; break; // Unten links
        case 6: aPnt=aRect.BottomCenter(); eKind=HDL_LOWER; break; // Unten
        case 7: aPnt=aRect.BottomRight();  eKind=HDL_LWRGT; break; // Unten rechts
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

FASTBOOL SdrTextObj::HasSpecialDrag() const
{
    return TRUE;
}

Rectangle SdrTextObj::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    Rectangle aTmpRect(aRect);
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    FASTBOOL bEcke=(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
    FASTBOOL bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    FASTBOOL bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    // Unrotate:
    if (aGeo.nDrehWink!=0) RotatePoint(aPos,aTmpRect.TopLeft(),-aGeo.nSin,aGeo.nCos);
    // Unshear:
    if (aGeo.nShearWink!=0) ShearPoint(aPos,aTmpRect.TopLeft(),-aGeo.nTan);
    //
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
    ImpJustifyRect(aTmpRect);
    return aTmpRect;
}

class ImpTextDragUser
{
public:
    Rectangle aR;
};

FASTBOOL SdrTextObj::BegDrag(SdrDragStat& rDrag) const
{
    if (bSizProt) return FALSE; // Groesse geschuetzt
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    if (eHdl==HDL_UPLFT || eHdl==HDL_UPPER || eHdl==HDL_UPRGT ||
        eHdl==HDL_LEFT  ||                    eHdl==HDL_RIGHT ||
        eHdl==HDL_LWLFT || eHdl==HDL_LOWER || eHdl==HDL_LWRGT)
    {
        ImpTextDragUser* pUser=new ImpTextDragUser;
        pUser->aR=aRect;
        rDrag.SetUser(pUser);
        return TRUE;
    }
    return FALSE;
}

FASTBOOL SdrTextObj::MovDrag(SdrDragStat& rDrag) const
{
    ImpTextDragUser* pUser=(ImpTextDragUser*)rDrag.GetUser();
    Rectangle aOldRect(pUser->aR);
    pUser->aR=ImpDragCalcRect(rDrag);
    return pUser->aR!=aOldRect;
}

FASTBOOL SdrTextObj::EndDrag(SdrDragStat& rDrag)
{
    ImpTextDragUser* pUser=(ImpTextDragUser*)rDrag.GetUser();
    Rectangle aNewRect(pUser->aR);
    if (aNewRect.TopLeft()!=aRect.TopLeft() &&
        (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0))
    {
        Point aNewPos(aNewRect.TopLeft());
        if (aGeo.nShearWink!=0) ShearPoint(aNewPos,aRect.TopLeft(),aGeo.nTan);
        if (aGeo.nDrehWink!=0) RotatePoint(aNewPos,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        aNewRect.SetPos(aNewPos);
    }
    if (aNewRect!=aRect) {
        long nHgt0=aRect.Bottom()-aRect.Top();
        long nHgt1=aNewRect.Bottom()-aNewRect.Top();
        long nWdt0=aRect.Right()-aRect.Left();
        long nWdt1=aNewRect.Right()-aNewRect.Left();
        SetLogicRect(aNewRect);
    }
    delete pUser;
    rDrag.SetUser(NULL);
    return TRUE;
}

void SdrTextObj::BrkDrag(SdrDragStat& rDrag) const
{
    delete rDrag.GetUser();
    rDrag.SetUser(NULL);
}

XubString SdrTextObj::GetDragComment(const SdrDragStat& rDrag, FASTBOOL bUndoDragComment, FASTBOOL bCreateComment) const
{
    XubString aStr;
    if (!bCreateComment) ImpTakeDescriptionStr(STR_DragRectResize,aStr);
    return aStr;
}

void SdrTextObj::TakeDragPoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    rXPP.Clear();
    Rectangle aTmpRect(ImpDragCalcRect(rDrag));
    if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0) {
        Polygon aPoly(aTmpRect);
        if (aGeo.nShearWink!=0) ShearPoly(aPoly,aRect.TopLeft(),aGeo.nTan);
        if (aGeo.nDrehWink!=0) RotatePoly(aPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
        rXPP.Insert(XPolygon(aPoly));
    } else {
        rXPP.Insert(XPolygon(aTmpRect));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Create

FASTBOOL SdrTextObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    return TRUE;
}

FASTBOOL SdrTextObj::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    aRect=aRect1; // fuer ObjName
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    if (HAS_BASE(SdrRectObj,this)) {
        ((SdrRectObj*)this)->SetXPolyDirty();
    }
    return TRUE;
}

FASTBOOL SdrTextObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aRect);
    ImpJustifyRect(aRect);
    if (bTextFrame) {
        if (IsAutoGrowHeight()) {
            // MinTextHeight
            long nHgt=aRect.GetHeight()-1;
            if (nHgt==1) nHgt=0;
            NbcSetMinTextFrameHeight(nHgt);
        }
        if (IsAutoGrowWidth()) {
            // MinTextWidth
            long nWdt=aRect.GetWidth()-1;
            if (nWdt==1) nWdt=0;
            NbcSetMinTextFrameWidth(nWdt);
        }
        // Textrahmen neu berechnen
        NbcAdjustTextFrameWidthAndHeight();
    }
    SetRectsDirty();
    if (HAS_BASE(SdrRectObj,this)) {
        ((SdrRectObj*)this)->SetXPolyDirty();
    }
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrTextObj::BrkCreate(SdrDragStat& rStat)
{
}

FASTBOOL SdrTextObj::BckCreate(SdrDragStat& rStat)
{
    return TRUE;
}

void SdrTextObj::TakeCreatePoly(const SdrDragStat& rDrag, XPolyPolygon& rXPP) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();
    rXPP=XPolyPolygon(XPolygon(aRect1));
}

Pointer SdrTextObj::GetCreatePointer() const
{
    if (IsTextFrame()) return Pointer(POINTER_DRAW_TEXT);
    return Pointer(POINTER_CROSS);
}

