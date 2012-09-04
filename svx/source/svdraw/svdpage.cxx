/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <svx/svdpage.hxx>

// HACK
#include <sot/storage.hxx>
#include <sot/clsids.hxx>
#include <svx/svdview.hxx>
#include <string.h>
#ifndef _STRING_H
#define _STRING_H
#endif
#include <vcl/svapp.hxx>

#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>

#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdograf.hxx> // for SwapInAll()
#include <svx/svdoedge.hxx> // for copying the connectors
#include <svx/svdoole2.hxx> // special case OLE at SdrExchangeFormat
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
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <algorithm>
#include <svl/smplhint.hxx>
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;

class SdrObjList::WeakSdrObjectContainerType
    : public ::std::vector<SdrObjectWeakRef>
{
public:
    WeakSdrObjectContainerType (const sal_Int32 nInitialSize)
        : ::std::vector<SdrObjectWeakRef>(nInitialSize) {};
};



static const sal_Int32 InitialObjectContainerCapacity (64);
DBG_NAME(SdrObjList)

TYPEINIT0(SdrObjList);

SdrObjList::SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList):
    maList(),
    mpNavigationOrder(),
    mbIsNavigationOrderDirty(false)
{
    DBG_CTOR(SdrObjList,NULL);
    maList.reserve(InitialObjectContainerCapacity);
    pModel=pNewModel;
    pPage=pNewPage;
    pUpList=pNewUpList;
    bObjOrdNumsDirty=sal_False;
    bRectsDirty=sal_False;
    pOwnerObj=NULL;
    eListKind=SDROBJLIST_UNKNOWN;
}

SdrObjList::SdrObjList(const SdrObjList& rSrcList):
    maList(),
    mpNavigationOrder(),
    mbIsNavigationOrderDirty(false)
{
    DBG_CTOR(SdrObjList,NULL);
    maList.reserve(InitialObjectContainerCapacity);
    pModel=NULL;
    pPage=NULL;
    pUpList=NULL;
    bObjOrdNumsDirty=sal_False;
    bRectsDirty=sal_False;
    pOwnerObj=NULL;
    eListKind=SDROBJLIST_UNKNOWN;
    *this=rSrcList;
}

SdrObjList::~SdrObjList()
{
    DBG_DTOR(SdrObjList,NULL);

    // To avoid that the Clear() method will broadcast changes when in destruction
    // which would call virtual methos (not allowed in destructor), the model is set
    // to NULL here.
    pModel = 0L;

    Clear(); // delete contents of container
}

void SdrObjList::operator=(const SdrObjList& rSrcList)
{
    Clear();
    eListKind=rSrcList.eListKind;
    CopyObjects(rSrcList);
}

void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
{
    Clear();
    bObjOrdNumsDirty=sal_False;
    bRectsDirty     =sal_False;
    sal_uIntPtr nCloneErrCnt=0;
    sal_uIntPtr nAnz=rSrcList.GetObjCount();
    SdrInsertReason aReason(SDRREASON_COPY);
    sal_uIntPtr no;
    for (no=0; no<nAnz; no++) {
        SdrObject* pSO=rSrcList.GetObj(no);

        SdrObject* pDO = pSO->Clone();

        if (pDO!=NULL) {
            pDO->SetModel(pModel);
            pDO->SetPage(pPage);
            NbcInsertObject(pDO,CONTAINER_APPEND,&aReason);
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
        for (no=0; no<nAnz; no++) {
            const SdrObject* pSrcOb=rSrcList.GetObj(no);
            SdrEdgeObj* pSrcEdge=PTR_CAST(SdrEdgeObj,pSrcOb);
            if (pSrcEdge!=NULL) {
                SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(sal_True);
                SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(sal_False);
                if (pSrcNode1!=NULL && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=NULL; // can't do this
                if (pSrcNode2!=NULL && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=NULL; // across all lists (yet)
                if (pSrcNode1!=NULL || pSrcNode2!=NULL) {
                    SdrObject* pEdgeObjTmp=GetObj(no);
                    SdrEdgeObj* pDstEdge=PTR_CAST(SdrEdgeObj,pEdgeObjTmp);
                    if (pDstEdge!=NULL) {
                        if (pSrcNode1!=NULL) {
                            sal_uIntPtr nDstNode1=pSrcNode1->GetOrdNum();
                            SdrObject* pDstNode1=GetObj(nDstNode1);
                            if (pDstNode1!=NULL) { // else we get an error!
                                pDstEdge->ConnectToNode(sal_True,pDstNode1);
                            } else {
                                OSL_FAIL("SdrObjList::operator=(): pDstNode1==NULL!");
                            }
                        }
                        if (pSrcNode2!=NULL) {
                            sal_uIntPtr nDstNode2=pSrcNode2->GetOrdNum();
                            SdrObject* pDstNode2=GetObj(nDstNode2);
                            if (pDstNode2!=NULL) { // else the node was probably not selected
                                pDstEdge->ConnectToNode(sal_False,pDstNode2);
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
        rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
            "SdrObjList::operator=(): Error when cloning "));

        if(nCloneErrCnt == 1)
        {
            aStr.append(RTL_CONSTASCII_STRINGPARAM("a drawing object."));
        }
        else
        {
            aStr.append(static_cast<sal_Int32>(nCloneErrCnt));
            aStr.append(RTL_CONSTASCII_STRINGPARAM(" drawing objects."));
        }

        aStr.append(RTL_CONSTASCII_STRINGPARAM(
            " Not copying connectors."));

        OSL_FAIL(aStr.getStr());
#endif
    }
}

void SdrObjList::Clear()
{
    sal_Bool bObjectsRemoved(sal_False);

    while( ! maList.empty())
    {
        // remove last object from list
        SdrObject* pObj = maList.back();
        RemoveObjectFromContainer(maList.size()-1);

        // flushViewObjectContacts() is done since SdrObject::Free is not guaranteed
        // to delete the object and thus refresh visualisations
        pObj->GetViewContact().flushViewObjectContacts(true);

        bObjectsRemoved = sal_True;

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
        sal_uIntPtr nAnz=GetObjCount();
        for (sal_uIntPtr no=0; no<nAnz; no++) {
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
        sal_uIntPtr nAnz=GetObjCount();
        for (sal_uIntPtr i=0; i<nAnz; i++) {
            SdrObject* pObj=GetObj(i);
            pObj->SetModel(pModel);
        }
    }
}

void SdrObjList::RecalcObjOrdNums()
{
    sal_uIntPtr nAnz=GetObjCount();
    for (sal_uIntPtr no=0; no<nAnz; no++) {
        SdrObject* pObj=GetObj(no);
        pObj->SetOrdNum(no);
    }
    bObjOrdNumsDirty=sal_False;
}

void SdrObjList::RecalcRects()
{
    aOutRect=Rectangle();
    aSnapRect=aOutRect;
    sal_uIntPtr nAnz=GetObjCount();
    sal_uIntPtr i;
    for (i=0; i<nAnz; i++) {
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
    bRectsDirty=sal_True;
    if (pUpList!=NULL) pUpList->SetRectsDirty();
}

void SdrObjList::impChildInserted(SdrObject& rChild) const
{
    sdr::contact::ViewContact* pParent = rChild.GetViewContact().GetParentContact();

    if(pParent)
    {
        pParent->ActionChildInserted(rChild.GetViewContact());
    }
}

void SdrObjList::NbcInsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* /*pReason*/)
{
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcInsertObject(NULL)");
    if (pObj!=NULL) {
        DBG_ASSERT(!pObj->IsInserted(),"ZObjekt already has the status Inserted.");
        sal_uIntPtr nAnz=GetObjCount();
        if (nPos>nAnz) nPos=nAnz;
        InsertObjectIntoContainer(*pObj,nPos);

        if (nPos<nAnz) bObjOrdNumsDirty=sal_True;
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
        pObj->SetInserted(sal_True); // calls the UserCall (among others)
    }
}

void SdrObjList::InsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* pReason)
{
    DBG_ASSERT(pObj!=NULL,"SdrObjList::InsertObject(NULL)");

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

SdrObject* SdrObjList::NbcRemoveObject(sal_uIntPtr nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return NULL;
    }

    sal_uIntPtr nAnz=GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=NULL,"Could not find object to remove.");
    if (pObj!=NULL) {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        DBG_ASSERT(pObj->IsInserted(),"ZObjekt does not have the status Inserted.");
        pObj->SetInserted(sal_False); // Ruft u.a. den UserCall
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        if (!bObjOrdNumsDirty) { // optimizing for the case that the last object has to be removed
            if (nObjNum!=sal_uIntPtr(nAnz-1)) {
                bObjOrdNumsDirty=sal_True;
            }
        }
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::RemoveObject(sal_uIntPtr nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return NULL;
    }

    sal_uIntPtr nAnz=GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=NULL,"Object to remove not found.");
    if(pObj)
    {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        DBG_ASSERT(pObj->IsInserted(),"ZObjekt does not have the status Inserted.");
        if (pModel!=NULL) {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=NULL) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
            pModel->SetChanged();
        }
        pObj->SetInserted(sal_False); // calls, among other things, the UserCall
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        if (!bObjOrdNumsDirty) { // optimization for the case that the last object is removed
            if (nObjNum!=sal_uIntPtr(nAnz-1)) {
                bObjOrdNumsDirty=sal_True;
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

SdrObject* SdrObjList::NbcReplaceObject(SdrObject* pNewObj, sal_uIntPtr nObjNum)
{
    if (nObjNum >= maList.size() || pNewObj == NULL)
    {
        OSL_ASSERT(nObjNum<maList.size());
        OSL_ASSERT(pNewObj!=NULL);
        return NULL;
    }

    SdrObject* pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Could not find object to remove.");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt does not have status Inserted.");
        pObj->SetInserted(sal_False);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(sal_True);
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::ReplaceObject(SdrObject* pNewObj, sal_uIntPtr nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return NULL;
    }
    if (pNewObj == NULL)
    {
        OSL_ASSERT(pNewObj!=NULL);
        return NULL;
    }

    SdrObject* pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Could not find object to remove.");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt does not have status Inserted.");
        if (pModel!=NULL) {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=NULL) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
        }
        pObj->SetInserted(sal_False);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(sal_True);
        if (pModel!=NULL) {
            // TODO: We need a different broadcast here.
            if (pNewObj->GetPage()!=NULL) {
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

SdrObject* SdrObjList::NbcSetObjectOrdNum(sal_uIntPtr nOldObjNum, sal_uIntPtr nNewObjNum)
{
    if (nOldObjNum >= maList.size() || nNewObjNum >= maList.size())
    {
        OSL_ASSERT(nOldObjNum<maList.size());
        OSL_ASSERT(nNewObjNum<maList.size());
        return NULL;
    }

    SdrObject* pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcSetObjectOrdNum: Object not found.");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::NbcSetObjectOrdNum: ZObjekt does not have status Inserted.");
        RemoveObjectFromContainer(nOldObjNum);

        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=sal_True;
    }
    return pObj;
}

SdrObject* SdrObjList::SetObjectOrdNum(sal_uIntPtr nOldObjNum, sal_uIntPtr nNewObjNum)
{
    if (nOldObjNum >= maList.size() || nNewObjNum >= maList.size())
    {
        OSL_ASSERT(nOldObjNum<maList.size());
        OSL_ASSERT(nNewObjNum<maList.size());
        return NULL;
    }

    SdrObject* pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::SetObjectOrdNum: Object not found.");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: ZObjekt does not have status Inserted.");
        RemoveObjectFromContainer(nOldObjNum);
        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=sal_True;
        if (pModel!=NULL)
        {
            // TODO: We need a different broadcast here.
            if (pObj->GetPage()!=NULL) pModel->Broadcast(SdrHint(*pObj));
            pModel->SetChanged();
        }
    }
    return pObj;
}

const Rectangle& SdrObjList::GetAllObjSnapRect() const
{
    if (bRectsDirty) {
        ((SdrObjList*)this)->RecalcRects();
        ((SdrObjList*)this)->bRectsDirty=sal_False;
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
        ((SdrObjList*)this)->RecalcRects();
        ((SdrObjList*)this)->bRectsDirty=sal_False;
    }
    return aOutRect;
}

void SdrObjList::NbcReformatAllTextObjects()
{
    sal_uIntPtr nAnz=GetObjCount();
    sal_uIntPtr nNum=0;

    while (nNum<nAnz)
    {
        SdrObject* pObj = GetObj(nNum);

        pObj->NbcReformatText();
        nAnz=GetObjCount();             // ReformatText may delete an object
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
    const sal_uInt32 nCount=GetObjCount();
    sal_uInt32 nObj;

    for( nObj = 0; nObj < nCount; nObj++ )
    {
        SdrObject* pObj = GetObj(nObj);
        if( pObj->ISA(SdrEdgeObj) )
            static_cast<SdrEdgeObj*>(pObj)->Reformat();
    }
}

void SdrObjList::BurnInStyleSheetAttributes()
{
    for(sal_uInt32 a(0L); a < GetObjCount(); a++)
    {
        GetObj(a)->BurnInStyleSheetAttributes();
    }
}

sal_uIntPtr SdrObjList::GetObjCount() const
{
    return maList.size();
}




SdrObject* SdrObjList::GetObj(sal_uIntPtr nNum) const
{
    if (nNum >= maList.size())
    {
        OSL_ASSERT(nNum<maList.size());
        return NULL;
    }
    else
        return maList[nNum];
}




bool SdrObjList::IsReadOnly() const
{
    bool bRet = false;
    if (pPage!=NULL && pPage!=this) bRet=pPage->IsReadOnly();
    return bRet;
}

sal_uIntPtr SdrObjList::CountAllObjects() const
{
    sal_uIntPtr nCnt=GetObjCount();
    sal_uIntPtr nAnz=nCnt;
    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++) {
        SdrObjList* pSubOL=GetObj(nNum)->GetSubList();
        if (pSubOL!=NULL) {
            nCnt+=pSubOL->CountAllObjects();
        }
    }
    return nCnt;
}

void SdrObjList::ForceSwapInObjects() const
{
    sal_uIntPtr nObjAnz=GetObjCount();
    for (sal_uIntPtr nObjNum=nObjAnz; nObjNum>0;) {
        SdrObject* pObj=GetObj(--nObjNum);
        SdrGrafObj* pGrafObj=PTR_CAST(SdrGrafObj,pObj);
        if (pGrafObj!=NULL) {
            pGrafObj->ForceSwapIn();
        }
        SdrObjList* pOL=pObj->GetSubList();
        if (pOL!=NULL) {
            pOL->ForceSwapInObjects();
        }
    }
}

void SdrObjList::ForceSwapOutObjects() const
{
    sal_uIntPtr nObjAnz=GetObjCount();
    for (sal_uIntPtr nObjNum=nObjAnz; nObjNum>0;) {
        SdrObject* pObj=GetObj(--nObjNum);
        SdrGrafObj* pGrafObj=PTR_CAST(SdrGrafObj,pObj);
        if (pGrafObj!=NULL) {
            pGrafObj->ForceSwapOut();
        }
        SdrObjList* pOL=pObj->GetSubList();
        if (pOL!=NULL) {
            pOL->ForceSwapOutObjects();
        }
    }
}

void SdrObjList::FlattenGroups()
{
    sal_Int32 nObj = GetObjCount();
    sal_Int32 i;
    for( i=nObj-1; i>=0; --i)
        UnGroupObj(i);
}

void SdrObjList::UnGroupObj( sal_uIntPtr nObjNum )
{
    // if the given object is no group, this method is a noop
    SdrObject* pUngroupObj = GetObj( nObjNum );
    if( pUngroupObj )
    {
        SdrObjList* pSrcLst = pUngroupObj->GetSubList();
        if( pUngroupObj->ISA( SdrObjGroup ) && pSrcLst )
        {
            SdrObjGroup* pUngroupGroup = static_cast< SdrObjGroup* > (pUngroupObj);

            // ungroup recursively (has to be head recursion,
            // otherwise our indices will get trashed when doing it in
            // the loop)
            pSrcLst->FlattenGroups();

            // the position at which we insert the members of rUngroupGroup
            sal_Int32 nInsertPos( pUngroupGroup->GetOrdNum() );

            SdrObject* pObj;
            sal_Int32 i, nAnz = pSrcLst->GetObjCount();
            for( i=0; i<nAnz; ++i )
            {
                pObj = pSrcLst->RemoveObject(0);
                SdrInsertReason aReason(SDRREASON_VIEWCALL, pUngroupGroup);
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




bool SdrObjList::HasObjectNavigationOrder (void) const
{
    return mpNavigationOrder.get() != NULL;
}




void SdrObjList::SetObjectNavigationPosition (
    SdrObject& rObject,
    const sal_uInt32 nNewPosition)
{
    // When the navigation order container has not yet been created then
    // create one now.  It is initialized with the z-order taken from
    // maList.
    if (mpNavigationOrder.get() == NULL)
    {
        mpNavigationOrder.reset(new WeakSdrObjectContainerType(maList.size()));
        ::std::copy(
            maList.begin(),
            maList.end(),
            mpNavigationOrder->begin());
    }
    OSL_ASSERT(mpNavigationOrder.get()!=NULL);
    OSL_ASSERT( mpNavigationOrder->size() == maList.size());

    SdrObjectWeakRef aReference (&rObject);

    // Look up the object whose navigation position is to be changed.
    WeakSdrObjectContainerType::iterator iObject (::std::find(
        mpNavigationOrder->begin(),
        mpNavigationOrder->end(),
        aReference));
    if (iObject == mpNavigationOrder->end())
    {
        // The given object is not a member of the navigation order.
        return;
    }

    // Move the object to its new position.
    const sal_uInt32 nOldPosition = ::std::distance(mpNavigationOrder->begin(), iObject);
    if (nOldPosition != nNewPosition)
    {
        mpNavigationOrder->erase(iObject);
        sal_uInt32 nInsertPosition (nNewPosition);
        // Adapt insertion position for the just erased object.
        if (nNewPosition >= nOldPosition)
            nInsertPosition -= 1;
        if (nInsertPosition >= mpNavigationOrder->size())
            mpNavigationOrder->push_back(aReference);
        else
            mpNavigationOrder->insert(mpNavigationOrder->begin()+nInsertPosition, aReference);

        mbIsNavigationOrderDirty = true;

        // The navigation order is written out to file so mark the model as modified.
        if (pModel != NULL)
            pModel->SetChanged();
    }
}




SdrObject* SdrObjList::GetObjectForNavigationPosition (const sal_uInt32 nNavigationPosition) const
{
    if (HasObjectNavigationOrder())
    {
        // There is a user defined navigation order. Make sure the object
        // index is correct and look up the object in mpNavigationOrder.
        if (nNavigationPosition >= mpNavigationOrder->size())
        {
            OSL_ASSERT(nNavigationPosition < mpNavigationOrder->size());
        }
        else
            return (*mpNavigationOrder)[nNavigationPosition].get();
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
    return NULL;
}




void SdrObjList::ClearObjectNavigationOrder (void)
{
    mpNavigationOrder.reset();
    mbIsNavigationOrderDirty = true;
}




bool SdrObjList::RecalcNavigationPositions (void)
{
    if (mbIsNavigationOrderDirty)
    {
        if (mpNavigationOrder.get() != NULL)
        {
            mbIsNavigationOrderDirty = false;

            WeakSdrObjectContainerType::iterator iObject;
            WeakSdrObjectContainerType::const_iterator iEnd (mpNavigationOrder->end());
            sal_uInt32 nIndex (0);
            for (iObject=mpNavigationOrder->begin(); iObject!=iEnd; ++iObject,++nIndex)
                (*iObject)->SetNavigationPosition(nIndex);
        }
    }

    return mpNavigationOrder.get() != NULL;
}




void SdrObjList::SetNavigationOrder (const uno::Reference<container::XIndexAccess>& rxOrder)
{
    if (rxOrder.is())
    {
        const sal_Int32 nCount = rxOrder->getCount();
        if ((sal_uInt32)nCount != maList.size())
            return;

        if (mpNavigationOrder.get() == NULL)
            mpNavigationOrder.reset(new WeakSdrObjectContainerType(nCount));

        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {
            uno::Reference<uno::XInterface> xShape (rxOrder->getByIndex(nIndex), uno::UNO_QUERY);
            SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
            if (pObject == NULL)
                break;
            (*mpNavigationOrder)[nIndex] = pObject;
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
        rObject.SetNavigationPosition(mpNavigationOrder->size());
        mpNavigationOrder->push_back(&rObject);
    }

    // Insert object into object list.  Because the insert() method requires
    // a valid iterator as insertion position, we have to use push_back() to
    // insert at the end of the list.
    if (nInsertPosition >= maList.size())
        maList.push_back(&rObject);
    else
        maList.insert(maList.begin()+nInsertPosition, &rObject);
    bObjOrdNumsDirty=sal_True;
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
            mpNavigationOrder->begin(),
            mpNavigationOrder->end(),
            aReference));
        if (iObject != mpNavigationOrder->end())
            mpNavigationOrder->erase(iObject);

        mpNavigationOrder->push_back(&rNewObject);

        mbIsNavigationOrderDirty = true;
    }

    maList[nObjectPosition] = &rNewObject;
    bObjOrdNumsDirty=sal_True;
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
            mpNavigationOrder->begin(),
            mpNavigationOrder->end(),
            aReference));
        if (iObject != mpNavigationOrder->end())
            mpNavigationOrder->erase(iObject);
        mbIsNavigationOrderDirty = true;
    }

    maList.erase(maList.begin()+nObjectPosition);
    bObjOrdNumsDirty=sal_True;
}




////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageGridFrameList::Clear()
{
    sal_uInt16 nAnz=GetCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// PageUser section

void SdrPage::AddPageUser(sdr::PageUser& rNewUser)
{
    maPageUsers.push_back(&rNewUser);
}

void SdrPage::RemovePageUser(sdr::PageUser& rOldUser)
{
    const ::sdr::PageUserVector::iterator aFindResult = ::std::find(maPageUsers.begin(), maPageUsers.end(), &rOldUser);
    if(aFindResult != maPageUsers.end())
    {
        maPageUsers.erase(aFindResult);
    }
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrPage::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrPage(*this);
}

sdr::contact::ViewContact& SdrPage::GetViewContact() const
{
    if(!mpViewContact)
    {
        const_cast< SdrPage* >(this)->mpViewContact =
            const_cast< SdrPage* >(this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
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

    if(rSdrPage.GetModel())
    {
        rSdrPage.GetModel()->SetChanged(true);
        SdrHint aHint(HINT_PAGEORDERCHG);
        aHint.SetPage(&rSdrPage);
        rSdrPage.GetModel()->Broadcast(aHint);
    }
}

SdrPageProperties::SdrPageProperties(SdrPage& rSdrPage)
:   SfxListener(),
    mpSdrPage(&rSdrPage),
    mpStyleSheet(0),
    mpProperties(new SfxItemSet(mpSdrPage->GetModel()->GetItemPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST))
{
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

TYPEINIT1(SdrPage,SdrObjList);
DBG_NAME(SdrPage)
SdrPage::SdrPage(SdrModel& rNewModel, bool bMasterPage)
:   SdrObjList(&rNewModel, this),
    mpViewContact(0L),
    nWdt(10L),
    nHgt(10L),
    nBordLft(0L),
    nBordUpp(0L),
    nBordRgt(0L),
    nBordLwr(0L),
    pLayerAdmin(new SdrLayerAdmin(&rNewModel.GetLayerAdmin())),
    mpSdrPageProperties(0),
    mpMasterPageDescriptor(0L),
    nPageNum(0L),
    mbMaster(bMasterPage),
    mbInserted(false),
    mbObjectsNotPersistent(false),
    mbSwappingLocked(false),
    mbPageBorderOnlyLeftRight(false)
{
    DBG_CTOR(SdrPage,NULL);
    aPrefVisiLayers.SetAll();
    eListKind = (bMasterPage) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;

    mpSdrPageProperties = new SdrPageProperties(*this);
}

SdrPage::SdrPage(const SdrPage& rSrcPage)
:   SdrObjList(rSrcPage.pModel, this),
    tools::WeakBase< SdrPage >(),
    mpViewContact(0L),
    nWdt(rSrcPage.nWdt),
    nHgt(rSrcPage.nHgt),
    nBordLft(rSrcPage.nBordLft),
    nBordUpp(rSrcPage.nBordUpp),
    nBordRgt(rSrcPage.nBordRgt),
    nBordLwr(rSrcPage.nBordLwr),
    pLayerAdmin(new SdrLayerAdmin(rSrcPage.pModel->GetLayerAdmin())),
    mpSdrPageProperties(0),
    mpMasterPageDescriptor(0L),
    nPageNum(rSrcPage.nPageNum),
    mbMaster(rSrcPage.mbMaster),
    mbInserted(false),
    mbObjectsNotPersistent(rSrcPage.mbObjectsNotPersistent),
    mbSwappingLocked(rSrcPage.mbSwappingLocked),
    mbPageBorderOnlyLeftRight(rSrcPage.mbPageBorderOnlyLeftRight)
{
    DBG_CTOR(SdrPage,NULL);
    aPrefVisiLayers.SetAll();
    eListKind = (mbMaster) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;

    // copy things from source
    // Warning: this leads to slicing (see issue 93186) and has to be
    // removed as soon as possible.
    *this = rSrcPage;
    OSL_ENSURE(mpSdrPageProperties,
        "SdrPage::SdrPage: operator= did not create needed SdrPageProperties (!)");

    // be careful and correct eListKind, a member of SdrObjList which
    // will be changed by the SdrOIbjList::operator= before...
    eListKind = (mbMaster) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;

    // The previous assignment to *this may have resulted in a call to
    // createUnoPage at a partially initialized (sliced) SdrPage object.
    // Due to the vtable being not yet fully set-up at this stage,
    // createUnoPage() may have been called at the wrong class.
    // To force a call to the right createUnoPage() at a later time when the
    // new object is full constructed mxUnoPage is disposed now.
    uno::Reference<lang::XComponent> xComponent (mxUnoPage, uno::UNO_QUERY);
    if (xComponent.is())
    {
        mxUnoPage = NULL;
        xComponent->dispose();
    }
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
    ::sdr::PageUserVector aListCopy (maPageUsers.begin(), maPageUsers.end());
    for(::sdr::PageUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); ++aIterator)
    {
        sdr::PageUser* pPageUser = *aIterator;
        DBG_ASSERT(pPageUser, "SdrPage::~SdrPage: corrupt PageUser list (!)");
        pPageUser->PageInDestruction(*this);
    }

    // Clear the vector. This means that user do not need to call RemovePageUser()
    // when they get called from PageInDestruction().
    maPageUsers.clear();

    delete pLayerAdmin;

    TRG_ClearMasterPage();

    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }

    {
        delete mpSdrPageProperties;
        mpSdrPageProperties = 0;
    }

    DBG_DTOR(SdrPage,NULL);
}

SdrPage& SdrPage::operator=(const SdrPage& rSrcPage)
{
    if( this == &rSrcPage )
        return *this;
    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }

    // Joe also sets some parameters for the class this one
    // is derived from. SdrObjList does the same bad handling of
    // copy constructor and operator=, so i better let it stand here.
    pPage = this;

    // copy all the local parameters to make this instance
    // a valid copy of source page before copying and inserting
    // the contained objects
    mbMaster = rSrcPage.mbMaster;
    mbSwappingLocked = rSrcPage.mbSwappingLocked;
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
        // #i111122# delete SdrPageProperties when model is different
        if(mpSdrPageProperties && GetModel() != rSrcPage.GetModel())
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
            mpSdrPageProperties->PutItemSet(rSrcPage.getSdrPageProperties().GetItemSet());
        }

        mpSdrPageProperties->SetStyleSheet(rSrcPage.getSdrPageProperties().GetStyleSheet());
    }

    // Now copy the contained objects (by cloning them)
    SdrObjList::operator=(rSrcPage);
    return *this;
}

SdrPage* SdrPage::Clone() const
{
    return Clone(NULL);
}

SdrPage* SdrPage::Clone(SdrModel* pNewModel) const
{
    if (pNewModel==NULL) pNewModel=pModel;
    SdrPage* pPage2=new SdrPage(*pNewModel);
    *pPage2=*this;
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

void SdrPage::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel=pModel;
    SdrObjList::SetModel(pNewModel);
    if (pNewModel!=pOldModel)
    {
        if (pNewModel!=NULL) {
            pLayerAdmin->SetParent(&pNewModel->GetLayerAdmin());
        } else {
            pLayerAdmin->SetParent(NULL);
        }
        pLayerAdmin->SetModel(pNewModel);

        // create new SdrPageProperties with new model (due to SfxItemSet there)
        // and copy ItemSet and StyleSheet
        SdrPageProperties *pNew = new SdrPageProperties(*this);

        if(!IsMasterPage())
        {
            pNew->PutItemSet(getSdrPageProperties().GetItemSet());
        }

        pNew->SetStyleSheet(getSdrPageProperties().GetStyleSheet());

        delete mpSdrPageProperties;
        mpSdrPageProperties = pNew;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
            ((SdrModel*)pModel)->RecalcPageNums(sal_True);
    } else {
        if (pModel && pModel->IsPagNumsDirty())
            ((SdrModel*)pModel)->RecalcPageNums(sal_False);
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// MasterPage interface

void SdrPage::TRG_SetMasterPage(SdrPage& rNew)
{
    if(mpMasterPageDescriptor && &(mpMasterPageDescriptor->GetUsedPage()) == &rNew)
        return;

    if(mpMasterPageDescriptor)
        TRG_ClearMasterPage();

    mpMasterPageDescriptor = new ::sdr::MasterPageDescriptor(*this, rNew);
    GetViewContact().ActionChanged();
}

void SdrPage::TRG_ClearMasterPage()
{
    if(mpMasterPageDescriptor)
    {
        SetChanged();

        // the flushViewObjectContacts() will do needed invalidates by deleting the involved VOCs
        mpMasterPageDescriptor->GetUsedPage().GetViewContact().flushViewObjectContacts(true);

        delete mpMasterPageDescriptor;
        mpMasterPageDescriptor = 0L;
    }
}

SdrPage& SdrPage::TRG_GetMasterPage() const
{
    DBG_ASSERT(mpMasterPageDescriptor != 0L, "TRG_GetMasterPage(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetUsedPage();
}

const SetOfByte& SdrPage::TRG_GetMasterPageVisibleLayers() const
{
    DBG_ASSERT(mpMasterPageDescriptor != 0L, "TRG_GetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    return mpMasterPageDescriptor->GetVisibleLayers();
}

void SdrPage::TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew)
{
    DBG_ASSERT(mpMasterPageDescriptor != 0L, "TRG_SetMasterPageVisibleLayers(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
    mpMasterPageDescriptor->SetVisibleLayers(rNew);
}

sdr::contact::ViewContact& SdrPage::TRG_GetMasterPageDescriptorViewContact() const
{
    DBG_ASSERT(mpMasterPageDescriptor != 0L, "TRG_GetMasterPageDescriptorViewContact(): No MasterPage available. Use TRG_HasMasterPage() before access (!)");
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
    return NULL;
}

XubString SdrPage::GetLayoutName() const
{
    return String();
}

void SdrPage::SetInserted( bool bIns )
{
    if( (bool) mbInserted != bIns )
    {
        mbInserted = bIns;

        SdrObjListIter aIter( *this, IM_FLAT );
         while ( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            if ( pObj->ISA(SdrOle2Obj) )
            {
                if( mbInserted )
                    ( (SdrOle2Obj*) pObj)->Connect();
                else
                    ( (SdrOle2Obj*) pObj)->Disconnect();
            }
        }
    }
}

void SdrPage::SetUnoPage(uno::Reference<drawing::XDrawPage> const& xNewPage)
{
    mxUnoPage = xNewPage;
}

uno::Reference< uno::XInterface > SdrPage::getUnoPage()
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
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt =
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
        if(XFILL_NONE == ((const XFillStyleItem&)pBackgroundFill->Get(XATTR_FILLSTYLE)).GetValue())
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
    return GetPageBackgroundColor( NULL, true );
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

//////////////////////////////////////////////////////////////////////////////
// sdr::Comment interface

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
// use new redirector instead of pPaintProc

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
        if(pObject->GetPage())
        {
            if(pObject->GetPage()->checkVisibility(rOriginal, rDisplayInfo, false))
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
