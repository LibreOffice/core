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

#include <svl/lstner.hxx>

#include <svx/svdundo.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/xfillit0.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/scene3d.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdogrp.hxx>
#include <sdr/properties/itemsettools.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <svx/svdocapt.hxx>
#include <svl/whiter.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/svdviter.hxx>
#include <svx/svdograf.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <svx/svdotable.hxx> // #i124389#
#include <vcl/svapp.hxx>
#include <sfx2/viewsh.hxx>


// iterates over all views and unmarks this SdrObject if it is marked
static void ImplUnmarkObject( SdrObject* pObj )
{
    SdrViewIter aIter( pObj );
    for ( SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView() )
    {
        pView->MarkObj( pObj, pView->GetSdrPageView(), true );
    }
}

SdrUndoAction::SdrUndoAction(SdrModel& rNewMod)
    : rMod(rNewMod), m_nViewShellId(-1)
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
    if (pV!=nullptr) return GetSdrRepeatComment(*pV);
    return OUString();
}

bool SdrUndoAction::CanSdrRepeat(SdrView& /*rView*/) const
{
    return false;
}

void SdrUndoAction::SdrRepeat(SdrView& /*rView*/)
{
}

OUString SdrUndoAction::GetSdrRepeatComment(SdrView& /*rView*/) const
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

OUString SdrUndoGroup::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    return aComment.replaceAll("%1", SvxResId(STR_ObjNameSingulPlural));
}

SdrUndoObj::SdrUndoObj(SdrObject& rNewObj)
:   SdrUndoAction(rNewObj.getSdrModelFromSdrObject())
    ,pObj(&rNewObj)
{
}

OUString SdrUndoObj::GetDescriptionStringForObject( const SdrObject& _rForObject, const char* pStrCacheID, bool bRepeat )
{
    const OUString rStr {SvxResId(pStrCacheID)};

    const sal_Int32 nPos = rStr.indexOf("%1");
    if (nPos < 0)
        return rStr;

    if (bRepeat)
        return rStr.replaceAt(nPos, 2, SvxResId(STR_ObjNameSingulPlural));

    return rStr.replaceAt(nPos, 2, _rForObject.TakeObjNameSingul());
}

void SdrUndoObj::ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr, bool bRepeat) const
{
    if ( pObj )
        rStr = GetDescriptionStringForObject( *pObj, pStrCacheID, bRepeat );
}

// common call method for possible change of the page when UNDO/REDO is triggered
void SdrUndoObj::ImpShowPageOfThisObject()
{
    if(pObj && pObj->IsInserted() && pObj->getSdrPageFromSdrObject())
    {
        SdrHint aHint(SdrHintKind::SwitchToPage, *pObj, pObj->getSdrPageFromSdrObject());
        pObj->getSdrModelFromSdrObject().Broadcast(aHint);
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
    , mxUndoStyleSheet()
    , mxRedoStyleSheet()
    , bHaveToTakeRedoSet(true)
{
    bStyleSheet = bStyleSheet1;

    SdrObjList* pOL = rNewObj.GetSubList();
    bool bIsGroup(pOL!=nullptr && pOL->GetObjCount());
    bool bIs3DScene(bIsGroup && dynamic_cast< E3dScene* >(pObj) !=  nullptr);

    if(bIsGroup)
    {
        // it's a group object!
        pUndoGroup.reset(new SdrUndoGroup(pObj->getSdrModelFromSdrObject()));
        const size_t nObjCount(pOL->GetObjCount());

        for(size_t nObjNum = 0; nObjNum < nObjCount; ++nObjNum)
        {
            pUndoGroup->AddAction(
                std::make_unique<SdrUndoAttrObj>(*pOL->GetObj(nObjNum), bStyleSheet1));
        }
    }

    if(!bIsGroup || bIs3DScene)
    {
        pUndoSet.reset( new SfxItemSet(pObj->GetMergedItemSet()) );

        if(bStyleSheet)
            mxUndoStyleSheet = pObj->GetStyleSheet();

        if(bSaveText)
        {
            auto p = pObj->GetOutlinerParaObject();
            if(p)
                pTextUndo.reset( new OutlinerParaObject(*p) );
        }
    }
}

SdrUndoAttrObj::~SdrUndoAttrObj()
{
    pUndoSet.reset();
    pRedoSet.reset();
    pUndoGroup.reset();
    pTextUndo.reset();
    pTextRedo.reset();
}

void SdrUndoAttrObj::Undo()
{
    E3DModifySceneSnapRectUpdater aUpdater(pObj);
    bool bIs3DScene(dynamic_cast< E3dScene* >(pObj) !=  nullptr);

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    if(!pUndoGroup || bIs3DScene)
    {
        if(bHaveToTakeRedoSet)
        {
            bHaveToTakeRedoSet = false;

            pRedoSet.reset( new SfxItemSet(pObj->GetMergedItemSet()) );

            if(bStyleSheet)
                mxRedoStyleSheet = pObj->GetStyleSheet();

            if(pTextUndo)
            {
                // #i8508#
                auto p = pObj->GetOutlinerParaObject();
                if(p)
                    pTextRedo.reset( new OutlinerParaObject(*p) );
            }
        }

        if(bStyleSheet)
        {
            mxRedoStyleSheet = pObj->GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxUndoStyleSheet.get());

            if(pSheet && pObj->getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*pObj->getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                pObj->SetStyleSheet(pSheet, true);
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
        // losing the geometry size info for the object when it is
        // laid out again from AdjustTextFrameWidthAndHeight(). This makes
        // rescuing the size of the object necessary.
        const tools::Rectangle aSnapRect = pObj->GetSnapRect();

        if(pUndoSet)
        {
            if(dynamic_cast<const SdrCaptionObj*>( pObj) !=  nullptr)
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
                    if(SfxItemState::SET != pUndoSet->GetItemState(nWhich, false))
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
            pObj->SetOutlinerParaObject(std::make_unique<OutlinerParaObject>(*pTextUndo));
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
    bool bIs3DScene(dynamic_cast< E3dScene* >(pObj) !=  nullptr);

    if(!pUndoGroup || bIs3DScene)
    {
        if(bStyleSheet)
        {
            mxUndoStyleSheet = pObj->GetStyleSheet();
            SfxStyleSheet* pSheet = dynamic_cast< SfxStyleSheet* >(mxRedoStyleSheet.get());

            if(pSheet && pObj->getSdrModelFromSdrObject().GetStyleSheetPool())
            {
                ensureStyleSheetInStyleSheetPool(*pObj->getSdrModelFromSdrObject().GetStyleSheetPool(), *pSheet);
                pObj->SetStyleSheet(pSheet, true);
            }
            else
            {
                OSL_ENSURE(false, "OOps, something went wrong in SdrUndoAttrObj (!)");
            }
        }

        sdr::properties::ItemChangeBroadcaster aItemChange(*pObj);

        const tools::Rectangle aSnapRect = pObj->GetSnapRect();

        if(pRedoSet)
        {
            if(dynamic_cast<const SdrCaptionObj*>( pObj) !=  nullptr)
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
                    if(SfxItemState::SET != pRedoSet->GetItemState(nWhich, false))
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
            pObj->SetOutlinerParaObject(std::make_unique<OutlinerParaObject>(*pTextRedo));
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

OUString SdrUndoAttrObj::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;

    if(bStyleSheet)
    {
        ImpTakeDescriptionStr(STR_EditSetStylesheet, aStr, true);
    }
    else
    {
        ImpTakeDescriptionStr(STR_EditSetAttributes, aStr, true);
    }

    return aStr;
}


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
    ImpTakeDescriptionStr(STR_EditMove,aStr,true);
    return aStr;
}


SdrUndoGeoObj::SdrUndoGeoObj(SdrObject& rNewObj)
     : SdrUndoObj(rNewObj)
     , mbSkipChangeLayout(false)
{
    SdrObjList* pOL=rNewObj.GetSubList();
    if (pOL!=nullptr && pOL->GetObjCount() && dynamic_cast<const E3dScene* >( &rNewObj) ==  nullptr)
    {
        // this is a group object!
        // If this were 3D scene, we'd only add an Undo for the scene itself
        // (which we do elsewhere).
        pUndoGroup.reset(new SdrUndoGroup(pObj->getSdrModelFromSdrObject()));
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t nObjNum = 0; nObjNum<nObjCount; ++nObjNum) {
            pUndoGroup->AddAction(std::make_unique<SdrUndoGeoObj>(*pOL->GetObj(nObjNum)));
        }
    }
    else
    {
        pUndoGeo.reset(pObj->GetGeoData());
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
        pObj->ActionChanged();
    }
    else
    {
        pRedoGeo.reset(pObj->GetGeoData());

        auto pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pObj);
        if (pTableObj && mbSkipChangeLayout)
            pTableObj->SetSkipChangeLayout(true);
        pObj->SetGeoData(*pUndoGeo);
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
        pObj->ActionChanged();
    }
    else
    {
        pUndoGeo.reset(pObj->GetGeoData());
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


SdrUndoObjList::SdrUndoObjList(SdrObject& rNewObj, bool bOrdNumDirect)
    : SdrUndoObj(rNewObj)
    , bOwner(false)
{
    pObjList=pObj->getParentSdrObjListFromSdrObject();
    if (bOrdNumDirect)
    {
        nOrdNum=pObj->GetOrdNumDirect();
    }
    else
    {
        nOrdNum=pObj->GetOrdNum();
    }
}

SdrUndoObjList::~SdrUndoObjList()
{
    SolarMutexGuard aGuard;

    if (pObj!=nullptr && IsOwner())
    {
        // Attribute have to go back to the regular Pool
        SetOwner(false);

        // now delete
        SdrObject::Free( pObj );
    }
}

void SdrUndoObjList::SetOwner(bool bNew)
{
    bOwner = bNew;
}


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

        if (dynamic_cast< const SdrObjGroup* >(pObjList->getSdrObjectFromSdrObjList()) != nullptr)
        {
            aOwnerAnchorPos = pObjList->getSdrObjectFromSdrObjList()->GetAnchorPos();
        }

        E3DModifySceneSnapRectUpdater aUpdater(pObjList->getSdrObjectFromSdrObjList());
        pObjList->InsertObject(pObj,nOrdNum);

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

SdrUndoRemoveObj::~SdrUndoRemoveObj()
{
}


void SdrUndoInsertObj::Undo()
{
    // Trigger PageChangeCall
    ImpShowPageOfThisObject();

    DBG_ASSERT(pObj->IsInserted(),"UndoInsertObj: pObj is not inserted.");
    if (pObj->IsInserted())
    {
        ImplUnmarkObject( pObj );

        SdrObject* pChkObj= pObjList->RemoveObject(nOrdNum);
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

        if (dynamic_cast<const SdrObjGroup*>(pObjList->getSdrObjectFromSdrObjList()) != nullptr)
        {
            aAnchorPos = pObj->GetAnchorPos();
        }

        pObjList->InsertObject(pObj,nOrdNum);

        // Arcs lose position when grouped (#i45952#)
        if ( aAnchorPos.X() || aAnchorPos.Y() )
        {
            pObj->NbcSetAnchorPos( aAnchorPos );
        }
    }

    // Trigger PageChangeCall
    ImpShowPageOfThisObject();
}

SdrUndoDelObj::SdrUndoDelObj(SdrObject& rNewObj, bool bOrdNumDirect)
:   SdrUndoRemoveObj(rNewObj,bOrdNumDirect)
{
    SetOwner(true);
}

void SdrUndoDelObj::Undo()
{
    SdrUndoRemoveObj::Undo();
    DBG_ASSERT(IsOwner(),"UndoDeleteObj: pObj does not belong to UndoAction");
    SetOwner(false);
}

void SdrUndoDelObj::Redo()
{
    SdrUndoRemoveObj::Redo();
    DBG_ASSERT(!IsOwner(),"RedoDeleteObj: pObj already belongs to UndoAction");
    SetOwner(true);
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
    ImpTakeDescriptionStr(STR_EditDelete,aStr,true);
    return aStr;
}


void SdrUndoNewObj::Undo()
{
    SdrUndoInsertObj::Undo();
    DBG_ASSERT(!IsOwner(),"RedoNewObj: pObj already belongs to UndoAction");
    SetOwner(true);
}

void SdrUndoNewObj::Redo()
{
    SdrUndoInsertObj::Redo();
    DBG_ASSERT(IsOwner(),"RedoNewObj: pObj does not belong to UndoAction");
    SetOwner(false);
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
    , bOldOwner(false)
    , bNewOwner(false)
    , pNewObj(&rNewObj1)
{
    SetOldOwner(true);

    pObjList=pObj->getParentSdrObjListFromSdrObject();
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
    if (pObj!=nullptr && IsOldOwner())
    {
        // Attribute have to go back into the Pool
        SetOldOwner(false);

        // now delete
        SdrObject::Free( pObj );
    }
    if (pNewObj!=nullptr && IsNewOwner())
    {
        // Attribute have to go back into the Pool
        SetNewOwner(false);

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
        SetOldOwner(false);
        SetNewOwner(true);

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
        SetOldOwner(true);
        SetNewOwner(false);

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


OUString SdrUndoCopyObj::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopyObj,aStr);
    return aStr;
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
    pObj->SetLayer(maOldLayer);
}

void SdrUndoObjectLayerChange::Redo()
{
    pObj->SetLayer(maNewLayer);
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

    SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
    if (pOL==nullptr)
    {
        OSL_FAIL("UndoObjOrdNum: pObj does not have an ObjList.");
        return;
    }
    pOL->SetObjectOrdNum(nNewOrdNum,nOldOrdNum);
}

void SdrUndoObjOrdNum::Redo()
{
    SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
    if (pOL==nullptr)
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


SdrUndoObjSetText::SdrUndoObjSetText(SdrObject& rNewObj, sal_Int32 nText)
    : SdrUndoObj(rNewObj)
    , bNewTextAvailable(false)
    , bEmptyPresObj(false)
    , mnText(nText)
{
    SdrText* pText = static_cast< SdrTextObj*>( &rNewObj )->getText(mnText);
    if( pText && pText->GetOutlinerParaObject() )
        pOldText.reset( new OutlinerParaObject(*pText->GetOutlinerParaObject()) );

    bEmptyPresObj = rNewObj.IsEmptyPresObj();
}

SdrUndoObjSetText::~SdrUndoObjSetText()
{
    pOldText.reset();
    pNewText.reset();
}

void SdrUndoObjSetText::AfterSetText()
{
    if (!bNewTextAvailable)
    {
        SdrText* pText = static_cast< SdrTextObj*>( pObj )->getText(mnText);
        if( pText && pText->GetOutlinerParaObject() )
            pNewText.reset( new OutlinerParaObject(*pText->GetOutlinerParaObject()) );
        bNewTextAvailable=true;
    }
}

void SdrUndoObjSetText::Undo()
{
    // only works with SdrTextObj
    SdrTextObj* pTarget = dynamic_cast< SdrTextObj* >(pObj);

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
        std::unique_ptr<OutlinerParaObject> pText1( pOldText ? new OutlinerParaObject(*pOldText) : nullptr );
        pTarget->NbcSetOutlinerParaObjectForText(std::move(pText1), pText);
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
    SdrTextObj* pTarget = dynamic_cast< SdrTextObj* >(pObj);

    if(!pTarget)
    {
        OSL_ENSURE(false, "SdrUndoObjSetText::Redo with SdrObject not based on SdrTextObj (!)");
        return;
    }

    SdrText* pText = pTarget->getText(mnText);
    if (pText)
    {
        // copy text for Undo, because the original now belongs to SetOutlinerParaObject()
        std::unique_ptr<OutlinerParaObject> pText1( pNewText ? new OutlinerParaObject(*pNewText) : nullptr );
        pTarget->NbcSetOutlinerParaObjectForText( std::move(pText1), pText );
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

        const size_t nCount=rML.GetMarkCount();
        for (size_t nm=0; nm<nCount; ++nm)
        {
            SdrObject* pObj2=rML.GetMark(nm)->GetMarkedSdrObj();
            SdrTextObj* pTextObj=dynamic_cast<SdrTextObj*>( pObj2 );
            if (pTextObj!=nullptr)
            {
                if( bUndo )
                    rView.AddUndo(std::make_unique<SdrUndoObjSetText>(*pTextObj,0));

                std::unique_ptr<OutlinerParaObject> pText1;
                if (pNewText)
                    pText1.reset(new OutlinerParaObject(*pNewText));
                pTextObj->SetOutlinerParaObject(std::move(pText1));
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
        bOk=true;
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
    case ObjStrAttrType::Name:
        pObj->SetName( msOldStr );
        break;
    case ObjStrAttrType::Title:
        pObj->SetTitle( msOldStr );
        break;
    case ObjStrAttrType::Description:
        pObj->SetDescription( msOldStr );
        break;
    }
}

void SdrUndoObjStrAttr::Redo()
{
    switch ( meObjStrAttr )
    {
    case ObjStrAttrType::Name:
        pObj->SetName( msNewStr );
        break;
    case ObjStrAttrType::Title:
        pObj->SetTitle( msNewStr );
        break;
    case ObjStrAttrType::Description:
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
    case ObjStrAttrType::Name:
        ImpTakeDescriptionStr( STR_UndoObjName, aStr );
        aStr += " '" + msNewStr + "'";
        break;
    case ObjStrAttrType::Title:
        ImpTakeDescriptionStr( STR_UndoObjTitle, aStr );
        break;
    case ObjStrAttrType::Description:
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
    SdrLayer* pCmpLayer= pLayerAdmin->RemoveLayer(nNum).release();
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoNewLayer::Undo(): Removed layer is != pLayer.");
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
    SdrLayer* pCmpLayer= pLayerAdmin->RemoveLayer(nNum).release();
    DBG_ASSERT(pCmpLayer==pLayer,"SdrUndoDelLayer::Redo(): Removed layer is != pLayer.");
}

OUString SdrUndoDelLayer::GetComment() const
{
    return SvxResId(STR_UndoDelLayer);
}


SdrUndoPage::SdrUndoPage(SdrPage& rNewPg)
:   SdrUndoAction(rNewPg.getSdrModelFromSdrPage())
    ,mrPage(rNewPg)
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
        SdrPage* pChkPg=nullptr;
        if (mrPage.IsMasterPage())
        {
            pChkPg=rMod.RemoveMasterPage(nNum);
        }
        else
        {
            pChkPg=rMod.RemovePage(nNum);
        }
        DBG_ASSERT(pChkPg==&mrPage,"SdrUndoPage::ImpRemovePage(): RemovePage!=&mrPage");
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

void SdrUndoPage::ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr)
{
    rStr = SvxResId(pStrCacheID);
}


SdrUndoPageList::SdrUndoPageList(SdrPage& rNewPg)
    : SdrUndoPage(rNewPg)
    , bItsMine(false)
{
    nPageNum=rNewPg.GetPageNum();
}

SdrUndoPageList::~SdrUndoPageList()
{
    if(bItsMine)
    {
        delete &mrPage;
    }
}


SdrUndoDelPage::SdrUndoDelPage(SdrPage& rNewPg)
    : SdrUndoPageList(rNewPg)
    , mbHasFillBitmap(false)
{
    bItsMine = true;

    // keep fill bitmap separately to remove it from pool if not used elsewhere
    if (mrPage.IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mrPage.getSdrPageProperties().GetStyleSheet();
        if (pStyleSheet)
            queryFillBitmap(pStyleSheet->GetItemSet());
    }
    else
    {
        queryFillBitmap(mrPage.getSdrPageProperties().GetItemSet());
    }
    if (bool(mpFillBitmapItem))
        clearFillBitmap();

    // now remember the master page relationships
    if(mrPage.IsMasterPage())
    {
        sal_uInt16 nPageCnt(rMod.GetPageCount());

        for(sal_uInt16 nPageNum2(0); nPageNum2 < nPageCnt; nPageNum2++)
        {
            SdrPage* pDrawPage = rMod.GetPage(nPageNum2);

            if(pDrawPage->TRG_HasMasterPage())
            {
                SdrPage& rMasterPage = pDrawPage->TRG_GetMasterPage();

                if(&mrPage == &rMasterPage)
                {
                    if(!pUndoGroup)
                    {
                        pUndoGroup.reset( new SdrUndoGroup(rMod) );
                    }

                    pUndoGroup->AddAction(rMod.GetSdrUndoFactory().CreateUndoPageRemoveMasterPage(*pDrawPage));
                }
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
    DBG_ASSERT(bItsMine,"UndoDeletePage: mrPage does not belong to UndoAction.");
    bItsMine=false;
}

void SdrUndoDelPage::Redo()
{
    ImpRemovePage(nPageNum);
    if (bool(mpFillBitmapItem))
        clearFillBitmap();
    // master page relations are dissolved automatically
    DBG_ASSERT(!bItsMine,"RedoDeletePage: mrPage already belongs to UndoAction.");
    bItsMine=true;
}

OUString SdrUndoDelPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoDelPage,aStr);
    return aStr;
}

OUString SdrUndoDelPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoDelPage,aStr);
    return aStr;
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
    const SfxPoolItem *pItem = nullptr;
    if (rItemSet.GetItemState(XATTR_FILLBITMAP, false, &pItem) == SfxItemState::SET)
        mpFillBitmapItem.reset(pItem->Clone());
    if (rItemSet.GetItemState(XATTR_FILLSTYLE, false, &pItem) == SfxItemState::SET)
        mbHasFillBitmap = static_cast<const XFillStyleItem*>(pItem)->GetValue() == css::drawing::FillStyle_BITMAP;
}

void SdrUndoDelPage::clearFillBitmap()
{
    if (mrPage.IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mrPage.getSdrPageProperties().GetStyleSheet();
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
        SdrPageProperties &rPageProps = mrPage.getSdrPageProperties();
        rPageProps.ClearItem(XATTR_FILLBITMAP);
        if (mbHasFillBitmap)
            rPageProps.ClearItem(XATTR_FILLSTYLE);
    }
}

void SdrUndoDelPage::restoreFillBitmap()
{
    if (mrPage.IsMasterPage())
    {
        SfxStyleSheet* const pStyleSheet = mrPage.getSdrPageProperties().GetStyleSheet();
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
        SdrPageProperties &rPageProps = mrPage.getSdrPageProperties();
        rPageProps.PutItem(*mpFillBitmapItem);
        if (mbHasFillBitmap)
            rPageProps.PutItem(XFillStyleItem(css::drawing::FillStyle_BITMAP));
    }
}


void SdrUndoNewPage::Undo()
{
    ImpRemovePage(nPageNum);
    DBG_ASSERT(!bItsMine,"UndoNewPage: mrPage already belongs to UndoAction.");
    bItsMine=true;
}

void SdrUndoNewPage::Redo()
{
    ImpInsertPage(nPageNum);
    DBG_ASSERT(bItsMine,"RedoNewPage: mrPage does not belong to UndoAction.");
    bItsMine=false;
}

OUString SdrUndoNewPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoNewPage,aStr);
    return aStr;
}


OUString SdrUndoCopyPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopPage,aStr);
    return aStr;
}

OUString SdrUndoCopyPage::GetSdrRepeatComment(SdrView& /*rView*/) const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoCopPage,aStr);
    return aStr;
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
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoMovPage,aStr);
    return aStr;
}

SdrUndoPageMasterPage::SdrUndoPageMasterPage(SdrPage& rChangedPage)
    : SdrUndoPage(rChangedPage)
    , mbOldHadMasterPage(mrPage.TRG_HasMasterPage())
    , maOldMasterPageNumber(0)
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

SdrUndoPageRemoveMasterPage::SdrUndoPageRemoveMasterPage(SdrPage& rChangedPage)
:   SdrUndoPageMasterPage(rChangedPage)
{
}

void SdrUndoPageRemoveMasterPage::Undo()
{
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_SetMasterPage(*mrPage.getSdrModelFromSdrPage().GetMasterPage(maOldMasterPageNumber));
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
    ImpTakeDescriptionStr(STR_UndoDelPageMasterDscr,aStr);
    return aStr;
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
    if(mrPage.TRG_HasMasterPage())
    {
        mbNewHadMasterPage = true;
        maNewSet = mrPage.TRG_GetMasterPageVisibleLayers();
        maNewMasterPageNumber = mrPage.TRG_GetMasterPage().GetPageNum();
    }

    // restore old values
    if(mbOldHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrPage.getSdrModelFromSdrPage().GetMasterPage(maOldMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maOldSet);
    }
}

void SdrUndoPageChangeMasterPage::Redo()
{
    // restore new values
    if(mbNewHadMasterPage)
    {
        mrPage.TRG_ClearMasterPage();
        mrPage.TRG_SetMasterPage(*mrPage.getSdrModelFromSdrPage().GetMasterPage(maNewMasterPageNumber));
        mrPage.TRG_SetMasterPageVisibleLayers(maNewSet);
    }
}

OUString SdrUndoPageChangeMasterPage::GetComment() const
{
    OUString aStr;
    ImpTakeDescriptionStr(STR_UndoChgPageMasterDscr,aStr);
    return aStr;
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

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoAttrObject( SdrObject& rObject, bool bStyleSheet1, bool bSaveText )
{
    return std::make_unique<SdrUndoAttrObj>( rObject, bStyleSheet1, bSaveText );
}

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoRemoveObject( SdrObject& rObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoRemoveObj>( rObject, bOrdNumDirect );
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

std::unique_ptr<SdrUndoAction> SdrUndoFactory::CreateUndoReplaceObject( SdrObject& rOldObject, SdrObject& rNewObject, bool bOrdNumDirect )
{
    return std::make_unique<SdrUndoReplaceObj>( rOldObject, rNewObject, bOrdNumDirect );
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
