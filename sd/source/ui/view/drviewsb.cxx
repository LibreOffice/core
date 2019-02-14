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
#include <svx/svdlayer.hxx>
#include <svx/svxids.hrc>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hlnkitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
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

#include <app.hrc>
#include <strings.hrc>

#include <Outliner.hxx>
#include <Window.hxx>
#include <sdmod.hxx>
#include <sdattr.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <unokywds.hxx>
#include <sdpage.hxx>
#include <DrawViewShell.hxx>
#include <drawview.hxx>
#include <unmodpg.hxx>
#include <undolayer.hxx>
#include <ViewShellBase.hxx>
#include <FormShellManager.hxx>
#include <LayerTabBar.hxx>
#include <sdabstdlg.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>

namespace sd {

bool DrawViewShell::RenameSlide( sal_uInt16 nPageId, const OUString & rName  )
{
    bool   bOutDummy;
    if( GetDoc()->GetPageByName( rName, bOutDummy ) != SDRPAGE_NOTFOUND )
        return false;

    SdPage* pPageToRename = nullptr;
    PageKind ePageKind = GetPageKind();

    if( GetEditMode() == EditMode::Page )
    {
        pPageToRename = GetDoc()->GetSdPage( maTabControl->GetPagePos(nPageId), ePageKind );

        // Undo
        SdPage* pUndoPage = pPageToRename;
        SdrLayerAdmin &  rLayerAdmin = GetDoc()->GetLayerAdmin();
        SdrLayerID nBackground = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        SdrLayerID nBgObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        SdrLayerIDSet aVisibleLayers = mpActualPage->TRG_GetMasterPageVisibleLayers();

        SfxUndoManager* pManager = GetDoc()->GetDocSh()->GetUndoManager();
        pManager->AddUndoAction(
            std::make_unique<ModifyPageUndoAction>(
                GetDoc(), pUndoPage, rName, pUndoPage->GetAutoLayout(),
                aVisibleLayers.IsSet( nBackground ),
                aVisibleLayers.IsSet( nBgObj )));

        // rename
        pPageToRename->SetName( rName );

        if( ePageKind == PageKind::Standard )
        {
            // also rename notes-page
            SdPage* pNotesPage = GetDoc()->GetSdPage( maTabControl->GetPagePos(nPageId), PageKind::Notes );
            pNotesPage->SetName( rName );
        }
    }
    else
    {
        // rename MasterPage -> rename LayoutTemplate
        pPageToRename = GetDoc()->GetMasterSdPage( maTabControl->GetPagePos(nPageId), ePageKind );
        GetDoc()->RenameLayoutTemplate( pPageToRename->GetLayoutName(), rName );
    }

    bool bSuccess = (rName == pPageToRename->GetName());

    if( bSuccess )
    {
        // user edited page names may be changed by the page so update control
        maTabControl->SetPageText( nPageId, rName );

        // set document to modified state
        GetDoc()->SetChanged();

        // inform navigator about change
        SfxBoolItem aItem( SID_NAVIGATOR_INIT, true );
        GetViewFrame()->GetDispatcher()->ExecuteList(SID_NAVIGATOR_INIT,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });

        // Tell the slide sorter about the name change (necessary for
        // accessibility.)
        slidesorter::SlideSorterViewShell* pSlideSorterViewShell
            = slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
        if (pSlideSorterViewShell != nullptr)
        {
            pSlideSorterViewShell->GetSlideSorter().GetController().PageNameHasChanged(
                maTabControl->GetPagePos(nPageId), rName);
        }
    }

    return bSuccess;
}

IMPL_LINK( DrawViewShell, RenameSlideHdl, AbstractSvxNameDialog&, rDialog, bool )
{
    OUString aNewName;
    rDialog.GetName( aNewName );

    SdPage* pCurrentPage = GetDoc()->GetSdPage( maTabControl->GetCurPagePos(), GetPageKind() );

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
    if(!GetLayerTabControl()) // #i87182#
    {
        OSL_ENSURE(false, "No LayerTabBar (!)");
        return;
    }

    if( !pLayer )
        return;

    const sal_uInt16 nPageCount = GetLayerTabControl()->GetPageCount();
    sal_uInt16 nCurPage = 0;
    sal_uInt16 nPos;
    for( nPos = 0; nPos < nPageCount; nPos++ )
    {
        sal_uInt16 nId = GetLayerTabControl()->GetPageId( nPos );
        if (GetLayerTabControl()->GetLayerName(nId) == pLayer->GetName())
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

    GetDoc()->SetChanged();

    GetLayerTabControl()->SetPageText(nCurPage, rLayerName);

    // Set page bits for modified tab name display

    TabBarPageBits nBits = TabBarPageBits::NONE;

    if (!bIsVisible)
    {
        nBits = TabBarPageBits::Blue;
    }
    if (bIsLocked)
    {
        nBits |= TabBarPageBits::Italic;
    }
    if (!bIsPrintable)
    {
        nBits |= TabBarPageBits::Underline;
    }

    // Save the bits

    GetLayerTabControl()->SetPageBits(nCurPage, nBits);

    GetViewFrame()->GetDispatcher()->Execute(
        SID_SWITCHLAYER,
        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

    // Call Invalidate at the form shell.
    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell != nullptr)
        pFormShell->Invalidate();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
