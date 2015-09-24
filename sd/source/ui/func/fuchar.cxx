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

#include "fuchar.hxx"
#include <svx/dialogs.hrc>
#include <sfx2/viewfrm.hxx>

#include <editeng/editdata.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include "View.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "ViewShell.hxx"
#include "DrawDocShell.hxx"
#include "sdabstdlg.hxx"
#include <memory>
#include <boost/scoped_ptr.hpp>

namespace sd {

TYPEINIT1( FuChar, FuPoor );

FuChar::FuChar (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuChar::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuChar( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuChar::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aEditAttr( mpDoc->GetPool() );
        mpView->GetAttributes( aEditAttr );

        SfxItemSet aNewAttr( mpViewShell->GetPool(),
                                EE_ITEMS_START, EE_ITEMS_END );
        aNewAttr.Put( aEditAttr, false );

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        boost::scoped_ptr<SfxAbstractTabDialog> pDlg(pFact ? pFact->CreateSdTabCharDialog( NULL, &aNewAttr, mpDoc->GetDocSh() ) : 0);
        sal_uInt16 nResult = RET_CANCEL;
        if( pDlg )
        {
            if (rReq.GetSlot() == SID_CHAR_DLG_EFFECT)
            {
                pDlg->SetCurPageId("RID_SVXPAGE_CHAR_EFFECTS");
            }

            nResult = pDlg->Execute();

            if( nResult == RET_OK )
            {
                rReq.Done( *( pDlg->GetOutputItemSet() ) );
                pArgs = rReq.GetArgs();
            }
        }
        if( nResult != RET_OK )
        {
            return;
        }
    }
    mpView->SetAttributes(*pArgs);

    // invalidate the Slots which are in DrTxtObjBar
    static sal_uInt16 SidArray[] = {
                    SID_ATTR_CHAR_FONT,
                    SID_ATTR_CHAR_POSTURE,
                    SID_ATTR_CHAR_WEIGHT,
                    SID_ATTR_CHAR_SHADOWED,
                    SID_ATTR_CHAR_STRIKEOUT,
                    SID_ATTR_CHAR_UNDERLINE,
                    SID_ATTR_CHAR_FONTHEIGHT,
                    SID_ATTR_CHAR_COLOR,
                    SID_ATTR_CHAR_KERNING,
                    SID_ATTR_CHAR_CASEMAP,
                    SID_SET_SUPER_SCRIPT,
                    SID_SET_SUB_SCRIPT,
                    0 };

    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArray );

    if( mpDoc->GetOnlineSpell() )
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pArgs->GetItemState(EE_CHAR_LANGUAGE, false, &pItem ) ||
            SfxItemState::SET == pArgs->GetItemState(EE_CHAR_LANGUAGE_CJK, false, &pItem ) ||
            SfxItemState::SET == pArgs->GetItemState(EE_CHAR_LANGUAGE_CTL, false, &pItem ) )
        {
            mpDoc->StopOnlineSpelling();
            mpDoc->StartOnlineSpelling();
        }
    }
}

void FuChar::Activate()
{
}

void FuChar::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
