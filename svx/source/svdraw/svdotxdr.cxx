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
#include <svx/strings.hrc>
#include <svx/svdoashp.hxx>
#include <tools/bigint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/ptrstyle.hxx>


sal_uInt32 SdrTextObj::GetHdlCount() const
{
    return 8;
}

void SdrTextObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    for(sal_uInt32 nHdlNum=0; nHdlNum<8; ++nHdlNum)
    {
        Point aPnt;
        SdrHdlKind eKind = SdrHdlKind::UpperLeft;
        const tools::Rectangle rRectangle = getRectangle();
        switch (nHdlNum) {
            case 0: aPnt = rRectangle.TopLeft();      eKind=SdrHdlKind::UpperLeft; break;
            case 1: aPnt = rRectangle.TopCenter();    eKind=SdrHdlKind::Upper; break;
            case 2: aPnt = rRectangle.TopRight();     eKind=SdrHdlKind::UpperRight; break;
            case 3: aPnt = rRectangle.LeftCenter();   eKind=SdrHdlKind::Left ; break;
            case 4: aPnt = rRectangle.RightCenter();  eKind=SdrHdlKind::Right; break;
            case 5: aPnt = rRectangle.BottomLeft();   eKind=SdrHdlKind::LowerLeft; break;
            case 6: aPnt = rRectangle.BottomCenter(); eKind=SdrHdlKind::Lower; break;
            case 7: aPnt = rRectangle.BottomRight();  eKind=SdrHdlKind::LowerRight; break;
        }
        if (maGeo.m_nShearAngle)
            ShearPoint(aPnt, rRectangle.TopLeft(), maGeo.mfTanShearAngle);
        if (maGeo.m_nRotationAngle)
            RotatePoint(aPnt, rRectangle.TopLeft(), maGeo.mfSinRotationAngle, maGeo.mfCosRotationAngle);
        std::unique_ptr<SdrHdl> pH(new SdrHdl(aPnt,eKind));
        pH->SetObj(const_cast<SdrTextObj*>(this));
        pH->SetRotationAngle(maGeo.m_nRotationAngle);
        rHdlList.AddHdl(std::move(pH));
    }
}

bool SdrTextObj::hasSpecialDrag() const
{
    return true;
}

tools::Rectangle SdrTextObj::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    tools::Rectangle aTmpRect(getRectangle());
    Point aPos(rDrag.GetNow());
    // Unrotate:
    if (maGeo.m_nRotationAngle) RotatePoint(aPos,aTmpRect.TopLeft(),-maGeo.mfSinRotationAngle,maGeo.mfCosRotationAngle);
    // Unshear:
    if (maGeo.m_nShearAngle) ShearPoint(aPos,aTmpRect.TopLeft(),-maGeo.mfTanShearAngle);
    ImpCommonDragCalcRect( rDrag, aTmpRect );
    if (dynamic_cast<const SdrObjCustomShape*>(this) ==  nullptr)        // not justifying when in CustomShapes, to be able to detect if a shape has to be mirrored
        ImpJustifyRect(aTmpRect);
    return aTmpRect;
}


// drag

bool SdrTextObj::applySpecialDrag(SdrDragStat& rDrag)
{
    tools::Rectangle aNewRect(ImpDragCalcRect(rDrag));

    if(aNewRect.TopLeft() != getRectangle().TopLeft() && (maGeo.m_nRotationAngle || maGeo.m_nShearAngle))
    {
        Point aNewPos(aNewRect.TopLeft());

        if (maGeo.m_nShearAngle)
            ShearPoint(aNewPos, getRectangle().TopLeft(), maGeo.mfTanShearAngle);

        if (maGeo.m_nRotationAngle)
            RotatePoint(aNewPos, getRectangle().TopLeft(), maGeo.mfSinRotationAngle, maGeo.mfCosRotationAngle);

        aNewRect.SetPos(aNewPos);
    }

    if (aNewRect != getRectangle())
    {
        NbcSetLogicRect(aNewRect);
    }

    return true;
}

OUString SdrTextObj::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    return ImpGetDescriptionStr(STR_DragRectResize);
}


// Create

bool SdrTextObj::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    tools::Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Normalize();
    rStat.SetActionRect(aRect1);
    setRectangle(aRect1);
    return true;
}

bool SdrTextObj::MovCreate(SdrDragStat& rStat)
{
    tools::Rectangle aRect1;
    rStat.TakeCreateRect(aRect1);
    ImpJustifyRect(aRect1);
    rStat.SetActionRect(aRect1);
    setRectangle(aRect1); // for ObjName
    SetBoundRectDirty();
    m_bSnapRectDirty=true;
    if (auto pRectObj = dynamic_cast<SdrRectObj *>(this)) {
        pRectObj->SetXPolyDirty();
    }
    return true;
}

bool SdrTextObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    tools::Rectangle aRectangle(getRectangle());
    rStat.TakeCreateRect(aRectangle);
    ImpJustifyRect(aRectangle);
    setRectangle(aRectangle);

    AdaptTextMinSize();

    SetBoundAndSnapRectsDirty();
    if (auto pRectObj = dynamic_cast<SdrRectObj *>(this)) {
        pRectObj->SetXPolyDirty();
    }
    return (eCmd==SdrCreateCmd::ForceEnd || rStat.GetPointCount()>=2);
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
    tools::Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Normalize();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange = vcl::unotools::b2DRectangleFromRectangle(aRect1);
    aRetval.append(basegfx::utils::createPolygonFromRect(aRange));
    return aRetval;
}

PointerStyle SdrTextObj::GetCreatePointer() const
{
    if (IsTextFrame()) return PointerStyle::DrawText;
    return PointerStyle::Cross;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
