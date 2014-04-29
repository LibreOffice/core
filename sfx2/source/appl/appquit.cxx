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
#include <basic/basmgr.hxx>
#include <basic/sbstar.hxx>

#include <svl/svdde.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/eitem.hxx>

#include <unotools/saveopt.hxx>
#include <unotools/misccfg.hxx>

#ifndef GCC
#endif

#include "app.hrc"
#include <sfx2/app.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/unoctitm.hxx>
#include "appdata.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include "arrdecl.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/event.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/docfile.hxx>
#include "sfxtypes.hxx"
#include "sfxlocal.hrc"
#include <sfx2/fcontnr.hxx>
#include "nochaos.hxx"
#include <sfx2/appuno.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include "appbaslib.hxx"
#include <basic/basicmanagerrepository.hxx>

using ::basic::BasicManagerRepository;

//===================================================================
sal_Bool SfxApplication::QueryExit_Impl()
{
    sal_Bool bQuit = sal_True;

    // will trotzdem noch jemand, den man nicht abschiessen kann, die App haben?
    if ( !bQuit )
    {
        // nicht wirklich beenden, nur minimieren
        InfoBox aInfoBox( NULL, SfxResId(MSG_CANT_QUIT) );
        aInfoBox.Execute();
        DBG_TRACE( "QueryExit => FALSE (in use)" );
        return sal_False;
    }

    return sal_True;
}

//-------------------------------------------------------------------------

void SfxApplication::Deinitialize()
{
    if ( pAppData_Impl->bDowning )
        return;

    StarBASIC::Stop();

    // ggf. BASIC speichern
    BasicManager* pBasMgr = BasicManagerRepository::getApplicationBasicManager( false );
    if ( pBasMgr && pBasMgr->IsModified() )
        SaveBasicManager();

    SaveBasicAndDialogContainer();

    pAppData_Impl->bDowning = sal_True; // wegen Timer aus DecAliveCount und QueryExit

    DELETEZ( pAppData_Impl->pTemplates );

    // By definition there shouldn't be any open view frames when we reach
    // this method. Therefore this call makes no sense and is the source of
    // some stack traces, which we don't understand.
    // For more information see:
    // #123501#
    //SetViewFrame(0);
    pAppData_Impl->bDowning = sal_False;
    DBG_ASSERT( !SfxViewFrame::GetFirst(),
                "existing SfxViewFrame after Execute" );
    DBG_ASSERT( !SfxObjectShell::GetFirst(),
                "existing SfxObjectShell after Execute" );
    pAppData_Impl->pAppDispat->Pop( *this, SFX_SHELL_POP_UNTIL );
    pAppData_Impl->pAppDispat->Flush();
    pAppData_Impl->bDowning = sal_True;
    pAppData_Impl->pAppDispat->DoDeactivate_Impl( sal_True, NULL );

    // call derived application-exit
    Exit();

    // Controller u."a. freigeben
    // dabei sollten auch restliche Komponenten ( Beamer! ) verschwinden
    BasicManagerRepository::resetApplicationBasicManager();
    pAppData_Impl->pBasicManager->reset( NULL );
        // this will also delete pBasMgr

    DBG_ASSERT( pAppData_Impl->pViewFrame == 0, "active foreign ViewFrame" );

    delete[] pAppData_Impl->pInterfaces, pAppData_Impl->pInterfaces = 0;

    // free administration managers
    DELETEZ(pAppData_Impl->pAppDispat);
    SfxResId::DeleteResMgr();
    DELETEZ(pAppData_Impl->pOfaResMgr);

    // ab hier d"urfen keine SvObjects mehr existieren
    DELETEZ(pAppData_Impl->pMatcher);

    DELETEX(pAppData_Impl->pSlotPool);
    DELETEX(pAppData_Impl->pFactArr);
    DELETEX(pAppData_Impl->pInitLinkList);

    DELETEX(pAppData_Impl->pTbxCtrlFac);
    DELETEX(pAppData_Impl->pStbCtrlFac);
    DELETEX(pAppData_Impl->pMenuCtrlFac);
    DELETEX(pAppData_Impl->pViewFrames);
    DELETEX(pAppData_Impl->pViewShells);
    DELETEX(pAppData_Impl->pObjShells);

    //TODO/CLEANTUP
    //ReleaseArgs could be used instead!
/* This leak is intended !
   Otherwise the TestTool can't use .uno:QuitApp ...
   because every destructed ItemSet work's on an already
   released pool pointer .-)

    NoChaos::ReleaseItemPool();
*/
    pAppData_Impl->pPool = NULL;
    DELETEZ(pAppData_Impl->pBasicResMgr);
    DELETEZ(pAppData_Impl->pSvtResMgr);
}
