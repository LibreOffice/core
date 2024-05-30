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

#include <fuline.hxx>

#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <svx/svxdlg.hxx>

namespace sd {


FuLine::FuLine (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuLine::DoExecute( SfxRequest& rReq )
{
    rReq.Ignore();

    const SfxItemSet* pArgs = rReq.GetArgs();
    if (pArgs)
        return;

    const SdrObject* pObj = nullptr;
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    SfxItemSet aNewAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aNewAttr );

    bool bHasMarked = rMarkList.GetMarkCount() != 0;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    VclPtr<SfxAbstractTabDialog> pDlg( pFact->CreateSvxLineTabDialog(mpViewShell->GetFrameWeld(), &aNewAttr, mpDoc, pObj, bHasMarked) );

    pDlg->StartExecuteAsync([pDlg, this](sal_Int32 nResult){
        if (nResult == RET_OK)
        {
            mpView->SetAttributes (*(pDlg->GetOutputItemSet ()));

            // some attributes are changed, we have to update the listboxes in the objectbars
            static const sal_uInt16 SidArray[] = {
                SID_ATTR_LINE_STYLE,                // ( SID_SVX_START + 169 )
                SID_ATTR_LINE_DASH,                 // ( SID_SVX_START + 170 )
                SID_ATTR_LINE_WIDTH,                // ( SID_SVX_START + 171 )
                SID_ATTR_LINE_COLOR,                // ( SID_SVX_START + 172 )
                SID_ATTR_LINE_START,                // ( SID_SVX_START + 173 )
                SID_ATTR_LINE_END,                  // ( SID_SVX_START + 174 )
                SID_ATTR_LINE_TRANSPARENCE,         // (SID_SVX_START+1107)
                SID_ATTR_LINE_JOINT,                // (SID_SVX_START+1110)
                SID_ATTR_LINE_CAP,                  // (SID_SVX_START+1111)
                0 };

            mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
        }

        // deferred until the dialog ends
        mpViewShell->Cancel();

        pDlg->disposeOnce();
    });
}

void FuLine::Activate()
{
}

void FuLine::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
