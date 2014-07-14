/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdpage.hxx>
#include <sot/storage.hxx>
#include <sot/clsids.hxx>
#include <sot/storage.hxx>
#include <svx/svdview.hxx>
#include <string.h>
#ifndef _STRING_H
#define _STRING_H
#endif
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>
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
#include <vcl/salbtype.hxx>
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <algorithm>
#include <svl/smplhint.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdpool.hxx>

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow changing ord num at SdrObject, but only from SdrObjList

void SVX_DLLPRIVATE SetOrdNumAtSdrObjectFromSdrObjList(SdrObject& rSdrObject, sal_uInt32 nOrdNum)
{
    rSdrObject.SetOrdNum(nOrdNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow changing parent at SdrObject, but only from SdrObjList

void SVX_DLLPRIVATE SetParentAtSdrObjectFromSdrObjList(SdrObject& rSdrObject, SdrObjList* pNew)
{
    rSdrObject.setParentOfSdrObject(pNew);
}

//////////////////////////////////////////////////////////////////////////////

void SdrObjList::EnsureValidNavigationPositions(sal_uInt32 nFrom)
{
    if(!maList.empty() && (!nFrom || nFrom < maList.size()))
    {
        SdrObjectVector::iterator aCandidate;
        SdrObjectVector::const_iterator aEnd;
        sal_uInt32 a(nFrom);

        if(maUserNavigationOrder.empty())
        {
            aCandidate = maList.begin() + a;
            aEnd = maList.end();
        }
        else
        {
            aCandidate = maUserNavigationOrder.begin() + a;
            aEnd = maUserNavigationOrder.end();
        }

        for(;aCandidate != aEnd; a++, aCandidate++)
        {
            OSL_ENSURE(*aCandidate, "SdrObjList with empty entries (!)");
            SetOrdNumAtSdrObjectFromSdrObjList(**aCandidate, a);
        }
    }
}

SdrObjList::SdrObjList()
:   maList(),
    maUserNavigationOrder()
{
}

SdrObjList::~SdrObjList()
{
    ClearSdrObjList();
}

SdrObjectVector SdrObjList::getSdrObjectVector() const
{
    if(maUserNavigationOrder.empty())
    {
        return maList;
    }
    else
    {
        return maUserNavigationOrder;
    }
}

void SdrObjList::copyDataFromSdrObjList(const SdrObjList& rSource)
{
    ClearSdrObjList();
    sal_uInt32 nCloneErrCnt(0);
    const sal_uInt32 nAnz(rSource.GetObjCount());
    sal_uInt32 no(0);

    for(no = 0; no < nAnz; no++)
    {
        SdrObject* pSO = rSource.GetObj(no);
        SdrObject* pDO = pSO->CloneSdrObject(&getSdrModelFromSdrObjList());

        if(pDO)
        {
            InsertObjectToSdrObjList(*pDO);
        }
        else
        {
            nCloneErrCnt++;
        }
    }

    OSL_ENSURE(0 == nCloneErrCnt, "copyDataFromSdrObjList: copy error (!)");

    // clone evtl. connectors and evtl. connections
    if(!nCloneErrCnt)
    {
        for(no = 0; no < nAnz; no++)
        {
            const SdrEdgeObj* pSrcEdge = dynamic_cast< SdrEdgeObj* >(rSource.GetObj(no));

            if(pSrcEdge)
            {
                SdrObject* pSrcNode1 = pSrcEdge->GetSdrObjectConnection(true);
                SdrObject* pSrcNode2 = pSrcEdge->GetSdrObjectConnection(false);

                if(pSrcNode1 && pSrcNode1->getParentOfSdrObject() != pSrcEdge->getParentOfSdrObject())
                {
                    pSrcNode1 = 0; // Listenuebergreifend
                }

                if(pSrcNode2 && pSrcNode2->getParentOfSdrObject() != pSrcEdge->getParentOfSdrObject())
                {
                    pSrcNode2 = 0; // ist (noch) nicht
                }

                if(pSrcNode1 || pSrcNode2)
                {
                    SdrEdgeObj* pDstEdge = dynamic_cast< SdrEdgeObj* >(GetObj(no));

                    if(pDstEdge)
                    {
                        if(pSrcNode1)
                        {
                            const sal_uInt32 nDstNode1(pSrcNode1->GetNavigationPosition());
                            SdrObject* pDstNode1 = GetObj(nDstNode1);

                            if(pDstNode1)
                            {
                                // Sonst grober Fehler!
                                pDstEdge->ConnectToSdrObject(true, pDstNode1);
                            }
                            else
                            {
                                DBG_ERROR("SdrObjList::operator=(): pDstNode1==0!");
                            }
                        }

                        if(pSrcNode2)
                        {
                            const sal_uInt32 nDstNode2(pSrcNode2->GetNavigationPosition());
                            SdrObject* pDstNode2 = GetObj(nDstNode2);

                            if(pDstNode2)
                            {
                                // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToSdrObject(false, pDstNode2);
                            }
                            else
                            {
                                DBG_ERROR("SdrObjList::operator=(): pDstNode2==0!");
                            }
                        }
                    }
                    else
                    {
                        DBG_ERROR("SdrObjList::operator=(): pDstEdge==0!");
                    }
                }
            }
        }
    }
}

void SdrObjList::ClearSdrObjList()
{
    maUserNavigationOrder.clear();

    while(!maList.empty())
    {
        // remove last object from list
        SdrObject* pObj = maList.back();

        if(pObj)
        {
            {
                // broadcast immediately (before removal)
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj, HINT_OBJREMOVED);
            }

            RemoveObjectFromContainer(maList.size() - 1);

            // flushViewObjectContacts() is done since deleteSdrObjectSafeAndClearPointer is not guaranteed
            // to delete the object due to UNDO buffer; thus refresh visualisations here
            pObj->GetViewContact().flushViewObjectContacts(true);

            // delete the object itself (due to UNDO buffer this may not really destroy
            // the SdrObject)
            deleteSdrObjectSafeAndClearPointer(pObj);
        }
        else
        {
            OSL_ENSURE(false, "Invalid SdrObjList (!)");
        }
    }
}

SdrPage* SdrObjList::getSdrPageFromSdrObjList() const
{
    // default is no page and returns zero
    return 0;
}

SdrObject* SdrObjList::getSdrObjectFromSdrObjList() const
{
    // default is no SdrObject (SdrObjGroup)
    return 0;
}

void SdrObjList::handleContentChange(const SfxHint& /*rHint*/)
{
    // default has nothing to do
}

void SdrObjList::InsertObjectToSdrObjList(SdrObject& rObj, sal_uInt32 nPos)
{
    SdrObject* pOwningGroupObject = getSdrObjectFromSdrObjList();

    // if anchor is used, reset it before grouping (for SW)
    if(pOwningGroupObject)
    {
        rObj.SetAnchorPos(basegfx::B2DPoint());
    }

    OSL_ENSURE(!rObj.IsObjectInserted(), "Object already has Inserted-State (!)");
    InsertObjectIntoContainer(rObj, nPos);
    SetParentAtSdrObjectFromSdrObjList(rObj, this);

    {
        // broadcast after insert
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(rObj, HINT_OBJINSERTED);
    }

    // inform visible parents
    sdr::contact::ViewContact* pParent = rObj.GetViewContact().GetParentContact();

    if(pParent)
    {
        pParent->ActionChildInserted(rObj.GetViewContact());
    }

    if(pOwningGroupObject)
    {
        // repaint needed
        pOwningGroupObject->ActionChanged();
    }

    getSdrModelFromSdrObjList().SetChanged();
}

SdrObject* SdrObjList::RemoveObjectFromSdrObjList(sal_uInt32 nObjNum)
{
    if(nObjNum < maList.size())
    {
        SdrObject* pObj = *(maList.begin() + nObjNum);

        if(pObj)
        {
            {
                // broadcast immediately (before removal)
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj, HINT_OBJREMOVED);
            }

            RemoveObjectFromContainer(nObjNum);

            // flushViewObjectContacts() clears the VOC's and those invalidate
            pObj->GetViewContact().flushViewObjectContacts(true);
            OSL_ENSURE(pObj->IsObjectInserted(), "Object has no Inserted-State (!)");
            getSdrModelFromSdrObjList().SetChanged();
            SetParentAtSdrObjectFromSdrObjList(*pObj, 0);

            if(!GetObjCount())
            {
                SdrObject* pOwningSdrObject = getSdrObjectFromSdrObjList();

                // empty group created; it needs to be repainted since it's
                // visualisation changes
                if(pOwningSdrObject)
                {
                    pOwningSdrObject->ActionChanged();
                }
            }
        }
        else
        {
            OSL_ENSURE(false, "removed non-existent object (!)");
        }

        return pObj;
    }
    else
    {
        OSL_ENSURE(false, "SdrObjList::RemoveObjectFromSdrObjList with invalid index (!)");
        return 0;
    }
}

SdrObject* SdrObjList::ReplaceObjectInSdrObjList(SdrObject& rNewObj, sal_uInt32 nObjNum)
{
    OSL_ENSURE(nObjNum < maList.size(), "SdrObjList::ReplaceObjectInSdrObjList with invalid index (!)");
    SdrObject* pRetval = RemoveObjectFromSdrObjList(nObjNum);
    InsertObjectToSdrObjList(rNewObj, nObjNum);

    return pRetval;
}

SdrObject* SdrObjList::SetNavigationPosition(sal_uInt32 nOldObjNum, sal_uInt32 nNewObjNum)
{
    if(nOldObjNum < maList.size() && nNewObjNum < maList.size())
    {
        SdrObject* pObj = *(maList.begin() + nOldObjNum);

        if(pObj)
        {
            if(nOldObjNum == nNewObjNum)
            {
                return pObj;
            }

            OSL_ENSURE(pObj->IsObjectInserted(), "Object has no Inserted-State (!)");

            {
                // broadcast immediately (before removal)
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj, HINT_OBJREMOVED);
            }

            RemoveObjectFromContainer(nOldObjNum);
            InsertObjectIntoContainer(*pObj, nNewObjNum);

            {
                // broadcast after insert
                const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObj, HINT_OBJINSERTED);
            }

            // #110094#No need to delete visualisation data since same object
            // gets inserted again. Also a single ActionChanged is enough
            pObj->ActionChanged();
            getSdrModelFromSdrObjList().SetChanged();

            return pObj;
        }
        else
        {
            OSL_ENSURE(false, "SdrObjList::SetNavigationPosition at non-existent object (!)");
        }
    }
    else
    {
        OSL_ENSURE(nOldObjNum < maList.size(), "SdrObjList::SetNavigationPosition with invalid old index (!)");
        OSL_ENSURE(nNewObjNum < maList.size(), "SdrObjList::SetNavigationPosition with invalid new index (!)");
    }

    return 0;
}

SdrObject* SdrObjList::GetObj(sal_uInt32 nNum) const
{
    if(maUserNavigationOrder.empty())
    {
        if(nNum < maList.size())
        {
            return *(maList.begin() + nNum);
        }
    }
    else
    {
        if(nNum < maUserNavigationOrder.size())
        {
            return *(maUserNavigationOrder.begin() + nNum);
        }
    }

    OSL_ENSURE(false, "SdrObjList::GetObj with wrong Index (!)");
    return 0;
}

void SdrObjList::FlattenGroups()
{
    for(sal_uInt32 nObjCount(GetObjCount()); nObjCount;)
    {
        nObjCount--;
        UnGroupObj(nObjCount);
    }
}

void SdrObjList::UnGroupObj(sal_uInt32 nObjNum)
{
    // if the given object is no group, this method is a noop
    SdrObjGroup* pUngroupObj = dynamic_cast< SdrObjGroup* >(GetObj(nObjNum));

    if( pUngroupObj )
    {
        // ungroup recursively (has to be head recursion,
        // otherwise our indices will get trashed when doing it in
        // the loop)
        pUngroupObj->FlattenGroups();

        // the position at which we insert the members of rUngroupGroup
        sal_uInt32 nInsertPos(pUngroupObj->GetNavigationPosition());
        const sal_uInt32 nAnz(pUngroupObj->GetObjCount());

        for(sal_uInt32 i(0); i < nAnz; i++)
        {
            SdrObject* pObj = pUngroupObj->RemoveObjectFromSdrObjList(0);
            InsertObjectToSdrObjList(*pObj, nInsertPos);
            nInsertPos++;
        }

        RemoveObjectFromSdrObjList(nInsertPos);
    }
    else
    {
        OSL_ENSURE(false, "SdrObjList::UnGroupObj: object index invalid");
    }
}

bool SdrObjList::HasUserNavigationOrder(void) const
{
    return !maUserNavigationOrder.empty();
}

void SdrObjList::SetUserNavigationPosition(SdrObject& rObject, const sal_uInt32 nNewPosition)
{
    // When the navigation order container has not yet been created then
    // create one now.  It is initialized with the z-order taken from
    // maList.
    bool bUnchanged(false);

    if(maUserNavigationOrder.empty())
    {
        maUserNavigationOrder = maList;
        bUnchanged = true;
    }

    OSL_ASSERT(!maUserNavigationOrder.empty());
    OSL_ASSERT(maUserNavigationOrder.size() == maList.size());

    // Look up the object whose navigation position is to be changed.
    SdrObjectVector::iterator iObject(::std::find(
        maUserNavigationOrder.begin(),
        maUserNavigationOrder.end(),
        &rObject));

    if(maUserNavigationOrder.end() == iObject)
    {
        // The given object is not a member of the navigation order.
        if(bUnchanged)
        {
            maUserNavigationOrder.clear();
        }

        return;
    }

    // Move the object to its new position.
    const sal_uInt32 nOldPosition(::std::distance(maUserNavigationOrder.begin(), iObject));

    if (nOldPosition != nNewPosition)
    {
        maUserNavigationOrder.erase(iObject);
        sal_uInt32 nInsertPosition (nNewPosition);

        // Adapt insertion position for the just erased object.
        if (nNewPosition >= nOldPosition)
        {
            nInsertPosition -= 1;
        }

        if(nInsertPosition >= maUserNavigationOrder.size())
        {
            maUserNavigationOrder.push_back(&rObject);
        }
        else
        {
            maUserNavigationOrder.insert(maUserNavigationOrder.begin() + nInsertPosition, &rObject);
        }

        // re-create OrdNums based on new order
        EnsureValidNavigationPositions(::std::min(nOldPosition, nNewPosition));

        // The navigation order is written out to file so mark the model as modified.
        getSdrModelFromSdrObjList().SetChanged();
    }
}

void SdrObjList::ClearUserNavigationOrder(void)
{
    if(!maUserNavigationOrder.empty())
    {
        maUserNavigationOrder.clear();
        EnsureValidNavigationPositions();
    }
}

void SdrObjList::SetUserNavigationOrder(const uno::Reference<container::XIndexAccess>& rxOrder)
{
    if (rxOrder.is())
    {
        const sal_Int32 nCount(rxOrder->getCount());

        if ((sal_uInt32)nCount != maList.size())
        {
            return;
        }

        maUserNavigationOrder.clear();
        maUserNavigationOrder.reserve(nCount);

        for(sal_Int32 nIndex(0); nIndex < nCount; nIndex++)
        {
            uno::Reference<uno::XInterface> xShape (rxOrder->getByIndex(nIndex), uno::UNO_QUERY);
            SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);

            if(!pObject)
                break;

            maUserNavigationOrder.push_back(pObject);
        }

        EnsureValidNavigationPositions();
    }
    else
    {
        ClearUserNavigationOrder();
    }
}

void SdrObjList::InsertObjectIntoContainer(SdrObject& rObject, const sal_uInt32 nInsertPosition)
{
    if(&rObject.getSdrModelFromSdrObject() != &getSdrModelFromSdrObjList())
    {
        OSL_ENSURE(false, "InsertObjectToSdrObjList with SdrModel of SdrObject != SdrModel from SdrObjList (!)");
        deleteSdrObjectSafe(&rObject);
        return;
    }

    // Update the navigation positions.
    if(!maUserNavigationOrder.empty())
    {
        // The new object does not have a user defined position so append it
        // to the list.
        maUserNavigationOrder.push_back(&rObject);
    }

    // Insert object into object list
    if (nInsertPosition >= maList.size())
    {
        maList.push_back(&rObject);
        SetOrdNumAtSdrObjectFromSdrObjList(rObject, maList.size() - 1);
    }
    else
    {
        maList.insert(maList.begin()+nInsertPosition, &rObject);
        EnsureValidNavigationPositions(nInsertPosition);
    }
}

void SdrObjList::RemoveObjectFromContainer(const sal_uInt32 nObjectPosition)
{
    if (nObjectPosition >= maList.size())
    {
        OSL_ASSERT(nObjectPosition<maList.size());
        return;
    }

    SdrObjectVector::iterator aListPosition(maList.begin() + nObjectPosition);

    // Update the navigation positions.
    if(!maUserNavigationOrder.empty())
    {
        SdrObjectVector::iterator iObject(::std::find(
            maUserNavigationOrder.begin(),
            maUserNavigationOrder.end(),
            *aListPosition));

        if(iObject != maUserNavigationOrder.end())
        {
            maUserNavigationOrder.erase(iObject);
        }
    }

    maList.erase(aListPosition);
    EnsureValidNavigationPositions(nObjectPosition);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageGridFrame::SdrPageGridFrame(const basegfx::B2DRange& rPaper)
:   maPaper(rPaper),
    maUserArea(rPaper)
{
}

SdrPageGridFrame::SdrPageGridFrame(const basegfx::B2DRange& rPaper, const basegfx::B2DRange& rUser)
:   maPaper(rPaper),
    maUserArea(rUser)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageGridFrameList::SdrPageGridFrameList()
:   maList()
{
}

SdrPageGridFrameList::~SdrPageGridFrameList()
{
    Clear();
}

SdrPageGridFrame* SdrPageGridFrameList::GetObject(sal_uInt32 i) const
{
    if(i < maList.size())
    {
        return *(maList.begin() + i);
    }
    else
    {
        OSL_ENSURE(false, "SdrPageGridFrameList::GetObject access out of range (!)");
        return 0;
    }
}

void SdrPageGridFrameList::Clear()
{
    for(SdrPageGridFrameContainerType::iterator aCandidate(maList.begin());
        aCandidate != maList.end(); aCandidate++)
    {
        delete *aCandidate;
    }

    maList.clear();
}

void SdrPageGridFrameList::Insert(const SdrPageGridFrame& rGF, sal_uInt32 nPos)
{
    if(nPos >= maList.size())
    {
        maList.push_back(new SdrPageGridFrame(rGF));
    }
    else
    {
        maList.insert(maList.begin() + nPos, new SdrPageGridFrame(rGF));
    }
}

void SdrPageGridFrameList::Delete(sal_uInt32 nPos)
{
    if(nPos < maList.size())
    {
        SdrPageGridFrameContainerType::iterator a(maList.begin() + nPos);
        delete *a;
        maList.erase(a);
    }
    else
    {
        OSL_ENSURE(false, "SdrPageGridFrameList::Delete with wrong index (!)");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageProperties::ImpRemoveStyleSheet()
{
    if(mpStyleSheet)
    {
        EndListening(*mpStyleSheet);
        mpProperties->SetParent(0);
        mpStyleSheet = 0;
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
    rSdrPage.getSdrModelFromSdrPage().SetChanged(true);
    rSdrPage.getSdrModelFromSdrPage().Broadcast(SdrBaseHint(rSdrPage, HINT_PAGEORDERCHG));
}

SdrPageProperties::SdrPageProperties(SdrPage& rSdrPage)
:   SfxListener(),
    mpSdrPage(&rSdrPage),
    mpStyleSheet(0),
    mpProperties(0)
{
    mpProperties = new SfxItemSet(mpSdrPage->getSdrModelFromSdrPage().GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

    if(!rSdrPage.IsMasterPage())
    {
        mpProperties->Put(XFillStyleItem(XFILL_NONE));
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

const SfxItemSet& SdrPageProperties::GetItemSet() const
{
    return *mpProperties;
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

SfxStyleSheet* SdrPageProperties::GetStyleSheet() const
{
    return mpStyleSheet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPage::AddListener(SfxListener& rListener)
{
    rListener.StartListening(*this);
}

void SdrPage::RemoveListener(SfxListener& rListener)
{
    rListener.EndListening(*this);
}

sdr::contact::ViewContact* SdrPage::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrPage(*this);
}

sdr::contact::ViewContact& SdrPage::GetViewContact() const
{
    if(!mpViewContact)
    {
        const_cast< SdrPage* >(this)->mpViewContact = const_cast< SdrPage* >(this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
}

SdrPage::SdrPage(SdrModel& rNewModel, bool bMasterPage)
:   SdrObjList(),
    SfxBroadcaster(),
    tools::WeakBase< SdrPage >(),
    mrSdrModelFromSdrPage(rNewModel),
    mpViewContact(0),
    mpSdrPageProperties(0),
    maComments(),
    mnPageNum(0),
    mpPageLayerAdmin(new SdrLayerAdmin(rNewModel, &rNewModel.GetModelLayerAdmin())),
    mpMasterPageDescriptor(0),
    mxUnoPage(),
    maPageScale(10.0, 10.0),
    mfLeftPageBorder(0.0),
    mfTopPageBorder(0.0),
    mfRightPageBorder(0.0),
    mfBottomPageBorder(0.0),
    mbMaster(bMasterPage),
    mbInserted(false),
    mbPageBorderOnlyLeftRight(false)
{
    // do NOT move this to the member initialisation; it internally will use
    // stuff from the SdrPage, in this case mbMaster (!)
    mpSdrPageProperties = new SdrPageProperties(*this);
}

SdrPage::~SdrPage()
{
    // Broadcast dying hint to registered SdrPageUsers to allow them to
    // no longer reference this page
    Broadcast(SdrBaseHint(*this, HINT_SDRPAGEDYING));

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

    delete mpPageLayerAdmin;

    TRG_ClearMasterPage();

    // #110094#
    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0;
    }

    {
        delete mpSdrPageProperties;
        mpSdrPageProperties = 0;
    }

    if(GetObjCount())
    {
        // cannot be called in SdrObjList::ClearSdrObjList() where it originally was
        // since there it would be a pure virtual function call. Needs to be called
        // in all destructors of classes derived from SdrObjList
        getSdrModelFromSdrObjList().SetChanged();
    }
}

void SdrPage::copyDataFromSdrPage(const SdrPage& rSource)
{
    if(this != &rSource)
    {
        if(mpViewContact)
        {
            delete mpViewContact;
            mpViewContact = 0;
        }

        // copy all the local parameters to make this instance
        // a valid copy od source page before copying and inserting
        // the contained objects
        mbMaster = rSource.mbMaster;
        mbPageBorderOnlyLeftRight = rSource.mbPageBorderOnlyLeftRight;
        maPageScale = rSource.maPageScale;
        mfLeftPageBorder = rSource.mfLeftPageBorder;
        mfTopPageBorder = rSource.mfTopPageBorder;
        mfRightPageBorder = rSource.mfRightPageBorder;
        mfBottomPageBorder = rSource.mfBottomPageBorder;
        mnPageNum = rSource.GetPageNumber();

        if(rSource.TRG_HasMasterPage())
        {
            TRG_SetMasterPage(rSource.TRG_GetMasterPage());
            TRG_SetMasterPageVisibleLayers(rSource.TRG_GetMasterPageVisibleLayers());
        }
        else
        {
            TRG_ClearMasterPage();
        }

        // delete SdrPageProperties when model is different
        if(mpSdrPageProperties && &getSdrModelFromSdrPage() != &rSource.getSdrModelFromSdrPage())
        {
            delete mpSdrPageProperties;
            mpSdrPageProperties = 0;
        }

        if(!mpSdrPageProperties)
        {
            mpSdrPageProperties = new SdrPageProperties(*this);
        }
        else
        {
            mpSdrPageProperties->ClearItem(0);
        }

        if(!IsMasterPage())
        {
            mpSdrPageProperties->PutItemSet(rSource.getSdrPageProperties().GetItemSet());
        }

        mpSdrPageProperties->SetStyleSheet(rSource.getSdrPageProperties().GetStyleSheet());

        // Now copy the contained obejcts (by cloning them)
        copyDataFromSdrObjList(rSource);
    }
}

SdrPage* SdrPage::CloneSdrPage(SdrModel* pTargetModel) const
{
    SdrPage* pClone = new SdrPage(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrPage(),
        IsMasterPage());
    OSL_ENSURE(pClone, "CloneSdrPage error (!)");
    pClone->copyDataFromSdrPage(*this);

    return pClone;
}

basegfx::B2DRange SdrPage::GetInnerPageRange() const
{
    return basegfx::B2DRange(
        mfLeftPageBorder,
        mfTopPageBorder,
        maPageScale.getX() - mfRightPageBorder,
        maPageScale.getY() - mfBottomPageBorder);
}

basegfx::B2DVector SdrPage::GetInnerPageScale() const
{
    return basegfx::B2DVector(
        maPageScale.getX() - (mfLeftPageBorder + mfRightPageBorder),
        maPageScale.getY() - (mfTopPageBorder + mfBottomPageBorder));
}

void SdrPage::SetPageScale(const basegfx::B2DVector& aNewScale)
{
    if(!aNewScale.equal(maPageScale))
    {
        maPageScale = aNewScale;
        SetChanged();
    }
}

void SdrPage::SetOrientation(Orientation eOri)
{
    const basegfx::B2DVector aOldPageScale(GetPageScale());

    if(!basegfx::fTools::equal(aOldPageScale.getX(), aOldPageScale.getY()))
    {
        if((ORIENTATION_PORTRAIT == eOri) == (aOldPageScale.getX() > aOldPageScale.getY()))
        {
            SetPageScale(basegfx::B2DVector(aOldPageScale.getY(), aOldPageScale.getX()));
        }
    }
}

Orientation SdrPage::GetOrientation() const
{
    Orientation eRet(ORIENTATION_PORTRAIT);
    const basegfx::B2DVector& rPageScale(GetPageScale());

    if(rPageScale.getX() > rPageScale.getY())
    {
        eRet = ORIENTATION_LANDSCAPE;
    }

    return eRet;
}

void SdrPage::SetPageBorder(double fLeft, double fTop, double fRight, double fBottom)
{
    bool bChanged(false);

    if(mfLeftPageBorder != fLeft)
    {
        mfLeftPageBorder = fLeft;
        bChanged = true;
    }

    if(mfTopPageBorder != fTop)
    {
        mfTopPageBorder = fTop;
        bChanged = true;
    }

    if(mfRightPageBorder != fRight)
    {
        mfRightPageBorder = fRight;
        bChanged = true;
    }

    if(mfBottomPageBorder != fBottom)
    {
        mfBottomPageBorder = fBottom;
        bChanged = true;
    }

    if(bChanged)
    {
        SetChanged();
    }
}

void  SdrPage::SetLeftPageBorder(double fBorder)
{
    if(mfLeftPageBorder != fBorder)
    {
        mfLeftPageBorder = fBorder;
        SetChanged();
    }
}

void  SdrPage::SetTopPageBorder(double fBorder)
{
    if(mfTopPageBorder != fBorder)
    {
        mfTopPageBorder = fBorder;
        SetChanged();
    }
}

void  SdrPage::SetRightPageBorder(double fBorder)
{
    if(mfRightPageBorder != fBorder)
    {
        mfRightPageBorder = fBorder;
        SetChanged();
    }
}

void  SdrPage::SetBottomPageBorder(double fBorder)
{
    if(mfBottomPageBorder != fBorder)
    {
        mfBottomPageBorder = fBorder;
        SetChanged();
    }
}

double SdrPage::GetLeftPageBorder() const
{
    return mfLeftPageBorder;
}

double SdrPage::GetTopPageBorder() const
{
    return mfTopPageBorder;
}

double SdrPage::GetRightPageBorder() const
{
    return mfRightPageBorder;
}

double SdrPage::GetBottomPageBorder() const
{
    return mfBottomPageBorder;
}

SdrModel& SdrPage::getSdrModelFromSdrObjList() const
{
    return getSdrModelFromSdrPage();
}

// only called from SetPageNumberAtSdrPageFromSdrModel (see svdmodel.cxx)
void SdrPage::SetPageNumber(sal_uInt32 nNew)
{
    if(nNew != GetPageNumber())
    {
        // change
        mnPageNum = nNew;

        // notify visualisations, also notifies e.g. buffered MasterPages
        ActionChanged();
    }
}

void SdrPage::SetInserted(bool bInserted)
{
    if(bInserted != IsInserted())
    {
        mbInserted = bInserted;

        // TTTT: Check if the original's travel over OLEs and setting them to connect/disconnect is needed or moved elsewhere
    }
}

sal_uInt32 SdrPage::GetPageNumber() const
{
    if(!IsInserted())
    {
        return 0;
    }

    return mnPageNum;
}

void SdrPage::SetChanged()
{
    ActionChanged();
    getSdrModelFromSdrPage().SetChanged();
}

void SdrPage::TRG_SetMasterPage(SdrPage& rNew)
{
    if(TRG_HasMasterPage() && &(mpMasterPageDescriptor->GetUsedPage()) == &rNew)
    {
        return;
    }

    if(TRG_HasMasterPage())
    {
        TRG_ClearMasterPage();
    }

    mpMasterPageDescriptor = new ::sdr::MasterPageDescriptor(*this, rNew);

    GetViewContact().ActionChanged();
}

void SdrPage::TRG_ClearMasterPage()
{
    if(TRG_HasMasterPage())
    {
        SetChanged();

        // the flushViewObjectContacts() will do needed invalidates by deleting the involved VOCs
        mpMasterPageDescriptor->GetUsedPage().GetViewContact().flushViewObjectContacts(true);

        delete mpMasterPageDescriptor;
        mpMasterPageDescriptor = 0;
    }
}

SdrPage& SdrPage::TRG_GetMasterPage() const
{
    OSL_ENSURE(TRG_HasMasterPage(), "TRG_GetMasterPage(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetUsedPage();
}

const SetOfByte& SdrPage::TRG_GetMasterPageVisibleLayers() const
{
    OSL_ENSURE(TRG_HasMasterPage(), "TRG_GetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetVisibleLayers();
}

void SdrPage::TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew)
{
    OSL_ENSURE(TRG_HasMasterPage(), "TRG_SetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    mpMasterPageDescriptor->SetVisibleLayers(rNew);
}

sdr::contact::ViewContact& SdrPage::TRG_GetMasterPageDescriptorViewContact() const
{
    OSL_ENSURE(TRG_HasMasterPage(), "TRG_GetMasterPageDescriptorViewContact(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetViewContact();
}

void SdrPage::TRG_MasterPageRemoved(const SdrPage& rRemovedPage)
{
    if(TRG_HasMasterPage())
    {
        if(&TRG_GetMasterPage() == &rRemovedPage)
        {
            TRG_ClearMasterPage();
        }
    }
}

const SdrPageGridFrameList* SdrPage::GetGridFrameList(const SdrView& /*rSdrView*/, const Rectangle* /*pRect*/) const
{
    return 0;
}

XubString SdrPage::GetLayoutName() const
{
    return String();
}

uno::Reference< uno::XInterface > SdrPage::getUnoPage()
{
    // try weak reference first
    if( !mxUnoPage.is() )
    {
        // create one
        mxUnoPage = createUnoPage();
    }

    return mxUnoPage;
}

uno::Reference< uno::XInterface > SdrPage::createUnoPage()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt;

    if(dynamic_cast< FmFormModel* >(&getSdrModelFromSdrPage()))
    {
        // when no model be careful and construct a SvxFmDrawPage
        xInt = static_cast<cppu::OWeakObject*>( new SvxFmDrawPage( this ) );
    }
    else
    {
        xInt = static_cast<cppu::OWeakObject*>( new SvxDrawPage( this ) );
    }

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
        if(XFILL_NONE == ((const XFillStyleItem&)pBackgroundFill->Get(XATTR_FILLSTYLE)).GetValue())
        {
            pBackgroundFill = &TRG_GetMasterPage().getSdrPageProperties().GetItemSet();
        }
    }

    GetDraftFillColor(*pBackgroundFill, aColor);

    return aColor;
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

// #110094# DrawContact support: Methods for handling Page changes
void SdrPage::ActionChanged() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();

    // #i48535# also handle MasterPage change
    if(TRG_HasMasterPage())
    {
        TRG_GetMasterPageDescriptorViewContact().ActionChanged();
    }
}

SdrPage* SdrPage::getSdrPageFromSdrObjList() const
{
    return const_cast< SdrPage* >(this);
}

bool SdrPage::isHandoutMasterPage() const
{
    if(IsMasterPage())
    {
        return getSdrModelFromSdrPage().GetMasterPageCount()
            && getSdrModelFromSdrPage().GetMasterPage(0) == this;
    }

    return false;
}

const sdr::Comment& SdrPage::GetCommentByIndex(sal_uInt32 nIndex)
{
    DBG_ASSERT(nIndex < maComments.size(), "SdrPage::GetCommentByIndex: Access out of range (!)");
    return maComments[nIndex];
}

void SdrPage::AddComment(const sdr::Comment& rNew)
{
    maComments.push_back(rNew);
    ::std::sort(maComments.begin(), maComments.end());
}

void SdrPage::ReplaceCommentByIndex(sal_uInt32 nIndex, const sdr::Comment& rNew)
{
    DBG_ASSERT(nIndex < maComments.size(), "SdrPage::GetCommentByIndex: Access out of range (!)");

    if(maComments[nIndex] != rNew)
    {
        maComments[nIndex] = rNew;
        ::std::sort(maComments.begin(), maComments.end());
    }
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

//////////////////////////////////////////////////////////////////////////////

StandardCheckVisisbilityRedirector::StandardCheckVisisbilityRedirector()
:   ViewObjectContactRedirector()
{
}

StandardCheckVisisbilityRedirector::~StandardCheckVisisbilityRedirector()
{
}

drawinglayer::primitive2d::Primitive2DSequence StandardCheckVisisbilityRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        SdrPage* pOwningSdrPage = pObject->getSdrPageFromSdrObject();

        if(pOwningSdrPage)
        {
            if(pOwningSdrPage->checkVisibility(rOriginal, rDisplayInfo, false))
            {
                return ::sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
            }
        }

        return drawinglayer::primitive2d::Primitive2DSequence();
    }
    else
    {
        // not an object, maybe a page
        return ::sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
