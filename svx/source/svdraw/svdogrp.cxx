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

#include <svx/svdogrp.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <sdr/properties/groupproperties.hxx>
#include <sdr/contact/viewcontactofgroup.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <libxml/xmlwriter.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/canvastools.hxx>

// BaseProperties section
std::unique_ptr<sdr::properties::BaseProperties> SdrObjGroup::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::GroupProperties>(*this);
}

// DrawContact section
std::unique_ptr<sdr::contact::ViewContact> SdrObjGroup::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfGroup>(*this);
}

SdrObjGroup::SdrObjGroup(SdrModel& rSdrModel)
:   SdrObject(rSdrModel),
    SdrObjList(),
    aRefPoint(0, 0)
{
    m_bClosedObj=false;
}

SdrObjGroup::SdrObjGroup(SdrModel& rSdrModel, SdrObjGroup const & rSource)
:   SdrObject(rSdrModel, rSource),
    SdrObjList()
{
    m_bClosedObj=false;

    // copy child SdrObjects
    if(nullptr != rSource.GetSubList())
    {
        // #i36404# Copy SubList, init model and page first
        const SdrObjList& rSourceSubList(*rSource.GetSubList());

        CopyObjects(rSourceSubList);

        // tdf#116979: needed here, we need bSnapRectDirty to be true
        // which it is after using SdrObject::operator= (see above),
        // but set to false again using CopyObjects
        SetRectsDirty();
    }

    // copy local parameters
    aRefPoint  = rSource.aRefPoint;
}

SdrObjGroup::~SdrObjGroup()
{
}

SdrPage* SdrObjGroup::getSdrPageFromSdrObjList() const
{
    return getSdrPageFromSdrObject();
}

SdrObject* SdrObjGroup::getSdrObjectFromSdrObjList() const
{
    return const_cast< SdrObjGroup* >(this);
}

SdrObjList* SdrObjGroup::getChildrenOfSdrObject() const
{
    return const_cast< SdrObjGroup* >(this);
}

void SdrObjGroup::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bNoContortion=false;
    const size_t nObjCount(GetObjCount());
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj(GetObj(i));
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);
        if (!aInfo.bMoveAllowed            ) rInfo.bMoveAllowed            =false;
        if (!aInfo.bResizeFreeAllowed      ) rInfo.bResizeFreeAllowed      =false;
        if (!aInfo.bResizePropAllowed      ) rInfo.bResizePropAllowed      =false;
        if (!aInfo.bRotateFreeAllowed      ) rInfo.bRotateFreeAllowed      =false;
        if (!aInfo.bRotate90Allowed        ) rInfo.bRotate90Allowed        =false;
        if (!aInfo.bMirrorFreeAllowed      ) rInfo.bMirrorFreeAllowed      =false;
        if (!aInfo.bMirror45Allowed        ) rInfo.bMirror45Allowed        =false;
        if (!aInfo.bMirror90Allowed        ) rInfo.bMirror90Allowed        =false;
        if (!aInfo.bShearAllowed           ) rInfo.bShearAllowed           =false;
        if (!aInfo.bEdgeRadiusAllowed      ) rInfo.bEdgeRadiusAllowed      =false;
        if (!aInfo.bNoOrthoDesired         ) rInfo.bNoOrthoDesired         =false;
        if (aInfo.bNoContortion            ) rInfo.bNoContortion           =true;
        if (!aInfo.bCanConvToPath          ) rInfo.bCanConvToPath          =false;

        if(!aInfo.bCanConvToContour)
            rInfo.bCanConvToContour = false;

        if (!aInfo.bCanConvToPoly          ) rInfo.bCanConvToPoly          =false;
        if (!aInfo.bCanConvToPathLineToArea) rInfo.bCanConvToPathLineToArea=false;
        if (!aInfo.bCanConvToPolyLineToArea) rInfo.bCanConvToPolyLineToArea=false;
    }
    if (nObjCount==0) {
        rInfo.bRotateFreeAllowed=false;
        rInfo.bRotate90Allowed  =false;
        rInfo.bMirrorFreeAllowed=false;
        rInfo.bMirror45Allowed  =false;
        rInfo.bMirror90Allowed  =false;
        rInfo.bTransparenceAllowed = false;
        rInfo.bShearAllowed     =false;
        rInfo.bEdgeRadiusAllowed=false;
        rInfo.bNoContortion     =true;
    }
    if(nObjCount != 1)
    {
        // only allowed if single object selected
        rInfo.bTransparenceAllowed = false;
    }
}

void SdrObjGroup::SetBoundRectDirty()
{
    // avoid resetting aOutRect which in case of this object is model data,
    // not re-creatable view data
}

SdrObjKind SdrObjGroup::GetObjIdentifier() const
{
    return OBJ_GRUP;
}

SdrLayerID SdrObjGroup::GetLayer() const
{
    bool b1st = true;
    SdrLayerID nLay = SdrObject::GetLayer();
    const size_t nObjCount(GetObjCount());
    for (size_t i=0; i<nObjCount; ++i) {
        SdrLayerID nLay1(GetObj(i)->GetLayer());
        if (b1st) { nLay=nLay1; b1st = false; }
        else if (nLay1!=nLay) return SdrLayerID(0);
    }
    return nLay;
}

void SdrObjGroup::NbcSetLayer(SdrLayerID nLayer)
{
    SdrObject::NbcSetLayer(nLayer);
    const size_t nObjCount(GetObjCount());
    for (size_t i=0; i<nObjCount; ++i) {
        GetObj(i)->NbcSetLayer(nLayer);
    }
}

void SdrObjGroup::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    // call parent
    SdrObject::handlePageChange(pOldPage, pNewPage);

    for(size_t i(0); i < GetObjCount(); i++)
    {
        GetObj(i)->handlePageChange(pOldPage, pNewPage);
    }
}

SdrObjList* SdrObjGroup::GetSubList() const
{
    return const_cast< SdrObjGroup* >(this);
}

const tools::Rectangle& SdrObjGroup::GetCurrentBoundRect() const
{
    // <aOutRect> has to contain the bounding rectangle
    if(0 != GetObjCount())
    {
        m_aOutRect = GetAllObjBoundRect();
    }

    return m_aOutRect;
}

const tools::Rectangle& SdrObjGroup::GetSnapRect() const
{
    // <aOutRect> has to contain the bounding rectangle
    if(0 != GetObjCount())
    {
        return GetAllObjSnapRect();
    }
    else
    {
        return m_aOutRect;
    }
}

SdrObjGroup* SdrObjGroup::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new SdrObjGroup(rTargetModel, *this);
}

OUString SdrObjGroup::TakeObjNameSingul() const
{
    OUString sName;

    if(0 == GetObjCount())
    {
        sName = SvxResId(STR_ObjNameSingulGRUPEMPTY);
    }
    else
    {
        sName = SvxResId(STR_ObjNameSingulGRUP);
    }

    const OUString aName(GetName());

    if (!aName.isEmpty())
        sName += " '" + aName + "'";

    return sName;
}


OUString SdrObjGroup::TakeObjNamePlural() const
{
    if(0 == GetObjCount())
        return SvxResId(STR_ObjNamePluralGRUPEMPTY);
    return SvxResId(STR_ObjNamePluralGRUP);
}


void SdrObjGroup::RecalcSnapRect()
{
    // TODO: unnecessary, because we use the Rects from the SubList
}

basegfx::B2DPolyPolygon SdrObjGroup::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const size_t nObjCount(GetObjCount());

    for(size_t a = 0; a < nObjCount; ++a)
    {
        SdrObject* pObj(GetObj(a));
        aRetval.append(pObj->TakeXorPoly());
    }

    if(!aRetval.count())
    {
        const basegfx::B2DRange aRange = vcl::unotools::b2DRectangleFromRectangle(m_aOutRect);
        aRetval.append(basegfx::utils::createPolygonFromRect(aRange));
    }

    return aRetval;
}

bool SdrObjGroup::beginSpecialDrag(SdrDragStat& /*rDrag*/) const
{
    return false;
}


bool SdrObjGroup::BegCreate(SdrDragStat& /*rStat*/)
{
    return false;
}


Degree100 SdrObjGroup::GetRotateAngle() const
{
    Degree100 nRetval(0);

    if(0 != GetObjCount())
    {
        SdrObject* pObj(GetObj(0));

        nRetval = pObj->GetRotateAngle();
    }

    return nRetval;
}


Degree100 SdrObjGroup::GetShearAngle(bool /*bVertical*/) const
{
    Degree100 nRetval(0);

    if(0 != GetObjCount())
    {
        SdrObject* pObj(GetObj(0));

        nRetval = pObj->GetShearAngle();
    }

    return nRetval;
}


void SdrObjGroup::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aOld(GetSnapRect());
    tools::Long nMulX=rRect.Right()-rRect.Left();
    tools::Long nDivX=aOld.Right()-aOld.Left();
    tools::Long nMulY=rRect.Bottom()-rRect.Top();
    tools::Long nDivY=aOld.Bottom()-aOld.Top();
    if (nDivX==0) { nMulX=1; nDivX=1; }
    if (nDivY==0) { nMulY=1; nDivY=1; }
    if (nMulX!=nDivX || nMulY!=nDivY) {
        Fraction aX(nMulX,nDivX);
        Fraction aY(nMulY,nDivY);
        NbcResize(aOld.TopLeft(),aX,aY);
    }
    if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
        NbcMove(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
    }
}


void SdrObjGroup::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}


void SdrObjGroup::NbcMove(const Size& rSiz)
{
    aRefPoint.Move(rSiz);
    const size_t nObjCount(GetObjCount());

    if(0 != nObjCount)
    {
        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            pObj->NbcMove(rSiz);
        }
    }
    else
    {
        m_aOutRect.Move(rSiz);
        SetRectsDirty();
    }
}


void SdrObjGroup::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustY( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustX( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }

    ResizePoint(aRefPoint,rRef,xFact,yFact);

    const size_t nObjCount(GetObjCount());
    if(0 != nObjCount)
    {
        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            pObj->NbcResize(rRef,xFact,yFact);
        }
    }
    else
    {
        ResizeRect(m_aOutRect,rRef,xFact,yFact);
        SetRectsDirty();
    }
}


void SdrObjGroup::NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    RotatePoint(aRefPoint,rRef,sn,cs);
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        pObj->NbcRotate(rRef,nAngle,sn,cs);
    }

    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    MirrorPoint(aRefPoint,rRef1,rRef2); // implementation missing in SvdEtc!
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        pObj->NbcMirror(rRef1,rRef2);
    }

    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);
    ShearPoint(aRefPoint,rRef,tn);
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        pObj->NbcShear(rRef,nAngle,tn,bVShear);
    }

    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcSetAnchorPos(const Point& rPnt)
{
    m_aAnchor=rPnt;
    Size aSiz(rPnt.X()-m_aAnchor.X(),rPnt.Y()-m_aAnchor.Y());
    aRefPoint.Move(aSiz);
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        pObj->NbcSetAnchorPos(rPnt);
    }
}


void SdrObjGroup::SetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    tools::Rectangle aOld(GetSnapRect());
    if (aOld.IsEmpty())
    {
        Fraction aX(1,1);
        Fraction aY(1,1);
        Resize(aOld.TopLeft(),aX,aY);
    }
    else
    {
        tools::Long nMulX=rRect.Right()-rRect.Left();
        tools::Long nDivX=aOld.Right()-aOld.Left();
        tools::Long nMulY=rRect.Bottom()-rRect.Top();
        tools::Long nDivY=aOld.Bottom()-aOld.Top();
        if (nDivX==0) { nMulX=1; nDivX=1; }
        if (nDivY==0) { nMulY=1; nDivY=1; }
        if (nMulX!=nDivX || nMulY!=nDivY) {
            Fraction aX(nMulX,nDivX);
            Fraction aY(nMulY,nDivY);
            Resize(aOld.TopLeft(),aX,aY);
        }
    }
    if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
        Move(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
    }

    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrObjGroup::SetLogicRect(const tools::Rectangle& rRect)
{
    SetSnapRect(rRect);
}


void SdrObjGroup::Move(const Size& rSiz)
{
    if (rSiz.Width()==0 && rSiz.Height()==0)
        return;

    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    aRefPoint.Move(rSiz);
    const size_t nObjCount(GetObjCount());

    if(0 != nObjCount)
    {
        // first move the connectors, then everything else
        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            if (pObj->IsEdgeObj())
                pObj->Move(rSiz);
        }

        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            if (!pObj->IsEdgeObj())
                pObj->Move(rSiz);
        }
    }
    else
    {
        m_aOutRect.Move(rSiz);
        SetRectsDirty();
    }

    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
}


void SdrObjGroup::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()==xFact.GetDenominator() && yFact.GetNumerator()==yFact.GetDenominator())
        return;

    bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustY( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustX( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    ResizePoint(aRefPoint,rRef,xFact,yFact);
    const size_t nObjCount(GetObjCount());

    if(0 != nObjCount)
    {
        // move the connectors first, everything else afterwards
        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            if (pObj->IsEdgeObj())
                pObj->Resize(rRef,xFact,yFact,bUnsetRelative);
        }

        for (size_t i=0; i<nObjCount; ++i)
        {
            SdrObject* pObj(GetObj(i));
            if (!pObj->IsEdgeObj())
                pObj->Resize(rRef,xFact,yFact,bUnsetRelative);
        }
    }
    else
    {
        ResizeRect(m_aOutRect,rRef,xFact,yFact);
        SetRectsDirty();
    }

    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrObjGroup::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    if (nAngle==0_deg100)
        return;

    SetGlueReallyAbsolute(true);
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    RotatePoint(aRefPoint,rRef,sn,cs);
    // move the connectors first, everything else afterwards
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (pObj->IsEdgeObj())
            pObj->Rotate(rRef,nAngle,sn,cs);
    }

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (!pObj->IsEdgeObj())
            pObj->Rotate(rRef,nAngle,sn,cs);
    }

    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrObjGroup::Mirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    MirrorPoint(aRefPoint,rRef1,rRef2); // implementation missing in SvdEtc!
    // move the connectors first, everything else afterwards
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (pObj->IsEdgeObj())
            pObj->Mirror(rRef1,rRef2);
    }

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (!pObj->IsEdgeObj())
            pObj->Mirror(rRef1,rRef2);
    }

    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


void SdrObjGroup::Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    if (nAngle==0_deg100)
        return;

    SetGlueReallyAbsolute(true);
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    ShearPoint(aRefPoint,rRef,tn);
    // move the connectors first, everything else afterwards
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (pObj->IsEdgeObj())
            pObj->Shear(rRef,nAngle,tn,bVShear);
    }

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (!pObj->IsEdgeObj())
            pObj->Shear(rRef,nAngle,tn,bVShear);
    }

    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);

}


void SdrObjGroup::SetAnchorPos(const Point& rPnt)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    bool bChg=m_aAnchor!=rPnt;
    m_aAnchor=rPnt;
    Size aSiz(rPnt.X()-m_aAnchor.X(),rPnt.Y()-m_aAnchor.Y());
    aRefPoint.Move(aSiz);
    // move the connectors first, everything else afterwards
    const size_t nObjCount(GetObjCount());

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (pObj->IsEdgeObj())
            pObj->SetAnchorPos(rPnt);
    }

    for (size_t i=0; i<nObjCount; ++i)
    {
        SdrObject* pObj(GetObj(i));
        if (!pObj->IsEdgeObj())
            pObj->SetAnchorPos(rPnt);
    }

    if (bChg)
    {
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}


void SdrObjGroup::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-m_aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // this also calls SetRectsDirty()
}

void SdrObjGroup::SetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-m_aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    if (aSiz.Width()!=0 || aSiz.Height()!=0) Move(aSiz); // this also calls SetRectsDirty() and Broadcast, ...
}

void SdrObjGroup::NbcReformatText()
{
    NbcReformatAllTextObjects();
}

SdrObjectUniquePtr SdrObjGroup::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    SdrObjectUniquePtr pGroup( new SdrObjGroup(getSdrModelFromSdrObject()) );
    const size_t nObjCount(GetObjCount());

    for(size_t a=0; a < nObjCount; ++a)
    {
        SdrObject* pIterObj(GetObj(a));
        SdrObjectUniquePtr pResult(pIterObj->DoConvertToPolyObj(bBezier, bAddText));

        // pResult can be NULL e.g. for empty objects
        if( pResult )
            pGroup->GetSubList()->NbcInsertObject(pResult.release());
    }

    return pGroup;
}

void SdrObjGroup::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrObjGroup"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    SdrObject::dumpAsXml(pWriter);
    SdrObjList::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
