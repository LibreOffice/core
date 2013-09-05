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


#include <svl/lstner.hxx>

#include <svx/svdundo.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include "svx/svdstr.hrc"   // names taken from the resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/scene3d.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <svx/svdocapt.hxx>
#include <svl/whiter.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/svdviter.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

// iterates over all views and unmarks this SdrObject if it is marked
static void ImplUnmarkObject( SdrObject* pObj )
{
    SdrViewIter aIter( pObj );
    for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
    {
        pView->MarkObj( pObj, pView->GetSdrPageView(), sal_True );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrUndoAction,SfxUndoAction);

SdrUndoAction::~SdrUndoAction() {}

bool SdrUndoAction::CanRepeat(SfxRepeatTarget& rView) const
{
    SdrView* pV=PTR_CAST(SdrView,&rView);
    if (pV!=NULL) return CanSdrRepeat(*pV);
    return false;
}

void SdrUndoAction::Repeat(SfxRepeatTarget& rView)
{
    SdrView* pV=PTR_CAST(SdrView,&rView);
    if (pV!=NULL) SdrRepeat(*pV);
    DBG_ASSERT(pV!=NULL,"Repeat: SfxRepeatTarget that was handed over is not a SdrView");
}

OUString SdrUndoAction::GetRepeatComment(SfxRepeatTarget& rView) const
{
    SdrView* pV=PTR_CAST(SdrView,&rView);
    if (pV!=NULL) return GetSdrRepeatComment(*pV);
    return OUString();
}

bool SdrUndoAction::CanSdrRepeat(SdrView& /*rView*/) const
{
    return sal_False;
}

void SdrUndoAction::SdrRepeat(SdrView& /*rView*/)
{
}

OUString SdrUndoAction::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    return OUString();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoGroup::SdrUndoGroup(SdrModel& rNewMod)
:   SdrUndoAction(rNewMod),
    aBuf(),
    eFunction(SDRREPFUNC_OBJ_NONE)
{}

SdrUndoGroup::~SdrUndoGroup()
{
    Clear();
}

void SdrUndoGroup::Clear()
{
    for (sal_uIntPtr nu=0; nu<GetActionCount(); nu++) {
        SdrUndoAction* pAct=GetAction(nu);
        delete pAct;
    }
    aBuf.clear();
}

void SdrUndoGroup::AddAction(SdrUndoAction* pAct)
{
    aBuf.push_back(pAct);
}

void SdrUndoGroup::Undo()
{
    for (sal_uIntPtr nu=GetActionCount(); nu>0;) {
        nu--;
        SdrUndoAction* pAct=GetAction(nu);
        pAct->Undo();
    }
}

void SdrUndoGroup::Redo()
{
    for (sal_uIntPtr nu=0; nu<GetActionCount(); nu++) {
        SdrUndoAction* pAct=GetAction(nu);
        pAct->Redo();
    }
}

OUString SdrUndoGroup::GetComment() const
{
    return aComment.replaceAll("%1", aObjDescription);
}

bool SdrUndoGroup::CanSdrRepeat(SdrView& rView) const
{
    switch (eFunction)
    {
    case SDRREPFUNC_OBJ_NONE            :  return false;
    case SDRREPFUNC_OBJ_DELETE          :  return rView.AreObjectsMarked();
    case SDRREPFUNC_OBJ_COMBINE_POLYPOLY:  return rView.IsCombinePossible(sal_False);
    case SDRREPFUNC_OBJ_COMBINE_ONEPOLY :  return rView.IsCombinePossible(sal_True);
    case SDRREPFUNC_OBJ_DISMANTLE_POLYS :  return rView.IsDismantlePossible(sal_False);
    case SDRREPFUNC_OBJ_DISMANTLE_LINES :  return rView.IsDismantlePossible(sal_True);
    case SDRREPFUNC_OBJ_CONVERTTOPOLY   :  return rView.IsConvertToPolyObjPossible(sal_False);
    case SDRREPFUNC_OBJ_CONVERTTOPATH   :  return rView.IsConvertToPathObjPossible(sal_False);
    case SDRREPFUNC_OBJ_GROUP           :  return rView.IsGroupPossible();
    case SDRREPFUNC_OBJ_UNGROUP         :  return rView.IsUnGroupPossible();
    case SDRREPFUNC_OBJ_PUTTOTOP        :  return rView.IsToTopPossible();
    case SDRREPFUNC_OBJ_PUTTOBTM        :  return rView.IsToBtmPossible();
    case SDRREPFUNC_OBJ_MOVTOTOP        :  return rView.IsToTopPossible();
    case SDRREPFUNC_OBJ_MOVTOBTM        :  return rView.IsToBtmPossible();
    case SDRREPFUNC_OBJ_REVORDER        :  return rView.IsReverseOrderPossible();
    case SDRREPFUNC_OBJ_IMPORTMTF       :  return rView.IsImportMtfPossible();
    default: break;
    } // switch
    return false;
}

void SdrUndoGroup::SdrRepeat(SdrView& rView)
{
    switch (eFunction)
    {
    case SDRREPFUNC_OBJ_NONE            :  break;
    case SDRREPFUNC_OBJ_DELETE          :  rView.DeleteMarked();                break;
    case SDRREPFUNC_OBJ_COMBINE_POLYPOLY:  rView.CombineMarkedObjects(sal_False);   break;
    case SDRREPFUNC_OBJ_COMBINE_ONEPOLY :  rView.CombineMarkedObjects(sal_True);    break;
    case SDRREPFUNC_OBJ_DISMANTLE_POLYS :  rView.DismantleMarkedObjects(sal_False); break;
    case SDRREPFUNC_OBJ_DISMANTLE_LINES :  rView.DismantleMarkedObjects(sal_True);  break;
    case SDRREPFUNC_OBJ_CONVERTTOPOLY   :  rView.ConvertMarkedToPolyObj(sal_False); break;
    case SDRREPFUNC_OBJ_CONVERTTOPATH   :  rView.ConvertMarkedToPathObj(sal_False); break;
    case SDRREPFUNC_OBJ_GROUP           :  rView.GroupMarked();                 break;
    case SDRREPFUNC_OBJ_UNGROUP         :  rView.UnGroupMarked();               break;
    case SDRREPFUNC_OBJ_PUTTOTOP        :  rView.PutMarkedToTop();              break;
    case SDRREPFUNC_OBJ_PUTTOBTM        :  rView.PutMarkedToBtm();              break;
    case SDRREPFUNC_OBJ_MOVTOTOP        :  rView.MovMarkedToTop();              break;
    case SDRREPFUNC_OBJ_MOVTOBTM        :  rView.MovMarkedToBtm();              break;
    case SDRREPFUNC_OBJ_REVORDER        :  rView.ReverseOrderOfMarked();        break;
    case SDRREPFUNC_OBJ_IMPORTMTF       :  rView.DoImportMarkedMtf();           break;
    default: break;
    } // switch
}

OUString SdrUndoGroup::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    return aComment.replaceAll("%1", ImpGetResStr(STR_ObjNameSingulPlural));
}

SdrUndoObj::SdrUndoObj(SdrObject& rNewObj)
    : SdrUndoAction(*rNewObj.GetModel())
    , pObj(&rNewObj)
{
}

OUString SdrUndoObj::GetDescriptionStringForObject( const SdrObject& _rForObject, sal_uInt16 nStrCacheID, bool bRepeat )
{
    OUString rStr = ImpGetResStr(nStrCacheID);

    sal_Int32 nPos = rStr.indexOf("%1");

    if(nPos != -1)
    {
        if(bRepeat)
        {
            rStr = rStr.replaceAt(nPos, 2, ImpGetResStr(STR_ObjNameSingulPlural));
        }
        else
        {
            OUString aStr(_rForObject.TakeObjNameSingul());
            rStr = rStr.replaceAt(nPos, 2, aStr);
        }
    }

    return rStr;
}

void SdrUndoObj::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, bool bRepeat) const
{
    if ( pObj )
        rStr = GetDescriptionStringForObject( *pObj, nStrCacheID, bRepeat );
}

// common call method for possible change of the page when UNDO/REDO is triggered
void SdrUndoObj::ImpShowPageOfThisObject()
{
    if(pObj && pObj->IsInserted() && pObj->GetPage() && pObj->GetModel())
    {
        SdrHint aHint(HINT_SWITCHTOPAGE);

        aHint.SetObject(pObj);
        aHint.SetPage(pObj->GetPage());

        pObj->GetModel()->Broadcast(aHint);
    }
}

void SdrUndoAttrObj::ensureStyleSheetInStyleSheetPool(SfxStyleSheetBasePool& rStyleSheetPool, SfxStyleSheet& rSheet)
{
    SfxStyleSheetBase* pThere = rStyleSheetPool.Find(rSheet.GetName(), rSheet.GetFamily());

    if(!pThere)
    {
        // re-insert remembered style which was removed in the meantime. To do this
        // without assertion, do it without parent and set parent after insertion
        const OUString aParent(rSheet.GetParent());

        rSheet.SetParent(OUString());
        rStyleSheetPool.Insert(&rSheet);
        rSheet.SetParent(aParent);
    }
}

SdrUndoAttrObj::SdrUndoAttrObj(SdrObject& rNewObj, bool bStyleSheet1, bool bSaveText)
    : SdrUndoObj(rNewObj)
    , pUndoSet(NULL)
    , pRedoSet(NULL)
    , pRepeatSet(NULL)
    , mxUndoStyleSheet()
    , mxRedoStyleSheet()
    , bHaveToTakeRedoSet(sal_True)
    , pTextUndo(NULL)
    , pTextRedo(NULL)
    , pUndoGroup(NULL)
{
    bStyleSheet = bStyleSheet1;

    SdrObjList* pOL = rNewObj.GetSubList();
    bool bIsGroup(pOL!=NULL && pOL->GetObjCount());
    bool bIs3DScene(bIsGroup && pObj->ISA(E3dScene));

    if(bIsGroup)
    {
        // it's a group object!
        pUndoGroup = new SdrUndoGroup(*pObj->GetModel());
        sal_uInt32 nObjAnz(pOL->GetObjCount());

        for(sal_uInt32 nObjNum(0); nObjNum < nObjAnz; nObjNum++)
        {
            pUndoGroup->AddAction(
                new SdrUndoAttrObj(*pOL->GetObj(nObjNum), bStyleSheet1));
        }
    }

    if(!bIsGroup || bIs3DScene)
    {
        pUndoSet = new SfxItemSet(pObj->GetMergedItemSet());

        if(bStyleSheet)
            mxUndoStyleSheet = pObj->GetStyleSheet();

        if(bSaveText)
        {
            pTextUndo = pObj->GetOutlinerParaObject();
            if(pTextUndo)
                pTextUndo = new OutlinerParaObject(*pTextUndo);
        }
    }
}

SdrUndoAttrObj::~SdrUndoAttrObj()
{
    delete pUndoSet;
    delete pRedoSet;
    delete pRepeatSet;
    delete pUndoGroup;
    delete pTextUndo;
    delete pTextRedo;
}

void SdrUndoAttrObj::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObj);
    bool bIs3DScene(pObj && pObj->ISA(E3dScene));

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(!pUndoGroup || bIs3DScene)
    {
        if(bHaveToTakeRedoSet)
        {
            bHaveToTakeRedoSet = sal_False;

            delete pRedoSet;

            pRedoSet = new SfxItemSet(pObj->GetMergedItemSet());

            if(bStyleSheet)
                mxRedoStyleSheet = pObj->GetStyleSheet();

            if(pTextUndo)
            {
                // #i8508#
                pTextRedo = pObj->GetOutlinerParaObject();

                if(pTextRedo)
                    pTextRedo = new OutlinerParaObject(*pTextRedo);
            }
        }

        if(bStyleSheet)
        {
            mxRedoStyleSheet = pObj->GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxUndoStyleSheet.get());

            if(pSheet && pObj->GetModel() && pObj->GetModel()->GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*pObj->GetModel()->GetStyleSheetPool(), *pSheet);
                pObj->SetStyleSheet(pSheet, sal_True);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(*pObj);

        // Since ClearItem sets back everything to normal
        // it also sets fit-to-size text to non-fit-to-size text and
        // switches on autogrowheight (the default). That may lead to
        // loosing the geometry size info for the object when it is
        // laid out again from AdjustTextFrameWidthAndHeight(). This makes
        // rescuing the size of the object necessary.
        const Rectangle aSnapRect = pObj->GetSnapRect();

        if(pUndoSet)
        {
            if(pObj->ISA(SdrCaptionObj))
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*pUndoSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_SET != pUndoSet->GetItemState(nWhich, sal_False))
                    {
                        pObj->ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                pObj->ClearMergedItem();
            }

            pObj->SetMergedItemSet(*pUndoSet);
        }

        // Restore previous size here when it was changed.
        if(aSnapRect != pObj->GetSnapRect())
        {
            pObj->NbcSetSnapRect(aSnapRect);
        }

        pObj->GetProperties().BroadcastItemChange(aItemChange);

        if(pTextUndo)
        {
            pObj->SetOutlinerParaObject(new OutlinerParaObject(*pTextUndo));
        }
    }

    if(pUndoGroup)
    {
        pUndoGroup->Undo();
    }
}

void SdrUndoAttrObj::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObj);
    bool bIs3DScene(pObj && pObj->ISA(E3dScene));

    if(!pUndoGroup || bIs3DScene)
    {
        if(bStyleSheet)
        {
            mxUndoStyleSheet = pObj->GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxRedoStyleSheet.get());

            if(pSheet && pObj->GetModel() && pObj->GetModel()->GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*pObj->GetModel()->GetStyleSheetPool(), *pSheet);
                pObj->SetStyleSheet(pSheet, sal_True);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(*pObj);

        const Rectangle aSnapRect = pObj->GetSnapRect();

        if(pRedoSet)
        {
            if(pObj->ISA(SdrCaptionObj))
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*pRedoSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SFX_ITEM_SET != pRedoSet->GetItemState(nWhich, sal_False))
                    {
                        pObj->ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                pObj->ClearMergedItem();
            }

            pObj->SetMergedItemSet(*pRedoSet);
        }

        // Restore previous size here when it was changed.
        if(aSnapRect != pObj->GetSnapRect())
        {
            pObj->NbcSetSnapRect(aSnapRect);
        }

        pObj->GetProperties().BroadcastItemChange(aItemChange);

        // #i8508#
        if(pTextRedo)
        {
            pObj->SetOutlinerParaObject(new OutlinerParaObject(*pTextRedo));
        }
    }

    if(pUndoGroup)
    {
        pUndoGroup->Redo();
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoAttrObj::GetComment() const
{
    OUString aStr;

    if(bStyleSheet)
    {
        ImpTakeDescriptionStr(STR_EditSetStylesheet, aStr);
    }
    else
    {
        ImpTakeDescriptionStr(STR_EditSetAttributes, aStr);
    }

    return aStr;
}

void SdrUndoAttrObj::SdrRepeat(SdrView& rView)
{
    if(pRepeatSet)
    {
        rView.SetAttrToMarked(*pRepeatSet, sal_False);
    }
}

bool SdrUndoAttrObj::CanSdrRepeat(SdrView& rView) const
{
    return (pRepeatSet!=0L && rView.AreObjectsMarked());
}

OUString SdrUndoAttrObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;

    if(bStyleSheet)
    {
        ImpTakeDescriptionStr(STR_EditSetStylesheet, aStr, sal_True);
    }
    else
    {
        ImpTakeDescriptionStr(STR_EditSetAttributes, aStr, sal_True);
    }

    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoMoveObj::~SdrUndoMoveObj() {}

void SdrUndoMoveObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    pObj->Move(Size(-aDistance.Width(),-aDistance.Height()));
}

void SdrUndoMoveObj::Redo()
{
    pObj->Move(Size(aDistance.Width(),aDistance.Height()));

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoMoveObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_EditMove,aStr);
    return aStr;
}

void SdrUndoMoveObj::SdrRepeat(SdrView& rView)
{
    rView.MoveMarkedObj(aDistance);
}

bool SdrUndoMoveObj::CanSdrRepeat(SdrView& rView) const
{
    return rView.AreObjectsMarked();
}

OUString SdrUndoMoveObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_EditMove,aStr,sal_True);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoGeoObj::SdrUndoGeoObj(SdrObject& rNewObj)
     : SdrUndoObj(rNewObj)
     , pUndoGeo(NULL)
     , pRedoGeo(NULL)
     , pUndoGroup(NULL)
{
    SdrObjList* pOL=rNewObj.GetSubList();
    if (pOL!=NULL && pOL->GetObjCount() && !rNewObj.ISA(E3dScene))
    {
        // this is a group object!
        // If this were 3D scene, we'd only add an Undo for the scene itself
        // (which we do elsewhere).
        pUndoGroup=new SdrUndoGroup(*pObj->GetModel());
        sal_uIntPtr nObjAnz=pOL->GetObjCount();
        for (sal_uIntPtr nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            pUndoGroup->AddAction(new SdrUndoGeoObj(*pOL->GetObj(nObjNum)));
        }
    }
    else
    {
        pUndoGeo=pObj->GetGeoData();
    }
}

SdrUndoGeoObj::~SdrUndoGeoObj()
{
    delete pUndoGeo;
    delete pRedoGeo;
    delete pUndoGroup;
}

void SdrUndoGeoObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(pUndoGroup)
    {
        pUndoGroup->Undo();

        // only repaint, no objectchange
        pObj->ActionChanged();
    }
    else
    {
        delete pRedoGeo;
        pRedoGeo=pObj->GetGeoData();
        pObj->SetGeoData(*pUndoGeo);
    }
}

void SdrUndoGeoObj::Redo()
{
    if(pUndoGroup)
    {
        pUndoGroup->Redo();

        // only repaint, no objectchange
        pObj->ActionChanged();
    }
    else
    {
        delete pUndoGeo;
        pUndoGeo=pObj->GetGeoData();
        pObj->SetGeoData(*pRedoGeo);
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoGeoObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_DragMethObjOwn,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrUndoObjList::ObjListListener : public SfxListener
{
public:
    ObjListListener(SdrUndoObjList& rThat, SdrObject& rObject, SfxBroadcaster& rBroadcaster);
    ~ObjListListener();

private:
    virtual void Notify(SfxBroadcaster& rBroadcaster, const SfxHint& rHint);

private:
    SdrUndoObjList& m_rThat;
    SdrObject& m_rObject;
    SfxBroadcaster* m_pBroadcaster;
};

SdrUndoObjList::ObjListListener::ObjListListener(SdrUndoObjList& rThat, SdrObject& rObject, SfxBroadcaster& rBroadcaster)
    : m_rThat(rThat)
    , m_rObject(rObject)
    , m_pBroadcaster(&rBroadcaster)
{
    StartListening(*m_pBroadcaster);
}

SdrUndoObjList::ObjListListener::~ObjListListener()
{
    if (m_pBroadcaster)
        EndListening(*m_pBroadcaster);
}

void SdrUndoObjList::ObjListListener::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SdrHint* const pSdrHint(dynamic_cast<const SdrHint*>(&rHint));
    if (pSdrHint)
    {
        if (pSdrHint->GetObject() == &m_rObject)
        {
            switch (pSdrHint->GetKind())
            {
            case HINT_OBJCHG :
                if (IsListening(*m_pBroadcaster))
                {
                    const sal_uInt32 nNewOrdNum(m_rObject.GetOrdNum());
                    if (nNewOrdNum != m_rThat.GetOrdNum())
                        m_rThat.SetOrdNum(nNewOrdNum);
                }
                break;
            case HINT_OBJREMOVED :
                SAL_WARN_IF(!IsListening(*m_pBroadcaster), "svx.sdr", "Object is not in any list");
                EndListening(*m_pBroadcaster);
                break;
            case HINT_OBJINSERTED :
                SAL_WARN_IF(IsListening(*m_pBroadcaster), "svx.sdr", "Object is already in a list");
                StartListening(*m_pBroadcaster);
                break;
            default :
                break;
            }
        }
    }
}

SdrUndoObjList::SdrUndoObjList(SdrObject& rNewObj, bool bOrdNumDirect)
    : SdrUndoObj(rNewObj)
    , bOwner(sal_False)
    , pView(NULL)
    , pPageView(NULL)
    , m_pListener(NULL)
{
    pObjList=pObj->GetObjList();
    if (bOrdNumDirect)
    {
        nOrdNum=pObj->GetOrdNumDirect();
    }
    else
    {
        nOrdNum=pObj->GetOrdNum();
    }

    m_pListener = new ObjListListener(*this, *pObj, *pObj->GetModel());
}

SdrUndoObjList::~SdrUndoObjList()
{
    delete m_pListener;

    if (pObj!=NULL && IsOwner())
    {
        // Attribute have to go back to the regular Pool
        SetOwner(sal_False);

        // now delete
        SdrObject::Free( pObj );
    }
}

void SdrUndoObjList::SetOwner(bool bNew)
{
    bOwner = bNew;
}

sal_uInt32 SdrUndoObjList::GetOrdNum() const
{
    return nOrdNum;
}

void SdrUndoObjList::SetOrdNum(sal_uInt32 nOrdNum_)
{
    nOrdNum = nOrdNum_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoRemoveObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(!pObj->IsInserted(),"UndoRemoveObj: pObj has already been inserted.");
    if (!pObj->IsInserted())
    {
        // #i11426#
        // For UNDOs in Calc/Writer it is necessary to adapt the anchor
        // position of the target object.
        Point aOwnerAnchorPos(0, 0);

        if(pObjList &&
           pObjList->GetOwnerObj() &&
           pObjList->GetOwnerObj()->ISA(SdrObjGroup))
        {
            aOwnerAnchorPos = pObjList->GetOwnerObj()->GetAnchorPos();
        }

        E3DModifySceneSnapRectUpdater aUpdater(pObjList->GetOwnerObj());
        SdrInsertReason aReason(SDRREASON_UNDO);
        pObjList->InsertObject(pObj,nOrdNum,&aReason);

        // #i11426#
        if(aOwnerAnchorPos.X() || aOwnerAnchorPos.Y())
        {
            pObj->NbcSetAnchorPos(aOwnerAnchorPos);
        }
    }
}

void SdrUndoRemoveObj::Redo()
{
    DBG_ASSERT(pObj->IsInserted(),"RedoRemoveObj: pObj is not inserted.");
    if (pObj->IsInserted())
    {
        ImplUnmarkObject( pObj );
        E3DModifySceneSnapRectUpdater aUpdater(pObj);
        pObjList->RemoveObject(nOrdNum);
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoInsertObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(pObj->IsInserted(),"UndoInsertObj: pObj is not inserted.");
    if (pObj->IsInserted())
    {
        ImplUnmarkObject( pObj );

#ifdef DBG_UTIL
        SdrObject* pChkObj=
#endif
        pObjList->RemoveObject(nOrdNum);
        DBG_ASSERT(pChkObj==pObj,"UndoInsertObj: RemoveObjNum!=pObj");
    }
}

void SdrUndoInsertObj::Redo()
{
    DBG_ASSERT(!pObj->IsInserted(),"RedoInsertObj: pObj is already inserted");
    if (!pObj->IsInserted())
    {
        // Restore anchor position of an object,
        // which becomes a member of a group, because its cleared in method
        // <InsertObject(..)>. Needed for correct Redo in Writer. (#i45952#)
        Point aAnchorPos( 0, 0 );
        if ( pObjList &&
             pObjList->GetOwnerObj() &&
             pObjList->GetOwnerObj()->ISA(SdrObjGroup) )
        {
            aAnchorPos = pObj->GetAnchorPos();
        }

        SdrInsertReason aReason(SDRREASON_UNDO);
        pObjList->InsertObject(pObj,nOrdNum,&aReason);

        // Arcs lose position when grouped (#i45952#)
        if ( aAnchorPos.X() || aAnchorPos.Y() )
        {
            pObj->NbcSetAnchorPos( aAnchorPos );
        }
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoDelObj::TryToFlushGraphicContent()
{
    SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pObj);

    if(pSdrGrafObj)
    {
        sdr::contact::ViewContactOfGraphic* pVC = dynamic_cast< sdr::contact::ViewContactOfGraphic* >(&pSdrGrafObj->GetViewContact());

        if(pVC)
        {
            pVC->flushViewObjectContacts();
            pVC->flushGraphicObjects();
        }

        pSdrGrafObj->ForceSwapOut();
    }
}

SdrUndoDelObj::SdrUndoDelObj(SdrObject& rNewObj, bool bOrdNumDirect)
:   SdrUndoRemoveObj(rNewObj,bOrdNumDirect)
{
    SetOwner(true);

    // #i122985# if graphic object is deleted (but goes to undo) flush it's graphic content
    // since it is potentially no longer needed
    TryToFlushGraphicContent();
}

void SdrUndoDelObj::Undo()
{
    SdrUndoRemoveObj::Undo();
    DBG_ASSERT(IsOwner(),"UndoDeleteObj: pObj does not belong to UndoAction");
    SetOwner(sal_False);
}

void SdrUndoDelObj::Redo()
{
    SdrUndoRemoveObj::Redo();
    DBG_ASSERT(!IsOwner(),"RedoDeleteObj: pObj already belongs to UndoAction");
    SetOwner(sal_True);

    // #i122985# if graphic object is deleted (but goes to undo) flush it's graphic content
    // since it is potentially no longer needed
    TryToFlushGraphicContent();
}

OUString SdrUndoDelObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_EditDelete,aStr);
    return aStr;
}

void SdrUndoDelObj::SdrRepeat(SdrView& rView)
{
    rView.DeleteMarked();
}

bool SdrUndoDelObj::CanSdrRepeat(SdrView& rView) const
{
    return rView.AreObjectsMarked();
}

OUString SdrUndoDelObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_EditDelete,aStr,sal_True);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewObj::Undo()
{
    SdrUndoInsertObj::Undo();
    DBG_ASSERT(!IsOwner(),"RedoNewObj: pObj already belongs to UndoAction");
    SetOwner(sal_True);
}

void SdrUndoNewObj::Redo()
{
    SdrUndoInsertObj::Redo();
    DBG_ASSERT(IsOwner(),"RedoNewObj: pObj does not belong to UndoAction");
    SetOwner(sal_False);
}

OUString SdrUndoNewObj::GetComment( const SdrObject& _rForObject )
{
    return GetDescriptionStringForObject( _rForObject, STR_UndoInsertObj );
}

OUString SdrUndoNewObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoInsertObj,aStr);
    return aStr;
}

SdrUndoReplaceObj::SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1, bool bOrdNumDirect)
    : SdrUndoObj(rOldObj1)
    , bOldOwner(sal_False)
    , bNewOwner(sal_False)
    , pNewObj(&rNewObj1)
{
    SetOldOwner(sal_True);

    pObjList=pObj->GetObjList();
    if (bOrdNumDirect)
    {
        nOrdNum=pObj->GetOrdNumDirect();
    }
    else
    {
        nOrdNum=pObj->GetOrdNum();
    }
}

SdrUndoReplaceObj::~SdrUndoReplaceObj()
{
    if (pObj!=NULL && IsOldOwner())
    {
        // Attribute have to go back into the Pool
        SetOldOwner(sal_False);

        // now delete
        SdrObject::Free( pObj );
    }
    if (pNewObj!=NULL && IsNewOwner())
    {
        // Attribute have to go back into the Pool
        SetNewOwner(sal_False);

        // now delete
        SdrObject::Free( pNewObj );
    }
}

void SdrUndoReplaceObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if (IsOldOwner() && !IsNewOwner())
    {
        DBG_ASSERT(!pObj->IsInserted(),"SdrUndoReplaceObj::Undo(): Old object is already inserted!");
        DBG_ASSERT(pNewObj->IsInserted(),"SdrUndoReplaceObj::Undo(): New object is not inserted!");
        SetOldOwner(sal_False);
        SetNewOwner(sal_True);

        ImplUnmarkObject( pNewObj );
        pObjList->ReplaceObject(pObj,nOrdNum);
    }
    else
    {
        OSL_FAIL("SdrUndoReplaceObj::Undo(): Wrong IsMine flags. Did you call Undo twice?");
    }
}

void SdrUndoReplaceObj::Redo()
{
    if (!IsOldOwner() && IsNewOwner())
    {
        DBG_ASSERT(!pNewObj->IsInserted(),"SdrUndoReplaceObj::Redo(): New object is already inserted!!");
        DBG_ASSERT(pObj->IsInserted(),"SdrUndoReplaceObj::Redo(): Old object is not inserted!!");
        SetOldOwner(sal_True);
        SetNewOwner(sal_False);

        ImplUnmarkObject( pObj );
        pObjList->ReplaceObject(pNewObj,nOrdNum);

    }
    else
    {
        OSL_FAIL("SdrUndoReplaceObj::Redo(): Wrong IsMine flags. Did you call Redo twice?");
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

void SdrUndoReplaceObj::SetNewOwner(bool bNew)
{
    bNewOwner = bNew;
}

void SdrUndoReplaceObj::SetOldOwner(bool bNew)
{
    bOldOwner = bNew;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OUString SdrUndoCopyObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopyObj,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i11702#

SdrUndoObjectLayerChange::SdrUndoObjectLayerChange(SdrObject& rObj, SdrLayerID aOldLayer, SdrLayerID aNewLayer)
  : SdrUndoObj(rObj)
  , maOldLayer(aOldLayer)
  , maNewLayer(aNewLayer)
{
}

void SdrUndoObjectLayerChange::Undo()
{
    ImpShowPageOfThisObject();
    pObj->SetLayer(maOldLayer);
}

void SdrUndoObjectLayerChange::Redo()
{
    pObj->SetLayer(maNewLayer);
    ImpShowPageOfThisObject();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObjOrdNum::SdrUndoObjOrdNum(SdrObject& rNewObj, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
    : SdrUndoObj(rNewObj)
    , nOldOrdNum(nOldOrdNum1)
    , nNewOrdNum(nNewOrdNum1)
{
}

void SdrUndoObjOrdNum::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    SdrObjList* pOL=pObj->GetObjList();
    if (pOL==NULL)
    {
        OSL_FAIL("UndoObjOrdNum: pObj does not have an ObjList.");
        return;
    }
    pOL->SetObjectOrdNum(nNewOrdNum,nOldOrdNum);
}

void SdrUndoObjOrdNum::Redo()
{
    SdrObjList* pOL=pObj->GetObjList();
    if (pOL==NULL)
    {
        OSL_FAIL("RedoObjOrdNum: pObj does not have an ObjList.");
        return;
    }
    pOL->SetObjectOrdNum(nOldOrdNum,nNewOrdNum);

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoObjOrdNum::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoObjOrdNum,aStr);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoObjSetText::SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText)
    : SdrUndoObj(rNewObj)
    , pOldText(NULL)
    , pNewText(NULL)
    , bNewTextAvailable(sal_False)
    , bEmptyPresObj(sal_False)
    , mnText(nText)
{
    SdrText* pText = static_cast< SdrTextObj*>( &rNewObj )->getText(mnText);
    if( pText && pText->GetOutlinerParaObject() )
        pOldText = new OutlinerParaObject(*pText->GetOutlinerParaObject());

    bEmptyPresObj = rNewObj.IsEmptyPresObj();
}

SdrUndoObjSetText::~SdrUndoObjSetText()
{
    delete pOldText;
    delete pNewText;
}

void SdrUndoObjSetText::AfterSetText()
{
    if (!bNewTextAvailable)
    {
        SdrText* pText = static_cast< SdrTextObj*>( pObj )->getText(mnText);
        if( pText && pText->GetOutlinerParaObject() )
            pNewText = new OutlinerParaObject(*pText->GetOutlinerParaObject());
        bNewTextAvailable=sal_True;
    }
}

void SdrUndoObjSetText::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    // save old text for Redo
    if (!bNewTextAvailable)
        AfterSetText();

    SdrText* pText = static_cast< SdrTextObj*>( pObj )->getText(mnText);
    if (pText)
    {
        // copy text for Undo, because the original now belongs to SetOutlinerParaObject()
        OutlinerParaObject* pText1 = pOldText ? new OutlinerParaObject(*pOldText) : NULL;
        pText->SetOutlinerParaObject(pText1);
        static_cast< SdrTextObj* >( pObj )->NbcSetOutlinerParaObjectForText( pText1, pText );
    }

    pObj->SetEmptyPresObj( bEmptyPresObj );
    pObj->ActionChanged();

    // #i122410# SetOutlinerParaObject at SdrText does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    pObj->BroadcastObjectChange();
}

void SdrUndoObjSetText::Redo()
{
    SdrText* pText = static_cast< SdrTextObj*>( pObj )->getText(mnText);
    if (pText)
    {
        // copy text for Undo, because the original now belongs to SetOutlinerParaObject()
        OutlinerParaObject* pText1 = pNewText ? new OutlinerParaObject(*pNewText) : NULL;
        static_cast< SdrTextObj* >( pObj )->NbcSetOutlinerParaObjectForText( pText1, pText );
    }
    pObj->ActionChanged();

    // #i122410# NbcSetOutlinerParaObjectForText at SdrTextObj does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    pObj->BroadcastObjectChange();

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoObjSetText::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoObjSetText,aStr);
    return aStr;
}

OUString SdrUndoObjSetText::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoObjSetText,aStr);
    return aStr;
}

void SdrUndoObjSetText::SdrRepeat(SdrView& rView)
{
    if (bNewTextAvailable && rView.AreObjectsMarked())
    {
        const SdrMarkList& rML=rView.GetMarkedObjectList();

        const bool bUndo = rView.IsUndoEnabled();
        if( bUndo )
        {
            OUString aStr;
            ImpTakeDescriptionStr(STR_UndoObjSetText,aStr);
            rView.BegUndo(aStr);
        }

        sal_uIntPtr nAnz=rML.GetMarkCount();
        for (sal_uIntPtr nm=0; nm<nAnz; nm++)
        {
            SdrObject* pObj2=rML.GetMark(nm)->GetMarkedSdrObj();
            SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,pObj2);
            if (pTextObj!=NULL)
            {
                if( bUndo )
                    rView.AddUndo(new SdrUndoObjSetText(*pTextObj,0));

                OutlinerParaObject* pText1=pNewText;
                if (pText1!=NULL)
                    pText1 = new OutlinerParaObject(*pText1);
                pTextObj->SetOutlinerParaObject(pText1);
            }
        }

        if( bUndo )
            rView.EndUndo();
    }
}

bool SdrUndoObjSetText::CanSdrRepeat(SdrView& rView) const
{
    bool bOk = false;
    if (bNewTextAvailable && rView.AreObjectsMarked()) {
        bOk=sal_True;
    }
    return bOk;
}

// Undo/Redo for setting object's name (#i73249#)
SdrUndoObjStrAttr::SdrUndoObjStrAttr( SdrObject& rNewObj,
                                      const ObjStrAttrType eObjStrAttr,
                                      const OUString& sOldStr,
                                      const OUString& sNewStr)
    : SdrUndoObj( rNewObj )
    , meObjStrAttr( eObjStrAttr )
    , msOldStr( sOldStr )
    , msNewStr( sNewStr )
{
}

void SdrUndoObjStrAttr::Undo()
{
    ImpShowPageOfThisObject();

    switch ( meObjStrAttr )
    {
    case OBJ_NAME:
        pObj->SetName( msOldStr );
        break;
    case OBJ_TITLE:
        pObj->SetTitle( msOldStr );
        break;
    case OBJ_DESCRIPTION:
        pObj->SetDescription( msOldStr );
        break;
    }
}

void SdrUndoObjStrAttr::Redo()
{
    switch ( meObjStrAttr )
    {
    case OBJ_NAME:
        pObj->SetName( msNewStr );
        break;
    case OBJ_TITLE:
        pObj->SetTitle( msNewStr );
        break;
    case OBJ_DESCRIPTION:
        pObj->SetDescription( msNewStr );
        break;
    }

    ImpShowPageOfThisObject();
}

OUString SdrUndoObjStrAttr::GetComment() const
{
    OUString aStr;
    switch ( meObjStrAttr )
    {
    case OBJ_NAME:
        ImpTakeDescriptionStr( STR_UndoObjName, aStr );
        aStr += " '" + msNewStr + "'";
        break;
    case OBJ_TITLE:
        ImpTakeDescriptionStr( STR_UndoObjTitle, aStr );
        break;
    case OBJ_DESCRIPTION:
        ImpTakeDescriptionStr( STR_UndoObjDescription, aStr );
        break;
    }

    return aStr;
}


SdrUndoLayer::SdrUndoLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    : SdrUndoAction(rNewModel)
    , pLayer(rNewLayerAdmin.GetLayer(nLayerNum))
    , pLayerAdmin(&rNewLayerAdmin)
    , nNum(nLayerNum)
    , bItsMine(sal_False)
{
}

SdrUndoLayer::~SdrUndoLayer()
{
    if (bItsMine)
    {
        delete pLayer;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewLayer::Undo()
{
    DBG_ASSERT(!bItsMine,"SdrUndoNewLayer::Undo(): Layer already belongs to UndoAction.");
    bItsMine=sal_True;
#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    pLayerAdmin->RemoveLayer(nNum);
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoNewLayer::Undo(): Removed layer is != pLayer.");
}

void SdrUndoNewLayer::Redo()
{
    DBG_ASSERT(bItsMine,"SdrUndoNewLayer::Undo(): Layer does not belong to UndoAction.");
    bItsMine=sal_False;
    pLayerAdmin->InsertLayer(pLayer,nNum);
}

OUString SdrUndoNewLayer::GetComment() const
{
    return ImpGetResStr(STR_UndoNewLayer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoDelLayer::Undo()
{
    DBG_ASSERT(bItsMine,"SdrUndoDelLayer::Undo(): Layer does not belong to UndoAction.");
    bItsMine=sal_False;
    pLayerAdmin->InsertLayer(pLayer,nNum);
}

void SdrUndoDelLayer::Redo()
{
    DBG_ASSERT(!bItsMine,"SdrUndoDelLayer::Undo(): Layer already belongs to UndoAction.");
    bItsMine=sal_True;
#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    pLayerAdmin->RemoveLayer(nNum);
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoDelLayer::Redo(): Removed layer is != pLayer.");
}

OUString SdrUndoDelLayer::GetComment() const
{
    return ImpGetResStr(STR_UndoDelLayer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoMoveLayer::Undo()
{
#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    pLayerAdmin->RemoveLayer(nNeuPos);
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoMoveLayer::Undo(): Removed layer is != pLayer.");
    pLayerAdmin->InsertLayer(pLayer,nNum);
}

void SdrUndoMoveLayer::Redo()
{
#ifdef DBG_UTIL
    SdrLayer* pCmpLayer=
#endif
    pLayerAdmin->RemoveLayer(nNum);
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoMoveLayer::Redo(): Removed layer is != pLayer.");
    pLayerAdmin->InsertLayer(pLayer,nNeuPos);
}

OUString SdrUndoMoveLayer::GetComment() const
{
    return ImpGetResStr(STR_UndoMovLayer);
}


SdrUndoPage::SdrUndoPage(SdrPage& rNewPg)
    : SdrUndoAction(*rNewPg.GetModel())
    , mrPage(rNewPg)
{
}

void SdrUndoPage::ImpInsertPage(sal_uInt16 nNum)
{
    DBG_ASSERT(!mrPage.IsInserted(),"SdrUndoPage::ImpInsertPage(): mrPage is already inserted.");
    if (!mrPage.IsInserted())
    {
        if (mrPage.IsMasterPage())
        {
            rMod.InsertMasterPage(&mrPage,nNum);
        }
        else
        {
            rMod.InsertPage(&mrPage,nNum);
        }
    }
}

void SdrUndoPage::ImpRemovePage(sal_uInt16 nNum)
{
    DBG_ASSERT(mrPage.IsInserted(),"SdrUndoPage::ImpRemovePage(): mrPage is not inserted.");
    if (mrPage.IsInserted())
    {
        SdrPage* pChkPg=NULL;
        if (mrPage.IsMasterPage())
        {
            pChkPg=rMod.RemoveMasterPage(nNum);
        }
        else
        {
            pChkPg=rMod.RemovePage(nNum);
        }
        DBG_ASSERT(pChkPg==&mrPage,"SdrUndoPage::ImpRemovePage(): RemovePage!=&mrPage");
        (void)pChkPg;
    }
}

void SdrUndoPage::ImpMovePage(sal_uInt16 nOldNum, sal_uInt16 nNewNum)
{
    DBG_ASSERT(mrPage.IsInserted(),"SdrUndoPage::ImpMovePage(): mrPage is not inserted.");
    if (mrPage.IsInserted())
    {
        if (mrPage.IsMasterPage())
        {
            rMod.MoveMasterPage(nOldNum,nNewNum);
        }
        else
        {
            rMod.MovePage(nOldNum,nNewNum);
        }
    }
}

void SdrUndoPage::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 /*n*/, bool /*bRepeat*/) const
{
    rStr = ImpGetResStr(nStrCacheID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageList::SdrUndoPageList(SdrPage& rNewPg)
    : SdrUndoPage(rNewPg)
    , bItsMine(sal_False)
{
    nPageNum=rNewPg.GetPageNum();
}

SdrUndoPageList::~SdrUndoPageList()
{
    if(bItsMine)
    {
        delete (&mrPage);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoDelPage::SdrUndoDelPage(SdrPage& rNewPg)
    : SdrUndoPageList(rNewPg)
    , pUndoGroup(NULL)
{
    bItsMine = sal_True;

    // now remember the master page relationships
    if(mrPage.IsMasterPage())
    {
        sal_uInt16 nPageAnz(rMod.GetPageCount());

        for(sal_uInt16 nPageNum2(0); nPageNum2 < nPageAnz; nPageNum2++)
        {
            SdrPage* pDrawPage = rMod.GetPage(nPageNum2);

            if(pDrawPage->TRG_HasMasterPage())
            {
                SdrPage& rMasterPage = pDrawPage->TRG_GetMasterPage();

                if(&mrPage == &rMasterPage)
                {
                    if(!pUndoGroup)
                    {
                        pUndoGroup = new SdrUndoGroup(rMod);
                    }

                    pUndoGroup->AddAction(rMod.GetSdrUndoFactory().CreateUndoPageRemoveMasterPage(*pDrawPage));
                }
            }
        }
    }
}

SdrUndoDelPage::~SdrUndoDelPage()
{
    delete pUndoGroup;
}

void SdrUndoDelPage::Undo()
{
    ImpInsertPage(nPageNum);
    if (pUndoGroup!=NULL)
    {
        // recover master page relationships
        pUndoGroup->Undo();
    }
    DBG_ASSERT(bItsMine,"UndoDeletePage: mrPage does not belong to UndoAction.");
    bItsMine=sal_False;
}

void SdrUndoDelPage::Redo()
{
    ImpRemovePage(nPageNum);
    // master page relations are dissolved automatically
    DBG_ASSERT(!bItsMine,"RedoDeletePage: mrPage already belongs to UndoAction.");
    bItsMine=sal_True;
}

OUString SdrUndoDelPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoDelPage,aStr,0,sal_False);
    return aStr;
}

OUString SdrUndoDelPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoDelPage,aStr,0,sal_False);
    return aStr;
}

void SdrUndoDelPage::SdrRepeat(SdrView& /*rView*/)
{
}

bool SdrUndoDelPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoNewPage::Undo()
{
    ImpRemovePage(nPageNum);
    DBG_ASSERT(!bItsMine,"UndoNewPage: mrPage already belongs to UndoAction.");
    bItsMine=sal_True;
}

void SdrUndoNewPage::Redo()
{
    ImpInsertPage(nPageNum);
    DBG_ASSERT(bItsMine,"RedoNewPage: mrPage does not belong to UndoAction.");
    bItsMine=sal_False;
}

OUString SdrUndoNewPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoNewPage,aStr,0,sal_False);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OUString SdrUndoCopyPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopPage,aStr,0,sal_False);
    return aStr;
}

OUString SdrUndoCopyPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopPage,aStr,0,sal_False);
    return aStr;
}

void SdrUndoCopyPage::SdrRepeat(SdrView& /*rView*/)
{

}

bool SdrUndoCopyPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrUndoSetPageNum::Undo()
{
    ImpMovePage(nNewPageNum,nOldPageNum);
}

void SdrUndoSetPageNum::Redo()
{
    ImpMovePage(nOldPageNum,nNewPageNum);
}

OUString SdrUndoSetPageNum::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoMovPage,aStr,0,sal_False);
    return aStr;
}

SdrUndoPageMasterPage::SdrUndoPageMasterPage(SdrPage& rChangedPage)
:   SdrUndoPage(rChangedPage),
    mbOldHadMasterPage(mrPage.TRG_HasMasterPage())
{
    // get current state from page
    if(mbOldHadMasterPage)
    {
        maOldSet = mrPage.TRG_GetMasterPageVisibleLayers();
        maOldMasterPageNumber = mrPage.TRG_GetMasterPage().GetPageNum();
    }
}

SdrUndoPageMasterPage::~SdrUndoPageMasterPage()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageRemoveMasterPage::SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage)
{
}

void SdrUndoPageRemoveMasterPage::Undo()
{
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_SetMasterPage(*mrPage.GetModel()->GetMasterPage(maOldMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageRemoveMasterPage::Redo()
{
    mrPage.TRG_ClearMasterPage();
}

OUString SdrUndoPageRemoveMasterPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoDelPageMasterDscr,aStr,0,sal_False);
    return aStr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrUndoPageChangeMasterPage::SdrUndoPageChangeMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage),
    mbNewHadMasterPage(sal_False)
{
}

void SdrUndoPageChangeMasterPage::Undo()
{
    // remember values from new page
    if(mrPage.TRG_HasMasterPage())
    {
        mbNewHadMasterPage = sal_True;
        maNewSet = mrPage.TRG_GetMasterPageVisibleLayers();
        maNewMasterPageNumber = mrPage.TRG_GetMasterPage().GetPageNum();
    }

    // restore old values
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrPage.GetModel()->GetMasterPage(maOldMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageChangeMasterPage::Redo()
{
    // restore new values
    if(mbNewHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrPage.GetModel()->GetMasterPage(maNewMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maNewSet);
    }
}

OUString SdrUndoPageChangeMasterPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoChgPageMasterDscr,aStr,0,sal_False);
    return aStr;
}

///////////////////////////////////////////////////////////////////////
SdrUndoFactory::~SdrUndoFactory(){}
// shapes
SdrUndoAction* SdrUndoFactory::CreateUndoMoveObject( SdrObject& rObject )
{
    return new SdrUndoMoveObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoMoveObject( SdrObject& rObject, const Size& rDist )
{
    return new SdrUndoMoveObj( rObject, rDist );
}

SdrUndoAction* SdrUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return new SdrUndoGeoObj( rObject );
}

SdrUndoAction* SdrUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return new SdrUndoAttrObj( rObject, bStyleSheet1 ? sal_True : sal_False, bSaveText ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new SdrUndoRemoveObj( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new SdrUndoInsertObj( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new SdrUndoDelObj( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new SdrUndoNewObj( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return new SdrUndoCopyObj( rObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return new SdrUndoObjOrdNum( rObject, nOldOrdNum1, nNewOrdNum1 );
}

SdrUndoAction* SdrUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return new SdrUndoReplaceObj( rOldObject, rNewObject, bOrdNumDirect ? sal_True : sal_False );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return new SdrUndoObjectLayerChange( rObject, aOldLayer, aNewLayer );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return new SdrUndoObjSetText( rNewObj, nText );
}

SdrUndoAction* SdrUndoFactory::CreateUndoObjectStrAttr( SdrObject& rObject,
                                                        SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType,
                                                        const OUString& sOldStr,
                                                        const OUString& sNewStr )
{
    return new SdrUndoObjStrAttr( rObject, eObjStrAttrType, sOldStr, sNewStr );
}


// layer
SdrUndoAction* SdrUndoFactory::CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return new SdrUndoNewLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* SdrUndoFactory::CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return new SdrUndoDelLayer( nLayerNum, rNewLayerAdmin, rNewModel );
}

SdrUndoAction* SdrUndoFactory::CreateUndoMoveLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel, sal_uInt16 nNeuPos1)
{
    return new SdrUndoMoveLayer( nLayerNum, rNewLayerAdmin, rNewModel, nNeuPos1 );
}

// page
SdrUndoAction*  SdrUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return new SdrUndoDelPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return new SdrUndoNewPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return new SdrUndoCopyPage( rPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
{
    return new SdrUndoSetPageNum( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
SdrUndoAction* SdrUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return new SdrUndoPageRemoveMasterPage( rChangedPage );
}

SdrUndoAction* SdrUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return new SdrUndoPageChangeMasterPage(rChangedPage);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
