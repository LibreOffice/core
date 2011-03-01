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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdpage.hxx>

// HACK
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
#include <svx/svdograf.hxx> // fuer SwapInAll()
#include <svx/svdoedge.hxx> // Zum kopieren der Konnektoren
#include <svx/svdoole2.hxx> // Sonderbehandlung OLE beim SdrExchangeFormat
#include "svditer.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/fmglob.hxx>
#include <svx/polysc3d.hxx>

#include <svx/fmdpage.hxx>

#include <sfx2/objsh.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <algorithm>
#include <svl/smplhint.hxx>

using namespace ::com::sun::star;

namespace {
void DumpObjectList (const ::std::vector<SdrObjectWeakRef>& rContainer)
{
    ::std::vector<SdrObjectWeakRef>::const_iterator iObject (rContainer.begin());
    ::std::vector<SdrObjectWeakRef>::const_iterator iEnd (rContainer.end());
    for (int nIndex=0 ; iObject!=iEnd; ++iObject,++nIndex)
    {
        const SdrObject* pObject = iObject->get();
        OSL_TRACE("%d : %x, %s", nIndex,
            pObject,
            ::rtl::OUStringToOString(pObject->GetName(),RTL_TEXTENCODING_UTF8).getStr());
    }
}
}


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
    bObjOrdNumsDirty=FALSE;
    bRectsDirty=FALSE;
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
    bObjOrdNumsDirty=FALSE;
    bRectsDirty=FALSE;
    pOwnerObj=NULL;
    eListKind=SDROBJLIST_UNKNOWN;
    *this=rSrcList;
}

SdrObjList::~SdrObjList()
{
    DBG_DTOR(SdrObjList,NULL);

    // #111111#
    // To avoid that the Clear() method will broadcast changes when in destruction
    // which would call virtual methos (not allowed in destructor), the model is set
    // to NULL here.
    pModel = 0L;

    Clear(); // Containerinhalt loeschen!
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
    bObjOrdNumsDirty=FALSE;
    bRectsDirty     =FALSE;
    ULONG nCloneErrCnt=0;
    ULONG nAnz=rSrcList.GetObjCount();
    SdrInsertReason aReason(SDRREASON_COPY);
    ULONG no;
    for (no=0; no<nAnz; no++) {
        SdrObject* pSO=rSrcList.GetObj(no);

        // #116235#
        //SdrObject* pDO=pSO->Clone(pPage,pModel);
        SdrObject* pDO = pSO->Clone();
        pDO->SetModel(pModel);
        pDO->SetPage(pPage);

        if (pDO!=NULL) {
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
                SdrObject* pSrcNode1=pSrcEdge->GetConnectedNode(TRUE);
                SdrObject* pSrcNode2=pSrcEdge->GetConnectedNode(FALSE);
                if (pSrcNode1!=NULL && pSrcNode1->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode1=NULL; // Listenuebergreifend
                if (pSrcNode2!=NULL && pSrcNode2->GetObjList()!=pSrcEdge->GetObjList()) pSrcNode2=NULL; // ist (noch) nicht
                if (pSrcNode1!=NULL || pSrcNode2!=NULL) {
                    SdrObject* pEdgeObjTmp=GetObj(no);
                    SdrEdgeObj* pDstEdge=PTR_CAST(SdrEdgeObj,pEdgeObjTmp);
                    if (pDstEdge!=NULL) {
                        if (pSrcNode1!=NULL) {
                            ULONG nDstNode1=pSrcNode1->GetOrdNum();
                            SdrObject* pDstNode1=GetObj(nDstNode1);
                            if (pDstNode1!=NULL) { // Sonst grober Fehler!
                                pDstEdge->ConnectToNode(TRUE,pDstNode1);
                            } else {
                                OSL_FAIL("SdrObjList::operator=(): pDstNode1==NULL!");
                            }
                        }
                        if (pSrcNode2!=NULL) {
                            ULONG nDstNode2=pSrcNode2->GetOrdNum();
                            SdrObject* pDstNode2=GetObj(nDstNode2);
                            if (pDstNode2!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(FALSE,pDstNode2);
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
        ByteString aStr("SdrObjList::operator=(): Fehler beim Clonen ");

        if(nCloneErrCnt == 1)
        {
            aStr += "eines Zeichenobjekts.";
        }
        else
        {
            aStr += "von ";
            aStr += ByteString::CreateFromInt32( nCloneErrCnt );
            aStr += " Zeichenobjekten.";
        }

        aStr += " Objektverbindungen werden nicht mitkopiert.";

        DBG_ERROR(aStr.GetBuffer());
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
        ULONG nAnz=GetObjCount();
        for (ULONG no=0; no<nAnz; no++) {
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
        ULONG nAnz=GetObjCount();
        for (ULONG i=0; i<nAnz; i++) {
            SdrObject* pObj=GetObj(i);
            pObj->SetModel(pModel);
        }
    }
}

void SdrObjList::RecalcObjOrdNums()
{
    ULONG nAnz=GetObjCount();
    for (ULONG no=0; no<nAnz; no++) {
        SdrObject* pObj=GetObj(no);
        pObj->SetOrdNum(no);
    }
    bObjOrdNumsDirty=FALSE;
}

void SdrObjList::RecalcRects()
{
    aOutRect=Rectangle();
    aSnapRect=aOutRect;
    ULONG nAnz=GetObjCount();
    ULONG i;
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
    bRectsDirty=TRUE;
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

void SdrObjList::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* /*pReason*/)
{
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcInsertObject(NULL)");
    if (pObj!=NULL) {
        DBG_ASSERT(!pObj->IsInserted(),"ZObjekt hat bereits Inserted-Status");
        ULONG nAnz=GetObjCount();
        if (nPos>nAnz) nPos=nAnz;
        InsertObjectIntoContainer(*pObj,nPos);

        if (nPos<nAnz) bObjOrdNumsDirty=TRUE;
        pObj->SetOrdNum(nPos);
        pObj->SetObjList(this);
        pObj->SetPage(pPage);

        // #110094# Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pObj);

        if (!bRectsDirty) {
            aOutRect.Union(pObj->GetCurrentBoundRect());
            aSnapRect.Union(pObj->GetSnapRect());
        }
        pObj->SetInserted(TRUE); // Ruft u.a. den UserCall
    }
}

void SdrObjList::InsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
{
    DBG_ASSERT(pObj!=NULL,"SdrObjList::InsertObject(NULL)");

    if(pObj)
    {
        // #69055# if anchor is used, reset it before grouping
        if(GetOwnerObj())
        {
            const Point& rAnchorPos = pObj->GetAnchorPos();
            if(rAnchorPos.X() || rAnchorPos.Y())
                pObj->NbcSetAnchorPos(Point());
        }

        // do insert to new group
        NbcInsertObject(pObj, nPos, pReason);

        // Falls das Objekt in eine Gruppe eingefuegt wird
        // und nicht mit seinen Bruedern ueberlappt, muss es
        // einen eigenen Redraw bekommen
        if(pOwnerObj)
        {
            // only repaint here
            pOwnerObj->ActionChanged();
        }

        if(pModel)
        {
            // Hier muss ein anderer Broadcast her!
            // Repaint ab Objekt Nummer ... (Achtung: GroupObj)
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

SdrObject* SdrObjList::NbcRemoveObject(ULONG nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return NULL;
    }

    ULONG nAnz=GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    // flushViewObjectContacts() clears the VOC's and those invalidate
    pObj->GetViewContact().flushViewObjectContacts(true);

    DBG_ASSERT(pObj!=NULL,"Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"ZObjekt hat keinen Inserted-Status");
        pObj->SetInserted(FALSE); // Ruft u.a. den UserCall
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        if (!bObjOrdNumsDirty) { // Optimierung fuer den Fall, dass das letzte Obj rausgenommen wird
            if (nObjNum!=ULONG(nAnz-1)) {
                bObjOrdNumsDirty=TRUE;
            }
        }
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::RemoveObject(ULONG nObjNum)
{
    if (nObjNum >= maList.size())
    {
        OSL_ASSERT(nObjNum<maList.size());
        return NULL;
    }

    ULONG nAnz=GetObjCount();
    SdrObject* pObj=maList[nObjNum];
    RemoveObjectFromContainer(nObjNum);

    DBG_ASSERT(pObj!=NULL,"Object zum Removen nicht gefunden");
    if(pObj)
    {
        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        DBG_ASSERT(pObj->IsInserted(),"ZObjekt hat keinen Inserted-Status");
        if (pModel!=NULL) {
            // Hier muss ein anderer Broadcast her!
            if (pObj->GetPage()!=NULL) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
            pModel->SetChanged();
        }
        pObj->SetInserted(FALSE); // Ruft u.a. den UserCall
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        if (!bObjOrdNumsDirty) { // Optimierung fuer den Fall, dass das letzte Obj rausgenommen wird
            if (nObjNum!=ULONG(nAnz-1)) {
                bObjOrdNumsDirty=TRUE;
            }
        }
        SetRectsDirty();

        if(pOwnerObj && !GetObjCount())
        {
            // empty group created; it needs to be repainted since it's
            // visualisation changes
            pOwnerObj->ActionChanged();
        }
    }
    return pObj;
}

SdrObject* SdrObjList::NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    if (nObjNum >= maList.size() || pNewObj == NULL)
    {
        OSL_ASSERT(nObjNum<maList.size());
        OSL_ASSERT(pNewObj!=NULL);
        return NULL;
    }

    SdrObject* pObj=maList[nObjNum];
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt hat keinen Inserted-Status");
        pObj->SetInserted(FALSE);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // #110094#  Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(TRUE);
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::ReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
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
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt hat keinen Inserted-Status");
        if (pModel!=NULL) {
            // Hier muss ein anderer Broadcast her!
            if (pObj->GetPage()!=NULL) {
                SdrHint aHint(*pObj);
                aHint.SetKind(HINT_OBJREMOVED);
                pModel->Broadcast(aHint);
            }
        }
        pObj->SetInserted(FALSE);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        ReplaceObjectInContainer(*pNewObj,nObjNum);

        // flushViewObjectContacts() clears the VOC's and those invalidate
        pObj->GetViewContact().flushViewObjectContacts(true);

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // #110094#  Inform the parent about change to allow invalidations at
        // evtl. existing parent visualisations
        impChildInserted(*pNewObj);

        pNewObj->SetInserted(TRUE);
        if (pModel!=NULL) {
            // Hier muss ein anderer Broadcast her!
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

SdrObject* SdrObjList::NbcSetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum)
{
    if (nOldObjNum >= maList.size() || nNewObjNum >= maList.size())
    {
        OSL_ASSERT(nOldObjNum<maList.size());
        OSL_ASSERT(nNewObjNum<maList.size());
        return NULL;
    }

    SdrObject* pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcSetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::NbcSetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        RemoveObjectFromContainer(nOldObjNum);

        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // #110094# No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=TRUE;
    }
    return pObj;
}

SdrObject* SdrObjList::SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum)
{
    if (nOldObjNum >= maList.size() || nNewObjNum >= maList.size())
    {
        OSL_ASSERT(nOldObjNum<maList.size());
        OSL_ASSERT(nNewObjNum<maList.size());
        return NULL;
    }

    SdrObject* pObj=maList[nOldObjNum];
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::SetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        RemoveObjectFromContainer(nOldObjNum);
        InsertObjectIntoContainer(*pObj,nNewObjNum);

        // #110094#No need to delete visualisation data since same object
        // gets inserted again. Also a single ActionChanged is enough
        pObj->ActionChanged();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=TRUE;
        if (pModel!=NULL)
        {
            // Hier muss ein anderer Broadcast her!
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
        ((SdrObjList*)this)->bRectsDirty=FALSE;
    }
    return aSnapRect;
}

const Rectangle& SdrObjList::GetAllObjBoundRect() const
{
    // #i106183# for deep group hierarchies like in chart2, the invalidates
    // through the hierarchy are not correct; use a 2nd hint for the needed
    // recalculation. Future versions will have no bool flag at all, but
    // just aOutRect in empty state to representate an invalid state, thus
    // it's a step in the right direction.
    if (bRectsDirty || aOutRect.IsEmpty())
    {
        ((SdrObjList*)this)->RecalcRects();
        ((SdrObjList*)this)->bRectsDirty=FALSE;
    }
    return aOutRect;
}

void SdrObjList::NbcReformatAllTextObjects()
{
    ULONG nAnz=GetObjCount();
    ULONG nNum=0;

    Printer* pPrinter = NULL;

    if (pModel)
    {
        if (pModel->GetRefDevice() && pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
        {
            // Kein RefDevice oder RefDevice kein Printer
            pPrinter = (Printer*) pModel->GetRefDevice();
        }
    }

    while (nNum<nAnz)
    {
        SdrObject* pObj = GetObj(nNum);
        if (pPrinter &&
            pObj->GetObjInventor() == SdrInventor &&
            pObj->GetObjIdentifier() == OBJ_OLE2  &&
            !( (SdrOle2Obj*) pObj )->IsEmpty() )
        {
            //const SvInPlaceObjectRef& xObjRef = ((SdrOle2Obj*) pObj)->GetObjRef();
            //TODO/LATER: PrinterChangeNotification needed
            //if( xObjRef.Is() && ( xObjRef->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE ) )
            //  xObjRef->OnDocumentPrinterChanged(pPrinter);
        }

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
    they may reposition itselfs.
    #103122#
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

ULONG SdrObjList::GetObjCount() const
{
    return maList.size();
}




SdrObject* SdrObjList::GetObj(ULONG nNum) const
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

ULONG SdrObjList::CountAllObjects() const
{
    ULONG nCnt=GetObjCount();
    ULONG nAnz=nCnt;
    for (USHORT nNum=0; nNum<nAnz; nNum++) {
        SdrObjList* pSubOL=GetObj(nNum)->GetSubList();
        if (pSubOL!=NULL) {
            nCnt+=pSubOL->CountAllObjects();
        }
    }
    return nCnt;
}

void SdrObjList::ForceSwapInObjects() const
{
    ULONG nObjAnz=GetObjCount();
    for (ULONG nObjNum=nObjAnz; nObjNum>0;) {
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
    ULONG nObjAnz=GetObjCount();
    for (ULONG nObjNum=nObjAnz; nObjNum>0;) {
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

void SdrObjList::UnGroupObj( ULONG nObjNum )
{
    // if the given object is no group, this method is a noop
    SdrObject* pUngroupObj = GetObj( nObjNum );
    if( pUngroupObj )
    {
        SdrObjList* pSrcLst = pUngroupObj->GetSubList();
        //sal_Int32 nCount( 0 );
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
        // There is a user defined navigation order.  Make sure the object
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
        // There is no user defined navigation order.  Use the z-order
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
    bObjOrdNumsDirty=TRUE;
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
    bObjOrdNumsDirty=TRUE;
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
    bObjOrdNumsDirty=TRUE;
}




////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageGridFrameList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

//////////////////////////////////////////////////////////////////////////////
// #111111# PageUser section

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
// #110094# DrawContact section

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

    // #111111#
    // tell all the registered PageUsers that the page is in destruction
    // This causes some (all?) PageUsers to remove themselves from the list
    // of page users.  Therefore we have to use a copy of the list for the
    // iteration.
    ::sdr::PageUserVector aListCopy (maPageUsers.begin(), maPageUsers.end());
    for(::sdr::PageUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); aIterator++)
    {
        sdr::PageUser* pPageUser = *aIterator;
        DBG_ASSERT(pPageUser, "SdrPage::~SdrPage: corrupt PageUser list (!)");
        pPageUser->PageInDestruction(*this);
    }

    // #111111#
    // Clear the vector. This means that user do not need to call RemovePageUser()
    // when they get called from PageInDestruction().
    maPageUsers.clear();

    delete pLayerAdmin;

    TRG_ClearMasterPage();

    // #110094#
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

void SdrPage::operator=(const SdrPage& rSrcPage)
{
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
    // a valid copy od source page before copying and inserting
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
    //aMasters = rSrcPage.aMasters;

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

    // Now copy the contained obejcts (by cloning them)
    SdrObjList::operator=(rSrcPage);
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

INT32 SdrPage::GetWdt() const
{
    return nWdt;
}

void SdrPage::SetOrientation(Orientation eOri)
{
    // Quadratisch ist und bleibt immer Portrait
    Size aSiz(GetSize());
    if (aSiz.Width()!=aSiz.Height()) {
        if ((eOri==ORIENTATION_PORTRAIT) == (aSiz.Width()>aSiz.Height())) {
            SetSize(Size(aSiz.Height(),aSiz.Width()));
        }
    }
}

Orientation SdrPage::GetOrientation() const
{
    // Quadratisch ist Portrait
    Orientation eRet=ORIENTATION_PORTRAIT;
    Size aSiz(GetSize());
    if (aSiz.Width()>aSiz.Height()) eRet=ORIENTATION_LANDSCAPE;
    return eRet;
}

INT32 SdrPage::GetHgt() const
{
    return nHgt;
}

void  SdrPage::SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 nLwr)
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

void  SdrPage::SetLftBorder(INT32 nBorder)
{
    if(nBordLft != nBorder)
    {
        nBordLft = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetUppBorder(INT32 nBorder)
{
    if(nBordUpp != nBorder)
    {
        nBordUpp = nBorder;
        SetChanged();
    }
}

void  SdrPage::SetRgtBorder(INT32 nBorder)
{
    if(nBordRgt != nBorder)
    {
        nBordRgt=nBorder;
        SetChanged();
    }
}

void  SdrPage::SetLwrBorder(INT32 nBorder)
{
    if(nBordLwr != nBorder)
    {
        nBordLwr=nBorder;
        SetChanged();
    }
}

INT32 SdrPage::GetLftBorder() const
{
    return nBordLft;
}

INT32 SdrPage::GetUppBorder() const
{
    return nBordUpp;
}

INT32 SdrPage::GetRgtBorder() const
{
    return nBordRgt;
}

INT32 SdrPage::GetLwrBorder() const
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

    // update listeners at possible api wrapper object
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

USHORT SdrPage::GetPageNum() const
{
    if (!mbInserted)
        return 0;

    if (mbMaster) {
        if (pModel && pModel->IsMPgNumsDirty())
            ((SdrModel*)pModel)->RecalcPageNums(TRUE);
    } else {
        if (pModel && pModel->IsPagNumsDirty())
            ((SdrModel*)pModel)->RecalcPageNums(FALSE);
    }
    return nPageNum;
}

void SdrPage::SetChanged()
{
    // #110094#-11
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

// #115423# used from SdrModel::RemoveMasterPage
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
    // Die wollte Dieter haben.
    return String();
}

void SdrPage::SetInserted( bool bIns )
{
    if( mbInserted != bIns )
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
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt =
        static_cast<cppu::OWeakObject*>( new SvxFmDrawPage( this ) );
    return xInt;
}

SfxStyleSheet* SdrPage::GetTextStyleSheetForObject( SdrObject* pObj ) const
{
    return pObj->GetStyleSheet();
}

bool SdrPage::HasTransparentObjects( BOOL bCheckForAlphaChannel ) const
{
    bool bRet = false;

    for( ULONG n = 0, nCount = GetObjCount(); ( n < nCount ) && !bRet; n++ )
        if( GetObj( n )->IsTransparent( bCheckForAlphaChannel ) )
            bRet = true;

    return bRet;
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

// NYI: Dummy implementations for declarations in svdpage.hxx
Bitmap      SdrPage::GetBitmap(const SetOfByte& /*rVisibleLayers*/, bool /*bTrimBorders*/) const
{
    DBG_ASSERT(0, "SdrPage::GetBitmap(): not yet implemented.");
    return Bitmap();
}
GDIMetaFile SdrPage::GetMetaFile(const SetOfByte& /*rVisibleLayers*/, bool /*bTrimBorders*/)
{
    DBG_ASSERT(0, "SdrPage::GetMetaFile(): not yet implemented.");
    return GDIMetaFile();
}

bool SdrPage::isHandoutMasterPage() const
{
    return mbMaster && GetModel() && GetModel()->GetMasterPageCount()
        && GetModel()->GetMasterPage(0) == this;
}

//////////////////////////////////////////////////////////////////////////////
// sdr::Comment interface

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
