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

#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svxids.hrc>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hlnkitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdorect.hxx>
#include <sfx2/docfile.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>
#include <svx/fmshell.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <unotools/useroptions.hxx>
#include <tools/diagnose_ex.h>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "Outliner.hxx"
#include "Window.hxx"
#include "sdmod.hxx"
#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include "unmodpg.hxx"
#include "undolayer.hxx"
#include "ViewShellBase.hxx"
#include "FormShellManager.hxx"
#include "LayerTabBar.hxx"
#include "sdabstdlg.hxx"
#include "SlideSorterViewShell.hxx"
#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"

namespace sd {

bool DrawViewShell::RenameSlide( sal_uInt16 nPageId, const OUString & rName  )
{
    sal_Bool   bOutDummy;
    if( GetDoc()->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
        return false;

    SdPage* pPageToRename = NULL;
    PageKind ePageKind = GetPageKind();

    if( GetEditMode() == EM_PAGE )
    {
        pPageToRename = GetDoc()->GetSdPage( nPageId - 1, ePageKind );

        // Undo
        SdPage* pUndoPage = pPageToRename;
        SdrLayerAdmin &  rLayerAdmin = GetDoc()->GetLayerAdmin();
        sal_uInt8 nBackground = rLayerAdmin.GetLayerID( SD_RESSTR(STR_LAYER_BCKGRND), sal_False );
        sal_uInt8 nBgObj = rLayerAdmin.GetLayerID( SD_RESSTR(STR_LAYER_BCKGRNDOBJ), sal_False );
        SetOfByte aVisibleLayers = mpActualPage->TRG_GetMasterPageVisibleLayers();

        ::svl::IUndoManager* pManager = GetDoc()->GetDocSh()->GetUndoManager();
        ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
            GetDoc(), pUndoPage, rName, pUndoPage->GetAutoLayout(),
            aVisibleLayers.IsSet( nBackground ),
            aVisibleLayers.IsSet( nBgObj ));
        pManager->AddUndoAction( pAction );

        // rename
        pPageToRename->SetName( rName );

        if( ePageKind == PK_STANDARD )
        {
            // also rename notes-page
            SdPage* pNotesPage = GetDoc()->GetSdPage( nPageId - 1, PK_NOTES );
            pNotesPage->SetName( rName );
        }
    }
    else
    {
        // rename MasterPage -> rename LayoutTemplate
        pPageToRename = GetDoc()->GetMasterSdPage( nPageId - 1, ePageKind );
        GetDoc()->RenameLayoutTemplate( pPageToRename->GetLayoutName(), rName );
    }

    bool bSuccess = (rName == pPageToRename->GetName());

    if( bSuccess )
    {
        // user edited page names may be changed by the page so update control
        maTabControl.SetPageText( nPageId, rName );

        // set document to modified state
        GetDoc()->SetChanged( sal_True );

        // inform navigator about change
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, sal_True );
        GetViewFrame()->GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

        // Tell the slide sorter about the name change (necessary for
        // accessibility.)
        slidesorter::SlideSorterViewShell* pSlideSorterViewShell
            = slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
        if (pSlideSorterViewShell != NULL)
        {
            pSlideSorterViewShell->GetSlideSorter().GetController().PageNameHasChanged(
                nPageId-1, rName);
        }
    }

    return bSuccess;
}




IMPL_LINK( DrawViewShell, RenameSlideHdl, AbstractSvxNameDialog*, pDialog )
{
    if( ! pDialog )
        return 0;

    OUString aNewName;
    pDialog->GetName( aNewName );

    SdPage* pCurrentPage = GetDoc()->GetSdPage( maTabControl.GetCurPageId() - 1, GetPageKind() );

    return pCurrentPage && ( aNewName == pCurrentPage->GetName() || GetDocSh()->IsNewPageNameValid( aNewName ) );
}




void DrawViewShell::ModifyLayer (
    SdrLayer* pLayer,
    const OUString& rLayerName,
    const OUString& rLayerTitle,
    const OUString& rLayerDesc,
    bool bIsVisible,
    bool bIsLocked,
    bool bIsPrintable)
{
    if( pLayer )
    {
        const sal_uInt16 nPageCount = GetLayerTabControl()->GetPageCount();
        sal_uInt16 nCurPage = 0;
        sal_uInt16 nPos;
        for( nPos = 0; nPos < nPageCount; nPos++ )
        {
            sal_uInt16 nId = GetLayerTabControl()->GetPageId( nPos );
            if (GetLayerTabControl()->GetPageText(nId).equals(pLayer->GetName()))
            {
                nCurPage = nId;
                break;
            }
        }

        pLayer->SetName( rLayerName );
        pLayer->SetTitle( rLayerTitle );
        pLayer->SetDescription( rLayerDesc );
        mpDrawView->SetLayerVisible( rLayerName, bIsVisible );
        mpDrawView->SetLayerLocked( rLayerName, bIsLocked);
        mpDrawView->SetLayerPrintable(rLayerName, bIsPrintable);

        GetDoc()->SetChanged(sal_True);

        GetLayerTabControl()->SetPageText(nCurPage, rLayerName);

        TabBarPageBits nBits = 0;

        if (!bIsVisible)
        {
            // invisible layers are presented different
            nBits = TPB_SPECIAL;
        }

        GetLayerTabControl()->SetPageBits(nCurPage, nBits);

        GetViewFrame()->GetDispatcher()->Execute(
            SID_SWITCHLAYER,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

        // Call Invalidate at the form shell.
        FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
        if (pFormShell != NULL)
            pFormShell->Invalidate();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
