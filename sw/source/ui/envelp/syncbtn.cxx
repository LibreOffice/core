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
#include "precompiled_sw.hxx"




#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include "cmdid.h"
#include "swmodule.hxx"
#include "view.hxx"
#include "edtwin.hxx"
#include "label.hrc"

#define _SYNCDLG
#include "syncbtn.hxx"
#include "swtypes.hxx"

SFX_IMPL_FLOATINGWINDOW( SwSyncChildWin, FN_SYNC_LABELS )

SwSyncChildWin::SwSyncChildWin( Window* _pParent,
                                sal_uInt16 nId,
                                SfxBindings* pBindings,
                                SfxChildWinInfo* pInfo ) :
                                SfxChildWindow( _pParent, nId )
{
    pWindow = new SwSyncBtnDlg( pBindings, this, _pParent);

    if (!pInfo->aSize.Width() || !pInfo->aSize.Height())
    {
        SwView* pActiveView = ::GetActiveView();
        if(pActiveView)
        {
            const SwEditWin &rEditWin = pActiveView->GetEditWin();
            pWindow->SetPosPixel(rEditWin.OutputToScreenPixel(Point(0, 0)));
        }
        else
            pWindow->SetPosPixel(_pParent->OutputToScreenPixel(Point(0, 0)));
        pInfo->aPos = pWindow->GetPosPixel();
        pInfo->aSize = pWindow->GetSizePixel();
    }

    ((SwSyncBtnDlg *)pWindow)->Initialize(pInfo);

    pWindow->Show();
}

SwSyncBtnDlg::SwSyncBtnDlg( SfxBindings* _pBindings,
                            SfxChildWindow* pChild,
                            Window *pParent) :
    SfxFloatingWindow(_pBindings, pChild, pParent, SW_RES(DLG_SYNC_BTN)),
    aSyncBtn        (this, SW_RES(BTN_SYNC ))
{
    FreeResource();
    aSyncBtn.SetClickHdl(LINK(this, SwSyncBtnDlg, BtnHdl));
}

SwSyncBtnDlg::~SwSyncBtnDlg()
{
}

IMPL_LINK( SwSyncBtnDlg, BtnHdl, PushButton *, EMPTYARG )
{
    SfxViewFrame::Current()->GetDispatcher()->Execute(FN_UPDATE_ALL_LINKS, SFX_CALLMODE_ASYNCHRON);
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
