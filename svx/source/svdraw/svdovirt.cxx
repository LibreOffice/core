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
#include <svx/xpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdhdl.hxx>
#include <svx/sdr/contact/viewcontactofvirtobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
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
    bVirtObj=true; // this is only a virtual object
    rRefObj.AddReference(*this);
    bClosedObj=rRefObj.IsClosedObj();
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
    bClosedObj=rRefObj.IsClosedObj();
    SetRectsDirty(); // TODO: Optimize this.

    // Only a repaint here, rRefObj may have changed and broadcasts
    ActionChanged();
}

void SdrVirtObj::NbcSetAnchorPos(const Point& rAnchorPos)
{
    aAnchor=rAnchorPos;
}

void SdrVirtObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rRefObj.TakeObjInfo(rInfo);
}

SdrInventor SdrVirtObj::GetObjInventor() const
{
    return rRefObj.GetObjInventor();
}

sal_uInt16 SdrVirtObj::GetObjIdentifier() const
{
    return rRefObj.GetObjIdentifier();
}

SdrObjList* SdrVirtObj::GetSubList() const
{
    return rRefObj.GetSubList();
}

const tools::Rectangle& SdrVirtObj::GetCurrentBoundRect() const
{
    const_cast<SdrVirtObj*>(this)->aOutRect=rRefObj.GetCurrentBoundRect(); // TODO: Optimize this.
    const_cast<SdrVirtObj*>(this)->aOutRect+=aAnchor;
    return aOutRect;
}

const tools::Rectangle& SdrVirtObj::GetLastBoundRect() const
{
    const_cast<SdrVirtObj*>(this)->aOutRect=rRefObj.GetLastBoundRect(); // TODO: Optimize this.
    const_cast<SdrVirtObj*>(this)->aOutRect+=aAnchor;
    return aOutRect;
}

void SdrVirtObj::RecalcBoundRect()
{
    aOutRect=rRefObj.GetCurrentBoundRect();
    aOutRect+=aAnchor;
}

SdrVirtObj* SdrVirtObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrVirtObj >(rTargetModel);
    // TTTT not sure if the above works - how could SdrObjFactory::MakeNewObject
    // create an object with correct rRefObj (?) OTOH VirtObj probably needs not
    // to be cloned ever - only used in Writer for multiple instances e.g. Header/Footer
    // return new SdrVirtObj(
    //     getSdrModelFromSdrObject(),
    //     rRefObj); // only a further reference
}

SdrVirtObj& SdrVirtObj::operator=(const SdrVirtObj& rObj)
{
    SdrObject::operator=(rObj);

    // reference different object?? TTTT -> yes!
    rRefObj.DelReference(*this);
    rRefObj = rObj.rRefObj;
    rRefObj.AddReference(*this);

    aSnapRect = rObj.aSnapRect;
    aAnchor = rObj.aAnchor;

    return *this;
}

OUString SdrVirtObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(rRefObj.TakeObjNameSingul());
    sName.insert(0, '[');
    sName.append(']');

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

OUString SdrVirtObj::TakeObjNamePlural() const
{
    OUStringBuffer sName(rRefObj.TakeObjNamePlural());
    sName.insert(0, '[');
    sName.append(']');
    return sName.makeStringAndClear();
}

bool SdrVirtObj::HasLimitedRotation() const
{
    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation
    return rRefObj.HasLimitedRotation();
}

basegfx::B2DPolyPolygon SdrVirtObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPolygon(rRefObj.TakeXorPoly());

    if(aAnchor.X() || aAnchor.Y())
    {
        aPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(aAnchor.X(), aAnchor.Y()));
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
        Point aP(pHdl->GetPos()+aAnchor);
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
        Point aP(pHdl->GetPos()+aAnchor);
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

SdrObject* SdrVirtObj::getFullDragClone() const
{
    SdrObject& rReferencedObject = const_cast<SdrVirtObj*>(this)->ReferencedObj();
    return new SdrGrafObj(
        getSdrModelFromSdrObject(),
        SdrDragView::GetObjGraphic(rReferencedObject),
        GetLogicRect());
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
    aAnchor.Move(rSiz);
    SetRectsDirty();
}

void SdrVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    rRefObj.NbcResize(rRef-aAnchor,xFact,yFact);
    SetRectsDirty();
}

void SdrVirtObj::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    rRefObj.NbcRotate(rRef-aAnchor,nAngle,sn,cs);
    SetRectsDirty();
}

void SdrVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    rRefObj.NbcMirror(rRef1-aAnchor,rRef2-aAnchor);
    SetRectsDirty();
}

void SdrVirtObj::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    rRefObj.NbcShear(rRef-aAnchor,nAngle,tn,bVShear);
    SetRectsDirty();
}


void SdrVirtObj::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

void SdrVirtObj::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Resize(rRef-aAnchor,xFact,yFact, bUnsetRelative);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Rotate(const Point& rRef, long nAngle, double sn, double cs)
{
    if (nAngle!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Rotate(rRef-aAnchor,nAngle,sn,cs);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Mirror(const Point& rRef1, const Point& rRef2)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    rRefObj.Mirror(rRef1-aAnchor,rRef2-aAnchor);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::Shear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    if (nAngle!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        rRefObj.Shear(rRef-aAnchor,nAngle,tn,bVShear);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}


void SdrVirtObj::RecalcSnapRect()
{
    aSnapRect=rRefObj.GetSnapRect();
    aSnapRect+=aAnchor;
}

const tools::Rectangle& SdrVirtObj::GetSnapRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect=rRefObj.GetSnapRect();
    const_cast<SdrVirtObj*>(this)->aSnapRect+=aAnchor;
    return aSnapRect;
}

void SdrVirtObj::SetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    rRefObj.SetSnapRect(aR);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    SetRectsDirty();
    rRefObj.NbcSetSnapRect(aR);
}


const tools::Rectangle& SdrVirtObj::GetLogicRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect=rRefObj.GetLogicRect();  // An abuse of aSnapRect!
    const_cast<SdrVirtObj*>(this)->aSnapRect+=aAnchor;                // If there's trouble, we need another Rectangle Member (or a Heap).
    return aSnapRect;
}

void SdrVirtObj::SetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    rRefObj.SetLogicRect(aR);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    SetRectsDirty();
    rRefObj.NbcSetLogicRect(aR);
}


long SdrVirtObj::GetRotateAngle() const
{
    return rRefObj.GetRotateAngle();
}

long SdrVirtObj::GetShearAngle(bool bVertical) const
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
    aP+=aAnchor;
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
    return rRefObj.GetPoint(i) + aAnchor;
}

void SdrVirtObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    Point aP(rPnt);
    aP-=aAnchor;
    rRefObj.SetPoint(aP,i);
    SetRectsDirty();
}


SdrObjGeoData* SdrVirtObj::NewGeoData() const
{
    return rRefObj.NewGeoData();
}

void SdrVirtObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rRefObj.SaveGeoData(rGeo);
}

void SdrVirtObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    rRefObj.RestGeoData(rGeo);
    SetRectsDirty();
}


SdrObjGeoData* SdrVirtObj::GetGeoData() const
{
    return rRefObj.GetGeoData();
}

void SdrVirtObj::SetGeoData(const SdrObjGeoData& rGeo)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    rRefObj.SetGeoData(rGeo);
    SetRectsDirty();
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

const Point SdrVirtObj::GetOffset() const
{
    // #i73248# default offset of SdrVirtObj is aAnchor
    return aAnchor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
