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

#include <com/sun/star/drawing/FillStyle.hpp>

#include <svx/svdundo.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdogrp.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <svx/svdocapt.hxx>
#include <svl/whiter.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/svdviter.hxx>
#include <svx/svdotable.hxx> // #i124389#
#include <utility>
#include <sfx2/viewsh.hxx>
#include <svx/svdoashp.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <svx/diagram/datamodel.hxx>
#include <svx/diagram/IDiagramHelper.hxx>
#include <tools/debug.hxx>


// iterates over all views and unmarks this SdrObject if it is marked
static void ImplUnmarkObject( SdrObject* pObj )
{
    SdrViewIter::ForAllViews( pObj,
        [&pObj] (SdrView* pView)
        {
            pView->MarkObj( pObj, pView->GetSdrPageView(), true );
        });
}

SdrUndoAction::SdrUndoAction(SdrModel& rNewMod)
    : m_rMod(rNewMod), m_nViewShellId(-1)
{
    if (SfxViewShell* pViewShell = SfxViewShell::Current())
        m_nViewShellId = pViewShell->GetViewShellId();
}

SdrUndoAction::~SdrUndoAction() {}

bool SdrUndoAction::CanRepeat(SfxRepeatTarget& rView) const
{
    SdrView* pV=dynamic_cast<SdrView*>( &rView );
    if (pV!=nullptr) return CanSdrRepeat(*pV);
    return false;
}

void SdrUndoAction::Repeat(SfxRepeatTarget& rView)
{
    SdrView* pV=dynamic_cast<SdrView*>( &rView );
    if (pV!=nullptr) SdrRepeat(*pV);
    DBG_ASSERT(pV!=nullptr,"Repeat: SfxRepeatTarget that was handed over is not a SdrView");
}

OUString SdrUndoAction::GetRepeatComment(SfxRepeatTarget& rView) const
{
    SdrView* pV=dynamic_cast<SdrView*>( &rView );
    if (pV!=nullptr) return GetSdrRepeatComment();
    return OUString();
}

bool SdrUndoAction::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

void SdrUndoAction::SdrRepeat(SdrView& /*rView*/)
{
}

OUString SdrUndoAction::GetSdrRepeatComment() const
{
    return OUString();
}

ViewShellId SdrUndoAction::GetViewShellId() const
{
    return m_nViewShellId;
}

SdrUndoGroup::SdrUndoGroup(SdrModel& rNewMod)
:   SdrUndoAction(rNewMod),
    eFunction(SdrRepeatFunc::NONE)
{}

SdrUndoGroup::~SdrUndoGroup()
{
}

void SdrUndoGroup::AddAction(std::unique_ptr<SdrUndoAction> pAct)
{
    maActions.push_back(std::move(pAct));
}

void SdrUndoGroup::Undo()
{
    for (auto it = maActions.rbegin(); it != maActions.rend(); ++it)
        (*it)->Undo();
}

void SdrUndoGroup::Redo()
{
    for (std::unique_ptr<SdrUndoAction> & pAction : maActions)
        pAction->Redo();
}

OUString SdrUndoGroup::GetComment() const
{
    return aComment.replaceAll("%1", aObjDescription);
}

bool SdrUndoGroup::CanSdrRepeat(SdrView& rView) const
{
    switch (eFunction)
    {
    case SdrRepeatFunc::NONE           :  return false;
    case SdrRepeatFunc::Delete         :  return rView.AreObjectsMarked();
    case SdrRepeatFunc::CombinePolyPoly:  return rView.IsCombinePossible();
    case SdrRepeatFunc::CombineOnePoly :  return rView.IsCombinePossible(true);
    case SdrRepeatFunc::DismantlePolys :  return rView.IsDismantlePossible();
    case SdrRepeatFunc::DismantleLines :  return rView.IsDismantlePossible(true);
    case SdrRepeatFunc::ConvertToPoly  :  return rView.IsConvertToPolyObjPossible();
    case SdrRepeatFunc::ConvertToPath  :  return rView.IsConvertToPathObjPossible();
    case SdrRepeatFunc::Group          :  return rView.IsGroupPossible();
    case SdrRepeatFunc::Ungroup        :  return rView.IsUnGroupPossible();
    case SdrRepeatFunc::PutToTop       :  return rView.IsToTopPossible();
    case SdrRepeatFunc::PutToBottom    :  return rView.IsToBtmPossible();
    case SdrRepeatFunc::MoveToTop      :  return rView.IsToTopPossible();
    case SdrRepeatFunc::MoveToBottom   :  return rView.IsToBtmPossible();
    case SdrRepeatFunc::ReverseOrder   :  return rView.IsReverseOrderPossible();
    case SdrRepeatFunc::ImportMtf      :  return rView.IsImportMtfPossible();
    default: break;
    } // switch
    return false;
}

void SdrUndoGroup::SdrRepeat(SdrView& rView)
{
    switch (eFunction)
    {
    case SdrRepeatFunc::NONE            :  break;
    case SdrRepeatFunc::Delete          :  rView.DeleteMarked();                break;
    case SdrRepeatFunc::CombinePolyPoly :  rView.CombineMarkedObjects(false);   break;
    case SdrRepeatFunc::CombineOnePoly  :  rView.CombineMarkedObjects();        break;
    case SdrRepeatFunc::DismantlePolys  :  rView.DismantleMarkedObjects();      break;
    case SdrRepeatFunc::DismantleLines  :  rView.DismantleMarkedObjects(true);  break;
    case SdrRepeatFunc::ConvertToPoly   :  rView.ConvertMarkedToPolyObj();      break;
    case SdrRepeatFunc::ConvertToPath   :  rView.ConvertMarkedToPathObj(false); break;
    case SdrRepeatFunc::Group           :  rView.GroupMarked();                 break;
    case SdrRepeatFunc::Ungroup         :  rView.UnGroupMarked();               break;
    case SdrRepeatFunc::PutToTop        :  rView.PutMarkedToTop();              break;
    case SdrRepeatFunc::PutToBottom     :  rView.PutMarkedToBtm();              break;
    case SdrRepeatFunc::MoveToTop       :  rView.MovMarkedToTop();              break;
    case SdrRepeatFunc::MoveToBottom    :  rView.MovMarkedToBtm();              break;
    case SdrRepeatFunc::ReverseOrder    :  rView.ReverseOrderOfMarked();        break;
    case SdrRepeatFunc::ImportMtf       :  rView.DoImportMarkedMtf();           break;
    default: break;
    } // switch
}

OUString SdrUndoGroup::GetSdrRepeatComment() const
{
    return aComment.replaceAll("%1", SvxResId(STR_ObjNameSingulPlural));
}

SdrUndoObj::SdrUndoObj(SdrObject& rNewObj)
:   SdrUndoAction(rNewObj.getSdrModelFromSdrObject())
    ,mxObj(&rNewObj)
{
}

SdrUndoObj::~SdrUndoObj() {}

OUString SdrUndoObj::GetDescriptionStringForObject( const SdrObject& _rForObject, TranslateId pStrCacheID, bool bRepeat )
{
    const OUString rStr {SvxResId(pStrCacheID)};

    const sal_Int32 nPos = rStr.indexOf("%1");
    if (nPos < 0)
        return rStr;

    if (bRepeat)
        return rStr.replaceAt(nPos, 2, SvxResId(STR_ObjNameSingulPlural));

    return rStr.replaceAt(nPos, 2, _rForObject.TakeObjNameSingul());
}

OUString SdrUndoObj::ImpGetDescriptionStr(TranslateId pStrCacheID, bool bRepeat) const
{
    if ( mxObj )
        return GetDescriptionStringForObject( *mxObj, pStrCacheID, bRepeat );
    return OUString();
}

// common call method for possible change of the page when UNDO/REDO is triggered
void SdrUndoObj::ImpShowPageOfThisObject()
{
    if(mxObj && mxObj->IsInserted() && mxObj->getSdrPageFromSdrObject())
    {
        SdrHint aHint(SdrHintKind::SwitchToPage, *mxObj, mxObj->getSdrPageFromSdrObject());
        mxObj->getSdrModelFromSdrObject().Broadcast(aHint);
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
    , bHaveToTakeRedoSet(true)
{
    bStyleSheet = bStyleSheet1;

    SdrObjList* pOL = rNewObj.GetSubList();
    bool bIsGroup(pOL!=nullptr && pOL->GetObjCount());
    bool bIs3DScene(bIsGroup && DynCastE3dScene(mxObj.get()));

    if(bIsGroup)
    {
        // it's a group object!
        pUndoGroup.reset(new SdrUndoGroup(mxObj->getSdrModelFromSdrObject()));

        for (const rtl::Reference<SdrObject>& pObj : *pOL)
        {
            pUndoGroup->AddAction(
                std::make_unique<SdrUndoAttrObj>(*pObj, bStyleSheet1));
        }
    }

    if(bIsGroup && !bIs3DScene)
        return;

    moUndoSet.emplace( mxObj->GetMergedItemSet() );

    if(bStyleSheet)
        mxUndoStyleSheet = mxObj->GetStyleSheet();

    if(bSaveText)
    {
        auto p = mxObj->GetOutlinerParaObject();
        if(p)
            pTextUndo = *p;
    }
}

SdrUndoAttrObj::~SdrUndoAttrObj()
{
    moUndoSet.reset();
    moRedoSet.reset();
    pUndoGroup.reset();
    pTextUndo.reset();
    pTextRedo.reset();
}

void SdrUndoAttrObj::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(mxObj.get());
    bool bIs3DScene(DynCastE3dScene(mxObj.get()));

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(!pUndoGroup || bIs3DScene)
    {
        if(bHaveToTakeRedoSet)
        {
            bHaveToTakeRedoSet = false;

            moRedoSet.emplace( mxObj->GetMergedItemSet() );

            if(bStyleSheet)
                mxRedoStyleSheet = mxObj->GetStyleSheet();

            if(pTextUndo)
            {
                // #i8508#
                auto p = mxObj->GetOutlinerParaObject();
                if(p)
                    pTextRedo = *p;
            }
        }

        if(bStyleSheet)
        {
            mxRedoStyleSheet = mxObj->GetStyleSheet();
            SfxStyleSheet* pSheet = mxUndoStyleSheet.get();

            if(pSheet && mxObj->getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*mxObj->getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                mxObj->SetStyleSheet(pSheet, true);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(*mxObj);

        // Since ClearItem sets back everything to normal
        // it also sets fit-to-size text to non-fit-to-size text and
        // switches on autogrowheight (the default). That may lead to
        // losing the geometry size info for the object when it is
        // laid out again from AdjustTextFrameWidthAndHeight(). This makes
        // rescuing the size of the object necessary.
        const tools::Rectangle aSnapRect = mxObj->GetSnapRect();
        // SdrObjCustomShape::NbcSetSnapRect needs logic instead of snap rect
        const tools::Rectangle aLogicRect = mxObj->GetLogicRect();

        if(moUndoSet)
        {
            if(dynamic_cast<const SdrCaptionObj*>( mxObj.get() ) !=  nullptr)
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*moUndoSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SfxItemState::SET != aIter.GetItemState(false))
                    {
                        mxObj->ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                mxObj->ClearMergedItem();
            }

            mxObj->SetMergedItemSet(*moUndoSet);
        }

        // Restore previous size here when it was changed.
        if(aSnapRect != mxObj->GetSnapRect())
        {
            if(dynamic_cast<const SdrObjCustomShape*>(mxObj.get()))
                mxObj->NbcSetSnapRect(aLogicRect);
            else
                mxObj->NbcSetSnapRect(aSnapRect);
        }

        mxObj->GetProperties().BroadcastItemChange(aItemChange);

        if(pTextUndo)
        {
            mxObj->SetOutlinerParaObject(*pTextUndo);
        }
    }

    if(pUndoGroup)
    {
        pUndoGroup->Undo();
    }
}

void SdrUndoAttrObj::Redo()
{
    E3DModifySceneSnapRectUpdater aUpdater(mxObj.get());
    bool bIs3DScene(DynCastE3dScene(mxObj.get()));

    if(!pUndoGroup || bIs3DScene)
    {
        if(bStyleSheet)
        {
            mxUndoStyleSheet = mxObj->GetStyleSheet();
            SfxStyleSheet* pSheet = mxRedoStyleSheet.get();

            if(pSheet && mxObj->getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*mxObj->getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                mxObj->SetStyleSheet(pSheet, true);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(*mxObj);

        const tools::Rectangle aSnapRect = mxObj->GetSnapRect();
        const tools::Rectangle aLogicRect = mxObj->GetLogicRect();

        if(moRedoSet)
        {
            if(dynamic_cast<const SdrCaptionObj*>( mxObj.get() ) !=  nullptr)
            {
                // do a more smooth item deletion here, else the text
                // rect will be reformatted, especially when information regarding
                // vertical text is changed. When clearing only set items it's
                // slower, but safer regarding such information (it's not changed
                // usually)
                SfxWhichIter aIter(*moRedoSet);
                sal_uInt16 nWhich(aIter.FirstWhich());

                while(nWhich)
                {
                    if(SfxItemState::SET != aIter.GetItemState(false))
                    {
                        mxObj->ClearMergedItem(nWhich);
                    }

                    nWhich = aIter.NextWhich();
                }
            }
            else
            {
                mxObj->ClearMergedItem();
            }

            mxObj->SetMergedItemSet(*moRedoSet);
        }

        // Restore previous size here when it was changed.
        if(aSnapRect != mxObj->GetSnapRect())
        {
            if(dynamic_cast<const SdrObjCustomShape*>(mxObj.get()))
                mxObj->NbcSetSnapRect(aLogicRect);
            else
                mxObj->NbcSetSnapRect(aSnapRect);
        }

        mxObj->GetProperties().BroadcastItemChange(aItemChange);

        // #i8508#
        if(pTextRedo)
        {
            mxObj->SetOutlinerParaObject(*pTextRedo);
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
    if(bStyleSheet)
    {
        return ImpGetDescriptionStr(STR_EditSetStylesheet);
    }
    else
    {
        return ImpGetDescriptionStr(STR_EditSetAttributes);
    }
}

OUString SdrUndoAttrObj::GetSdrRepeatComment() const
{
    if(bStyleSheet)
    {
        return ImpGetDescriptionStr(STR_EditSetStylesheet, true);
    }
    else
    {
        return ImpGetDescriptionStr(STR_EditSetAttributes, true);
    }
}


SdrUndoMoveObj::~SdrUndoMoveObj() {}

void SdrUndoMoveObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    mxObj->Move(Size(-aDistance.Width(),-aDistance.Height()));
}

void SdrUndoMoveObj::Redo()
{
    mxObj->Move(Size(aDistance.Width(),aDistance.Height()));

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoMoveObj::GetComment() const
{
    return ImpGetDescriptionStr(STR_EditMove);
}

void SdrUndoMoveObj::SdrRepeat(SdrView& rView)
{
    rView.MoveMarkedObj(aDistance);
}

bool SdrUndoMoveObj::CanSdrRepeat(SdrView& rView) const
{
    return rView.AreObjectsMarked();
}

OUString SdrUndoMoveObj::GetSdrRepeatComment() const
{
    return ImpGetDescriptionStr(STR_EditMove,true);
}


SdrUndoGeoObj::SdrUndoGeoObj(SdrObject& rNewObj)
     : SdrUndoObj(rNewObj)
     , mbSkipChangeLayout(false)
{
    SdrObjList* pOL=rNewObj.GetSubList();
    if (pOL!=nullptr && pOL->GetObjCount() && !DynCastE3dScene(&rNewObj))
    {
        // this is a group object!
        // If this were 3D scene, we'd only add an Undo for the scene itself
        // (which we do elsewhere).
        pUndoGroup.reset(new SdrUndoGroup(mxObj->getSdrModelFromSdrObject()));
        for (const rtl::Reference<SdrObject>& pObj : *pOL)
            pUndoGroup->AddAction(std::make_unique<SdrUndoGeoObj>(*pObj));
    }
    else
    {
        pUndoGeo = mxObj->GetGeoData();
    }
}

SdrUndoGeoObj::~SdrUndoGeoObj()
{
    pUndoGeo.reset();
    pRedoGeo.reset();
    pUndoGroup.reset();
}

void SdrUndoGeoObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(pUndoGroup)
    {
        pUndoGroup->Undo();

        // only repaint, no objectchange
        mxObj->ActionChanged();
    }
    else
    {
        pRedoGeo = mxObj->GetGeoData();

        auto pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(mxObj.get());
        if (pTableObj && mbSkipChangeLayout)
            pTableObj->SetSkipChangeLayout(true);
        mxObj->SetGeoData(*pUndoGeo);
        if (pTableObj && mbSkipChangeLayout)
            pTableObj->SetSkipChangeLayout(false);
    }
}

void SdrUndoGeoObj::Redo()
{
    if(pUndoGroup)
    {
        pUndoGroup->Redo();

        // only repaint, no objectchange
        mxObj->ActionChanged();
    }
    else
    {
        pUndoGeo = mxObj->GetGeoData();
        mxObj->SetGeoData(*pRedoGeo);
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoGeoObj::GetComment() const
{
    return ImpGetDescriptionStr(STR_DragMethObjOwn);
}

SdrUndoDiagramModelData::SdrUndoDiagramModelData(SdrObject& rNewObj, svx::diagram::DiagramDataStatePtr& rStartState)
: SdrUndoObj(rNewObj)
, m_aStartState(rStartState)
, m_aEndState()
{
    if(rNewObj.isDiagram())
        m_aEndState = rNewObj.getDiagramHelper()->extractDiagramDataState();
}

SdrUndoDiagramModelData::~SdrUndoDiagramModelData()
{
}

void SdrUndoDiagramModelData::implUndoRedo(bool bUndo)
{
    if(!mxObj)
        return;

    if(!mxObj->isDiagram())
        return;

    mxObj->getDiagramHelper()->applyDiagramDataState(
        bUndo ? m_aStartState : m_aEndState);
    mxObj->getDiagramHelper()->reLayout(*static_cast<SdrObjGroup*>(mxObj.get()));
}

void SdrUndoDiagramModelData::Undo()
{
    implUndoRedo(true);
}

void SdrUndoDiagramModelData::Redo()
{
    implUndoRedo(false);
}

OUString SdrUndoDiagramModelData::GetComment() const
{
    return ImpGetDescriptionStr(STR_DiagramModelDataChange);
}

SdrUndoObjList::SdrUndoObjList(SdrObject& rNewObj, bool bOrdNumDirect)
    : SdrUndoObj(rNewObj)
{
    pObjList=mxObj->getParentSdrObjListFromSdrObject();
    if (bOrdNumDirect)
    {
        nOrdNum=mxObj->GetOrdNumDirect();
    }
    else
    {
        nOrdNum=mxObj->GetOrdNum();
    }
}

SdrUndoObjList::~SdrUndoObjList()
{
}

void SdrUndoRemoveObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(!mxObj->IsInserted(),"UndoRemoveObj: mxObj has already been inserted.");
    if (mxObj->IsInserted())
        return;

    // #i11426#
    // For UNDOs in Calc/Writer it is necessary to adapt the anchor
    // position of the target object.
    Point aOwnerAnchorPos(0, 0);

    if (dynamic_cast< const SdrObjGroup* >(pObjList->getSdrObjectFromSdrObjList()) != nullptr)
    {
        aOwnerAnchorPos = pObjList->getSdrObjectFromSdrObjList()->GetAnchorPos();
    }

    E3DModifySceneSnapRectUpdater aUpdater(pObjList->getSdrObjectFromSdrObjList());
    pObjList->InsertObject(mxObj.get(), nOrdNum);

    // #i11426#
    if(aOwnerAnchorPos.X() || aOwnerAnchorPos.Y())
    {
        mxObj->NbcSetAnchorPos(aOwnerAnchorPos);
    }
}

void SdrUndoRemoveObj::Redo()
{
    DBG_ASSERT(mxObj->IsInserted(),"RedoRemoveObj: mxObj is not inserted.");
    if (mxObj->IsInserted())
    {
        ImplUnmarkObject( mxObj.get() );
        E3DModifySceneSnapRectUpdater aUpdater(mxObj.get());
        pObjList->RemoveObject(mxObj->GetOrdNum());
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

SdrUndoRemoveObj::~SdrUndoRemoveObj()
{
}


void SdrUndoInsertObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(mxObj->IsInserted(),"UndoInsertObj: mxObj is not inserted.");
    if (mxObj->IsInserted())
    {
        ImplUnmarkObject( mxObj.get() );

        rtl::Reference<SdrObject> pChkObj= pObjList->RemoveObject(mxObj->GetOrdNum());
        DBG_ASSERT(pChkObj.get()==mxObj.get(),"UndoInsertObj: RemoveObjNum!=mxObj");
    }
}

void SdrUndoInsertObj::Redo()
{
    DBG_ASSERT(!mxObj->IsInserted(),"RedoInsertObj: mxObj is already inserted");
    if (!mxObj->IsInserted())
    {
        // Restore anchor position of an object,
        // which becomes a member of a group, because its cleared in method
        // <InsertObject(..)>. Needed for correct Redo in Writer. (#i45952#)
        Point aAnchorPos( 0, 0 );

        if (dynamic_cast<const SdrObjGroup*>(pObjList->getSdrObjectFromSdrObjList()) != nullptr)
        {
            aAnchorPos = mxObj->GetAnchorPos();
        }

        pObjList->InsertObject(mxObj.get(), nOrdNum);

        // Arcs lose position when grouped (#i45952#)
        if ( aAnchorPos.X() || aAnchorPos.Y() )
        {
            mxObj->NbcSetAnchorPos( aAnchorPos );
        }
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

SdrUndoDelObj::SdrUndoDelObj(SdrObject& rNewObj, bool bOrdNumDirect)
:   SdrUndoRemoveObj(rNewObj,bOrdNumDirect)
{
}

void SdrUndoDelObj::Undo()
{
    SdrUndoRemoveObj::Undo();
}

void SdrUndoDelObj::Redo()
{
    SdrUndoRemoveObj::Redo();
}

OUString SdrUndoDelObj::GetComment() const
{
    return ImpGetDescriptionStr(STR_EditDelete);
}

void SdrUndoDelObj::SdrRepeat(SdrView& rView)
{
    rView.DeleteMarked();
}

bool SdrUndoDelObj::CanSdrRepeat(SdrView& rView) const
{
    return rView.AreObjectsMarked();
}

OUString SdrUndoDelObj::GetSdrRepeatComment() const
{
    return ImpGetDescriptionStr(STR_EditDelete,true);
}


void SdrUndoNewObj::Undo()
{
    SdrUndoInsertObj::Undo();
}

void SdrUndoNewObj::Redo()
{
    SdrUndoInsertObj::Redo();
}

OUString SdrUndoNewObj::GetComment( const SdrObject& _rForObject )
{
    return GetDescriptionStringForObject( _rForObject, STR_UndoInsertObj );
}

OUString SdrUndoNewObj::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoInsertObj);
}

SdrUndoReplaceObj::SdrUndoReplaceObj(SdrObject& rOldObj1, SdrObject& rNewObj1)
    : SdrUndoObj(rOldObj1)
    , mxNewObj(&rNewObj1)
{
    pObjList=mxObj->getParentSdrObjListFromSdrObject();
}

SdrUndoReplaceObj::~SdrUndoReplaceObj()
{
}

void SdrUndoReplaceObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(!mxObj->IsInserted(),"SdrUndoReplaceObj::Undo(): Old object is already inserted!");
    DBG_ASSERT(mxNewObj->IsInserted(),"SdrUndoReplaceObj::Undo(): New object is not inserted!");

    ImplUnmarkObject( mxNewObj.get() );
    pObjList->ReplaceObject(mxObj.get(), mxNewObj->GetOrdNum());
}

void SdrUndoReplaceObj::Redo()
{
    ImplUnmarkObject( mxObj.get() );
    pObjList->ReplaceObject(mxNewObj.get(), mxObj->GetOrdNum());

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}


OUString SdrUndoCopyObj::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoCopyObj);
}


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
    mxObj->SetLayer(maOldLayer);
}

void SdrUndoObjectLayerChange::Redo()
{
    mxObj->SetLayer(maNewLayer);
    ImpShowPageOfThisObject();
}


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

    SdrObjList* pOL=mxObj->getParentSdrObjListFromSdrObject();
    if (pOL==nullptr)
    {
        OSL_FAIL("UndoObjOrdNum: mxObj does not have an ObjList.");
        return;
    }
    pOL->SetObjectOrdNum(nNewOrdNum,nOldOrdNum);
}

void SdrUndoObjOrdNum::Redo()
{
    SdrObjList* pOL=mxObj->getParentSdrObjListFromSdrObject();
    if (pOL==nullptr)
    {
        OSL_FAIL("RedoObjOrdNum: mxObj does not have an ObjList.");
        return;
    }
    pOL->SetObjectOrdNum(nOldOrdNum,nNewOrdNum);

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoObjOrdNum::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoObjOrdNum);
}

SdrUndoSort::SdrUndoSort(const SdrPage & rPage,
        ::std::vector<sal_Int32> const& rSortOrder)
    : SdrUndoAction(rPage.getSdrModelFromSdrPage())
    , m_OldSortOrder(rSortOrder.size())
    , m_NewSortOrder(rSortOrder)
    , m_nPage(rPage.GetPageNum())
{
    // invert order
    for (size_t i = 0; i < rSortOrder.size(); ++i)
    {
        m_OldSortOrder[rSortOrder[i]] = i;
    }
}

void SdrUndoSort::Do(::std::vector<sal_Int32> & rSortOrder)
{
    SdrPage & rPage(*m_rMod.GetPage(m_nPage));
    if (rPage.GetObjCount() != rSortOrder.size())
    {
        // can probably happen with sw's cursed SdrVirtObj mess - no good solution for that
        SAL_WARN("svx", "SdrUndoSort size mismatch");
        return;
    }

    // hopefully this can't throw
    rPage.sort(rSortOrder);
}

void SdrUndoSort::Undo()
{
    Do(m_OldSortOrder);
}

void SdrUndoSort::Redo()
{
    Do(m_NewSortOrder);
}

OUString SdrUndoSort::GetComment() const
{
    return SvxResId(STR_SortShapes);
}

SdrUndoObjSetText::SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText)
    : SdrUndoObj(rNewObj)
    , bNewTextAvailable(false)
    , bEmptyPresObj(false)
    , mnText(nText)
{
    SdrText* pText = static_cast< SdrTextObj*>( &rNewObj )->getText(mnText);
    if( pText && pText->GetOutlinerParaObject() )
        pOldText = *pText->GetOutlinerParaObject();

    bEmptyPresObj = rNewObj.IsEmptyPresObj();
}

SdrUndoObjSetText::~SdrUndoObjSetText()
{
    pOldText.reset();
    pNewText.reset();
}

bool SdrUndoObjSetText::IsDifferent() const
{
    if (!pOldText || !pNewText)
        return pOldText || pNewText;
    return *pOldText != *pNewText;
}

void SdrUndoObjSetText::AfterSetText()
{
    if (!bNewTextAvailable)
    {
        SdrText* pText = static_cast< SdrTextObj*>( mxObj.get() )->getText(mnText);
        if( pText && pText->GetOutlinerParaObject() )
            pNewText = *pText->GetOutlinerParaObject();
        bNewTextAvailable=true;
    }
}

void SdrUndoObjSetText::Undo()
{
    // only works with SdrTextObj
    SdrTextObj* pTarget = DynCastSdrTextObj(mxObj.get());

    if(!pTarget)
    {
        OSL_ENSURE(false, "SdrUndoObjSetText::Undo with SdrObject not based on SdrTextObj (!)");
        return;
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    // save old text for Redo
    if(!bNewTextAvailable)
    {
        AfterSetText();
    }

    SdrText* pText = pTarget->getText(mnText);
    if (pText)
    {
        // copy text for Undo, because the original now belongs to SetOutlinerParaObject()
        pTarget->NbcSetOutlinerParaObjectForText(pOldText, pText);
    }

    pTarget->SetEmptyPresObj(bEmptyPresObj);
    pTarget->ActionChanged();

    // #i124389# if it's a table, also need to relayout TextFrame
    if(dynamic_cast< sdr::table::SdrTableObj* >(pTarget) !=  nullptr)
    {
        pTarget->NbcAdjustTextFrameWidthAndHeight();
    }

    // #i122410# SetOutlinerParaObject at SdrText does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    pTarget->BroadcastObjectChange();
}

void SdrUndoObjSetText::Redo()
{
    // only works with SdrTextObj
    SdrTextObj* pTarget = DynCastSdrTextObj(mxObj.get());

    if(!pTarget)
    {
        OSL_ENSURE(false, "SdrUndoObjSetText::Redo with SdrObject not based on SdrTextObj (!)");
        return;
    }

    SdrText* pText = pTarget->getText(mnText);
    if (pText)
    {
        // copy text for Undo, because the original now belongs to SetOutlinerParaObject()
        pTarget->NbcSetOutlinerParaObjectForText( pNewText, pText );
    }

    pTarget->ActionChanged();

    // #i124389# if it's a table, also need to relayout TextFrame
    if(dynamic_cast< sdr::table::SdrTableObj* >(pTarget) !=  nullptr)
    {
        pTarget->NbcAdjustTextFrameWidthAndHeight();
    }

    // #i122410# NbcSetOutlinerParaObjectForText at SdrTextObj does not trigger a
    // BroadcastObjectChange, but it is needed to make evtl. SlideSorters
    // update their preview.
    pTarget->BroadcastObjectChange();

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

OUString SdrUndoObjSetText::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoObjSetText);
}

OUString SdrUndoObjSetText::GetSdrRepeatComment() const
{
    return ImpGetDescriptionStr(STR_UndoObjSetText);
}

void SdrUndoObjSetText::SdrRepeat(SdrView& rView)
{
    if (!(bNewTextAvailable && rView.AreObjectsMarked()))
        return;

    const SdrMarkList& rML=rView.GetMarkedObjectList();

    const bool bUndo = rView.IsUndoEnabled();
    if( bUndo )
    {
        OUString aStr = ImpGetDescriptionStr(STR_UndoObjSetText);
        rView.BegUndo(aStr);
    }

    const size_t nCount=rML.GetMarkCount();
    for (size_t nm=0; nm<nCount; ++nm)
    {
        SdrObject* pObj2=rML.GetMark(nm)->GetMarkedSdrObj();
        SdrTextObj* pTextObj=DynCastSdrTextObj( pObj2 );
        if (pTextObj!=nullptr)
        {
            if( bUndo )
                rView.AddUndo(std::make_unique<SdrUndoObjSetText>(*pTextObj,0));

            pTextObj->SetOutlinerParaObject(pNewText);
        }
    }

    if( bUndo )
        rView.EndUndo();
}

bool SdrUndoObjSetText::CanSdrRepeat(SdrView& rView) const
{
    bool bOk = false;
    if (bNewTextAvailable && rView.AreObjectsMarked()) {
        bOk=true;
    }
    return bOk;
}

// Undo/Redo for setting object's name (#i73249#)
SdrUndoObjStrAttr::SdrUndoObjStrAttr( SdrObject& rNewObj,
                                      const ObjStrAttrType eObjStrAttr,
                                      OUString sOldStr,
                                      OUString sNewStr)
    : SdrUndoObj( rNewObj )
    , meObjStrAttr( eObjStrAttr )
    , msOldStr(std::move( sOldStr ))
    , msNewStr(std::move( sNewStr ))
{
}

void SdrUndoObjStrAttr::Undo()
{
    ImpShowPageOfThisObject();

    switch ( meObjStrAttr )
    {
    case ObjStrAttrType::Name:
        mxObj->SetName( msOldStr );
        break;
    case ObjStrAttrType::Title:
        mxObj->SetTitle( msOldStr );
        break;
    case ObjStrAttrType::Description:
        mxObj->SetDescription( msOldStr );
        break;
    }
}

void SdrUndoObjStrAttr::Redo()
{
    switch ( meObjStrAttr )
    {
    case ObjStrAttrType::Name:
        mxObj->SetName( msNewStr );
        break;
    case ObjStrAttrType::Title:
        mxObj->SetTitle( msNewStr );
        break;
    case ObjStrAttrType::Description:
        mxObj->SetDescription( msNewStr );
        break;
    }

    ImpShowPageOfThisObject();
}

OUString SdrUndoObjStrAttr::GetComment() const
{
    OUString aStr;
    switch ( meObjStrAttr )
    {
    case ObjStrAttrType::Name:
        aStr = ImpGetDescriptionStr( STR_UndoObjName) +
                " '" + msNewStr + "'";
        break;
    case ObjStrAttrType::Title:
        aStr = ImpGetDescriptionStr( STR_UndoObjTitle );
        break;
    case ObjStrAttrType::Description:
        aStr = ImpGetDescriptionStr( STR_UndoObjDescription );
        break;
    }

    return aStr;
}

SdrUndoObjDecorative::SdrUndoObjDecorative(SdrObject & rObj, bool const WasDecorative)
    : SdrUndoObj(rObj)
    , m_WasDecorative(WasDecorative)
{
}

void SdrUndoObjDecorative::Undo()
{
    ImpShowPageOfThisObject();

    mxObj->SetDecorative(m_WasDecorative);
}

void SdrUndoObjDecorative::Redo()
{
    mxObj->SetDecorative(!m_WasDecorative);

    ImpShowPageOfThisObject();
}

OUString SdrUndoObjDecorative::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoObjDecorative);
}


SdrUndoLayer::SdrUndoLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
    : SdrUndoAction(rNewModel)
    , pLayer(rNewLayerAdmin.GetLayer(nLayerNum))
    , pLayerAdmin(&rNewLayerAdmin)
    , nNum(nLayerNum)
    , bItsMine(false)
{
}

SdrUndoLayer::~SdrUndoLayer()
{
    if (bItsMine)
    {
        delete pLayer;
    }
}

void SdrUndoNewLayer::Undo()
{
    DBG_ASSERT(!bItsMine,"SdrUndoNewLayer::Undo(): Layer already belongs to UndoAction.");
    bItsMine=true;
    // coverity[leaked_storage] - owned by this SdrUndoNewLayer as pLayer
    SdrLayer* pCmpLayer = pLayerAdmin->RemoveLayer(nNum).release();
    assert(pCmpLayer == pLayer && "SdrUndoNewLayer::Undo(): Removed layer is != pLayer."); (void)pCmpLayer;
}

void SdrUndoNewLayer::Redo()
{
    DBG_ASSERT(bItsMine,"SdrUndoNewLayer::Undo(): Layer does not belong to UndoAction.");
    bItsMine=false;
    pLayerAdmin->InsertLayer(std::unique_ptr<SdrLayer>(pLayer),nNum);
}

OUString SdrUndoNewLayer::GetComment() const
{
    return SvxResId(STR_UndoNewLayer);
}


void SdrUndoDelLayer::Undo()
{
    DBG_ASSERT(bItsMine,"SdrUndoDelLayer::Undo(): Layer does not belong to UndoAction.");
    bItsMine=false;
    pLayerAdmin->InsertLayer(std::unique_ptr<SdrLayer>(pLayer),nNum);
}

void SdrUndoDelLayer::Redo()
{
    DBG_ASSERT(!bItsMine,"SdrUndoDelLayer::Undo(): Layer already belongs to UndoAction.");
    bItsMine=true;
    // coverity[leaked_storage] - owned by this SdrUndoNewLayer as pLayer
    SdrLayer* pCmpLayer= pLayerAdmin->RemoveLayer(nNum).release();
    assert(pCmpLayer == pLayer && "SdrUndoDelLayer::Redo(): Removed layer is != pLayer."); (void)pCmpLayer;
}

OUString SdrUndoDelLayer::GetComment() const
{
    return SvxResId(STR_UndoDelLayer);
}


SdrUndoPage::SdrUndoPage(SdrPage& rNewPg)
:   SdrUndoAction(rNewPg.getSdrModelFromSdrPage())
    ,mxPage(&rNewPg)
{
}

SdrUndoPage::~SdrUndoPage() {}

void SdrUndoPage::ImpInsertPage(sal_uInt16 nNum)
{
    DBG_ASSERT(!mxPage->IsInserted(),"SdrUndoPage::ImpInsertPage(): mxPage is already inserted.");
    if (!mxPage->IsInserted())
    {
        if (mxPage->IsMasterPage())
        {
            m_rMod.InsertMasterPage(mxPage.get(), nNum);
        }
        else
        {
            m_rMod.InsertPage(mxPage.get(), nNum);
        }
    }
}

void SdrUndoPage::ImpRemovePage(sal_uInt16 nNum)
{
    DBG_ASSERT(mxPage->IsInserted(),"SdrUndoPage::ImpRemovePage(): mxPage is not inserted.");
    if (!mxPage->IsInserted())
        return;

    rtl::Reference<SdrPage> pChkPg;
    if (mxPage->IsMasterPage())
    {
        pChkPg = m_rMod.RemoveMasterPage(nNum);
    }
    else
    {
        pChkPg = m_rMod.RemovePage(nNum);
    }
    DBG_ASSERT(pChkPg==mxPage,"SdrUndoPage::ImpRemovePage(): RemovePage!=mxPage");
}

void SdrUndoPage::ImpMovePage(sal_uInt16 nOldNum, sal_uInt16 nNewNum)
{
    DBG_ASSERT(mxPage->IsInserted(),"SdrUndoPage::ImpMovePage(): mxPage is not inserted.");
    if (mxPage->IsInserted())
    {
        if (mxPage->IsMasterPage())
        {
            m_rMod.MoveMasterPage(nOldNum,nNewNum);
        }
        else
        {
            m_rMod.MovePage(nOldNum,nNewNum);
        }
    }
}

OUString SdrUndoPage::ImpGetDescriptionStr(TranslateId pStrCacheID)
{
    return SvxResId(pStrCacheID);
}


SdrUndoPageList::SdrUndoPageList(SdrPage& rNewPg)
    : SdrUndoPage(rNewPg)
{
    nPageNum=rNewPg.GetPageNum();
}

SdrUndoPageList::~SdrUndoPageList()
{
}


SdrUndoDelPage::SdrUndoDelPage(SdrPage& rNewPg)
    : SdrUndoPageList(rNewPg)
    , mbHasFillBitmap(false)
{
    // keep fill bitmap separately to remove it from pool if not used elsewhere
    if (mxPage->IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mxPage->getSdrPageProperties().GetStyleSheet();
        if (pStyleSheet)
            queryFillBitmap(pStyleSheet->GetItemSet());
    }
    else
    {
        queryFillBitmap(mxPage->getSdrPageProperties().GetItemSet());
    }
    if (bool(mpFillBitmapItem))
        clearFillBitmap();

    // now remember the master page relationships
    if(!mxPage->IsMasterPage())
        return;

    sal_uInt16 nPageCnt(m_rMod.GetPageCount());

    for(sal_uInt16 nPageNum2(0); nPageNum2 < nPageCnt; nPageNum2++)
    {
        SdrPage* pDrawPage = m_rMod.GetPage(nPageNum2);

        if(pDrawPage->TRG_HasMasterPage())
        {
            SdrPage& rMasterPage = pDrawPage->TRG_GetMasterPage();

            if(mxPage.get() == &rMasterPage)
            {
                if(!pUndoGroup)
                {
                    pUndoGroup.reset( new SdrUndoGroup(m_rMod) );
                }

                pUndoGroup->AddAction(m_rMod.GetSdrUndoFactory().CreateUndoPageRemoveMasterPage(*pDrawPage));
            }
        }
    }
}

SdrUndoDelPage::~SdrUndoDelPage()
{
}

void SdrUndoDelPage::Undo()
{
    if (bool(mpFillBitmapItem))
        restoreFillBitmap();
    ImpInsertPage(nPageNum);
    if (pUndoGroup!=nullptr)
    {
        // recover master page relationships
        pUndoGroup->Undo();
    }
}

void SdrUndoDelPage::Redo()
{
    ImpRemovePage(nPageNum);
    if (bool(mpFillBitmapItem))
        clearFillBitmap();
}

OUString SdrUndoDelPage::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoDelPage);
}

OUString SdrUndoDelPage::GetSdrRepeatComment() const
{
    return ImpGetDescriptionStr(STR_UndoDelPage);
}

void SdrUndoDelPage::SdrRepeat(SdrView& /*rView*/)
{
}

bool SdrUndoDelPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

void SdrUndoDelPage::queryFillBitmap(const SfxItemSet& rItemSet)
{
    if (const XFillBitmapItem *pItem = rItemSet.GetItemIfSet(XATTR_FILLBITMAP, false))
        mpFillBitmapItem.reset(pItem->Clone());
    if (const XFillStyleItem *pItem = rItemSet.GetItemIfSet(XATTR_FILLSTYLE, false))
        mbHasFillBitmap = pItem->GetValue() == css::drawing::FillStyle_BITMAP;
}

void SdrUndoDelPage::clearFillBitmap()
{
    if (mxPage->IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mxPage->getSdrPageProperties().GetStyleSheet();
        assert(bool(pStyleSheet)); // who took away my stylesheet?
        if (pStyleSheet->GetListenerCount() == 1)
        {
            SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
            rItemSet.ClearItem(XATTR_FILLBITMAP);
            if (mbHasFillBitmap)
                rItemSet.ClearItem(XATTR_FILLSTYLE);
        }
    }
    else
    {
        SdrPageProperties &rPageProps = mxPage->getSdrPageProperties();
        rPageProps.ClearItem(XATTR_FILLBITMAP);
        if (mbHasFillBitmap)
            rPageProps.ClearItem(XATTR_FILLSTYLE);
    }
}

void SdrUndoDelPage::restoreFillBitmap()
{
    if (mxPage->IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mxPage->getSdrPageProperties().GetStyleSheet();
        assert(bool(pStyleSheet)); // who took away my stylesheet?
        if (pStyleSheet->GetListenerCount() == 1)
        {
            SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
            rItemSet.Put(*mpFillBitmapItem);
            if (mbHasFillBitmap)
                rItemSet.Put(XFillStyleItem(css::drawing::FillStyle_BITMAP));
        }
    }
    else
    {
        SdrPageProperties &rPageProps = mxPage->getSdrPageProperties();
        rPageProps.PutItem(*mpFillBitmapItem);
        if (mbHasFillBitmap)
            rPageProps.PutItem(XFillStyleItem(css::drawing::FillStyle_BITMAP));
    }
}


void SdrUndoNewPage::Undo()
{
    ImpRemovePage(nPageNum);
}

void SdrUndoNewPage::Redo()
{
    ImpInsertPage(nPageNum);
}

OUString SdrUndoNewPage::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoNewPage);
}


OUString SdrUndoCopyPage::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoCopPage);
}

OUString SdrUndoCopyPage::GetSdrRepeatComment() const
{
    return ImpGetDescriptionStr(STR_UndoCopPage);
}

void SdrUndoCopyPage::SdrRepeat(SdrView& /*rView*/)
{

}

bool SdrUndoCopyPage::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}


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
    return ImpGetDescriptionStr(STR_UndoMovPage);
}

SdrUndoPageMasterPage::SdrUndoPageMasterPage(SdrPage& rChangedPage)
    : SdrUndoPage(rChangedPage)
    , mbOldHadMasterPage(mxPage->TRG_HasMasterPage())
    , maOldMasterPageNumber(0)
{
    // get current state from page
    if(mbOldHadMasterPage)
    {
        maOldSet = mxPage->TRG_GetMasterPageVisibleLayers();
        maOldMasterPageNumber = mxPage->TRG_GetMasterPage().GetPageNum();
    }
}

SdrUndoPageMasterPage::~SdrUndoPageMasterPage()
{
}

SdrUndoPageRemoveMasterPage::SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage)
{
}

void SdrUndoPageRemoveMasterPage::Undo()
{
    if(mbOldHadMasterPage)
    {
        mxPage->TRG_SetMasterPage(*mxPage->getSdrModelFromSdrPage().GetMasterPage(maOldMasterPageNumber));
        mxPage->TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageRemoveMasterPage::Redo()
{
    mxPage->TRG_ClearMasterPage();
}

OUString SdrUndoPageRemoveMasterPage::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoDelPageMasterDscr);
}

SdrUndoPageChangeMasterPage::SdrUndoPageChangeMasterPage(SdrPage& rChangedPage)
    : SdrUndoPageMasterPage(rChangedPage)
    , mbNewHadMasterPage(false)
    , maNewMasterPageNumber(0)
{
}

void SdrUndoPageChangeMasterPage::Undo()
{
    // remember values from new page
    if(mxPage->TRG_HasMasterPage())
    {
        mbNewHadMasterPage = true;
        maNewSet = mxPage->TRG_GetMasterPageVisibleLayers();
        maNewMasterPageNumber = mxPage->TRG_GetMasterPage().GetPageNum();
    }

    // restore old values
    if(mbOldHadMasterPage)
    {
        mxPage->TRG_ClearMasterPage();
        mxPage->TRG_SetMasterPage(*mxPage->getSdrModelFromSdrPage().GetMasterPage(maOldMasterPageNumber));
        mxPage->TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageChangeMasterPage::Redo()
{
    // restore new values
    if(mbNewHadMasterPage)
    {
        mxPage->TRG_ClearMasterPage();
        mxPage->TRG_SetMasterPage(*mxPage->getSdrModelFromSdrPage().GetMasterPage(maNewMasterPageNumber));
        mxPage->TRG_SetMasterPageVisibleLayers(maNewSet);
    }
}

OUString SdrUndoPageChangeMasterPage::GetComment() const
{
    return ImpGetDescriptionStr(STR_UndoChgPageMasterDscr);
}


SdrUndoFactory::~SdrUndoFactory(){}

// shapes

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoMoveObject( SdrObject& rObject, const Size& rDist )
{
    return std::make_unique<SdrUndoMoveObj>( rObject, rDist );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoGeoObject( SdrObject& rObject )
{
    return std::make_unique<SdrUndoGeoObj>( rObject );
}

// Diagram ModelData changes
std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoDiagramModelData( SdrObject& rObject, std::shared_ptr< svx::diagram::DiagramDataState >& rStartState )
{
    return std::make_unique<SdrUndoDiagramModelData>( rObject, rStartState );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return std::make_unique<SdrUndoAttrObj>( rObject, bStyleSheet1, bSaveText );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoRemoveObject(SdrObject& rObject)
{
    return std::make_unique<SdrUndoRemoveObj>(rObject);
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoInsertObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoInsertObj>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoDeleteObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoDelObj>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoNewObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoNewObj>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoCopyObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoCopyObj>( rObject, bOrdNumDirect );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoObjectOrdNum( SdrObject& rObject, sal_uInt32 nOldOrdNum1, sal_uInt32 nNewOrdNum1)
{
    return std::make_unique<SdrUndoObjOrdNum>( rObject, nOldOrdNum1, nNewOrdNum1 );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoSort(SdrPage & rPage, ::std::vector<sal_Int32> const& rSortOrder)
{
    return std::make_unique<SdrUndoSort>(rPage, rSortOrder);
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject )
{
    return std::make_unique<SdrUndoReplaceObj>( rOldObject, rNewObject );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoObjectLayerChange( SdrObject& rObject, SdrLayerID aOldLayer, SdrLayerID aNewLayer )
{
    return std::make_unique<SdrUndoObjectLayerChange>( rObject, aOldLayer, aNewLayer );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoObjectSetText( SdrObject& rNewObj, sal_Int32 nText )
{
    return std::make_unique<SdrUndoObjSetText>( rNewObj, nText );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoObjectStrAttr( SdrObject& rObject,
                                                        SdrUndoObjStrAttr::ObjStrAttrType eObjStrAttrType,
                                                        const OUString& sOldStr,
                                                        const OUString& sNewStr )
{
    return std::make_unique<SdrUndoObjStrAttr>( rObject, eObjStrAttrType, sOldStr, sNewStr );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoObjectDecorative(
        SdrObject& rObject, bool const WasDecorative)
{
    return std::make_unique<SdrUndoObjDecorative>(rObject, WasDecorative);
}


// layer
std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoNewLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return std::make_unique<SdrUndoNewLayer>( nLayerNum, rNewLayerAdmin, rNewModel );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoDeleteLayer(sal_uInt16 nLayerNum, SdrLayerAdmin& rNewLayerAdmin, SdrModel& rNewModel)
{
    return std::make_unique<SdrUndoDelLayer>( nLayerNum, rNewLayerAdmin, rNewModel );
}

// page
std::unique_ptr<SdrUndoAction>  SdrUndoFactory::CreateUndoDeletePage(SdrPage& rPage)
{
    return std::make_unique<SdrUndoDelPage>(rPage);
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoNewPage(SdrPage& rPage)
{
    return std::make_unique<SdrUndoNewPage>( rPage );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoCopyPage(SdrPage& rPage)
{
    return std::make_unique<SdrUndoCopyPage>( rPage );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoSetPageNum(SdrPage& rNewPg, sal_uInt16 nOldPageNum1, sal_uInt16 nNewPageNum1)
{
    return std::make_unique<SdrUndoSetPageNum>( rNewPg, nOldPageNum1, nNewPageNum1 );
}
    // master page
std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoPageRemoveMasterPage(SdrPage& rChangedPage)
{
    return std::make_unique<SdrUndoPageRemoveMasterPage>( rChangedPage );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoPageChangeMasterPage(SdrPage& rChangedPage)
{
    return std::make_unique<SdrUndoPageChangeMasterPage>(rChangedPage);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
