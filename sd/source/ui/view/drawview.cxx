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

#include <sfx2/dispatch.hxx>
#include <vcl/weld.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/request.hxx>
#include <svl/style.hxx>
#include <editeng/outliner.hxx>
#include <svx/view3d.hxx>
#include <svx/svxids.hrc>
#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdorect.hxx>
#include <svl/poolitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/bulletitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/numitem.hxx>
#include <svl/whiter.hxx>
#include <sal/log.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <stlsheet.hxx>

#include <svx/svdoutl.hxx>
#include <svx/svdundo.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <strings.hrc>
#include <View.hxx>
#include <sdattr.hxx>
#include <drawview.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdpage.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <pres.hxx>
#include <sdresid.hxx>
#include <Window.hxx>
#include <unchss.hxx>
#include <FrameView.hxx>
#include <anminfo.hxx>
#include <slideshow.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include <undo/undomanager.hxx>

using namespace ::com::sun::star;

namespace sd {


/**
 * Shows the first page of document at position 0,0. In the case
 * that there is no page a page is created.
 */

DrawView::DrawView(
    DrawDocShell* pDocSh,
    OutputDevice* pOutDev,
    DrawViewShell* pShell)
:   ::sd::View(*pDocSh->GetDoc(), pOutDev, pShell)
    ,mpDocShell(pDocSh)
    ,mpDrawViewShell(pShell)
    ,mnPOCHSmph(0)
{
    SetCurrentObj(OBJ_RECT);
}

DrawView::~DrawView()
{
}

/**
 * Virtual method from SdrView, called at selection change.
 */

void DrawView::MarkListHasChanged()
{
    ::sd::View::MarkListHasChanged();

    if (mpDrawViewShell)
        mpDrawViewShell->SelectionHasChanged();
}

/**
 * Virtual method from SdrView, called at model change.
 */

void DrawView::ModelHasChanged()
{
    ::sd::View::ModelHasChanged();

    // force framer to rerender
    SfxStyleSheetBasePool* pSSPool = mrDoc.GetStyleSheetPool();
    pSSPool->Broadcast(SfxStyleSheetPoolHint());

    if( mpDrawViewShell )
        mpDrawViewShell->ModelHasChanged();

}

/**
 * Redirect attributes onto title and outline text and background
 * rectangle of a masterpage into templates, otherwise pass on baseclass.
 */

bool DrawView::SetAttributes(const SfxItemSet& rSet,
                             bool bReplaceAll)
{
    bool bOk = false;

    // is there a masterpage edit?
    if ( mpDrawViewShell && mpDrawViewShell->GetEditMode() == EditMode::MasterPage )
    {
        SfxStyleSheetBasePool* pStShPool = mrDoc.GetStyleSheetPool();
        SdPage& rPage = *mpDrawViewShell->getCurrentPage();
        SdrTextObj* pEditObject = GetTextEditObject();

        if (pEditObject)
        {
            // Textedit

            SdrInventor nInv = pEditObject->GetObjInventor();

            if (nInv == SdrInventor::Default)
            {
                sal_uInt16 eObjKind = pEditObject->GetObjIdentifier();
                PresObjKind ePresObjKind = rPage.GetPresObjKind(pEditObject);

                if ( ePresObjKind == PRESOBJ_TITLE ||
                     ePresObjKind == PRESOBJ_NOTES )
                {
                    // Presentation object (except outline)
                    SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj( ePresObjKind );
                    DBG_ASSERT(pSheet, "StyleSheet not found");

                    SfxItemSet aTempSet( pSheet->GetItemSet() );
                    aTempSet.Put( rSet );
                    aTempSet.ClearInvalidItems();

                    // Undo-Action
                    mpDocSh->GetUndoManager()->AddUndoAction(
                        std::make_unique<StyleSheetUndoAction>(&mrDoc, pSheet, &aTempSet));

                    pSheet->GetItemSet().Put(aTempSet);
                    pSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                    bOk = true;
                }
                else if (eObjKind == OBJ_OUTLINETEXT)
                {
                    // Presentation object outline
                    OutlinerView* pOV   = GetTextEditOutlinerView();
                    ::Outliner* pOutliner = pOV->GetOutliner();

                    pOutliner->SetUpdateMode(false);
                    mpDocSh->SetWaitCursor( true );

                    // replace placeholder by template name
                    OUString aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
                    aComment = aComment.replaceFirst("$", SdResId(STR_PSEUDOSHEET_OUTLINE));
                    mpDocSh->GetUndoManager()->EnterListAction( aComment, OUString(), 0, mpDrawViewShell->GetViewShellBase().GetViewShellId() );

                    std::vector<Paragraph*> aSelList;
                    pOV->CreateSelectionList(aSelList);

                    std::vector<Paragraph*>::reverse_iterator iter = aSelList.rbegin();
                    Paragraph* pPara = iter != aSelList.rend() ? *iter : nullptr;

                    while (pPara)
                    {
                        sal_Int32 nParaPos = pOutliner->GetAbsPos( pPara );
                        sal_Int16 nDepth = pOutliner->GetDepth( nParaPos );
                        OUString aName = rPage.GetLayoutName() + " " +
                            OUString::number((nDepth <= 0) ? 1 : nDepth + 1);
                        SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(pStShPool->Find(aName, SfxStyleFamily::Page));
                        //We have no stylesheet if we access outline level 10
                        //in the master preview, there is no true style backing
                        //that entry
                        SAL_WARN_IF(!pSheet, "sd", "StyleSheet " << aName << " not found");
                        if (pSheet)
                        {
                            SfxItemSet aTempSet( pSheet->GetItemSet() );
                            aTempSet.Put( rSet );
                            aTempSet.ClearInvalidItems();

                            if( nDepth > 0 && aTempSet.GetItemState( EE_PARA_NUMBULLET ) == SfxItemState::SET )
                            {
                                // no SvxNumBulletItem in outline level 1 to 8!
                                aTempSet.ClearItem( EE_PARA_NUMBULLET );
                            }

                            // Undo-Action
                            mpDocSh->GetUndoManager()->AddUndoAction(
                                std::make_unique<StyleSheetUndoAction>(&mrDoc, pSheet, &aTempSet));

                            pSheet->GetItemSet().Put(aTempSet);
                            pSheet->Broadcast(SfxHint(SfxHintId::DataChanged));

                            // now also broadcast any child sheets
                            sal_Int16 nChild;
                            for( nChild = nDepth + 1; nChild < 9; nChild++ )
                            {
                                OUString aSheetName = rPage.GetLayoutName() + " " +
                                    OUString::number((nChild <= 0) ? 1 : nChild + 1);
                                SfxStyleSheet* pOutlSheet = static_cast< SfxStyleSheet* >(pStShPool->Find(aSheetName, SfxStyleFamily::Page));

                                if( pOutlSheet )
                                    pOutlSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                            }
                        }

                        ++iter;
                        pPara = iter != aSelList.rend() ? *iter : nullptr;

                        bool bJumpToLevel1 = false;
                        if( !pPara && nDepth > 0 && rSet.GetItemState( EE_PARA_NUMBULLET ) == SfxItemState::SET )
                            bJumpToLevel1 = true;

                        if (bJumpToLevel1)
                        {
                            iter = aSelList.rend();
                            --iter;

                            if (pOutliner->GetDepth(pOutliner->GetAbsPos(*iter)) > 0)
                                pPara = pOutliner->GetParagraph( 0 );  // Put NumBulletItem in outline level 1
                        }
                    }

                    mpDocSh->SetWaitCursor( false );
                    pOV->GetOutliner()->SetUpdateMode(true);

                    mpDocSh->GetUndoManager()->LeaveListAction();

                    bOk = true;
                }
                else
                {
                    bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
                }
            }
        }
        else
        {
            // Selection
            const SdrMarkList& rList = GetMarkedObjectList();
            const size_t nMarkCount = rList.GetMarkCount();
            for (size_t nMark = 0; nMark < nMarkCount; ++nMark)
            {
                SdrObject*  pObject = rList.GetMark(nMark)->GetMarkedSdrObj();
                SdrInventor nInv    = pObject->GetObjInventor();

                if (nInv == SdrInventor::Default)
                {
                    sal_uInt16 eObjKind = pObject->GetObjIdentifier();
                    PresObjKind ePresObjKind = rPage.GetPresObjKind(pObject);

                    if (ePresObjKind == PRESOBJ_TITLE ||
                        ePresObjKind == PRESOBJ_NOTES)
                    {
                        // Presentation object (except outline)
                        SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj( ePresObjKind );
                        DBG_ASSERT(pSheet, "StyleSheet not found");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        // Undo-Action
                        mpDocSh->GetUndoManager()->AddUndoAction(
                            std::make_unique<StyleSheetUndoAction>(&mrDoc, pSheet, &aTempSet));

                        pSheet->GetItemSet().Put(aTempSet,false);
                        pSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                        bOk = true;
                    }
                    else if (eObjKind == OBJ_OUTLINETEXT)
                    {
                        // Presentation object outline
                        for (sal_uInt16 nLevel = 9; nLevel > 0; nLevel--)
                        {
                            OUString aName = rPage.GetLayoutName() + " " +
                                OUString::number(nLevel);
                            SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(pStShPool->
                                                Find(aName, SfxStyleFamily::Page));
                            DBG_ASSERT(pSheet, "StyleSheet not found");

                            SfxItemSet aTempSet( pSheet->GetItemSet() );

                            if( nLevel > 1 )
                            {
                                // for all levels over 1, clear all items that will be
                                // hard set to level 1
                                SfxWhichIter aWhichIter(rSet);
                                sal_uInt16 nWhich(aWhichIter.FirstWhich());
                                while( nWhich )
                                {
                                    if( SfxItemState::SET == rSet.GetItemState( nWhich ) )
                                        aTempSet.ClearItem( nWhich );
                                    nWhich = aWhichIter.NextWhich();
                                }

                            }
                            else
                            {
                                // put the items hard into level one
                                aTempSet.Put( rSet );
                            }

                            aTempSet.ClearInvalidItems();

                            // Undo-Action
                            mpDocSh->GetUndoManager()->AddUndoAction(
                                std::make_unique<StyleSheetUndoAction>(&mrDoc, pSheet, &aTempSet));

                            pSheet->GetItemSet().Set(aTempSet,false);
                            pSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                        }

                        // remove all hard set items from shape that are now set in style
                        SfxWhichIter aWhichIter(rSet);
                        sal_uInt16 nWhich(aWhichIter.FirstWhich());
                        while( nWhich )
                        {
                            if( SfxItemState::SET == rSet.GetItemState( nWhich ) )
                                pObject->ClearMergedItem( nWhich );
                            nWhich = aWhichIter.NextWhich();
                        }

                        bOk = true;
                    }
                }
            }

            if(!bOk)
                bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
        }
    }
    else    // not at masterpage
    {
        bOk = ::sd::View::SetAttributes(rSet, bReplaceAll);
    }

    return bOk;
}

/**
 * Notify for change of site arrangement
 */

void DrawView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( mpDrawViewShell && dynamic_cast<const SdrHint*>(&rHint) )
    {
        SdrHintKind eHintKind = static_cast<const SdrHint&>(rHint).GetKind();

        if ( mnPOCHSmph == 0 && eHintKind == SdrHintKind::PageOrderChange )
        {
            mpDrawViewShell->ResetActualPage();
        }
        else if ( eHintKind == SdrHintKind::LayerChange || eHintKind == SdrHintKind::LayerOrderChange )
        {
            mpDrawViewShell->ResetActualLayer();
        }

        // switch to that page when it's not a master page
        if(SdrHintKind::SwitchToPage == eHintKind)
        {
            const SdrPage* pPage = static_cast<const SdrHint&>(rHint).GetPage();

            if(pPage && !pPage->IsMasterPage())
            {
                if(mpDrawViewShell->GetActualPage() != pPage)
                {
                    sal_uInt16 nPageNum = (pPage->GetPageNum() - 1) / 2; // Sdr --> Sd
                    mpDrawViewShell->SwitchPage(nPageNum);
                }
            }
        }
    }

    ::sd::View::Notify(rBC, rHint);
}

/**
 * Lock/Unlock PageOrderChangedHint
 */

void DrawView::BlockPageOrderChangedHint(bool bBlock)
{
    if (bBlock)
        mnPOCHSmph++;
    else
    {
        DBG_ASSERT(mnPOCHSmph, "counter overflow");
        mnPOCHSmph--;
    }
}

/**
 * If presentation objects are selected, intercept stylesheet-positing at
 * masterpage.
 */

bool DrawView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    bool bResult = true;

    // is there a masterpage edit?
    if (mpDrawViewShell && mpDrawViewShell->GetEditMode() == EditMode::MasterPage)
    {
        if (IsPresObjSelected(false))
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(mpDrawViewShell->GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          SdResId(STR_ACTION_NOTPOSSIBLE)));
            xInfoBox->run();
            bResult = false;
        }
        else
        {
            bResult = ::sd::View::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
        }
    }
    else
    {
        bResult = ::sd::View::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
    }
    return bResult;
}

/**
 * Paint-method: Redirect event to the view
 */

void DrawView::CompleteRedraw(OutputDevice* pOutDev, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0*/)
{
    bool bStandardPaint = true;

    SdDrawDocument* pDoc = mpDocShell->GetDoc();
    if( pDoc && pDoc->GetDocumentType() == DocumentType::Impress)
    {
        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( pDoc ) );
        if(xSlideshow.is() && xSlideshow->isRunning())
        {
            OutputDevice* pShowWindow = xSlideshow->getShowWindow();
            if( (pShowWindow == pOutDev) || (xSlideshow->getAnimationMode() == ANIMATIONMODE_PREVIEW) )
            {
                if( pShowWindow == pOutDev && mpViewSh )
                    xSlideshow->paint();
                bStandardPaint = false;
            }
        }
    }

    if(bStandardPaint)
    {
        ::sd::View::CompleteRedraw(pOutDev, rReg, pRedirector);
    }
}

/**
 * Make passed region visible (scrolling if necessary)
 */

void DrawView::MakeVisible(const ::tools::Rectangle& rRect, vcl::Window& rWin)
{
    if (!rRect.IsEmpty() && mpDrawViewShell)
    {
        mpDrawViewShell->MakeVisible(rRect, rWin);
    }
}

/**
 * Hide page.
 */

void DrawView::HideSdrPage()
{
    if (mpDrawViewShell)
    {
        mpDrawViewShell->HidePage();
    }

    ::sd::View::HideSdrPage();
}

void DrawView::DeleteMarked()
{
    sd::UndoManager* pUndoManager = mrDoc.GetUndoManager();
    DBG_ASSERT( pUndoManager, "sd::DrawView::DeleteMarked(), ui action without undo manager!?" );

    if( pUndoManager )
    {
        OUString aUndo(SvxResId(STR_EditDelete));
        aUndo = aUndo.replaceFirst("%1", GetDescriptionOfMarkedObjects());
        ViewShellId nViewShellId = mpDrawViewShell ? mpDrawViewShell->GetViewShellBase().GetViewShellId() : ViewShellId(-1);
        pUndoManager->EnterListAction(aUndo, aUndo, 0, nViewShellId);
    }

    SdPage* pPage = nullptr;
    bool bResetLayout = false;

    const size_t nMarkCount = GetMarkedObjectList().GetMarkCount();
    if( nMarkCount )
    {
        SdrMarkList aList( GetMarkedObjectList() );
        for (size_t nMark = 0; nMark < nMarkCount; ++nMark)
        {
            SdrObject* pObj = aList.GetMark(nMark)->GetMarkedSdrObj();
            if( pObj && !pObj->IsEmptyPresObj() && pObj->GetUserCall() )
            {
                pPage = static_cast< SdPage* >( pObj->getSdrPageFromSdrObject() );
                if (pPage)
                {
                    PresObjKind ePresObjKind(pPage->GetPresObjKind(pObj));
                    switch( ePresObjKind )
                    {
                    case PRESOBJ_NONE:
                        continue; // ignore it
                    case PRESOBJ_GRAPHIC:
                    case PRESOBJ_OBJECT:
                    case PRESOBJ_CHART:
                    case PRESOBJ_ORGCHART:
                    case PRESOBJ_TABLE:
                    case PRESOBJ_CALC:
                    case PRESOBJ_MEDIA:
                        ePresObjKind = PRESOBJ_OUTLINE;
                        break;
                    default:
                        break;
                    }
                    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                    bool bVertical = pTextObj && pTextObj->IsVerticalWriting();
                    ::tools::Rectangle aRect( pObj->GetLogicRect() );
                    SdrObject* pNewObj = pPage->InsertAutoLayoutShape( nullptr, ePresObjKind, bVertical, aRect, true );

                    // pUndoManager should not be NULL (see assert above)
                    // but since we have defensive code
                    // for it earlier and later in the function
                    // we might as well be consistent
                    if(pUndoManager)
                    {
                        // Move the new PresObj to the position before the
                        // object it will replace.
                        pUndoManager->AddUndoAction(
                            mrDoc.GetSdrUndoFactory().CreateUndoObjectOrdNum(
                                *pNewObj,
                                pNewObj->GetOrdNum(),
                                pObj->GetOrdNum()));
                    }
                    pPage->SetObjectOrdNum( pNewObj->GetOrdNum(), pObj->GetOrdNum() );

                    bResetLayout = true;
                }
            }
        }
    }

    ::sd::View::DeleteMarked();

    if( pPage && bResetLayout )
        pPage->SetAutoLayout( pPage->GetAutoLayout() );

    if( pUndoManager )
        pUndoManager->LeaveListAction();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
