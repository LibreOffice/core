/*************************************************************************
 *
 *  $RCSfile: svdpage.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-19 09:49:17 $
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
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
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
#include "fmglob.hxx"

#ifndef SVX_LIGHT
#include "fmdpage.hxx"
#else
#include "unopage.hxx"
#endif

#define CONVERT_STARIMAGE_OLE_OBJECT_TO_GRAPHIC 1

using namespace ::com::sun::star;

DBG_NAME(SdrObjList);

TYPEINIT0(SdrObjList);

SdrObjList::SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList):
    aList(1024,64,64)
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
    aList(1024,64,64)
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
        SdrObject* pDO=pSO->Clone(pPage,pModel);
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
    ULONG nAnz=GetObjCount();
    for (ULONG no=0; no<nAnz; no++) {
        SdrObject* pObj=GetObj(no);
        delete pObj;
    }
    aList.Clear();
    if (pModel!=NULL && nAnz!=0) {
        pModel->SetChanged();
        SdrHint aHint(HINT_OBJLISTCLEARED);
        aHint.SetPage(pPage);
        aHint.SetObjList(this);
        pModel->Broadcast(aHint);
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
            aOutRect=pObj->GetBoundRect();
            aSnapRect=pObj->GetSnapRect();
        } else {
            aOutRect.Union(pObj->GetBoundRect());
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
        aList.Insert(pObj,nPos);
        if (nPos<nAnz) bObjOrdNumsDirty=TRUE;
        pObj->SetOrdNum(nPos);
        pObj->SetObjList(this);
        pObj->SetPage(pPage);
        if (!bRectsDirty) {
            aOutRect.Union(pObj->GetBoundRect());
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
        if(pOwnerObj && !GetObjCount())
        {
            // damit der graue Rahmen des leeren Gruppenobjekts korrekt weggemalt wird
            pOwnerObj->SendRepaintBroadcast();
        }

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
            pOwnerObj->SendRepaintBroadcast();
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
    SdrObject* pObj=(SdrObject*)aList.Remove(nObjNum);
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
    SdrObject* pObj=(SdrObject*)aList.Remove(nObjNum);
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
        if (pOwnerObj!=NULL && GetObjCount()==0) {
            // damit der graue Rahmen des leeren Gruppenobjekts korrekt gemalt wird
            pOwnerObj->SendRepaintBroadcast();
        }
    }
    return pObj;
}

SdrObject* SdrObjList::NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    SdrObject* pObj=(SdrObject*)aList.GetObject(nObjNum);
    DBG_ASSERT(pObj!=NULL,"SdrObjList::ReplaceObject: Object zum Removen nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::ReplaceObject: ZObjekt hat keinen Inserted-Status");
        pObj->SetInserted(FALSE);
        pObj->SetObjList(NULL);
        pObj->SetPage(NULL);
        aList.Replace(pNewObj,nObjNum);
        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);
        pNewObj->SetInserted(TRUE);
        SetRectsDirty();
    }
    return pObj;
}

SdrObject* SdrObjList::ReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    //ULONG nAnz=GetObjCount();
    SdrObject* pObj=(SdrObject*)aList.GetObject(nObjNum);
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
        aList.Replace(pNewObj,nObjNum);
        pNewObj->SetOrdNum(nObjNum);
        pNewObj->SetObjList(this);
        pNewObj->SetPage(pPage);
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
    SdrObject* pObj=(SdrObject*)aList.GetObject(nOldObjNum);
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::NbcSetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::NbcSetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        aList.Remove(nOldObjNum);
        aList.Insert(pObj,nNewObjNum);
        pObj->SetOrdNum(nNewObjNum);
        bObjOrdNumsDirty=TRUE;
    }
    return pObj;
}

SdrObject* SdrObjList::SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum)
{
    SdrObject* pObj=(SdrObject*)aList.GetObject(nOldObjNum);
    if (nOldObjNum==nNewObjNum) return pObj;
    DBG_ASSERT(pObj!=NULL,"SdrObjList::SetObjectOrdNum: Object nicht gefunden");
    if (pObj!=NULL) {
        DBG_ASSERT(pObj->IsInserted(),"SdrObjList::SetObjectOrdNum: ZObjekt hat keinen Inserted-Status");
        /*if (pModel!=NULL) {
            // Hier muss ein anderer Broadcast her!
            if (pObj->GetPage()!=NULL) pModel->Broadcast(SdrHint(*pObj));
        }*/
        aList.Remove(nOldObjNum);
        aList.Insert(pObj,nNewObjNum);
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

FASTBOOL SdrObjList::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec, FASTBOOL bRestoreColors) const
{
    BOOL bOk(TRUE);
    BOOL bWasNotActive = rInfoRec.bNotActive;
    BOOL bIsEnteredGroup(FALSE);
    UINT32 nWasDrawMode = rXOut.GetOutDev()->GetDrawMode();

    if(!rInfoRec.bOriginalDrawModeSet)
    {
        // Original-Paintmode retten
        ((SdrPaintInfoRec&)rInfoRec).bOriginalDrawModeSet = TRUE;
        ((SdrPaintInfoRec&)rInfoRec).nOriginalDrawMode = rXOut.GetOutDev()->GetDrawMode();
    }

    if((rInfoRec.pPV && rInfoRec.pPV->GetObjList() == this)
        || (rInfoRec.nPaintMode & SDRPAINTMODE_MASTERPAGE))
    {
        bIsEnteredGroup = TRUE;
    }

    if(bIsEnteredGroup && bWasNotActive)
    {
        // auf aktive Elemente schalten
        ((SdrPaintInfoRec&)rInfoRec).bNotActive = FALSE;
    }

    if(rInfoRec.pPV && rInfoRec.bNotActive)
    {
        if(rInfoRec.pPV->GetView().DoVisualizeEnteredGroup())
        {
            // Darstellung schmal
            rXOut.GetOutDev()->SetDrawMode(nWasDrawMode | (
                DRAWMODE_GHOSTEDLINE|DRAWMODE_GHOSTEDFILL|DRAWMODE_GHOSTEDTEXT|DRAWMODE_GHOSTEDBITMAP|DRAWMODE_GHOSTEDGRADIENT));
        }
    }
    else
    {
        // Darstellung normal
        rXOut.GetOutDev()->SetDrawMode(rInfoRec.nOriginalDrawMode);
    }

    bOk = Paint(rXOut, rInfoRec, bRestoreColors, IMP_PAGEPAINT_NORMAL);

    if(bIsEnteredGroup && bWasNotActive)
    {
        // Zurueck auf Ursprung, Zustand wieder verlassen
        ((SdrPaintInfoRec&)rInfoRec).bNotActive = TRUE;
    }

    // Darstellung restaurieren
    rXOut.GetOutDev()->SetDrawMode(nWasDrawMode);

    return bOk;
}


#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

FASTBOOL SdrObjList::Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec, FASTBOOL bRestoreColors, USHORT nImpMode) const
{
    FASTBOOL bOk=TRUE;
    FASTBOOL bBrk=FALSE;
    ULONG nObjAnz=GetObjCount();
    if (nObjAnz==0) return TRUE;
    USHORT nEvent=rInfoRec.nBrkEvent;
    const SetOfByte* pVisiLayer=&rInfoRec.aPaintLayer;
    FASTBOOL bPrinter=rInfoRec.bPrinter;
    OutputDevice* pOut=rXOut.GetOutDev();
    Rectangle aCheckRect(rInfoRec.aCheckRect);
    FASTBOOL bDrawAll=aCheckRect.IsEmpty();
    ImpSdrHdcMerk aHDCMerk(*pOut,SDRHDC_SAVEPENANDBRUSHANDFONT,bRestoreColors);
    FASTBOOL bColorsDirty=FALSE;
    if (bDrawAll || aCheckRect.IsOver(GetAllObjBoundRect())) {
        Application* pAppPtr=NULL;
        if (nEvent!=0) pAppPtr=GetpApp();
        SetOfByte aPaintLayer(*pVisiLayer);
        USHORT nPaintCycles = 1;
        SdrLayerID nLayerId = 0;

        if (pModel)
        {
            const SdrLayerAdmin& rLayerAdmin = pModel->GetLayerAdmin();
            nLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), FALSE);
        }

        if( pPage == NULL || ( !pPage->IsMasterPage() && aPaintLayer.IsSet( nLayerId ) ) )
        {
            // Der ControlLayer soll gezeichnet werden
            // Wenn Controls existieren, wird der ControlLayer als letztes gezeichnet
            SetOfByte aTestLayerSet = aPaintLayer;
            aTestLayerSet.Clear(nLayerId);

            if (!aTestLayerSet.IsEmpty())
            {
                // Es soll nicht nur der ControlLayer gezeichnet werden
                ULONG nObjNum=0;
                while (nObjNum<nObjAnz && nPaintCycles < 2)
                {
                    if (GetObj(nObjNum)->GetLayer() == nLayerId)
                    {
                        // Objekt auf ControlLayer gefunden
                        // Der ControlLayer wird ersteinmal unsichtbar geschaltet
                        nPaintCycles = 2;
                        aPaintLayer.Clear(nLayerId);
                    }

                    nObjNum++;
                }
            }
        }

        for (USHORT nCycle = 1; nCycle <= nPaintCycles; nCycle++)
        {
            USHORT      nPaintImpMode = nImpMode;
            FASTBOOL    bNormal = ( nPaintImpMode == IMP_PAGEPAINT_NORMAL );
            FASTBOOL    bCachePrepare = ( nPaintImpMode == IMP_PAGEPAINT_PREPARE_CACHE );
            FASTBOOL    bBGCachePrepare = ( nPaintImpMode == IMP_PAGEPAINT_PREPARE_BG_CACHE );
            FASTBOOL    bCachePaint = ( nPaintImpMode == IMP_PAGEPAINT_PAINT_CACHE );
            FASTBOOL    bBGCachePaint = ( nPaintImpMode == IMP_PAGEPAINT_PAINT_BG_CACHE );
            FASTBOOL    bPaintFlag = ( bNormal || bCachePrepare || bBGCachePrepare );

            if( nCycle == 2 )
            {
                // Im zweiten Durchgang nur den ControlLayer zeichnen
                aPaintLayer.ClearAll();
                aPaintLayer.Set(nLayerId);
            }

            ULONG nObjNum = 0UL;

            while( ( nObjNum < nObjAnz ) && !bBrk )
            {
                SdrObject* pObj = GetObj( nObjNum );

                if( nObjNum == 0 && eListKind == SDROBJLIST_MASTERPAGE &&
                    pPage && pPage->IsMasterPage() && rInfoRec.pPV )
                {
                    // painting pages background obj instead of masterpages background obj
                    SdrPage* pPg = rInfoRec.pPV->GetPage();
                    SdrObject* pBackgroundObj = pPg ? pPg->GetBackgroundObj() : NULL;
                    if( pBackgroundObj )
                    {
                        if( rXOut.GetOutDev()->GetDrawMode() == DRAWMODE_DEFAULT )
                        {
                            pObj = pBackgroundObj;
                            Point aPos ( pPage->GetLftBorder(), pPage->GetUppBorder() );
                            Size aSize ( pPage->GetSize() );
                            aSize.Width()  -= pPage->GetLftBorder() + pPage->GetRgtBorder() - 1;
                            aSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder() - 1;
                            Rectangle aLogicRect( aPos, aSize );

                            if( pBackgroundObj->GetLogicRect() != aLogicRect )
                            {
                                pBackgroundObj->SetLogicRect( aLogicRect );
                                pBackgroundObj->RecalcBoundRect();
                            }
                        }
                        else
                            pObj = NULL;

                    }
                }

                if( pObj && ( bDrawAll || aCheckRect.IsOver( pObj->GetBoundRect() ) ) )
                {
                    SdrObjList* pSubList = pObj->GetSubList();

                    // Gruppenobjekte beruecksichtigen sichtbare Layer selbst (Ansonsten nur Painten, wenn Layer sichtbar)
                    if( pSubList!=NULL || ((!bPrinter || pObj->IsPrintable()) && aPaintLayer.IsSet(pObj->GetLayer())) )
                    {
                        if( !bNormal && !pObj->IsMasterCachable() )
                        {
                            if( bCachePrepare || bBGCachePrepare )
                                bBrk = TRUE, bPaintFlag = FALSE;
                            else if( bCachePaint || bBGCachePaint )
                            {
                                bPaintFlag = bNormal = TRUE;
                            }
                        }

                        if( bPaintFlag )
                        {
                            if( pObj->IsNeedColorRestore() )
                            {
                                if (bColorsDirty && bRestoreColors)
                                    aHDCMerk.Restore(*pOut);

                                bColorsDirty=FALSE;
                            }
                            else
                                bColorsDirty=TRUE; // andere aendern die Farben

                            if( rInfoRec.pPaintProc!=NULL )
                            {
                                SdrPaintProcRec aRec(pObj,rXOut,rInfoRec);
                                Link aLink(*rInfoRec.pPaintProc);
                                aLink.Call(&aRec); // sollte mal 'nen ReturnCode liefern
                            }
                            else
                            {
                                bOk=pObj->Paint(rXOut,rInfoRec);

//////////////////////////////////////////////////////////////////////////////
//
//  Vector2D aTRScale;
//  double fTRShear;
//  double fTRRotate;
//  Vector2D aTRTranslate;
//  Matrix3D aOrigMat;
//  XPolyPolygon aTRPolyPolygon;
//
//  BOOL bIsPath = pObj->TRGetBaseGeometry(aOrigMat, aTRPolyPolygon);
//  aOrigMat.DecomposeAndCorrect(aTRScale, fTRShear, fTRRotate, aTRTranslate);
//  Vector2D aVectorTranslate;
//  aVectorTranslate.X() = FRound(aTRTranslate.X());
//  aVectorTranslate.Y() = FRound(aTRTranslate.Y());
//
//  Point aPoint(aVectorTranslate.X(), aVectorTranslate.Y());
//  Rectangle aTRBaseRect(
//      aPoint,
//      Size(FRound(aTRScale.X()), FRound(aTRScale.Y())));
//
//  Color aLineColorMerk(rXOut.GetOutDev()->GetLineColor());
//  Color aFillColorMerk(rXOut.GetOutDev()->GetFillColor());
//  rXOut.GetOutDev()->SetFillColor();
//
//  rXOut.GetOutDev()->SetLineColor(COL_BLACK);
//  rXOut.GetOutDev()->DrawRect(aTRBaseRect);
//
//  if(bIsPath)
//  {
//      rXOut.GetOutDev()->SetLineColor(COL_LIGHTRED);
//      XPolyPolygon aTRPoPo(aTRPolyPolygon);
//      aTRPoPo.Move(aTRBaseRect.Left(), aTRBaseRect.Top());
//      sal_uInt16 nCount(aTRPoPo.Count());
//      for(sal_uInt16 a(0); a < nCount; a++)
//          rXOut.GetOutDev()->DrawPolygon(XOutCreatePolygon(aTRPoPo[a], rXOut.GetOutDev()));
//  }
//
//  rXOut.GetOutDev()->SetLineColor(aLineColorMerk);
//  rXOut.GetOutDev()->SetFillColor(aFillColorMerk);
//
//  static BOOL bDoTestSetAllGeometry(FALSE);
//  if(bDoTestSetAllGeometry)
//      pObj->TRSetBaseGeometry(aOrigMat, aTRPolyPolygon);
//
//
//////////////////////////////////////////////////////////////////////////////
                            }

                            // nach dem ersten Objekt bei reinem Hintergrundcache
                            // sollen die folgenden Objekte natuerlich nicht gezeichnet werden
                            if( bBGCachePrepare )
                                bPaintFlag = FALSE;
                        }
                        else if( bBGCachePaint )
                            bPaintFlag = TRUE;
                    }

                    if( bOk && nEvent != 0 )
                        bOk = !pAppPtr->AnyInput( nEvent );

                    if( !bOk )
                        bBrk = TRUE;
                }
                nObjNum++;
            }
        }
    }

    if (bColorsDirty && bRestoreColors)
        aHDCMerk.Restore(*pOut);

    return bOk;
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
            if (R.IsOver(pObj->GetBoundRect())) {
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

void SdrObjList::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
{
    ULONG nAnz=GetObjCount();
    ULONG nNum=0;
    while (nNum<nAnz) {
        GetObj(nNum)->BurnInStyleSheetAttributes( bPseudoSheetsOnly );
        nNum++;
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

void SdrObjList::RestartAllAnimations(SdrPageView* pPageView) const
{
    ULONG nAnz=GetObjCount();
    ULONG nNum=0;
    while (nNum<nAnz) {
        GetObj(nNum)->RestartAnimation(pPageView);
        nNum++;
    }
}

FASTBOOL SdrObjList::ImpGetFillColor(SdrObject* pObj, Color& rCol) const
{
    return GetDraftFillColor(pObj->GetItemSet(),rCol);
}

FASTBOOL SdrObjList::GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
                                  FASTBOOL bLayerSorted, Color& rCol) const
{
    if (pModel==NULL) return FALSE;
    FASTBOOL bRet=FALSE;
    FASTBOOL bMaster=pPage!=NULL ? pPage->IsMasterPage() : FALSE;
    for (ULONG no=GetObjCount(); !bRet && no>0; ) {
        no--;
        SdrObject* pObj=GetObj(no);
        SdrObjList* pOL=pObj->GetSubList();
        if (pOL!=NULL) { // Aha, Gruppenobjekt
            bRet=pOL->GetFillColor(rPnt,rVisLayers,bLayerSorted,rCol);
        } else {
            SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pObj);
            if (pTextObj!=NULL &&
                pObj->IsClosedObj() && rVisLayers.IsSet(pObj->GetLayer()) &&
                (!bMaster || !pObj->IsNotVisibleAsMaster()) &&
                pObj->GetBoundRect().IsInside(rPnt) &&
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
        nObjNum--;
        SdrObject* pObj=GetObj(nObjNum);
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
    if (rIn.GetError()!=0) return;
    FASTBOOL bEnde=FALSE;
    SdrInsertReason aReason(SDRREASON_STREAMING);
    while (rIn.GetError()==0 && !rIn.IsEof() && !bEnde) {
        SdrObjIOHeaderLookAhead aHead(rIn,STREAM_READ);
        if (!aHead.IsEnde()) {
            SdrObject* pObj=SdrObjFactory::MakeNewObject(aHead.nInventor,aHead.nIdentifier,&rPage);
            if (pObj!=NULL) {
                rIn>>*pObj;
#ifdef CONVERT_STARIMAGE_OLE_OBJECT_TO_GRAPHIC
                if( ( pObj->GetObjIdentifier() == OBJ_OLE2 ) &&
                    ( pObj->GetObjInventor() == SdrInventor ) &&
                    ( ( SdrOle2Obj*) pObj )->GetProgName() == String( RTL_CONSTASCII_USTRINGPARAM( "StarImage" ) ) )
                {
                    SotStorage*     pModelStorage = pModel->GetModelStorage();
                    const String    aSimStorageName( ( (SdrOle2Obj*) pObj )->GetName() );

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
#endif
                InsertObject(pObj,CONTAINER_APPEND,&aReason);
            } else { // aha, das wil keiner. Also ueberlesen.
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
                aStr += char(aHead.nInventor);
                aStr += char(aHead.nInventor >> 8);
                aStr += char(aHead.nInventor >> 16);
                aStr += char(aHead.nInventor >> 24);
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
        } else {
            bEnde=TRUE;
            aHead.SkipRecord(); // die Endemarke weglesen
        }
        SdrModel* pMd=pModel;
        if (pMd==NULL) pMd=rPage.GetModel();
        if (pMd!=NULL) pMd->DoProgress(rIn.Tell());
    }
}

void SdrObjList::AfterRead()
{
    ULONG nAnz=GetObjCount();
    for (ULONG i=0; i<nAnz; i++) {
        GetObj(i)->AfterRead();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvStream& operator<<(SvStream& rOut, const SdrMasterPageDescriptor& rMDP)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOMPgDID);
    rOut<<rMDP.nPgNum;
    rOut<<rMDP.aVisLayers;
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrMasterPageDescriptor& rMDP)
{
    if (rIn.GetError()!=0) return rIn;
    SdrIOHeader aHead(rIn,STREAM_READ);
    rIn>>rMDP.nPgNum;
    rIn>>rMDP.aVisLayers;
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMasterPageDescriptorList::Clear()
{
    USHORT nAnz=GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetObject(i);
    }
    aList.Clear();
}

void SdrMasterPageDescriptorList::operator=(const SdrMasterPageDescriptorList& rSrcList)
{
    Clear();
    USHORT nAnz=rSrcList.GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        Insert(rSrcList[i]);
    }
}

SvStream& operator<<(SvStream& rOut, const SdrMasterPageDescriptorList& rMPDL)
{
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOMPDLID);
    USHORT nAnz=rMPDL.GetCount();
    rOut<<nAnz;
    for (USHORT i=0; i<nAnz; i++) {
        rOut<<rMPDL[i];
    }
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrMasterPageDescriptorList& rMPDL)
{
    if (rIn.GetError()!=0) return rIn;
    SdrIOHeader aHead(rIn,STREAM_READ);
    rMPDL.Clear();
    USHORT nAnz;
    rIn>>nAnz;
    for (USHORT i=0; i<nAnz; i++) {
        SdrMasterPageDescriptor* pMPD=new SdrMasterPageDescriptor;
        rIn>>*pMPD;
        rMPDL.aList.Insert(pMPD,CONTAINER_APPEND);
    }
    return rIn;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrPage,SdrObjList);

SdrPage::SdrPage(SdrModel& rNewModel, FASTBOOL bMasterPage):
    SdrObjList(&rNewModel,this),
    pBackgroundObj( NULL )
{
    pLayerAdmin=new SdrLayerAdmin(&rNewModel.GetLayerAdmin());
    bMaster=bMasterPage;
    bInserted=FALSE;
    aPrefVisiLayers.SetAll();
    nWdt=10;
    nHgt=10;
    nBordLft=0;
    nBordUpp=0;
    nBordRgt=0;
    nBordLwr=0;
    nPageNum=0;
    eListKind=bMasterPage ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;
    bSwappingLocked=FALSE;
    bObjectsNotPersistent=FALSE;
}

SdrPage::SdrPage(const SdrPage& rSrcPage):
    SdrObjList(rSrcPage.pModel,this),
    pBackgroundObj( NULL )
{
    pLayerAdmin=new SdrLayerAdmin(rSrcPage.pModel->GetLayerAdmin());
    *this=rSrcPage;
    eListKind=bMaster ? SDROBJLIST_MASTERPAGE : SDROBJLIST_DRAWPAGE;
}

SdrPage::~SdrPage()
{
    delete pBackgroundObj;
    delete pLayerAdmin;
}

void SdrPage::operator=(const SdrPage& rSrcPage)
{
    SdrObjList::operator=(rSrcPage);
    pPage=this;
    bMaster        =rSrcPage.bMaster        ;
    bSwappingLocked=rSrcPage.bSwappingLocked;
    aPrefVisiLayers=rSrcPage.aPrefVisiLayers;
    nWdt           =rSrcPage.nWdt           ;
    nHgt           =rSrcPage.nHgt           ;
    nBordLft       =rSrcPage.nBordLft       ;
    nBordUpp       =rSrcPage.nBordUpp       ;
    nBordRgt       =rSrcPage.nBordRgt       ;
    nBordLwr       =rSrcPage.nBordLwr       ;
    nPageNum       =rSrcPage.nPageNum       ;
    aMasters       =rSrcPage.aMasters       ;
    bObjectsNotPersistent=rSrcPage.bObjectsNotPersistent;

    if( rSrcPage.pBackgroundObj )
    {
        pBackgroundObj = rSrcPage.pBackgroundObj->Clone();
          pBackgroundObj->SetPage( this );
        pBackgroundObj->SetModel( pModel );
    }
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
    pModel->SetChanged();
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
    pModel->SetChanged();
}

void  SdrPage::SetLftBorder(INT32 nBorder)
{
    nBordLft=nBorder; pModel->SetChanged();
}

void  SdrPage::SetUppBorder(INT32 nBorder)
{
    nBordUpp=nBorder; pModel->SetChanged();
}

void  SdrPage::SetRgtBorder(INT32 nBorder)
{
    nBordRgt=nBorder; pModel->SetChanged();
}

void  SdrPage::SetLwrBorder(INT32 nBorder)
{
    nBordLwr=nBorder; pModel->SetChanged();
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

Point SdrPage::GetOffset() const
{
    return Point();
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
        if (pModel->IsMPgNumsDirty()) ((SdrModel*)pModel)->RecalcPageNums(TRUE);
    } else {
        if (pModel->IsPagNumsDirty()) ((SdrModel*)pModel)->RecalcPageNums(FALSE);
    }
    return nPageNum;
}

void SdrPage::SetChanged()
{
    pModel->SetChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPage::SendRepaintBroadcast() const
{
    if (bInserted && pModel!=NULL) pModel->Broadcast(SdrHint(*this));
}

USHORT SdrPage::GetMasterPagePos(USHORT nPgNum) const
{
    USHORT nAnz=aMasters.GetCount();
    for (USHORT i=0; i<nAnz; i++) {
        USHORT nMaPgNum=aMasters[i].GetPageNum();
        if (nMaPgNum==nPgNum) return i;
    }
    return SDRPAGE_NOTFOUND;
}

void SdrPage::InsertMasterPage(USHORT nPgNum, USHORT nPos)
{
    aMasters.Insert(nPgNum,nPos);
    SetChanged();
    SendRepaintBroadcast();
}

void SdrPage::InsertMasterPage(const SdrMasterPageDescriptor& rMPD, USHORT nPos)
{
    aMasters.Insert(rMPD,nPos);
    SetChanged();
    SendRepaintBroadcast();
}

void SdrPage::RemoveMasterPage(USHORT nPos)
{
    if (nPos<aMasters.GetCount()) {
        aMasters.Remove(nPos);
        SetChanged();
        SendRepaintBroadcast();
    }
}

void SdrPage::MoveMasterPage(USHORT nPos, USHORT nNewPos)
{
    if (nPos<aMasters.GetCount()) {
        aMasters.Move(nPos,nNewPos);
        SendRepaintBroadcast();
    }
}

SdrPage* SdrPage::GetMasterPage(USHORT nPos) const
{
    USHORT nPgNum=GetMasterPageNum(nPos);
    SdrPage* pPg=pModel->GetMasterPage(nPgNum);
    return pPg;
}

void SdrPage::SetMasterPageNum(USHORT nPgNum, USHORT nPos)
{
    aMasters[nPos].SetPageNum(nPgNum);
    SendRepaintBroadcast();
}

void SdrPage::SetMasterPageVisibleLayers(const SetOfByte& rVL, USHORT nPos)
{
    aMasters[nPos].SetVisibleLayers(rVL);
    SendRepaintBroadcast();
}

void SdrPage::SetMasterPageDescriptor(const SdrMasterPageDescriptor& rMPD, USHORT nPos)
{
    aMasters[nPos]=rMPD;
    SendRepaintBroadcast();
}

void SdrPage::SetMasterPageDescriptorList(const SdrMasterPageDescriptorList& rMPDL)
{
    aMasters=rMPDL;
    SendRepaintBroadcast();
}

void SdrPage::ImpMasterPageRemoved(USHORT nMasterPageNum)
{
    USHORT nMasterAnz=GetMasterPageCount();
    for (USHORT nm=nMasterAnz; nm>0;) {
        nm--;
        USHORT nNum=aMasters[nm].GetPageNum();
        if (nNum==nMasterPageNum) { RemoveMasterPage(nm); SendRepaintBroadcast(); }
        if (nNum>nMasterPageNum) {
            // Hintere anpassen wegen Verschiebung durch entfernen
            aMasters[nm].SetPageNum(USHORT(nNum-1));
        }
    }
}

void SdrPage::ImpMasterPageInserted(USHORT nMasterPageNum)
{
    USHORT nMasterAnz=GetMasterPageCount();
    for (USHORT nm=nMasterAnz; nm>0;) {
        nm--;
        USHORT nNum=aMasters[nm].GetPageNum();
        if (nNum>=nMasterPageNum) {
            // Hintere anpassen wegen Verschiebung durch einfuegen
            aMasters[nm].SetPageNum(nNum+1);
        }
    }
}

void SdrPage::ImpMasterPageMoved(USHORT nMasterPageNum, USHORT nNewMasterPageNum)
{
    USHORT nMasterAnz=GetMasterPageCount();
    for (USHORT nm=nMasterAnz; nm>0;) {
        nm--;
        USHORT nNum=aMasters[nm].GetPageNum();
        if (nNum==nMasterPageNum) {
            aMasters[nm].SetPageNum(nNewMasterPageNum);
        } else {
            // Hintere anpassen wegen Verschiebung durch entfernen und einfuegen
            USHORT nNeuNum=nNum;
            if (nNeuNum>nMasterPageNum) nNeuNum--;
            if (nNeuNum>=nNewMasterPageNum) nNeuNum++;
            aMasters[nm].SetPageNum(nNeuNum);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrPage::GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
                               FASTBOOL bLayerSorted, Color& rCol) const
{
    if (pModel==NULL) return FALSE;
    FASTBOOL bRet=SdrObjList::GetFillColor(rPnt,rVisLayers,bLayerSorted,rCol);
    if (!bRet && !bMaster) {
        // nun zu den Masterpages
        USHORT nMasterAnz=GetMasterPageCount();
        for (USHORT nMaster=nMasterAnz; nMaster>0 && !bRet;) {
            nMaster--;
            const SdrMasterPageDescriptor& rMaster=GetMasterPageDescriptor(nMaster);
            SetOfByte aSet(rVisLayers);
            aSet&=rMaster.GetVisibleLayers();
            SdrPage* pMaster=pModel->GetMasterPage(rMaster.GetPageNum());
            if (pMaster!=NULL) {
                bRet=pMaster->GetFillColor(rPnt,aSet,bLayerSorted,rCol);
            }
        }
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const SdrPageGridFrameList* SdrPage::GetGridFrameList(const SdrPageView* pPV, const Rectangle* pRect) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPage");
#endif

    if (rHead.GetVersion()>=11) {
        // damit ich meine eigenen SubRecords erkenne (ab V11)
        char cMagic[4];
        if (rIn.Read(cMagic,4)!=4 || memcmp(cMagic,SdrIOJoeMagic,4)!=0) {
            rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
            return;
        }
    }

    { // MiscellaneousData ab V11 eingepackt
        SdrDownCompat* pPageMiscCompat=NULL;
        if (rHead.GetVersion()>=11) {
            pPageMiscCompat=new SdrDownCompat(rIn,STREAM_READ);
#ifdef DBG_UTIL
            pPageMiscCompat->SetID("SdrPage(Miscellaneous)");
#endif
        }
        rIn>>nWdt;
        rIn>>nHgt;
        rIn>>nBordLft;
        rIn>>nBordUpp;
        rIn>>nBordRgt;
        rIn>>nBordLwr;
        USHORT n;
        rIn>>n; //aName;
        if (pPageMiscCompat!=NULL) {
            delete pPageMiscCompat;
        }
    }

    FASTBOOL bEnde=FALSE;
    while (rIn.GetError()==0 && !rIn.IsEof() && !bEnde) {
        SdrIOHeaderLookAhead aHead(rIn);   // Layerdefinitionen lesen
        if (aHead.IsID(SdrIOLayrID)) {
            SdrLayer* pLay=new SdrLayer;       // Layerdefinition lesen
            rIn>>*pLay;
            pLayerAdmin->InsertLayer(pLay);
        } else
        if (aHead.IsID(SdrIOLSetID)) {
            SdrLayerSet* pSet=new SdrLayerSet; // Layersetdefinition lesen
            rIn>>*pSet;
            pLayerAdmin->InsertLayerSet(pSet);
        } else
        // Fuer den Fall der Faelle kann hier ww. MPgDscr oder MPgDscrList stehen
        if (aHead.IsID(SdrIOMPgDID)) { // Masterpagedescriptor
            SdrMasterPageDescriptor aDscr;
            rIn>>aDscr;
            aMasters.Insert(aDscr);
        } else
        if (aHead.IsID(SdrIOMPDLID)) { // MasterpagedescriptorList
            SdrMasterPageDescriptorList aDscrList;
            rIn>>aDscrList;
            USHORT nAnz=aDscrList.GetCount();
            for (USHORT nNum=0; nNum<nAnz; nNum++) {
                aMasters.Insert(aDscrList[nNum]);
            }
        } else bEnde=TRUE;
    }

    if (rHead.GetVersion()>=1) {
    } else {
        USHORT nMaAnz=0,i;
        rIn>>nMaAnz;
        for (i=0; i<nMaAnz; i++) {
            USHORT nMaPgNum;
            rIn>>nMaPgNum;
            InsertMasterPage(nMaPgNum);
        }
    }
    SdrObjList::Load(rIn,*this);  // Liste der Objekte lesen

    if ( rHead.GetVersion() >= 16 )
    {
        BOOL bBackgroundObj = FALSE;
        rIn >> bBackgroundObj;
        if( bBackgroundObj )
        {
            SdrObjIOHeaderLookAhead aHead( rIn,STREAM_READ );
            if ( !aHead.IsEnde() )
            {
                pBackgroundObj = SdrObjFactory::MakeNewObject( aHead.nInventor, aHead.nIdentifier, this );
                if ( bBackgroundObj )
                    rIn >> *pBackgroundObj;
            }
            else
                aHead.SkipRecord(); // skip end mark
        }
    }
}

void SdrPage::WriteData(SvStream& rOut) const
{
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPage");
#endif
    rOut.Write(SdrIOJoeMagic,4); // damit ich meine eigenen SubRecords erkenne (ab V11)
    { // MiscellaneousData ab V11 eingepackt
        SdrDownCompat aPageMiscCompat(rOut,STREAM_WRITE);
#ifdef DBG_UTIL
        aPageMiscCompat.SetID("SdrPage(Miscellaneous)");
#endif
        rOut<<nWdt;
        rOut<<nHgt;
        rOut<<nBordLft;
        rOut<<nBordUpp;
        rOut<<nBordRgt;
        rOut<<nBordLwr;
        USHORT n=0;
        rOut<<n; //rPg.aName;
    }

    USHORT i; // Lokale Layerdefinitionen der Seite
    for (i=0; i<pLayerAdmin->GetLayerCount(); i++) {
        rOut<<*pLayerAdmin->GetLayer(i);
    }
    for (i=0; i<pLayerAdmin->GetLayerSetCount(); i++) {
        rOut<<*pLayerAdmin->GetLayerSet(i);
    }

    rOut<<aMasters;
    SdrObjList::Save(rOut);

    BOOL bBackgroundObj = pBackgroundObj ? TRUE : FALSE;
    rOut << bBackgroundObj;
    if( pBackgroundObj )
        rOut << *pBackgroundObj;
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

#ifdef GCC
// Dummy-Implementationen fuer Deklarationen in svdpage.hxx
Bitmap      SdrPage::GetBitmap(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders=TRUE) const {}
GDIMetaFile SdrPage::GetMetaFile(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders=TRUE) {}
#endif

