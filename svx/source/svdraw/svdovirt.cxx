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


#include <svx/svdovirt.hxx>
#include <svx/svdhdl.hxx>
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svddrgv.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

sdr::properties::BaseProperties& SdrVirtObj::GetProperties() const
{
    return rRefObj.GetProperties();
}


// #i27224#
std::unique_ptr<sdr::contact::ViewContact> SdrVirtObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfVirtObj>(*this);
}

SdrVirtObj::SdrVirtObj(
    SdrModel& rSdrModel,
    SdrObject& rNewObj)
:   SdrObject(rSdrModel),
    rRefObj(rNewObj)
{
    m_bVirtObj=true; // this is only a virtual object
    rRefObj.AddReference(*this);
    m_bClosedObj=rRefObj.IsClosedObj();
}

SdrVirtObj::SdrVirtObj(
    SdrModel& rSdrModel, SdrVirtObj const & rSource)
:   SdrObject(rSdrModel, rSource),
    rRefObj(rSource.rRefObj)
{
    m_bVirtObj=true; // this is only a virtual object
    m_bClosedObj=rRefObj.IsClosedObj();

    rRefObj.AddReference(*this);

    aSnapRect = rSource.aSnapRect;
    m_aAnchor = rSource.m_aAnchor;
}

SdrVirtObj::~SdrVirtObj()
{
    rRefObj.DelReference(*this);
}

const SdrObject& SdrVirtObj::GetReferencedObj() const
{
    return rRefObj;
}

SdrObject& SdrVirtObj::ReferencedObj()
{
    return rRefObj;
}

void SdrVirtObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
    m_bClosedObj=rRefObj.IsClosedObj();
    SetBoundAndSnapRectsDirty(); // TODO: Optimize this.

    // Only a repaint here, rRefObj may have changed and broadcasts
    ActionChanged();
}

void SdrVirtObj::NbcSetAnchorPos(const Point& rAnchorPos)
{
    m_aAnchor=rAnchorPos;
}

void SdrVirtObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rRefObj.TakeObjInfo(rInfo);
}

SdrInventor SdrVirtObj::GetObjInventor() const
{
    return rRefObj.GetObjInventor();
}

SdrObjKind SdrVirtObj::GetObjIdentifier() const
{
    return rRefObj.GetObjIdentifier();
}

SdrObjList* SdrVirtObj::GetSubList() const
{
    return rRefObj.GetSubList();
}

const tools::Rectangle& SdrVirtObj::GetCurrentBoundRect() const
{
    auto aRectangle = rRefObj.GetCurrentBoundRect(); // TODO: Optimize this.
    aRectangle += m_aAnchor;
    setOutRectangleConst(aRectangle);
    return m_aOutRect;
}

const tools::Rectangle& SdrVirtObj::GetLastBoundRect() const
{
    auto aRectangle = rRefObj.GetLastBoundRect(); // TODO: Optimize this.
    aRectangle += m_aAnchor;
    setOutRectangleConst(aRectangle);
    return m_aOutRect;
}

void SdrVirtObj::RecalcBoundRect()
{
    tools::Rectangle aRectangle = rRefObj.GetCurrentBoundRect();
    aRectangle += m_aAnchor;
    setOutRectangle(aRectangle);
}

SdrVirtObj* SdrVirtObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new SdrVirtObj(rTargetModel, *this);
    // TTTT not sure if the above works - how could SdrObjFactory::MakeNewObject
    // create an object with correct rRefObj (?) OTOH VirtObj probably needs not
    // to be cloned ever - only used in Writer for multiple instances e.g. Header/Footer
    // return new SdrVirtObj(
    //     getSdrModelFromSdrObject(),
    //     rRefObj); // only a further reference
}

OUString SdrVirtObj::TakeObjNameSingul() const
{
    OUString sName = "[" + rRefObj.TakeObjNameSingul() + "]";

    OUString aName(GetName());
    if (!aName.isEmpty())
        sName += " '" + aName + "'";

    return sName;
}

OUString SdrVirtObj::TakeObjNamePlural() const
{
    return "[" + rRefObj.TakeObjNamePlural() + "]";
}

bool SdrVirtObj::HasLimitedRotation() const
{
    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation
    return rRefObj.HasLimitedRotation();
}

basegfx::B2DPolyPolygon SdrVirtObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPolygon(rRefObj.TakeXorPoly());

    if(m_aAnchor.X() || m_aAnchor.Y())
    {
        aPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(m_aAnchor.X(), m_aAnchor.Y()));
    }

    return aPolyPolygon;
}


sal_uInt32 SdrVirtObj::GetHdlCount() const
{
    return rRefObj.GetHdlCount();
}

void SdrVirtObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    SdrHdlList tempList(nullptr);
    rRefObj.AddToHdlList(tempList);
    for (size_t i=0; i<tempList.GetHdlCount(); ++i)
    {
        SdrHdl* pHdl = tempList.GetHdl(i);
        Point aP(pHdl->GetPos()+m_aAnchor);
        pHdl->SetPos(aP);
    }
    tempList.MoveTo(rHdlList);
}

void SdrVirtObj::AddToPlusHdlList(SdrHdlList& rHdlList, SdrHdl& rHdl) const
{
    SdrHdlList tempList(nullptr);
    rRefObj.AddToPlusHdlList(tempList, rHdl);
    for (size_t i=0; i<tempList.GetHdlCount(); ++i)
    {
        SdrHdl* pHdl = tempList.GetHdl(i);
        Point aP(pHdl->GetPos()+m_aAnchor);
        pHdl->SetPos(aP);
    }
    tempList.MoveTo(rHdlList);
}

bool SdrVirtObj::hasSpecialDrag() const
{
    return rRefObj.hasSpecialDrag();
}

bool SdrVirtObj::supportsFullDrag() const
{
    return false;
}

SdrObjectUniquePtr SdrVirtObj::getFullDragClone() const
{
    SdrObject& rReferencedObject = const_cast<SdrVirtObj*>(this)->ReferencedObj();
    return SdrObjectUniquePtr(new SdrGrafObj(
        getSdrModelFromSdrObject(),
        SdrDragView::GetObjGraphic(rReferencedObject),
        GetLogicRect()));
}

bool SdrVirtObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    return rRefObj.beginSpecialDrag(rDrag);
}

bool SdrVirtObj::applySpecialDrag(SdrDragStat& rDrag)
{
    return rRefObj.applySpecialDrag(rDrag);
}

basegfx::B2DPolyPolygon SdrVirtObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragPoly(rDrag);
    // TODO: we don't handle offsets yet!
}

OUString SdrVirtObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return rRefObj.getSpecialDragComment(rDrag);
}


bool SdrVirtObj::BegCreate(SdrDragStat& rStat)
{
    return rRefObj.BegCreate(rStat);
}

bool SdrVirtObj::MovCreate(SdrDragStat& rStat)
{
    return rRefObj.MovCreate(rStat);
}

bool SdrVirtObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    return rRefObj.EndCreate(rStat,eCmd);
}

bool SdrVirtObj::BckCreate(SdrDragStat& rStat)
{
    return rRefObj.BckCreate(rStat);
}

void SdrVirtObj::BrkCreate(SdrDragStat& rStat)
{
    rRefObj.BrkCreate(rStat);
}

basegfx::B2DPolyPolygon SdrVirtObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    return rRefObj.TakeCreatePoly(rDrag);
    // TODO: we don't handle offsets yet!
}


void SdrVirtObj::NbcMove(const Size& rSiz)
{
    m_aAnchor.Move(rSiz);
    SetBoundAndSnapRectsDirty();
}

void SdrVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    rRefObj.NbcResize(rRef-m_aAnchor,xFact,yFact);
    SetBoundAndSnapRectsDirty();
}

void SdrVirtObj::NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    rRefObj.NbcRotate(rRef-m_aAnchor,nAngle,sn,cs);
    SetBoundAndSnapRectsDirty();
}

void SdrVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    rRefObj.NbcMirror(rRef1-m_aAnchor,rRef2-m_aAnchor);
    SetBoundAndSnapRectsDirty();
}

void SdrVirtObj::NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    rRefObj.NbcShear(rRef-m_aAnchor,nAngle,tn,bVShear);
    SetBoundAndSnapRectsDirty();
}


void SdrVirtObj::Move(const Size& rSiz)
{
    if (!rSiz.IsEmpty()) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

void SdrVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Resize(rRef-m_aAnchor,xFact,yFact, bUnsetRelative);
        SetBoundAndSnapRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Rotate(rRef-m_aAnchor,nAngle,sn,cs);
        SetBoundAndSnapRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Mirror(const Point& rRef1, const Point& rRef2)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    rRefObj.Mirror(rRef1-m_aAnchor,rRef2-m_aAnchor);
    SetBoundAndSnapRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Shear(rRef-m_aAnchor,nAngle,tn,bVShear);
        SetBoundAndSnapRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}


void SdrVirtObj::RecalcSnapRect()
{
    aSnapRect=rRefObj.GetSnapRect();
    aSnapRect+=m_aAnchor;
}

const tools::Rectangle& SdrVirtObj::GetSnapRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect=rRefObj.GetSnapRect();
    const_cast<SdrVirtObj*>(this)->aSnapRect+=m_aAnchor;
    return aSnapRect;
}

void SdrVirtObj::SetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=m_aAnchor;
    rRefObj.SetSnapRect(aR);
    SetBoundAndSnapRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=m_aAnchor;
    SetBoundAndSnapRectsDirty();
    rRefObj.NbcSetSnapRect(aR);
}


const tools::Rectangle& SdrVirtObj::GetLogicRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect=rRefObj.GetLogicRect();  // An abuse of aSnapRect!
    const_cast<SdrVirtObj*>(this)->aSnapRect+=m_aAnchor;                // If there's trouble, we need another Rectangle Member (or a Heap).
    return aSnapRect;
}

void SdrVirtObj::SetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=m_aAnchor;
    rRefObj.SetLogicRect(aR);
    SetBoundAndSnapRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=m_aAnchor;
    SetBoundAndSnapRectsDirty();
    rRefObj.NbcSetLogicRect(aR);
}


Degree100 SdrVirtObj::GetRotateAngle() const
{
    return rRefObj.GetRotateAngle();
}

Degree100 SdrVirtObj::GetShearAngle(bool bVertical) const
{
    return rRefObj.GetShearAngle(bVertical);
}


sal_uInt32 SdrVirtObj::GetSnapPointCount() const
{
    return rRefObj.GetSnapPointCount();
}

Point SdrVirtObj::GetSnapPoint(sal_uInt32 i) const
{
    Point aP(rRefObj.GetSnapPoint(i));
    aP+=m_aAnchor;
    return aP;
}

bool SdrVirtObj::IsPolyObj() const
{
    return rRefObj.IsPolyObj();
}

sal_uInt32 SdrVirtObj::GetPointCount() const
{
    return rRefObj.GetPointCount();
}

Point SdrVirtObj::GetPoint(sal_uInt32 i) const
{
    return rRefObj.GetPoint(i) + m_aAnchor;
}

void SdrVirtObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    Point aP(rPnt);
    aP-=m_aAnchor;
    rRefObj.SetPoint(aP,i);
    SetBoundAndSnapRectsDirty();
}


std::unique_ptr<SdrObjGeoData> SdrVirtObj::NewGeoData() const
{
    return rRefObj.NewGeoData();
}

void SdrVirtObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rRefObj.SaveGeoData(rGeo);
}

void SdrVirtObj::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    rRefObj.RestoreGeoData(rGeo);
    SetBoundAndSnapRectsDirty();
}


std::unique_ptr<SdrObjGeoData> SdrVirtObj::GetGeoData() const
{
    return rRefObj.GetGeoData();
}

void SdrVirtObj::SetGeoData(const SdrObjGeoData& rGeo)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    rRefObj.SetGeoData(rGeo);
    SetBoundAndSnapRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrVirtObj::NbcReformatText()
{
    rRefObj.NbcReformatText();
}

bool SdrVirtObj::HasMacro() const
{
    return rRefObj.HasMacro();
}

SdrObject* SdrVirtObj::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.CheckMacroHit(rRec); // TODO: positioning offset
}

PointerStyle SdrVirtObj::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    return rRefObj.GetMacroPointer(rRec); // TODO: positioning offset
}

void SdrVirtObj::PaintMacro(OutputDevice& rOut, const tools::Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const
{
    rRefObj.PaintMacro(rOut,rDirtyRect,rRec); // TODO: positioning offset
}

bool SdrVirtObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return rRefObj.DoMacro(rRec); // TODO: positioning offset
}

Point SdrVirtObj::GetOffset() const
{
    // #i73248# default offset of SdrVirtObj is aAnchor
    return m_aAnchor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
