/*************************************************************************
 *
 *  $RCSfile: svdpage.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:48:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// HACK
#ifdef SVX_LIGHT
#define _IPOBJ_HXX
#endif

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SOT_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _PERSIST_HXX
#include <so3/persist.hxx>
#endif
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif
#include "svdpage.hxx"
#include <string.h>
#ifndef _STRING_H
#define _STRING_H
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif

#include "svdetc.hxx"
#include "svdxout.hxx"
#include "svdio.hxx"
#include "svdobj.hxx"
#include "svdogrp.hxx"
#include "svdograf.hxx" // fuer SwapInAll()
#include "svdoedge.hxx" // Zum kopieren der Konnektoren
#include "svdoole2.hxx" // Sonderbehandlung OLE beim SdrExchangeFormat
#include "svditer.hxx"
#include "svdmodel.hxx"
#include "svdlayer.hxx"
#include "svdotext.hxx"
#include "svdpagv.hxx"
#include "svdundo.hxx"
#include "fmglob.hxx"
#include "polysc3d.hxx"

#ifndef SVX_LIGHT
#include "fmdpage.hxx"
#else
#include "unopage.hxx"
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

// #110094#
#ifndef _SDR_CONTACT_VIEWCONTACTOFSDRPAGE_HXX
#include <svx/sdr/contact/viewcontactofsdrpage.hxx>
#endif

// #110094#
#ifndef _SDR_CONTACT_VIEWCONTACTOFMASTERPAGE_HXX
#include <svx/sdr/contact/viewcontactofmasterpage.hxx>
#endif

// StandardCheckVisisbilityRedirector
#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

// #111111#
#include <algorithm>

using namespace ::com::sun::star;

DBG_NAME(SdrObjList);

TYPEINIT0(SdrObjList);

SdrObjList::SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList):
    maList(1024,64,64)
{
    DBG_CTOR(SdrObjList,NULL);
    pModel=pNewModel;
    pPage=pNewPage;
    pUpList=pNewUpList;
    bObjOrdNumsDirty=FALSE;
    bRectsDirty=FALSE;
    pOwnerObj=NULL;
    eListKind=SDROBJLIST_UNKNOWN;
}

SdrObjList::SdrObjList(const SdrObjList& rSrcList):
    maList(1024,64,64)
{
    DBG_CTOR(SdrObjList,NULL);
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
    // und nun zu den Konnektoren
    // Die neuen Objekte werden auf die der rSrcList abgebildet
    // und so die Objektverbindungen hergestellt.
    // Aehnliche Implementation an folgenden Stellen:
    //    void SdrObjList::CopyObjects(const SdrObjList& rSrcList)
    //    SdrModel* SdrExchangeView::GetMarkedObjModel() const
    //    FASTBOOL SdrExchangeView::Paste(const SdrModel& rMod,...)
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
                                DBG_ERROR("SdrObjList::operator=(): pDstNode1==NULL!");
                            }
                        }
                        if (pSrcNode2!=NULL) {
                            ULONG nDstNode2=pSrcNode2->GetOrdNum();
                            SdrObject* pDstNode2=GetObj(nDstNode2);
                            if (pDstNode2!=NULL) { // Node war sonst wohl nicht markiert
                                pDstEdge->ConnectToNode(FALSE,pDstNode2);
                            } else {
                                DBG_ERROR("SdrObjList::operator=(): pDstNode2==NULL!");
                            }
                        }
                    } else {
                        DBG_ERROR("SdrObjList::operator=(): pDstEdge==NULL!");
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
    // #110094#-9
    sal_Bool bObjectsRemoved(sal_False);

    while(maList.Count())
    {
        // remove last object from list
        SdrObject* pObj = (SdrObject*)maList.Remove(maList.Count() - 1L);

        // #110094#
        pObj->ActionRemoved();

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
        delete pObj;
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

void SdrObjList::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
{
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcInsertObject(NULL)");
    if (pObj!=NULL) {
        DBG_ASSERT(!pObj->IsInserted(),"ZObjekt hat bereits Inserted-Status");
        ULONG nAnz=GetObjCount();
        if (nPos>nAnz) nPos=nAnz;
        maList.Insert(pObj,nPos);

        if (nPos<nAnz) bObjOrdNumsDirty=TRUE;
        pObj->SetOrdNum(nPos);
        pObj->SetObjList(this);
        pObj->SetPage(pPage);

        // #110094#
        pObj->ActionInserted();

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
            // pOwnerObj->BroadcastObjectChange();
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
    ULONG nAnz=GetObjCount();
    SdrObject* pObj=(SdrObject*)maList.Remove(nObjNum);

    // #110094#
    pObj->ActionRemoved();

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
    ULONG nAnz=GetObjCount();
    SdrObject* pObj=(SdrObject*)maList.Remove(nObjNum);

    // #110094#
    pObj->ActionRemoved();

    DBG_ASSERT(pObj!=NULL,"Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
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
        if (pOwnerObj!=NULL && GetObjCount()==0)
        {
            // only repaint here
            pOwnerObj->ActionChanged();
            // pOwnerObj->BroadcastObjectChange();
        }
    }
    return pObj;
}

SdrObject* SdrObjList::NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    SdrObject* pObj=(SdrObject*)maList.GetObject(nObjNum);
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt hat keinen Inserted-Status");
        pObj->SetInserted(FALSE);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        maList.Replace(pNewObj,nObjNum);

        // #110094#
        pObj->ActionRemoved();

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // #110094#
        pNewObj->ActionInserted();

        pNewObj->SetInserted(TRUE);
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::ReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    //ULONG nAnz=GetObjCount();
    SdrObject* pObj=(SdrObject*)maList.GetObject(nObjNum);
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
        maList.Replace(pNewObj,nObjNum);

        // #110094#
        pObj->ActionRemoved();

        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);

        // #110094#
        pNewObj->ActionInserted();

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
    SdrObject* pObj=(SdrObject*)maList.GetObject(nOldObjNum);
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcSetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::NbcSetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        maList.Remove(nOldObjNum);

        // #110094#
        pObj->ActionRemoved();

        maList.Insert(pObj,nNewObjNum);

        // #110094#
        pObj->ActionInserted();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=TRUE;
    }
    return pObj;
}

SdrObject* SdrObjList::SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum)
{
    SdrObject* pObj=(SdrObject*)maList.GetObject(nOldObjNum);
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::SetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        maList.Remove(nOldObjNum);

        // #110094#
        pObj->ActionRemoved();

        maList.Insert(pObj,nNewObjNum);

        // #110094#
        pObj->ActionInserted();

        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=TRUE;
        if (pModel!=NULL) {
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
    if (bRectsDirty) {
        ((SdrObjList*)this)->RecalcRects();
        ((SdrObjList*)this)->bRectsDirty=FALSE;
    }
    return aOutRect;
}

SdrObject* SdrObjList::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bBackward) const
{
    SdrObject* pHit=NULL;
    Rectangle R(rPnt.X()-nTol,rPnt.Y()-nTol,rPnt.X()+nTol,rPnt.Y()+nTol);
    if (R.IsOver(GetAllObjBoundRect())) {
        ULONG nObjAnz=GetObjCount();
        ULONG nObjNum=bBackward ? 0 : nObjAnz;
        while (pHit==NULL && (bBackward ? nObjNum<nObjAnz : nObjNum>0)) {
            if (!bBackward) nObjNum--;
            SdrObject* pObj=GetObj(nObjNum);
            if (R.IsOver(pObj->GetCurrentBoundRect())) {
                SdrObjList* pSubList=pObj->GetSubList();
                if (pSubList!=NULL || pVisiLayer==NULL) { // Gruppenobjekte beruecksichtigen sichtbare Layer selbst
                    pHit=pObj->CheckHit(rPnt,nTol,pVisiLayer/*,bBackward*/);
                } else {             // Ansonsten nur wenn Layer sichtbar
                    SdrLayerID nLayer=pObj->GetLayer();
                    if (pVisiLayer->IsSet(nLayer)) {
                        pHit=pObj->CheckHit(rPnt,nTol,pVisiLayer/*,bBackward*/);
                    }
                }
            }
            if (bBackward) nObjNum++;
        }
    }
    return pHit;
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
#ifndef SVX_LIGHT
        if (pPrinter &&
            pObj->GetObjInventor() == SdrInventor &&
            pObj->GetObjIdentifier() == OBJ_OLE2  &&
            !( (SdrOle2Obj*) pObj )->IsEmpty() )
        {
            const SvInPlaceObjectRef& xObjRef = ((SdrOle2Obj*) pObj)->GetObjRef();

            if( xObjRef.Is() && ( xObjRef->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE ) )
                xObjRef->OnDocumentPrinterChanged(pPrinter);
        }
#endif

        pObj->NbcReformatText();
        nAnz=GetObjCount();             // ReformatText may delete an object
        nNum++;
    }

}

void SdrObjList::ReformatAllTextObjects()
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

#ifndef SVX_LIGHT
        if (pPrinter &&
            pObj->GetObjInventor() == SdrInventor &&
            pObj->GetObjIdentifier() == OBJ_OLE2  &&
            !( (SdrOle2Obj*) pObj )->IsEmpty() )
        {
            const SvInPlaceObjectRef& xObjRef = ((SdrOle2Obj*) pObj)->GetObjRef();

            if( xObjRef.Is() && ( xObjRef->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE ) )
                xObjRef->OnDocumentPrinterChanged(pPrinter);
        }
#endif

        pObj->ReformatText();
        nAnz=GetObjCount();             // ReformatText may delete an object
        nNum++;
    }
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

void SdrObjList::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
{
    for(sal_uInt32 a(0L); a < GetObjCount(); a++)
    {
        GetObj(a)->BurnInStyleSheetAttributes(bPseudoSheetsOnly);
    }
}

void SdrObjList::RemoveNotPersistentObjects(FASTBOOL bNoBroadcast)
{
    FASTBOOL bNoOLE=pModel!=NULL && pModel->IsStreamingSdrModel();
    ULONG nObjAnz=GetObjCount();
    for (ULONG nObjNum=nObjAnz; nObjNum>0;) {
        nObjNum--;
        SdrObject* pObj=GetObj(nObjNum);
        FASTBOOL bThisObjNot=pObj->IsNotPersistent();
        if (!bThisObjNot && bNoOLE && pObj->ISA(SdrOle2Obj)) {
            bThisObjNot=TRUE;
        }
        if (bThisObjNot) {
            if (bNoBroadcast) NbcRemoveObject(nObjNum);
            else RemoveObject(nObjNum);
        } else {
            SdrObjList* pOL=pObj->GetSubList();
            if (pOL!=NULL) {
                pOL->RemoveNotPersistentObjects(bNoBroadcast);
            }
        }
    }
}

FASTBOOL SdrObjList::ImpGetFillColor(SdrObject* pObj, Color& rCol) const
{
    return GetDraftFillColor(pObj->GetMergedItemSet(), rCol);
}

FASTBOOL SdrObjList::GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
    /*FASTBOOL bLayerSorted,*/ Color& rCol) const
{
    if (pModel==NULL) return FALSE;
    FASTBOOL bRet=FALSE;
    FASTBOOL bMaster=pPage!=NULL ? pPage->IsMasterPage() : FALSE;
    for (ULONG no=GetObjCount(); !bRet && no>0; ) {
        no--;
        SdrObject* pObj=GetObj(no);
        SdrObjList* pOL=pObj->GetSubList();
        if (pOL!=NULL) { // Aha, Gruppenobjekt
            bRet=pOL->GetFillColor(rPnt,rVisLayers,/*bLayerSorted,*/rCol);
        } else {
            SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pObj);
            // #108867# Exclude zero master page object (i.e. background
            // shape) from color query
            if (pTextObj!=NULL &&
                pObj->IsClosedObj() && rVisLayers.IsSet(pObj->GetLayer()) &&
                (!bMaster || (!pObj->IsNotVisibleAsMaster() && no!=0)) &&
                pObj->GetCurrentBoundRect().IsInside(rPnt) &&
                !pTextObj->IsHideContour() && pObj->IsHit(rPnt,0,NULL))
            {   // Nachfolgend extra Funktion um Stack zu sparen,
                // da diese Methode hier rekursiv ist.
                bRet=ImpGetFillColor(pObj,rCol);
            }
        }
    }
    return bRet;
}

FASTBOOL SdrObjList::IsReadOnly() const
{
    FASTBOOL bRet=FALSE;
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

void SdrObjList::Save(SvStream& rOut) const
{
    FASTBOOL bNotPersist=pPage!=NULL && pPage->IsObjectsNotPersistent();
    FASTBOOL bNoOLE=pModel!=NULL && pModel->IsStreamingSdrModel();
    if (!bNotPersist) {
        SdrObjListIter aIter(*this,IM_FLAT);
        while (aIter.IsMore()) {
            SdrObject* pObj=aIter.Next();
            FASTBOOL bThisObjNot=pObj->IsNotPersistent();
            if (!bThisObjNot && bNoOLE && pObj->ISA(SdrOle2Obj)) {
                bThisObjNot=TRUE;
            }
            if (!bThisObjNot) rOut<<*pObj;
            if (pModel!=NULL) pModel->IncProgress();
        }
    }
    SdrIOHeader(rOut,STREAM_WRITE,SdrIOEndeID); // Endemarke
}

void SdrObjList::Load(SvStream& rIn, SdrPage& rPage)
{
    Clear();

    if (rIn.GetError()!=0)
        return;

    SdrInsertReason aReason(SDRREASON_STREAMING);
    FASTBOOL        bEnde=FALSE;

    while( rIn.GetError()==0 && !rIn.IsEof() && !bEnde )
    {
        SdrObjIOHeaderLookAhead aHead(rIn,STREAM_READ);

        if (!aHead.IsEnde())
        {
            SdrObject* pObj=SdrObjFactory::MakeNewObject(aHead.nInventor,aHead.nIdentifier,&rPage);

            if( pObj!=NULL )
            {
                rIn >> *pObj;

#ifndef SVX_LIGHT
                if( ( pObj->GetObjIdentifier() == OBJ_OLE2 ) && ( pObj->GetObjInventor() == SdrInventor ) )
                {
                    // convert StarImage OLE objects to normal graphic objects
                    SdrOle2Obj* pOLEObj = (SdrOle2Obj*) pObj;
                    BOOL        bImageOLE = FALSE;

                    if( pOLEObj->GetProgName() == String( RTL_CONSTASCII_USTRINGPARAM( "StarImage" ) ) )
                        bImageOLE = TRUE;
                    else if( pModel->GetPersist() )
                    {
                        SvInfoObjectRef     xInfo( pModel->GetPersist()->Find( pOLEObj->GetPersistName() ) );
                        const SvGlobalName  aSim30Name( SO3_SIM_CLASSID_30 );
                        const SvGlobalName  aSim40Name( SO3_SIM_CLASSID_40 );
                        const SvGlobalName  aSim50Name( SO3_SIM_CLASSID_50 );

                        if( xInfo.Is() &&
                            ( xInfo->GetClassName() == aSim30Name ||
                              xInfo->GetClassName() == aSim40Name ||
                              xInfo->GetClassName() == aSim50Name ) )
                        {
                            bImageOLE = TRUE;
                        }
                    }

                    if( bImageOLE && pOLEObj->GetPersistName().Len() )
                    {
                        SotStorage*     pModelStorage = pModel->GetModelStorage();
                        const String    aSimStorageName( pOLEObj->GetPersistName() );

                        if( pModelStorage && pModelStorage->IsStorage( aSimStorageName ) )
                        {
                            SotStorageRef xSimStorage( pModelStorage->OpenSotStorage( aSimStorageName ) );

                            if( xSimStorage.Is() )
                            {
                                String aStmName( RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument" ) );

                                if( xSimStorage->IsStream( aStmName ) ||
                                    xSimStorage->IsStream( aStmName = String( RTL_CONSTASCII_USTRINGPARAM( "StarImageDocument 4.0" ) ) ) )
                                {
                                    SotStorageStreamRef xSimStm( xSimStorage->OpenSotStream( aStmName ) );

                                    if( xSimStm.Is() && !xSimStm->GetError() )
                                    {
                                        Graphic aGraphic;

                                        xSimStm->SetBufferSize( 32768 );
                                        xSimStm->SetKey( xSimStorage->GetKey() );
                                        *xSimStm >> aGraphic;
                                        xSimStm->SetBufferSize( 0 );

                                        SdrGrafObj* pNewObj = (SdrGrafObj*) SdrObjFactory::MakeNewObject( SdrInventor, OBJ_GRAF, &rPage );

                                        if( pNewObj )
                                        {
                                            pNewObj->SetGraphic( aGraphic );
                                            pNewObj->SetLogicRect( pObj->GetLogicRect() );
                                            delete pObj;
                                            pObj = pNewObj;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
#endif // SVX_LIGHT

                InsertObject(pObj,CONTAINER_APPEND,&aReason);
            }
            else
            { // aha, das wil keiner. Also ueberlesen.
#ifdef SVX_LIGHT
                if( aHead.nInventor != FmFormInventor )
                {
#endif

#ifdef DBG_UTIL
                ByteString aStr("SdrObjList::Load(): Zeichenobjekt kann von der Factory nicht erzeugt werden:\n");
                UINT32 nPos(GetObjCount());

                aStr += "Listenposition: ";
                aStr += ByteString::CreateFromInt32( nPos );
                aStr += "\n";
                aStr += "Inventor: ";
                sal_Int32 nInv = SWAPLONG( aHead.nInventor );
                aStr += ByteString::CreateFromInt32( nInv );
                aStr += ", Identifier: ";
                aStr += ByteString::CreateFromInt32( aHead.nIdentifier );
                aStr += "\n";
                aStr += "FilePos: ";
                aStr += ByteString::CreateFromInt32( aHead.GetFilePos() );
                aStr += ", BlockSize: ";
                aStr += ByteString::CreateFromInt32( aHead.GetBlockSize() );

                DBG_ERROR(aStr.GetBuffer());
#endif

#ifdef SVX_LIGHT
                }
#endif
                aHead.SkipRecord();
            }
        }
        else
        {
            bEnde=TRUE;
            aHead.SkipRecord(); // die Endemarke weglesen
        }

        SdrModel* pMd=pModel;

        if (pMd==NULL)
            pMd=rPage.GetModel();

        if (pMd!=NULL)
            pMd->DoProgress(rIn.Tell());
    }
}

void SdrObjList::AfterRead()
{
    ULONG nAnz=GetObjCount();
    for (ULONG i=0; i<nAnz; i++) {
        GetObj(i)->AfterRead();
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
        sal_Int32 nCount( 0 );
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
        DBG_ERROR("SdrObjList::UnGroupObj: object index invalid");
#endif
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
    if(IsMasterPage())
    {
        return new sdr::contact::ViewContactOfMasterPage(*this);
    }
    else
    {
        return new sdr::contact::ViewContactOfSdrPage(*this);
    }
}

sdr::contact::ViewContact& SdrPage::GetViewContact() const
{
    if(!mpViewContact)
    {
        ((SdrPage*)this)->mpViewContact = ((SdrPage*)this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrPage,SdrObjList);

SdrPage::SdrPage(SdrModel& rNewModel, FASTBOOL bMasterPage)
:   SdrObjList(&rNewModel, this),
    mpViewContact(0L),
    nWdt(10L),
    nHgt(10L),
    nBordLft(0L),
    nBordUpp(0L),
    nBordRgt(0L),
    nBordLwr(0L),
    pLayerAdmin(new SdrLayerAdmin(&rNewModel.GetLayerAdmin())),
    pBackgroundObj(0L),
    mpMasterPageDescriptor(0L),
    nPageNum(0L),
    bMaster(bMasterPage),
    bInserted(sal_False),
    bObjectsNotPersistent(sal_False),
    bSwappingLocked(sal_False)
{
    aPrefVisiLayers.SetAll();
    eListKind = (bMasterPage) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;
}

SdrPage::SdrPage(const SdrPage& rSrcPage)
:   SdrObjList(rSrcPage.pModel, this),
    mpViewContact(0L),
    nWdt(rSrcPage.nWdt),
    nHgt(rSrcPage.nHgt),
    nBordLft(rSrcPage.nBordLft),
    nBordUpp(rSrcPage.nBordUpp),
    nBordRgt(rSrcPage.nBordRgt),
    nBordLwr(rSrcPage.nBordLwr),
    pLayerAdmin(new SdrLayerAdmin(rSrcPage.pModel->GetLayerAdmin())),
    pBackgroundObj(0L),
    mpMasterPageDescriptor(0L),
    nPageNum(rSrcPage.nPageNum),
    bMaster(rSrcPage.bMaster),
    bInserted(sal_False),
    bObjectsNotPersistent(rSrcPage.bObjectsNotPersistent),
    bSwappingLocked(rSrcPage.bSwappingLocked)
{
    aPrefVisiLayers.SetAll();
    eListKind = (bMaster) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;

    // copy things from source
    *this = rSrcPage;

    // be careful and correct eListKind, a member of SdrObjList which
    // will be changed by the SdrOIbjList::operator= before...
    eListKind = (bMaster) ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;
}
SdrPage::~SdrPage()
{
    // #111111#
    // tell all the registered PageUsers that the page is in destruction
    for(::sdr::PageUserVector::iterator aIterator = maPageUsers.begin(); aIterator != maPageUsers.end(); aIterator++)
    {
        sdr::PageUser* pPageUser = *aIterator;
        DBG_ASSERT(pPageUser, "SdrPage::~SdrPage: corrupt PageUser list (!)");
        pPageUser->PageInDestruction(*this);
    }

    // #111111#
    // Clear the vector. This means that user do not need to call RemovePageUser()
    // when they get called from PageInDestruction().
    maPageUsers.clear();

    delete pBackgroundObj;
    delete pLayerAdmin;

    TRG_ClearMasterPage();

    // #110094#
    if(mpViewContact)
    {
        mpViewContact->PrepareDelete();
        delete mpViewContact;
        mpViewContact = 0L;
    }
}

void SdrPage::operator=(const SdrPage& rSrcPage)
{
    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }

    if(pBackgroundObj)
    {
        delete pBackgroundObj;
        pBackgroundObj = 0L;
    }

    // Joe also sets some parameters for the class this one
    // is derived from. SdrObjList does the same bad handling of
    // copy constructor and operator=, so i better let it stand here.
    pPage = this;

    // copy all the local parameters to make this instance
    // a valid copy od source page before copying and inserting
    // the contained objects
    bMaster = rSrcPage.bMaster;
    bSwappingLocked = rSrcPage.bSwappingLocked;
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

    bObjectsNotPersistent = rSrcPage.bObjectsNotPersistent;

    if(rSrcPage.pBackgroundObj)
    {
        pBackgroundObj = rSrcPage.pBackgroundObj->Clone();
          pBackgroundObj->SetPage( this );
        pBackgroundObj->SetModel( pModel );
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
    SdrPage* pPage=new SdrPage(*pNewModel);
    *pPage=*this;
    return pPage;
}

SfxItemPool& SdrPage::GetItemPool() const
{
    return pModel->GetItemPool();
}

void SdrPage::SetSize(const Size& aSiz)
{
    nWdt=aSiz.Width();
    nHgt=aSiz.Height();
    SetChanged();
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
    nBordLft=nLft;
    nBordUpp=nUpp;
    nBordRgt=nRgt;
    nBordLwr=nLwr;
    SetChanged();
}

void  SdrPage::SetLftBorder(INT32 nBorder)
{
    nBordLft=nBorder;
    SetChanged();
}

void  SdrPage::SetUppBorder(INT32 nBorder)
{
    nBordUpp=nBorder;
    SetChanged();
}

void  SdrPage::SetRgtBorder(INT32 nBorder)
{
    nBordRgt=nBorder;
    SetChanged();
}

void  SdrPage::SetLwrBorder(INT32 nBorder)
{
    nBordLwr=nBorder;
    SetChanged();
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

        if( pBackgroundObj )
            pBackgroundObj->SetModel( pNewModel );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

USHORT SdrPage::GetPageNum() const
{
    if (!bInserted) return 0;
    if (bMaster) {
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
    GetViewContact().ActionChildInserted(rNew.GetViewContact());
}

void SdrPage::TRG_ClearMasterPage()
{
    if(mpMasterPageDescriptor)
    {
        SetChanged();
        sdr::contact::ViewContact& rMasterPageViewContact = mpMasterPageDescriptor->GetUsedPage().GetViewContact();
        rMasterPageViewContact.ActionRemoved();

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

    sdr::contact::ViewContact& rMasterPageViewContact = mpMasterPageDescriptor->GetUsedPage().GetViewContact();
    rMasterPageViewContact.ActionChanged();
    rMasterPageViewContact.SetVisibleLayers(rNew);
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

// MasterPage interface
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrPage::ImplGetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
    /* FASTBOOL bLayerSorted,*/ Color& rCol, FASTBOOL bSkipBackgroundShape) const
{
    if (pModel==NULL) return FALSE;
    FASTBOOL bRet=SdrObjList::GetFillColor(rPnt,rVisLayers,/*bLayerSorted,*/rCol);
    if (!bRet && !bMaster)
    {
        // nun zu den Masterpages
        if(TRG_HasMasterPage())
        {
            SetOfByte aSet(rVisLayers);
            aSet &= TRG_GetMasterPageVisibleLayers();
            SdrPage& rMasterPage = TRG_GetMasterPage();

            // #108867# Don't fall back to background shape on
            // master pages. This is later handled by
            // GetBackgroundColor, and is necessary to cater for
            // the silly ordering: 1. shapes, 2. master page
            // shapes, 3. page background, 4. master page
            // background.
            bRet = rMasterPage.ImplGetFillColor(rPnt, aSet, rCol, TRUE);
        }
    }

    // #108867# Only now determine background color from background shapes
    if( !bRet && !bSkipBackgroundShape )
    {
        rCol = GetBackgroundColor();
        return TRUE;
    }

    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrPage::GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
    /*FASTBOOL bLayerSorted,*/ Color& rCol) const
{
    // #108867# Wrapper for ImplGetFillColor. Used to properly set the
    // bSkipBackgroundShape parameter. Never skip background shape on
    // first level of recursion
    return ImplGetFillColor(rPnt,rVisLayers,/*bLayerSorted,*/rCol,FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const SdrPageGridFrameList* SdrPage::GetGridFrameList(const SdrPageView* pPV, const Rectangle* pRect) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    DBG_ERROR("SdrPage::ReadData(): binfilter still used, but should not (!)");
//  if (rIn.GetError()!=0) return;
//  SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//#ifdef DBG_UTIL
//  aCompat.SetID("SdrPage");
//#endif
//
//  if (rHead.GetVersion()>=11) {
//      // damit ich meine eigenen SubRecords erkenne (ab V11)
//      char cMagic[4];
//      if (rIn.Read(cMagic,4)!=4 || memcmp(cMagic,SdrIOJoeMagic,4)!=0) {
//          rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
//          return;
//      }
//  }
//
//  { // MiscellaneousData ab V11 eingepackt
//      SdrDownCompat* pPageMiscCompat=NULL;
//      if (rHead.GetVersion()>=11) {
//          pPageMiscCompat=new SdrDownCompat(rIn,STREAM_READ);
//#ifdef DBG_UTIL
//          pPageMiscCompat->SetID("SdrPage(Miscellaneous)");
//#endif
//      }
//      rIn>>nWdt;
//      rIn>>nHgt;
//      rIn>>nBordLft;
//      rIn>>nBordUpp;
//      rIn>>nBordRgt;
//      rIn>>nBordLwr;
//      USHORT n;
//      rIn>>n; //aName;
//      if (pPageMiscCompat!=NULL) {
//          delete pPageMiscCompat;
//      }
//  }
//
//  FASTBOOL bEnde=FALSE;
//  while (rIn.GetError()==0 && !rIn.IsEof() && !bEnde) {
//      SdrIOHeaderLookAhead aHead(rIn);   // Layerdefinitionen lesen
//      if (aHead.IsID(SdrIOLayrID)) {
//          SdrLayer* pLay=new SdrLayer;       // Layerdefinition lesen
//          rIn>>*pLay;
//          pLayerAdmin->InsertLayer(pLay);
//      }
//      //#110094#-10
//      //else if (aHead.IsID(SdrIOLSetID))
//      //{
//      //  SdrLayerSet* pSet=new SdrLayerSet; // Layersetdefinition lesen
//      //  rIn>>*pSet;
//      //  pLayerAdmin->InsertLayerSet(pSet);
//      //}
//      else
//      // Fuer den Fall der Faelle kann hier ww. MPgDscr oder MPgDscrList stehen
//      if (aHead.IsID(SdrIOMPgDID)) { // Masterpagedescriptor
//          SdrMasterPageDescriptor aDscr;
//          rIn>>aDscr;
//          aMasters.Insert(aDscr);
//      } else
//      if (aHead.IsID(SdrIOMPDLID)) { // MasterpagedescriptorList
//          SdrMasterPageDescriptorList aDscrList;
//          rIn>>aDscrList;
//          USHORT nAnz=aDscrList.GetCount();
//          for (USHORT nNum=0; nNum<nAnz; nNum++) {
//              aMasters.Insert(aDscrList[nNum]);
//          }
//      } else bEnde=TRUE;
//  }
//
//  if (rHead.GetVersion()>=1) {
//  } else {
//      USHORT nMaAnz=0,i;
//      rIn>>nMaAnz;
//      for (i=0; i<nMaAnz; i++) {
//          USHORT nMaPgNum;
//          rIn>>nMaPgNum;
//          InsertMasterPage(nMaPgNum);
//      }
//  }
//  SdrObjList::Load(rIn,*this);  // Liste der Objekte lesen
//
//  if ( rHead.GetVersion() >= 16 )
//  {
//      BOOL bBackgroundObj = FALSE;
//      rIn >> bBackgroundObj;
//      if( bBackgroundObj )
//      {
//          SdrObjIOHeaderLookAhead aHead( rIn,STREAM_READ );
//          if ( !aHead.IsEnde() )
//          {
//              pBackgroundObj = SdrObjFactory::MakeNewObject( aHead.nInventor, aHead.nIdentifier, this );
//              if ( bBackgroundObj )
//                  rIn >> *pBackgroundObj;
//          }
//          else
//              aHead.SkipRecord(); // skip end mark
//      }
//  }
//
//  // #88340#
//  if(!aMasters.GetCount() && !IsMasterPage())
//  {
//      if(pModel && pModel->GetMasterPageCount() > 2)
//      {
//          // This is not allowed. Create a dummy entry
//          // to compensate this error.
//          SdrMasterPageDescriptor aDscr(1/*PageMaster*/);
//          aMasters.Insert(aDscr);
//      }
//      else
//      {
//          SdrMasterPageDescriptor aDscr(0);
//          aMasters.Insert(aDscr);
//      }
//  }
}

void SdrPage::WriteData(SvStream& rOut) const
{
    DBG_ERROR("SdrPage::WriteData(): binfilter still used, but should not (!)");
//  SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
//#ifdef DBG_UTIL
//  aCompat.SetID("SdrPage");
//#endif
//  rOut.Write(SdrIOJoeMagic,4); // damit ich meine eigenen SubRecords erkenne (ab V11)
//  { // MiscellaneousData ab V11 eingepackt
//      SdrDownCompat aPageMiscCompat(rOut,STREAM_WRITE);
//#ifdef DBG_UTIL
//      aPageMiscCompat.SetID("SdrPage(Miscellaneous)");
//#endif
//      rOut<<nWdt;
//      rOut<<nHgt;
//      rOut<<nBordLft;
//      rOut<<nBordUpp;
//      rOut<<nBordRgt;
//      rOut<<nBordLwr;
//      USHORT n=0;
//      rOut<<n; //rPg.aName;
//  }
//
//  USHORT i; // Lokale Layerdefinitionen der Seite
//  for (i=0; i<pLayerAdmin->GetLayerCount(); i++) {
//      rOut<<*pLayerAdmin->GetLayer(i);
//  }
//  //#110094#-10
//  //for (i=0; i<pLayerAdmin->GetLayerSetCount(); i++) {
//  //  rOut<<*pLayerAdmin->GetLayerSet(i);
//  //}
//
//  rOut<<aMasters;
//  SdrObjList::Save(rOut);
//
//  BOOL bBackgroundObj = pBackgroundObj ? TRUE : FALSE;
//  rOut << bBackgroundObj;
//  if( pBackgroundObj )
//      rOut << *pBackgroundObj;
}

SvStream& operator>>(SvStream& rIn, SdrPage& rPg)
{
    if (rIn.GetError()!=0) return rIn;
    SdrIOHeader aHead(rIn,STREAM_READ);
    rPg.ReadData(aHead,rIn);
    return rIn;
}

SvStream& operator<<(SvStream& rOut, const SdrPage& rPg)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,!rPg.bMaster ? SdrIOPageID : SdrIOMaPgID);
    rPg.WriteData(rOut);
    return rOut;
}

XubString SdrPage::GetLayoutName() const
{
    // Die wollte Dieter haben.
    return String();
}

void SdrPage::SetBackgroundObj( SdrObject* pObj )
{
    if ( pObj )
    {
        pObj->SetPage( this );
        pObj->SetModel( pModel );
        pObj->SetLayer( 1 );        // Nothing known about the backgroundlayer...
    }
    delete pBackgroundObj, pBackgroundObj = pObj;
}

void SdrPage::SetInserted( FASTBOOL bIns )
{
    if( bInserted != bIns )
    {
        bInserted = bIns;

        SdrObjListIter aIter( *this, IM_FLAT );
         while ( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();
            if ( pObj->ISA(SdrOle2Obj) )
            {
                if( bInserted )
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
    uno::Reference< uno::XInterface > xPage( mxUnoPage );

    if( !xPage.is() )
    {
        // create one
        xPage = createUnoPage();

        mxUnoPage = xPage;
    }

    return xPage;
}

uno::Reference< uno::XInterface > SdrPage::createUnoPage()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt =
#ifndef SVX_LIGHT
        static_cast<cppu::OWeakObject*>( new SvxFmDrawPage( this ) );
#else
        static_cast<cppu::OWeakObject*>( new SvxDrawPage( this ) );
#endif
    return xInt;
}

SfxStyleSheet* SdrPage::GetTextStyleSheetForObject( SdrObject* pObj ) const
{
    return pObj->GetStyleSheet();
}

FASTBOOL SdrPage::HasTransparentObjects( BOOL bCheckForAlphaChannel ) const
{
    FASTBOOL bRet = FALSE;

    for( ULONG n = 0, nCount = GetObjCount(); ( n < nCount ) && !bRet; n++ )
        if( GetObj( n )->IsTransparent( bCheckForAlphaChannel ) )
            bRet = TRUE;

    return bRet;
}

/** returns an averaged background color of this page */
Color SdrPage::GetBackgroundColor( SdrPageView* pView ) const
{
    Color aColor;

    if( (NULL == pView) || (pView->GetApplicationDocumentColor() == COL_AUTO) )
    {
        svtools::ColorConfig aColorConfig;
        aColor = aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor;
    }
    else
    {
        aColor = pView->GetApplicationDocumentColor();
    }

    // first, see if we have a background object
    SdrObject* pBackgroundObj = NULL;


    if( IsMasterPage() )
    {
        if( GetObjCount() )
            pBackgroundObj = GetObj( 0 );
    }
    else
    {
        pBackgroundObj = GetBackgroundObj();
        if( NULL == pBackgroundObj )
        {
            // if not, see if we have a masterpage and get that background object
            if(TRG_HasMasterPage())
            {
                SdrPage& rMasterPage = TRG_GetMasterPage();

                if(rMasterPage.GetObjCount())
                {
                    pBackgroundObj = rMasterPage.GetObj( 0 );
                }
            }
        }
    }

    if( pBackgroundObj )
    {
        const SfxItemSet& rSet = pBackgroundObj->GetMergedItemSet();
        GetDraftFillColor( rSet, aColor );
    }

    return aColor;
}

/** *deprecated, use GetBackgroundColor with SdrPageView */
Color SdrPage::GetBackgroundColor() const
{
    return GetBackgroundColor( NULL );
}

/** this method returns true if the object from the ViewObjectContact should
    be visible on this page while rendering.
    bEdit selects if visibility test is for an editing view or a final render,
    like printing.
*/
bool SdrPage::checkVisibility(
    ::sdr::contact::ViewObjectContact& rOriginal,
    ::sdr::contact::DisplayInfo& rDisplayInfo,
    bool bEdit )
{
    // this will be handled in the application if needed
    return true;
}

// #110094# DrawContact support: Methods for handling Page changes
void SdrPage::ActionChanged() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();
}

#ifdef GCC
// Dummy-Implementationen fuer Deklarationen in svdpage.hxx
Bitmap      SdrPage::GetBitmap(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders) const {}
GDIMetaFile SdrPage::GetMetaFile(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders) {}
#endif

//////////////////////////////////////////////////////////////////////////////
// use new redirector instead of pPaintProc

StandardCheckVisisbilityRedirector::StandardCheckVisisbilityRedirector()
:   ViewObjectContactRedirector()
{
}

StandardCheckVisisbilityRedirector::~StandardCheckVisisbilityRedirector()
{
}

void StandardCheckVisisbilityRedirector::PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        if(pObject->GetPage())
        {
            if(pObject->GetPage()->checkVisibility(rOriginal, rDisplayInfo, false))
            {
                rOriginal.PaintObject(rDisplayInfo);
            }
        }
    }
    else
    {
        // not an object, maybe a page
        rOriginal.PaintObject(rDisplayInfo);
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
