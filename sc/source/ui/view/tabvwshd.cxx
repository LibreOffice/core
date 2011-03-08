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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/childwin.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include "tabvwsh.hxx"
#include "global.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "sc.hrc"


// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), TRUE, ppItem ) == SFX_ITEM_SET)

//!         Parent-Window fuer Dialoge
//!         Problem: OLE Server!

Window* ScTabViewShell::GetDialogParent()
{
    //  if a ref-input dialog is open, use it as parent
    //  (necessary when a slot is executed from the dialog's OK handler)
    if ( nCurRefDlgId && nCurRefDlgId == SC_MOD()->GetCurRefDlgId() )
    {
        SfxViewFrame* pViewFrm = GetViewFrame();
        if ( pViewFrm->HasChildWindow(nCurRefDlgId) )
        {
            SfxChildWindow* pChild = pViewFrm->GetChildWindow(nCurRefDlgId);
            if (pChild)
            {
                Window* pWin = pChild->GetWindow();
                if (pWin && pWin->IsVisible())
                    return pWin;
            }
        }
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if ( pDocSh->IsOle() )
    {
        //TODO/LATER: how to GetEditWindow in embedded document?!
        //It should be OK to return the VieShell Window!
        return GetWindow();
        //SvInPlaceEnvironment* pEnv = pDocSh->GetIPEnv();
        //if (pEnv)
        //    return pEnv->GetEditWin();
    }

    return GetActiveWin();      // for normal views, too
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
