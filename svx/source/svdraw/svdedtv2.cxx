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

#include <svx/svdedtv.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdogrp.hxx>   // fuer's Gruppieren
#include <svx/svdopath.hxx>  // fuer CombineObjects
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svditer.hxx>
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
#include <svx/svdoashp.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdview.hxx>
#include <svx/obj3d.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrEditView::GetMaxToTopObj(SdrObject* /*pObj*/) const
{
  return 0;
}

SdrObject* SdrEditView::GetMaxToBtmObj(SdrObject* /*pObj*/) const
{
  return 0;
}

void SdrEditView::ObjOrderChanged(SdrObject* /*pObj*/, sal_uInt32 /*nOldPos*/, sal_uInt32 /*nNewPos*/)
{
}

void SdrEditView::MovMarkedToTop()
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        clearSdrObjectSelection(); // TTTT: check if this works

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditMovToTop), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_MOVTOTOP);
        }

        sal_uInt32 nm(0);
        SdrObjList* pOL0 = 0;
        sal_uInt32 nNewPos(0);

        for(nm = aSelection.size(); nm > 0;)
        {
            nm--;
            SdrObject* pObj = aSelection[nm];
            SdrObjList* pOL = pObj->getParentOfSdrObject();

            if (pOL!=pOL0)
            {
                nNewPos = sal_uInt32(pOL->GetObjCount() - 1);
                pOL0=pOL;
            }

            const sal_uInt32 nNowPos(pObj->GetNavigationPosition());
            sal_uInt32 nCmpPos(nNowPos + 1);
            SdrObject* pMaxObj=GetMaxToTopObj(pObj);

            if(pMaxObj)
            {
                sal_uInt32 nMaxPos(pMaxObj->GetNavigationPosition());

                if(nMaxPos)
                {
                    nMaxPos--;
                }

                if (nNewPos>nMaxPos)
                {
                    nNewPos=nMaxPos; // diesen nicht ueberholen.
                }

                if (nNewPos<nNowPos)
                {
                    nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                }
            }

            bool bEnd(false);
            const basegfx::B2DRange aBoundRange(pObj->getObjectRange(getAsSdrView()));

            while (nCmpPos<nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);

                if(!pCmpObj)
                {
                    DBG_ERROR("MovMarkedToTop(): Vergleichsobjekt nicht gefunden");
                    bEnd = true;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos--;
                    bEnd = true;
                }
                else if(aBoundRange.overlaps(pCmpObj->getObjectRange(getAsSdrView())))
                {
                    nNewPos=nCmpPos;
                    bEnd = true;
                }
                else
                {
                    nCmpPos++;
                }
            }

            if (nNowPos!=nNewPos)
            {
                pOL->SetNavigationPosition(nNowPos, nNewPos);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                }

                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }

            nNewPos--;
        }

        if( bUndo )
        {
            EndUndo();
        }

        setSdrObjectSelection(aSelection); // TTTT: check if this works
    }
}

void SdrEditView::MovMarkedToBtm()
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        clearSdrObjectSelection(); // TTTT: check if this works

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditMovToBtm), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_MOVTOBTM);
        }

        sal_uInt32 nm(0);
        SdrObjList* pOL0 = 0;
        sal_uInt32 nNewPos(0);

        for(nm = 0; nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            SdrObjList* pOL = pObj->getParentOfSdrObject();

            if (pOL!=pOL0)
            {
                nNewPos=0;
                pOL0=pOL;
            }

            const sal_uInt32 nNowPos(pObj->GetNavigationPosition());
            sal_uInt32 nCmpPos(nNowPos);

            if(nCmpPos > 0)
            {
                nCmpPos--;
            }

            SdrObject* pMaxObj=GetMaxToBtmObj(pObj);

            if(pMaxObj)
            {
                sal_uInt32 nMinPos(pMaxObj->GetNavigationPosition() + 1);

                if (nNewPos<nMinPos)
                {
                    nNewPos=nMinPos; // diesen nicht ueberholen.
                }

                if (nNewPos>nNowPos)
                {
                    nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                }
            }

            bool bEnd(false);
            const basegfx::B2DRange aBoundRange(pObj->getObjectRange(getAsSdrView()));
            // nNewPos ist an dieser Stelle noch die maximale Position,
            // an der das Obj hinruecken darf, ohne seinen Vorgaenger
            // (Mehrfachselektion) zu ueberholen.

            while (nCmpPos>nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);

                if(!pCmpObj)
                {
                    DBG_ERROR("MovMarkedToBtm(): Vergleichsobjekt nicht gefunden");
                    bEnd = true;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos++;
                    bEnd = true;
                }
                else if(aBoundRange.overlaps(pCmpObj->getObjectRange(getAsSdrView())))
                {
                    nNewPos=nCmpPos;
                    bEnd = true;
                }
                else
                {
                    nCmpPos--;
                }
            }

            if (nNowPos!=nNewPos)
            {
                pOL->SetNavigationPosition(nNowPos, nNewPos);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                }

                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }

            nNewPos++;
        }

        if(bUndo)
        {
            EndUndo();
        }

        setSdrObjectSelection(aSelection); // TTTT: check if this works
    }
}

void SdrEditView::PutMarkedToTop()
{
    PutMarkedInFrontOfObj(0);
}

void SdrEditView::PutMarkedInFrontOfObj(const SdrObject* pRefObj)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(bUndo)
        {
            BegUndo(ImpGetResStr(STR_EditPutToTop), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_PUTTOTOP);
        }

        if(pRefObj)
        {
            // Damit "Vor das Objekt" auch funktioniert wenn die
            // markierten Objekte bereits vor dem Objekt stehen
            const bool bWasSelected(isSdrObjectSelected(*pRefObj));

            if(bWasSelected)
            {
                removeSdrObjectFromSelection(*pRefObj);
            }

            PutMarkedToBtm();

            if(bWasSelected)
            {
                addSdrObjectToSelection(*pRefObj);
            }
        }

        // selection was changed, re-fetch
        aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
        clearSdrObjectSelection(); // TTTT: check if this works

        sal_uInt32 nm(0);
        SdrObjList* pOL0 = 0;
        sal_uInt32 nNewPos(0);

        for(nm = aSelection.size(); nm > 0;)
        {
            nm--;
            SdrObject* pObj = aSelection[nm];

            if (pObj!=pRefObj)
            {
                SdrObjList* pOL = pObj->getParentOfSdrObject();

                if (pOL!=pOL0)
                {
                    nNewPos = sal_uInt32(pOL->GetObjCount() - 1);
                    pOL0=pOL;
                }

                const sal_uInt32 nNowPos(pObj->GetNavigationPosition());
                SdrObject* pMaxObj=GetMaxToTopObj(pObj);

                if(pMaxObj)
                {
                    sal_uInt32 nMaxOrd(pMaxObj->GetNavigationPosition()); // geht leider nicht anders

                    if (nMaxOrd>0)
                    {
                        nMaxOrd--;
                    }

                    if (nNewPos>nMaxOrd)
                    {
                        nNewPos=nMaxOrd; // nicht ueberholen.
                    }

                    if (nNewPos<nNowPos)
                    {
                        nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                    }
                }

                if(pRefObj)
                {
                    if(pRefObj->getParentOfSdrObject() == pObj->getParentOfSdrObject())
                    {
                        const sal_uInt32 nMaxOrd(pRefObj->GetNavigationPosition()); // geht leider nicht anders

                        if (nNewPos>nMaxOrd)
                        {
                            nNewPos=nMaxOrd; // nicht ueberholen.
                        }

                        if (nNewPos<nNowPos)
                        {
                            nNewPos=nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                        }
                    }
                    else
                    {
                        nNewPos=nNowPos; // andere PageView, also nicht veraendern
                    }
                }

                if (nNowPos!=nNewPos)
                {
                    pOL->SetNavigationPosition(nNowPos, nNewPos);

                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj, nNowPos, nNewPos));
                    }

                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }

                nNewPos--;
            }
        }

        if( bUndo )
        {
            EndUndo();
        }

        setSdrObjectSelection(aSelection); // TTTT: check if this works
    }
}

void SdrEditView::PutMarkedToBtm()
{
    PutMarkedBehindObj(0);
}

void SdrEditView::PutMarkedBehindObj(const SdrObject* pRefObj)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditPutToBtm), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_PUTTOBTM);
        }

        if(pRefObj)
        {
            // Damit "Hinter das Objekt" auch funktioniert wenn die
            // markierten Objekte bereits hinter dem Objekt stehen
            const bool bWasSelected(isSdrObjectSelected(*pRefObj));

            if(bWasSelected)
            {
                removeSdrObjectFromSelection(*pRefObj);
            }

            PutMarkedToTop();

            if(bWasSelected)
            {
                addSdrObjectToSelection(*pRefObj);
            }
        }

        // selection was changed, re-fetch
        aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
        clearSdrObjectSelection(); // TTTT: check if this works

        sal_uInt32 nm(0);
        SdrObjList* pOL0 = 0;
        sal_uInt32 nNewPos(0);

        for(nm = 0; nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];

            if(pObj != pRefObj)
            {
                SdrObjList* pOL = pObj->getParentOfSdrObject();

                if(pOL != pOL0)
                {
                    nNewPos=0;
                    pOL0=pOL;
                }

                const sal_uInt32 nNowPos(pObj->GetNavigationPosition());
                SdrObject* pMinObj=GetMaxToBtmObj(pObj);

                if(pMinObj)
                {
                    const sal_uInt32 nMinOrd(pMinObj->GetNavigationPosition() + 1); // geht leider nicht anders

                    if(nNewPos < nMinOrd)
                    {
                        nNewPos = nMinOrd; // nicht ueberholen.
                    }

                    if(nNewPos > nNowPos)
                    {
                        nNewPos = nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                    }
                }

                if(pRefObj)
                {
                    if(pRefObj->getParentOfSdrObject() == pObj->getParentOfSdrObject())
                    {
                        const sal_uInt32 nMinOrd(pRefObj->GetNavigationPosition()); // geht leider nicht anders

                        if(nNewPos < nMinOrd)
                        {
                            nNewPos = nMinOrd; // nicht ueberholen.
                        }

                        if(nNewPos > nNowPos)
                        {
                            nNewPos = nNowPos; // aber dabei auch nicht in die falsche Richtung schieben
                        }
                    }
                    else
                    {
                        nNewPos=nNowPos; // andere PageView, also nicht veraendern
                    }
                }

                if(nNowPos != nNewPos)
                {
                    pOL->SetNavigationPosition(nNowPos, nNewPos);

                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj, nNowPos, nNewPos));
                    }

                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }

                nNewPos++;
            }
        }

        if(bUndo)
        {
            EndUndo();
        }

        setSdrObjectSelection(aSelection); // TTTT: check if this works
    }
}

void SdrEditView::ReverseOrderOfMarked()
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        clearSdrObjectSelection(); // TTTT: check if this works
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            BegUndo(ImpGetResStr(STR_EditRevOrder), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_REVORDER);
        }

        sal_uInt32 a(0);
        sal_uInt32 b(aSelection.size() - 1);

        while(a < b)
        {
            SdrObject* pObj1 = aSelection[a];
            SdrObject* pObj2 = aSelection[b];

            if(pObj1 && pObj2)
            {
                SdrObjList* pOL = pObj1->getParentOfSdrObject();

                if(pOL && pOL == pObj2->getParentOfSdrObject())
                {
                    const sal_uInt32 nTargetPos1(pObj2->GetNavigationPosition());
                    const sal_uInt32 nTargetPos2(pObj1->GetNavigationPosition());

                    if(bUndo)
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj1, pObj1->GetNavigationPosition(), nTargetPos1));
                    }

                    pOL->SetNavigationPosition(pObj1->GetNavigationPosition(), nTargetPos1);

                    // check if 2nd move is not needed; may be the case when the list
                    // has already correctly changed by 1st operation
                    if(pObj2->GetNavigationPosition() != nTargetPos2)
                    {
                        if(bUndo)
                        {
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj2, pObj2->GetNavigationPosition(), nTargetPos2));
                        }

                        pOL->SetNavigationPosition(pObj2->GetNavigationPosition(), nTargetPos2);
                    }
                }
                else
                {
                    OSL_ENSURE(false, "ReverseOrderOfMarked: objects with different parent in selection (!)");
                }
            }
            else
            {
                OSL_ENSURE(false, "ReverseOrderOfMarked: selection contains NULL-SdrObjects (!)");
            }

            a++;
            b--;
        }

        if(bUndo)
        {
            EndUndo();
        }

        setSdrObjectSelection(aSelection); // TTTT: check if this works
    }
}

void SdrEditView::ImpCheckToTopBtmPossible()
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(1 == aSelection.size())
        {
            // Sonderbehandlung fuer Einzelmarkierung
            SdrObject* pObj = aSelection[0];
            SdrObjList* pOL = pObj->getParentOfSdrObject();
            sal_uInt32 nMax(pOL->GetObjCount());
            sal_uInt32 nMin(0);
            const sal_uInt32 nObjNum(pObj->GetNavigationPosition());
            SdrObject* pRestrict=GetMaxToTopObj(pObj);

            if(pRestrict)
            {
                const sal_uInt32 nRestrict(pRestrict->GetNavigationPosition());

                if(nRestrict < nMax)
                {
                    nMax = nRestrict;
                }
            }

            pRestrict=GetMaxToBtmObj(pObj);

            if(pRestrict)
            {
                const sal_uInt32 nRestrict(pRestrict->GetNavigationPosition());

                if(nRestrict > nMin)
                {
                    nMin = nRestrict;
                }
            }

            mbToTopPossible = nObjNum < sal_uInt32(nMax - 1);
            mbToBtmPossible = nObjNum > nMin;
        }
        else
        {
            // Mehrfachselektion
            sal_uInt32 nm(0);
            SdrObjList* pOL0 = 0;
            sal_Int32 nPos0(-1);

            while(!mbToBtmPossible && nm < aSelection.size())
            {
                // 'nach hinten' checken
                SdrObject* pObj = aSelection[nm];
                SdrObjList* pOL = pObj->getParentOfSdrObject();

                if(pOL != pOL0)
                {
                    nPos0=-1;
                    pOL0=pOL;
                }

                const sal_uInt32 nPos(pObj->GetNavigationPosition());

                mbToBtmPossible = (nPos > sal_uInt32(nPos0 + 1));
                nPos0 = sal_Int32(nPos);
                nm++;
            }

            nm = aSelection.size();
            pOL0 = 0;
            nPos0=0x7FFFFFFF;

            while(!mbToTopPossible && nm > 0)
            {
                // 'nach vorn' checken
                nm--;
                SdrObject* pObj = aSelection[nm];
                SdrObjList* pOL = pObj->getParentOfSdrObject();

                if(pOL != pOL0)
                {
                    nPos0=pOL->GetObjCount();
                    pOL0=pOL;
                }

                const sal_uInt32 nPos(pObj->GetNavigationPosition());

                mbToTopPossible = (nPos + 1 < sal_uInt32(nPos0));
                nPos0=nPos;
            }
        }
    }
}

void SdrEditView::ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const
{
    if(pSource)
    {
        SdrObjList* pOL = pSource->getChildrenOfSdrObject();

        if(pOL && !dynamic_cast< const E3dObject* >(pSource))
        {
            // erstes Nichtgruppenobjekt aus der Gruppe holen
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);

            pSource=aIter.Next();
        }
    }

    if(pSource && pDest)
    {
        SfxItemSet aSet(pDest->GetObjectItemPool(),
            SDRATTR_START,              SDRATTR_NOTPERSIST_FIRST-1,
            SDRATTR_NOTPERSIST_LAST+1,  SDRATTR_END,
            EE_ITEMS_START,             EE_ITEMS_END,
            0, 0); // #52757#, #52762#

        aSet.Put(pSource->GetMergedItemSet());
        pDest->ClearMergedItem();
        pDest->SetMergedItemSet(aSet);
        pDest->SetLayer(pSource->GetLayer());
        pDest->SetStyleSheet(pSource->GetStyleSheet(), true);
    }
}

bool SdrEditView::ImpCanConvertForCombine1(const SdrObject* pObj) const
{
    // #69711 : new condition isLine() to be able to combine simple Lines
    bool bIsLine(false);
    const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);

    if(pPath)
    {
        bIsLine = pPath->isLine();
    }

    SdrObjTransformInfoRec aInfo;

    pObj->TakeObjInfo(aInfo);

    return (aInfo.mbCanConvToPath || aInfo.mbCanConvToPoly || bIsLine);
}

bool SdrEditView::ImpCanConvertForCombine(const SdrObject* pObj) const
{
    SdrObjList* pOL = pObj->getChildrenOfSdrObject();

    if(pOL && !dynamic_cast< const E3dObject* >(pObj))
    {
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();

            // Es muessen alle Member einer Gruppe konvertierbar sein
            if(!ImpCanConvertForCombine1(pObj1))
            {
                return false;
            }
        }
    }
    else
    {
        if(!ImpCanConvertForCombine1(pObj))
        {
            return false;
        }
    }

    return true;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon1(const SdrObject* pObj, bool bCombine) const
{
    basegfx::B2DPolyPolygon aRetval;
    const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);

    if(bCombine && pPath && !pObj->GetOutlinerParaObject())
    {
        aRetval = pPath->getB2DPolyPolygonInObjectCoordinates();
    }
    else
    {
        SdrObject* pConvObj = pObj->ConvertToPolyObj(bCombine, false);

        if(pConvObj)
        {
            SdrObjList* pOL = pConvObj->getChildrenOfSdrObject();

            if(pOL)
            {
                SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj1 = aIter.Next();
                    pPath = dynamic_cast< SdrPathObj* >(pObj1);

                    if(pPath)
                    {
                        aRetval.append(pPath->getB2DPolyPolygonInObjectCoordinates());
                    }
                }
            }
            else
            {
                pPath = dynamic_cast< SdrPathObj* >(pConvObj);

                if(pPath)
                {
                    aRetval = pPath->getB2DPolyPolygonInObjectCoordinates();
                }
            }

            deleteSdrObjectSafeAndClearPointer( pConvObj );
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon(const SdrObject* pObj, bool bCombine) const
{
    SdrObjList* pOL = pObj->getChildrenOfSdrObject();

    if(pOL && !dynamic_cast< const E3dObject* >(pObj))
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

    if(!nPolyCount)
    {
        return basegfx::B2DPolygon();
    }
    else if(1 == nPolyCount)
    {
        return rPolyPolygon.getB2DPolygon(0);
    }
    else
    {
        basegfx::B2DPolygon aRetval(rPolyPolygon.getB2DPolygon(0));

        for(sal_uInt32 a(1); a < nPolyCount; a++)
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
    double                      mfPosition;
    double                      mfWidth;

    // for ::std::sort
    bool operator<(const ImpDistributeEntry& rComp) const
    {
        return (mfPosition < rComp.mfPosition);
    }
};

// moved to stl vector, double and transformations
// TTTT: check changes
typedef ::std::vector< ImpDistributeEntry* > ImpDistributeEntryVector;

void SdrEditView::DistributeMarkedObjects()
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size() > 2)
        {
            SfxItemSet aNewAttr(getSdrModelFromSdrView().GetItemPool());
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

            if(pFact)
            {
                AbstractSvxDistributeDialog *pDlg = pFact->CreateSvxDistributeDialog(0, aNewAttr);
                DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
                const sal_uInt16 nResult(pDlg->Execute());

                if(RET_OK == nResult)
                {
                    clearSdrObjectSelection();
                    const SvxDistributeHorizontal eHor(pDlg->GetDistributeHor());
                    const SvxDistributeVertical eVer(pDlg->GetDistributeVer());
                    sal_uInt32 a(0);
                    double fFullLength(0.0);
                    const bool bUndo(IsUndoEnabled());

                    if( bUndo )
                    {
                        BegUndo();
                    }

                    if(eHor != SvxDistributeHorizontalNone)
                    {
                        // build sorted entry list
                        ImpDistributeEntryVector aEntries;
                        aEntries.reserve(aSelection.size());
                        fFullLength = 0.0;

                        for(a = 0; a < aSelection.size(); a++)
                        {
                            ImpDistributeEntry* pNew = new ImpDistributeEntry;
                            pNew->mpObj = aSelection[a];
                            const basegfx::B2DRange aOldObjSnapRange(sdr::legacy::GetSnapRange(*pNew->mpObj));

                            switch(eHor)
                            {
                                case SvxDistributeHorizontalLeft:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getMinX();
                                    break;
                                }
                                case SvxDistributeHorizontalCenter:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getCenterX();
                                    break;
                                }
                                case SvxDistributeHorizontalDistance:
                                {
                                    pNew->mfWidth = aOldObjSnapRange.getWidth();
                                    fFullLength += pNew->mfWidth;
                                    pNew->mfPosition = aOldObjSnapRange.getCenterX();
                                    break;
                                }
                                case SvxDistributeHorizontalRight:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getMaxX();
                                    break;
                                }
                                default: break;
                            }

                            aEntries.push_back(pNew);
                        }

                        // sort by mfPosition
                        ::std::sort(aEntries.begin(), aEntries.end());

                        if(eHor == SvxDistributeHorizontalDistance)
                        {
                            // calc room in-between
                            const double fWidth(sdr::legacy::GetAllObjBoundRange(getSelectedSdrObjectVectorFromSdrMarkView()).getWidth());
                            const double fStepWidth((fWidth - fFullLength) / (double)(aEntries.size() - 1));
                            double fStepStart(aEntries[0]->mfPosition);

                            fStepStart += fStepWidth + ((aEntries[0]->mfWidth + aEntries[1]->mfWidth) * 0.5);

                            // move entries 1..n-1
                            for(a = 1; a < aEntries.size() - 1; a++)
                            {
                                ImpDistributeEntry* pCurr = aEntries[a];
                                ImpDistributeEntry* pNext = aEntries[a + 1];
                                const double fDelta((fStepStart + 0.5) - pCurr->mfPosition);

                                if(bUndo)
                                {
                                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                                }

                                sdr::legacy::transformSdrObject(
                                    *pCurr->mpObj,
                                    basegfx::tools::createTranslateB2DHomMatrix(fDelta, 0.0));

                                fStepStart += fStepWidth + ((pCurr->mfWidth + pNext->mfWidth) * 0.5);
                            }
                        }
                        else
                        {
                            // calc distances
                            const double fWidth(aEntries[aEntries.size() - 1]->mfPosition - aEntries[0]->mfPosition);
                            const double fStepWidth(fWidth / (double)(aEntries.size() - 1));
                            double fStepStart(aEntries[0]->mfPosition);

                            fStepStart += fStepWidth;

                            // move entries 1..n-1
                            for(a = 1 ; a < aEntries.size() - 1; a++)
                            {
                                ImpDistributeEntry* pCurr = aEntries[a];
                                const double fDelta((fStepStart + 0.5) - pCurr->mfPosition);

                                if( bUndo )
                                {
                                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                                }

                                sdr::legacy::transformSdrObject(
                                    *pCurr->mpObj,
                                    basegfx::tools::createTranslateB2DHomMatrix(fDelta, 0.0));

                                fStepStart += fStepWidth;
                            }
                        }

                        for(a = 0; a < aEntries.size(); a++)
                        {
                            delete aEntries[a];
                        }
                    }

                    if(eVer != SvxDistributeVerticalNone)
                    {
                        // build sorted entry list
                        ImpDistributeEntryVector aEntries;
                        aEntries.reserve(aSelection.size());
                        fFullLength = 0.0;

                        for(a = 0; a < aSelection.size(); a++)
                        {
                            ImpDistributeEntry* pNew = new ImpDistributeEntry;
                            pNew->mpObj = aSelection[a];
                            const basegfx::B2DRange aOldObjSnapRange(sdr::legacy::GetSnapRange(*pNew->mpObj));

                            switch(eVer)
                            {
                                case SvxDistributeVerticalTop:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getMinY();
                                    break;
                                }
                                case SvxDistributeVerticalCenter:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getCenterY();
                                    break;
                                }
                                case SvxDistributeVerticalDistance:
                                {
                                    pNew->mfWidth = aOldObjSnapRange.getHeight();
                                    fFullLength += pNew->mfWidth;
                                    pNew->mfPosition = aOldObjSnapRange.getCenterY();
                                    break;
                                }
                                case SvxDistributeVerticalBottom:
                                {
                                    pNew->mfPosition = aOldObjSnapRange.getMaxY();
                                break;
                                }
                                default: break;
                            }

                            aEntries.push_back(pNew);
                        }

                        // sort by mfPosition
                        ::std::sort(aEntries.begin(), aEntries.end());

                        if(eVer == SvxDistributeVerticalDistance)
                        {
                            // calc room in-between
                            const double fHeight(sdr::legacy::GetAllObjBoundRange(getSelectedSdrObjectVectorFromSdrMarkView()).getHeight());
                            const double fStepWidth((fHeight - fFullLength) / (double)(aEntries.size() - 1));
                            double fStepStart(aEntries[0]->mfPosition);

                            fStepStart += fStepWidth + ((aEntries[0]->mfWidth + aEntries[1]->mfWidth) * 0.5);

                            // move entries 1..n-1
                            for(a = 1; a < aEntries.size() - 1; a++)
                            {
                                ImpDistributeEntry* pCurr = aEntries[a];
                                ImpDistributeEntry* pNext = aEntries[a + 1];
                                const double fDelta((fStepStart + 0.5) - pCurr->mfPosition);

                                if(bUndo)
                                {
                                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                                }

                                sdr::legacy::transformSdrObject(
                                    *pCurr->mpObj,
                                    basegfx::tools::createTranslateB2DHomMatrix(0.0, fDelta));

                                fStepStart += fStepWidth + ((pCurr->mfWidth + pNext->mfWidth) * 0.5);
                            }
                        }
                        else
                        {
                            // calc distances
                            const double fHeight(aEntries[aEntries.size() - 1]->mfPosition - aEntries[0]->mfPosition);
                            const double fStepWidth(fHeight / (double)(aEntries.size() - 1));
                            double fStepStart(aEntries[0]->mfPosition);

                            fStepStart += fStepWidth;

                            // move entries 1..n-1
                            for(a = 1; a < aEntries.size() - 1; a++)
                            {
                                ImpDistributeEntry* pCurr = aEntries[a];
                                const double fDelta((fStepStart + 0.5) - pCurr->mfPosition);

                                if( bUndo )
                                {
                                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                                }

                                sdr::legacy::transformSdrObject(
                                    *pCurr->mpObj,
                                    basegfx::tools::createTranslateB2DHomMatrix(0.0, fDelta));

                                fStepStart += fStepWidth;
                            }
                        }

                        for(a = 0; a < aEntries.size(); a++)
                        {
                            delete aEntries[a];
                        }
                    }

                    // UNDO-Comment and end of UNDO
                    SetUndoComment(ImpGetResStr(STR_DistributeMarkedObjects));

                    if( bUndo )
                    {
                        EndUndo();
                    }

                    setSdrObjectSelection(aSelection); // TTTT: check if this works
                }

                delete(pDlg);
            }
        }
    }
}

void SdrEditView::MergeMarkedObjects(SdrMergeMode eMode)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());

        if( bUndo )
        {
            BegUndo();
        }

        sal_uInt32 nInsPos(0xFFFFFFFF);
        const SdrObject* pAttrObj = 0;
        basegfx::B2DPolyPolygon aMergePolyPolygonA;
        basegfx::B2DPolyPolygon aMergePolyPolygonB;

        SdrObjList* pInsOL = 0;
        bool bFirstObjectComplete(false);

        // make sure selected objects are contour objects
        // since now basegfx::tools::adaptiveSubdivide() is used, it is no longer
        // necessary to use ConvertMarkedToPolyObj which will subdivide curves using the old
        // mechanisms. In a next step the polygon clipper will even be able to clip curves...
        // ConvertMarkedToPolyObj(true);
        ConvertMarkedToPathObj(true);
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        OSL_ENSURE(aSelection.size(), "no more objects selected after preparations (!)");
        SdrObjectVector aRemove;
        clearSdrObjectSelection();

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            SdrObject* pObj = aSelection[a];

            if(ImpCanConvertForCombine(pObj))
            {
                if(!pAttrObj)
                {
                    pAttrObj = pObj;
                }

                nInsPos = pObj->GetNavigationPosition() + 1;
                pInsOL = pObj->getParentOfSdrObject();

                // #i76891# use single iter from SJ here which works on SdrObjects and takes
                // groups into account by itself
                SdrObjListIter aIter(*pObj, IM_DEEPWITHGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pCandidate = aIter.Next();
                    SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >(pCandidate);

                    if(pPathObj)
                    {
                        basegfx::B2DPolyPolygon aTmpPoly(pPathObj->getB2DPolyPolygonInObjectCoordinates());

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
                    bFirstObjectComplete = true;
                }

                // move object to temporary delete list
                aRemove.push_back(pObj);
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
            SdrPathObj* pPath = new SdrPathObj(getSdrModelFromSdrView(), aMergePolyPolygonA);

            ImpCopyAttributes(pAttrObj, pPath);
            pInsOL->InsertObjectToSdrObjList(*pPath, nInsPos);

            if(bUndo)
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pPath));
            }

            MarkObj(*pPath, false );
        }
        else
        {
            setSdrObjectSelection(aSelection);
        }

        // sort as if selection to get comment identical to behaviour before selection change
        sortSdrObjectSelection(aRemove);

        switch(eMode)
        {
            case SDR_MERGE_MERGE:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeMergePoly),
                    getSelectionDescription(aRemove));
                break;
            }
            case SDR_MERGE_SUBSTRACT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeSubstractPoly),
                    getSelectionDescription(aRemove));
                break;
            }
            case SDR_MERGE_INTERSECT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeIntersectPoly),
                    getSelectionDescription(aRemove));
                break;
            }
        }

        deleteSdrObjectsWithUndo(aRemove);

        if( bUndo )
        {
            EndUndo();
        }
    }
}

void SdrEditView::CombineMarkedObjects(bool bNoPolyPoly)
{
    if(areSdrObjectsSelected())
    {
        // #105899# Start of Combine-Undo put to front, else ConvertMarkedToPolyObj would
        // create a 2nd Undo-action and Undo-Comment.
        const bool bUndo(IsUndoEnabled());

        // Undo-String will be set later
        if( bUndo )
        {
            BegUndo(String(), String(), bNoPolyPoly ? SDRREPFUNC_OBJ_COMBINE_ONEPOLY : SDRREPFUNC_OBJ_COMBINE_POLYPOLY);
        }

        // #105899# First, guarantee that all objects are converted to polyobjects,
        // especially for SdrGrafObj with bitmap filling this is necessary to not
        // loose the bitmap filling.

        // #i12392#
        // ConvertMarkedToPolyObj was too strong here, it will loose quality and
        // information when curve objects are combined. This can be replaced by
        // using ConvertMarkedToPathObj without changing the previous fix.

        // #i21250#
        // Instead of simply passing true as LineToArea, use bNoPolyPoly as info
        // if this command is a 'Combine' or a 'Connect' command. On Connect it's true.
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
        // ConvertMarkedToPathObj with a false in any case. Only drawback is that
        // simple polygons will be changed to curves, but with no information loss.
        ConvertMarkedToPathObj(false /* bLineToArea */);

        // continue as before
        basegfx::B2DPolyPolygon aPolyPolygon;
        SdrObjList* pAktOL = 0;
        SdrObjectVector aRemoveMerker;

        sal_uInt32 nInsPos(0xFFFFFFFF);
        SdrObjList* pInsOL = 0;
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const SdrObject* pAttrObj = 0;
        clearSdrObjectSelection();

        for(sal_uInt32 a(aSelection.size()); a > 0;)
        {
            a--;
            SdrObject* pObj = aSelection[a];
            SdrObjList* pThisOL = pObj->getParentOfSdrObject();

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
                basegfx::B2DPolyPolygon aTmpPoly(basegfx::tools::simplifyCurveSegments(ImpGetPolyPolygon(pObj, true)));
                aPolyPolygon.insert(0, aTmpPoly);

                if(!pInsOL)
                {
                    nInsPos = pObj->GetNavigationPosition() + 1L;
                    pInsOL = pObj->getParentOfSdrObject();
                }

                    aRemoveMerker.push_back(pObj);
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
            if(nPolyCount > 1L)
            {
                aPolyPolygon.setClosed(true);
            }
            else
            {
                // auf Polyline Checken
                const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0L));
                const sal_uInt32 nPointCount(aPolygon.count());

                if(nPointCount > 2)
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
                    }
                }
            }

            SdrPathObj* pPath = new SdrPathObj(getSdrModelFromSdrView(), aPolyPolygon);

            // Attribute des untersten Objekts
            ImpCopyAttributes(pAttrObj, pPath);

            // #100408# If LineStyle of pAttrObj is XLINE_NONE force to XLINE_SOLID to make visible.
            const XLineStyle eLineStyle = ((const XLineStyleItem&)pAttrObj->GetMergedItem(XATTR_LINESTYLE)).GetValue();
            const XFillStyle eFillStyle = ((const XFillStyleItem&)pAttrObj->GetMergedItem(XATTR_FILLSTYLE)).GetValue();

            // #110635#
            // Take fill style/closed state of pAttrObj in account when deciding to change the line style
            const SdrPathObj* pSdrPathObj = dynamic_cast< const SdrPathObj* >(pAttrObj);
            const bool bIsClosedPathObj(pSdrPathObj && pSdrPathObj->isClosed());

            if(XLINE_NONE == eLineStyle && (XFILL_NONE == eFillStyle || !bIsClosedPathObj))
            {
                pPath->SetMergedItem(XLineStyleItem(XLINE_SOLID));
            }

            pInsOL->InsertObjectToSdrObjList(*pPath,nInsPos);

            if( bUndo )
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pPath));
            }

            // #111111#
            // Here was a severe error: Without UnmarkAllObj, the new object was marked
            // additionally to the two ones which are deleted below. As sal_Int32 as those are
            // in the UNDO there is no problem, but as soon as they get deleted, the
            // MarkList will contain deleted objects -> GPF.
//          UnmarkAllObj();
            MarkObj(*pPath, false );
        }
        else
        {
            setSdrObjectSelection(aSelection);
        }

        // sort as if selection to get comment identical to behaviour before selection change
        sortSdrObjectSelection(aRemoveMerker);

        if( bUndo )
        {
            SetUndoComment(
                ImpGetResStr(bNoPolyPoly ? STR_EditCombine_OnePoly : STR_EditCombine_PolyPoly),
                getSelectionDescription(aRemoveMerker));
        }

        // die tatsaechlich verwendeten Objekte aus der Liste entfernen
        deleteSdrObjectsWithUndo(aRemoveMerker);

        if( bUndo )
        {
            EndUndo();
        }
    }
}

bool SdrEditView::ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPolygon, bool bMakeLines) const
{
    bool bCan(false);
    const sal_uInt32 nPolygonCount(rPpolyPolygon.count());

    if(nPolygonCount >= 2)
    {
        // #i69172# dismantle makes sense with 2 or more polygons in a polyPolygon
        bCan = true;
    }
    else if(bMakeLines && 1 == nPolygonCount)
    {
        // #i69172# ..or with at least 2 edges (curves or lines)
        const basegfx::B2DPolygon aPolygon(rPpolyPolygon.getB2DPolygon(0L));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount > 2)
        {
            bCan = true;
        }
    }

    return bCan;
}

bool SdrEditView::ImpCanDismantle(const SdrObject* pObj, bool bMakeLines) const
{
    bool bOtherObjs(false);    // true=andere Objekte ausser PathObj's vorhanden
    bool bMin1PolyPoly(false); // true=mind. 1 PolyPolygon mit mehr als ein Polygon vorhanden
    SdrObjList* pOL = pObj->getChildrenOfSdrObject();

    if(pOL)
    {
        // Aha, Gruppenobjekt. Also alle Member ansehen.
        // Alle muessen PathObjs sein !
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore() && !bOtherObjs)
        {
            const SdrObject* pObj1 = aIter.Next();
            const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >( pObj1);

            if(pPath)
            {
                if(ImpCanDismantle(pPath->getB2DPolyPolygonInObjectCoordinates(), bMakeLines))
                {
                    bMin1PolyPoly = true;
                }

                SdrObjTransformInfoRec aInfo;
                pObj1->TakeObjInfo(aInfo);

                if(!aInfo.mbCanConvToPath)
                {
                    // Passiert z.B. im Falle Fontwork (Joe, 28-11-95)
                    bOtherObjs = true;
                }
            }
            else
            {
                bOtherObjs = true;
            }
        }
    }
    else
    {
        const SdrPathObj* pPath = dynamic_cast< const SdrPathObj* >(pObj);
        const SdrObjCustomShape* pCustomShape = dynamic_cast< const SdrObjCustomShape* >(pObj);

        // #i37011#
        if(pPath)
        {
            if(ImpCanDismantle(pPath->getB2DPolyPolygonInObjectCoordinates(), bMakeLines))
            {
                bMin1PolyPoly = true;
            }

            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            // #69711 : new condition isLine() to be able to break simple Lines
            if(!(aInfo.mbCanConvToPath || aInfo.mbCanConvToPoly) && !pPath->isLine())
            {
                // Passiert z.B. im Falle Fontwork (Joe, 28-11-95)
                bOtherObjs = true;
            }
        }
        else if(pCustomShape)
        {
            if(bMakeLines)
            {
                // allow break command
                bMin1PolyPoly = true;
            }
        }
        else
        {
            bOtherObjs = true;
        }
    }
    return bMin1PolyPoly && !bOtherObjs;
}

void SdrEditView::ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uInt32& rPos, bool bMakeLines)
{
    const SdrPathObj* pSrcPath = dynamic_cast< const SdrPathObj* >(pObj);
    const SdrObjCustomShape* pCustomShape = dynamic_cast< const SdrObjCustomShape* >(pObj);
    const bool bUndo(IsUndoEnabled());

    if(pSrcPath)
    {
        // #i74631# redesigned due to XpolyPolygon removal and explicit constructors
        SdrObject* pLast = 0; // fuer die Zuweisung des OutlinerParaObject
        const basegfx::B2DPolyPolygon aPolyPolygon(pSrcPath->getB2DPolyPolygonInObjectCoordinates());
        const sal_uInt32 nPolyCount(aPolyPolygon.count());

        for(sal_uInt32 a(0); a < nPolyCount; a++)
        {
            const basegfx::B2DPolygon& rCandidate(aPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(rCandidate.count());

            if(!bMakeLines || nPointCount < 2)
            {
                SdrPathObj* pPath = new SdrPathObj(getSdrModelFromSdrView(), basegfx::B2DPolyPolygon(rCandidate));
                ImpCopyAttributes(pSrcPath, pPath);
                pLast = pPath;
                rOL.InsertObjectToSdrObjList(*pPath, rPos);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pPath));
                }

                MarkObj(*pPath, false );
                rPos++;
            }
            else
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                for(sal_uInt32 b(0); b < nLoopCount; b++)
                {
                    basegfx::B2DPolygon aNewPolygon;
                    const sal_uInt32 nNextIndex((b + 1) % nPointCount);

                    aNewPolygon.append(rCandidate.getB2DPoint(b));

                    if(rCandidate.areControlPointsUsed())
                    {
                        aNewPolygon.appendBezierSegment(
                            rCandidate.getNextControlPoint(b),
                            rCandidate.getPrevControlPoint(nNextIndex),
                            rCandidate.getB2DPoint(nNextIndex));
                    }
                    else
                    {
                        aNewPolygon.append(rCandidate.getB2DPoint(nNextIndex));
                    }

                    SdrPathObj* pPath = new SdrPathObj(getSdrModelFromSdrView(), basegfx::B2DPolyPolygon(aNewPolygon));
                    ImpCopyAttributes(pSrcPath, pPath);
                    pLast = pPath;
                    rOL.InsertObjectToSdrObjList(*pPath, rPos);

                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pPath));
                    }

                    MarkObj(*pPath, false );
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
                SdrObject* pCandidate = pReplacement->CloneSdrObject();
                DBG_ASSERT(pCandidate, "SdrEditView::ImpDismantleOneObject: Could not clone SdrObject (!)");
                //pCandidate->SetModel(pCustomShape->GetModel());

                if(((SdrOnOffItem&)pCustomShape->GetMergedItem(SDRATTR_SHADOW)).GetValue())
                {
                    if(dynamic_cast< const SdrObjGroup* >(pReplacement))
                    {
                        pCandidate->SetMergedItem(SdrOnOffItem(SDRATTR_SHADOW, true));
                    }
                }

                rOL.InsertObjectToSdrObjList(*pCandidate, rPos);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pCandidate));
                }

                MarkObj(*pCandidate, false );

                if(pCustomShape->HasText() && !pCustomShape->IsTextPath())
                {
                    // #i37011# also create a text object and add at rPos + 1
                    SdrTextObj* pTextObj = (SdrTextObj*)SdrObjFactory::MakeNewObject(
                        getSdrModelFromSdrView(),
                        SdrObjectCreationInfo(OBJ_TEXT, pCustomShape->GetObjInventor()));

                    // Copy text content
                    OutlinerParaObject* pParaObj = pCustomShape->GetOutlinerParaObject();

                    if(pParaObj)
                    {
                        pTextObj->SetOutlinerParaObject(new OutlinerParaObject(*pParaObj));
                    }

                    // copy all attributes
                    SfxItemSet aTargetItemSet(pCustomShape->GetMergedItemSet());

                    // clear fill and line style
                    aTargetItemSet.Put(XLineStyleItem(XLINE_NONE));
                    aTargetItemSet.Put(XFillStyleItem(XFILL_NONE));

                    // create transformation for text object.
                    // TTTT: Need to check text border distances handling
                    const basegfx::B2DRange aRawUnitTextRange(pCustomShape->getRawUnifiedTextRange());

                    // get TopLeft and BottomRight when applying current objects scale and translate,
                    // but leave out rot and shear TTTT need to check mirroring
                    const basegfx::B2DHomMatrix aJustScaleTranslate(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            pCustomShape->getSdrObjectScale(),
                            pCustomShape->getSdrObjectTranslate()));
                    const basegfx::B2DPoint aTopLeft(aJustScaleTranslate * aRawUnitTextRange.getMinimum());
                    const basegfx::B2DPoint aBottomRight(aJustScaleTranslate * aRawUnitTextRange.getMaximum());

                    // aBottomRight may now be top/left of aTopLeft containing the evtl. applied
                    // mirroring. Use it to create scale for the traget transformation. Also use
                    // already created TopLeft as translation
                    const basegfx::B2DHomMatrix aTextTransform(
                        basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aBottomRight - aTopLeft,
                            pCustomShape->getSdrObjectShearX(),
                            pCustomShape->getSdrObjectRotate(),
                            aTopLeft));

                    // set new transformation to text object
                    pTextObj->setSdrObjectTransformation(aTextTransform);

                    // set modified ItemSet at text object
                    pTextObj->SetMergedItemSet(aTargetItemSet);

                    // insert object
                    rOL.InsertObjectToSdrObjList(*pTextObj, rPos + 1);

                    if( bUndo )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pTextObj));
                    }

                    MarkObj(*pTextObj, false );
                }
            }
        }
    }
}

void SdrEditView::DismantleMarkedObjects(bool bMakeLines)
{
    if(areSdrObjectsSelected())
    {
        // Temporaere Marklist
        SdrObjectVector aRemoveMerker;
        const bool bUndo(IsUndoEnabled());

        if( bUndo )
        {
            // Der Comment wird spaeter zusammengebaut
            BegUndo(String(), String(),
                bMakeLines ? SDRREPFUNC_OBJ_DISMANTLE_LINES : SDRREPFUNC_OBJ_DISMANTLE_POLYS);
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        clearSdrObjectSelection();
        sal_uInt32 nm(0);
        SdrObjList* pOL0 = 0;

        for(nm = aSelection.size(); nm > 0;)
        {
            nm--;
            SdrObject* pObj = aSelection[nm];
            SdrObjList* pOL = pObj->getParentOfSdrObject();

            if(pOL != pOL0)
            {
                pOL0 = pOL;
            }

            if(ImpCanDismantle(pObj, bMakeLines))
            {
                aRemoveMerker.push_back(pObj);
                const sal_uInt32 nPos0(pObj->GetNavigationPosition());
                sal_uInt32 nPos(nPos0 + 1);
                SdrObjList* pSubList = pObj->getChildrenOfSdrObject();

                if(pSubList && !dynamic_cast< const E3dObject* >(pObj))
                {
                    SdrObjListIter aIter(*pSubList,IM_DEEPNOGROUPS);

                    while(aIter.IsMore())
                    {
                        const SdrObject* pObj1=aIter.Next();

                        ImpDismantleOneObject(pObj1, *pOL, nPos, bMakeLines);
                    }
                }
                else
                {
                    ImpDismantleOneObject(pObj, *pOL, nPos, bMakeLines);
                }

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }

                pOL->RemoveObjectFromSdrObjList(nPos0);

                if( !bUndo )
                {
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
            }
        }

        if( bUndo )
        {
            // UndoComment aus den tatsaechlich verwendeten Objekten zusammenbauen
            const String aDescription(getSelectionDescription(aRemoveMerker));
            SetUndoComment(ImpGetResStr(bMakeLines ? STR_EditDismantle_Lines : STR_EditDismantle_Polys), aDescription);

            // die tatsaechlich verwendeten Objekten aus der Liste entfernen
            EndUndo();
        }
    }
}

void SdrEditView::GroupMarked(const SdrObject* pUserGrp)
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bUndo(IsUndoEnabled());
        clearSdrObjectSelection();

        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditGroup), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_GROUP);

            for(sal_uInt32 nm(aSelection.size()); nm > 0;)
            {
                // UndoActions fuer alle betroffenen Objekte anlegen
                nm--;
                SdrObject* pObj = aSelection[nm];
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );

                AddUndoActions( vConnectorUndoActions );
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoRemoveObject( *pObj ));
            }
        }

        SdrObjectVector aNewMark;
        SdrPageView* pSdrPageView = GetSdrPageView();

        if(pSdrPageView)
        {
            SdrObjList* pAktLst = pSdrPageView->GetCurrentObjectList();
            SdrObjList* pSrcLst=pAktLst;
            SdrObjList* pSrcLst0=pSrcLst;
            SdrPage& rPage = pSdrPageView->getSdrPageFromSdrPageView();
            SdrObject* pGrp = 0;
            SdrObject* pRefObj = 0; // Referenz fuer InsertReason (-> rumankern im Writer)
            SdrObject* pRefObj1 = 0; // Referenz fuer InsertReason (-> rumankern im Writer)
            SdrObjList* pDstLst = 0;

            // Falls alle markierten Objekte aus Fremden Obj-Listen
            // kommen, kommt das Gruppenobjekt an das Ende der Liste.
            sal_uInt32 nInsPos(pSrcLst->GetObjCount());
            bool bNeedInsPos(true);

            for(sal_uInt32 nm(aSelection.size()); nm > 0;)
            {
                nm--;

                if(!pGrp)
                {
                    if(pUserGrp)
                    {
                        pGrp = pUserGrp->CloneSdrObject();
                    }

                    if(!pGrp)
                    {
                        pGrp = new SdrObjGroup(getSdrModelFromSdrView());
                    }

                    pDstLst = pGrp->getChildrenOfSdrObject();
                    DBG_ASSERT(pDstLst!=0,"Angebliches Gruppenobjekt liefert keine Objektliste");
                }

                SdrObject* pObj = aSelection[nm];
                pSrcLst = pObj->getParentOfSdrObject();

                const bool bForeignList(pSrcLst != pAktLst);
                const bool bGrouped(pSrcLst != &rPage);

                if (!bForeignList && bNeedInsPos)
                {
                    nInsPos = pObj->GetNavigationPosition(); // ua, damit sind alle ObjOrdNum der Page gesetzt
                    nInsPos++;
                    bNeedInsPos = false;
                }

                pSrcLst->RemoveObjectFromSdrObjList(pObj->GetNavigationPosition());

                if (!bForeignList)
                {
                    nInsPos--; // InsertPos korregieren
                }

                pDstLst->InsertObjectToSdrObjList(*pObj, 0);
                removeSdrObjectFromSelection(*pObj);

                if(!pRefObj1)
                {
                    pRefObj1=pObj; // Das oberste sichtbare Objekt
                }

                if (!bGrouped)
                {
                    if(!pRefObj)
                    {
                            pRefObj=pObj; // Das oberste sichtbare nicht gruppierte Objekt
                    }
                }

                pSrcLst0 = pSrcLst;
            }

            if(!pRefObj)
            {
                pRefObj=pRefObj1;
            }

            if(pGrp)
            {
                aNewMark.push_back(pGrp);
                pAktLst->InsertObjectToSdrObjList(*pGrp, nInsPos);

                if( bUndo )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pGrp));
                    const sal_uInt32 nAnz(pDstLst->GetObjCount());

                    for(sal_uInt32 no(0); no < nAnz; no++)
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoInsertObject(*pDstLst->GetObj(no)));
                    }
                }
            }
        }

        if(aNewMark.size())
        {
            setSdrObjectSelection(aNewMark); // TTTT should be the same, check grouping
        }
        else
        {
            setSdrObjectSelection(aSelection); // TTTT: set old selection
        }

        if( bUndo )
        {
            EndUndo();
        }
    }
}

void SdrEditView::UnGroupMarked()
{
    if(areSdrObjectsSelected() && GetSdrPageView())
    {
        SdrObjList* pDstLst = GetSdrPageView()->GetCurrentObjectList();

        if(pDstLst)
        {
            const bool bUndo(IsUndoEnabled());

            if( bUndo )
            {
                BegUndo(String(), String(), SDRREPFUNC_OBJ_UNGROUP);
            }

            sal_uInt32 nCount(0);
            XubString aName1;
            XubString aName;
            bool bNameOk(false);
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
            SdrObjectVector aNewMark;
            clearSdrObjectSelection();

            for(sal_uInt32 nm(aSelection.size()); nm > 0;)
            {
                nm--;
                SdrObject* pGrp = aSelection[nm];
                SdrObjList* pSrcLst = pGrp->getChildrenOfSdrObject();

                if(pSrcLst)
                {
                    nCount++;

                    if(1 == nCount)
                    {
                        pGrp->TakeObjNameSingul(aName);  // Bezeichnung der Gruppe holen
                        pGrp->TakeObjNamePlural(aName1); // Bezeichnung der Gruppe holen
                        bNameOk = true;
                    }
                    else
                    {
                        if(2 == nCount)
                        {
                            aName = aName1; // Pluralname setzen
                        }

                        if(bNameOk)
                        {
                            XubString aStr;
                            pGrp->TakeObjNamePlural(aStr); // Bezeichnung der Gruppe holen

                            if(!aStr.Equals(aName))
                            {
                                bNameOk = false;
                            }
                        }
                    }

                    // FIRST move contained objects to parent of group, so that
                    // the contained objects are NOT migrated to the UNDO-ItemPool
                    // when AddUndo(new SdrUndoDelObj(*pGrp)) is called.
                    const sal_uInt32 nAnz(pSrcLst->GetObjCount());
                    sal_uInt32 nDstCnt(pGrp->GetNavigationPosition());
                    sal_uInt32 no;

                    if( bUndo )
                    {
                        for (no=nAnz; no>0;)
                        {
                            no--;
                            SdrObject* pObj=pSrcLst->GetObj(no);
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoRemoveObject(*pObj));
                        }
                    }

                    for (no=0; no<nAnz; no++)
                    {
                        SdrObject* pObj = pSrcLst->RemoveObjectFromSdrObjList(0);
                        pDstLst->InsertObjectToSdrObjList(*pObj, nDstCnt);

                        if( bUndo )
                        {
                            AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoInsertObject(*pObj));
                        }

                        nDstCnt++;

                        // Kein SortCheck beim einfuegen in die MarkList, denn das
                        // wuerde wg. pObj->GetNavigationPosition() jedesmal ein RecalcOrdNums()
                        // provozieren:
                        aNewMark.push_back(pObj);
                    }

                    if( bUndo )
                    {
                        // nDstCnt is right, because previous inserts move group
                        // object deeper and increase nDstCnt.
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pGrp));
                    }

                    pDstLst->RemoveObjectFromSdrObjList(nDstCnt);
                    removeSdrObjectFromSelection(*pGrp);

                    if(!bUndo)
                    {
                        deleteSdrObjectSafeAndClearPointer(pGrp);
                    }
                }
            }

            if(nCount)
            {
                if (!bNameOk)
                {
                    aName=ImpGetResStr(STR_ObjNamePluralGRUP); // Oberbegriff Gruppenobjekte verwenden, wenn verschiedene Objekte.
                }

                SetUndoComment(ImpGetResStr(STR_EditUngroup),aName);
            }

            if( bUndo )
            {
                EndUndo();
            }

            if(!aNewMark.empty())
            {
                setSdrObjectSelection(aNewMark); // TTTT: check if this is the same
            }
            else
            {
                setSdrObjectSelection(aSelection);
            }
        }
    }
}

SdrObject* SdrEditView::ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea)
{
    SdrObject* pNewObj = pObj->ConvertToPolyObj(bPath, bLineToArea);

    if(pNewObj)
    {
        SdrObjList* pOL = pObj->getParentOfSdrObject();
        DBG_ASSERT(pOL!=0,"ConvertTo: Obj liefert keine ObjList");

        if(pOL)
        {
            const bool bUndo(IsUndoEnabled());

            if( bUndo )
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoReplaceObject(*pObj, *pNewObj));
            }

            pOL->ReplaceObjectInSdrObjList(*pNewObj, pObj->GetNavigationPosition());

            if( !bUndo )
            {
                deleteSdrObjectSafeAndClearPointer(pObj);
            }
        }
    }

    return pNewObj;
}

void SdrEditView::ImpConvertTo(bool bPath, bool bLineToArea)
{
    bool bModChg(false);

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        sal_uInt16 nDscrID(0);
        clearSdrObjectSelection();

        if(bLineToArea)
        {
            if(1 == aSelection.size())
            {
                nDscrID = STR_EditConvToContour;
            }
            else
            {
                nDscrID = STR_EditConvToContours;
            }

            BegUndo(ImpGetResStr(nDscrID), getSelectionDescription(aSelection));
        }
        else
        {
            if(bPath)
            {
                if(1 == aSelection.size())
                {
                    nDscrID = STR_EditConvToCurve;
                }
                else
                {
                    nDscrID = STR_EditConvToCurves;
                }

                BegUndo(ImpGetResStr(nDscrID), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_CONVERTTOPATH);
            }
            else
            {
                if(1 == aSelection.size())
                {
                    nDscrID = STR_EditConvToPoly;
                }
                else
                {
                    nDscrID = STR_EditConvToPolys;
                }

                BegUndo(ImpGetResStr(nDscrID), getSelectionDescription(aSelection), SDRREPFUNC_OBJ_CONVERTTOPOLY);
            }
        }

        SdrObjectVector aNewSelection;

        for(sal_uInt32 nm(aSelection.size()); nm > 0;)
        {
            nm--;
            SdrObject* pObj = aSelection[nm];

            if(pObj->getChildrenOfSdrObject() && !dynamic_cast< const E3dObject* >(pObj))
            {
                SdrObject* pGrp=pObj;
                SdrObjListIter aIter(*pGrp,IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    pObj=aIter.Next();

                    if(ImpConvertOneObj(pObj, bPath, bLineToArea))
                    {
                        bModChg = true;
                    }
                }
            }
            else
            {
                SdrObject* pNewObj=ImpConvertOneObj(pObj,bPath,bLineToArea);

                if(pNewObj)
                {
                    bModChg = true;
                    aNewSelection.push_back(pNewObj);
                }
            }
        }

        EndUndo();

        if(!aNewSelection.empty())
        {
            setSdrObjectSelection(aNewSelection); // TTTT check if it's the same
        }
        else
        {
            setSdrObjectSelection(aSelection);
        }
    }
}

void SdrEditView::ConvertMarkedToPathObj(bool bLineToArea)
{
    ImpConvertTo(true, bLineToArea);
}

void SdrEditView::ConvertMarkedToPolyObj(bool bLineToArea)
{
    ImpConvertTo(false, bLineToArea);
}

void SdrEditView::DoImportMarkedMtf(SvdProgressInfo *pProgrInfo)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        SdrObjectVector aNewMarked;

        if(bUndo)
        {
            BegUndo(
                ImpGetResStr(STR_EditImportMtf),
                getSelectionDescription(aSelection),
                SDRREPFUNC_OBJ_IMPORTMTF);
        }

        clearSdrObjectSelection();

        for(sal_uInt32 nm(aSelection.size()); nm > 0;)
        {
            // Undo Objekte fuer alle neuen Objekte erzeugen
            // zwischen den Metafiles auf Abbruch testen
            if(pProgrInfo)
            {
                pProgrInfo->SetNextObject();

                if(!pProgrInfo->ReportActions(0))
                {
                    break;
                }
            }

            nm--;
            SdrObject* pObj = aSelection[nm];
            SdrObjList* pOL = pObj->getParentOfSdrObject();
            const sal_uInt32 nInsPos(pObj->GetNavigationPosition() + 1);
            SdrGrafObj* pGraf = dynamic_cast< SdrGrafObj* >(pObj);
            SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >(pObj);
            sal_uInt32 nInsAnz(0);
            GDIMetaFile aMetaFile;
            basegfx::B2DHomMatrix aObjectTransform;

            if(pGraf && (pGraf->HasGDIMetaFile() || pGraf->isEmbeddedSvg()))
            {
                if(pGraf->HasGDIMetaFile())
                {
                    aMetaFile = pGraf->GetTransformedGraphic(
                        SDRGRAFOBJ_TRANSFORMATTR_COLOR|SDRGRAFOBJ_TRANSFORMATTR_MIRROR).GetGDIMetaFile();
                }
                else if(pGraf->isEmbeddedSvg())
                {
                    aMetaFile = pGraf->getMetafileFromEmbeddedSvg();
                }

                if(aMetaFile.GetActionCount())
                {
                    aObjectTransform = pObj->getSdrObjectTransformation();
                }
            }

            if(pOle2 && pOle2->GetGraphic())
            {
                aMetaFile = pOle2->GetGraphic()->GetGDIMetaFile();

                if(aMetaFile.GetActionCount())
                {
                    aObjectTransform = pOle2->getSdrObjectTransformation();
                }
            }

            // TTTT: ObjectTransform probably needs to be adapted, e.g. when metafile
            // already contains rotations, it should be removed
            if(aMetaFile.GetActionCount())
            {
                ImpSdrGDIMetaFileImport aFilter(getSdrModelFromSdrView(), pObj->GetLayer(), aObjectTransform);

                nInsAnz = aFilter.DoImport(aMetaFile, *pOL, nInsPos, pProgrInfo);
            }

            if(nInsAnz)
            {
                for(sal_uInt32 i(0), nObj(nInsPos); i < nInsAnz; i++, nObj++)
                {
                    SdrObject* pCandidate = pOL->GetObj(nObj);

                    if(bUndo)
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pCandidate));
                    }

                    // add to new selection
                    aNewMarked.push_back(pCandidate);
                }

                if(bUndo)
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }

                // remove object
                pOL->RemoveObjectFromSdrObjList(nInsPos - 1);

                if(!bUndo)
                {
                    // if no undo, delete object
                    deleteSdrObjectSafeAndClearPointer(pObj);
                }
            }
            else
            {
                // keep unchanged object in new selection
                aNewMarked.push_back(pObj);
            }
        }

        if(!aNewMarked.empty())
        {
            setSdrObjectSelection(aNewMarked); // TTTT check for equality
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
