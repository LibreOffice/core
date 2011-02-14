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

#include <svx/svdedtv.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdogrp.hxx>   // fuer's Gruppieren
#include <svx/svdovirt.hxx>  // fuer VirtualObject-Bundling (Writer)
#include <svx/svdopath.hxx>  // fuer CombineObjects
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svditer.hxx"
#include <svx/svdograf.hxx>  // fuer Possibilities
#include <svx/svdoole2.hxx>  // und Mtf-Import
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include "svdfmtf.hxx"
#include <svx/svdetc.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/eeitem.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001

// #i37011#
#include <svx/svdoashp.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@ @@@@@  @@ @@@@@@  @@ @@ @@ @@@@@ @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@   @@
//  @@    @@  @@ @@   @@    @@ @@ @@ @@    @@ @ @@
//  @@@@  @@  @@ @@   @@    @@@@@ @@ @@@@  @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@@@@@
//  @@    @@  @@ @@   @@     @@@  @@ @@    @@@ @@@
//  @@@@@ @@@@@  @@   @@      @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ImpBundleVirtObjOfMarkList()
{
  // ... fehlende Implementation
}

SdrObject* SdrEditView::GetMaxToTopObj(SdrObject* /*pObj*/) const
{
  return NULL;
}

SdrObject* SdrEditView::GetMaxToBtmObj(SdrObject* /*pObj*/) const
{
  return NULL;
}

void SdrEditView::ObjOrderChanged(SdrObject* /*pObj*/, sal_uIntPtr /*nOldPos*/, sal_uIntPtr /*nNewPos*/)
{
}

void SdrEditView::MovMarkedToTop()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditMovToTop),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_MOVTOTOP);

        SortMarkedObjects();
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { // Ordnums muessen alle stimmen!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        sal_Bool bChg=sal_False;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0)
            {
                nNewPos=sal_uIntPtr(pOL->GetObjCount()-1);
                pOL0=pOL;
            }
            sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
            const Rectangle& rBR=pObj->GetCurrentBoundRect();
            sal_uIntPtr nCmpPos=nNowPos+1;
            SdrObject* pMaxObj=GetMaxToTopObj(pObj);
            if (pMaxObj!=NULL)
            {
                sal_uIntPtr nMaxPos=pMaxObj->GetOrdNum();
                if (nMaxPos!=0)
                    nMaxPos--;
                if (nNewPos>nMaxPos)
                    nNewPos=nMaxPos; // diesen nicht ueberholen.
                if (nNewPos<nNowPos)
                    nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
            }
            sal_Bool bEnd=sal_False;
            while (nCmpPos<nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==NULL)
                {
                    DBG_ERROR("MovMarkedToTop(): Vergleichsobjekt nicht gefunden");
                    bEnd=sal_True;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos--;
                    bEnd=sal_True;
                }
                else if (rBR.IsOver(pCmpObj->GetCurrentBoundRect()))
                {
                    nNewPos=nCmpPos;
                    bEnd=sal_True;
                }
                else
                {
                    nCmpPos++;
                }
            }
            if (nNowPos!=nNewPos)
            {
                bChg=sal_True;
                pOL->SetObjectOrdNum(nNowPos,nNewPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }
            nNewPos--;
        }

        if( bUndo )
            EndUndo();

        if (bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::MovMarkedToBtm()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditMovToBtm),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_MOVTOBTM);

        SortMarkedObjects();
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { // Ordnums muessen alle stimmen!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }

        sal_Bool bChg=sal_False;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=0; nm<nAnz; nm++)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0)
            {
                nNewPos=0;
                pOL0=pOL;
            }
            sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
            const Rectangle& rBR=pObj->GetCurrentBoundRect();
            sal_uIntPtr nCmpPos=nNowPos; if (nCmpPos>0) nCmpPos--;
            SdrObject* pMaxObj=GetMaxToBtmObj(pObj);
            if (pMaxObj!=NULL)
            {
                sal_uIntPtr nMinPos=pMaxObj->GetOrdNum()+1;
                if (nNewPos<nMinPos)
                    nNewPos=nMinPos; // diesen nicht ueberholen.
                if (nNewPos>nNowPos)
                    nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
            }
            sal_Bool bEnd=sal_False;
            // nNewPos ist an dieser Stelle noch die maximale Position,
            // an der das Obj hinruecken darf, ohne seinen Vorgaenger
            // (Mehrfachselektion) zu ueberholen.
            while (nCmpPos>nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==NULL)
                {
                    DBG_ERROR("MovMarkedToBtm(): Vergleichsobjekt nicht gefunden");
                    bEnd=sal_True;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos++;
                    bEnd=sal_True;
                }
                else if (rBR.IsOver(pCmpObj->GetCurrentBoundRect()))
                {
                    nNewPos=nCmpPos;
                    bEnd=sal_True;
                }
                else
                {
                    nCmpPos--;
                }
            }
            if (nNowPos!=nNewPos)
            {
                bChg=sal_True;
                pOL->SetObjectOrdNum(nNowPos,nNewPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }
            nNewPos++;
        }

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::PutMarkedToTop()
{
    PutMarkedInFrontOfObj(NULL);
}

void SdrEditView::PutMarkedInFrontOfObj(const SdrObject* pRefObj)
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditPutToTop),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_PUTTOTOP);

        SortMarkedObjects();

        if (pRefObj!=NULL)
        {
            // Damit "Vor das Objekt" auch funktioniert wenn die
            // markierten Objekte bereits vor dem Objekt stehen
            sal_uIntPtr nRefMark=TryToFindMarkedObject(pRefObj);
            SdrMark aRefMark;
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                aRefMark=*GetSdrMarkByIndex(nRefMark);
                GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
            }
            PutMarkedToBtm();
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
                SortMarkedObjects();
            }
        }
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { // Ordnums muessen alle stimmen!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        sal_Bool bChg=sal_False;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            if (pObj!=pRefObj)
            {
                SdrObjList* pOL=pObj->GetObjList();
                if (pOL!=pOL0)
                {
                    nNewPos=sal_uIntPtr(pOL->GetObjCount()-1);
                    pOL0=pOL;
                }
                sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
                SdrObject* pMaxObj=GetMaxToTopObj(pObj);
                if (pMaxObj!=NULL)
                {
                    sal_uIntPtr nMaxOrd=pMaxObj->GetOrdNum(); // geht leider nicht anders
                    if (nMaxOrd>0)
                        nMaxOrd--;
                    if (nNewPos>nMaxOrd)
                        nNewPos=nMaxOrd; // nicht ueberholen.
                    if (nNewPos<nNowPos)
                        nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                }
                if (pRefObj!=NULL)
                {
                    if (pRefObj->GetObjList()==pObj->GetObjList())
                    {
                        sal_uIntPtr nMaxOrd=pRefObj->GetOrdNum(); // geht leider nicht anders
                        if (nNewPos>nMaxOrd)
                            nNewPos=nMaxOrd; // nicht ueberholen.
                        if (nNewPos<nNowPos)
                            nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                    }
                    else
                    {
                        nNewPos=nNowPos; // andere PageView, also nicht veraendern
                    }
                }
                if (nNowPos!=nNewPos)
                {
                    bChg=sal_True;
                    pOL->SetObjectOrdNum(nNowPos,nNewPos);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }
                nNewPos--;
            } // if (pObj!=pRefObj)
        } // for-Schleife ueber alle Markierten Objekte

        if( bUndo )
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::PutMarkedToBtm()
{
    PutMarkedBehindObj(NULL);
}

void SdrEditView::PutMarkedBehindObj(const SdrObject* pRefObj)
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditPutToBtm),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_PUTTOBTM);

        SortMarkedObjects();
        if (pRefObj!=NULL)
        {
            // Damit "Hinter das Objekt" auch funktioniert wenn die
            // markierten Objekte bereits hinter dem Objekt stehen
            sal_uIntPtr nRefMark=TryToFindMarkedObject(pRefObj);
            SdrMark aRefMark;
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                aRefMark=*GetSdrMarkByIndex(nRefMark);
                GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
            }
            PutMarkedToTop();
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
                SortMarkedObjects();
            }
        }
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++) { // Ordnums muessen alle stimmen!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        sal_Bool bChg=sal_False;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=0; nm<nAnz; nm++) {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            if (pObj!=pRefObj) {
                SdrObjList* pOL=pObj->GetObjList();
                if (pOL!=pOL0) {
                    nNewPos=0;
                    pOL0=pOL;
                }
                sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
                SdrObject* pMinObj=GetMaxToBtmObj(pObj);
                if (pMinObj!=NULL) {
                    sal_uIntPtr nMinOrd=pMinObj->GetOrdNum()+1; // geht leider nicht anders
                    if (nNewPos<nMinOrd) nNewPos=nMinOrd; // nicht ueberholen.
                    if (nNewPos>nNowPos) nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                }
                if (pRefObj!=NULL) {
                    if (pRefObj->GetObjList()==pObj->GetObjList()) {
                        sal_uIntPtr nMinOrd=pRefObj->GetOrdNum(); // geht leider nicht anders
                        if (nNewPos<nMinOrd) nNewPos=nMinOrd; // nicht ueberholen.
                        if (nNewPos>nNowPos) nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                    } else {
                        nNewPos=nNowPos; // andere PageView, also nicht veraendern
                    }
                }
                if (nNowPos!=nNewPos) {
                    bChg=sal_True;
                    pOL->SetObjectOrdNum(nNowPos,nNewPos);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }
                nNewPos++;
            } // if (pObj!=pRefObj)
        } // for-Schleife ueber alle markierten Objekte

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::ReverseOrderOfMarked()
{
    SortMarkedObjects();
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    if (nMarkAnz>0)
    {
        //sal_Bool bNeedBundle=sal_False;
        sal_Bool bChg=sal_False;

        bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditRevOrder),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_REVORDER);

        sal_uIntPtr a=0;
        do {
            // Markierung ueber mehrere PageViews berueksichtigen
            sal_uIntPtr b=a+1;
            while (b<nMarkAnz && GetSdrPageViewOfMarkedByIndex(b) == GetSdrPageViewOfMarkedByIndex(a)) b++;
            b--;
            SdrObjList* pOL=GetSdrPageViewOfMarkedByIndex(a)->GetObjList();
            sal_uIntPtr c=b;
            if (a<c) { // Sicherstellen, dass die OrdNums nicht Dirty sind
                GetMarkedObjectByIndex(a)->GetOrdNum();
            }
            while (a<c) {
                SdrObject* pObj1=GetMarkedObjectByIndex(a);
                SdrObject* pObj2=GetMarkedObjectByIndex(c);
                sal_uIntPtr nOrd1=pObj1->GetOrdNumDirect();
                sal_uIntPtr nOrd2=pObj2->GetOrdNumDirect();
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj1,nOrd1,nOrd2));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj2,nOrd2-1,nOrd1));
                }
                pOL->SetObjectOrdNum(nOrd1,nOrd2);
                // Obj 2 ist um eine Position nach vorn gerutscht, deshalb nun nOrd2-1
                pOL->SetObjectOrdNum(nOrd2-1,nOrd1);
                // Verwendung von Replace statt SetOrdNum wg. Performance (Neuberechnung der Ordnums)
                a++; c--;
                bChg=sal_True;
            }
            a=b+1;
        } while (a<nMarkAnz);

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::ImpCheckToTopBtmPossible()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz==0)
        return;
    if (nAnz==1)
    { // Sonderbehandlung fuer Einzelmarkierung
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        SdrObjList* pOL=pObj->GetObjList();
        sal_uIntPtr nMax=pOL->GetObjCount();
        sal_uIntPtr nMin=0;
        sal_uIntPtr nObjNum=pObj->GetOrdNum();
        SdrObject* pRestrict=GetMaxToTopObj(pObj);
        if (pRestrict!=NULL) {
            sal_uIntPtr nRestrict=pRestrict->GetOrdNum();
            if (nRestrict<nMax) nMax=nRestrict;
        }
        pRestrict=GetMaxToBtmObj(pObj);
        if (pRestrict!=NULL) {
            sal_uIntPtr nRestrict=pRestrict->GetOrdNum();
            if (nRestrict>nMin) nMin=nRestrict;
        }
        bToTopPossible=nObjNum<sal_uIntPtr(nMax-1);
        bToBtmPossible=nObjNum>nMin;
    } else { // Mehrfachselektion
        sal_uIntPtr nm=0;
        SdrObjList* pOL0=NULL;
        long nPos0=-1;
        while (!bToBtmPossible && nm<nAnz) { // 'nach hinten' checken
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0) {
                nPos0=-1;
                pOL0=pOL;
            }
            sal_uIntPtr nPos=pObj->GetOrdNum();
            bToBtmPossible=nPos>sal_uIntPtr(nPos0+1);
            nPos0=long(nPos);
            nm++;
        }
        nm=nAnz;
        pOL0=NULL;
        nPos0=0x7FFFFFFF;
        while (!bToTopPossible && nm>0) { // 'nach vorn' checken
            nm--;
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0) {
                nPos0=pOL->GetObjCount();
                pOL0=pOL;
            }
            sal_uIntPtr nPos=pObj->GetOrdNum();
            bToTopPossible=nPos+1<sal_uIntPtr(nPos0);
            nPos0=nPos;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@    @@@@   @@   @@  @@@@@   @@  @@  @@  @@@@@
//  @@  @@  @@  @@  @@@ @@@  @@  @@  @@  @@@ @@  @@
//  @@      @@  @@  @@@@@@@  @@  @@  @@  @@@@@@  @@
//  @@      @@  @@  @@@@@@@  @@@@@   @@  @@@@@@  @@@@
//  @@      @@  @@  @@ @ @@  @@  @@  @@  @@ @@@  @@
//  @@  @@  @@  @@  @@   @@  @@  @@  @@  @@  @@  @@
//   @@@@    @@@@   @@   @@  @@@@@   @@  @@  @@  @@@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const
{
    if (pSource!=NULL) {
        SdrObjList* pOL=pSource->GetSubList();
        if (pOL!=NULL && !pSource->Is3DObj()) { // erstes Nichtgruppenobjekt aus der Gruppe holen
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            pSource=aIter.Next();
        }
    }

    if(pSource && pDest)
    {
        SfxItemSet aSet(pMod->GetItemPool(),
            SDRATTR_START,              SDRATTR_NOTPERSIST_FIRST-1,
            SDRATTR_NOTPERSIST_LAST+1,  SDRATTR_END,
            EE_ITEMS_START,             EE_ITEMS_END,
            0, 0); // #52757#, #52762#

        aSet.Put(pSource->GetMergedItemSet());

        pDest->ClearMergedItem();
        pDest->SetMergedItemSet(aSet);

        pDest->NbcSetLayer(pSource->GetLayer());
        pDest->NbcSetStyleSheet(pSource->GetStyleSheet(), sal_True);
    }
}

sal_Bool SdrEditView::ImpCanConvertForCombine1(const SdrObject* pObj) const
{
    // #69711 : new condition IsLine() to be able to combine simple Lines
    sal_Bool bIsLine(sal_False);

    const SdrPathObj* pPath = PTR_CAST(SdrPathObj,pObj);

    if(pPath)
    {
        bIsLine = pPath->IsLine();
    }

    SdrObjTransformInfoRec aInfo;
    pObj->TakeObjInfo(aInfo);

    return (aInfo.bCanConvToPath || aInfo.bCanConvToPoly || bIsLine);
}

sal_Bool SdrEditView::ImpCanConvertForCombine(const SdrObject* pObj) const
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();

            // Es muessen alle Member einer Gruppe konvertierbar sein
            if(!ImpCanConvertForCombine1(pObj1))
            {
                return sal_False;
            }
        }
    }
    else
    {
        if(!ImpCanConvertForCombine1(pObj))
        {
            return sal_False;
        }
    }

    return sal_True;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon1(const SdrObject* pObj, sal_Bool bCombine) const
{
    basegfx::B2DPolyPolygon aRetval;
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);

    if(bCombine && pPath && !pObj->GetOutlinerParaObject())
    {
        aRetval = pPath->GetPathPoly();
    }
    else
    {
        SdrObject* pConvObj = pObj->ConvertToPolyObj(bCombine, sal_False);

        if(pConvObj)
        {
            SdrObjList* pOL = pConvObj->GetSubList();

            if(pOL)
            {
                SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj1 = aIter.Next();
                    pPath = PTR_CAST(SdrPathObj, pObj1);

                    if(pPath)
                    {
                        aRetval.append(pPath->GetPathPoly());
                    }
                }
            }
            else
            {
                pPath = PTR_CAST(SdrPathObj, pConvObj);

                if(pPath)
                {
                    aRetval = pPath->GetPathPoly();
                }
            }

            SdrObject::Free( pConvObj );
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon(const SdrObject* pObj, sal_Bool bCombine) const
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        basegfx::B2DPolyPolygon aRetval;
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();
            aRetval.append(ImpGetPolyPolygon1(pObj1, bCombine));
        }

        return aRetval;
    }
    else
    {
        return ImpGetPolyPolygon1(pObj, bCombine);
    }
}

basegfx::B2DPolygon SdrEditView::ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon) const
{
    const sal_uInt32 nPolyCount(rPolyPolygon.count());

    if(0L == nPolyCount)
    {
        return basegfx::B2DPolygon();
    }
    else if(1L == nPolyCount)
    {
        return rPolyPolygon.getB2DPolygon(0L);
    }
    else
    {
        basegfx::B2DPolygon aRetval(rPolyPolygon.getB2DPolygon(0L));

        for(sal_uInt32 a(1L); a < nPolyCount; a++)
        {
            basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));

            if(aRetval.count())
            {
                if(aCandidate.count())
                {
                    const basegfx::B2DPoint aCA(aCandidate.getB2DPoint(0L));
                    const basegfx::B2DPoint aCB(aCandidate.getB2DPoint(aCandidate.count() - 1L));
                    const basegfx::B2DPoint aRA(aRetval.getB2DPoint(0L));
                    const basegfx::B2DPoint aRB(aRetval.getB2DPoint(aRetval.count() - 1L));

                    const double fRACA(basegfx::B2DVector(aCA - aRA).getLength());
                    const double fRACB(basegfx::B2DVector(aCB - aRA).getLength());
                    const double fRBCA(basegfx::B2DVector(aCA - aRB).getLength());
                    const double fRBCB(basegfx::B2DVector(aCB - aRB).getLength());

                    const double fSmallestRA(fRACA < fRACB ? fRACA : fRACB);
                    const double fSmallestRB(fRBCA < fRBCB ? fRBCA : fRBCB);

                    if(fSmallestRA < fSmallestRB)
                    {
                        // flip result
                        aRetval.flip();
                    }

                    const double fSmallestCA(fRACA < fRBCA ? fRACA : fRBCA);
                    const double fSmallestCB(fRACB < fRBCB ? fRACB : fRBCB);

                    if(fSmallestCB < fSmallestCA)
                    {
                        // flip candidate
                        aCandidate.flip();
                    }

                    // append candidate to retval
                    aRetval.append(aCandidate);
                }
            }
            else
            {
                aRetval = aCandidate;
            }
        }

        return aRetval;
    }
}

// for distribution dialog function
struct ImpDistributeEntry
{
    SdrObject*                  mpObj;
    sal_Int32                       mnPos;
    sal_Int32                       mnLength;
};

DECLARE_LIST(ImpDistributeEntryList, ImpDistributeEntry*)

void SdrEditView::DistributeMarkedObjects()
{
    sal_uInt32 nMark(GetMarkedObjectCount());

    if(nMark > 2)
    {
        SfxItemSet aNewAttr(pMod->GetItemPool());
        //CHINA001 SvxDistributeDialog* pDlg = new SvxDistributeDialog(NULL, aNewAttr);
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxDistributeDialog *pDlg = pFact->CreateSvxDistributeDialog(NULL, aNewAttr);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001

            sal_uInt16 nResult = pDlg->Execute();

            if(nResult == RET_OK)
            {
                SvxDistributeHorizontal eHor = pDlg->GetDistributeHor();
                SvxDistributeVertical eVer = pDlg->GetDistributeVer();
                ImpDistributeEntryList aEntryList;
                sal_uInt32 a, nInsPos, nFullLength;

                const bool bUndo = IsUndoEnabled();
                if( bUndo )
                    BegUndo();

                if(eHor != SvxDistributeHorizontalNone)
                {
                    // build sorted entry list
                    nFullLength = 0L;

                    for(a=0;a<nMark;a++)
                    {
                        SdrMark* pMark = GetSdrMarkByIndex(a);
                        ImpDistributeEntry* pNew = new ImpDistributeEntry;

                        pNew->mpObj = pMark->GetMarkedSdrObj();
                        nInsPos = 0;

                        switch(eHor)
                        {
                            case SvxDistributeHorizontalLeft:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Left();
                                break;
                            }
                            case SvxDistributeHorizontalCenter:
                            {
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Right() + pNew->mpObj->GetSnapRect().Left()) / 2;
                                break;
                            }
                            case SvxDistributeHorizontalDistance:
                            {
                                pNew->mnLength = pNew->mpObj->GetSnapRect().GetWidth() + 1;
                                nFullLength += pNew->mnLength;
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Right() + pNew->mpObj->GetSnapRect().Left()) / 2;
                                break;
                            }
                            case SvxDistributeHorizontalRight:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Right();
                                break;
                            }
                            default: break;
                        }

                        while(nInsPos < aEntryList.Count() && aEntryList.GetObject(nInsPos)->mnPos < pNew->mnPos)
                            nInsPos++;

                        aEntryList.Insert(pNew, nInsPos);
                    }

                    if(eHor == SvxDistributeHorizontalDistance)
                    {
                        // calc room in-between
                        sal_Int32 nWidth = GetAllMarkedBoundRect().GetWidth() + 1;
                        double fStepWidth = ((double)nWidth - (double)nFullLength) / (double)(aEntryList.Count() - 1);
                        double fStepStart = (double)aEntryList.GetObject(0)->mnPos;
                        fStepStart += fStepWidth + (double)((aEntryList.GetObject(0)->mnLength + aEntryList.GetObject(1)->mnLength) / 2);

                        // move entries 1..n-1
                        for(a=1;a<aEntryList.Count()-1;a++)
                        {
                            ImpDistributeEntry* pCurr = aEntryList.GetObject(a);
                            ImpDistributeEntry* pNext = aEntryList.GetObject(a+1);
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(nDelta, 0));
                            fStepStart += fStepWidth + (double)((pCurr->mnLength + pNext->mnLength) / 2);
                        }
                    }
                    else
                    {
                        // calc distances
                        sal_Int32 nWidth = aEntryList.GetObject(aEntryList.Count() - 1)->mnPos - aEntryList.GetObject(0)->mnPos;
                        double fStepWidth = (double)nWidth / (double)(aEntryList.Count() - 1);
                        double fStepStart = (double)aEntryList.GetObject(0)->mnPos;
                        fStepStart += fStepWidth;

                        // move entries 1..n-1
                        for(a=1;a<aEntryList.Count()-1;a++)
                        {
                            ImpDistributeEntry* pCurr = aEntryList.GetObject(a);
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(nDelta, 0));
                            fStepStart += fStepWidth;
                        }
                    }

                    // clear list
                    while(aEntryList.Count())
                        delete aEntryList.Remove((sal_uIntPtr)0L);
                }

                if(eVer != SvxDistributeVerticalNone)
                {
                    // build sorted entry list
                    nFullLength = 0L;

                    for(a=0;a<nMark;a++)
                    {
                        SdrMark* pMark = GetSdrMarkByIndex(a);
                        ImpDistributeEntry* pNew = new ImpDistributeEntry;

                        pNew->mpObj = pMark->GetMarkedSdrObj();
                        nInsPos = 0;

                        switch(eVer)
                        {
                            case SvxDistributeVerticalTop:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Top();
                                break;
                            }
                            case SvxDistributeVerticalCenter:
                            {
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Bottom() + pNew->mpObj->GetSnapRect().Top()) / 2;
                                break;
                            }
                            case SvxDistributeVerticalDistance:
                            {
                                pNew->mnLength = pNew->mpObj->GetSnapRect().GetHeight() + 1;
                                nFullLength += pNew->mnLength;
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Bottom() + pNew->mpObj->GetSnapRect().Top()) / 2;
                                break;
                            }
                            case SvxDistributeVerticalBottom:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Bottom();
                                break;
                            }
                            default: break;
                        }

                        while(nInsPos < aEntryList.Count() && aEntryList.GetObject(nInsPos)->mnPos < pNew->mnPos)
                            nInsPos++;

                        aEntryList.Insert(pNew, nInsPos);
                    }

                    if(eVer == SvxDistributeVerticalDistance)
                    {
                        // calc room in-between
                        sal_Int32 nHeight = GetAllMarkedBoundRect().GetHeight() + 1;
                        double fStepWidth = ((double)nHeight - (double)nFullLength) / (double)(aEntryList.Count() - 1);
                        double fStepStart = (double)aEntryList.GetObject(0)->mnPos;
                        fStepStart += fStepWidth + (double)((aEntryList.GetObject(0)->mnLength + aEntryList.GetObject(1)->mnLength) / 2);

                        // move entries 1..n-1
                        for(a=1;a<aEntryList.Count()-1;a++)
                        {
                            ImpDistributeEntry* pCurr = aEntryList.GetObject(a);
                            ImpDistributeEntry* pNext = aEntryList.GetObject(a+1);
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(0, nDelta));
                            fStepStart += fStepWidth + (double)((pCurr->mnLength + pNext->mnLength) / 2);
                        }
                    }
                    else
                    {
                        // calc distances
                        sal_Int32 nHeight = aEntryList.GetObject(aEntryList.Count() - 1)->mnPos - aEntryList.GetObject(0)->mnPos;
                        double fStepWidth = (double)nHeight / (double)(aEntryList.Count() - 1);
                        double fStepStart = (double)aEntryList.GetObject(0)->mnPos;
                        fStepStart += fStepWidth;

                        // move entries 1..n-1
                        for(a=1;a<aEntryList.Count()-1;a++)
                        {
                            ImpDistributeEntry* pCurr = aEntryList.GetObject(a);
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(0, nDelta));
                            fStepStart += fStepWidth;
                        }
                    }

                    // clear list
                    while(aEntryList.Count())
                        delete aEntryList.Remove((sal_uIntPtr)0L);
                }

                // UNDO-Comment and end of UNDO
                SetUndoComment(ImpGetResStr(STR_DistributeMarkedObjects));

                if( bUndo )
                    EndUndo();
            }

            delete(pDlg);
        }
    }
}

void SdrEditView::MergeMarkedObjects(SdrMergeMode eMode)
{
    // #i73441# check content
    if(AreObjectsMarked())
    {
        SdrMarkList aRemove;
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo();

        sal_uInt32 nInsPos=0xFFFFFFFF;
        const SdrObject* pAttrObj = NULL;
        basegfx::B2DPolyPolygon aMergePolyPolygonA;
        basegfx::B2DPolyPolygon aMergePolyPolygonB;

        SdrObjList* pInsOL = NULL;
        SdrPageView* pInsPV = NULL;
        sal_Bool bFirstObjectComplete(sal_False);

        // make sure selected objects are contour objects
        // since now basegfx::tools::adaptiveSubdivide() is used, it is no longer
        // necessary to use ConvertMarkedToPolyObj which will subdivide curves using the old
        // mechanisms. In a next step the polygon clipper will even be able to clip curves...
        // ConvertMarkedToPolyObj(sal_True);
        ConvertMarkedToPathObj(sal_True);
        OSL_ENSURE(AreObjectsMarked(), "no more objects selected after preparations (!)");

        for(sal_uInt32 a=0;a<GetMarkedObjectCount();a++)
        {
            SdrMark* pM = GetSdrMarkByIndex(a);
            SdrObject* pObj = pM->GetMarkedSdrObj();

            if(ImpCanConvertForCombine(pObj))
            {
                if(!pAttrObj)
                    pAttrObj = pObj;

                nInsPos = pObj->GetOrdNum() + 1;
                pInsPV = pM->GetPageView();
                pInsOL = pObj->GetObjList();

                // #i76891# use single iter from SJ here whcih works on SdrObjects and takes
                // groups into account by itself
                SdrObjListIter aIter(*pObj, IM_DEEPWITHGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pCandidate = aIter.Next();
                    SdrPathObj* pPathObj = PTR_CAST(SdrPathObj, pCandidate);
                    if(pPathObj)
                    {
                        basegfx::B2DPolyPolygon aTmpPoly(pPathObj->GetPathPoly());

                        // #i76891# unfortunately ConvertMarkedToPathObj has converted all
                        // involved polygon data to curve segments, even if not necessary.
                        // It is better to try to reduce to more simple polygons.
                        aTmpPoly = basegfx::tools::simplifyCurveSegments(aTmpPoly);

                        // for each part polygon as preparation, remove self-intersections
                        // correct orientations and get rid of evtl. neutral polygons.
                        aTmpPoly = basegfx::tools::prepareForPolygonOperation(aTmpPoly);

                        if(!bFirstObjectComplete)
                        {
                            // #i111987# Also need to collect ORed source shape when more than
                            // a single polygon is involved
                            if(aMergePolyPolygonA.count())
                            {
                                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aTmpPoly);
                            }
                            else
                            {
                                aMergePolyPolygonA = aTmpPoly;
                            }
                        }
                        else
                        {
                            if(aMergePolyPolygonB.count())
                            {
                                // to topologically correctly collect the 2nd polygon
                                // group it is necessary to OR the parts (each is seen as
                                // XOR-FillRule polygon and they are drawn over each-other)
                                aMergePolyPolygonB = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonB, aTmpPoly);
                            }
                            else
                            {
                                aMergePolyPolygonB = aTmpPoly;
                            }
                        }
                    }
                }

                // was there something added to the first poly?
                if(!bFirstObjectComplete && aMergePolyPolygonA.count())
                {
                    bFirstObjectComplete = sal_True;
                }

                // move object to temporary delete list
                aRemove.InsertEntry(SdrMark(pObj, pM->GetPageView()));
            }
        }

        switch(eMode)
        {
            case SDR_MERGE_MERGE:
            {
                // merge all contained parts (OR)
                static bool bTestXOR(false);
                if(bTestXOR)
                {
                    aMergePolyPolygonA = basegfx::tools::solvePolygonOperationXor(aMergePolyPolygonA, aMergePolyPolygonB);
                }
                else
                {
                    aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
                }
                break;
            }
            case SDR_MERGE_SUBSTRACT:
            {
                // Substract B from A
                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationDiff(aMergePolyPolygonA, aMergePolyPolygonB);
                break;
            }
            case SDR_MERGE_INTERSECT:
            {
                // AND B and A
                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
                break;
            }
        }

        // #i73441# check insert list before taking actions
        if(pInsOL)
        {
            SdrPathObj* pPath = new SdrPathObj(OBJ_PATHFILL, aMergePolyPolygonA);
            ImpCopyAttributes(pAttrObj, pPath);
            SdrInsertReason aReason(SDRREASON_VIEWCALL, pAttrObj);
            pInsOL->InsertObject(pPath, nInsPos, &aReason);
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));
            MarkObj(pPath, pInsPV, sal_False, sal_True);
        }

        aRemove.ForceSort();
        switch(eMode)
        {
            case SDR_MERGE_MERGE:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeMergePoly),
                    aRemove.GetMarkDescription());
                break;
            }
            case SDR_MERGE_SUBSTRACT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeSubstractPoly),
                    aRemove.GetMarkDescription());
                break;
            }
            case SDR_MERGE_INTERSECT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeIntersectPoly),
                    aRemove.GetMarkDescription());
                break;
            }
        }
        DeleteMarkedList(aRemove);

        if( bUndo )
            EndUndo();
    }
}

void SdrEditView::CombineMarkedObjects(sal_Bool bNoPolyPoly)
{
    // #105899# Start of Combine-Undo put to front, else ConvertMarkedToPolyObj would
    // create a 2nd Undo-action and Undo-Comment.

    bool bUndo = IsUndoEnabled();

    // Undo-String will be set later
    if( bUndo )
        BegUndo(String(), String(), bNoPolyPoly ? SDRREPFUNC_OBJ_COMBINE_ONEPOLY : SDRREPFUNC_OBJ_COMBINE_POLYPOLY);

    // #105899# First, guarantee that all objects are converted to polyobjects,
    // especially for SdrGrafObj with bitmap filling this is necessary to not
    // loose the bitmap filling.

    // #i12392#
    // ConvertMarkedToPolyObj was too strong here, it will loose quality and
    // information when curve objects are combined. This can be replaced by
    // using ConvertMarkedToPathObj without changing the previous fix.

    // #i21250#
    // Instead of simply passing sal_True as LineToArea, use bNoPolyPoly as info
    // if this command is a 'Combine' or a 'Connect' command. On Connect it's sal_True.
    // To not concert line segments with a set line width to polygons in that case,
    // use this info. Do not convert LineToArea on Connect commands.
    // ConvertMarkedToPathObj(!bNoPolyPoly);

    // #114310#
    // This is used for Combine and Connect. In no case it is necessary to force
    // the content to curve, but it is also not good to force to polygons. Thus,
    // curve is the less information loosing one. Remember: This place is not
    // used for merge.
    // LineToArea is never necessary, both commands are able to take over the
    // set line style and to display it correctly. Thus, i will use a
    // ConvertMarkedToPathObj with a sal_False in any case. Only drawback is that
    // simple polygons will be changed to curves, but with no information loss.
    ConvertMarkedToPathObj(sal_False /* bLineToArea */);

    // continue as before
    basegfx::B2DPolyPolygon aPolyPolygon;
    SdrObjList* pAktOL = 0L;
    SdrMarkList aRemoveMerker;

    SortMarkedObjects();
    sal_uInt32 nInsPos(0xFFFFFFFF);
    SdrObjList* pInsOL = 0L;
    SdrPageView* pInsPV = 0L;
    const sal_uInt32 nAnz(GetMarkedObjectCount());
    const SdrObject* pAttrObj = 0L;

    for(sal_uInt32 a(nAnz); a > 0L; )
    {
        a--;
        SdrMark* pM = GetSdrMarkByIndex(a);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        SdrObjList* pThisOL = pObj->GetObjList();

        if(pAktOL != pThisOL)
        {
            pAktOL = pThisOL;
        }

        if(ImpCanConvertForCombine(pObj))
        {
            // Obj merken fuer Attribute kopieren
            pAttrObj = pObj;

            // unfortunately ConvertMarkedToPathObj has converted all
            // involved polygon data to curve segments, even if not necessary.
            // It is better to try to reduce to more simple polygons.
            basegfx::B2DPolyPolygon aTmpPoly(basegfx::tools::simplifyCurveSegments(ImpGetPolyPolygon(pObj, sal_True)));
            aPolyPolygon.insert(0L, aTmpPoly);

            if(!pInsOL)
            {
                nInsPos = pObj->GetOrdNum() + 1L;
                pInsPV = pM->GetPageView();
                pInsOL = pObj->GetObjList();
            }

            aRemoveMerker.InsertEntry(SdrMark(pObj, pM->GetPageView()));
        }
    }

    if(bNoPolyPoly)
    {
        basegfx::B2DPolygon aCombinedPolygon(ImpCombineToSinglePolygon(aPolyPolygon));
        aPolyPolygon.clear();
        aPolyPolygon.append(aCombinedPolygon);
    }

    const sal_uInt32 nPolyCount(aPolyPolygon.count());

    if(nPolyCount)
    {
        SdrObjKind eKind = OBJ_PATHFILL;

        if(nPolyCount > 1L)
        {
            aPolyPolygon.setClosed(true);
        }
        else
        {
            // auf Polyline Checken
            const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0L));
            const sal_uInt32 nPointCount(aPolygon.count());

            if(nPointCount <= 2L)
            {
                eKind = OBJ_PATHLINE;
            }
            else
            {
                if(!aPolygon.isClosed())
                {
                    const basegfx::B2DPoint aPointA(aPolygon.getB2DPoint(0L));
                    const basegfx::B2DPoint aPointB(aPolygon.getB2DPoint(nPointCount - 1L));
                    const double fDistance(basegfx::B2DVector(aPointB - aPointA).getLength());
                    const double fJoinTolerance(10.0);

                    if(fDistance < fJoinTolerance)
                    {
                        aPolyPolygon.setClosed(true);
                    }
                    else
                    {
                        eKind = OBJ_PATHLINE;
                    }
                }
            }
        }

        SdrPathObj* pPath = new SdrPathObj(eKind,aPolyPolygon);

        // Attribute des untersten Objekts
        ImpCopyAttributes(pAttrObj, pPath);

        // #100408# If LineStyle of pAttrObj is XLINE_NONE force to XLINE_SOLID to make visible.
        const XLineStyle eLineStyle = ((const XLineStyleItem&)pAttrObj->GetMergedItem(XATTR_LINESTYLE)).GetValue();
        const XFillStyle eFillStyle = ((const XFillStyleItem&)pAttrObj->GetMergedItem(XATTR_FILLSTYLE)).GetValue();

        // #110635#
        // Take fill style/closed state of pAttrObj in account when deciding to change the line style
        sal_Bool bIsClosedPathObj(pAttrObj->ISA(SdrPathObj) && ((SdrPathObj*)pAttrObj)->IsClosed());

        if(XLINE_NONE == eLineStyle && (XFILL_NONE == eFillStyle || !bIsClosedPathObj))
        {
            pPath->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        }

        SdrInsertReason aReason(SDRREASON_VIEWCALL,pAttrObj);
        pInsOL->InsertObject(pPath,nInsPos,&aReason);
        if( bUndo )
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));

        // #111111#
        // Here was a severe error: Without UnmarkAllObj, the new object was marked
        // additionally to the two ones which are deleted below. As long as those are
        // in the UNDO there is no problem, but as soon as they get deleted, the
        // MarkList will contain deleted objects -> GPF.
        UnmarkAllObj(pInsPV);
        MarkObj(pPath, pInsPV, sal_False, sal_True);
    }

    // UndoComment aus den tatsaechlich verwendeten Objekten zusammenbauen
    aRemoveMerker.ForceSort(); // wichtig fuer Remove (s.u.)
    if( bUndo )
        SetUndoComment(ImpGetResStr(bNoPolyPoly?STR_EditCombine_OnePoly:STR_EditCombine_PolyPoly),aRemoveMerker.GetMarkDescription());

    // die tatsaechlich verwendeten Objekten aus der Liste entfernen
    DeleteMarkedList(aRemoveMerker);
    if( bUndo )
        EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@   @@   @@@@   @@   @@   @@@@   @@  @@  @@@@@@  @@     @@@@@
//  @@  @@  @@  @@  @@  @@@ @@@  @@  @@  @@@ @@    @@    @@     @@
//  @@  @@  @@  @@      @@@@@@@  @@  @@  @@@@@@    @@    @@     @@
//  @@  @@  @@   @@@@   @@@@@@@  @@@@@@  @@@@@@    @@    @@     @@@@
//  @@  @@  @@      @@  @@ @ @@  @@  @@  @@ @@@    @@    @@     @@
//  @@  @@  @@  @@  @@  @@   @@  @@  @@  @@  @@    @@    @@     @@
//  @@@@@   @@   @@@@   @@   @@  @@  @@  @@  @@    @@    @@@@@  @@@@@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrEditView::ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPolygon, sal_Bool bMakeLines) const
{
    sal_Bool bCan(sal_False);
    const sal_uInt32 nPolygonCount(rPpolyPolygon.count());

    if(nPolygonCount >= 2L)
    {
        // #i69172# dismantle makes sense with 2 or more polygons in a polyPolygon
        bCan = sal_True;
    }
    else if(bMakeLines && 1L == nPolygonCount)
    {
        // #i69172# ..or with at least 2 edges (curves or lines)
        const basegfx::B2DPolygon aPolygon(rPpolyPolygon.getB2DPolygon(0L));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount > 2L)
        {
            bCan = sal_True;
        }
    }

    return bCan;
}

sal_Bool SdrEditView::ImpCanDismantle(const SdrObject* pObj, sal_Bool bMakeLines) const
{
    sal_Bool bOtherObjs(sal_False);    // sal_True=andere Objekte ausser PathObj's vorhanden
    sal_Bool bMin1PolyPoly(sal_False); // sal_True=mind. 1 PolyPolygon mit mehr als ein Polygon vorhanden
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL)
    {
        // Aha, Gruppenobjekt. Also alle Member ansehen.
        // Alle muessen PathObjs sein !
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore() && !bOtherObjs)
        {
            const SdrObject* pObj1 = aIter.Next();
            const SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj1);

            if(pPath)
            {
                if(ImpCanDismantle(pPath->GetPathPoly(), bMakeLines))
                {
                    bMin1PolyPoly = sal_True;
                }

                SdrObjTransformInfoRec aInfo;
                pObj1->TakeObjInfo(aInfo);

                if(!aInfo.bCanConvToPath)
                {
                    // Passiert z.B. im Falle Fontwork (Joe, 28-11-95)
                    bOtherObjs = sal_True;
                }
            }
            else
            {
                bOtherObjs = sal_True;
            }
        }
    }
    else
    {
        const SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);
        const SdrObjCustomShape* pCustomShape = PTR_CAST(SdrObjCustomShape, pObj);

        // #i37011#
        if(pPath)
        {
            if(ImpCanDismantle(pPath->GetPathPoly(),bMakeLines))
            {
                bMin1PolyPoly = sal_True;
            }

            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            // #69711 : new condition IsLine() to be able to break simple Lines
            if(!(aInfo.bCanConvToPath || aInfo.bCanConvToPoly) && !pPath->IsLine())
            {
                // Passiert z.B. im Falle Fontwork (Joe, 28-11-95)
                bOtherObjs = sal_True;
            }
        }
        else if(pCustomShape)
        {
            if(bMakeLines)
            {
                // allow break command
                bMin1PolyPoly = sal_True;
            }
        }
        else
        {
            bOtherObjs = sal_True;
        }
    }
    return bMin1PolyPoly && !bOtherObjs;
}

void SdrEditView::ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uIntPtr& rPos, SdrPageView* pPV, sal_Bool bMakeLines)
{
    const SdrPathObj* pSrcPath = PTR_CAST(SdrPathObj, pObj);
    const SdrObjCustomShape* pCustomShape = PTR_CAST(SdrObjCustomShape, pObj);

    const bool bUndo = IsUndoEnabled();

    if(pSrcPath)
    {
        // #i74631# redesigned due to XpolyPolygon removal and explicit constructors
        SdrObject* pLast = 0; // fuer die Zuweisung des OutlinerParaObject
        const basegfx::B2DPolyPolygon& rPolyPolygon(pSrcPath->GetPathPoly());
        const sal_uInt32 nPolyCount(rPolyPolygon.count());

        for(sal_uInt32 a(0); a < nPolyCount; a++)
        {
            const basegfx::B2DPolygon& rCandidate(rPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(rCandidate.count());

            if(!bMakeLines || nPointCount < 2)
            {
                SdrPathObj* pPath = new SdrPathObj((SdrObjKind)pSrcPath->GetObjIdentifier(), basegfx::B2DPolyPolygon(rCandidate));
                ImpCopyAttributes(pSrcPath, pPath);
                pLast = pPath;
                SdrInsertReason aReason(SDRREASON_VIEWCALL, pSrcPath);
                rOL.InsertObject(pPath, rPos, &aReason);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, sal_True));
                MarkObj(pPath, pPV, sal_False, sal_True);
                rPos++;
            }
            else
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                for(sal_uInt32 b(0); b < nLoopCount; b++)
                {
                    SdrObjKind eKind(OBJ_PLIN);
                    basegfx::B2DPolygon aNewPolygon;
                    const sal_uInt32 nNextIndex((b + 1) % nPointCount);

                    aNewPolygon.append(rCandidate.getB2DPoint(b));

                    if(rCandidate.areControlPointsUsed())
                    {
                        aNewPolygon.appendBezierSegment(
                            rCandidate.getNextControlPoint(b),
                            rCandidate.getPrevControlPoint(nNextIndex),
                            rCandidate.getB2DPoint(nNextIndex));
                        eKind = OBJ_PATHLINE;
                    }
                    else
                    {
                        aNewPolygon.append(rCandidate.getB2DPoint(nNextIndex));
                    }

                    SdrPathObj* pPath = new SdrPathObj(eKind, basegfx::B2DPolyPolygon(aNewPolygon));
                    ImpCopyAttributes(pSrcPath, pPath);
                    pLast = pPath;
                    SdrInsertReason aReason(SDRREASON_VIEWCALL, pSrcPath);
                    rOL.InsertObject(pPath, rPos, &aReason);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, sal_True));
                    MarkObj(pPath, pPV, sal_False, sal_True);
                    rPos++;
                }
            }
        }

        if(pLast && pSrcPath->GetOutlinerParaObject())
        {
            pLast->SetOutlinerParaObject(new OutlinerParaObject(*pSrcPath->GetOutlinerParaObject()));
        }
    }
    else if(pCustomShape)
    {
        if(bMakeLines)
        {
            // break up custom shape
            const SdrObject* pReplacement = pCustomShape->GetSdrObjectFromCustomShape();

            if(pReplacement)
            {
                SdrObject* pCandidate = pReplacement->Clone();
                DBG_ASSERT(pCandidate, "SdrEditView::ImpDismantleOneObject: Could not clone SdrObject (!)");
                pCandidate->SetModel(pCustomShape->GetModel());

                if(((SdrShadowItem&)pCustomShape->GetMergedItem(SDRATTR_SHADOW)).GetValue())
                {
                    if(pReplacement->ISA(SdrObjGroup))
                    {
                        pCandidate->SetMergedItem(SdrShadowItem(sal_True));
                    }
                }

                SdrInsertReason aReason(SDRREASON_VIEWCALL, pCustomShape);
                rOL.InsertObject(pCandidate, rPos, &aReason);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pCandidate, true));
                MarkObj(pCandidate, pPV, sal_False, sal_True);

                if(pCustomShape->HasText() && !pCustomShape->IsTextPath())
                {
                    // #i37011# also create a text object and add at rPos + 1
                    SdrTextObj* pTextObj = (SdrTextObj*)SdrObjFactory::MakeNewObject(
                        pCustomShape->GetObjInventor(), OBJ_TEXT, 0L, pCustomShape->GetModel());

                    // Copy text content
                    OutlinerParaObject* pParaObj = pCustomShape->GetOutlinerParaObject();
                    if(pParaObj)
                    {
                        pTextObj->NbcSetOutlinerParaObject(new OutlinerParaObject(*pParaObj));
                    }

                    // copy all attributes
                    SfxItemSet aTargetItemSet(pCustomShape->GetMergedItemSet());

                    // clear fill and line style
                    aTargetItemSet.Put(XLineStyleItem(XLINE_NONE));
                    aTargetItemSet.Put(XFillStyleItem(XFILL_NONE));

                    // get the text bounds and set at text object
                    Rectangle aTextBounds = pCustomShape->GetSnapRect();
                    if(pCustomShape->GetTextBounds(aTextBounds))
                    {
                        pTextObj->SetSnapRect(aTextBounds);
                    }

                    // if rotated, copy GeoStat, too.
                    const GeoStat& rSourceGeo = pCustomShape->GetGeoStat();
                    if(rSourceGeo.nDrehWink)
                    {
                        pTextObj->NbcRotate(
                            pCustomShape->GetSnapRect().Center(), rSourceGeo.nDrehWink,
                            rSourceGeo.nSin, rSourceGeo.nCos);
                    }

                    // set modified ItemSet at text object
                    pTextObj->SetMergedItemSet(aTargetItemSet);

                    // insert object
                    rOL.InsertObject(pTextObj, rPos + 1, &aReason);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pTextObj, true));
                    MarkObj(pTextObj, pPV, sal_False, sal_True);
                }
            }
        }
    }
}

void SdrEditView::DismantleMarkedObjects(sal_Bool bMakeLines)
{
    //sal_uInt32 nCnt(0);
    // Temporaere Marklist
    SdrMarkList aRemoveMerker;

    SortMarkedObjects();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        // Der Comment wird spaeter zusammengebaut
        BegUndo(String(), String(),
            bMakeLines ? SDRREPFUNC_OBJ_DISMANTLE_LINES : SDRREPFUNC_OBJ_DISMANTLE_POLYS);
    }

    sal_uIntPtr nm;
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    SdrObjList* pOL0=NULL;
    for (nm=nAnz; nm>0;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList* pOL=pObj->GetObjList();
        if (pOL!=pOL0) { pOL0=pOL; pObj->GetOrdNum(); } // sicherstellen, dass OrdNums stimmen!
        if (ImpCanDismantle(pObj,bMakeLines)) {
            aRemoveMerker.InsertEntry(SdrMark(pObj,pM->GetPageView()));
            sal_uIntPtr nPos0=pObj->GetOrdNumDirect();
            sal_uIntPtr nPos=nPos0+1;
            SdrObjList* pSubList=pObj->GetSubList();
            if (pSubList!=NULL && !pObj->Is3DObj()) {
                SdrObjListIter aIter(*pSubList,IM_DEEPNOGROUPS);
                while (aIter.IsMore()) {
                    const SdrObject* pObj1=aIter.Next();
                    ImpDismantleOneObject(pObj1,*pOL,nPos,pPV,bMakeLines);
                }
            } else {
                ImpDismantleOneObject(pObj,*pOL,nPos,pPV,bMakeLines);
            }
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj,sal_True));
            pOL->RemoveObject(nPos0);

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }

    if( bUndo )
    {
        // UndoComment aus den tatsaechlich verwendeten Objekten zusammenbauen
        SetUndoComment(ImpGetResStr(bMakeLines?STR_EditDismantle_Lines:STR_EditDismantle_Polys),aRemoveMerker.GetMarkDescription());
        // die tatsaechlich verwendeten Objekten aus der Liste entfernen
        EndUndo();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   #### ####   ###  #   # ####
//  #     #   # #   # #   # #   #
//  #  ## ####  #   # #   # ####
//  #   # #   # #   # #   # #
//   #### #   #  ###   ###  #
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::GroupMarked(const SdrObject* pUserGrp)
{
    if (AreObjectsMarked())
    {
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditGroup),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_GROUP);

            const sal_uIntPtr nAnz = GetMarkedObjectCount();
            for(sal_uIntPtr nm = nAnz; nm>0; )
            {
                // UndoActions fuer alle betroffenen Objekte anlegen
                nm--;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                    std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );
                    AddUndoActions( vConnectorUndoActions );
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject( *pObj ));
            }
        }

        SdrMarkList aNewMark;
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            SdrObjList* pAktLst=pPV->GetObjList();
            SdrObjList* pSrcLst=pAktLst;
            SdrObjList* pSrcLst0=pSrcLst;
            SdrPage*    pPage=pPV->GetPage();
            // sicherstellen, dass die OrdNums stimmen
            if (pSrcLst->IsObjOrdNumsDirty())
                pSrcLst->RecalcObjOrdNums();
            SdrObject*  pGrp=NULL;
            SdrObject*  pRefObj=NULL; // Referenz fuer InsertReason (-> rumankern im Writer)
            SdrObject*  pRefObj1=NULL; // Referenz fuer InsertReason (-> rumankern im Writer)
            SdrObjList* pDstLst=NULL;
            // Falls alle markierten Objekte aus Fremden Obj-Listen
            // kommen, kommt das Gruppenobjekt an das Ende der Liste.
            sal_uIntPtr       nInsPos=pSrcLst->GetObjCount();
            sal_Bool    bNeedInsPos=sal_True;
            for (sal_uIntPtr nm=GetMarkedObjectCount(); nm>0;)
            {
                nm--;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                if (pM->GetPageView()==pPV)
                {
                    if (pGrp==NULL)
                    {
                        if (pUserGrp!=NULL)
                            pGrp=pUserGrp->Clone();
                        if (pGrp==NULL)
                            pGrp=new SdrObjGroup;
                        pDstLst=pGrp->GetSubList();
                        DBG_ASSERT(pDstLst!=NULL,"Angebliches Gruppenobjekt liefert keine Objektliste");
                    }
                    SdrObject* pObj=pM->GetMarkedSdrObj();
                    pSrcLst=pObj->GetObjList();
                    if (pSrcLst!=pSrcLst0)
                    {
                        if (pSrcLst->IsObjOrdNumsDirty())
                            pSrcLst->RecalcObjOrdNums();
                    }
                    sal_Bool bForeignList=pSrcLst!=pAktLst;
                    sal_Bool bGrouped=pSrcLst!=pPage;
                    if (!bForeignList && bNeedInsPos)
                    {
                        nInsPos=pObj->GetOrdNum(); // ua, damit sind alle ObjOrdNum der Page gesetzt
                        nInsPos++;
                        bNeedInsPos=sal_False;
                    }
                    pSrcLst->RemoveObject(pObj->GetOrdNumDirect());
                    if (!bForeignList)
                        nInsPos--; // InsertPos korregieren
                    SdrInsertReason aReason(SDRREASON_VIEWCALL);
                    pDstLst->InsertObject(pObj,0,&aReason);
                    GetMarkedObjectListWriteAccess().DeleteMark(nm);
                    if (pRefObj1==NULL)
                        pRefObj1=pObj; // Das oberste sichtbare Objekt
                    if (!bGrouped)
                    {
                        if (pRefObj==NULL)
                            pRefObj=pObj; // Das oberste sichtbare nicht gruppierte Objekt
                    }
                    pSrcLst0=pSrcLst;
                }
            }
            if (pRefObj==NULL)
                pRefObj=pRefObj1;
            if (pGrp!=NULL)
            {
                aNewMark.InsertEntry(SdrMark(pGrp,pPV));
                sal_uIntPtr nAnz=pDstLst->GetObjCount();
                SdrInsertReason aReason(SDRREASON_VIEWCALL,pRefObj);
                pAktLst->InsertObject(pGrp,nInsPos,&aReason);
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pGrp,true)); // Kein Recalc!
                    for (sal_uIntPtr no=0; no<nAnz; no++)
                    {
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoInsertObject(*pDstLst->GetObj(no)));
                    }
                }
            }
        }
        GetMarkedObjectListWriteAccess().Merge(aNewMark);
        MarkListHasChanged();

        if( bUndo )
            EndUndo();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  #   # #   #  #### ####   ###  #   # ####
//  #   # ##  # #     #   # #   # #   # #   #
//  #   # # # # #  ## ####  #   # #   # ####
//  #   # #  ## #   # #   # #   # #   # #
//   ###  #   #  #### #   #  ###   ###  #
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::UnGroupMarked()
{
    SdrMarkList aNewMark;

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(String(), String(), SDRREPFUNC_OBJ_UNGROUP);

    sal_uIntPtr nCount=0;
    XubString aName1;
    XubString aName;
    sal_Bool bNameOk=sal_False;
    for (sal_uIntPtr nm=GetMarkedObjectCount(); nm>0;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pGrp=pM->GetMarkedSdrObj();
        SdrObjList* pSrcLst=pGrp->GetSubList();
        if (pSrcLst!=NULL) {
            nCount++;
            if (nCount==1) {
                pGrp->TakeObjNameSingul(aName);  // Bezeichnung der Gruppe holen
                pGrp->TakeObjNamePlural(aName1); // Bezeichnung der Gruppe holen
                bNameOk=sal_True;
            } else {
                if (nCount==2) aName=aName1; // Pluralname setzen
                if (bNameOk) {
                    XubString aStr;
                    pGrp->TakeObjNamePlural(aStr); // Bezeichnung der Gruppe holen

                    if(!aStr.Equals(aName))
                        bNameOk = sal_False;
                }
            }
            sal_uIntPtr nDstCnt=pGrp->GetOrdNum();
            SdrObjList* pDstLst=pM->GetPageView()->GetObjList();

            // FIRST move contained objects to parent of group, so that
            // the contained objects are NOT migrated to the UNDO-ItemPool
            // when AddUndo(new SdrUndoDelObj(*pGrp)) is called.
            sal_uIntPtr nAnz=pSrcLst->GetObjCount();
            sal_uIntPtr no;

            if( bUndo )
            {
                for (no=nAnz; no>0;)
                {
                    no--;
                    SdrObject* pObj=pSrcLst->GetObj(no);
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject(*pObj));
                }
            }
            for (no=0; no<nAnz; no++)
            {
                SdrObject* pObj=pSrcLst->RemoveObject(0);
                SdrInsertReason aReason(SDRREASON_VIEWCALL,pGrp);
                pDstLst->InsertObject(pObj,nDstCnt,&aReason);
                if( bUndo )
                    AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoInsertObject(*pObj,true));
                nDstCnt++;
                // Kein SortCheck beim einfuegen in die MarkList, denn das
                // wuerde wg. pObj->GetOrdNum() jedesmal ein RecalcOrdNums()
                // provozieren:
                aNewMark.InsertEntry(SdrMark(pObj,pM->GetPageView()),sal_False);
            }

            if( bUndo )
            {
                // Now it is safe to add the delete-UNDO which trigers the
                // MigrateItemPool now only for itself, not for the subobjects.
                // nDstCnt is right, because previous inserts move group
                // object deeper and increase nDstCnt.
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pGrp));
            }
            pDstLst->RemoveObject(nDstCnt);

            if( !bUndo )
                SdrObject::Free(pGrp);

            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
    }
    if (nCount!=0)
    {
        if (!bNameOk)
            aName=ImpGetResStr(STR_ObjNamePluralGRUP); // Oberbegriff Gruppenobjekte verwenden, wenn verschiedene Objekte.
        SetUndoComment(ImpGetResStr(STR_EditUngroup),aName);
    }

    if( bUndo )
        EndUndo();

    if (nCount!=0)
    {
        GetMarkedObjectListWriteAccess().Merge(aNewMark,sal_True); // Durch das obige Einsortieren ist aNewMark genau verkehrtherum
        MarkListHasChanged();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   ###   ###  #   # #   # ##### ####  #####   #####  ###    ####   ###  #  #   #
//  #   # #   # ##  # #   # #     #   #   #       #   #   #   #   # #   # #   # #
//  #     #   # # # # #   # ####  ####    #       #   #   #   ####  #   # #    #
//  #   # #   # #  ##  # #  #     #   #   #       #   #   #   #     #   # #    #
//   ###   ###  #   #   #   ##### #   #   #       #    ###    #      ###  #### #
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrEditView::ImpConvertOneObj(SdrObject* pObj, sal_Bool bPath, sal_Bool bLineToArea)
{
    SdrObject* pNewObj = pObj->ConvertToPolyObj(bPath, bLineToArea);
    if (pNewObj!=NULL)
    {
        SdrObjList* pOL=pObj->GetObjList();
        DBG_ASSERT(pOL!=NULL,"ConvertTo: Obj liefert keine ObjList");
        if (pOL!=NULL)
        {
            const bool bUndo = IsUndoEnabled();
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoReplaceObject(*pObj,*pNewObj));

            pOL->ReplaceObject(pNewObj,pObj->GetOrdNum());

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }
    return pNewObj;
}

void SdrEditView::ImpConvertTo(sal_Bool bPath, sal_Bool bLineToArea)
{
    sal_Bool bMrkChg=sal_False;
    sal_Bool bModChg=sal_False;
    if (AreObjectsMarked()) {
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        sal_uInt16 nDscrID=0;
        if(bLineToArea)
        {
            if(nMarkAnz == 1)
                nDscrID = STR_EditConvToContour;
            else
                nDscrID = STR_EditConvToContours;

            BegUndo(ImpGetResStr(nDscrID), GetDescriptionOfMarkedObjects());
        }
        else
        {
            if (bPath) {
                if (nMarkAnz==1) nDscrID=STR_EditConvToCurve;
                else nDscrID=STR_EditConvToCurves;
                BegUndo(ImpGetResStr(nDscrID),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_CONVERTTOPATH);
            } else {
                if (nMarkAnz==1) nDscrID=STR_EditConvToPoly;
                else nDscrID=STR_EditConvToPolys;
                BegUndo(ImpGetResStr(nDscrID),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_CONVERTTOPOLY);
            }
        }
        for (sal_uIntPtr nm=nMarkAnz; nm>0;) {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrPageView* pPV=pM->GetPageView();
            if (pObj->IsGroupObject() && !pObj->Is3DObj()) {
                SdrObject* pGrp=pObj;
                SdrObjListIter aIter(*pGrp,IM_DEEPNOGROUPS);
                while (aIter.IsMore()) {
                    pObj=aIter.Next();
                    if (ImpConvertOneObj(pObj,bPath,bLineToArea)) bModChg=sal_True;
                }
            } else {
                SdrObject* pNewObj=ImpConvertOneObj(pObj,bPath,bLineToArea);
                if (pNewObj!=NULL) {
                    bModChg=sal_True;
                    bMrkChg=sal_True;
                    GetMarkedObjectListWriteAccess().ReplaceMark(SdrMark(pNewObj,pPV),nm);
                }
            }
        }
        EndUndo();
        if (bMrkChg) AdjustMarkHdl();
        if (bMrkChg) MarkListHasChanged();
    }
}

void SdrEditView::ConvertMarkedToPathObj(sal_Bool bLineToArea)
{
    ImpConvertTo(sal_True, bLineToArea);
}

void SdrEditView::ConvertMarkedToPolyObj(sal_Bool bLineToArea)
{
    ImpConvertTo(sal_False, bLineToArea);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  #   # ##### #####  ###  ##### # #    #####      # #   # ####   ###  ####  #####
//  ## ## #       #   #   # #     # #    #          # ## ## #   # #   # #   #   #
//  # # # ####    #   ##### ###   # #    ####  ###  # # # # ####  #   # ####    #
//  #   # #       #   #   # #     # #    #          # #   # #     #   # #   #   #
//  #   # #####   #   #   # #     # #### #####      # #   # #      ###  #   #   #
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::DoImportMarkedMtf(SvdProgressInfo *pProgrInfo)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(String(), String(), SDRREPFUNC_OBJ_IMPORTMTF);

    SortMarkedObjects();
    SdrMarkList aForTheDescription;
    SdrMarkList aNewMarked;
    sal_uIntPtr nAnz=GetMarkedObjectCount();

    for (sal_uIntPtr nm=nAnz; nm>0;)
    { // Undo Objekte fuer alle neuen Objekte erzeugen
        // zwischen den Metafiles auf Abbruch testen
        if( pProgrInfo != NULL )
        {
            pProgrInfo->SetNextObject();
            if(!pProgrInfo->ReportActions(0))
                break;
        }

        nm--;
        SdrMark*     pM=GetSdrMarkByIndex(nm);
        SdrObject*   pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList*  pOL=pObj->GetObjList();
        sal_uIntPtr        nInsPos=pObj->GetOrdNum()+1;
        SdrGrafObj*  pGraf=PTR_CAST(SdrGrafObj,pObj);
        SdrOle2Obj*  pOle2=PTR_CAST(SdrOle2Obj,pObj);
        sal_uIntPtr        nInsAnz=0;
        if (pGraf!=NULL && pGraf->HasGDIMetaFile())
        {
            ImpSdrGDIMetaFileImport aFilter(*pMod);
            aFilter.SetScaleRect(pGraf->GetSnapRect());
            aFilter.SetLayer(pObj->GetLayer());
            nInsAnz=aFilter.DoImport(pGraf->GetTransformedGraphic().GetGDIMetaFile(),*pOL,nInsPos,pProgrInfo);
        }
        if ( pOle2!=NULL && pOle2->GetGraphic() )
        {
            //const GDIMetaFile* pMtf=pOle2->GetGDIMetaFile();
            ImpSdrGDIMetaFileImport aFilter(*pMod);
            aFilter.SetScaleRect(pOle2->GetLogicRect());
            aFilter.SetLayer(pObj->GetLayer());
            nInsAnz=aFilter.DoImport(pOle2->GetGraphic()->GetGDIMetaFile(),*pOL,nInsPos,pProgrInfo);
        }
        if (nInsAnz!=0)
        {
            sal_uIntPtr nObj=nInsPos;
            for (sal_uIntPtr i=0; i<nInsAnz; i++)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pOL->GetObj(nObj)));

                // Neue MarkList pflegen
                SdrMark aNewMark(pOL->GetObj(nObj), pPV);
                aNewMarked.InsertEntry(aNewMark);

                nObj++;
            }
            aForTheDescription.InsertEntry(*pM);

            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

            // Objekt aus selektion loesen und loeschen
            GetMarkedObjectListWriteAccess().DeleteMark(TryToFindMarkedObject(pObj));
            pOL->RemoveObject(nInsPos-1);

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }

    // MarkObj... fehlt... jetzt nicht mehr (AW)
    if(aNewMarked.GetMarkCount())
    {
        // Neue Selektion bilden
        for(sal_uIntPtr a(0); a < aNewMarked.GetMarkCount(); a++)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(*aNewMarked.GetMark(a));
        }

        SortMarkedObjects();
    }

    if( bUndo )
    {
        SetUndoComment(ImpGetResStr(STR_EditImportMtf),aForTheDescription.GetMarkDescription());
        EndUndo();
    }
}

