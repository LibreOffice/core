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

#include "fusearch.hxx"

#include <sfx2/viewfrm.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/bindings.hxx>
#include "fupoor.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "app.hrc"
#include "app.hxx"
#include "View.hxx"
#include "Outliner.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "ViewShellBase.hxx"

class SfxRequest;

namespace sd {

static sal_uInt16 SidArraySpell[] = {
            SID_DRAWINGMODE,
            SID_OUTLINEMODE,
            SID_DIAMODE,
            SID_NOTESMODE,
            SID_HANDOUTMODE,
            0 };

TYPEINIT1( FuSearch, FuPoor );

FuSearch::FuSearch (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq),
      pSdOutliner(NULL),
      bOwnOutliner(sal_False)
{
}

FunctionReference FuSearch::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSearch( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSearch::DoExecute( SfxRequest& )
{
    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if ( mpViewShell->ISA(DrawViewShell) )
    {
        bOwnOutliner = sal_True;
        pSdOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
    }
    else if ( mpViewShell->ISA(OutlineViewShell) )
    {
        bOwnOutliner = sal_False;
        pSdOutliner = mpDoc->GetOutliner();
    }

    if (pSdOutliner)
       pSdOutliner->PrepareSpelling();
}

FuSearch::~FuSearch()
{
    if ( ! mpDocSh->IsInDestruction() && mpDocSh->GetViewShell()!=NULL)
        mpDocSh->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( SidArraySpell );

    if (pSdOutliner)
        pSdOutliner->EndSpelling();

    if (bOwnOutliner)
        delete pSdOutliner;
}


void FuSearch::SearchAndReplace( const SvxSearchItem* pSearchItem )
{
    ViewShellBase* pBase = PTR_CAST(ViewShellBase, SfxViewShell::Current());
    ViewShell* pViewShell = NULL;
    if (pBase != NULL)
        pViewShell = pBase->GetMainViewShell().get();

    if (pViewShell != NULL)
    {
        if ( pSdOutliner && pViewShell->ISA(DrawViewShell) && !bOwnOutliner )
        {
            pSdOutliner->EndSpelling();

            bOwnOutliner = sal_True;
            pSdOutliner = new ::sd::Outliner( mpDoc, OUTLINERMODE_TEXTOBJECT );
            pSdOutliner->PrepareSpelling();
        }
        else if ( pSdOutliner && pViewShell->ISA(OutlineViewShell) && bOwnOutliner )
        {
            pSdOutliner->EndSpelling();
            delete pSdOutliner;

            bOwnOutliner = sal_False;
            pSdOutliner = mpDoc->GetOutliner();
            pSdOutliner->PrepareSpelling();
        }

        if (pSdOutliner)
        {
            sal_Bool bEndSpelling = pSdOutliner->StartSearchAndReplace(pSearchItem);

            if (bEndSpelling)
            {
                pSdOutliner->EndSpelling();
                pSdOutliner->PrepareSpelling();
            }
        }
    }
}



} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
