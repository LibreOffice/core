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
#include <rtl/ustrbuf.hxx>

sdr::properties::BaseProperties& SdrVirtObj::GetProperties() const
{
    return mxRefObj->GetProperties();
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
    mxRefObj(&rNewObj)
{
    bVirtObj=true; // this is only a virtual object
    mxRefObj->AddReference(*this);
    bClosedObj = mxRefObj->IsClosedObj();
}

SdrVirtObj::SdrVirtObj(
    SdrModel& rSdrModel, SdrVirtObj const & rSource)
:   SdrObject(rSdrModel, rSource),
    mxRefObj(rSource.mxRefObj)
{
    bVirtObj=true; // this is only a virtual object
    bClosedObj = mxRefObj->IsClosedObj();

    mxRefObj->AddReference(*this);

    aSnapRect = rSource.aSnapRect;
    aAnchor = rSource.aAnchor;
}

SdrVirtObj::~SdrVirtObj()
{
    mxRefObj->DelReference(*this);
}

const SdrObject& SdrVirtObj::GetReferencedObj() const
{
    return *mxRefObj;
}

SdrObject& SdrVirtObj::ReferencedObj()
{
    return *mxRefObj;
}

void SdrVirtObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& /*rHint*/)
{
    bClosedObj = mxRefObj->IsClosedObj();
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
    mxRefObj->TakeObjInfo(rInfo);
}

SdrInventor SdrVirtObj::GetObjInventor() const
{
    return mxRefObj->GetObjInventor();
}

SdrObjKind SdrVirtObj::GetObjIdentifier() const
{
    return mxRefObj->GetObjIdentifier();
}

SdrObjList* SdrVirtObj::GetSubList() const
{
    return mxRefObj->GetSubList();
}

const tools::Rectangle& SdrVirtObj::GetCurrentBoundRect() const
{
    const_cast<SdrVirtObj*>(this)->aOutRect = mxRefObj->GetCurrentBoundRect(); // TODO: Optimize this.
    const_cast<SdrVirtObj*>(this)->aOutRect+=aAnchor;
    return aOutRect;
}

const tools::Rectangle& SdrVirtObj::GetLastBoundRect() const
{
    const_cast<SdrVirtObj*>(this)->aOutRect = mxRefObj->GetLastBoundRect(); // TODO: Optimize this.
    const_cast<SdrVirtObj*>(this)->aOutRect+=aAnchor;
    return aOutRect;
}

void SdrVirtObj::RecalcBoundRect()
{
    aOutRect = mxRefObj->GetCurrentBoundRect();
    aOutRect+=aAnchor;
}

rtl::Reference<SdrObject> SdrVirtObj::CloneSdrObject(SdrModel& rTargetModel) const
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
    OUStringBuffer sName(mxRefObj->TakeObjNameSingul());
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
    OUStringBuffer sName(mxRefObj->TakeObjNamePlural());
    sName.insert(0, '[');
    sName.append(']');
    return sName.makeStringAndClear();
}

bool SdrVirtObj::HasLimitedRotation() const
{
    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation
    return mxRefObj->HasLimitedRotation();
}

basegfx::B2DPolyPolygon SdrVirtObj::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aPolyPolygon(mxRefObj->TakeXorPoly());

    if(aAnchor.X() || aAnchor.Y())
    {
        aPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(aAnchor.X(), aAnchor.Y()));
    }

    return aPolyPolygon;
}


sal_uInt32 SdrVirtObj::GetHdlCount() const
{
    return mxRefObj->GetHdlCount();
}

void SdrVirtObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    SdrHdlList tempList(nullptr);
    mxRefObj->AddToHdlList(tempList);
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
    mxRefObj->AddToPlusHdlList(tempList, rHdl);
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
    return mxRefObj->hasSpecialDrag();
}

bool SdrVirtObj::supportsFullDrag() const
{
    return false;
}

rtl::Reference<SdrObject> SdrVirtObj::getFullDragClone() const
{
    SdrObject& rReferencedObject = const_cast<SdrVirtObj*>(this)->ReferencedObj();
    return rtl::Reference<SdrObject>(new SdrGrafObj(
        getSdrModelFromSdrObject(),
        SdrDragView::GetObjGraphic(rReferencedObject),
        GetLogicRect()));
}

bool SdrVirtObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    return mxRefObj->beginSpecialDrag(rDrag);
}

bool SdrVirtObj::applySpecialDrag(SdrDragStat& rDrag)
{
    return mxRefObj->applySpecialDrag(rDrag);
}

basegfx::B2DPolyPolygon SdrVirtObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    return mxRefObj->getSpecialDragPoly(rDrag);
    // TODO: we don't handle offsets yet!
}

OUString SdrVirtObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    return mxRefObj->getSpecialDragComment(rDrag);
}


bool SdrVirtObj::BegCreate(SdrDragStat& rStat)
{
    return mxRefObj->BegCreate(rStat);
}

bool SdrVirtObj::MovCreate(SdrDragStat& rStat)
{
    return mxRefObj->MovCreate(rStat);
}

bool SdrVirtObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    return mxRefObj->EndCreate(rStat,eCmd);
}

bool SdrVirtObj::BckCreate(SdrDragStat& rStat)
{
    return mxRefObj->BckCreate(rStat);
}

void SdrVirtObj::BrkCreate(SdrDragStat& rStat)
{
    mxRefObj->BrkCreate(rStat);
}

basegfx::B2DPolyPolygon SdrVirtObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    return mxRefObj->TakeCreatePoly(rDrag);
    // TODO: we don't handle offsets yet!
}


void SdrVirtObj::NbcMove(const Size& rSiz)
{
    aAnchor.Move(rSiz);
    SetRectsDirty();
}

void SdrVirtObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    mxRefObj->NbcResize(rRef-aAnchor,xFact,yFact);
    SetRectsDirty();
}

void SdrVirtObj::NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    mxRefObj->NbcRotate(rRef-aAnchor,nAngle,sn,cs);
    SetRectsDirty();
}

void SdrVirtObj::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    mxRefObj->NbcMirror(rRef1-aAnchor,rRef2-aAnchor);
    SetRectsDirty();
}

void SdrVirtObj::NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    mxRefObj->NbcShear(rRef-aAnchor,nAngle,tn,bVShear);
    SetRectsDirty();
}


void SdrVirtObj::Move(const Size& rSiz)
{
    if (!rSiz.IsEmpty()) {
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
        mxRefObj->Resize(rRef-aAnchor,xFact,yFact, bUnsetRelative);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        mxRefObj->Rotate(rRef-aAnchor,nAngle,sn,cs);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrVirtObj::Mirror(const Point& rRef1, const Point& rRef2)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    mxRefObj->Mirror(rRef1-aAnchor,rRef2-aAnchor);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        mxRefObj->Shear(rRef-aAnchor,nAngle,tn,bVShear);
        SetRectsDirty();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}


void SdrVirtObj::RecalcSnapRect()
{
    aSnapRect = mxRefObj->GetSnapRect();
    aSnapRect+=aAnchor;
}

const tools::Rectangle& SdrVirtObj::GetSnapRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect=mxRefObj->GetSnapRect();
    const_cast<SdrVirtObj*>(this)->aSnapRect+=aAnchor;
    return aSnapRect;
}

void SdrVirtObj::SetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    mxRefObj->SetSnapRect(aR);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    SetRectsDirty();
    mxRefObj->NbcSetSnapRect(aR);
}


const tools::Rectangle& SdrVirtObj::GetLogicRect() const
{
    const_cast<SdrVirtObj*>(this)->aSnapRect = mxRefObj->GetLogicRect();  // An abuse of aSnapRect!
    const_cast<SdrVirtObj*>(this)->aSnapRect+=aAnchor;                // If there's trouble, we need another Rectangle Member (or a Heap).
    return aSnapRect;
}

void SdrVirtObj::SetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    mxRefObj->SetLogicRect(aR);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrVirtObj::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aR(rRect);
    aR-=aAnchor;
    SetRectsDirty();
    mxRefObj->NbcSetLogicRect(aR);
}


Degree100 SdrVirtObj::GetRotateAngle() const
{
    return mxRefObj->GetRotateAngle();
}

Degree100 SdrVirtObj::GetShearAngle(bool bVertical) const
{
    return mxRefObj->GetShearAngle(bVertical);
}


sal_uInt32 SdrVirtObj::GetSnapPointCount() const
{
    return mxRefObj->GetSnapPointCount();
}

Point SdrVirtObj::GetSnapPoint(sal_uInt32 i) const
{
    Point aP(mxRefObj->GetSnapPoint(i));
    aP+=aAnchor;
    return aP;
}

bool SdrVirtObj::IsPolyObj() const
{
    return mxRefObj->IsPolyObj();
}

sal_uInt32 SdrVirtObj::GetPointCount() const
{
    return mxRefObj->GetPointCount();
}

Point SdrVirtObj::GetPoint(sal_uInt32 i) const
{
    return mxRefObj->GetPoint(i) + aAnchor;
}

void SdrVirtObj::NbcSetPoint(const Point& rPnt, sal_uInt32 i)
{
    Point aP(rPnt);
    aP-=aAnchor;
    mxRefObj->SetPoint(aP,i);
    SetRectsDirty();
}


std::unique_ptr<SdrObjGeoData> SdrVirtObj::NewGeoData() const
{
    return mxRefObj->NewGeoData();
}

void SdrVirtObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    mxRefObj->SaveGeoData(rGeo);
}

void SdrVirtObj::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    mxRefObj->RestoreGeoData(rGeo);
    SetRectsDirty();
}


std::unique_ptr<SdrObjGeoData> SdrVirtObj::GetGeoData() const
{
    return mxRefObj->GetGeoData();
}

void SdrVirtObj::SetGeoData(const SdrObjGeoData& rGeo)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    mxRefObj->SetGeoData(rGeo);
    SetRectsDirty();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrVirtObj::NbcReformatText()
{
    mxRefObj->NbcReformatText();
}

bool SdrVirtObj::HasMacro() const
{
    return mxRefObj->HasMacro();
}

SdrObject* SdrVirtObj::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return mxRefObj->CheckMacroHit(rRec); // TODO: positioning offset
}

PointerStyle SdrVirtObj::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    return mxRefObj->GetMacroPointer(rRec); // TODO: positioning offset
}

void SdrVirtObj::PaintMacro(OutputDevice& rOut, const tools::Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const
{
    mxRefObj->PaintMacro(rOut,rDirtyRect,rRec); // TODO: positioning offset
}

bool SdrVirtObj::DoMacro(const SdrObjMacroHitRec& rRec)
{
    return mxRefObj->DoMacro(rRec); // TODO: positioning offset
}

Point SdrVirtObj::GetOffset() const
{
    // #i73248# default offset of SdrVirtObj is aAnchor
    return aAnchor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
