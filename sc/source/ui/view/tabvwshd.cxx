/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabvwshd.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 15:17:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#ifdef WNT
#pragma optimize ("", off)
#endif

// INCLUDE ---------------------------------------------------------------

#include <sfx2/childwin.hxx>
#include <sfx2/request.hxx>
#include <sfx2/topfrm.hxx>
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





