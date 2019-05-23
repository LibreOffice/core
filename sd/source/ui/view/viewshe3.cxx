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

#include <config_features.h>

#include <ViewShell.hxx>
#include <GraphicViewShell.hxx>
#include <GraphicViewShellBase.hxx>

#include <sfx2/viewfrm.hxx>
#include <svtools/strings.hrc>
#include <com/sun/star/lang/Locale.hpp>
#include <svtools/svtresid.hxx>
#include <utility>
#include <vector>

#include <app.hrc>
#include <strings.hrc>

#include <sdabstdlg.hxx>

#include <sal/log.hxx>
#include <fupoor.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdopage.hxx>
#include <sfx2/progress.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdundo.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <tools/multisel.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>
#include <unotools/localedatawrapper.hxx>
#include <rtl/ustrbuf.hxx>
#include <stlsheet.hxx>
#include <WindowUpdater.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <drawview.hxx>

#include <sdattr.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <unoaprms.hxx>
#include <sdundogr.hxx>
#include <Window.hxx>
#include <DrawDocShell.hxx>
#include <FrameView.hxx>
#include <framework/FrameworkHelper.hxx>
#include <optsitem.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <undo/undomanager.hxx>

#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/templdlg.hxx>
#include <svl/aeitem.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>
#include <xmloff/autolayout.hxx>

using namespace ::com::sun::star;

namespace sd {

/**
 * set state (enabled/disabled) of Menu SfxSlots
 */
void  ViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SfxItemState::DEFAULT == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        SfxStyleFamily const nFamily = GetDocSh()->GetStyleFamily();

        SdrView* pDrView = GetDrawView();

        if( pDrView->AreObjectsMarked() )
        {
            SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
            if( pStyleSheet )
            {
                if (pStyleSheet->GetFamily() == SfxStyleFamily::Page)
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                if( pStyleSheet )
                {
                    GetDocSh()->SetStyleFamily(pStyleSheet->GetFamily());
                }
            }
        }

        rSet.Put(SfxUInt16Item(SID_STYLE_FAMILY, static_cast<sal_uInt16>(nFamily)));
    }

    if(SfxItemState::DEFAULT == rSet.GetItemState(SID_GETUNDOSTRINGS))
    {
        ImpGetUndoStrings(rSet);
    }

    if(SfxItemState::DEFAULT == rSet.GetItemState(SID_GETREDOSTRINGS))
    {
        ImpGetRedoStrings(rSet);
    }

    if(SfxItemState::DEFAULT == rSet.GetItemState(SID_UNDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        if(pUndoManager)
        {
            if(pUndoManager->GetUndoActionCount() != 0)
            {
                // If another view created the first undo action, prevent redoing it from this view.
                const SfxUndoAction* pAction = pUndoManager->GetUndoAction();
                if (pAction->GetViewShellId() != GetViewShellBase().GetViewShellId())
                {
                    rSet.Put(SfxUInt32Item(SID_UNDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
                }
                else
                {
                    // Set the necessary string like in
                    // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1072 ff.
                    OUString aTmp(SvtResId(STR_UNDO));
                    aTmp += pUndoManager->GetUndoActionComment();
                    rSet.Put(SfxStringItem(SID_UNDO, aTmp));
                }
            }
            else
            {
                rSet.DisableItem(SID_UNDO);
            }
        }
    }

    if(SfxItemState::DEFAULT != rSet.GetItemState(SID_REDO))
        return;

    SfxUndoManager* pUndoManager = ImpGetUndoManager();
    if(!pUndoManager)
        return;

    if(pUndoManager->GetRedoActionCount() != 0)
    {
        // If another view created the first undo action, prevent redoing it from this view.
        const SfxUndoAction* pAction = pUndoManager->GetRedoAction();
        if (pAction->GetViewShellId() != GetViewShellBase().GetViewShellId())
        {
            rSet.Put(SfxUInt32Item(SID_REDO, static_cast<sal_uInt32>(SID_REPAIRPACKAGE)));
        }
        else
        {
            // Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1081 ff.
            OUString aTmp(SvtResId(STR_REDO));
            aTmp += pUndoManager->GetRedoActionComment();
            rSet.Put(SfxStringItem(SID_REDO, aTmp));
        }
    }
    else
    {
        rSet.DisableItem(SID_REDO);
    }
}

/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* ViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    sal_uInt16 nSId = rRequest.GetSlot();
    SdDrawDocument* pDocument = GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
    SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
    SdrLayerIDSet aVisibleLayers;
    // Determine the page from which to copy some values, such as layers,
    // size, master page, to the new page.  This is usually the given page.
    // When the given page is NULL then use the first page of the document.
    SdPage* pTemplatePage = pPage;
    if (pTemplatePage == nullptr)
        pTemplatePage = pDocument->GetSdPage(0, ePageKind);
    if (pTemplatePage != nullptr && pTemplatePage->TRG_HasMasterPage())
        aVisibleLayers = pTemplatePage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    OUString aStandardPageName;
    OUString aNotesPageName;
    AutoLayout eStandardLayout (AUTOLAYOUT_NONE);
    AutoLayout eNotesLayout (AUTOLAYOUT_NOTES);
    bool bIsPageBack = aVisibleLayers.IsSet(aBckgrnd);
    bool bIsPageObj = aVisibleLayers.IsSet(aBckgrndObj);

    // 1. Process the arguments.
    const SfxItemSet* pArgs = rRequest.GetArgs();
    if (! pArgs)
    {
        // AutoLayouts must be ready
        pDocument->StopWorkStartupDelay();

        // Use the layouts of the previous page and notes page as template.
        if (pTemplatePage != nullptr)
        {
            eStandardLayout = pTemplatePage->GetAutoLayout();
            if( eStandardLayout == AUTOLAYOUT_TITLE )
                eStandardLayout = AUTOLAYOUT_TITLE_CONTENT;

            SdPage* pNotesTemplatePage = static_cast<SdPage*>(pDocument->GetPage(pTemplatePage->GetPageNum()+1));
            if (pNotesTemplatePage != nullptr)
                eNotesLayout = pNotesTemplatePage->GetAutoLayout();
        }
    }
    else if (pArgs->Count() == 1)
    {
        pDocument->StopWorkStartupDelay();
        const SfxUInt32Item* pLayout = rRequest.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYOUT);
        if( pLayout )
        {
            if (ePageKind == PageKind::Notes)
            {
                eNotesLayout   = static_cast<AutoLayout>(pLayout->GetValue ());
            }
            else
            {
                eStandardLayout   = static_cast<AutoLayout>(pLayout->GetValue ());
            }
        }
    }
    else if (pArgs->Count() == 4)
    {
        // AutoLayouts must be ready
        pDocument->StopWorkStartupDelay();

        const SfxStringItem* pPageName = rRequest.GetArg<SfxStringItem>(ID_VAL_PAGENAME);
        const SfxUInt32Item* pLayout = rRequest.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYOUT);
        const SfxBoolItem* pIsPageBack = rRequest.GetArg<SfxBoolItem>(ID_VAL_ISPAGEBACK);
        const SfxBoolItem* pIsPageObj = rRequest.GetArg<SfxBoolItem>(ID_VAL_ISPAGEOBJ);

        if (CHECK_RANGE (AUTOLAYOUT_START, static_cast<AutoLayout>(pLayout->GetValue ()), AUTOLAYOUT_END))
        {
            if (ePageKind == PageKind::Notes)
            {
                aNotesPageName = pPageName->GetValue ();
                eNotesLayout   = static_cast<AutoLayout>(pLayout->GetValue ());
            }
            else
            {
                aStandardPageName = pPageName->GetValue ();
                eStandardLayout   = static_cast<AutoLayout>(pLayout->GetValue ());
            }

            bIsPageBack = pIsPageBack->GetValue ();
            bIsPageObj  = pIsPageObj->GetValue ();
        }
        else
        {
            Cancel();

            if(HasCurrentFunction( SID_BEZIER_EDIT ) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
            rRequest.Ignore ();
            return nullptr;
        }
    }
    else
    {
        Cancel();

        if(HasCurrentFunction(SID_BEZIER_EDIT) )
            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
#if HAVE_FEATURE_SCRIPTING
        StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
        rRequest.Ignore ();
        return nullptr;
    }

    // 2. Create a new page or duplicate an existing one.
    View* pDrView = GetView();
    const bool bUndo = pDrView && pDrView->IsUndoEnabled();
    if( bUndo )
        pDrView->BegUndo(SdResId(STR_INSERTPAGE));

    sal_uInt16 nNewPageIndex = 0xffff;
    switch (nSId)
    {
        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_INSERT_MASTER_PAGE:
            // There are three cases.  a) pPage is not NULL: we use it as a
            // template and create a new slide behind it. b) pPage is NULL
            // but the document is not empty: we use the first slide/notes
            // page as template, create a new slide after it and move it
            // then to the head of the document. c) pPage is NULL and the
            // document is empty: We use CreateFirstPages to create the
            // first page of the document.
            if (pPage == nullptr)
                if (pTemplatePage == nullptr)
                {
                    pDocument->CreateFirstPages();
                    nNewPageIndex = 0;
                }
                else
                {
                    // Create a new page with the first page as template and
                    // insert it after the first page.
                    nNewPageIndex = pDocument->CreatePage (
                        pTemplatePage,
                        ePageKind,
                        aStandardPageName,
                        aNotesPageName,
                        eStandardLayout,
                        eNotesLayout,
                        bIsPageBack,
                        bIsPageObj,
                        nInsertPosition);
                    // Select exactly the new page.
                    sal_uInt16 nPageCount (pDocument->GetSdPageCount(ePageKind));
                    for (sal_uInt16 i=0; i<nPageCount; i++)
                    {
                        pDocument->GetSdPage(i, PageKind::Standard)->SetSelected(
                            i == nNewPageIndex);
                        pDocument->GetSdPage(i, PageKind::Notes)->SetSelected(
                            i == nNewPageIndex);
                    }
                    // Move the selected page to the head of the document
                    pDocument->MovePages (sal_uInt16(-1));
                    nNewPageIndex = 0;
                }
            else
                nNewPageIndex = pDocument->CreatePage (
                    pPage,
                    ePageKind,
                    aStandardPageName,
                    aNotesPageName,
                    eStandardLayout,
                    eNotesLayout,
                    bIsPageBack,
                    bIsPageObj,
                    nInsertPosition);
            break;

        case SID_DUPLICATE_PAGE:
            // Duplication makes no sense when pPage is NULL.
            if (pPage != nullptr)
                nNewPageIndex = pDocument->DuplicatePage (
                    pPage,
                    ePageKind,
                    aStandardPageName,
                    aNotesPageName,
                    bIsPageBack,
                    bIsPageObj,
                    nInsertPosition);
            break;

        default:
            SAL_INFO("sd", "wrong slot id given to CreateOrDuplicatePage");
            // Try to handle another slot id gracefully.
    }
    SdPage* pNewPage = nullptr;
    if(nNewPageIndex != 0xffff)
        pNewPage = pDocument->GetSdPage(nNewPageIndex, PageKind::Standard);

    if( bUndo )
    {
        if( pNewPage )
        {
            pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pNewPage));
            pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pDocument->GetSdPage (nNewPageIndex, PageKind::Notes)));
        }

        pDrView->EndUndo();
    }

    return pNewPage;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
