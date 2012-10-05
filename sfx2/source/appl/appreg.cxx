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


#include <vcl/toolbox.hxx>

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/templdlg.hxx>
#include "inettbc.hxx"
#include "sfx2/stbitem.hxx"
#include <sfx2/infobar.hxx>
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
#include <sfx2/mnuitem.hxx>

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
    SfxInfoBarContainerChild::RegisterChildWindow( sal_True, NULL, SFX_CHILDWIN_NEVERHIDE );

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
    for ( sal_uInt16 n=0; n<pAppData_Impl->pTbxCtrlFac->size(); n++ )
    {
        SfxTbxCtrlFactory *pF = (*pAppData_Impl->pTbxCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("TbxController registration is not clearly defined!");
        }
    }
#endif

    pAppData_Impl->pTbxCtrlFac->push_back( pFact );
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
    for ( sal_uInt16 n=0; n<pAppData_Impl->pStbCtrlFac->size(); n++ )
    {
        SfxStbCtrlFactory *pF = (*pAppData_Impl->pStbCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("StbController registration is not clearly defined!");
        }
    }
#endif

    pAppData_Impl->pStbCtrlFac->push_back( pFact );
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
    for ( sal_uInt16 n=0; n<pAppData_Impl->pMenuCtrlFac->size(); n++ )
    {
        SfxMenuCtrlFactory *pF = (*pAppData_Impl->pMenuCtrlFac)[n];
        if ( pF->nTypeId && pF->nTypeId == pFact->nTypeId &&
            (pF->nSlotId == pFact->nSlotId || pF->nSlotId == 0) )
        {
            DBG_WARNING("MenuController register is not clearly defined!");
        }
    }
#endif

    pAppData_Impl->pMenuCtrlFac->push_back( pFact );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
