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

#include <fusearch.hxx>

#include <sfx2/viewfrm.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/bindings.hxx>
#include <fupoor.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <app.hrc>
#include <sdmod.hxx>
#include <View.hxx>
#include <Outliner.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <ViewShellBase.hxx>

class SfxRequest;

namespace sd {

static const sal_uInt16 SidArraySpell[] = {
            SID_DRAWINGMODE,
            SID_OUTLINE_MODE,
            SID_SLIDE_SORTER_MODE,
            SID_NOTES_MODE,
            SID_HANDOUT_MASTER_MODE,
            SID_SLIDE_MASTER_MODE,
            SID_NOTES_MASTER_MODE,
            0 };

FuSearch::FuSearch (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      pSdOutliner(nullptr),
      bOwnOutliner(false)
{
}

rtl::Reference<FuPoor> FuSearch::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuSearch( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSearch::DoExecute( SfxRequest& )
{
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if ( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr )
    {
        bOwnOutliner = true;
        pSdOutliner = new SdOutliner( mpDoc, OutlinerMode::TextObject );
    }
    else if ( dynamic_cast< const OutlineViewShell *>( mpViewShell ) !=  nullptr )
    {
        bOwnOutliner = false;
        pSdOutliner = mpDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}

FuSearch::~FuSearch()
{
    if ( ! mpDocSh->IsInDestruction() && mpDocSh->GetViewShell()!=nullptr)
        mpDocSh->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if (pSdOutliner)
        pSdOutliner->EndSpelling();

    if (bOwnOutliner)
        delete pSdOutliner;
}

void FuSearch::SearchAndReplace( const SvxSearchItem* pSearchItem )
{
    ViewShellBase* pBase = dynamic_cast<ViewShellBase*>( SfxViewShell::Current() );
    ViewShell* pViewShell = nullptr;
    if (pBase != nullptr)
        pViewShell = pBase->GetMainViewShell().get();

    if (pViewShell == nullptr)
        return;

    if ( pSdOutliner && dynamic_cast< const DrawViewShell *>( pViewShell ) !=  nullptr && !bOwnOutliner )
    {
        pSdOutliner->EndSpelling();

        bOwnOutliner = true;
        pSdOutliner = new SdOutliner( mpDoc, OutlinerMode::TextObject );
        pSdOutliner->PrepareSpelling();
    }
    else if ( pSdOutliner && dynamic_cast< const OutlineViewShell *>( pViewShell ) !=  nullptr && bOwnOutliner )
    {
        pSdOutliner->EndSpelling();
        delete pSdOutliner;

        bOwnOutliner = false;
        pSdOutliner = mpDoc->GetOutliner();
        pSdOutliner->PrepareSpelling();
    }

    if (pSdOutliner)
    {
        bool bEndSpelling = pSdOutliner->StartSearchAndReplace(pSearchItem);

        if (bEndSpelling)
        {
            pSdOutliner->EndSpelling();
            pSdOutliner->PrepareSpelling();
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
