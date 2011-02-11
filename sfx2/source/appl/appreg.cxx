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
#include "precompiled_sfx2.hxx"

#include <vcl/toolbox.hxx>

#ifndef GCC
#endif

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/templdlg.hxx>
#include "objmnctl.hxx"
#include "inettbc.hxx"
#include "sfx2/stbitem.hxx"
#include <sfx2/navigat.hxx>
#include <sfx2/taskpane.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include "partwnd.hxx"
#include <sfx2/sfxsids.hrc>
#include "recfloat.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objface.hxx>

//===================================================================

void SfxApplication::Registrations_Impl()
{
    // Interfaces
    SfxApplication::RegisterInterface();
    SfxModule::RegisterInterface();
    SfxViewFrame::RegisterInterface();
    SfxObjectShell::RegisterInterface();
    SfxViewShell::RegisterInterface();

    // ChildWindows
    SfxRecordingFloatWrapper_Impl::RegisterChildWindow();
    SfxNavigatorWrapper::RegisterChildWindow( sal_False, NULL, SFX_CHILDWIN_NEVERHIDE );
    SfxPartChildWnd_Impl::RegisterChildWindow();
    SfxTemplateDialogWrapper::RegisterChildWindow(sal_True);
    SfxDockingWrapper::RegisterChildWindow();

    // Controller
    SfxToolBoxControl::RegisterControl(SID_REPEAT);
    SfxURLToolBoxControl_Impl::RegisterControl(SID_OPENURL);
    SfxAppToolBoxControl_Impl::RegisterControl( SID_NEWDOCDIRECT );
    SfxAppToolBoxControl_Impl::RegisterControl( SID_AUTOPILOTMENU );
};

//--------------------------------------------------------------------

void SfxApplication::RegisterToolBoxControl_Impl( SfxModule *pMod, SfxTbxCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterToolBoxControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pAppData_Impl->pTbxCtrlFac->Count(); n++ )
    {
        SfxTbxCtrlFactory *pF = (*pAppData_Impl->pTbxCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("TbxController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pAppData_Impl->pTbxCtrlFac->C40_INSERT( SfxTbxCtrlFactory, pFact, pAppData_Impl->pTbxCtrlFac->Count() );
}

//--------------------------------------------------------------------

void SfxApplication::RegisterStatusBarControl_Impl( SfxModule *pMod, SfxStbCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterStatusBarControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pAppData_Impl->pStbCtrlFac->Count(); n++ )
    {
        SfxStbCtrlFactory *pF = (*pAppData_Impl->pStbCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("StbController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pAppData_Impl->pStbCtrlFac->C40_INSERT( SfxStbCtrlFactory, pFact, pAppData_Impl->pStbCtrlFac->Count() );
}

//--------------------------------------------------------------------

void SfxApplication::RegisterMenuControl_Impl( SfxModule *pMod, SfxMenuCtrlFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterMenuControl( pFact );
        return;
    }

#ifdef DBG_UTIL
    for ( sal_uInt16 n=0; n<pAppData_Impl->pMenuCtrlFac->Count(); n++ )
    {
        SfxMenuCtrlFactory *pF = (*pAppData_Impl->pMenuCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("MenuController-Registrierung ist nicht eindeutig!");
        }
    }
#endif

    pAppData_Impl->pMenuCtrlFac->C40_INSERT( SfxMenuCtrlFactory, pFact, pAppData_Impl->pMenuCtrlFac->Count() );
}
