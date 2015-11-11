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

#include <sfx2/linkmgr.hxx>

#include <unotools/datetime.hxx>

#include <svx/svdogrp.hxx>

#include <sfx2/lnkbase.hxx>

#include <svl/urihelper.hxx>

#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include "svx/svditer.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include "svdglob.hxx"
#include "svx/svdstr.hrc"

#include <svx/svxids.hrc>
#include <svl/whiter.hxx>
#include <svx/svdpool.hxx>
#include <sdr/properties/groupproperties.hxx>
#include <sdr/contact/viewcontactofgroup.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <libxml/xmlwriter.h>


// BaseProperties section

sdr::properties::BaseProperties* SdrObjGroup::CreateObjectSpecificProperties()
{
    return new sdr::properties::GroupProperties(*this);
}


// DrawContact section

sdr::contact::ViewContact* SdrObjGroup::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGroup(*this);
}




SdrObjGroup::SdrObjGroup()
{
    pSub=new SdrObjList(nullptr,nullptr);
    pSub->SetOwnerObj(this);
    pSub->SetListKind(SDROBJLIST_GROUPOBJ);
    bRefPoint=false;
    bClosedObj=false;
}


SdrObjGroup::~SdrObjGroup()
{
    delete pSub;
}

void SdrObjGroup::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bNoContortion=false;
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
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
        rInfo.bGradientAllowed = false;
        rInfo.bShearAllowed     =false;
        rInfo.bEdgeRadiusAllowed=false;
        rInfo.bNoContortion     =true;
    }
    if(nObjCount != 1)
    {
        // only allowed if single object selected
        rInfo.bTransparenceAllowed = false;
        rInfo.bGradientAllowed = false;
    }
}


void SdrObjGroup::SetBoundRectDirty()
{
    // avoid resetting aOutRect which in case of this object is model data,
    // not re-creatable view data
}

sal_uInt16 SdrObjGroup::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_GRUP);
}


SdrLayerID SdrObjGroup::GetLayer() const
{
    bool b1st = true;
    SdrLayerID nLay=SdrLayerID(SdrObject::GetLayer());
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrLayerID nLay1=pOL->GetObj(i)->GetLayer();
        if (b1st) { nLay=nLay1; b1st = false; }
        else if (nLay1!=nLay) return 0;
    }
    return nLay;
}


void SdrObjGroup::NbcSetLayer(SdrLayerID nLayer)
{
    SdrObject::NbcSetLayer(nLayer);
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        pOL->GetObj(i)->NbcSetLayer(nLayer);
    }
}


void SdrObjGroup::SetObjList(SdrObjList* pNewObjList)
{
    SdrObject::SetObjList(pNewObjList);
    pSub->SetUpList(pNewObjList);
}


void SdrObjGroup::SetPage(SdrPage* pNewPage)
{
    SdrObject::SetPage(pNewPage);
    pSub->SetPage(pNewPage);
}


void SdrObjGroup::SetModel(SdrModel* pNewModel)
{
    if(pNewModel!=pModel)
    {
        // #i30648#
        // This method also needs to migrate the used ItemSet
        // when the destination model uses a different pool
        // than the current one. Else it is possible to create
        // SdrObjGroups which reference the old pool which might
        // be destroyed (as the bug shows).
        SdrModel* pOldModel = pModel;

        // test for correct pool in ItemSet; move to new pool if necessary
        if(pNewModel && &GetObjectItemPool() != &pNewModel->GetItemPool())
        {
            MigrateItemPool(&GetObjectItemPool(), &pNewModel->GetItemPool(), pNewModel);
        }

        // call parent
        SdrObject::SetModel(pNewModel);

        // set new model at content
        pSub->SetModel(pNewModel);

        // modify properties
        GetProperties().SetModel(pOldModel, pNewModel);
    }
}


SdrObjList* SdrObjGroup::GetSubList() const
{
    return pSub;
}

const Rectangle& SdrObjGroup::GetCurrentBoundRect() const
{
    // <aOutRect> has to contain the bounding rectangle
    if ( pSub->GetObjCount()!=0 )
    {
        const_cast<SdrObjGroup*>(this)->aOutRect = pSub->GetAllObjBoundRect();
    }

    return aOutRect;
}

const Rectangle& SdrObjGroup::GetSnapRect() const
{
    // <aOutRect> has to contain the bounding rectangle
    if ( pSub->GetObjCount()!=0 )
    {
        return pSub->GetAllObjSnapRect();
    }
    else
    {
        return aOutRect;
    }
}

SdrObjGroup* SdrObjGroup::Clone() const
{
    return CloneHelper< SdrObjGroup >();
}

SdrObjGroup& SdrObjGroup::operator=(const SdrObjGroup& rObj)
{
    if( this == &rObj )
        return *this;
    // copy SdrObject stuff
    SdrObject::operator=(rObj);

    // #i36404#
    // copy SubList, init model and page first
    SdrObjList& rSourceSubList = *rObj.GetSubList();
    pSub->SetPage(rSourceSubList.GetPage());
    pSub->SetModel(rSourceSubList.GetModel());
    pSub->CopyObjects(*rObj.GetSubList());

    // copy local parameters
    aRefPoint  = rObj.aRefPoint;
    bRefPoint  = rObj.bRefPoint;
    return *this;
}


OUString SdrObjGroup::TakeObjNameSingul() const
{
    OUStringBuffer sName;

    if(!pSub->GetObjCount())
    {
        sName.append(ImpGetResStr(STR_ObjNameSingulGRUPEMPTY));
    }
    else
    {
        sName.append(ImpGetResStr(STR_ObjNameSingulGRUP));
    }

    const OUString aName(GetName());

    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}


OUString SdrObjGroup::TakeObjNamePlural() const
{
    if (pSub->GetObjCount()==0)
        return ImpGetResStr(STR_ObjNamePluralGRUPEMPTY);
    return ImpGetResStr(STR_ObjNamePluralGRUP);
}


void SdrObjGroup::RecalcSnapRect()
{
    // TODO: unnecessary, because we use the Rects from the SubList
}

basegfx::B2DPolyPolygon SdrObjGroup::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const size_t nObjCount(pSub->GetObjCount());

    for(size_t a = 0; a < nObjCount; ++a)
    {
        SdrObject* pObj = pSub->GetObj(a);
        aRetval.append(pObj->TakeXorPoly());
    }

    if(!aRetval.count())
    {
        const basegfx::B2DRange aRange(aOutRect.Left(), aOutRect.Top(), aOutRect.Right(), aOutRect.Bottom());
        aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
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


long SdrObjGroup::GetRotateAngle() const
{
    long nRetval(0);

    if(pSub->GetObjCount())
    {
        SdrObject* pObj = pSub->GetObj(0);

        nRetval = pObj->GetRotateAngle();
    }

    return nRetval;
}


long SdrObjGroup::GetShearAngle(bool /*bVertical*/) const
{
    long nRetval(0);

    if(pSub->GetObjCount())
    {
        SdrObject* pObj = pSub->GetObj(0);

        nRetval = pObj->GetShearAngle();
    }

    return nRetval;
}


void SdrObjGroup::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aOld(GetSnapRect());
    long nMulX=rRect.Right()-rRect.Left();
    long nDivX=aOld.Right()-aOld.Left();
    long nMulY=rRect.Bottom()-rRect.Top();
    long nDivY=aOld.Bottom()-aOld.Top();
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


void SdrObjGroup::NbcSetLogicRect(const Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}


void SdrObjGroup::NbcMove(const Size& rSiz)
{
    MovePoint(aRefPoint,rSiz);
    if (pSub->GetObjCount()!=0) {
        SdrObjList* pOL=pSub;
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            pObj->NbcMove(rSiz);
        }
    } else {
        MoveRect(aOutRect,rSiz);
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
            aRef2.Y()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.X()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }
    ResizePoint(aRefPoint,rRef,xFact,yFact);
    if (pSub->GetObjCount()!=0) {
        SdrObjList* pOL=pSub;
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            pObj->NbcResize(rRef,xFact,yFact);
        }
    } else {
        ResizeRect(aOutRect,rRef,xFact,yFact);
        SetRectsDirty();
    }
}


void SdrObjGroup::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    RotatePoint(aRefPoint,rRef,sn,cs);
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcRotate(rRef,nAngle,sn,cs);
    }
    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    MirrorPoint(aRefPoint,rRef1,rRef2); // implementation missing in SvdEtc!
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcMirror(rRef1,rRef2);
    }
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);
    ShearPoint(aRefPoint,rRef,tn);
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcShear(rRef,nAngle,tn,bVShear);
    }
    NbcShearGluePoints(rRef,nAngle,tn,bVShear);
    SetGlueReallyAbsolute(false);
}


void SdrObjGroup::NbcSetAnchorPos(const Point& rPnt)
{
    aAnchor=rPnt;
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    MovePoint(aRefPoint,aSiz);
    SdrObjList* pOL=pSub;
    const size_t nObjCount=pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        pObj->NbcSetAnchorPos(rPnt);
    }
}


void SdrObjGroup::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    Rectangle aOld(GetSnapRect());
    long nMulX=rRect.Right()-rRect.Left();
    long nDivX=aOld.Right()-aOld.Left();
    long nMulY=rRect.Bottom()-rRect.Top();
    long nDivY=aOld.Bottom()-aOld.Top();
    if (nDivX==0) { nMulX=1; nDivX=1; }
    if (nDivY==0) { nMulY=1; nDivY=1; }
    if (nMulX!=nDivX || nMulY!=nDivY) {
        Fraction aX(nMulX,nDivX);
        Fraction aY(nMulY,nDivY);
        Resize(aOld.TopLeft(),aX,aY);
    }
    if (rRect.Left()!=aOld.Left() || rRect.Top()!=aOld.Top()) {
        Move(Size(rRect.Left()-aOld.Left(),rRect.Top()-aOld.Top()));
    }

    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::SetLogicRect(const Rectangle& rRect)
{
    SetSnapRect(rRect);
}


void SdrObjGroup::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        MovePoint(aRefPoint,rSiz);
        if (pSub->GetObjCount()!=0) {
            // first move the connectors, then everything else
            SdrObjList* pOL=pSub;
            const size_t nObjCount = pOL->GetObjCount();
            for (size_t i=0; i<nObjCount; ++i) {
                SdrObject* pObj=pOL->GetObj(i);
                if (pObj->IsEdgeObj()) pObj->Move(rSiz);
            }
            for (size_t i=0; i<nObjCount; ++i) {
                SdrObject* pObj=pOL->GetObj(i);
                if (!pObj->IsEdgeObj()) pObj->Move(rSiz);
            }
        } else {
            MoveRect(aOutRect,rSiz);
            SetRectsDirty();
        }

        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}


void SdrObjGroup::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
        bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
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
        Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        ResizePoint(aRefPoint,rRef,xFact,yFact);
        if (pSub->GetObjCount()!=0) {
            // move the connectors first, everything else afterwards
            SdrObjList* pOL=pSub;
            const size_t nObjCount = pOL->GetObjCount();
            for (size_t i=0; i<nObjCount; ++i) {
                SdrObject* pObj=pOL->GetObj(i);
                if (pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact,bUnsetRelative);
            }
            for (size_t i=0; i<nObjCount; ++i) {
                SdrObject* pObj=pOL->GetObj(i);
                if (!pObj->IsEdgeObj()) pObj->Resize(rRef,xFact,yFact,bUnsetRelative);
            }
        } else {
            ResizeRect(aOutRect,rRef,xFact,yFact);
            SetRectsDirty();
        }

        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Rotate(const Point& rRef, long nAngle, double sn, double cs)
{
    if (nAngle!=0) {
        SetGlueReallyAbsolute(true);
        Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        RotatePoint(aRefPoint,rRef,sn,cs);
        // move the connectors first, everything else afterwards
        SdrObjList* pOL=pSub;
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            if (pObj->IsEdgeObj()) pObj->Rotate(rRef,nAngle,sn,cs);
        }
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            if (!pObj->IsEdgeObj()) pObj->Rotate(rRef,nAngle,sn,cs);
        }
        NbcRotateGluePoints(rRef,nAngle,sn,cs);
        SetGlueReallyAbsolute(false);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::Mirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    MirrorPoint(aRefPoint,rRef1,rRef2); // implementation missing in SvdEtc!
    // move the connectors first, everything else afterwards
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        if (pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
    }
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        if (!pObj->IsEdgeObj()) pObj->Mirror(rRef1,rRef2);
    }
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}


void SdrObjGroup::Shear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    if (nAngle!=0) {
        SetGlueReallyAbsolute(true);
        Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        ShearPoint(aRefPoint,rRef,tn);
        // move the connectors first, everything else afterwards
        SdrObjList* pOL=pSub;
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            if (pObj->IsEdgeObj()) pObj->Shear(rRef,nAngle,tn,bVShear);
        }
        for (size_t i=0; i<nObjCount; ++i) {
            SdrObject* pObj=pOL->GetObj(i);
            if (!pObj->IsEdgeObj()) pObj->Shear(rRef,nAngle,tn,bVShear);
        }
        NbcShearGluePoints(rRef,nAngle,tn,bVShear);
        SetGlueReallyAbsolute(false);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}


void SdrObjGroup::SetAnchorPos(const Point& rPnt)
{
    Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    bool bChg=aAnchor!=rPnt;
    aAnchor=rPnt;
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    MovePoint(aRefPoint,aSiz);
    // move the connectors first, everything else afterwards
    SdrObjList* pOL=pSub;
    const size_t nObjCount = pOL->GetObjCount();
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj=pOL->GetObj(i);
        if (pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
    }
    for (size_t i=0; i<nObjCount; ++i) {
        SdrObject* pObj = pOL->GetObj(i);
        if (!pObj->IsEdgeObj()) pObj->SetAnchorPos(rPnt);
    }
    if (bChg) {
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}



void SdrObjGroup::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // this also calls SetRectsDirty()
}

void SdrObjGroup::SetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    if (aSiz.Width()!=0 || aSiz.Height()!=0) Move(aSiz); // this also calls SetRectsDirty() and Broadcast, ...
}

void SdrObjGroup::NbcReformatText()
{
    pSub->NbcReformatAllTextObjects();
}

void SdrObjGroup::ReformatText()
{
    pSub->ReformatAllTextObjects();
}

SdrObject* SdrObjGroup::DoConvertToPolyObj(bool bBezier, bool bAddText) const
{
    SdrObject* pGroup = new SdrObjGroup;
    pGroup->SetModel(GetModel());

    for(size_t a=0; a<pSub->GetObjCount(); ++a)
    {
        SdrObject* pIterObj = pSub->GetObj(a);
        SdrObject* pResult = pIterObj->DoConvertToPolyObj(bBezier, bAddText);

        // pResult can be NULL e.g. for empty objects
        if( pResult )
            pGroup->GetSubList()->NbcInsertObject(pResult);
    }

    return pGroup;
}

void SdrObjGroup::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sdrObjGroup"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    SdrObject::dumpAsXml(pWriter);

    pSub->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
