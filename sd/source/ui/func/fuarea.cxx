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

#include <fuarea.hxx>

#include <svx/svxids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <ViewShell.hxx>

#include <drawdoc.hxx>
#include <View.hxx>
#include <svx/svxdlg.hxx>

namespace sd {

FuArea::FuArea( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* _pView, SdDrawDocument* pDoc, SfxRequest& rReq)
: FuPoor(rViewSh, pWin, _pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuArea::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* _pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuArea( rViewSh, pWin, _pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuArea::DoExecute( SfxRequest& rReq )
{
    rReq.Ignore ();

    const SfxItemSet* pArgs = rReq.GetArgs();
    if (pArgs)
        return;

    SfxItemSet aNewAttr( mpDoc->GetPool() );
    mpView->GetAttributes( aNewAttr );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    bool bHasSlideBackground = mrViewShell.GetDoc()->GetDocumentType() == DocumentType::Impress;
    VclPtr<AbstractSvxAreaTabDialog> pDlg(
        pFact->CreateSvxAreaTabDialog(mrViewShell.GetFrameWeld(), &aNewAttr, mpDoc, true, bHasSlideBackground));

    pDlg->StartExecuteAsync([pDlg, pView = this->mpView, pViewShell = &this->mrViewShell](sal_Int32 nResult){
        if (nResult == RET_OK)
        {
            pView->SetAttributes (*(pDlg->GetOutputItemSet ()));

            // attributes changed, update Listboxes in Objectbars
            static const sal_uInt16 SidArray[] = {
                SID_ATTR_FILL_STYLE,
                SID_ATTR_FILL_COLOR,
                SID_ATTR_FILL_GRADIENT,
                SID_ATTR_FILL_HATCH,
                SID_ATTR_FILL_BITMAP,
                SID_ATTR_FILL_TRANSPARENCE,
                SID_ATTR_FILL_FLOATTRANSPARENCE,
                SID_ATTR_FILL_USE_SLIDE_BACKGROUND,
                0 };

            pViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );
        }

        // deferred until the dialog ends
        pViewShell->Cancel();

        pDlg->disposeOnce();
    });
}

void FuArea::Activate()
{
}

void FuArea::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
