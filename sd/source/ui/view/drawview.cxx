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
#include <vcl/msgbox.hxx>
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

#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include "stlsheet.hxx"

#include <svx/svdoutl.hxx>
#undef BMP_OLEOBJ
#include <svx/svdstr.hrc>
#include <svx/dialmgr.hxx>

#include "glob.hrc"
#include "strings.hrc"
#include "View.hxx"
#include "sdattr.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "pres.hxx"
#include "sdresid.hxx"
#include "Window.hxx"
#include "unchss.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "anminfo.hxx"
#include "slideshow.hxx"
#include <vcl/virdev.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

#include "undo/undomanager.hxx"

using namespace ::com::sun::star;

namespace sd {

TYPEINIT1(DrawView, View);

/**
 * Shows the first page of document at position 0,0. In the case
 * that there is no page a page is created.
 */

DrawView::DrawView( DrawDocShell* pDocSh, OutputDevice* pOutDev, DrawViewShell* pShell)
: ::sd::View(*pDocSh->GetDoc(), pOutDev, pShell)
, mpDocShell(pDocSh)
, mpDrawViewShell(pShell)
, mpVDev(NULL)
, mnPOCHSmph(0)
{
    SetCurrentObj(OBJ_RECT, SdrInventor);
}

DrawView::~DrawView()
{
    delete mpVDev;
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
    pSSPool->Broadcast(SfxStyleSheetPoolHint(SFX_STYLESHEETPOOL_CHANGES));

    if( mpDrawViewShell )
        mpDrawViewShell->ModelHasChanged();

}

/**
 * Redirect attributes onto title and outline text and background
 * rectangle of a masterpage into templates, otherwise pass on baseclass.
 */

sal_Bool DrawView::SetAttributes(const SfxItemSet& rSet,
                                            sal_Bool bReplaceAll)
{
    sal_Bool bOk = sal_False;

    // is there a masterpage edit?
    if ( mpDrawViewShell && mpDrawViewShell->GetEditMode() == EM_MASTERPAGE )
    {
        SfxStyleSheetBasePool* pStShPool = mrDoc.GetStyleSheetPool();
        SdPage& rPage = *mpDrawViewShell->getCurrentPage();
        String aLayoutName = rPage.GetName();
        SdrTextObj* pEditObject = static_cast< SdrTextObj* >( GetTextEditObject() );

        if (pEditObject)
        {
            // Textedit
            String aTemplateName(aLayoutName);

            sal_uInt32 nInv = pEditObject->GetObjInventor();

            if (nInv == SdrInventor)
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
                    StyleSheetUndoAction* pAction = new StyleSheetUndoAction(&mrDoc, pSheet, &aTempSet);
                    mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                    pSheet->GetItemSet().Put(aTempSet);
                    pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    bOk = sal_True;
                }
                else if (eObjKind == OBJ_OUTLINETEXT)
                {
                    // Presentation object outline
                    OutlinerView* pOV   = GetTextEditOutlinerView();
                    ::Outliner* pOutliner = pOV->GetOutliner();

                    aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));

                    pOutliner->SetUpdateMode(sal_False);
                    mpDocSh->SetWaitCursor( sal_True );

                    // replace placeholder by template name
                    String aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
                    xub_StrLen nPos = aComment.Search( (sal_Unicode)'$' );
                    aComment.Erase(nPos, 1);
                    aComment.Insert( String((SdResId(STR_PSEUDOSHEET_OUTLINE))), nPos);
                    mpDocSh->GetUndoManager()->EnterListAction( aComment, String() );

                    std::vector<Paragraph*> aSelList;
                    pOV->CreateSelectionList(aSelList);

                    std::vector<Paragraph*>::reverse_iterator iter = aSelList.rbegin();
                    Paragraph* pPara = iter != aSelList.rend() ? *iter : NULL;

                    while (pPara)
                    {
                        sal_uLong nParaPos = pOutliner->GetAbsPos( pPara );
                        sal_Int16 nDepth = pOutliner->GetDepth( (sal_uInt16) nParaPos );
                        String aName(rPage.GetLayoutName());
                        aName += (sal_Unicode)(' ');
                        aName += OUString::number( (nDepth <= 0) ? 1 : nDepth + 1 );
                        SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
                        DBG_ASSERT(pSheet, "StyleSheet not found");

                        SfxItemSet aTempSet( pSheet->GetItemSet() );
                        aTempSet.Put( rSet );
                        aTempSet.ClearInvalidItems();

                        if( nDepth > 0 && aTempSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
                        {
                            // no SvxNumBulletItem in outline level 1 to 8!
                            aTempSet.ClearItem( EE_PARA_NUMBULLET );
                        }

                        // Undo-Action
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(&mrDoc, pSheet, &aTempSet);
                        mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));

                        // now also broadcast any child sheets
                        sal_Int16 nChild;
                        for( nChild = nDepth + 1; nChild < 9; nChild++ )
                        {
                            String aSheetName(rPage.GetLayoutName());
                            aSheetName += (sal_Unicode)(' ');
                            aSheetName += OUString::number( nChild <= 0 ? 1 : nChild + 1 );
                            SfxStyleSheet* pOutlSheet = static_cast< SfxStyleSheet* >(pStShPool->Find(aSheetName, SD_STYLE_FAMILY_MASTERPAGE));

                            if( pOutlSheet )
                                pOutlSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        }

                        ++iter;
                        pPara = iter != aSelList.rend() ? *iter : NULL;

                        if( !pPara && nDepth > 0 &&  rSet.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON &&
                            pOutliner->GetDepth( (sal_uInt16) pOutliner->GetAbsPos(*(aSelList.begin())) ) > 0 )
                            pPara = pOutliner->GetParagraph( 0 );  // Put NumBulletItem in outline level 1
                    }

                    mpDocSh->SetWaitCursor( sal_False );
                    pOV->GetOutliner()->SetUpdateMode(sal_True);

                    mpDocSh->GetUndoManager()->LeaveListAction();

                    bOk = sal_True;
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
            sal_uLong nMarkCount         = rList.GetMarkCount();
            for (sal_uLong nMark = 0; nMark < nMarkCount; nMark++)
            {
                SdrObject* pObject = rList.GetMark(nMark)->GetMarkedSdrObj();
                sal_uInt32 nInv = pObject->GetObjInventor();

                if (nInv == SdrInventor)
                {
                    sal_uInt16 eObjKind = pObject->GetObjIdentifier();
                    PresObjKind ePresObjKind = rPage.GetPresObjKind(pObject);
                    String aTemplateName(aLayoutName);

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
                        StyleSheetUndoAction* pAction = new StyleSheetUndoAction(&mrDoc, pSheet, &aTempSet);
                        mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                        pSheet->GetItemSet().Put(aTempSet,false);
                        pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        bOk = sal_True;
                    }
                    else if (eObjKind == OBJ_OUTLINETEXT)
                    {
                        // Presentation object outline
                        aTemplateName += String(SdResId(STR_LAYOUT_OUTLINE));
                        for (sal_uInt16 nLevel = 9; nLevel > 0; nLevel--)
                        {
                            String aName(rPage.GetLayoutName());
                            aName += (sal_Unicode)(' ');
                            aName += OUString::number( (sal_Int32)nLevel );
                            SfxStyleSheet* pSheet = (SfxStyleSheet*)pStShPool->
                                                Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
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
                                    if( SFX_ITEM_ON == rSet.GetItemState( nWhich ) )
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
                            StyleSheetUndoAction* pAction = new StyleSheetUndoAction(&mrDoc, pSheet, &aTempSet);
                            mpDocSh->GetUndoManager()->AddUndoAction(pAction);

                            pSheet->GetItemSet().Set(aTempSet,false);
                            pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                        }

                        // remove all hard set items from shape that are now set in style
                        SfxWhichIter aWhichIter(rSet);
                        sal_uInt16 nWhich(aWhichIter.FirstWhich());
                        while( nWhich )
                        {
                            if( SFX_ITEM_ON == rSet.GetItemState( nWhich ) )
                                pObject->ClearMergedItem( nWhich );
                            nWhich = aWhichIter.NextWhich();
                        }

                        bOk = sal_True;
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

    return (bOk);
}

/**
 * Notify for change of site arragement
 */

void DrawView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( mpDrawViewShell && rHint.ISA(SdrHint) )
    {
        SdrHintKind eHintKind = ( (SdrHint&) rHint).GetKind();

        if ( mnPOCHSmph == 0 && eHintKind == HINT_PAGEORDERCHG )
        {
            mpDrawViewShell->ResetActualPage();
        }
        else if ( eHintKind == HINT_LAYERCHG || eHintKind == HINT_LAYERORDERCHG )
        {
            mpDrawViewShell->ResetActualLayer();
        }

        // switch to that page when it's not a master page
        if(HINT_SWITCHTOPAGE == eHintKind)
        {
            const SdrPage* pPage = ((const SdrHint&)rHint).GetPage();

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

void DrawView::BlockPageOrderChangedHint(sal_Bool bBlock)
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

sal_Bool DrawView::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    sal_Bool bResult = sal_True;

    // is there a masterpage edit?
    if (mpDrawViewShell && mpDrawViewShell->GetEditMode() == EM_MASTERPAGE)
    {
        if (IsPresObjSelected(sal_False, sal_True))
        {

            InfoBox(mpDrawViewShell->GetActiveWindow(),
                    String(SdResId(STR_ACTION_NOTPOSSIBLE))).Execute();
            bResult = sal_False;
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

void DrawView::CompleteRedraw(OutputDevice* pOutDev, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector /*=0L*/)
{
    if( mpVDev )
    {
        delete mpVDev;
        mpVDev = NULL;
    }

    sal_Bool bStandardPaint = sal_True;

    SdDrawDocument* pDoc = mpDocShell->GetDoc();
    if( pDoc && pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS)
    {
        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( pDoc ) );
        if(xSlideshow.is() && xSlideshow->isRunning())
        {
            OutputDevice* pShowWindow = ( OutputDevice* )xSlideshow->getShowWindow();
            if( (pShowWindow == pOutDev) || (xSlideshow->getAnimationMode() == ANIMATIONMODE_PREVIEW) )
            {
                if( pShowWindow == pOutDev )
                    PresPaint(rReg);
                bStandardPaint = sal_False;
            }
        }
    }

    if(bStandardPaint)
    {
        ::sd::View::CompleteRedraw(pOutDev, rReg, pRedirector);
    }
}

/**
 * Paint-Event during running slide show
 */

void DrawView::PresPaint(const Region& rRegion)
{
    if(mpViewSh)
    {
        rtl::Reference< SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if( xSlideshow.is() && xSlideshow->isRunning() )
            xSlideshow->paint( rRegion.GetBoundRect() );
    }
}

/**
 * Decides if an object could get marked (eg. unreleased animation objects
 * in slide show).
 */

sal_Bool DrawView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    return FmFormView::IsObjMarkable(pObj, pPV);
}

/**
 * Make passed region visible (scrolling if necessary)
 */

void DrawView::MakeVisible(const Rectangle& rRect, ::Window& rWin)
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
    OSL_TRACE( "DrawView::DeleteMarked() - enter" );

    sd::UndoManager* pUndoManager = mrDoc.GetUndoManager();
    DBG_ASSERT( pUndoManager, "sd::DrawView::DeleteMarked(), ui action without undo manager!?" );

    if( pUndoManager )
    {
        String aUndo( SVX_RES(STR_EditDelete) );
        String aSearchString(RTL_CONSTASCII_USTRINGPARAM("%1"));
        aUndo.SearchAndReplace(aSearchString, GetDescriptionOfMarkedObjects());
        pUndoManager->EnterListAction(aUndo, aUndo);
    }

    SdPage* pPage = 0;
    bool bResetLayout = false;

    const sal_uLong nMarkCount = GetMarkedObjectList().GetMarkCount();
    if( nMarkCount )
    {
        SdrMarkList aList( GetMarkedObjectList() );
        for (sal_uLong nMark = 0; nMark < nMarkCount; nMark++)
        {
            SdrObject* pObj = aList.GetMark(nMark)->GetMarkedSdrObj();
            if( pObj && !pObj->IsEmptyPresObj() && pObj->GetUserCall() )
            {
                pPage = static_cast< SdPage* >( pObj->GetPage() );
                PresObjKind ePresObjKind;
                if( pPage && ((ePresObjKind = pPage->GetPresObjKind(pObj)) != PRESOBJ_NONE))
                {
                    switch( ePresObjKind )
                    {
                    case PRESOBJ_GRAPHIC:
                    case PRESOBJ_OBJECT:
                    case PRESOBJ_CHART:
                    case PRESOBJ_ORGCHART:
                    case PRESOBJ_TABLE:
                    case PRESOBJ_CALC:
                    case PRESOBJ_IMAGE:
                    case PRESOBJ_MEDIA:
                        ePresObjKind = PRESOBJ_OUTLINE;
                        break;
                    default:
                        break;
                    }
                    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );
                    bool bVertical = pTextObj && pTextObj->IsVerticalWriting();
                    Rectangle aRect( pObj->GetLogicRect() );
                    SdrObject* pNewObj = pPage->InsertAutoLayoutShape( 0, ePresObjKind, bVertical, aRect, true );

                    pPage->SetObjectOrdNum( pNewObj->GetOrdNum(), pObj->GetOrdNum() );

                    bResetLayout = true;

                    OSL_TRACE( "DrawView::InsertAutoLayoutShape() - InsertAutoLayoutShape" );
                }
            }
        }
    }

    ::sd::View::DeleteMarked();

    if( pPage && bResetLayout )
        pPage->SetAutoLayout( pPage->GetAutoLayout() );

    if( pUndoManager )
        pUndoManager->LeaveListAction();

    OSL_TRACE( "DrawView::InsertAutoLayoutShape() - leave" );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
