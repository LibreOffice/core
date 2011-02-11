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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdview.hxx>
#include <svx/svdorect.hxx> // fuer SetXPolyDirty in MovCreate bei SolidDragging
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include <svx/svdoashp.hxx>
#include <tools/bigint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

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

sal_uInt32 SdrTextObj::GetHdlCount() const
{
    return 8L;
}

SdrHdl* SdrTextObj::GetHdl(sal_uInt32 nHdlNum) const
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

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrTextObj::hasSpecialDrag() const
{
    return true;
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
                long nHgt0b=aRect.Bottom()-aRect.Top();
                long nNeed=long(BigInt(nHgt0b)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.Top()-=(nNeed-nHgt0b)/2;
                aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                long nWdt0b=aRect.Right()-aRect.Left();
                long nNeed=long(BigInt(nWdt0b)*BigInt(nYMul)/BigInt(nYDiv));
                aTmpRect.Left()-=(nNeed-nWdt0b)/2;
                aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        }
    }
    if (!ISA(SdrObjCustomShape))        // not justifying for CustomShapes to be able to detect if a shape has to be mirrored
        ImpJustifyRect(aTmpRect);
    return aTmpRect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// drag

bool SdrTextObj::applySpecialDrag(SdrDragStat& rDrag)
{
    Rectangle aNewRect(ImpDragCalcRect(rDrag));

    if(aNewRect.TopLeft() != aRect.TopLeft() && (aGeo.nDrehWink || aGeo.nShearWink))
    {
        Point aNewPos(aNewRect.TopLeft());

        if(aGeo.nShearWink)
            ShearPoint(aNewPos,aRect.TopLeft(),aGeo.nTan);

        if(aGeo.nDrehWink)
            RotatePoint(aNewPos,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);

        aNewRect.SetPos(aNewPos);
    }

    if(aNewRect != aRect)
    {
          NbcSetLogicRect(aNewRect);
    }

    return true;
}

String SdrTextObj::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    XubString aStr;
    ImpTakeDescriptionStr(STR_DragRectResize,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Create

FASTBOOL SdrTextObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    aRect = aRect1;
    return sal_True;
}

FASTBOOL SdrTextObj::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    aRect=aRect1; // fuer ObjName
    SetBoundRectDirty();
    bSnapRectDirty=sal_True;
    if (HAS_BASE(SdrRectObj,this)) {
        ((SdrRectObj*)this)->SetXPolyDirty();
    }
    return sal_True;
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

void SdrTextObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

FASTBOOL SdrTextObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return sal_True;
}

basegfx::B2DPolyPolygon SdrTextObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect1.Left(), aRect1.Top(), aRect1.Right(), aRect1.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
    return aRetval;
}

Pointer SdrTextObj::GetCreatePointer() const
{
    if (IsTextFrame()) return Pointer(POINTER_DRAW_TEXT);
    return Pointer(POINTER_CROSS);
}

