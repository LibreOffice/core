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
#include "precompiled_sfx2.hxx"

#include <vcl/toolbox.hxx>

#ifndef GCC
#endif

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/templdlg.hxx>
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

    // Controller
    SfxToolBoxControl::RegisterControl(SID_REPEAT);
    SfxURLToolBoxControl_Impl::RegisterControl(SID_OPENURL);
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
