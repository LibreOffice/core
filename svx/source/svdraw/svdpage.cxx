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

#include <memory>
#include <cassert>

#include <svx/svdpage.hxx>

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
#include <svx/svditer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/fmglob.hxx>

#include <svx/fmdpage.hxx>

#include <sfx2/objsh.hxx>
#include <sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <algorithm>
#include <svl/hint.hxx>
#include <rtl/strbuf.hxx>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

class SdrObjList::WeakSdrObjectContainerType
    : public ::std::vector<tools::WeakReference<SdrObject>>
{
public:
    explicit WeakSdrObjectContainerType (const sal_Int32 nInitialSize)
        : ::std::vector<tools::WeakReference<SdrObject>>(nInitialSize) {};
};

static const sal_Int32 InitialObjectContainerCapacity (64);

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow changing parent at SdrObject, but only from SdrObjList

void SetParentAtSdrObjectFromSdrObjList(SdrObject& rSdrObject, SdrObjList* pNew)
{
    rSdrObject.setParentOfSdrObject(pNew);
}

//////////////////////////////////////////////////////////////////////////////

SdrObjList::SdrObjList()
:   maList(),
    maSdrObjListOutRect(),
    maSdrObjListSnapRect(),
    mbObjOrdNumsDirty(false),
    mbRectsDirty(false),
    mxNavigationOrder(),
    mbIsNavigationOrderDirty(false)
{
    maList.reserve(InitialObjectContainerCapacity);
}

void SdrObjList::impClearSdrObjList(bool bBroadcast)
{
    SdrModel* pSdrModelFromRemovedSdrObject(nullptr);

    while(!maList.empty())
    {
        // remove last object from list
        SdrObject* pObj(maList.back());
        RemoveObjectFromContainer(maList.size()-1);

        // flushViewObjectContacts() is done since SdrObject::Free is not guaranteed
        // to delete the object and thus refresh visualisations
        pObj->GetViewContact().flushViewObjectContacts();

        if(bBroadcast)
        {
            if(nullptr == pSdrModelFromRemovedSdrObject)
            {
                pSdrModelFromRemovedSdrObject = &pObj->getSdrModelFromSdrObject();
            }

            // sent remove hint (after removal, see RemoveObject())
            // TTTT SdrPage not needed, can be accessed using SdrObject
            SdrHint aHint(SdrHintKind::ObjectRemoved, *pObj, getSdrPageFromSdrObjList());
            pObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        // delete the object itself
        SdrObject::Free( pObj );
    }

    if(bBroadcast && nullptr != pSdrModelFromRemovedSdrObject)
    {
        pSdrModelFromRemovedSdrObject->SetChanged();
    }
}

void SdrObjList::ClearSdrObjList()
{
    // clear SdrObjects with broadcasting
    impClearSdrObjList(true);
}

SdrObjList::~SdrObjList()
{
    // clear SdrObjects without broadcasting
    impClearSdrObjList(false);
}

SdrPage* SdrObjList::getSdrPageFromSdrObjList() const
{
    // default is no page and returns zero
    return nullptr;
}

SdrObject* SdrObjList::getSdrObjectFromSdrObjList() const
{
    // default is no SdrObject (SdrObjGroup)
    return nullptr;
}

void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
{
    // clear SdrObjects with broadcasting
    ClearSdrObjList();

    mbObjOrdNumsDirty = false;
    mbRectsDirty = false;
    size_t nCloneErrCnt(0);
    const size_t nCount(rSrcList.GetObjCount());

    if(nullptr == getSdrObjectFromSdrObjList() && nullptr == getSdrPageFromSdrObjList())
    {
        OSL_ENSURE(false, "SdrObjList which is not part of SdrPage or SdrObject (!)");
        return;
    }

    SdrModel& rTargetSdrModel(nullptr == getSdrObjectFromSdrObjList()
        ? getSdrPageFromSdrObjList()->getSdrModelFromSdrPage()
        : getSdrObjectFromSdrObjList()->getSdrModelFromSdrObject());

    for (size_t no(0); no < nCount; ++no)
    {
        SdrObject* pSO(rSrcList.GetObj(no));
        SdrObject* pDO(pSO->CloneSdrObject(rTargetSdrModel));

        if(nullptr != pDO)
        {
            NbcInsertObject(pDO, SAL_MAX_SIZE);
        }
        else
        {
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
                if (pSrcNode1!=nullptr && pSrcNode1->getParentSdrObjListFromSdrObject()!=pSrcEdge->getParentSdrObjListFromSdrObject()) pSrcNode1=nullptr; // can't do this
                if (pSrcNode2!=nullptr && pSrcNode2->getParentSdrObjListFromSdrObject()!=pSrcEdge->getParentSdrObjListFromSdrObject()) pSrcNode2=nullptr; // across all lists (yet)
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

void SdrObjList::RecalcObjOrdNums()
{
    const size_t nCount = GetObjCount();
    for (size_t no=0; no<nCount; ++no) {
        SdrObject* pObj=GetObj(no);
        pObj->SetOrdNum(no);
    }
    mbObjOrdNumsDirty=false;
}

void SdrObjList::RecalcRects()
{
    maSdrObjListOutRect=tools::Rectangle();
    maSdrObjListSnapRect=maSdrObjListOutRect;
    const size_t nCount = GetObjCount();
    for (size_t i=0; i<nCount; ++i) {
        SdrObject* pObj=GetObj(i);
        if (i==0) {
            maSdrObjListOutRect=pObj->GetCurrentBoundRect();
            maSdrObjListSnapRect=pObj->GetSnapRect();
        } else {
            maSdrObjListOutRect.Union(pObj->GetCurrentBoundRect());
            maSdrObjListSnapRect.Union(pObj->GetSnapRect());
        }
    }
}

void SdrObjList::SetSdrObjListRectsDirty()
{
    mbRectsDirty=true;
    SdrObject* pParentSdrObject(getSdrObjectFromSdrObjList());

    if(nullptr != pParentSdrObject)
    {
        pParentSdrObject->SetRectsDirty();
    }
}

void SdrObjList::impChildInserted(SdrObject const & rChild)
{
    sdr::contact::ViewContact* pParent = rChild.GetViewContact().GetParentContact();

    if(pParent)
    {
        pParent->ActionChildInserted(rChild.GetViewContact());
    }
}

void SdrObjList::NbcInsertObject(SdrObject* pObj, size_t nPos)
{
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::NbcInsertObject(NULL)");
    if (pObj==nullptr)
        return;

    DBG_ASSERT(!pObj->IsInserted(),"The object already has the status Inserted.");
    const size_t nCount = GetObjCount();
    if (nPos>nCount) nPos=nCount;
    InsertObjectIntoContainer(*pObj,nPos);

    if (nPos<nCount) mbObjOrdNumsDirty=true;
    pObj->SetOrdNum(nPos);
    SetParentAtSdrObjectFromSdrObjList(*pObj, this);

    // Inform the parent about change to allow invalidations at
    // evtl. existing parent visualisations
    impChildInserted(*pObj);

    if (!mbRectsDirty) {
        maSdrObjListOutRect.Union(pObj->GetCurrentBoundRect());
        maSdrObjListSnapRect.Union(pObj->GetSnapRect());
    }
    pObj->InsertedStateChange(); // calls the UserCall (among others)
}

void SdrObjList::InsertObject(SdrObject* pObj, size_t nPos)
{
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::InsertObject(NULL)");

    if(pObj)
    {
        // if anchor is used, reset it before grouping
        if(getSdrObjectFromSdrObjList())
        {
            const Point& rAnchorPos = pObj->GetAnchorPos();
            if(rAnchorPos.X() || rAnchorPos.Y())
                pObj->NbcSetAnchorPos(Point());
        }

        // do insert to new group
        NbcInsertObject(pObj, nPos);

        // In case the object is inserted into a group and doesn't overlap with
        // the group's other members, it needs an own repaint.
        SdrObject* pParentSdrObject(getSdrObjectFromSdrObjList());

        if(pParentSdrObject)
        {
            // only repaint here
            pParentSdrObject->ActionChanged();
        }

        // TODO: We need a different broadcast here!
        // Repaint from object number ... (heads-up: GroupObj)
        if(pObj->getSdrPageFromSdrObject())
        {
            SdrHint aHint(SdrHintKind::ObjectInserted, *pObj);
            pObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        pObj->getSdrModelFromSdrObject().SetChanged();
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
    if (pObj!=nullptr)
    {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        DBG_ASSERT(pObj->IsInserted(),"The object does not have the status Inserted.");
        pObj->InsertedStateChange(); // calls UserCall, among other
        SetParentAtSdrObjectFromSdrObjList(*pObj, nullptr);
        if (!mbObjOrdNumsDirty)
        {
            // optimizing for the case that the last object has to be removed
            if (nObjNum+1!=nCount) {
                mbObjOrdNumsDirty=true;
            }
        }
        SetSdrObjListRectsDirty();
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
        DBG_ASSERT(pObj->IsInserted(),"The object does not have the status Inserted.");

        // TODO: We need a different broadcast here.
        if (pObj->getSdrPageFromSdrObject()!=nullptr)
        {
            SdrHint aHint(SdrHintKind::ObjectRemoved, *pObj);
            pObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        pObj->getSdrModelFromSdrObject().SetChanged();

        pObj->InsertedStateChange(); // calls, among other things, the UserCall
        SetParentAtSdrObjectFromSdrObjList(*pObj, nullptr);

        if (!mbObjOrdNumsDirty)
        {
            // optimization for the case that the last object is removed
            if (nObjNum+1!=nCount) {
                mbObjOrdNumsDirty=true;
            }
        }

        SetSdrObjListRectsDirty();
        SdrObject* pParentSdrObject(getSdrObjectFromSdrObjList());

        if(pParentSdrObject && !GetObjCount())
        {
            // empty group created; it needs to be repainted since it's
            // visualization changes
            pParentSdrObject->ActionChanged();
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
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: the object does not have status Inserted.");
        pObj->InsertedStateChange();
        SetParentAtSdrObjectFromSdrObjList(*pObj, nullptr);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        pNewObj->SetOrdNum(nObjNum);
        SetParentAtSdrObjectFromSdrObjList(*pNewObj, this);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->InsertedStateChange();
        SetSdrObjListRectsDirty();
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
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: the object does not have status Inserted.");

        // TODO: We need a different broadcast here.
        if (pObj->getSdrPageFromSdrObject()!=nullptr)
        {
            SdrHint aHint(SdrHintKind::ObjectRemoved, *pObj);
            pObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        pObj->InsertedStateChange();
        SetParentAtSdrObjectFromSdrObjList(*pObj, nullptr);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        pNewObj->SetOrdNum(nObjNum);
        SetParentAtSdrObjectFromSdrObjList(*pNewObj, this);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->InsertedStateChange();

        // TODO: We need a different broadcast here.
        if (pNewObj->getSdrPageFromSdrObject()!=nullptr) {
            SdrHint aHint(SdrHintKind::ObjectInserted, *pNewObj);
            pNewObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        pNewObj->getSdrModelFromSdrObject().SetChanged();

        SetSdrObjListRectsDirty();
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
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: the object does not have status Inserted.");
        RemoveObjectFromContainer(nOldObjNum);
        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        mbObjOrdNumsDirty=true;

        // TODO: We need a different broadcast here.
        if (pObj->getSdrPageFromSdrObject()!=nullptr)
            pObj->getSdrModelFromSdrObject().Broadcast(SdrHint(SdrHintKind::ObjectChange, *pObj));
        pObj->getSdrModelFromSdrObject().SetChanged();
    }
    return pObj;
}

const tools::Rectangle& SdrObjList::GetAllObjSnapRect() const
{
    if (mbRectsDirty) {
        const_cast<SdrObjList*>(this)->RecalcRects();
        const_cast<SdrObjList*>(this)->mbRectsDirty=false;
    }
    return maSdrObjListSnapRect;
}

const tools::Rectangle& SdrObjList::GetAllObjBoundRect() const
{
    // #i106183# for deep group hierarchies like in chart2, the invalidates
    // through the hierarchy are not correct; use a 2nd hint for the needed
    // recalculation. Future versions will have no bool flag at all, but
    // just maSdrObjListOutRect in empty state to represent an invalid state, thus
    // it's a step in the right direction.
    if (mbRectsDirty || maSdrObjListOutRect.IsEmpty())
    {
        const_cast<SdrObjList*>(this)->RecalcRects();
        const_cast<SdrObjList*>(this)->mbRectsDirty=false;
    }
    return maSdrObjListOutRect;
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
    SdrObjListIter aIter(this, SdrIterMode::DeepNoGroups);

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
    bool bRet(false);
    SdrObject* pParentSdrObject(getSdrObjectFromSdrObjList());

    if(nullptr != pParentSdrObject)
    {
        SdrPage* pSdrPage(pParentSdrObject->getSdrPageFromSdrObject());

        if(nullptr != pSdrPage)
        {
            bRet = pSdrPage->IsReadOnly();
        }
    }

    return bRet;
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
                InsertObject(pObj, nInsertPos);
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

    tools::WeakReference<SdrObject> aReference (&rObject);

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
        rObject.getSdrModelFromSdrObject().SetChanged();
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
        if (static_cast<sal_uInt32>(nCount) != maList.size())
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
    {
        ClearObjectNavigationOrder();
    }
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
    mbObjOrdNumsDirty=true;
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
        tools::WeakReference<SdrObject> aReference (maList[nObjectPosition]);
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
    mbObjOrdNumsDirty=true;
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
        tools::WeakReference<SdrObject> aReference (maList[nObjectPosition]);
        WeakSdrObjectContainerType::iterator iObject (::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference));
        if (iObject != mxNavigationOrder->end())
            mxNavigationOrder->erase(iObject);
        mbIsNavigationOrderDirty = true;
    }

    maList.erase(maList.begin()+nObjectPosition);
    mbObjOrdNumsDirty=true;
}

void SdrObjList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrObjList"));
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
        maProperties.SetParent(nullptr);
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
        maProperties.SetParent(&rNewStyleSheet.GetItemSet());
    }
}

void ImpPageChange(SdrPage& rSdrPage)
{
    rSdrPage.ActionChanged();
    rSdrPage.getSdrModelFromSdrPage().SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, &rSdrPage);
    rSdrPage.getSdrModelFromSdrPage().Broadcast(aHint);
}

SdrPageProperties::SdrPageProperties(SdrPage& rSdrPage)
:   SfxListener(),
    mpSdrPage(&rSdrPage),
    mpStyleSheet(nullptr),
    maProperties(
        mpSdrPage->getSdrModelFromSdrPage().GetItemPool(),
        svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{})
{
    if(!rSdrPage.IsMasterPage())
    {
        maProperties.Put(XFillStyleItem(drawing::FillStyle_NONE));
    }
}

SdrPageProperties::~SdrPageProperties()
{
    ImpRemoveStyleSheet();
}

void SdrPageProperties::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    switch(rHint.GetId())
    {
        case SfxHintId::DataChanged :
            {
                // notify change, broadcast
                ImpPageChange(*mpSdrPage);
                break;
            }
        case SfxHintId::Dying :
            {
                // Style needs to be forgotten
                ImpRemoveStyleSheet();
                break;
            }
        default: break;
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
    maProperties.Put(rSet);
    ImpPageChange(*mpSdrPage);
}

void SdrPageProperties::PutItem(const SfxPoolItem& rItem)
{
    OSL_ENSURE(!mpSdrPage->IsMasterPage(), "Item set at MasterPage Attributes (!)");
    maProperties.Put(rItem);
    ImpPageChange(*mpSdrPage);
}

void SdrPageProperties::ClearItem(const sal_uInt16 nWhich)
{
    maProperties.ClearItem(nWhich);
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


SdrPage::SdrPage(SdrModel& rModel, bool bMasterPage)
:   tools::WeakBase(),
    SdrObjList(),
    maPageUsers(),
    mpViewContact(nullptr),
    mrSdrModelFromSdrPage(rModel),
    mnWidth(10),
    mnHeight(10),
    mnBorderLeft(0),
    mnBorderUpper(0),
    mnBorderRight(0),
    mnBorderLower(0),
    mpLayerAdmin(new SdrLayerAdmin(&rModel.GetLayerAdmin())),
    mpSdrPageProperties(nullptr),
    mxUnoPage(),
    mpMasterPageDescriptor(nullptr),
    nPageNum(0),
    mbMaster(bMasterPage),
    mbInserted(false),
    mbObjectsNotPersistent(false),
    mbPageBorderOnlyLeftRight(false)
{
    mpSdrPageProperties.reset(new SdrPageProperties(*this));
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
        DBG_UNHANDLED_EXCEPTION("svx");
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

SdrModel& SdrPage::getSdrModelFromSdrObjList() const
{
    return getSdrModelFromSdrPage();
}

void SdrPage::lateInit(const SdrPage& rSrcPage)
{
    assert(!mpViewContact);
    assert(!mxUnoPage.is());

    // copy all the local parameters to make this instance
    // a valid copy of source page before copying and inserting
    // the contained objects
    mbMaster = rSrcPage.mbMaster;
    mbPageBorderOnlyLeftRight = rSrcPage.mbPageBorderOnlyLeftRight;
    mnWidth = rSrcPage.mnWidth;
    mnHeight = rSrcPage.mnHeight;
    mnBorderLeft = rSrcPage.mnBorderLeft;
    mnBorderUpper = rSrcPage.mnBorderUpper;
    mnBorderRight = rSrcPage.mnBorderRight;
    mnBorderLower = rSrcPage.mnBorderLower;
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
    if(0 != rSrcPage.GetObjCount())
    {
        CopyObjects(rSrcPage);
    }
}

SdrPage* SdrPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    SdrPage* pClonedPage(new SdrPage(rTargetModel));
    pClonedPage->lateInit(*this);
    return pClonedPage;
}

void SdrPage::SetSize(const Size& aSiz)
{
    bool bChanged(false);

    if(aSiz.Width() != mnWidth)
    {
        mnWidth = aSiz.Width();
        bChanged = true;
    }

    if(aSiz.Height() != mnHeight)
    {
        mnHeight = aSiz.Height();
        bChanged = true;
    }

    if(bChanged)
    {
        SetChanged();
    }
}

Size SdrPage::GetSize() const
{
    return Size(mnWidth,mnHeight);
}

sal_Int32 SdrPage::GetWidth() const
{
    return mnWidth;
}

void SdrPage::SetOrientation(Orientation eOri)
{
    // square: handle like portrait format
    Size aSiz(GetSize());
    if (aSiz.Width()!=aSiz.Height()) {
        if ((eOri==Orientation::Portrait) == (aSiz.Width()>aSiz.Height())) {
            SetSize(Size(aSiz.Height(),aSiz.Width()));
        }
    }
}

Orientation SdrPage::GetOrientation() const
{
    // square: handle like portrait format
    Orientation eRet=Orientation::Portrait;
    Size aSiz(GetSize());
    if (aSiz.Width()>aSiz.Height()) eRet=Orientation::Landscape;
    return eRet;
}

sal_Int32 SdrPage::GetHeight() const
{
    return mnHeight;
}

void  SdrPage::SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 nLwr)
{
    bool bChanged(false);

    if(mnBorderLeft != nLft)
    {
        mnBorderLeft = nLft;
        bChanged = true;
    }

    if(mnBorderUpper != nUpp)
    {
        mnBorderUpper = nUpp;
        bChanged = true;
    }

    if(mnBorderRight != nRgt)
    {
        mnBorderRight = nRgt;
        bChanged = true;
    }

    if(mnBorderLower != nLwr)
    {
        mnBorderLower =  nLwr;
        bChanged = true;
    }

    if(bChanged)
    {
        SetChanged();
    }
}

void  SdrPage::SetLeftBorder(sal_Int32 nBorder)
{
    if(mnBorderLeft != nBorder)
    {
        mnBorderLeft = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetUpperBorder(sal_Int32 nBorder)
{
    if(mnBorderUpper != nBorder)
    {
        mnBorderUpper = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetRightBorder(sal_Int32 nBorder)
{
    if(mnBorderRight != nBorder)
    {
        mnBorderRight=nBorder;
        SetChanged();
    }
}

void  SdrPage::SetLowerBorder(sal_Int32 nBorder)
{
    if(mnBorderLower != nBorder)
    {
        mnBorderLower=nBorder;
        SetChanged();
    }
}

sal_Int32 SdrPage::GetLeftBorder() const
{
    return mnBorderLeft;
}

sal_Int32 SdrPage::GetUpperBorder() const
{
    return mnBorderUpper;
}

sal_Int32 SdrPage::GetRightBorder() const
{
    return mnBorderRight;
}

sal_Int32 SdrPage::GetLowerBorder() const
{
    return mnBorderLower;
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
        if (getSdrModelFromSdrPage().IsMPgNumsDirty())
            getSdrModelFromSdrPage().RecalcPageNums(true);
    } else {
        if (getSdrModelFromSdrPage().IsPagNumsDirty())
            getSdrModelFromSdrPage().RecalcPageNums(false);
    }
    return nPageNum;
}

void SdrPage::SetChanged()
{
    // For test purposes, use the new ViewContact for change
    // notification now.
    ActionChanged();
    getSdrModelFromSdrPage().SetChanged();
}

SdrPage* SdrPage::getSdrPageFromSdrObjList() const
{
    return const_cast< SdrPage* >(this);
}

// MasterPage interface

void SdrPage::TRG_SetMasterPage(SdrPage& rNew)
{
    if(mpMasterPageDescriptor && &(mpMasterPageDescriptor->GetUsedPage()) == &rNew)
        return;

    if(mpMasterPageDescriptor)
        TRG_ClearMasterPage();

    mpMasterPageDescriptor.reset(new sdr::MasterPageDescriptor(*this, rNew));
    GetViewContact().ActionChanged();
}

void SdrPage::TRG_ClearMasterPage()
{
    if(mpMasterPageDescriptor)
    {
        SetChanged();

        // the flushViewObjectContacts() will do needed invalidates by deleting the involved VOCs
        mpMasterPageDescriptor->GetUsedPage().GetViewContact().flushViewObjectContacts();

        mpMasterPageDescriptor.reset();
    }
}

SdrPage& SdrPage::TRG_GetMasterPage() const
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_GetMasterPage(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetUsedPage();
}

const SdrLayerIDSet& SdrPage::TRG_GetMasterPageVisibleLayers() const
{
    DBG_ASSERT(mpMasterPageDescriptor != nullptr, "TRG_GetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetVisibleLayers();
}

void SdrPage::TRG_SetMasterPageVisibleLayers(const SdrLayerIDSet& rNew)
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

const SdrPageGridFrameList* SdrPage::GetGridFrameList(const SdrPageView* /*pPV*/, const tools::Rectangle* /*pRect*/) const
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
    if( mbInserted != bIns )
    {
        mbInserted = bIns;

        // #i120437# go over whole hierarchy, not only over object level null (seen from grouping)
        SdrObjListIter aIter(this, SdrIterMode::DeepNoGroups);

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
Color SdrPage::GetPageBackgroundColor( SdrPageView const * pView, bool bScreenDisplay ) const
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
        if(drawing::FillStyle_NONE == pBackgroundFill->Get(XATTR_FILLSTYLE).GetValue())
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
