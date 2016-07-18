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

#include <cassert>

#include <svx/svdpage.hxx>

// HACK
#include <sot/storage.hxx>
#include <comphelper/classids.hxx>
#include <svx/svdview.hxx>
#include <string.h>
#include <vcl/svapp.hxx>

#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>

#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdoole2.hxx>
#include "svx/svditer.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/fmglob.hxx>
#include <svx/polysc3d.hxx>

#include <svx/fmdpage.hxx>

#include <sfx2/objsh.hxx>
#include <sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <algorithm>
#include <svl/smplhint.hxx>
#include <rtl/strbuf.hxx>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

class SdrObjList::WeakSdrObjectContainerType
    : public ::std::vector<SdrObjectWeakRef>
{
public:
    explicit WeakSdrObjectContainerType (const sal_Int32 nInitialSize)
        : ::std::vector<SdrObjectWeakRef>(nInitialSize) {};
};


static const sal_Int32 InitialObjectContainerCapacity (64);


SdrObjList::SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList):
    maList(),
    mxNavigationOrder(),
    mbIsNavigationOrderDirty(false)
{
    maList.reserve(InitialObjectContainerCapacity);
    pModel=pNewModel;
    pPage=pNewPage;
    pUpList=pNewUpList;
    bObjOrdNumsDirty=false;
    bRectsDirty=false;
    pOwnerObj=nullptr;
    eListKind=SDROBJLIST_UNKNOWN;
}

SdrObjList::SdrObjList():
    maList(),
    mxNavigationOrder(),
    mbIsNavigationOrderDirty(false)
{
    maList.reserve(InitialObjectContainerCapacity);
    pModel=nullptr;
    pPage=nullptr;
    pUpList=nullptr;
    bObjOrdNumsDirty=false;
    bRectsDirty=false;
    pOwnerObj=nullptr;
    eListKind=SDROBJLIST_UNKNOWN;
}

SdrObjList::~SdrObjList()
{

    // To avoid that the Clear() method will broadcast changes when in destruction
    // which would call virtual methos (not allowed in destructor), the model is set
    // to NULL here.
    pModel = nullptr;

    Clear(); // delete contents of container
}

SdrObjList* SdrObjList::Clone() const
{
    SdrObjList* const pObjList = new SdrObjList();
    pObjList->lateInit(*this);
    return pObjList;
}

void SdrObjList::lateInit(const SdrObjList& rSrcList)
{
    // this function is only supposed to be called once, right after construction
    assert(maList.empty());

    eListKind=rSrcList.eListKind;
    CopyObjects(rSrcList);
}

void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
{
    Clear();
    bObjOrdNumsDirty=false;
    bRectsDirty     =false;
    size_t nCloneErrCnt = 0;
    const size_t nCount = rSrcList.GetObjCount();
    SdrInsertReason aReason(SDRREASON_COPY);
    for (size_t no=0; no<nCount; ++no) {
        SdrObject* pSO=rSrcList.GetObj(no);

        SdrObject* pDO = pSO->Clone();

        if (pDO!=nullptr) {
            pDO->SetModel(pModel);
            pDO->SetPage(pPage);
            NbcInsertObject(pDO, SAL_MAX_SIZE, &aReason);
        } else {
            nCloneErrCnt++;
        }
    }

    // and now for the Connectors
    // The new objects would be shown in the rSrcList
    // and then the object connections are made.
    // Similar implementation are setup as the following:
    //    void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
    //    SdrModel* SdrExchangeView::GetMarkedObjModel() const
    //    BOOL SdrExchangeView::Paste(const SdrModel& rMod,...)
    //    void SdrEditView::CopyMarked()
    if (nCloneErrCnt==0) {
        for (size_t no=0; no<nCount; ++no) {
            const SdrObject* pSrcOb=rSrcList.GetObj(no);
            const SdrEdgeObj* pSrcEdge=dynamic_cast<const SdrEdgeObj*>( pSrcOb );
            if (pSrcEdge!=nullptr) {
                SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(true);
                SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(false);
                if (pSrcNode1!=nullptr && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=nullptr; // can't do this
                if (pSrcNode2!=nullptr && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=nullptr; // across all lists (yet)
                if (pSrcNode1!=nullptr || pSrcNode2!=nullptr) {
                    SdrObject* pEdgeObjTmp=GetObj(no);
                    SdrEdgeObj* pDstEdge=dynamic_cast<SdrEdgeObj*>( pEdgeObjTmp );
                    if (pDstEdge!=nullptr) {
                        if (pSrcNode1!=nullptr) {
                            sal_uIntPtr nDstNode1=pSrcNode1->GetOrdNum();
                            SdrObject* pDstNode1=GetObj(nDstNode1);
                            if (pDstNode1!=nullptr) { // else we get an error!
                                pDstEdge->ConnectToNode(true,pDstNode1);
                            } else {
                                OSL_FAIL("SdrObjList::operator=(): pDstNode1==NULL!");
                            }
                        }
                        if (pSrcNode2!=nullptr) {
                            sal_uIntPtr nDstNode2=pSrcNode2->GetOrdNum();
                            SdrObject* pDstNode2=GetObj(nDstNode2);
                            if (pDstNode2!=nullptr) { // else the node was probably not selected
                                pDstEdge->ConnectToNode(false,pDstNode2);
                            } else {
                                OSL_FAIL("SdrObjList::operator=(): pDstNode2==NULL!");
                            }
                        }
                    } else {
                        OSL_FAIL("SdrObjList::operator=(): pDstEdge==NULL!");
                    }
                }
            }
        }
    } else {
#ifdef DBG_UTIL
        OStringBuffer aStr("SdrObjList::operator=(): Error when cloning ");

        if(nCloneErrCnt == 1)
        {
            aStr.append("a drawing object.");
        }
        else
        {
            aStr.append(static_cast<sal_Int32>(nCloneErrCnt));
            aStr.append(" drawing objects.");
        }

        aStr.append(" Not copying connectors.");

        OSL_FAIL(aStr.getStr());
#endif
    }
}

void SdrObjList::Clear()
{
    bool bObjectsRemoved(false);

    while( ! maList.empty())
    {
        // remove last object from list
        SdrObject* pObj = maList.back();
        RemoveObjectFromContainer(maList.size()-1);

        // flushViewObjectContacts() is done since SdrObject::Free is not guaranteed
        // to delete the object and thus refresh visualisations
        pObj->GetViewContact().flushViewObjectContacts();

        bObjectsRemoved = true;

        // sent remove hint (after removal, see RemoveObject())
        if(pModel)
        {
            SdrHint aHint(*pObj);
            aHint.SetKind(HINT_OBJREMOVED);
            aHint.SetPage(pPage);
            pModel->Broadcast(aHint);
        }

        // delete the object itself
        SdrObject::Free( pObj );
    }

    if(pModel && bObjectsRemoved)
    {
        pModel->SetChanged();
    }
}

SdrPage* SdrObjList::GetPage() const
{
    return pPage;
}

void SdrObjList::SetPage(SdrPage* pNewPage)
{
    if (pPage!=pNewPage) {
        pPage=pNewPage;
        const size_t nCount = GetObjCount();
        for (size_t no=0; no<nCount; ++no) {
            SdrObject* pObj=GetObj(no);
            pObj->SetPage(pPage);
        }
    }
}

SdrModel* SdrObjList::GetModel() const
{
    return pModel;
}

void SdrObjList::SetModel(SdrModel* pNewModel)
{
    if (pModel!=pNewModel) {
        pModel=pNewModel;
        const size_t nCount = GetObjCount();
        for (size_t i=0; i<nCount; ++i) {
            SdrObject* pObj=GetObj(i);
            pObj->SetModel(pModel);
        }
    }
}

void SdrObjList::RecalcObjOrdNums()
{
    const size_t nCount = GetObjCount();
    for (size_t no=0; no<nCount; ++no) {
        SdrObject* pObj=GetObj(no);
        pObj->SetOrdNum(no);
    }
    bObjOrdNumsDirty=false;
}

void SdrObjList::RecalcRects()
{
    aOutRect=Rectangle();
    aSnapRect=aOutRect;
    const size_t nCount = GetObjCount();
    for (size_t i=0; i<nCount; ++i) {
        SdrObject* pObj=GetObj(i);
        if (i==0) {
            aOutRect=pObj->GetCurrentBoundRect();
            aSnapRect=pObj->GetSnapRect();
        } else {
            aOutRect.Union(pObj->GetCurrentBoundRect());
            aSnapRect.Union(pObj->GetSnapRect());
        }
    }
}

void SdrObjList::SetRectsDirty()
{
    bRectsDirty=true;
    if (pUpList!=nullptr) pUpList->SetRectsDirty();
}

void SdrObjList::impChildInserted(SdrObject& rChild)
{
    sdr::contact::ViewContact* pParent = rChild.GetViewContact().GetParentContact();

    if(pParent)
    {
        pParent->ActionChildInserted(rChild.GetViewContact());
    }
}

void SdrObjList::NbcInsertObject(SdrObject* pObj, size_t nPos, const SdrInsertReason* /*pReason*/)
{
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::NbcInsertObject(NULL)");
    if (pObj!=nullptr) {
        DBG_ASSERT(!pObj->IsInserted(),"ZObjekt already has the status Inserted.");
        const size_t nCount = GetObjCount();
        if (nPos>nCount) nPos=nCount;
        InsertObjectIntoContainer(*pObj,nPos);

        if (nPos<nCount) bObjOrdNumsDirty=true;
        pObj->SetOrdNum(nPos);
        pObj->SetObjList(this);
        pObj->SetPage(pPage);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pObj);

        if (!bRectsDirty) {
            aOutRect.Union(pObj->GetCurrentBoundRect());
            aSnapRect.Union(pObj->GetSnapRect());
        }
        pObj->SetInserted(true); // calls the UserCall (among others)
    }
}

void SdrObjList::InsertObject(SdrObject* pObj, size_t nPos, const SdrInsertReason* pReason)
{
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::InsertObject(NULL)");

    if(pObj)
    {
        // if anchor is used, reset it before grouping
        if(GetOwnerObj())
        {
            const Point& rAnchorPos = pObj->GetAnchorPos();
            if(rAnchorPos.X() || rAnchorPos.Y())
                pObj->NbcSetAnchorPos(Point());
        }

        // do insert to new group
        NbcInsertObject(pObj, nPos, pReason);

        // In case the object is inserted into a group and doesn't overlap with
        // the group's other members, it needs an own repaint.
        if(pOwnerObj)
        {
            // only repaint here
            pOwnerObj->ActionChanged();
        }

        if(pModel)
        {
            // TODO: We need a different broadcast here!
            // Repaint from object number ... (heads-up: GroupObj)
            if(pObj->GetPage())
            {
                SdrHint aHint(*pObj);

                aHint.SetKind(HINT_OBJINSERTED);
                pModel->Broadcast(aHint);
            }

            pModel->SetChanged();
        }
    }
}

SdrObject* SdrObjList::NbcRemoveObject(size_t nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return nullptr;
    }

    const size_t nCount = GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=nullptr,"Could not find object to remove.");
    if (pObj!=nullptr) {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        DBG_ASSERT(pObj->IsInserted(),"ZObjekt does not have the status Inserted.");
        pObj->SetInserted(false); // Ruft u.a. den UserCall
        pObj->SetObjList(nullptr);
        pObj->SetPage(nullptr);
        if (!bObjOrdNumsDirty) { // optimizing for the case that the last object has to be removed
            if (nObjNum+1!=nCount) {
                bObjOrdNumsDirty=true;
            }
        }
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::RemoveObject(size_t nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return nullptr;
    }

    const size_t nCount = GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=nullptr,"Object to remove not found.");
    if(pObj)
    {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        DBG_ASSERT(pObj->IsInserted(),"ZObjekt does not have the status Inserted.");
        if (pModel!=nullptr) {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=nullptr) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
            pModel->SetChanged();
        }
        pObj->SetInserted(false); // calls, among other things, the UserCall
        pObj->SetObjList(nullptr);
        pObj->SetPage(nullptr);
        if (!bObjOrdNumsDirty) { // optimization for the case that the last object is removed
            if (nObjNum+1!=nCount) {
                bObjOrdNumsDirty=true;
            }
        }
        SetRectsDirty();

        if(pOwnerObj && !GetObjCount())
        {
            // empty group created; it needs to be repainted since it's
            // visualization changes
            pOwnerObj->ActionChanged();
        }
    }
    return pObj;
}

SdrObject* SdrObjList::NbcReplaceObject(SdrObject* pNewObj, size_t nObjNum)
{
    if (nObjNum >= maList.size() || pNewObj == nullptr)
    {
        OSL_ASSERT(nObjNum<maList.size());
        OSL_ASSERT(pNewObj!=nullptr);
        return nullptr;
    }

    SdrObject* pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::ReplaceObject: Could not find object to remove.");
    if (pObj!=nullptr) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt does not have status Inserted.");
        pObj->SetInserted(false);
        pObj->SetObjList(nullptr);
        pObj->SetPage(nullptr);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(true);
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::ReplaceObject(SdrObject* pNewObj, size_t nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return nullptr;
    }
    if (pNewObj == nullptr)
    {
        OSL_ASSERT(pNewObj!=nullptr);
        return nullptr;
    }

    SdrObject* pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::ReplaceObject: Could not find object to remove.");
    if (pObj!=nullptr) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt does not have status Inserted.");
        if (pModel!=nullptr) {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=nullptr) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
        }
        pObj->SetInserted(false);
        pObj->SetObjList(nullptr);
        pObj->SetPage(nullptr);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(true);
        if (pModel!=nullptr) {
            // TODO: We need a different broadcast here.
            if (pNewObj->GetPage()!=nullptr) {
                SdrHint aHint(*pNewObj);
                aHint.SetKind(HINT_OBJINSERTED);
                pModel->Broadcast(aHint);
            }
            pModel->SetChanged();
        }
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::SetObjectOrdNum(size_t nOldObjNum, size_t nNewObjNum)
{
    if (nOldObjNum >= maList.size() || nNewObjNum >= maList.size())
    {
        OSL_ASSERT(nOldObjNum<maList.size());
        OSL_ASSERT(nNewObjNum<maList.size());
        return nullptr;
    }

    SdrObject* pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::SetObjectOrdNum: Object not found.");
    if (pObj!=nullptr) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: ZObjekt does not have status Inserted.");
        RemoveObjectFromContainer(nOldObjNum);
        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=true;
        if (pModel!=nullptr)
        {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=nullptr) pModel->Broadcast(SdrHint(*pObj));
            pModel->SetChanged();
        }
    }
    return pObj;
}

const Rectangle& SdrObjList::GetAllObjSnapRect() const
{
    if (bRectsDirty) {
        const_cast<SdrObjList*>(this)->RecalcRects();
        const_cast<SdrObjList*>(this)->bRectsDirty=false;
    }
    return aSnapRect;
}

const Rectangle& SdrObjList::GetAllObjBoundRect() const
{
    // #i106183# for deep group hierarchies like in chart2, the invalidates
    // through the hierarchy are not correct; use a 2nd hint for the needed
    // recalculation. Future versions will have no bool flag at all, but
    // just aOutRect in empty state to represent an invalid state, thus
    // it's a step in the right direction.
    if (bRectsDirty || aOutRect.IsEmpty())
    {
        const_cast<SdrObjList*>(this)->RecalcRects();
        const_cast<SdrObjList*>(this)->bRectsDirty=false;
    }
    return aOutRect;
}

void SdrObjList::NbcReformatAllTextObjects()
{
    size_t nCount=GetObjCount();
    size_t nNum=0;

    while (nNum<nCount)
    {
        SdrObject* pObj = GetObj(nNum);

        pObj->NbcReformatText();
        nCount=GetObjCount();             // ReformatText may delete an object
        nNum++;
    }

}

void SdrObjList::ReformatAllTextObjects()
{
    NbcReformatAllTextObjects();
}

/** steps over all available objects and reformats all
    edge objects that are connected to other objects so that
    they may reposition themselves.
*/
void SdrObjList::ReformatAllEdgeObjects()
{
    // #i120437# go over whole hierarchy, not only over object level null (seen from grouping)
    SdrObjListIter aIter(*this, IM_DEEPNOGROUPS);

    while(aIter.IsMore())
    {
        SdrEdgeObj* pSdrEdgeObj = dynamic_cast< SdrEdgeObj* >(aIter.Next());

        if(pSdrEdgeObj)
        {
            pSdrEdgeObj->Reformat();
        }
    }
}

void SdrObjList::BurnInStyleSheetAttributes()
{
    for(size_t a = 0; a < GetObjCount(); ++a)
    {
        GetObj(a)->BurnInStyleSheetAttributes();
    }
}

size_t SdrObjList::GetObjCount() const
{
    return maList.size();
}


SdrObject* SdrObjList::GetObj(size_t nNum) const
{
    if (nNum >= maList.size())
    {
        OSL_ASSERT(nNum<maList.size());
        return nullptr;
    }
    else
        return maList[nNum];
}


bool SdrObjList::IsReadOnly() const
{
    bool bRet = false;
    if (pPage!=nullptr && pPage!=this) bRet=pPage->IsReadOnly();
    return bRet;
}

size_t SdrObjList::CountAllObjects() const
{
    const size_t nCount=GetObjCount();
    size_t nCnt=nCount;
    for (size_t nNum=0; nNum<nCount; nNum++) {
        SdrObjList* pSubOL=GetObj(nNum)->GetSubList();
        if (pSubOL!=nullptr) {
            nCnt+=pSubOL->CountAllObjects();
        }
    }
    return nCnt;
}

void SdrObjList::FlattenGroups()
{
    const size_t nObj = GetObjCount();
    for( size_t i = nObj; i>0; )
        UnGroupObj(--i);
}

void SdrObjList::UnGroupObj( size_t nObjNum )
{
    // if the given object is no group, this method is a noop
    SdrObject* pUngroupObj = GetObj( nObjNum );
    if( pUngroupObj )
    {
        SdrObjList* pSrcLst = pUngroupObj->GetSubList();
        if( dynamic_cast<const SdrObjGroup*>( pUngroupObj) !=  nullptr && pSrcLst )
        {
            SdrObjGroup* pUngroupGroup = static_cast< SdrObjGroup* > (pUngroupObj);

            // ungroup recursively (has to be head recursion,
            // otherwise our indices will get trashed when doing it in
            // the loop)
            pSrcLst->FlattenGroups();

            // the position at which we insert the members of rUngroupGroup
            size_t nInsertPos( pUngroupGroup->GetOrdNum() );

            const size_t nCount = pSrcLst->GetObjCount();
            for( size_t i=0; i<nCount; ++i )
            {
                SdrObject* pObj = pSrcLst->RemoveObject(0);
                SdrInsertReason aReason(SDRREASON_VIEWCALL);
                InsertObject(pObj, nInsertPos, &aReason);
                ++nInsertPos;
            }

            RemoveObject(nInsertPos);
        }
    }
#ifdef DBG_UTIL
    else
        OSL_FAIL("SdrObjList::UnGroupObj: object index invalid");
#endif
}


bool SdrObjList::HasObjectNavigationOrder() const
{
    return mxNavigationOrder.get() != nullptr;
}


void SdrObjList::SetObjectNavigationPosition (
    SdrObject& rObject,
    const sal_uInt32 nNewPosition)
{
    // When the navigation order container has not yet been created then
    // create one now.  It is initialized with the z-order taken from
    // maList.
    if (mxNavigationOrder.get() == nullptr)
    {
        mxNavigationOrder.reset(new WeakSdrObjectContainerType(maList.size()));
        ::std::copy(
            maList.begin(),
            maList.end(),
            mxNavigationOrder->begin());
    }
    OSL_ASSERT(mxNavigationOrder.get()!=nullptr);
    OSL_ASSERT( mxNavigationOrder->size() == maList.size());

    SdrObjectWeakRef aReference (&rObject);

    // Look up the object whose navigation position is to be changed.
    WeakSdrObjectContainerType::iterator iObject (::std::find(
        mxNavigationOrder->begin(),
        mxNavigationOrder->end(),
        aReference));
    if (iObject == mxNavigationOrder->end())
    {
        // The given object is not a member of the navigation order.
        return;
    }

    // Move the object to its new position.
    const sal_uInt32 nOldPosition = ::std::distance(mxNavigationOrder->begin(), iObject);
    if (nOldPosition != nNewPosition)
    {
        mxNavigationOrder->erase(iObject);
        sal_uInt32 nInsertPosition (nNewPosition);
        // Adapt insertion position for the just erased object.
        if (nNewPosition >= nOldPosition)
            nInsertPosition -= 1;
        if (nInsertPosition >= mxNavigationOrder->size())
            mxNavigationOrder->push_back(aReference);
        else
            mxNavigationOrder->insert(mxNavigationOrder->begin()+nInsertPosition, aReference);

        mbIsNavigationOrderDirty = true;

        // The navigation order is written out to file so mark the model as modified.
        if (pModel != nullptr)
            pModel->SetChanged();
    }
}


SdrObject* SdrObjList::GetObjectForNavigationPosition (const sal_uInt32 nNavigationPosition) const
{
    if (HasObjectNavigationOrder())
    {
        // There is a user defined navigation order. Make sure the object
        // index is correct and look up the object in mxNavigationOrder.
        if (nNavigationPosition >= mxNavigationOrder->size())
        {
            OSL_ASSERT(nNavigationPosition < mxNavigationOrder->size());
        }
        else
            return (*mxNavigationOrder)[nNavigationPosition].get();
    }
    else
    {
        // There is no user defined navigation order. Use the z-order
        // instead.
        if (nNavigationPosition >= maList.size())
        {
            OSL_ASSERT(nNavigationPosition < maList.size());
        }
        else
            return maList[nNavigationPosition];
    }
    return nullptr;
}


void SdrObjList::ClearObjectNavigationOrder()
{
    mxNavigationOrder.reset();
    mbIsNavigationOrderDirty = true;
}


bool SdrObjList::RecalcNavigationPositions()
{
    if (mbIsNavigationOrderDirty)
    {
        if (mxNavigationOrder.get() != nullptr)
        {
            mbIsNavigationOrderDirty = false;

            WeakSdrObjectContainerType::iterator iObject;
            WeakSdrObjectContainerType::const_iterator iEnd (mxNavigationOrder->end());
            sal_uInt32 nIndex (0);
            for (iObject=mxNavigationOrder->begin(); iObject!=iEnd; ++iObject,++nIndex)
                (*iObject)->SetNavigationPosition(nIndex);
        }
    }

    return mxNavigationOrder.get() != nullptr;
}


void SdrObjList::SetNavigationOrder (const uno::Reference<container::XIndexAccess>& rxOrder)
{
    if (rxOrder.is())
    {
        const sal_Int32 nCount = rxOrder->getCount();
        if ((sal_uInt32)nCount != maList.size())
            return;

        if (mxNavigationOrder.get() == nullptr)
            mxNavigationOrder.reset(new WeakSdrObjectContainerType(nCount));

        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            uno::Reference<uno::XInterface> xShape (rxOrder->getByIndex(nIndex), uno::UNO_QUERY);
            SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
            if (pObject == nullptr)
                break;
            (*mxNavigationOrder)[nIndex] = pObject;
        }

        mbIsNavigationOrderDirty = true;
    }
    else
        ClearObjectNavigationOrder();
}


void SdrObjList::InsertObjectIntoContainer (
    SdrObject& rObject,
    const sal_uInt32 nInsertPosition)
{
    OSL_ASSERT(nInsertPosition<=maList.size());

    // Update the navigation positions.
    if (HasObjectNavigationOrder())
    {
        // The new object does not have a user defined position so append it
        // to the list.
        rObject.SetNavigationPosition(mxNavigationOrder->size());
        mxNavigationOrder->push_back(&rObject);
    }

    // Insert object into object list.  Because the insert() method requires
    // a valid iterator as insertion position, we have to use push_back() to
    // insert at the end of the list.
    if (nInsertPosition >= maList.size())
        maList.push_back(&rObject);
    else
        maList.insert(maList.begin()+nInsertPosition, &rObject);
    bObjOrdNumsDirty=true;
}


void SdrObjList::ReplaceObjectInContainer (
    SdrObject& rNewObject,
    const sal_uInt32 nObjectPosition)
{
    if (nObjectPosition >= maList.size())
    {
        OSL_ASSERT(nObjectPosition<maList.size());
        return;
    }

    // Update the navigation positions.
    if (HasObjectNavigationOrder())
    {
        // A user defined position of the object that is to be replaced is
        // not transferred to the new object so erase the former and append
        // the later object from/to the navigation order.
        OSL_ASSERT(nObjectPosition < maList.size());
        SdrObjectWeakRef aReference (maList[nObjectPosition]);
        WeakSdrObjectContainerType::iterator iObject (::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference));
        if (iObject != mxNavigationOrder->end())
            mxNavigationOrder->erase(iObject);

        mxNavigationOrder->push_back(&rNewObject);

        mbIsNavigationOrderDirty = true;
    }

    maList[nObjectPosition] = &rNewObject;
    bObjOrdNumsDirty=true;
}


void SdrObjList::RemoveObjectFromContainer (
    const sal_uInt32 nObjectPosition)
{
    if (nObjectPosition >= maList.size())
    {
        OSL_ASSERT(nObjectPosition<maList.size());
        return;
    }

    // Update the navigation positions.
    if (HasObjectNavigationOrder())
    {
        SdrObjectWeakRef aReference (maList[nObjectPosition]);
        WeakSdrObjectContainerType::iterator iObject (::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference));
        if (iObject != mxNavigationOrder->end())
            mxNavigationOrder->erase(iObject);
        mbIsNavigationOrderDirty = true;
    }

    maList.erase(maList.begin()+nObjectPosition);
    bObjOrdNumsDirty=true;
}

void SdrObjList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("sdrObjList"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));

    size_t nObjCount = GetObjCount();
    for (size_t i = 0; i < nObjCount; ++i)
    {
        if (const SdrObject* pObject = GetObj(i))
            pObject->dumpAsXml(pWriter);
    }

    xmlTextWriterEndElement(pWriter);
}


void SdrPageGridFrameList::Clear()
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        delete GetObject(i);
    }
    aList.clear();
}


// PageUser section

void SdrPage::AddPageUser(sdr::PageUser& rNewUser)
{
    maPageUsers.push_back(&rNewUser);
}

void SdrPage::RemovePageUser(sdr::PageUser& rOldUser)
{
    const sdr::PageUserVector::iterator aFindResult = ::std::find(maPageUsers.begin(), maPageUsers.end(), &rOldUser);
    if(aFindResult != maPageUsers.end())
    {
        maPageUsers.erase(aFindResult);
    }
}


// DrawContact section

sdr::contact::ViewContact* SdrPage::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrPage(*this);
}

const sdr::contact::ViewContact& SdrPage::GetViewContact() const
{
    if (!mpViewContact)
        const_cast<SdrPage*>(this)->mpViewContact.reset(
            const_cast<SdrPage*>(this)->CreateObjectSpecificViewContact());

    return *mpViewContact;
}

sdr::contact::ViewContact& SdrPage::GetViewContact()
{
    if (!mpViewContact)
        mpViewContact.reset(CreateObjectSpecificViewContact());

    return *mpViewContact;
}

void SdrPageProperties::ImpRemoveStyleSheet()
{
    if(mpStyleSheet)
    {
        EndListening(*mpStyleSheet);
        mpProperties->SetParent(nullptr);
        mpStyleSheet = nullptr;
    }
}

void SdrPageProperties::ImpAddStyleSheet(SfxStyleSheet& rNewStyleSheet)
{
    if(mpStyleSheet != &rNewStyleSheet)
    {
        ImpRemoveStyleSheet();
        mpStyleSheet = &rNewStyleSheet;
        StartListening(rNewStyleSheet);
        mpProperties->SetParent(&rNewStyleSheet.GetItemSet());
    }
}

void ImpPageChange(SdrPage& rSdrPage)
{
    rSdrPage.ActionChanged();

    if(rSdrPage.GetModel())
    {
        rSdrPage.GetModel()->SetChanged();
        SdrHint aHint(HINT_PAGEORDERCHG);
        aHint.SetPage(&rSdrPage);
        rSdrPage.GetModel()->Broadcast(aHint);
    }
}

SdrPageProperties::SdrPageProperties(SdrPage& rSdrPage)
:   SfxListener(),
    mpSdrPage(&rSdrPage),
    mpStyleSheet(nullptr),
    mpProperties(new SfxItemSet(mpSdrPage->GetModel()->GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST))
{
    if(!rSdrPage.IsMasterPage())
    {
        mpProperties->Put(XFillStyleItem(drawing::FillStyle_NONE));
    }
}

SdrPageProperties::~SdrPageProperties()
{
    ImpRemoveStyleSheet();
    delete mpProperties;
}

void SdrPageProperties::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if(pSimpleHint)
    {
        switch(pSimpleHint->GetId())
        {
            case SFX_HINT_DATACHANGED :
            {
                // notify change, broadcast
                ImpPageChange(*mpSdrPage);
                break;
            }
            case SFX_HINT_DYING :
            {
                // Style needs to be forgotten
                ImpRemoveStyleSheet();
                break;
            }
        }
    }
}

bool SdrPageProperties::isUsedByModel() const
{
    assert(mpSdrPage);
    return mpSdrPage->IsInserted();
}


void SdrPageProperties::PutItemSet(const SfxItemSet& rSet)
{
    OSL_ENSURE(!mpSdrPage->IsMasterPage(), "Item set at MasterPage Attributes (!)");
    mpProperties->Put(rSet);
    ImpPageChange(*mpSdrPage);
}

void SdrPageProperties::PutItem(const SfxPoolItem& rItem)
{
    OSL_ENSURE(!mpSdrPage->IsMasterPage(), "Item set at MasterPage Attributes (!)");
    mpProperties->Put(rItem);
    ImpPageChange(*mpSdrPage);
}

void SdrPageProperties::ClearItem(const sal_uInt16 nWhich)
{
    mpProperties->ClearItem(nWhich);
    ImpPageChange(*mpSdrPage);
}

void SdrPageProperties::SetStyleSheet(SfxStyleSheet* pStyleSheet)
{
    if(pStyleSheet)
    {
        ImpAddStyleSheet(*pStyleSheet);
    }
    else
    {
        ImpRemoveStyleSheet();
    }

    ImpPageChange(*mpSdrPage);
}


SdrPage::SdrPage(SdrModel& rNewModel, bool bMasterPage)
:   SdrObjList(&rNewModel, this),
    mpViewContact(nullptr),
    nWdt(10L),
    nHgt(10L),
    nBordLft(0L),
    nBordUpp(0L),
    nBordRgt(0L),
    nBordLwr(0L),
    mpLayerAdmin(new SdrLayerAdmin(&rNewModel.GetLayerAdmin())),
    mpSdrPageProperties(nullptr),
    mpMasterPageDescriptor(nullptr),
    nPageNum(0L),
    mbMaster(bMasterPage),
    mbInserted(false),
    mbObjectsNotPersistent(false),
    mbPageBorderOnlyLeftRight(false)
{
    aPrefVisiLayers.SetAll();
    eListKind = (bMasterPage) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;

    mpSdrPageProperties.reset(new SdrPageProperties(*this));
}

SdrPage::SdrPage(const SdrPage& rSrcPage)
:   SdrObjList(rSrcPage.pModel, this),
    tools::WeakBase< SdrPage >(),
    mpViewContact(nullptr),
    nWdt(rSrcPage.nWdt),
    nHgt(rSrcPage.nHgt),
    nBordLft(rSrcPage.nBordLft),
    nBordUpp(rSrcPage.nBordUpp),
    nBordRgt(rSrcPage.nBordRgt),
    nBordLwr(rSrcPage.nBordLwr),
    mpLayerAdmin(new SdrLayerAdmin(rSrcPage.pModel->GetLayerAdmin())),
    mpSdrPageProperties(nullptr),
    mpMasterPageDescriptor(nullptr),
    nPageNum(rSrcPage.nPageNum),
    mbMaster(rSrcPage.mbMaster),
    mbInserted(false),
    mbObjectsNotPersistent(rSrcPage.mbObjectsNotPersistent),
    mbPageBorderOnlyLeftRight(rSrcPage.mbPageBorderOnlyLeftRight)
{
    aPrefVisiLayers.SetAll();
}

SdrPage::~SdrPage()
{
    if( mxUnoPage.is() ) try
    {
        uno::Reference< lang::XComponent > xPageComponent( mxUnoPage, uno::UNO_QUERY_THROW );
        mxUnoPage.clear();
        xPageComponent->dispose();
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // tell all the registered PageUsers that the page is in destruction
    // This causes some (all?) PageUsers to remove themselves from the list
    // of page users.  Therefore we have to use a copy of the list for the
    // iteration.
    sdr::PageUserVector aListCopy (maPageUsers.begin(), maPageUsers.end());
    for(sdr::PageUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); ++aIterator)
    {
        sdr::PageUser* pPageUser = *aIterator;
        DBG_ASSERT(pPageUser, "SdrPage::~SdrPage: corrupt PageUser list (!)");
        pPageUser->PageInDestruction(*this);
    }

    // Clear the vector. This means that user do not need to call RemovePageUser()
    // when they get called from PageInDestruction().
    maPageUsers.clear();

    mpLayerAdmin.reset();

    TRG_ClearMasterPage();

    mpViewContact.reset();
    mpSdrPageProperties.reset();
}

void SdrPage::lateInit(const SdrPage& rSrcPage, SdrModel* const pNewModel)
{
    assert(!mpViewContact);
    assert(!mpSdrPageProperties);
    assert(!mxUnoPage.is());

    if (pNewModel && (pNewModel != pModel))
    {
        pModel = pNewModel;
        impl_setModelForLayerAdmin(pNewModel);
    }

    // copy all the local parameters to make this instance
    // a valid copy of source page before copying and inserting
    // the contained objects
    mbMaster = rSrcPage.mbMaster;
    mbPageBorderOnlyLeftRight = rSrcPage.mbPageBorderOnlyLeftRight;
    aPrefVisiLayers = rSrcPage.aPrefVisiLayers;
    nWdt = rSrcPage.nWdt;
    nHgt = rSrcPage.nHgt;
    nBordLft = rSrcPage.nBordLft;
    nBordUpp = rSrcPage.nBordUpp;
    nBordRgt = rSrcPage.nBordRgt;
    nBordLwr = rSrcPage.nBordLwr;
    nPageNum = rSrcPage.nPageNum;

    if(rSrcPage.TRG_HasMasterPage())
    {
        TRG_SetMasterPage(rSrcPage.TRG_GetMasterPage());
        TRG_SetMasterPageVisibleLayers(rSrcPage.TRG_GetMasterPageVisibleLayers());
    }
    else
    {
        TRG_ClearMasterPage();
    }

    mbObjectsNotPersistent = rSrcPage.mbObjectsNotPersistent;

    {
        mpSdrPageProperties.reset(new SdrPageProperties(*this));

        if(!IsMasterPage())
        {
            mpSdrPageProperties->PutItemSet(rSrcPage.getSdrPageProperties().GetItemSet());
        }

        mpSdrPageProperties->SetStyleSheet(rSrcPage.getSdrPageProperties().GetStyleSheet());
    }

    // Now copy the contained objects
    SdrObjList::lateInit(rSrcPage);

    // be careful and correct eListKind, a member of SdrObjList which
    // will be changed by the SdrObjList::lateInit before...
    eListKind = (mbMaster) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;
}

SdrPage* SdrPage::Clone() const
{
    return Clone(nullptr);
}

SdrPage* SdrPage::Clone(SdrModel* pNewModel) const
{
    if (pNewModel==nullptr) pNewModel=pModel;
    SdrPage* pPage2=new SdrPage(*pNewModel);
    pPage2->lateInit(*this);
    return pPage2;
}

void SdrPage::SetSize(const Size& aSiz)
{
    bool bChanged(false);

    if(aSiz.Width() != nWdt)
    {
        nWdt = aSiz.Width();
        bChanged = true;
    }

    if(aSiz.Height() != nHgt)
    {
        nHgt = aSiz.Height();
        bChanged = true;
    }

    if(bChanged)
    {
        SetChanged();
    }
}

Size SdrPage::GetSize() const
{
    return Size(nWdt,nHgt);
}

sal_Int32 SdrPage::GetWdt() const
{
    return nWdt;
}

void SdrPage::SetOrientation(Orientation eOri)
{
    // square: handle like portrait format
    Size aSiz(GetSize());
    if (aSiz.Width()!=aSiz.Height()) {
        if ((eOri==ORIENTATION_PORTRAIT) == (aSiz.Width()>aSiz.Height())) {
            SetSize(Size(aSiz.Height(),aSiz.Width()));
        }
    }
}

Orientation SdrPage::GetOrientation() const
{
    // square: handle like portrait format
    Orientation eRet=ORIENTATION_PORTRAIT;
    Size aSiz(GetSize());
    if (aSiz.Width()>aSiz.Height()) eRet=ORIENTATION_LANDSCAPE;
    return eRet;
}

sal_Int32 SdrPage::GetHgt() const
{
    return nHgt;
}

void  SdrPage::SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 nLwr)
{
    bool bChanged(false);

    if(nBordLft != nLft)
    {
        nBordLft = nLft;
        bChanged = true;
    }

    if(nBordUpp != nUpp)
    {
        nBordUpp = nUpp;
        bChanged = true;
    }

    if(nBordRgt != nRgt)
    {
        nBordRgt = nRgt;
        bChanged = true;
    }

    if(nBordLwr != nLwr)
    {
        nBordLwr =  nLwr;
        bChanged = true;
    }

    if(bChanged)
    {
        SetChanged();
    }
}

void  SdrPage::SetLftBorder(sal_Int32 nBorder)
{
    if(nBordLft != nBorder)
    {
        nBordLft = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetUppBorder(sal_Int32 nBorder)
{
    if(nBordUpp != nBorder)
    {
        nBordUpp = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetRgtBorder(sal_Int32 nBorder)
{
    if(nBordRgt != nBorder)
    {
        nBordRgt=nBorder;
        SetChanged();
    }
}

void  SdrPage::SetLwrBorder(sal_Int32 nBorder)
{
    if(nBordLwr != nBorder)
    {
        nBordLwr=nBorder;
        SetChanged();
    }
}

sal_Int32 SdrPage::GetLftBorder() const
{
    return nBordLft;
}

sal_Int32 SdrPage::GetUppBorder() const
{
    return nBordUpp;
}

sal_Int32 SdrPage::GetRgtBorder() const
{
    return nBordRgt;
}

sal_Int32 SdrPage::GetLwrBorder() const
{
    return nBordLwr;
}

void SdrPage::impl_setModelForLayerAdmin(SdrModel* const pNewModel)
{
    if (pNewModel!=nullptr) {
        mpLayerAdmin->SetParent(&pNewModel->GetLayerAdmin());
    } else {
        mpLayerAdmin->SetParent(nullptr);
    }
    mpLayerAdmin->SetModel(pNewModel);
}

void SdrPage::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel=pModel;
    SdrObjList::SetModel(pNewModel);

    if (pNewModel!=pOldModel)
    {
        impl_setModelForLayerAdmin( pNewModel );

        // create new SdrPageProperties with new model (due to SfxItemSet there)
        // and copy ItemSet and StyleSheet
        std::unique_ptr<SdrPageProperties> pNew(new SdrPageProperties(*this));

        if(!IsMasterPage())
        {
            pNew->PutItemSet(getSdrPageProperties().GetItemSet());
        }

        pNew->SetStyleSheet(getSdrPageProperties().GetStyleSheet());

        mpSdrPageProperties = std::move(pNew);
    }

    // update listeners at possible API wrapper object
    if( pOldModel != pNewModel )
    {
        if( mxUnoPage.is() )
        {
            SvxDrawPage* pPage2 = SvxDrawPage::getImplementation( mxUnoPage );
            if( pPage2 )
                pPage2->ChangeModel( pNewModel );
        }
    }
}


// #i68775# React on PageNum changes (from Model in most cases)
void SdrPage::SetPageNum(sal_uInt16 nNew)
{
    if(nNew != nPageNum)
    {
        // change
        nPageNum = nNew;

        // notify visualisations, also notifies e.g. buffered MasterPages
        ActionChanged();
    }
}

sal_uInt16 SdrPage::GetPageNum() const
{
    if (!mbInserted)
        return 0;

    if (mbMaster) {
        if (pModel && pModel->IsMPgNumsDirty())
            pModel->RecalcPageNums(true);
    } else {
        if (pModel && pModel->IsPagNumsDirty())
            pModel->RecalcPageNums(false);
    }
    return nPageNum;
}

void SdrPage::SetChanged()
{
    // For test purposes, use the new ViewContact for change
    // notification now.
    ActionChanged();

    if( pModel )
    {
        pModel->SetChanged();
    }
}


// MasterPage interface

void SdrPage::TRG_SetMasterPage(SdrPage& rNew)
{
    if(mpMasterPageDescriptor && &(mpMasterPageDescriptor->GetUsedPage()) == &rNew)
        return;

    if(mpMasterPageDescriptor)
        TRG_ClearMasterPage();

    mpMasterPageDescriptor = new sdr::MasterPageDescriptor(*this, rNew);
    GetViewContact().ActionChanged();
}

void SdrPage::TRG_ClearMasterPage()
{
    if(mpMasterPageDescriptor)
    {
        SetChanged();

        // the flushViewObjectContacts() will do needed invalidates by deleting the involved VOCs
        mpMasterPageDescriptor->GetUsedPage().GetViewContact().flushViewObjectContacts();

        delete mpMasterPageDescriptor;
        mpMasterPageDescriptor = nullptr;
    }
}

SdrPage& SdrPage::TRG_GetMasterPage() const
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_GetMasterPage(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetUsedPage();
}

const SetOfByte& SdrPage::TRG_GetMasterPageVisibleLayers() const
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_GetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetVisibleLayers();
}

void SdrPage::TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew)
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_SetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    mpMasterPageDescriptor->SetVisibleLayers(rNew);
}

sdr::contact::ViewContact& SdrPage::TRG_GetMasterPageDescriptorViewContact() const
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_GetMasterPageDescriptorViewContact(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetViewContact();
}

// used from SdrModel::RemoveMasterPage
void SdrPage::TRG_ImpMasterPageRemoved(const SdrPage& rRemovedPage)
{
    if(TRG_HasMasterPage())
    {
        if(&TRG_GetMasterPage() == &rRemovedPage)
        {
            TRG_ClearMasterPage();
        }
    }
}

const SdrPageGridFrameList* SdrPage::GetGridFrameList(const SdrPageView* /*pPV*/, const Rectangle* /*pRect*/) const
{
    return nullptr;
}

const SdrLayerAdmin& SdrPage::GetLayerAdmin() const
{
    return *mpLayerAdmin;
}

SdrLayerAdmin& SdrPage::GetLayerAdmin()
{
    return *mpLayerAdmin;
}

OUString SdrPage::GetLayoutName() const
{
    return OUString();
}

void SdrPage::SetInserted( bool bIns )
{
    if( (bool) mbInserted != bIns )
    {
        mbInserted = bIns;

        // #i120437# go over whole hierarchy, not only over object level null (seen from grouping)
        SdrObjListIter aIter(*this, IM_DEEPNOGROUPS);

         while ( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            if ( dynamic_cast<const SdrOle2Obj* >(pObj) !=  nullptr )
            {
                if( mbInserted )
                    static_cast<SdrOle2Obj*>(pObj)->Connect();
                else
                    static_cast<SdrOle2Obj*>(pObj)->Disconnect();
            }
        }
    }
}

void SdrPage::SetUnoPage(uno::Reference<drawing::XDrawPage> const& xNewPage)
{
    mxUnoPage = xNewPage;
}

uno::Reference< uno::XInterface > const & SdrPage::getUnoPage()
{
    if( !mxUnoPage.is() )
    {
        // create one
        mxUnoPage = createUnoPage();
    }

    return mxUnoPage;
}

uno::Reference< uno::XInterface > SdrPage::createUnoPage()
{
    css::uno::Reference< css::uno::XInterface > xInt =
        static_cast<cppu::OWeakObject*>( new SvxFmDrawPage( this ) );
    return xInt;
}

SfxStyleSheet* SdrPage::GetTextStyleSheetForObject( SdrObject* pObj ) const
{
    return pObj->GetStyleSheet();
}

/** returns an averaged background color of this page */
// #i75566# GetBackgroundColor -> GetPageBackgroundColor and bScreenDisplay hint value
Color SdrPage::GetPageBackgroundColor( SdrPageView* pView, bool bScreenDisplay ) const
{
    Color aColor;

    if(bScreenDisplay && (!pView || pView->GetApplicationDocumentColor() == COL_AUTO))
    {
        svtools::ColorConfig aColorConfig;
        aColor = aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor;
    }
    else
    {
        aColor = pView->GetApplicationDocumentColor();
    }

    const SfxItemSet* pBackgroundFill = &getSdrPageProperties().GetItemSet();

    if(!IsMasterPage() && TRG_HasMasterPage())
    {
        if(drawing::FillStyle_NONE == static_cast<const XFillStyleItem&>(pBackgroundFill->Get(XATTR_FILLSTYLE)).GetValue())
        {
            pBackgroundFill = &TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
        }
    }

    GetDraftFillColor(*pBackgroundFill, aColor);

    return aColor;
}

/** *deprecated, use GetBackgroundColor with SdrPageView */
Color SdrPage::GetPageBackgroundColor() const
// #i75566# GetBackgroundColor -> GetPageBackgroundColor
{
    return GetPageBackgroundColor( nullptr );
}

/** this method returns true if the object from the ViewObjectContact should
    be visible on this page while rendering.
    bEdit selects if visibility test is for an editing view or a final render,
    like printing.
*/
bool SdrPage::checkVisibility(
    const sdr::contact::ViewObjectContact& /*rOriginal*/,
    const sdr::contact::DisplayInfo& /*rDisplayInfo*/,
    bool /*bEdit*/)
{
    // this will be handled in the application if needed
    return true;
}

// DrawContact support: Methods for handling Page changes
void SdrPage::ActionChanged()
{
    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();

    // #i48535# also handle MasterPage change
    if(TRG_HasMasterPage())
    {
        TRG_GetMasterPageDescriptorViewContact().ActionChanged();
    }
}

SdrPageProperties& SdrPage::getSdrPageProperties()
{
    return *mpSdrPageProperties;
}

const SdrPageProperties& SdrPage::getSdrPageProperties() const
{
    return *mpSdrPageProperties;
}

const SdrPageProperties* SdrPage::getCorrectSdrPageProperties() const
{
    if(mpMasterPageDescriptor)
    {
        return mpMasterPageDescriptor->getCorrectSdrPageProperties();
    }
    else
    {
        return &getSdrPageProperties();
    }
}


// use new redirector instead of pPaintProc

StandardCheckVisisbilityRedirector::StandardCheckVisisbilityRedirector()
:   ViewObjectContactRedirector()
{
}

StandardCheckVisisbilityRedirector::~StandardCheckVisisbilityRedirector()
{
}

drawinglayer::primitive2d::Primitive2DContainer StandardCheckVisisbilityRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        if(pObject->GetPage())
        {
            if(pObject->GetPage()->checkVisibility(rOriginal, rDisplayInfo, false))
            {
                return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
            }
        }

        return drawinglayer::primitive2d::Primitive2DContainer();
    }
    else
    {
        // not an object, maybe a page
        return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
