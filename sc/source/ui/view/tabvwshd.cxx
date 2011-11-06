/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    (pReqArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)

//!         Parent-Window fuer Dialoge
//!         Problem: OLE Server!

Window* ScTabViewShell::GetDialogParent()
{
    //  #95513# if a ref-input dialog is open, use it as parent
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





