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
