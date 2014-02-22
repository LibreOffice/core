/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "GraphicViewShellBase.hxx"

#include <sfx2/viewfrm.hxx>
#include <svtools/svtools.hrc>
#include <com/sun/star/lang/Locale.hpp>
#include <svtools/svtresid.hxx>
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
#include "unoaprms.hxx"
#include "sdundogr.hxx"
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

/**
 * set state (enabled/disabled) of Menu SfxSlots
 */
void  ViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        sal_uInt16 nFamily = (sal_uInt16)GetDocSh()->GetStyleFamily();

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
                    else 
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
        ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(sal_False);

        if(pUndoManager)
        {
            if(pUndoManager->GetUndoActionCount() != 0)
            {
                bActivate = sal_True;
            }
        }

        if(bActivate)
        {
            
            
            OUString aTmp(SVT_RESSTR(STR_UNDO));
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
        ::svl::IUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(sal_False);

        if(pUndoManager)
        {
            if(pUndoManager->GetRedoActionCount() != 0)
            {
                bActivate = sal_True;
            }
        }

        if(bActivate)
        {
            
            
            OUString aTmp(SVT_RESSTR(STR_REDO));
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
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    sal_uInt16 nSId = rRequest.GetSlot();
    SdDrawDocument* pDocument = GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
    SetOfByte aVisibleLayers;
    
    
    
    SdPage* pTemplatePage = pPage;
    if (pTemplatePage == NULL)
        pTemplatePage = pDocument->GetSdPage(0, ePageKind);
    if (pTemplatePage != NULL && pTemplatePage->TRG_HasMasterPage())
        aVisibleLayers = pTemplatePage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    OUString aStandardPageName;
    OUString aNotesPageName;
    AutoLayout eStandardLayout (AUTOLAYOUT_NONE);
    AutoLayout eNotesLayout (AUTOLAYOUT_NOTES);
    sal_Bool bIsPageBack = aVisibleLayers.IsSet(aBckgrnd);
    sal_Bool bIsPageObj = aVisibleLayers.IsSet(aBckgrndObj);

    
    const SfxItemSet* pArgs = rRequest.GetArgs();
    if (! pArgs)
    {
        
        pDocument->StopWorkStartupDelay();

        
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
        SFX_REQUEST_ARG (rRequest, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, false);
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
        
        pDocument->StopWorkStartupDelay();

        SFX_REQUEST_ARG (rRequest, pPageName, SfxStringItem, ID_VAL_PAGENAME, false);
        SFX_REQUEST_ARG (rRequest, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, false);
        SFX_REQUEST_ARG (rRequest, pIsPageBack, SfxBoolItem, ID_VAL_ISPAGEBACK, false);
        SFX_REQUEST_ARG (rRequest, pIsPageObj, SfxBoolItem, ID_VAL_ISPAGEOBJ, false);

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
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
            rRequest.Ignore ();
            return NULL;
        }
    }
    else
    {
        Cancel();

        if(HasCurrentFunction(SID_BEZIER_EDIT) )
            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
#ifndef DISABLE_SCRIPTING
        StarBASIC::FatalError (SbERR_WRONG_ARGS);
#endif
        rRequest.Ignore ();
        return NULL;
    }

    
    View* pDrView = GetView();
    const bool bUndo = pDrView && pDrView->IsUndoEnabled();
    if( bUndo )
        pDrView->BegUndo(SD_RESSTR(STR_INSERTPAGE));

    sal_uInt16 nNewPageIndex = 0xffff;
    switch (nSId)
    {
        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_INSERT_MASTER_PAGE:
            
            
            
            
            
            
            
            if (pPage == NULL)
                if (pTemplatePage == NULL)
                {
                    pDocument->CreateFirstPages();
                    nNewPageIndex = 0;
                }
                else
                {
                    
                    
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
                    
                    sal_uInt16 nPageCount (pDocument->GetSdPageCount(ePageKind));
                    for (sal_uInt16 i=0; i<nPageCount; i++)
                    {
                        pDocument->GetSdPage(i, PK_STANDARD)->SetSelected(
                            i == nNewPageIndex);
                        pDocument->GetSdPage(i, PK_NOTES)->SetSelected(
                            i == nNewPageIndex);
                    }
                    
                    pDocument->MovePages ((sal_uInt16)-1);
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
            
            if (pPage != NULL)
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
            DBG_WARNING("wrong slot id given to CreateOrDuplicatePage");
            
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


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
