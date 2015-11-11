/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svdotext.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdview.hxx>
#include <svx/svdorect.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include <svx/svdoashp.hxx>
#include <tools/bigint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>


sal_uInt32 SdrTextObj::GetHdlCount() const
{
    return 8L;
}

SdrHdl* SdrTextObj::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pH=nullptr;
    Point aPnt;
    SdrHdlKind eKind=HDL_MOVE;
    switch (nHdlNum) {
        case 0: aPnt=maRect.TopLeft();      eKind=HDL_UPLFT; break;
        case 1: aPnt=maRect.TopCenter();    eKind=HDL_UPPER; break;
        case 2: aPnt=maRect.TopRight();     eKind=HDL_UPRGT; break;
        case 3: aPnt=maRect.LeftCenter();   eKind=HDL_LEFT ; break;
        case 4: aPnt=maRect.RightCenter();  eKind=HDL_RIGHT; break;
        case 5: aPnt=maRect.BottomLeft();   eKind=HDL_LWLFT; break;
        case 6: aPnt=maRect.BottomCenter(); eKind=HDL_LOWER; break;
        case 7: aPnt=maRect.BottomRight();  eKind=HDL_LWRGT; break;
    }
    if (aGeo.nShearAngle!=0) ShearPoint(aPnt,maRect.TopLeft(),aGeo.nTan);
    if (aGeo.nRotationAngle!=0) RotatePoint(aPnt,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);
    if (eKind!=HDL_MOVE) {
        pH=new SdrHdl(aPnt,eKind);
        pH->SetObj(const_cast<SdrTextObj*>(this));
        pH->SetRotationAngle(aGeo.nRotationAngle);
    }
    return pH;
}



bool SdrTextObj::hasSpecialDrag() const
{
    return true;
}

Rectangle SdrTextObj::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    Rectangle aTmpRect(maRect);
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==nullptr ? HDL_MOVE : pHdl->GetKind();
    bool bEcke=(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
    bool bOrtho=rDrag.GetView()!=nullptr && rDrag.GetView()->IsOrtho();
    bool bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    // Unrotate:
    if (aGeo.nRotationAngle!=0) RotatePoint(aPos,aTmpRect.TopLeft(),-aGeo.nSin,aGeo.nCos);
    // Unshear:
    if (aGeo.nShearAngle!=0) ShearPoint(aPos,aTmpRect.TopLeft(),-aGeo.nTan);

    bool bLft=(eHdl==HDL_UPLFT || eHdl==HDL_LEFT  || eHdl==HDL_LWLFT);
    bool bRgt=(eHdl==HDL_UPRGT || eHdl==HDL_RIGHT || eHdl==HDL_LWRGT);
    bool bTop=(eHdl==HDL_UPRGT || eHdl==HDL_UPPER || eHdl==HDL_UPLFT);
    bool bBtm=(eHdl==HDL_LWRGT || eHdl==HDL_LOWER || eHdl==HDL_LWLFT);
    if (bLft) aTmpRect.Left()  =aPos.X();
    if (bRgt) aTmpRect.Right() =aPos.X();
    if (bTop) aTmpRect.Top()   =aPos.Y();
    if (bBtm) aTmpRect.Bottom()=aPos.Y();
    if (bOrtho) { // Ortho
        long nWdt0=maRect.Right() -maRect.Left();
        long nHgt0=maRect.Bottom()-maRect.Top();
        long nXMul=aTmpRect.Right() -aTmpRect.Left();
        long nYMul=aTmpRect.Bottom()-aTmpRect.Top();
        long nXDiv=nWdt0;
        long nYDiv=nHgt0;
        bool bXNeg=(nXMul<0)!=(nXDiv<0);
        bool bYNeg=(nYMul<0)!=(nYDiv<0);
        nXMul=std::abs(nXMul);
        nYMul=std::abs(nYMul);
        nXDiv=std::abs(nXDiv);
        nYDiv=std::abs(nYDiv);
        Fraction aXFact(nXMul,nXDiv); // fractions for canceling
        Fraction aYFact(nYMul,nYDiv); // and for comparing
        nXMul=aXFact.GetNumerator();
        nYMul=aYFact.GetNumerator();
        nXDiv=aXFact.GetDenominator();
        nYDiv=aYFact.GetDenominator();
        if (bEcke) { // corner point handles
            bool bUseX=(aXFact<aYFact) != bBigOrtho;
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
        } else { // apex handles
            if ((bLft || bRgt) && nXDiv!=0) {
                long nHgt0b=maRect.Bottom()-maRect.Top();
                long nNeed=long(BigInt(nHgt0b)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.Top()-=(nNeed-nHgt0b)/2;
                aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                long nWdt0b=maRect.Right()-maRect.Left();
                long nNeed=long(BigInt(nWdt0b)*BigInt(nYMul)/BigInt(nYDiv));
                aTmpRect.Left()-=(nNeed-nWdt0b)/2;
                aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        }
    }
    if (dynamic_cast<const SdrObjCustomShape*>(this) ==  nullptr)        // not justifying when in CustomShapes, to be able to detect if a shape has to be mirrored
        ImpJustifyRect(aTmpRect);
    return aTmpRect;
}


// drag

bool SdrTextObj::applySpecialDrag(SdrDragStat& rDrag)
{
    Rectangle aNewRect(ImpDragCalcRect(rDrag));

    if(aNewRect.TopLeft() != maRect.TopLeft() && (aGeo.nRotationAngle || aGeo.nShearAngle))
    {
        Point aNewPos(aNewRect.TopLeft());

        if(aGeo.nShearAngle)
            ShearPoint(aNewPos,maRect.TopLeft(),aGeo.nTan);

        if(aGeo.nRotationAngle)
            RotatePoint(aNewPos,maRect.TopLeft(),aGeo.nSin,aGeo.nCos);

        aNewRect.SetPos(aNewPos);
    }

    if (aNewRect != maRect)
    {
        NbcSetLogicRect(aNewRect);
    }

    return true;
}

OUString SdrTextObj::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_DragRectResize,aStr);
    return aStr;
}


// Create

bool SdrTextObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    maRect = aRect1;
    return true;
}

bool SdrTextObj::MovCreate(SdrDragStat& rStat)
{
    Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    maRect = aRect1; // for ObjName
    SetBoundRectDirty();
    bSnapRectDirty=true;
    if (dynamic_cast<const SdrRectObj *>(this) != nullptr) {
        static_cast<SdrRectObj*>(this)->SetXPolyDirty();
    }
    return true;
}

bool SdrTextObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(maRect);
    ImpJustifyRect(maRect);

    AdaptTextMinSize();

    SetRectsDirty();
    if (dynamic_cast<const SdrRectObj *>(this) != nullptr) {
        static_cast<SdrRectObj*>(this)->SetXPolyDirty();
    }
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrTextObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

bool SdrTextObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return true;
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
    if (IsTextFrame()) return Pointer(PointerStyle::DrawText);
    return Pointer(PointerStyle::Cross);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
