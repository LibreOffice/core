/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sd.hxx"


#include "ViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "GraphicViewShellBase.hxx"

#include <sfx2/viewfrm.hxx>
#include <com/sun/star/lang/Locale.hpp>

#include <utility>
#include <vector>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "sdabstdlg.hxx"

#include "fupoor.hxx"
#include <sfx2/dispatch.hxx>
#include <svx/prtqry.hxx>
#include <svx/svdopage.hxx>
#include <sfx2/progress.hxx>
#include <svx/svdobj.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdetc.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <tools/multisel.hxx>
#include <svl/intitem.hxx>
#include <svl/style.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include "stlsheet.hxx"
#include "WindowUpdater.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "drawview.hxx"

#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#include "Window.hxx"
#include "DrawDocShell.hxx"
#include "FrameView.hxx"
#include "framework/FrameworkHelper.hxx"
#include "optsitem.hxx"
#include "sdresid.hxx"

#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <basic/sbstar.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

namespace sd {

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void  ViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        UINT16 nFamily = (UINT16)GetDocSh()->GetStyleFamily();

        SdrView* pDrView = GetDrawView();

        if( pDrView->AreObjectsMarked() )
        {
            SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
            if( pStyleSheet )
            {
                if (pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                if( pStyleSheet )
                {
                    SfxStyleFamily eFamily = pStyleSheet->GetFamily();
                    if(eFamily == SD_STYLE_FAMILY_GRAPHICS)
                        nFamily = 2;
                    else if(eFamily == SD_STYLE_FAMILY_CELL )
                        nFamily = 3;
                    else // SD_STYLE_FAMILY_PSEUDO
                        nFamily = 5;

                    GetDocSh()->SetStyleFamily(nFamily);
                }
            }
        }
        rSet.Put(SfxUInt16Item(SID_STYLE_FAMILY, nFamily ));
    }

    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETUNDOSTRINGS))
    {
        ImpGetUndoStrings(rSet);
    }

    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETREDOSTRINGS))
    {
        ImpGetRedoStrings(rSet);
    }

    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_UNDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetUndoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1072 ff.
            String aTmp(ResId(STR_UNDO, *SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetUndoActionComment(0);
            rSet.Put(SfxStringItem(SID_UNDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_UNDO);
        }
    }

    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_REDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetRedoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1081 ff.
            String aTmp(ResId(STR_REDO, *SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetRedoActionComment(0);
            rSet.Put(SfxStringItem(SID_REDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_REDO);
        }
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
    SdPage* pPage)
{
    USHORT nSId = rRequest.GetSlot();
    SdDrawDocument* pDocument = GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
    SetOfByte aVisibleLayers;
    // Determine the page from which to copy some values, such as layers,
    // size, master page, to the new page.  This is usually the given page.
    // When the given page is NULL then use the first page of the document.
    SdPage* pTemplatePage = pPage;
    if (pTemplatePage == NULL)
        if (pDocument->GetSdPage(0, ePageKind) > 0)
            pTemplatePage = pDocument->GetSdPage(0, ePageKind);
    if (pTemplatePage != NULL && pTemplatePage->TRG_HasMasterPage())
        aVisibleLayers = pTemplatePage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    String aStandardPageName;
    String aNotesPageName;
    AutoLayout eStandardLayout (AUTOLAYOUT_NONE);
    AutoLayout eNotesLayout (AUTOLAYOUT_NOTES);
    BOOL bIsPageBack = aVisibleLayers.IsSet(aBckgrnd);
    BOOL bIsPageObj = aVisibleLayers.IsSet(aBckgrndObj);

    // 1. Process the arguments.
    const SfxItemSet* pArgs = rRequest.GetArgs();
    if (! pArgs)
    {
/*
        // Make the layout menu visible in the tool pane.
        const ViewShellBase& rBase (GetViewShellBase());
        if (rBase.GetMainViewShell()!=NULL
            && rBase.GetMainViewShell()->GetShellType()!=ViewShell::ST_OUTLINE
            && rBase.GetMainViewShell()->GetShellType()!=ViewShell::ST_DRAW)
        {
            framework::FrameworkHelper::Instance(GetViewShellBase())->RequestTaskPanel(
                framework::FrameworkHelper::msLayoutTaskPanelURL);
        }
*/

        // AutoLayouts muessen fertig sein
        pDocument->StopWorkStartupDelay();

        // Use the layouts of the previous page and notes page as template.
        if (pTemplatePage != NULL)
        {
            eStandardLayout = pTemplatePage->GetAutoLayout();
            if( eStandardLayout == AUTOLAYOUT_TITLE )
                eStandardLayout = AUTOLAYOUT_ENUM;

            SdPage* pNotesTemplatePage = static_cast<SdPage*>(pDocument->GetPage(pTemplatePage->GetPageNum()+1));
            if (pNotesTemplatePage != NULL)
                eNotesLayout = pNotesTemplatePage->GetAutoLayout();
        }
    }
    else if (pArgs->Count() == 1)
    {
        pDocument->StopWorkStartupDelay();
        SFX_REQUEST_ARG (rRequest, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
        if( pLayout )
        {
            if (ePageKind == PK_NOTES)
            {
                eNotesLayout   = (AutoLayout) pLayout->GetValue ();
            }
            else
            {
                eStandardLayout   = (AutoLayout) pLayout->GetValue ();
            }
        }
    }
    else if (pArgs->Count() == 4)
    {
        // AutoLayouts muessen fertig sein
        pDocument->StopWorkStartupDelay();

        SFX_REQUEST_ARG (rRequest, pPageName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
        SFX_REQUEST_ARG (rRequest, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
        SFX_REQUEST_ARG (rRequest, pIsPageBack, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
        SFX_REQUEST_ARG (rRequest, pIsPageObj, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);

        if (CHECK_RANGE (AUTOLAYOUT__START, (AutoLayout) pLayout->GetValue (), AUTOLAYOUT__END))
        {
            if (ePageKind == PK_NOTES)
            {
                aNotesPageName = pPageName->GetValue ();
                eNotesLayout   = (AutoLayout) pLayout->GetValue ();
            }
            else
            {
                aStandardPageName = pPageName->GetValue ();
                eStandardLayout   = (AutoLayout) pLayout->GetValue ();
            }

            bIsPageBack = pIsPageBack->GetValue ();
            bIsPageObj  = pIsPageObj->GetValue ();
        }
        else
        {
            Cancel();

            if(HasCurrentFunction( SID_BEZIER_EDIT ) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
            rRequest.Ignore ();
            return NULL;
        }
    }
    else
    {
        Cancel();

        if(HasCurrentFunction(SID_BEZIER_EDIT) )
            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

        StarBASIC::FatalError (SbERR_WRONG_ARGS);
        rRequest.Ignore ();
        return NULL;
    }

    // 2. Create a new page or duplicate an existing one.
    View* pDrView = GetView();
    const bool bUndo = pDrView && pDrView->IsUndoEnabled();
    if( bUndo )
        pDrView->BegUndo( String( SdResId(STR_INSERTPAGE) ) );

    USHORT nNewPageIndex = 0xffff;
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
            if (pPage == NULL)
                if (pTemplatePage == NULL)
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
                        bIsPageObj);
                    // Select exactly the new page.
                    USHORT nPageCount (pDocument->GetSdPageCount(ePageKind));
                    for (USHORT i=0; i<nPageCount; i++)
                    {
                        pDocument->GetSdPage(i, PK_STANDARD)->SetSelected(
                            i == nNewPageIndex);
                        pDocument->GetSdPage(i, PK_NOTES)->SetSelected(
                            i == nNewPageIndex);
                    }
                    // Move the selected page to the head of the document
                    pDocument->MovePages ((USHORT)-1);
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
                    bIsPageObj);
            break;

        case SID_DUPLICATE_PAGE:
            // Duplication makes no sense when pPage is NULL.
            if (pPage != NULL)
                nNewPageIndex = pDocument->DuplicatePage (
                    pPage,
                    ePageKind,
                    aStandardPageName,
                    aNotesPageName,
                    eStandardLayout,
                    eNotesLayout,
                    bIsPageBack,
                    bIsPageObj);
            break;

        default:
            DBG_WARNING("wrong slot id given to CreateOrDuplicatePage");
            // Try to handle another slot id gracefully.
    }
    SdPage* pNewPage = 0;
    if(nNewPageIndex != 0xffff)
        pNewPage = pDocument->GetSdPage(nNewPageIndex, PK_STANDARD);

    if( bUndo )
    {
        if( pNewPage )
        {
            pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pNewPage));
            pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pDocument->GetSdPage (nNewPageIndex, PK_NOTES)));
        }

        pDrView->EndUndo();
    }

    return pNewPage;
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
