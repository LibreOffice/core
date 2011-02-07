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

    delete pAppData_Impl->pLabelResMgr;

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
   Otherwise the TestTool cant use .uno:QuitApp ...
   because every destructed ItemSet work's on an already
   released pool pointer .-)

    NoChaos::ReleaseItemPool();
*/
    pAppData_Impl->pPool = NULL;
}
