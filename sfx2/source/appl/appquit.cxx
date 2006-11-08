/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appquit.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 11:57:21 $
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
#include "precompiled_sfx2.hxx"

#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#ifdef WIN
#define _TL_LANG_SPECIAL
#endif

#ifndef _SVDDE_HXX //autogen
#include <svtools/svdde.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

#include <svtools/saveopt.hxx>
#include <svtools/misccfg.hxx>

#ifndef GCC
#endif

#include "app.hrc"
#include "app.hxx"
#include "unoctitm.hxx"
#include "appdata.hxx"
#include "viewsh.hxx"
#include "dispatch.hxx"
#include "printer.hxx"
#include "arrdecl.hxx"
#include "sfxresid.hxx"
#include "newhdl.hxx"
#include "event.hxx"
#include "macrconf.hxx"
#include "mnumgr.hxx"
#include "templdlg.hxx"
#include "msgpool.hxx"
#include "docfile.hxx"
#include "sfxtypes.hxx"
#include "sfxlocal.hrc"
#include "fcontnr.hxx"
#include "nochaos.hxx"
#include "appuno.hxx"
#include "doctempl.hxx"
#include "viewfrm.hxx"
#include "objsh.hxx"
#include "docfac.hxx"
#include "appbaslib.hxx"

#ifndef BASICMANAGERREPOSITORY_HXX
#include <basic/basicmanagerrepository.hxx>
#endif

using ::basic::BasicManagerRepository;

#ifndef PRODUCT
DECLARE_LIST( SfxFrameWindowFactoryArray_Impl, SfxFrameWindowFactory* )
SV_DECL_PTRARR(SfxInitLinkList, Link*, 2, 2)
#endif

//===================================================================
BOOL SfxApplication::QueryExit_Impl()
{
    BOOL bQuit = TRUE;

/*
    BOOL bPrinting = FALSE;
    for ( SfxViewShell *pViewSh = SfxViewShell::GetFirst();
          !bPrinting && pViewSh;
          pViewSh = SfxViewShell::GetNext(*pViewSh) )
    {
        SfxPrinter *pPrinter = pViewSh->GetPrinter();
        bPrinting = pPrinter && pPrinter->IsPrinting();
    }

    if ( bPrinting )
    {
        // Benutzer fragen, ob abgebrochen werden soll
        if ( RET_OK == QueryBox( 0, SfxResId( MSG_ISPRINTING_QUERYABORT ) ).Execute() )
        {
            // alle Jobs canceln
            for ( SfxViewShell *pViewSh = SfxViewShell::GetFirst();
                  !bPrinting && pViewSh;
                  pViewSh = SfxViewShell::GetNext(*pViewSh) )
            {
                SfxPrinter *pPrinter = pViewSh->GetPrinter();
                if ( pPrinter && pPrinter->IsPrinting() )
                    pPrinter->AbortJob();
            }

            // da das Canceln asynchron ist, Quit erstmal wieder verlassen
            GetDispatcher_Impl()->Execute( SID_QUITAPP, SFX_CALLMODE_ASYNCHRON );
            DBG_TRACE( "QueryExit => FALSE (printing)" );
            return FALSE;
        }
    }
*/
    // alles canceln was zu canceln ist
    GetCancelManager()->Cancel(TRUE);

/*
    SfxObjectShell *pLastDocSh = SfxObjectShell::GetFirst();
    if ( bQuit )
    {
        // Jetzt zur Sicherheit auch hidden Frames abr"aumen
        SfxViewFrame::CloseHiddenFrames_Impl();
        pLastDocSh = SfxObjectShell::GetFirst();
    }
*/
    // will trotzdem noch jemand, den man nicht abschiessen kann, die App haben?
    if ( !bQuit )
    {
        // nicht wirklich beenden, nur minimieren
        InfoBox aInfoBox( NULL, SfxResId(MSG_CANT_QUIT) );
        aInfoBox.Execute();
        DBG_TRACE( "QueryExit => FALSE (in use)" );
        return FALSE;
    }

    return TRUE;
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

    SaveBasicContainer();
    SaveDialogContainer();

    pAppData_Impl->bDowning = TRUE; // wegen Timer aus DecAliveCount und QueryExit

    DELETEZ( pAppData_Impl->pTemplates );

    // By definition there shouldn't be any open view frames when we reach
    // this method. Therefore this call makes no sense and is the source of
    // some stack traces, which we don't understand.
    // For more information see:
    // #123501#
    //SetViewFrame(0);
    pAppData_Impl->bDowning = FALSE;
    DBG_ASSERT( !SfxViewFrame::GetFirst(),
                "existing SfxViewFrame after Execute" );
    DBG_ASSERT( !SfxObjectShell::GetFirst(),
                "existing SfxObjectShell after Execute" );
    pAppData_Impl->pAppDispat->Pop( *this, SFX_SHELL_POP_UNTIL );
    pAppData_Impl->pAppDispat->Flush();
    pAppData_Impl->bDowning = TRUE;
    pAppData_Impl->pAppDispat->DoDeactivate_Impl( TRUE, NULL );

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

#ifndef PRODUCT
    DELETEX(pAppData_Impl->pSlotPool);
    DELETEX(pAppData_Impl->pEventConfig);
    DELETEX(pAppData_Impl->pMiscConfig);
    SfxMacroConfig::Release_Impl();
    DELETEX(pAppData_Impl->pFactArr);
    DELETEX(pAppData_Impl->pInitLinkList);
#endif

#ifndef PRODUCT
    DELETEX(pAppData_Impl->pTbxCtrlFac);
    DELETEX(pAppData_Impl->pStbCtrlFac);
    DELETEX(pAppData_Impl->pMenuCtrlFac);
    SfxNewHdl::Delete();
    DELETEX(pAppData_Impl->pViewFrames);
    DELETEX(pAppData_Impl->pViewShells);
    DELETEX(pAppData_Impl->pObjShells);
#endif

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
