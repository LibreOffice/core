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
#include <set>
#include <unordered_set>

#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>

#include <o3tl/safeint.hxx>
#include <string.h>

#include <tools/debug.hxx>
#include <tools/json_writer.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <sfx2/viewsh.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/xfillit0.hxx>

#include <sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/annotation/Annotation.hxx>
#include <svx/annotation/ObjectAnnotationData.hxx>
#include <algorithm>
#include <clonelist.hxx>
#include <svl/hint.hxx>
#include <rtl/strbuf.hxx>
#include <libxml/xmlwriter.h>
#include <docmodel/theme/Theme.hxx>
#include <comphelper/lok.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

SdrObjList::SdrObjList()
:   mbObjOrdNumsDirty(false),
    mbRectsDirty(false),
    mbIsNavigationOrderDirty(false)
{
}

void SdrObjList::impClearSdrObjList()
{
    SdrModel* pSdrModelFromRemovedSdrObject(nullptr);

    while(!maList.empty())
    {
        // remove last object from list
        rtl::Reference<SdrObject> pObj(maList.back());
        RemoveObjectFromContainer(maList.size()-1);

        // flushViewObjectContacts() is done since SdrObject::Free is not guaranteed
        // to delete the object and thus refresh visualisations
        pObj->GetViewContact().flushViewObjectContacts();

        if(nullptr == pSdrModelFromRemovedSdrObject)
        {
            pSdrModelFromRemovedSdrObject = &pObj->getSdrModelFromSdrObject();
        }

        // sent remove hint (after removal, see RemoveObject())
        // TTTT SdrPage not needed, can be accessed using SdrObject
        SdrHint aHint(SdrHintKind::ObjectRemoved, *pObj, getSdrPageFromSdrObjList());
        pObj->getSdrModelFromSdrObject().Broadcast(aHint);

        pObj->setParentOfSdrObject(nullptr);
    }

    if(nullptr != pSdrModelFromRemovedSdrObject)
    {
        pSdrModelFromRemovedSdrObject->SetChanged();
    }
}

void SdrObjList::ClearSdrObjList()
{
    // clear SdrObjects with broadcasting
    impClearSdrObjList();
}

SdrObjList::~SdrObjList()
{
    // Clear SdrObjects without broadcasting.
    for (auto& rxObj : maList)
        rxObj->setParentOfSdrObject(nullptr);
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

OString SdrObjList::GetObjectRectangles(const SdrObjList& rSrcList)
{
    tools::JsonWriter jsWriter;

    {
        auto array = jsWriter.startAnonArray();

        for (const rtl::Reference<SdrObject>& item: rSrcList)
        {
            if (item->IsPrintable() && item->IsVisible())
            {
                tools::Rectangle rectangle = item->GetCurrentBoundRect();
                OString value = rectangle.toString() + ", "_ostr + OString::number(item->GetOrdNum());
                auto subArray = jsWriter.startAnonArray();
                jsWriter.putRaw(value);
            }
        }
    }

    return jsWriter.finishAndGetAsOString();
}

void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
{
    CloneList aCloneList;

    // clear SdrObjects with broadcasting
    ClearSdrObjList();

    mbObjOrdNumsDirty = false;
    mbRectsDirty = false;
#ifdef DBG_UTIL
    size_t nCloneErrCnt(0);
#endif

    if(nullptr == getSdrObjectFromSdrObjList() && nullptr == getSdrPageFromSdrObjList())
    {
        OSL_ENSURE(false, "SdrObjList which is not part of SdrPage or SdrObject (!)");
        return;
    }

    SdrModel& rTargetSdrModel(nullptr == getSdrObjectFromSdrObjList()
        ? getSdrPageFromSdrObjList()->getSdrModelFromSdrPage()
        : getSdrObjectFromSdrObjList()->getSdrModelFromSdrObject());

    for (const rtl::Reference<SdrObject>& pSourceObject : rSrcList)
    {
        rtl::Reference<SdrObject> pTargetObject(pSourceObject->CloneSdrObject(rTargetSdrModel));

        if (pTargetObject)
        {
            NbcInsertObject(pTargetObject.get(), SAL_MAX_SIZE);
            aCloneList.AddPair(pSourceObject.get(), pTargetObject.get());
            if (pSourceObject->isAnnotationObject())
            {
                pTargetObject->setAsAnnotationObject();
                pTargetObject->SetPrintable(false);
                rtl::Reference<sdr::annotation::Annotation> xNewAnnotation;
                SdrPage* pPage = pTargetObject->getSdrPageFromSdrObject();
                xNewAnnotation = pSourceObject->getAnnotationData()->mxAnnotation->clone(pPage);
                pTargetObject->getAnnotationData()->mxAnnotation = xNewAnnotation;
                pPage->addAnnotationNoNotify(xNewAnnotation, -1);
            }
        }
#ifdef DBG_UTIL
        else
        {
            nCloneErrCnt++;
        }
#endif
    }

    // Wires up the connections
    aCloneList.CopyConnections();
#ifdef DBG_UTIL
    if (nCloneErrCnt != 0)
    {
        OStringBuffer aStr("SdrObjList::operator=(): Error when cloning ");

        if(nCloneErrCnt == 1)
        {
            aStr.append("a drawing object.");
        }
        else
        {
            aStr.append(OString::number(static_cast<sal_Int32>(nCloneErrCnt))
                + " drawing objects.");
        }

        OSL_FAIL(aStr.getStr());
    }
#endif
}

void SdrObjList::RecalcObjOrdNums()
{
    size_t no=0;
    for (const rtl::Reference<SdrObject>& pObj : maList)
        pObj->SetOrdNum(no++);
    mbObjOrdNumsDirty=false;
}

void SdrObjList::RecalcRects()
{
    maSdrObjListOutRect=tools::Rectangle();
    maSdrObjListSnapRect=maSdrObjListOutRect;
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it) {
        SdrObject* pObj = it->get();
        if (it == begin()) {
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
        pParentSdrObject->SetBoundAndSnapRectsDirty();
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
    pObj->setParentOfSdrObject(this);

    // Inform the parent about change to allow invalidations at
    // evtl. existing parent visualisations
    impChildInserted(*pObj);

    if (!mbRectsDirty) {
        mbRectsDirty = true;
    }
    pObj->InsertedStateChange(); // calls the UserCall (among others)
}

void SdrObjList::InsertObjectThenMakeNameUnique(SdrObject* pObj)
{
    std::unordered_set<rtl::OUString> aNameSet;
    InsertObjectThenMakeNameUnique(pObj, aNameSet);
}

void SdrObjList::InsertObjectThenMakeNameUnique(SdrObject* pObj, std::unordered_set<OUString>& rNameSet, size_t nPos)
{
    InsertObject(pObj, nPos);
    if (pObj->GetName().isEmpty())
        return;

    pObj->MakeNameUnique(rNameSet);
    SdrObjList* pSdrObjList = pObj->GetSubList(); // group
    if (pSdrObjList)
    {
        SdrObject* pListObj;
        SdrObjListIter aIter(pSdrObjList, SdrIterMode::DeepWithGroups);
        while (aIter.IsMore())
        {
            pListObj = aIter.Next();
            pListObj->MakeNameUnique(rNameSet);
        }
    }
}

void SdrObjList::InsertObject(SdrObject* pObj, size_t nPos)
{
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::InsertObject(NULL)");

    if(!pObj)
        return;

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
    if(pObj->getSdrPageFromSdrObject() && !pObj->getSdrModelFromSdrObject().isLocked())
    {
        SdrHint aHint(SdrHintKind::ObjectInserted, *pObj);
        pObj->getSdrModelFromSdrObject().Broadcast(aHint);
    }

    pObj->getSdrModelFromSdrObject().SetChanged();
}

rtl::Reference<SdrObject> SdrObjList::NbcRemoveObject(size_t nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return nullptr;
    }

    const size_t nCount = GetObjCount();
    rtl::Reference<SdrObject> pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=nullptr,"Could not find object to remove.");
    if (pObj!=nullptr)
    {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts();

        DBG_ASSERT(pObj->IsInserted(),"The object does not have the status Inserted.");

        // tdf#121022 Do first remove from SdrObjList - InsertedStateChange
        // relies now on IsInserted which uses getParentSdrObjListFromSdrObject
        pObj->setParentOfSdrObject(nullptr);

        // calls UserCall, among other
        pObj->InsertedStateChange();

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

rtl::Reference<SdrObject> SdrObjList::RemoveObject(size_t nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return nullptr;
    }

    const size_t nCount = GetObjCount();
    rtl::Reference<SdrObject> pObj=maList[nObjNum];
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

        // tdf#121022 Do first remove from SdrObjList - InsertedStateChange
        // relies now on IsInserted which uses getParentSdrObjListFromSdrObject
        pObj->setParentOfSdrObject(nullptr);

        // calls, among other things, the UserCall
        pObj->InsertedStateChange();

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

rtl::Reference<SdrObject> SdrObjList::ReplaceObject(SdrObject* pNewObj, size_t nObjNum)
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

    rtl::Reference<SdrObject> pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=nullptr,"SdrObjList::ReplaceObject: Could not find object to remove.");
    if (pObj!=nullptr) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: the object does not have status Inserted.");

        // TODO: We need a different broadcast here.
        if (pObj->getSdrPageFromSdrObject()!=nullptr)
        {
            SdrHint aHint(SdrHintKind::ObjectRemoved, *pObj);
            pObj->getSdrModelFromSdrObject().Broadcast(aHint);
        }

        // Change parent and replace in SdrObjList
        pObj->setParentOfSdrObject(nullptr);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // tdf#121022 InsertedStateChange uses the parent
        // to detect if pObj is inserted or not, so have to call
        // it *after* changing these settings, else an obviously wrong
        // 'SdrUserCallType::Inserted' would be sent
        pObj->InsertedStateChange();

        // flushViewObjectContacts() clears the VOC's and those
        // trigger the evtl. needed invalidate(s)
        pObj->GetViewContact().flushViewObjectContacts();

        // Setup data at new SdrObject - it already *is* inserted to
        // the SdrObjList due to 'ReplaceObjectInContainer' above
        pNewObj->SetOrdNum(nObjNum);
        pNewObj->setParentOfSdrObject(this);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations, but also react on
        // newly inserted SdrObjects (as e.g. GraphCtrlUserCall does)
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

    rtl::Reference<SdrObject> pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj.get();
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
    return pObj.get();
}

void SdrObjList::SetExistingObjectOrdNum(SdrObject* pObj, size_t nNewObjNum)
{
    assert(std::find(maList.begin(), maList.end(), pObj) != maList.end() && "This method requires that the child object already be inserted");
    assert(pObj->IsInserted() && "SdrObjList::SetObjectOrdNum: the object does not have status Inserted.");

    // I am deliberately bypassing getOrdNum() because I don't want to unnecessarily
    // trigger RecalcObjOrdNums()
    const sal_uInt32 nOldOrdNum = pObj->m_nOrdNum;
    if (!mbObjOrdNumsDirty && nOldOrdNum == nNewObjNum)
       return;

    // Update the navigation positions.
    if (HasObjectNavigationOrder())
    {
        unotools::WeakReference<SdrObject> aReference (pObj);
        auto iObject = ::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference);
        mxNavigationOrder->erase(iObject);
        mbIsNavigationOrderDirty = true;
        // The new object does not have a user defined position so append it
        // to the list.
        pObj->SetNavigationPosition(mxNavigationOrder->size());
        mxNavigationOrder->push_back(pObj);
    }
    if (nOldOrdNum < maList.size() && maList[nOldOrdNum] == pObj)
        maList.erase(maList.begin()+nOldOrdNum);
    else
    {
        auto it = std::find(maList.begin(), maList.end(), pObj);
        maList.erase(it);
    }
    // Insert object into object list.  Because the insert() method requires
    // a valid iterator as insertion position, we have to use push_back() to
    // insert at the end of the list.
    if (nNewObjNum >= maList.size())
        maList.push_back(pObj);
    else
        maList.insert(maList.begin()+nNewObjNum, pObj);

    mbObjOrdNumsDirty=true;

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

void SdrObjList::sort( std::vector<sal_Int32>& sortOrder)
{
    // no negative indexes and indexes larger than maList size are allowed
    auto it = std::find_if( sortOrder.begin(), sortOrder.end(), [this](const sal_Int32& rIt)
         { return ( rIt < 0 || o3tl::make_unsigned(rIt) >= maList.size() ); } );
    if ( it != sortOrder.end())
        throw css::lang::IllegalArgumentException(u"negative index of shape"_ustr, nullptr, 1);

    // no duplicates
    std::vector<bool> aNoDuplicates(sortOrder.size(), false);
    for (const sal_Int32 nSortOrder : sortOrder )
    {
        size_t idx =  static_cast<size_t>( nSortOrder );

        if ( aNoDuplicates[idx] )
            throw css::lang::IllegalArgumentException(u"duplicate index of shape"_ustr, nullptr, 2);

        aNoDuplicates[idx] = true;
    }

    // example sortOrder [2 0 1]
    // example maList [T T S T T] ( T T = shape with textbox, S = just a shape )
    // (shapes at positions 0 and 2 have a textbox)

    std::deque<rtl::Reference<SdrObject>> aNewList(maList.size());
    std::set<sal_Int32> aShapesWithTextbox;
    std::vector<sal_Int32> aIncrements;
    std::vector<sal_Int32> aDuplicates;

    if ( maList.size() > 1)
    {
        for (size_t i = 1; i< maList.size(); ++i)
        {
            // if this shape is a textbox, then look at its left neighbour
            // (shape this textbox is in)
            // and insert the number of textboxes to the left of it
            if (maList[i]->IsTextBox())
              aShapesWithTextbox.insert( i - 1 - aShapesWithTextbox.size() );
        }
        // example aShapesWithTextbox [0 2]
    }

    if (aShapesWithTextbox.size() != maList.size() - sortOrder.size())
    {
        throw lang::IllegalArgumentException(u"mismatch of no. of shapes"_ustr, nullptr, 0);
    }

    for (size_t i = 0; i< sortOrder.size(); ++i)
    {

         if (aShapesWithTextbox.count(sortOrder[i]) > 0)
             aDuplicates.push_back(sortOrder[i]);

         aDuplicates.push_back(sortOrder[i]);

         // example aDuplicates [2 2 0 0 1]
    }
    assert(aDuplicates.size() == maList.size());

    aIncrements.push_back(0);
    for (size_t i = 1; i< sortOrder.size(); ++i)
    {
         if (aShapesWithTextbox.count(i - 1))
             aIncrements.push_back(aIncrements[i-1] + 1 );
         else
             aIncrements.push_back(aIncrements[i-1]);

         // example aIncrements [0 1 1]
    }
    assert(aIncrements.size() == sortOrder.size());

    std::vector<sal_Int32> aNewSortOrder(maList.size());
    sal_Int32 nPrev = -1;
    for (size_t i = 0; i< aDuplicates.size(); ++i)
    {
        if (nPrev != aDuplicates[i])
            aNewSortOrder[i] = aDuplicates[i] + aIncrements[aDuplicates[i]];
        else if (i > 0)
            aNewSortOrder[i] = aNewSortOrder[i-1] + 1;

        nPrev = aDuplicates[i];

        // example aNewSortOrder [3 4 0 1 2]
    }
    assert(aNewSortOrder.size() == maList.size());

#ifndef NDEBUG
    {
        std::vector<sal_Int32> tmp(aNewSortOrder);
        std::sort(tmp.begin(), tmp.end());
        for (size_t i = 0; i < tmp.size(); ++i)
        {
            assert(size_t(tmp[i]) == i);
        }
    }
#endif

    SdrModel & rModel(getSdrPageFromSdrObjList()->getSdrModelFromSdrPage());
    bool const isUndo(rModel.IsUndoEnabled());
    if (isUndo)
    {
        rModel.AddUndo(SdrUndoFactory::CreateUndoSort(*getSdrPageFromSdrObjList(), sortOrder));
    }

    for (size_t i = 0; i < aNewSortOrder.size(); ++i)
    {
        aNewList[i] = maList[ aNewSortOrder[i] ];
        aNewList[i]->SetOrdNum(i);
    }

    std::swap(aNewList, maList);
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
    ImplReformatAllEdgeObjects(*this);
}

void SdrObjList::ImplReformatAllEdgeObjects(const SdrObjList& rObjList)
{
    // #i120437# go over whole hierarchy, not only over object level null (seen from grouping)
    for(size_t nIdx(0), nCount(rObjList.GetObjCount()); nIdx < nCount; ++nIdx)
    {
        SdrObject* pSdrObject(rObjList.GetObjectForNavigationPosition(nIdx));
        const SdrObjList* pChildren(pSdrObject->getChildrenOfSdrObject());
        const bool bIsGroup(nullptr != pChildren);
        if(!bIsGroup)
        {
            // Check IsVirtualObj because sometimes we get SwDrawVirtObj here
            if (pSdrObject->GetObjIdentifier() == SdrObjKind::Edge
               && !pSdrObject->IsVirtualObj())
            {
                SdrEdgeObj* pSdrEdgeObj = static_cast< SdrEdgeObj* >(pSdrObject);
                pSdrEdgeObj->Reformat();
            }
        }
        else
        {
            ImplReformatAllEdgeObjects(*pChildren);
        }
    }
}

void SdrObjList::BurnInStyleSheetAttributes()
{
    for (const rtl::Reference<SdrObject>& pObj : *this)
        pObj->BurnInStyleSheetAttributes();
}

size_t SdrObjList::GetObjCount() const
{
    return maList.size();
}


SdrObject* SdrObjList::GetObj(size_t nNum) const
{
    if (nNum < maList.size())
        return maList[nNum].get();

    return nullptr;
}

SdrObject* SdrObjList::GetObjByName(std::u16string_view sName) const
{
    for (const rtl::Reference<SdrObject>& pObj : *this)
    {
        if (pObj->GetName() == sName)
            return pObj.get();
    }
    return nullptr;
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
        if(pSrcLst)
            if(auto pUngroupGroup = dynamic_cast<SdrObjGroup*>( pUngroupObj))
            {
                // ungroup recursively (has to be head recursion,
                // otherwise our indices will get trashed when doing it in
                // the loop)
                pSrcLst->FlattenGroups();

                // the position at which we insert the members of rUngroupGroup
                size_t nInsertPos( pUngroupGroup->GetOrdNum() );

                const size_t nCount = pSrcLst->GetObjCount();
                for( size_t i=0; i<nCount; ++i )
                {
                    rtl::Reference<SdrObject> pObj = pSrcLst->RemoveObject(0);
                    InsertObject(pObj.get(), nInsertPos);
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

bool SdrObjList::HasObjectNavigationOrder() const { return bool(mxNavigationOrder); }

void SdrObjList::SetObjectNavigationPosition (
    SdrObject& rObject,
    const sal_uInt32 nNewPosition)
{
    // When the navigation order container has not yet been created then
    // create one now.  It is initialized with the z-order taken from
    // maList.
    if (!mxNavigationOrder)
    {
        mxNavigationOrder.emplace(maList.begin(), maList.end());
    }
    OSL_ASSERT(bool(mxNavigationOrder));
    OSL_ASSERT( mxNavigationOrder->size() == maList.size());

    unotools::WeakReference<SdrObject> aReference (&rObject);

    // Look up the object whose navigation position is to be changed.
    auto iObject = ::std::find(
        mxNavigationOrder->begin(),
        mxNavigationOrder->end(),
        aReference);
    if (iObject == mxNavigationOrder->end())
    {
        // The given object is not a member of the navigation order.
        return;
    }

    // Move the object to its new position.
    const sal_uInt32 nOldPosition = ::std::distance(mxNavigationOrder->begin(), iObject);
    if (nOldPosition == nNewPosition)
        return;

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
            return (*mxNavigationOrder)[nNavigationPosition].get().get();
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
            return maList[nNavigationPosition].get();
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
        if (mxNavigationOrder)
        {
            mbIsNavigationOrderDirty = false;

            sal_uInt32 nIndex (0);
            for (auto& rpObject : *mxNavigationOrder)
            {
                rpObject.get()->SetNavigationPosition(nIndex);
                ++nIndex;
            }
        }
    }

    return bool(mxNavigationOrder);
}


void SdrObjList::SetNavigationOrder (const uno::Reference<container::XIndexAccess>& rxOrder)
{
    if (rxOrder.is())
    {
        const sal_Int32 nCount = rxOrder->getCount();
        if (static_cast<sal_uInt32>(nCount) != maList.size())
            return;

        if (!mxNavigationOrder)
            mxNavigationOrder = std::vector<unotools::WeakReference<SdrObject>>(nCount);

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
        unotools::WeakReference<SdrObject> aReference (maList[nObjectPosition].get());
        auto iObject = ::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference);
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
        unotools::WeakReference<SdrObject> aReference (maList[nObjectPosition]);
        auto iObject = ::std::find(
            mxNavigationOrder->begin(),
            mxNavigationOrder->end(),
            aReference);
        if (iObject != mxNavigationOrder->end())
            mxNavigationOrder->erase(iObject);
        mbIsNavigationOrderDirty = true;
    }

    maList.erase(maList.begin()+nObjectPosition);
    mbObjOrdNumsDirty=true;
}

void SdrObjList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrObjList"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));

    for (const rtl::Reference<SdrObject>& pObject : *this)
        pObject->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}


void SdrPageGridFrameList::Clear()
{
    sal_uInt16 nCount=GetCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        delete GetObject(i);
    }
    m_aList.clear();
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

std::unique_ptr<sdr::contact::ViewContact> SdrPage::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrPage>(*this);
}

const sdr::contact::ViewContact& SdrPage::GetViewContact() const
{
    if (!mpViewContact)
        const_cast<SdrPage*>(this)->mpViewContact =
            const_cast<SdrPage*>(this)->CreateObjectSpecificViewContact();

    return *mpViewContact;
}

sdr::contact::ViewContact& SdrPage::GetViewContact()
{
    if (!mpViewContact)
        mpViewContact = CreateObjectSpecificViewContact();

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

namespace
{

void ImpPageChange(SdrPage& rSdrPage)
{
    rSdrPage.ActionChanged();
    rSdrPage.getSdrModelFromSdrPage().SetChanged();
    SdrHint aHint(SdrHintKind::PageOrderChange, &rSdrPage);
    rSdrPage.getSdrModelFromSdrPage().Broadcast(aHint);
}

} // end anonymous namespace

SdrPageProperties::SdrPageProperties(SdrPage& rSdrPage)
    : mrSdrPage(rSdrPage)
    , mpStyleSheet(nullptr)
    , maProperties(
        mrSdrPage.getSdrModelFromSdrPage().GetItemPool(),
        svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>)
{
    if (!mrSdrPage.IsMasterPage())
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
                ImpPageChange(mrSdrPage);
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
    return mrSdrPage.IsInserted();
}


void SdrPageProperties::PutItemSet(const SfxItemSet& rSet)
{
    OSL_ENSURE(!mrSdrPage.IsMasterPage(), "Item set at MasterPage Attributes (!)");
    maProperties.Put(rSet);
    ImpPageChange(mrSdrPage);
}

void SdrPageProperties::PutItem(const SfxPoolItem& rItem)
{
    OSL_ENSURE(!mrSdrPage.IsMasterPage(), "Item set at MasterPage Attributes (!)");
    maProperties.Put(rItem);
    ImpPageChange(mrSdrPage);
}

void SdrPageProperties::ClearItem(const sal_uInt16 nWhich)
{
    maProperties.ClearItem(nWhich);
    ImpPageChange(mrSdrPage);
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

    ImpPageChange(mrSdrPage);
}

void SdrPageProperties::setTheme(std::shared_ptr<model::Theme> const& pTheme)
{
    // Only set the theme on a master page, else set it on the model

    if (mrSdrPage.IsMasterPage())
    {
        if (mpTheme != pTheme)
            mpTheme = pTheme;
    }
    else
    {
        mrSdrPage.getSdrModelFromSdrPage().setTheme(pTheme);
    }
}

std::shared_ptr<model::Theme> const& SdrPageProperties::getTheme() const
{
    // If the page theme is available use that, else get the theme from the model
    if (mpTheme)
        return mpTheme;
    else
        return mrSdrPage.getSdrModelFromSdrPage().getTheme();
}

void SdrPageProperties::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrPageProperties"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);

    if (mpTheme)
    {
        mpTheme->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

SdrPage::SdrPage(SdrModel& rModel, bool bMasterPage)
:   mrSdrModelFromSdrPage(rModel),
    mnWidth(10),
    mnHeight(10),
    mnBorderLeft(0),
    mnBorderUpper(0),
    mnBorderRight(0),
    mnBorderLower(0),
    mpLayerAdmin(new SdrLayerAdmin(&rModel.GetLayerAdmin())),
    m_nPageNum(0),
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
    for(sdr::PageUser* pPageUser : aListCopy)
    {
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
    mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
    m_nPageNum = rSrcPage.m_nPageNum;

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

rtl::Reference<SdrPage> SdrPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    rtl::Reference<SdrPage> pClonedPage(new SdrPage(rTargetModel));
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

tools::Long SdrPage::GetWidth() const
{
    return mnWidth;
}

void SdrPage::SetOrientation(Orientation eOri)
{
    // square: handle like portrait format
    Size aSiz(GetSize());
    if (aSiz.Width()!=aSiz.Height()) {
        if ((eOri==Orientation::Portrait) == (aSiz.Width()>aSiz.Height())) {
            // coverity[swapped_arguments : FALSE] - this is in the correct order
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

tools::Long SdrPage::GetHeight() const
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

void SdrPage::SetBackgroundFullSize(bool const bIn)
{
    if (bIn != mbBackgroundFullSize)
    {
        mbBackgroundFullSize = bIn;
        SetChanged();
    }
}

bool SdrPage::IsBackgroundFullSize() const
{
    return mbBackgroundFullSize;
}

// #i68775# React on PageNum changes (from Model in most cases)
void SdrPage::SetPageNum(sal_uInt16 nNew)
{
    if(nNew != m_nPageNum)
    {
        // change
        m_nPageNum = nNew;

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
    return m_nPageNum;
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

void SdrPage::MakePageObjectsNamesUnique()
{
    std::unordered_set<OUString> aNameSet;
    for (const rtl::Reference<SdrObject>& pObj : *this)
    {
        if (!pObj->GetName().isEmpty())
        {
            pObj->MakeNameUnique(aNameSet);
            SdrObjList* pSdrObjList = pObj->GetSubList(); // group
            if (pSdrObjList)
            {
                SdrObject* pListObj;
                SdrObjListIter aIter(pSdrObjList, SdrIterMode::DeepWithGroups);
                while (aIter.IsMore())
                {
                    pListObj = aIter.Next();
                    pListObj->MakeNameUnique(aNameSet);
                }
            }
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

void SdrPage::SetInserted( bool bIns )
{
    if( mbInserted == bIns )
        return;

    mbInserted = bIns;

    // #i120437# go over whole hierarchy, not only over object level null (seen from grouping)
    SdrObjListIter aIter(this, SdrIterMode::DeepNoGroups);

    while ( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        if ( auto pOleObj = dynamic_cast<SdrOle2Obj* >(pObj) )
        {
            if( mbInserted )
                pOleObj->Connect();
            else
                pOleObj->Disconnect();
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
    return cppu::getXWeak(new SvxDrawPage(this));
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
        if (const SfxViewShell* pViewShell = SfxViewShell::Current())
            aColor = pViewShell->GetColorConfigColor(svtools::DOCCOLOR);
        else
        {
            svtools::ColorConfig aColorConfig;
            aColor = aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor;
        }
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
            // See unomodel.cxx: "It is guaranteed, that after a standard page the corresponding notes page follows."
            bool notesPage = GetPageNum() % 2 == 0;

            if (!comphelper::LibreOfficeKit::isActive() || !notesPage || !getSdrModelFromSdrPage().IsImpress())
                pBackgroundFill = &TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
            else
            {
                /*
                    See sdrmasterpagedescriptor.cxx: e.g. the Notes MasterPage has no StyleSheet set (and there maybe others).
                */

                // This is a notes page. Try to get itemset from standard page's master.
                if (getSdrModelFromSdrPage().GetPage(GetPageNum() - 1))
                    pBackgroundFill = &getSdrModelFromSdrPage().GetPage(GetPageNum() - 1)->TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
                else
                    pBackgroundFill = &TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
            }
        }
    }

    if (auto oColor = GetDraftFillColor(*pBackgroundFill))
        aColor = *oColor;

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

void SdrPage::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrPage"));
    SdrObjList::dumpAsXml(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("width"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%s",
                                            BAD_CAST(OString::number(mnWidth).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("height"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("value"), "%s",
                                            BAD_CAST(OString::number(mnHeight).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    if (mpSdrPageProperties)
    {
        mpSdrPageProperties->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
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

rtl::Reference<sdr::annotation::Annotation> SdrPage::createAnnotation()
{
    assert(false);
    return nullptr;
}

void SdrPage::addAnnotation(rtl::Reference<sdr::annotation::Annotation> const& /*xAnnotation*/, int /*nIndex*/)
{
    assert(false);
}

void SdrPage::addAnnotationNoNotify(rtl::Reference<sdr::annotation::Annotation> const& /*xAnnotation*/, int /*nIndex*/)
{
    assert(false);
}

void SdrPage::removeAnnotation(rtl::Reference<sdr::annotation::Annotation> const& /*xAnnotation*/)
{
    assert(false);
}

void SdrPage::removeAnnotationNoNotify(rtl::Reference<sdr::annotation::Annotation> const& /*xAnnotation*/)
{
    assert(false);
}

std::vector<rtl::Reference<sdr::annotation::Annotation>> const& SdrPage::getAnnotations() const
{
    return maAnnotations;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
