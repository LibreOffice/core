/*************************************************************************
 *
 *  $RCSfile: dispatch.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-27 11:17:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXEITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVTOOLS_TTPROPS_HXX
#include <svtools/ttprops.hxx>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>             // wg. bsearch

#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#include <svtools/helpopt.hxx>

#pragma hdrstop

#include "sfxhelp.hxx"
#include "appdata.hxx"
#include "dispatch.hxx"
#include "minstack.hxx"
#include "msg.hxx"
#include "objface.hxx"
#include "bindings.hxx"
#include "request.hxx"
#include "app.hxx"
#include "hintpost.hxx"
#include "slotserv.hxx"
#include "ipfrm.hxx"
#include "interno.hxx"
#include "sfxtypes.hxx"
#include "macrconf.hxx"
#include "virtmenu.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "mnumgr.hxx"
#include "childwin.hxx"
#include "docfac.hxx"
#include "ipenv.hxx"
#include "msgpool.hxx"
#include "module.hxx"
#include "tbxconf.hxx"
#include "topfrm.hxx"
#include "sfxuno.hxx"

//==================================================================
DBG_NAME(SfxDispatcherFlush);
DBG_NAME(SfxDispatcherFillState);

//==================================================================
SV_IMPL_PTRARR( SfxItemPtrArray, SfxPoolItemPtr );

DECL_PTRSTACK(SfxShellStack_Impl, SfxShell*, 8, 4 );
//==================================================================

struct SfxToDo_Impl
{
    SfxShell*           pCluster;
    sal_Bool                bPush;
    sal_Bool                bDelete;
    sal_Bool                bUntil;

    SfxToDo_Impl():
        bPush(sal_False), bDelete(sal_False), bUntil(sal_False), pCluster(0) {}
    SfxToDo_Impl( sal_Bool bOpPush, sal_Bool bOpDelete, sal_Bool bOpUntil, SfxShell& rCluster ):
        bPush(bOpPush), bDelete(bOpDelete), bUntil(bOpUntil), pCluster(&rCluster) {}
    ~SfxToDo_Impl(){}

    sal_Bool operator==( const SfxToDo_Impl& rWith ) const
    { return pCluster==rWith.pCluster && bPush==rWith.bPush; }
};

DECL_OBJSTACK(SfxToDoStack_Impl, SfxToDo_Impl, 8, 4);
IMPL_OBJSTACK(SfxToDoStack_Impl, SfxToDo_Impl);

struct SfxObjectBars_Impl
{
    ResId               aResId; // Resource - und ConfigId der Toolbox
    sal_uInt16              nMode;  // spezielle Sichtbarkeitsflags
    String              aName;
    SfxInterface*       pIFace;

    SfxObjectBars_Impl() :
        aResId( 0,0 )
    {}
};

//------------------------------------------------------------------

struct SfxDispatcher_Impl
{
    const SfxSlotServer* pCachedServ1;       // zuletzt gerufene Message
    const SfxSlotServer* pCachedServ2;       // vorletzt gerufene Message
    SfxShellStack_Impl      aStack;             // aktive Funktionalitaet
    Timer                   aTimer;             // fuers flushen
    SfxToDoStack_Impl       aToDoStack;         // nicht abgearb. Push/Pop
    SfxViewFrame*           pFrame;             // 0 oder zugeh"or. Frame
    SfxDispatcher*          pParent;            // z.B. AppDispatcher, ggf. 0
    SfxHintPosterRef        xPoster;            // asynchrones Execute
    sal_Bool                    bFlushing;          // sal_True waehrend Flush //?
    sal_Bool                    bFlushed;           // aToDoStack.Count() == 0
    sal_Bool                    bUpdated;           // Update_Impl gelaufen
    sal_Bool                    bLocked;            // kein Execute
    sal_Bool                    bInvalidateOnUnlock;// da fragte jemand
    sal_Bool                    bActive;            // nicht verwechseln mit gesetzt!
    sal_Bool*                   pInCallAliveFlag;   // dem Stack den Dtor anzeigen
    SfxObjectBars_Impl      aObjBars[SFX_OBJECTBAR_MAX];
    SfxObjectBars_Impl      aFixedObjBars[SFX_OBJECTBAR_MAX];
    SvULongs                aChildWins;
    sal_uInt16                  nActionLevel;       // in EnterAction
    sal_uInt32                  nEventId;           // EventId UserEvent
    sal_Bool                    bUILocked;          // Update abgeklemmt (!zappeln)
    sal_Bool                    bNoUI;              // UI nur vom Parent Dispatcher
    sal_Bool                    bReadOnly;          // Dokument ist ReadOnly
    sal_Bool                    bQuiet;             // nur parent dispatcher verwenden
    sal_Bool                    bModal;             // nur Slots vom Parent-Dispatcher

    sal_Bool                    bFilterEnabling;    // sal_True=filter enabled slots, 2==ReadOnlyDoc uebersteuert
    sal_uInt16                  nFilterCount;       // Anzahl der SIDs in pFilterSIDs
    const sal_uInt16*           pFilterSIDs;        // sortiertes Array von SIDs
    sal_uInt16                  nStandardMode;      // ExecuteMode f. PlugInDispatcher
    SvUShorts*              pDisableList;
    sal_uInt32                  nDisableFlags;
};

#define NO_OBJECTBAR    0
#define OWN_OBJECTBAR   1
#define OTHER_OBJECTBAR 2

//------------------------------------------------------------------

#define SFX_FLUSH_TIMEOUT    50

//====================================================================
sal_Bool SfxDispatcher::IsLocked( sal_uInt16 nSID ) const

/*  [Beschreibung]

    Mit dieser Methode kann festgestellt werden, ob der SfxDispatcher
    gesperrt oder freigegeben ist. Ein gesperrter SfxDispatcher
    f"uhrt keine <SfxRequest>s mehr aus und liefert keine
    Status-Informationen mehr. Er verh"alt sich so als w"aren alle
    Slots disabled.

    Der Dispatcher gilt auch als gesperrt, wenn alle Dispatcher
    gelockt sind (<SfxApplication::LockDispatcher()>) oder der zugeh"orige
    Top-Frame im modal-mode ist und der angegebene Slot Frame-spezifisch
    (also nicht von der Application) bedient wird.
*/

{
    return nSID != SID_BROWSE_STOP && nSID != SID_HELP_PI &&
           ( pImp->bLocked || SFX_APP()->IsDispatcherLocked() );
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::IsAppDispatcher() const

/*  [Beschreibung]

    Mit dieser Methode l"a\st sich festellen, ob der SfxDispacher der
    Applikations-Dispatcher ist.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Es ist der Applikations-Dispatcher.

                        sal_False
                        Es ist ein Dispatcher eines SfxViewFrame.
*/

{
    return !pImp->pFrame;
}

//--------------------------------------------------------------------
int SfxDispatcher::Call_Impl( SfxShell& rShell, const SfxSlot &rSlot, SfxRequest &rReq, sal_Bool bRecord )

/*  [Beschreibung]

    Hilfsfunktion zum pr"ufen, ob ein Slot executed werden darf und
    der Execution selbst.
*/

{
    SFX_STACK(SfxDispatcher::Call_Impl);

    // darf der Slot gerufen werden (i.S.v. enabled)
    SfxApplication *pSfxApp = SFX_APP();
    if ( rSlot.IsMode(SFX_SLOT_FASTCALL) || rShell.CanExecuteSlot_Impl(rSlot) )
    {
        // ggf. die Bindings locken (MI: warum?)
        SfxBindings *pBindings = GetBindings();
#if modal_mode_sinnlos
        sal_Bool bLockBindings = rSlot.GetSlotId() != SID_OPENDOC && rSlot.GetSlotId() != SID_OPENURL;
        if ( bLockBindings && pBindings )
            pBindings->DENTERREGISTRATIONS();
#endif

        // Alles holen, was gebraucht wird, da der Slot den Execute evtl. nicht
        // "uberlebt, falls es ein 'Pseudoslot' f"ur Macros oder Verben ist
        sal_Bool bAutoUpdate = rSlot.IsMode(SFX_SLOT_AUTOUPDATE);
        SFX_REQUEST_ARG(rReq, pOrigItem, SfxExecuteItem, SID_SUBREQUEST, sal_False);
        SfxExecuteItem *pExecuteItem = pOrigItem
                            ? (SfxExecuteItem*)pOrigItem->Clone()
                            : 0;

        // ggf. TabPage-ID setzen und SID merken
        SfxAppData_Impl *pAppData = pSfxApp->Get_Impl();
        SFX_REQUEST_ARG(rReq, pTabPageItem, SfxUInt16Item, SID_TABPAGE, sal_False);
        if ( pTabPageItem )
        {
            pAppData->nAutoTabPageId = pTabPageItem->GetValue();
            rReq.RemoveItem( SID_TABPAGE ); // sonst ArgCount > 0 => Seiteneff.
        }
        pAppData->nExecutingSID = rReq.GetSlot();

        // API-Call-Klammerung und Document-Lock w"ahrend des Calls
        {
            // 'this' mu\s im Dtor bescheid sagen
            sal_Bool bThisDispatcherAlive = sal_True;
            sal_Bool *pOldInCallAliveFlag = pImp->pInCallAliveFlag;
            pImp->pInCallAliveFlag = &bThisDispatcherAlive;

            // eigentlichen Call durchf"uhren
            sal_Bool bDone = sal_False;
/*
            SfxViewFrame *pViewFrame = GetFrame();
            if ( pViewFrame )
            {
                SFX_REQUEST_ARG(rReq, pInterceptorItem, SfxBoolItem, SID_INTERCEPTOR, sal_False);
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >  xProv( pViewFrame->GetFrame()->GetInterceptor_Impl() );
                sal_Bool bDirect = ( pInterceptorItem != 0 );
                if ( bDirect )
                {
                    bDirect = !pInterceptorItem->GetValue();
                    rReq.RemoveItem( SID_INTERCEPTOR );
                }

                if ( xProv.is() && !bDirect )
                {
                    bDone = sal_True;
                    ::com::sun::star::util::URL aURL;
                    String aName( rSlot.GetUnoName() );
                    String aCmd;
                    if ( aName.getLength() )
                    {
                        aCmd = ".uno:";
                        aCmd += aName;
                    }
                    else
                    {
                        aCmd = "slot:";
                        aCmd += rSlot.GetSlotId();
                    }

                    aURL.Complete = S2U( aCmd );
                    SfxURLTransformer aTrans;
                    aTrans.parseStrict( aURL );
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp = xProv->queryDispatch( aURL, "", 0 );
                    if ( xDisp.is() )
                    {
// automatisch auskommentiert - [getImplementation] - Wird von OWeakObject nicht weiter unterst¸tzt!
//                      void* pDisp = xDisp->getImplementation( ::getCppuType((const SfxOfficeDispatch*)0) );

                        if ( pDisp)
                            bDone = sal_False;
                        else
                        {
                            ::com::sun::star::uno::Sequence <PropertyValue> aSequ;
                            if ( rReq.GetArgs() )
                                TransformItems( rSlot.GetSlotId(), *rReq.GetArgs(), aSequ, &rSlot );
                            xDisp->dispatch( aURL, aSequ );
                        }
                    }
                }
            }
*/
            if ( !bDone )
            {
                // Bei neuen/komplizierten Funktionen den HelpAgent feuern...
  //              Help* pHelp = Application::GetHelp();
  //              if ( pHelp )
  //                  ((SfxHelp_Impl*)pHelp)->SlotExecutedOrFocusChanged( rReq.GetSlot(), sal_True, SvtHelpOptions().IsHelpAgentAutoStartMode() );

                SfxExecFunc pFunc = rSlot.GetExecFnc();
                rShell.CallExec( pFunc, rReq );
            }

            // falls 'this' noch lebt
            if ( bThisDispatcherAlive )
                pImp->pInCallAliveFlag = pOldInCallAliveFlag;
            else if ( pOldInCallAliveFlag )
                // auch verschachtelte Stack-Frames sch"utzen
                *pOldInCallAliveFlag = sal_False;
        }

        // TabPage-ID und Executing-SID zurueck setzen
        if ( pTabPageItem )
            pAppData->nAutoTabPageId = 0;
        pAppData->nExecutingSID = 0;

#if modal_mode_sinnlos
        // ggf. Lock wieder freigeben
        if ( bLockBindings )
            pBindings->DLEAVEREGISTRATIONS();
#endif

        if( pExecuteItem )
        {
            Execute( *pExecuteItem );
            delete pExecuteItem;
        }

        if ( rReq.IsDone() )
        {
            // bei AutoUpdate sofort updaten; "Pseudoslots" d"urfen nicht
            // Autoupdate sein!
            if ( bAutoUpdate )
            {
                const SfxSlot* pSlave = rSlot.GetLinkedSlot();
                if (pSlave)
                {
                    // bei Enum-Slots irgendeinen gebundenen Slave-Slot nehmen
                    while (!pBindings->IsBound(pSlave->GetSlotId()) && pSlave != &rSlot )
                        pSlave = pSlave->GetLinkedSlot();
                    pBindings->Invalidate(pSlave->GetSlotId());
                    pBindings->Update(pSlave->GetSlotId());
                }
                else
                {
                    pBindings->Invalidate(rSlot.GetSlotId());
                    pBindings->Update(rSlot.GetSlotId());
                }
            }
            return sal_True;
        }
    }

    return sal_False;
}

//====================================================================
void SfxDispatcher::Construct_Impl( SfxDispatcher* pParent )
{
    pImp = new SfxDispatcher_Impl;
    bFlushed = sal_True;
    SfxApplication *pSfxApp = SFX_APP();

    pImp->pCachedServ1 = 0;
    pImp->pCachedServ2 = 0;
    pImp->bFlushing = sal_False;
    pImp->bUpdated = sal_False;
    pImp->bLocked = sal_False;
    pImp->bActive = sal_False;
    pImp->pParent = NULL;
    pImp->bUILocked = sal_False;
    pImp->bNoUI = sal_False;
    pImp->bReadOnly = sal_False;
    pImp->bQuiet = sal_False;
    pImp->bModal = sal_False;
    pImp->pInCallAliveFlag = 0;
    pImp->bFilterEnabling = sal_False;
    pImp->nFilterCount = 0;
    pImp->pFilterSIDs = 0;
    pImp->nStandardMode = 0;
    pImp->pDisableList = pSfxApp->GetDisabledSlotList_Impl();
    pImp->nDisableFlags = 0;

    pImp->pParent = pParent;

    pImp->bInvalidateOnUnlock = sal_False;
    pImp->nActionLevel = 0;

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].aResId = ResId( 0,0 );

    GenLink aGenLink( LINK(this, SfxDispatcher, PostMsgHandler) );

    pImp->xPoster = new SfxHintPoster(aGenLink);

    pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
    pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
}

SfxDispatcher::SfxDispatcher( SfxDispatcher* pParent )
{
    Construct_Impl( pParent );
    pImp->pFrame = 0;
}

SfxDispatcher::SfxDispatcher( SfxViewFrame *pViewFrame )

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxDispatcher legt einen leeren Stack
    von <SfxShell>-Pointern an. Er ist initial nicht gelockt und gilt als
    geflusht.
*/

{
    if ( pViewFrame )
    {
        SfxViewFrame *pFrame = pViewFrame->GetParentViewFrame();
        if ( pFrame )
            Construct_Impl(  pFrame->GetDispatcher() );
        else
            Construct_Impl( 0 );
    }
    else
        Construct_Impl( 0 );
    pImp->pFrame = pViewFrame;
}

//====================================================================
SfxDispatcher::~SfxDispatcher()

/*  [Beschreibung]

    Der Destruktor der Klasse SfxDispatcher darf nicht gerufen werden,
    wenn die SfxDispatcher-Instanz aktiv ist. Es d"urfen sich allerdings
    noch <SfxShell>-Pointer auf dem Stack befinden.
*/

{
#ifdef DBG_UTIL
    ByteString sTemp( "Delete Dispatcher " );
    sTemp += ByteString::CreateFromInt32( (sal_uInt32)this );
    DBG_TRACE( sTemp.GetBuffer() );
    DBG_ASSERT( !pImp->bActive, "deleting active Dispatcher" );
#endif

    // Damit in LeaveRegistrations kein Timer per Reschedule in PlugComm
    // zuschlaegt
    pImp->aTimer.Stop();
    pImp->xPoster->SetEventHdl( Link() );

    // die Stack-Varialblem in Call_Impl benachrichtigen
    if ( pImp->pInCallAliveFlag )
        *pImp->pInCallAliveFlag = sal_False;

    // Bindings und App besorgen
    SfxApplication *pSfxApp = SFX_APP();
    SfxBindings* pBindings = GetBindings();

//  if (pImp->nEventId)
//      pSfxApp->RemoveEventHdl(pImp->nEventId);

    // wenn noch nicht flushed, die Bindings wiederbeleben
    if ( pBindings && !pSfxApp->IsDowning() && !bFlushed )
        pBindings->DLEAVEREGISTRATIONS();

    // ggf. bei den Bindings abmelden
    while ( pBindings )
    {
        if ( pBindings->GetDispatcher_Impl() == this)
            pBindings->SetDispatcher(0);
        pBindings = pBindings->GetSubBindings_Impl();
    }

    delete pImp;
}

//====================================================================
void SfxDispatcher::Pop
(
    SfxShell&   rShell,     /*  Die vom Stack zu nehmende SfxShell-Instanz. */

    sal_uInt16      nMode       /*  SFX_SHELL_POP_UNTIL
                                Es werden auch alle "uber 'rShell' liegenenden
                                SfxShell's vom Stack genommen.

                                SFX_SHELL_POP_DELETE
                                Alle tats"achlich vom Stack genommenen
                                SfxShells werden gel"oscht.

                                SFX_SHELL_PUSH (InPlace use only)
                                Die Shell wird gepusht. */
)
/*  [Beschreibung]

    Mit dieser Methode wird eine oder mehrere <SfxShell> vom SfxDispatcher
    gepoppt. Die SfxShell wird zun"achst zum poppen vermerkt und
    es wird ein Timer aufgesetzt. Erst bei Ablauf des Timers wird
    tats"achlich gepoppt (<SfxDispatcher::Flush()>) und die <SfxBindings>
    werden invalidiert. W"ahrend der Timer l"auft gleichen sich
    entgegengesetzte Push und Pop Befehle mit derselben SfxShell aus.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( rShell.GetInterface(),
                "pushing SfxShell without previous RegisterInterface()" );
    DBG_ASSERT( pImp->nActionLevel == 0, "Push or Pop within Action" );
//  DBG_ASSERT( SFX_APP()->IsInAsynchronCall_Impl(),
//                "Dispatcher Push/Pop in synchron-call-stack" );

    FASTBOOL bDelete = (nMode & SFX_SHELL_POP_DELETE) == SFX_SHELL_POP_DELETE;
    FASTBOOL bUntil = (nMode & SFX_SHELL_POP_UNTIL) == SFX_SHELL_POP_UNTIL;
    FASTBOOL bPush = (nMode & SFX_SHELL_PUSH) == SFX_SHELL_PUSH;

    SfxApplication *pSfxApp = SFX_APP();

#ifdef DBG_UTIL
    ByteString aMsg( "SfxDispatcher(" );
    aMsg += ByteString::CreateFromInt32( (long) this );
    aMsg += bPush ? ")::Push(" : ")::Pop(";
    if ( rShell.GetInterface() )
        aMsg += rShell.GetInterface()->GetClassName();
    else
        aMsg += ByteString::CreateFromInt32( (long) &rShell );
    aMsg += bDelete ? ") with delete" : ")";
    DbgTrace( aMsg.GetBuffer() );
#endif

    // gleiche Shell wie on-Top des ToDo-Stacks?
    if ( pImp->aToDoStack.Count() && pImp->aToDoStack.Top().pCluster == &rShell )
    {
        // inverse Actions heben sich auf
        if ( pImp->aToDoStack.Top().bPush != bPush )
            pImp->aToDoStack.Pop();
        else
        {
            DBG_ASSERT( bPush, "SfxInterface pushed more than once" );
            DBG_ASSERT( !bPush, "SfxInterface popped more than once" );
        }
    }
    else
    {
        // ::com::sun::star::chaos::Action merken
        pImp->aToDoStack.Push( SfxToDo_Impl(bPush, bDelete, bUntil, rShell) );
        if ( bFlushed )
        {
            bFlushed = sal_False;
            pImp->bUpdated = sal_False;

            // Bindings schlafen legen
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DENTERREGISTRATIONS();
        }
    }

    if ( !pSfxApp->IsDowning() && pImp->aToDoStack.Count() )
    {
        //! if (SFX_APP()->AnyInput(INPUT_KEYBOARD | INPUT_MOUSE) )
        //! AnyInput haut nicht hin; hier muss noch ein Kriterium gefunden
        //! werden. Solange wieder immer mit Timer.

        if (sal_True)
        {
            // Kein sofortiges Update gewuenscht
            pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
            pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
            pImp->aTimer.Start();
        }
        else
        {
            // Schnellstmoegliches Update (sollte Normalfall sein)
            pImp->aTimer.Stop();
            GetpApp()->PostUserEvent(pImp->nEventId, (void*)0);
        }
    }
    else
    {
        // doch nichts zu tun
        pImp->aTimer.Stop();

        // ggf. Bindings wieder aufwecken
        if ( !pImp->aToDoStack.Count() )
        {
            SfxBindings* pBindings = GetBindings();
            if ( pBindings )
                pBindings->DLEAVEREGISTRATIONS();
        }
    }
}

//--------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxDispatcher, EventHdl_Impl, void *, pvoid )

/*  [Beschreibung]

    Dieser Handler wird nach <SfxDispatcher::Invalidate()> oder Bewegungen
    auf dem Stack (<SfxDispatcher::Push()> und <SfxDispatcher::Pop()) gerufen.

    Er flusht den Stack, falls er dirty ist, f"uhrt also die ausstehenden
    Push und Pop Befehle tats"achlich aus.
*/

{
    DBG_MEMTEST();

    Flush();
    Update_Impl();
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->StartUpdate_Impl(sal_False);
    return 0;
}
IMPL_LINK_INLINE_END( SfxDispatcher, EventHdl_Impl, void *, pvoid )

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::CheckVirtualStack( const SfxShell& rShell, sal_Bool bDeep )

/*  [Beschreibung]

    Mit dieser Methode kann gepr"uft werden, ob sich die <SfxShell> rShell
    auf dem Stack befindet, wenn er geflusht w"are. Dabei wird der
    SfxDispatcher jedoch nicht tats"achlich geflusht.

    Diese Methode ist u.a. dazu gedacht, Assertions zu erm"oglichen, ohne
    als Seiteneffekt den SfxDispathcer flushen zu m"ussen.
*/

{
    DBG_MEMTEST();
    SFX_STACK(SfxDispatcher::CheckVirtualStack);

    SfxShellStack_Impl aStack( pImp->aStack );
    for ( short nToDo = pImp->aToDoStack.Count()-1; nToDo >= 0; --nToDo )
    {
        SfxToDo_Impl aToDo( pImp->aToDoStack.Top(nToDo) );
        if ( aToDo.bPush )
            aStack.Push( (SfxShell*) aToDo.pCluster );
        else
        {
            SfxShell* pPopped = 0;
            do
            {
                DBG_ASSERT( aStack.Count(), "popping from empty stack" );
                pPopped = aStack.Pop();
            }
            while ( aToDo.bUntil && pPopped != aToDo.pCluster );
            DBG_ASSERT( pPopped == aToDo.pCluster, "popping unpushed SfxInterface" );
        }
    }

    sal_Bool bReturn;
    if ( bDeep )
        bReturn = aStack.Contains(&rShell);
    else
        bReturn = aStack.Top() == &rShell;
    return bReturn;
}

//--------------------------------------------------------------------
sal_uInt16 SfxDispatcher::GetShellLevel( const SfxShell& rShell )

/*  [Beschreibung]

    Ermittelt die Position einer SfxShell im Stack des Dispatchers.
    Dazu wird dieser ggf. zuvor geflusht.


    [RÅckgabewert]

    sal_uInt16                      == USRT_MAX
                                Die SfxShell befindet sich nicht auf
                                diesem SfxDispatcher.

                                < USHRT_MAX
                                Position der SfxShell auf dem Dispatcher
                                von oben mit 0 beginnend gez"ahlt.
*/

{
    DBG_MEMTEST();
    SFX_STACK(SfxDispatcher::GetShellLevel);
    Flush();

    for ( sal_uInt16 n = 0; n < pImp->aStack.Count(); ++n )
        if ( pImp->aStack.Top( n ) == &rShell )
            return n;
    if ( pImp->pParent )
    {
        sal_uInt16 nRet = pImp->pParent->GetShellLevel(rShell);
        if ( nRet == USHRT_MAX )
            return nRet;
        return  nRet + pImp->aStack.Count();
    }

    return USHRT_MAX;
}

//--------------------------------------------------------------------
SfxShell *SfxDispatcher::GetShell(sal_uInt16 nIdx) const

/*  [Beschreibung]

    Liefert einen Pointer auf die <SfxShell>, welche sich an der Position
    nIdx (von oben, letzt-gepushte liegt bei 0) auf dem Stack befindet.

    Dabei wird der SfxDispatcher nicht geflusht.

    Ist der Stack nicht tief genug, wird ein 0-Pointer zur"uckgegeben.
*/

{
    DBG_MEMTEST();

    sal_uInt16 nShellCount = pImp->aStack.Count();
    if ( nIdx < nShellCount )
        return pImp->aStack.Top(nIdx);
    else if ( pImp->pParent )
        return pImp->pParent->GetShell( nIdx - nShellCount );
    return 0;
}

//--------------------------------------------------------------------
SfxBindings* SfxDispatcher::GetBindings() const

/*  [Beschreibung]

    Diese Methode liefert einen Pointer auf die <SfxBindings> Instanz
    zur"uck, an die der SfxDispatcher gerade gebunden ist. Ein SfxDispatcher
    ist nur dann an SfxBindings gebunden, wenn er <UI-aktiv> ist. Ist
    er nicht UI-aktiv, wird ein 0-Pointer zur"uckgegeben.

    Der zur"uckgegebene Pointer ist nur im <unmittelbaren Kontext> des
    Methodenaufrufs g"ultig.
*/

{
    if ( pImp->pFrame )
        return &pImp->pFrame->GetBindings();
    else
        return NULL;
}

//--------------------------------------------------------------------
SfxViewFrame* SfxDispatcher::GetFrame() const

/*  [Beschreibung]

    Liefert einen Pointer auf die <SfxViewFrame> Instanz, der dieser
    SfxDispatcher geh"ort. Falls es sich um den Applikations-Dispatcher
    handelt, wird ein 0-Pointer zur"uckgegeben.
*/

{
    DBG_MEMTEST();
    return pImp->pFrame;
}

//--------------------------------------------------------------------
void SfxDispatcher::DoActivate_Impl( sal_Bool bMDI )

/*  [Beschreibung]

    Diese Methode steuert das Aktivieren eines Dispatchers.

    Da der Applikations-Dispatcher immer aktiv ist, entweder als
    Unterdispatcher des <SfxViewFrame>-Dispatchers oder selbst, wird
    er nie als ganzes Aktiviert, sondern nur seine einzelnen <SfxShell>s
    bei <SfxDispatcher::Push(SfxShell&)>.

    Beim Aktivieren eines SfxDispatchers wird an allen auf seinem
    Stack befindlichen SfxShells, beginnend mit der untersten, der Handler
    <SfxShell::Activate(sal_Bool)> gerufen.
*/

{
    DBG_MEMTEST();
    SFX_STACK(SfxDispatcher::DoActivate);
    if ( bMDI )
    {
        #ifdef DBG_UTIL
        ByteString sTemp("Activate Dispatcher ");
        sTemp += ByteString::CreateFromInt32( (sal_uInt32) this );
        DBG_TRACE(sTemp.GetBuffer());
        DBG_ASSERT( !pImp->bActive, "Activate-Fehler" );
        #endif
        pImp->bActive = sal_True;
        pImp->bUpdated = sal_False;
        SfxBindings* pBindings = GetBindings();
        if ( pBindings )
        {
            pBindings->SetDispatcher(this);
            pBindings->SetActiveFrame( pImp->pFrame->GetFrame()->GetFrameInterface() );
        }
    }
    else
    {
        #ifdef DBG_UTIL
        ByteString sTemp("Non-MDI-Activate Dispatcher");
        sTemp += ByteString::CreateFromInt32( (sal_uInt32) this );
        DBG_TRACE( sTemp.GetBuffer() );
        #endif
    }

    if ( IsAppDispatcher() )
        return;

    for ( int i = int(pImp->aStack.Count()) - 1; i >= 0; --i )
        pImp->aStack.Top( (sal_uInt16) i )->DoActivate(pImp->pFrame, bMDI);

    if ( pImp->aToDoStack.Count() )
    {
        if (sal_True)
        {
            // Kein sofortiges Update gewuenscht
            pImp->aTimer.SetTimeout(SFX_FLUSH_TIMEOUT);
            pImp->aTimer.SetTimeoutHdl( LINK(this, SfxDispatcher, EventHdl_Impl ) );
            pImp->aTimer.Start();
        }
        else
        {
            // Schnellstmoegliches Update (sollte Normalfall sein)
            pImp->aTimer.Stop();
            GetpApp()->PostUserEvent(pImp->nEventId, (void*)0);
        }
    }
}

void SfxDispatcher::DoParentActivate_Impl()
{
    for ( int i = int(pImp->aStack.Count()) - 1; i >= 0; --i )
        pImp->aStack.Top( (sal_uInt16) i )->ParentActivate();
}

//--------------------------------------------------------------------
void SfxDispatcher::DoDeactivate_Impl( sal_Bool bMDI )

/*  [Beschreibung]

    Diese Methode steuert das Deaktivieren eines Dispatchers.

    Da der Applikations-Dispatcher immer aktiv ist, entweder als
    Unterdispatcher des <SfxViewFrame>-Dispatchers oder selbst, wird
    er nie als ganzes Deaktiviert, sondern nur seine einzelnen <SfxShell>s
    bei <SfxDispatcher::Pop(SfxShell&)>.

    Beim Deaktivieren eines SfxDispatchers wird an allen auf seinem
    Stack befindlichen SfxShells, beginnend mit der obersten, der Handler
    <SfxShell::Deactivate(sal_Bool)> gerufen.
*/

{
    DBG_MEMTEST();
    SFX_STACK(SfxDispatcher::DoDeactivate);

    SfxApplication *pSfxApp = SFX_APP();

    if ( bMDI )
    {
        DBG_TRACE(ByteString("Deactivate Dispatcher ").Append(ByteString::CreateFromInt32( (sal_uInt32) this )).GetBuffer());
        DBG_ASSERT( pImp->bActive, "Deactivate-Fehler" );
        pImp->bActive = sal_False;

        if ( pImp->pFrame && !pImp->pFrame->IsA(TYPE(SfxInPlaceFrame)) )
        {
            SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame()->GetWorkWindow_Impl();
            if ( pWorkWin )
            {
                for (sal_uInt16 n=0; n<pImp->aChildWins.Count();)
                {
                    SfxChildWindow *pWin = pWorkWin->GetChildWindow_Impl( (sal_uInt16) ( pImp->aChildWins[n] & 0xFFFF ) );
                    if (!pWin || pWin && pWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT)
                        pImp->aChildWins.Remove(n);
                    else
                        n++;
                }
            }
        }
    }
    else
        DBG_TRACE( ByteString ("Non-MDI-DeActivate Dispatcher").Append(ByteString::CreateFromInt32( (sal_uInt32) this )).GetBuffer() );

    if ( IsAppDispatcher() && !pSfxApp->IsDowning() )
        return;

    for ( sal_uInt16 i = 0; i < pImp->aStack.Count(); ++i )
        pImp->aStack.Top(i)->DoDeactivate(pImp->pFrame, bMDI);

    Flush();
}

void SfxDispatcher::DoParentDeactivate_Impl()
{
    for ( int i = int(pImp->aStack.Count()) - 1; i >= 0; --i )
        pImp->aStack.Top( (sal_uInt16) i )->ParentDeactivate();
}

//--------------------------------------------------------------------
int SfxDispatcher::GetShellAndSlot_Impl
(
    sal_uInt16          nSlot,      // die zu suchende Slot-Id
    SfxShell**      ppShell,    // die SfxShell, welche nSlot z.Zt. bedient
    const SfxSlot** ppSlot,     // der SfxSlot, welcher nSlot z.Zt. bedient
    sal_Bool            bOwnShellsOnly,
    sal_Bool            bModal      // trotz ModalMode
)

/*  [Beschreibung]

    Diese Methode sucht im SfxDispatcher nach der <SfxShell>, von der
    die Slot-Id nSlot zur Zeit bedient wird. Dazu wird der Dispatcher
    zuvor geflusht.


    [R"uckgabewert]

    int                 sal_True
                        Die SfxShell wurde gefunden, ppShell und ppSlot
                        sind g"ultig.

                        sal_True
                        Die SfxShell wurde nicht gefunden, ppShell und ppSlot
                        sind ung"ultig.
*/

{
    SFX_STACK(SfxDispatcher::GetShellAndSlot_Impl);

    Flush();
    SfxSlotServer aSvr;
    if ( _FindServer(nSlot, aSvr, bModal) )
    {
        if ( bOwnShellsOnly && aSvr.GetShellLevel() >= pImp->aStack.Count() )
            return sal_False;

        *ppShell = GetShell(aSvr.GetShellLevel());
        *ppSlot = aSvr.GetSlot();
        if ( 0 == (*ppSlot)->GetExecFnc() )
            *ppSlot = (*ppShell)->GetInterface()->GetRealSlot(*ppSlot);

#ifdef DBG_UTILx
        ByteString aMsg( nSlot );
        aMsg += " found in ";
        aMsg += (*ppShell)->GetInterface()->GetClassName();
        DbgTrace( aMsg.GetBuffer() );
#endif

        return sal_True;
    }

#ifdef DBG_UTILx
    ByteString aMsg( nSlot );
    aMsg += " not found";
    DbgTrace( aMsg.GetBuffer() );
#endif

    return sal_False;
}

/*
struct Executer : public SfxHint
{
    SfxRequest *pRequest;
    const SfxSlot* pSlot;
    sal_uInt16 nLevel;

    Executer( SfxRequest* pReq, const SfxSlot* p, sal_uInt16 n )
        :  pRequest( pReq )
        , pSlot(p)
        , nLevel(n)
        {}
    ~Executer()
    {delete pRequest;}
};
*/

//--------------------------------------------------------------------
void SfxDispatcher::_Execute
(
    SfxShell&       rShell,     // zu rufende <SfxShell>
    const SfxSlot&  rSlot,      // zu rufender <SfxSlot>
    SfxRequest&     rReq,       // auszuf"uhrende Funktion (Id und optional Parameter)
    SfxCallMode     eCallMode   // synchron, asynchron oder wie beim Slot angegeben
)

/*  [Beschreibung]

    Diese Methode f"uhrt einen Request "uber einen gecachten <Slot-Server>
    aus.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( !pImp->bFlushing, "recursive call to dispatcher" );
    DBG_ASSERT( !pImp->aToDoStack.Count(), "unprepared InPlace _Execute" );

    if ( IsLocked( rSlot.GetSlotId() ) )
        return;

    sal_uInt16 nSlot = rSlot.GetSlotId();
    if ( SfxMacroConfig::IsMacroSlot( nSlot ) )
        SFX_APP()->GetMacroConfig()->RegisterSlotId( nSlot );

    if ( (eCallMode & SFX_CALLMODE_ASYNCHRON) ||
         ( !(eCallMode & SFX_CALLMODE_SYNCHRON) &&
           rSlot.IsMode(SFX_SLOT_ASYNCHRON) ) )
    {
        SfxDispatcher *pDispat = this;
        while ( pDispat )
        {
            sal_uInt16 nShellCount = pDispat->pImp->aStack.Count();
            for ( sal_uInt16 n=0; n<nShellCount; n++ )
            {
                if ( &rShell == pDispat->pImp->aStack.Top(n) )
                {
                    pDispat->pImp->xPoster->Post(new SfxRequest(rReq));
//                    pDispat->pImp->xPoster->Post(new Executer(new SfxRequest(rReq), &rSlot, n ));
                    return;
                }
            }

            pDispat = pDispat->pImp->pParent;
        }
    }
    else
        Call_Impl( rShell, rSlot, rReq, SFX_CALLMODE_RECORD==(eCallMode&SFX_CALLMODE_RECORD) );
}

//--------------------------------------------------------------------
void MappedPut_Impl( SfxAllItemSet &rSet, const SfxPoolItem &rItem )

/*  [Beschreibung]

    Hilfsfunktion zum putten von rItem unter der im Pool des Item-Sets
    rSet geltenden Which-Id.
*/

{
    // mit ggf. gemappter Which-Id putten
    const SfxItemPool *pPool = rSet.GetPool();
    sal_uInt16 nWhich = rItem.Which();
#ifdef TF_POOLABLE
    if ( pPool->IsSlot(nWhich) )
#else
    if ( pPool->HasMap() && pPool->IsSlot(nWhich) )
#endif
        nWhich = pPool->GetWhich(nWhich);
    rSet.Put( rItem, nWhich );
}

//--------------------------------------------------------------------

#ifndef SFX_USE_BINDINGS
#define SFX_USE_BINDINGS 0x8000
#endif

sal_uInt16 SfxDispatcher::ExecuteFunction( sal_uInt16 nSlot, SfxPoolItem **pArgs,
                                       sal_uInt16 nMode )
{
    if ( !nMode )
        nMode = pImp->nStandardMode;

    // via Bindings/Interceptor? (dann ist der Returnwert nicht exakt)
    sal_Bool bViaBindings = SFX_USE_BINDINGS == ( nMode & SFX_USE_BINDINGS );
    nMode &= ~sal_uInt16(SFX_USE_BINDINGS);
    if ( bViaBindings && GetBindings() )
        return GetBindings()->Execute( nSlot, (const SfxPoolItem **) pArgs, nMode )
                ? EXECUTE_POSSIBLE
                : EXECUTE_NO;

    // sonst via Dispatcher
    if ( IsLocked(nSlot) )
        return 0;
    SfxShell *pShell = 0;
    SfxCallMode eCall = SFX_CALLMODE_SYNCHRON;
    sal_uInt16 nRet = EXECUTE_NO;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False, sal_False ) )
    {
        // Ausf"uhrbarkeit vorher testen
        if ( pSlot->IsMode( SFX_SLOT_FASTCALL ) ||
            pShell->CanExecuteSlot_Impl( *pSlot ) )
                nRet = EXECUTE_POSSIBLE;

        if ( nMode == EXECUTEMODE_ASYNCHRON )
            eCall = SFX_CALLMODE_ASYNCHRON;
        else if ( nMode == EXECUTEMODE_DIALOGASYNCHRON && pSlot->IsMode( SFX_SLOT_HASDIALOG ) )
            eCall = SFX_CALLMODE_ASYNCHRON;
        else if ( pSlot->GetMode() & SFX_SLOT_ASYNCHRON )
            eCall = SFX_CALLMODE_ASYNCHRON;
        sal_Bool bDone = sal_False;
        if ( pArgs && *pArgs )
        {
            SfxAllItemSet aSet( pShell->GetPool() );
            for ( SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                MappedPut_Impl( aSet, **pArg );
            SfxRequest aReq( nSlot, eCall, aSet );
            _Execute( *pShell, *pSlot, aReq, eCall );
            bDone = aReq.IsDone();
        }
        else
        {
            SfxRequest aReq( nSlot, eCall, pShell->GetPool() );
            _Execute( *pShell, *pSlot, aReq, eCall );
            bDone = aReq.IsDone();
        }
    }

    return nRet;
}

sal_uInt16 SfxDispatcher::ExecuteFunction( sal_uInt16 nSlot, const SfxItemSet& rArgs,
                                       sal_uInt16 nMode )
{
    if ( !nMode )
        nMode = pImp->nStandardMode;

/*
    // at the moment not implemented
    // via Bindings/Interceptor? (dann ist der Returnwert nicht exakt)
    sal_Bool bViaBindings = SFX_USE_BINDINGS == ( nMode & SFX_USE_BINDINGS );
    nMode &= ~sal_uInt16(SFX_USE_BINDINGS);
    if ( bViaBindings && GetBindings() )
        return GetBindings()->Execute( nSlot, rArgs, nMode )
                ? EXECUTE_POSSIBLE
                : EXECUTE_NO;
*/
    // sonst via Dispatcher
    if ( IsLocked(nSlot) )
        return 0;
    SfxShell *pShell = 0;
    SfxCallMode eCall = SFX_CALLMODE_SYNCHRON;
    sal_uInt16 nRet = EXECUTE_NO;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False, sal_False ) )
    {
        // Ausf"uhrbarkeit vorher testen
        if ( pSlot->IsMode( SFX_SLOT_FASTCALL ) ||
            pShell->CanExecuteSlot_Impl( *pSlot ) )
                nRet = EXECUTE_POSSIBLE;

        if ( nMode == EXECUTEMODE_ASYNCHRON )
            eCall = SFX_CALLMODE_ASYNCHRON;
        else if ( nMode == EXECUTEMODE_DIALOGASYNCHRON && pSlot->IsMode( SFX_SLOT_HASDIALOG ) )
            eCall = SFX_CALLMODE_ASYNCHRON;
        else if ( pSlot->GetMode() & SFX_SLOT_ASYNCHRON )
            eCall = SFX_CALLMODE_ASYNCHRON;
        sal_Bool bDone = sal_False;
            SfxRequest aReq( nSlot, eCall, rArgs );
            _Execute( *pShell, *pSlot, aReq, eCall );
            bDone = aReq.IsDone();
    }

    return nRet;
}

sal_uInt16 SfxDispatcher::GetSlotId( const String& rCommand )
{
    // Anzahl der Shells auf den verkettenten Dispatchern z"ahlen
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.Count();
    if ( pImp->pParent )
    {
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount += pParent->pImp->aStack.Count();
            pParent = pParent->pImp->pParent;
        }
    }

    const SfxSlot *pSlot=NULL;
    sal_uInt16 nFirstShell = 0;
    for ( sal_uInt16 i = nFirstShell; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        pSlot = pIFace->GetSlot( rCommand );
        if ( pSlot )
            return pSlot->GetSlotId();
    }

    return 0;
}

//--------------------------------------------------------------------
int SfxExecuteItem::operator==( const SfxPoolItem& rItem ) const
{
    SfxExecuteItem& rArg = (SfxExecuteItem& )rItem;
    sal_uInt16 nCount = Count();
    if( nCount != rArg.Count() )
        return sal_False;
    while( nCount -- )
        if( *GetObject( nCount ) != *rArg.GetObject( nCount ) )
            return sal_False;
    return  eCall == rArg.eCall;
}

//--------------------------------------------------------------------
SfxPoolItem* SfxExecuteItem::Clone( SfxItemPool* pPool ) const
{
    return new SfxExecuteItem( *this );
}

//--------------------------------------------------------------------
SfxExecuteItem::SfxExecuteItem( const SfxExecuteItem& rArg )
    : SfxPoolItem( rArg ), nModifier( 0 )
{
    eCall = rArg.eCall;
    nSlot = rArg.nSlot;
    sal_uInt16 nCount = rArg.Count();
    for( sal_uInt16 nPos = 0; nPos < nCount; nPos++ )
        Insert( rArg[ nPos ]->Clone(), nPos );
}

//--------------------------------------------------------------------
SfxExecuteItem::SfxExecuteItem(
    sal_uInt16 nWhich, sal_uInt16 nSlotP, SfxCallMode eModeP,
    const SfxPoolItem*  pArg1, ... ) :
    SfxPoolItem( nWhich ), eCall( eModeP ), nSlot( nSlotP ), nModifier( 0 )
{
    va_list pVarArgs;
    va_start( pVarArgs, pArg1 );
    for ( const SfxPoolItem *pArg = pArg1; pArg;
          pArg = va_arg( pVarArgs, const SfxPoolItem* ) )
        Insert( pArg->Clone(), Count() );
    va_end(pVarArgs);
}

//--------------------------------------------------------------------
SfxExecuteItem::SfxExecuteItem(
    sal_uInt16 nWhich, sal_uInt16 nSlotP, SfxCallMode eModeP )
    : SfxPoolItem( nWhich ), nSlot( nSlotP ), eCall( eModeP ), nModifier( 0 )
{
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute( const SfxExecuteItem& rItem )
{
    const SfxPoolItem** pPtr = new const SfxPoolItem*[ rItem.Count() + 1 ];
    for( sal_uInt16 nPos = rItem.Count(); nPos--; )
        pPtr[ nPos ] = rItem.GetObject( nPos );
    pPtr[ rItem.Count() ] = 0;
    const SfxPoolItem* pRet = Execute(
        rItem.GetSlot(), rItem.GetCallMode(), pPtr, rItem.GetModifier() );

    __DELETE(rItem.Count() + 1) (SfxPoolItem**)pPtr;

    return pRet;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16 nSlot,               // die Id der auszufuehrenden Funktion
    SfxCallMode eCall,          // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON oder ..._SLOT
    const SfxPoolItem **pArgs,  // 0-terminiertes C-Array von Parametern
    sal_uInt16 nModi,
    const SfxPoolItem **pInternalArgs // 0-terminiertes C-Array von Parametern
)

/*  [Beschreibung]

    Methode zum Ausf"uhren eines <SfxSlot>s "uber die Slot-Id.


    [R"uckgabewert]

    const SfxPoolItem*      Pointer auf ein bis zum n"achsten Durchlauf
                            der Message-Loop g"ultiges SfxPoolItem,
                            welches den R"uckgabewert enth"alt.

                            Oder ein 0-Pointer, wenn die Funktion nicht
                            ausgef"uhrt wurde (z.B. Abbruch durch den
                            Benutzer).
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxRequest* pReq;
        if ( pArgs && *pArgs )
        {
            SfxAllItemSet aSet( pShell->GetPool() );
            for ( const SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                MappedPut_Impl( aSet, **pArg );
            pReq = new SfxRequest( nSlot, eCall, aSet );
        }
        else
            pReq =  new SfxRequest( nSlot, eCall, pShell->GetPool() );
        pReq->SetModifier( nModi );
        if( pInternalArgs && *pInternalArgs)
        {
            SfxAllItemSet aSet( SFX_APP()->GetPool() );
            for ( const SfxPoolItem **pArg = pInternalArgs; *pArg; ++pArg )
                aSet.Put( **pArg );
            pReq->SetInternalArgs_Impl( aSet );
        }
        _Execute( *pShell, *pSlot, *pReq, eCall );
        const SfxPoolItem* pRet = pReq->GetReturnValue();
        delete pReq; return pRet;
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16 nSlot,               // die Id der auszufuehrenden Funktion
    SfxCallMode eCall,          // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON oder ..._SLOT
    const SfxItemSet &rArgs     // <SfxItemSet> mit Parametern
)

/*  [Beschreibung]

    Methode zum Ausf"uhren eines <SfxSlot>s "uber die Slot-Id.


    [R"uckgabewert]

    const SfxPoolItem*      Pointer auf ein bis zum n"achsten Durchlauf
                            der Message-Loop g"ultiges SfxPoolItem,
                            welches den R"uckgabewert enth"alt.

                            Oder ein 0-Pointer, wenn die Funktion nicht
                            ausgef"uhrt wurde (z.B. Abbruch durch den
                            Benutzer).
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxAllItemSet aSet( pShell->GetPool() );
        SfxItemIter aIter(rArgs);
        for ( const SfxPoolItem *pArg = aIter.FirstItem();
              pArg;
              pArg = aIter.NextItem() )
            MappedPut_Impl( aSet, *pArg );
        SfxRequest aReq( nSlot, eCall, aSet );
        _Execute( *pShell, *pSlot, aReq, eCall );
        return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::_Execute
(
    sal_uInt16              nSlot,      // die Id der auszufuehrenden Funktion
    SfxCallMode         eCall,      // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON oder ..._SLOT
    va_list             pVarArgs,   // Parameterliste ab 2. Parameter
    const SfxPoolItem*  pArg1       // erster Parameter
)

/*  [Beschreibung]

    Methode zum Ausf"uhren eines <SfxSlot>s "uber die Slot-Id.


    [R"uckgabewert]

    const SfxPoolItem*      Pointer auf ein bis zum n"achsten Durchlauf
                            der Message-Loop g"ultiges SfxPoolItem,
                            welches den R"uckgabewert enth"alt.

                            Oder ein 0-Pointer, wenn die Funktion nicht
                            ausgef"uhrt wurde (z.B. Abbruch durch den
                            Benutzer).
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
       SfxAllItemSet aSet( pShell->GetPool() );

       for ( const SfxPoolItem *pArg = pArg1;
             pArg;
             pArg = va_arg( pVarArgs, const SfxPoolItem* ) )
           MappedPut_Impl( aSet, *pArg );

       SfxRequest aReq( nSlot, eCall, aSet );
       _Execute( *pShell, *pSlot, aReq, eCall );
       return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::Execute
(
    sal_uInt16              nSlot,      // die Id der auszufuehrenden Funktion
    SfxCallMode         eCall,      // SFX_CALLMODE_SYNCRHON, ..._ASYNCHRON oder ..._SLOT
    const SfxPoolItem*  pArg1,      // erster Parameter
    ...                             // 0-terminiertes Liste Parametern
)

/*  [Beschreibung]

    Methode zum Ausf"uhren eines <SfxSlot>s "uber die Slot-Id.


    [Anmerkung]

    Die Parameter werden kopiert, k"onnen daher als Adresse von
    Stack-Objekten "ubergeben werden.


    [R"uckgabewert]

    const SfxPoolItem*      Pointer auf ein bis zum n"achsten Durchlauf
                            der Message-Loop g"ultiges SfxPoolItem,
                            welches den R"uckgabewert enth"alt.

                            Oder ein 0-Pointer, wenn die Funktion nicht
                            ausgef"uhrt wurde (z.B. Abbruch durch den
                            Benutzer).


    [Beispiel]

    pDispatcher->Execute( SID_OPENDOCUMENT, SFX_CALLMODE_SYNCHRON,
        &SfxStringItem( SID_FILE_NAME, "\\tmp\\temp.sdd" ),
        &SfxStringItem( SID_FILTER_NAME, "StarDraw Presentation" ),
        &SfxBoolItem( SID_DOC_READONLY, sal_False ),
        0L );
*/

{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(eCall&SFX_CALLMODE_MODAL) ) )
    {
       SfxAllItemSet aSet( pShell->GetPool() );

       va_list pVarArgs;
       va_start( pVarArgs, pArg1 );
       for ( const SfxPoolItem *pArg = pArg1;
             pArg;
             pArg = va_arg( pVarArgs, const SfxPoolItem* ) )
           MappedPut_Impl( aSet, *pArg );
       va_end(pVarArgs);

       SfxRequest aReq( nSlot, eCall, aSet );
       _Execute( *pShell, *pSlot, aReq, eCall );
       return aReq.GetReturnValue();
    }
    return 0;
}

//--------------------------------------------------------------------

IMPL_LINK( SfxDispatcher, PostMsgHandler, SfxRequest*, pReq )

/*  [Beschreibung]

    Hilfsmethode zum Empfangen der asynchron auszuf"uhrenden <SfxRequest>s.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( !pImp->bFlushing, "recursive call to dispatcher" );
    SFX_STACK(SfxDispatcher::PostMsgHandler);

    // ist auch der Pool noch nicht gestorben?
//    SfxRequest* pReq = pExec->pRequest;
    if ( !pReq->IsCancelled() )
    {
        if ( !IsLocked(pReq->GetSlot()) )
        {
            Flush();
            SfxSlotServer aSvr;
            if ( _FindServer(pReq->GetSlot(), aSvr, HACK(x) sal_True ) )
//            SfxShell *pShell = GetShell(pExec->nLevel);
//            if ( pShell && pShell->GetInterface()->GetSlot( pExec->pSlot->GetSlotId() ) )
            {
                const SfxSlot *pSlot = aSvr.GetSlot();
                SfxShell *pSh = GetShell(aSvr.GetShellLevel());

                DBG( SfxApplication *pSfxApp = SFX_APP() );
                DBG( pSfxApp->EnterAsynchronCall_Impl() );

                // Wenn pSlot ein "Pseudoslot" f"ur Macros oder Verben ist, kann
                // er im Call_Impl zerst"ort werden, also nicht mehr benutzen!
                pReq->SetSynchronCall( sal_False );
                Call_Impl( *pSh, *pSlot, *pReq, sal_True ); //! woher bRecord?
//                Call_Impl( *pShell, *pExec->pSlot, *pReq, sal_True ); //! woher bRecord?
                DBG( pSfxApp->LeaveAsynchronCall_Impl() );
            }

//            delete pExec;
        }
        else
        {
//            pImp->xPoster->Post(pExec);
            pImp->xPoster->Post(new SfxRequest(*pReq));
        }
    }
//    else
//        delete pExec;

    delete pReq;
    return 0;
}
//--------------------------------------------------------------------
void SfxDispatcher::EnterAction( const String& rName )

// marks the beginning of a block of actions

{
    DBG_MEMTEST();
    Flush();
    DBG_ASSERT( pImp->aStack.Count() > 0, "EnterAction on empty dispatcher stack" );
    if ( ++pImp->nActionLevel == 1 )
    {
        SfxUndoManager *pUndoMgr = GetShell(0)->GetUndoManager();
        if ( pUndoMgr )
            pUndoMgr->EnterListAction( rName, rName HACK(RepeatComment), 0 HACK(ID) );
    }
}
//--------------------------------------------------------------------
void SfxDispatcher::LeaveAction()

// marks the end of a block of actions

{
    DBG_MEMTEST();
    DBG_ASSERT( pImp->nActionLevel > 0, "EnterAction without LeaveAction" );
    if ( --pImp->nActionLevel == 0 )
    {
        SfxUndoManager *pUndoMgr = GetShell(0)->GetUndoManager();
        if ( pUndoMgr )
            pUndoMgr->LeaveListAction();
    }
}
//--------------------------------------------------------------------
void SfxDispatcher::SetMenu_Impl()
{
    SfxApplication *pSfxApp = SFX_APP();
    if ( pImp->pFrame )
    {
        if ( !pImp->pFrame->GetViewShell() )
            return;

        SfxTopViewFrame* pTop= PTR_CAST( SfxTopViewFrame, pImp->pFrame->GetTopViewFrame() );
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxTopFrame* pFrm = pTop->GetTopFrame_Impl();
            SfxMenuBarManager* pMenuBar = pImp->pFrame->GetViewShell()->GetMenuBar_Impl();
            if ( pMenuBar )
            {
                MenuBar* pMenu = (MenuBar*) pMenuBar->GetMenu()->GetSVMenu();
                pFrm->LockResize_Impl( TRUE );
                pFrm->SetMenuBar_Impl( pMenu );
                pFrm->LockResize_Impl( FALSE );
            }
            else if ( pImp->pParent )
            {
                pImp->pParent->SetMenu_Impl();
            }
        }
    }
}

long SfxDispatcher::Update_Impl( sal_Bool bForce )

/*  [Beschreibung]

    Hilfsmethode zum Updaten der <Object-Bars> und <Object-Menus>,
    nach Bewegungen auf dem Stack.
*/

{
    SFX_STACK(SfxDispatcher::Update_Impl);

    Flush();

    if ( !pImp->pFrame )
        return 0;

    if ( pImp->bUILocked )
        return 0;

    SfxApplication *pSfxApp = SFX_APP();
    SfxDispatcher *pDisp = this;
    sal_Bool bUpdate = bForce;
    while ( pDisp && pDisp->pImp->pFrame )
    {
        SfxWorkWindow *pWork = pDisp->pImp->pFrame->GetFrame()->GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if ( pAct == pDisp || pAct == this )
        {
            if ( !bUpdate )
                bUpdate = !pDisp->pImp->bUpdated;
            pDisp->pImp->bUpdated = sal_True;
        }
        else
            break;

        pDisp = pDisp->pImp->pParent;
    }

    if ( !bUpdate || pImp->pFrame->GetFrame()->IsClosing_Impl() )
        return 0;

    SfxTopViewFrame* pTop = pImp->pFrame ? PTR_CAST( SfxTopViewFrame, pImp->pFrame->GetTopViewFrame() ) : NULL;
    sal_Bool bUIActive = pTop && pTop->GetBindings().GetDispatcher() == this;
    SfxMenuBarManager* pAppMenu = NULL;

    if ( !bUIActive && pTop && GetBindings() == &pTop->GetBindings() )
        // Eigene Tools nur intern festhalten und da"fur sorgen, da\s der
        // aktive Dispatcher sie abholt
        GetBindings()->GetDispatcher()->pImp->bUpdated = sal_False;

    // Bindings schlafen legen
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->DENTERREGISTRATIONS();

    // Test auf InPlaceFrame und speziell internes InPlace
    SfxInPlaceFrame *pIPFrame = PTR_CAST ( SfxInPlaceFrame, pImp->pFrame);
    sal_Bool bIsIPOwner = (pIPFrame && pIPFrame->GetObjectShell()->GetInPlaceObject()->GetIPClient()->Owner());
    SvInPlaceClient *pClient = pImp->pFrame ? pImp->pFrame->GetViewShell()->GetIPClient() : NULL;
    if ( bUIActive && !pIPFrame && ( !pClient || !pClient->GetProtocol().IsUIActive() ) )
    {
        SetMenu_Impl();
        pAppMenu = pImp->pFrame->GetViewShell()->GetMenuBar_Impl();
        pAppMenu->ResetObjectMenus();
    }

    // Environment
    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame()->GetWorkWindow_Impl();

    // der SfxInternalFrame oder SfxTopViewFrame, zu dem ich geh"ore
    SfxViewFrame *pAct =
        bIsIPOwner ?
        pImp->pFrame->GetParentViewFrame_Impl() :
        pImp->pFrame;

    // Ich kontrolliere die StatusBar einer Task auch wenn ich nicht aktiv bin, aber
    // zu einem internem InPlaceFrame oder einem anderen ViewFrame innerhalb der Task geh"ore
    SfxFrame *pTask = pAct ? pAct->GetFrame()->GetTopFrame() : NULL;

    SfxWorkWindow *pTaskWin = NULL;
    sal_Bool bSet = sal_True;
    if ( pImp->pFrame && pImp->pFrame->GetViewShell() )
    {
        SvInPlaceClient *pClient = pImp->pFrame->GetViewShell()->GetIPClient();
        if ( pClient && pClient->GetProtocol().IsUIActive() )
            bSet = sal_False;
    }

    if ( pTask && bSet )
    {
        pTaskWin = pTask->GetWorkWindow_Impl();
        pTaskWin->ResetStatusBar_Impl();
    }

    SfxDispatcher *pDispat = this;
    while ( pDispat )
    {
        SfxWorkWindow *pWork = pDispat->pImp->pFrame->GetFrame()->GetWorkWindow_Impl();
        SfxDispatcher *pAct = pWork->GetBindings().GetDispatcher_Impl();
        if ( pAct == pDispat || pAct == this )
        {
            pWork->ResetObjectBars_Impl();
            pWork->ResetChildWindows_Impl();
        }

        pDispat = pDispat->pImp->pParent;
    }

    sal_Bool bIsActive = sal_False;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    pDispat = this;
    while ( pActDispat && !bIsActive )
    {
        if ( pDispat == pActDispat )
            bIsActive = sal_True;
        pActDispat = pActDispat->pImp->pParent;
    }

    if ( !pIPFrame && !IsAppDispatcher() && bIsActive )
        CollectTools_Impl( pWorkWin );

    // Jetzt rekursiv die Dispatcher abklappern
    sal_uInt32 nHelpId = _Update_Impl( bUIActive, !pIPFrame, bIsIPOwner, pAppMenu, bSet ? pTaskWin : NULL );
    if ( bUIActive || bIsActive )
    {
        pWorkWin->UpdateObjectBars_Impl();

        if ( pAppMenu )
            pAppMenu->UpdateObjectMenus();

        if ( bUIActive )
        {
            // ggf. Hilfe-PI antriggern
//            SfxHelpPI* pHelpPI = pSfxApp->GetHelpPI();
//            if (pHelpPI && nHelpId)
//                pHelpPI->LoadTopic( nHelpId );
        }
    }

    if ( pTaskWin )
        pTaskWin->UpdateStatusBar_Impl();

    if ( pBindings )
        pBindings->DLEAVEREGISTRATIONS();
    return 1;
}


void SfxDispatcher::CollectTools_Impl( SfxWorkWindow* pWorkWin )
{
    // Innerhalb eines ToolSpace werden auch die Tools von nicht aktiven Frames
    // angezeigt, damit es beim Wechsel der Frames nicht zappelt
    SfxApplication *pSfxApp = SFX_APP();
    SfxToolBoxConfig *pTbxCfg = SfxToolBoxConfig::GetOrCreate();

    // Die Objectbars aller ViewFrames der aktuellen Task einsammeln
    for ( SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
            pViewFrame;
            pViewFrame = SfxViewFrame::GetNext(*pViewFrame) )
    {
        // Frames mit anderem WorkWindow interessieren hier nicht
        if ( pViewFrame->GetFrame()->GetWorkWindow_Impl() != pWorkWin )
            continue;

        // Nur weitermachen, wenn es nicht sowieso einer meiner parents ist
        SfxDispatcher *pDispat = pViewFrame->GetDispatcher();
        SfxDispatcher *pParent = this;
        while ( pParent )
        {
            if ( pParent == pDispat )
                break;
            pParent = pParent->pImp->pParent;
        }

        if ( pParent )
            continue;

        // Alle objectbars des ViewFrames vormerken
        sal_uInt16 n;
        for (n=0; n<SFX_OBJECTBAR_MAX; n++)
        {
            SfxObjectBars_Impl& rBar = pDispat->pImp->aObjBars[n];
            sal_uInt16 nId = rBar.aResId.GetId();
            if ( nId && pTbxCfg->GetAlignment(n) != SFX_ALIGN_NOALIGNMENT )
                pWorkWin->SetObjectBar_Impl( rBar.nMode,
                    rBar.aResId, rBar.pIFace, &rBar.aName );
        }

        SfxShell *pShell = pDispat->GetShell(0);
        SfxModule *pMod = pShell->GetInterface()->GetModule();
        SfxSlotPool& rSlotPool = pMod ? *pMod->GetSlotPool() : pShell->GetSlotPool_Impl();

        // ChildWindows auch "bunkern"
        for (n=0; n<pDispat->pImp->aChildWins.Count(); n++)
        {
            sal_uInt32 nId = pDispat->pImp->aChildWins[n];
            const SfxSlot *pSlot = rSlotPool.GetSlot( (sal_uInt16) nId );
            sal_uInt16 nMode = SFX_VISIBILITY_STANDARD;
            if( pSlot )
            {
                if ( pSlot->IsMode(SFX_SLOT_CONTAINER) )
                {
                    if ( !pWorkWin->IsVisible_Impl( SFX_VISIBILITY_CLIENT ) )
                        continue;
                    nMode |= SFX_VISIBILITY_CLIENT;
                }
                else
                {
                    if ( !pWorkWin->IsVisible_Impl( SFX_VISIBILITY_SERVER ) )
                        continue;
                    nMode |= SFX_VISIBILITY_SERVER;
                }
            }

            pWorkWin->SetChildWindowVisible_Impl( nId, sal_False, nMode );
        }
    }
}


sal_uInt32 SfxDispatcher::_Update_Impl( sal_Bool bUIActive, sal_Bool bIsMDIApp,
            sal_Bool bIsIPOwner, SfxMenuBarManager *pAppMenu, SfxWorkWindow *pTaskWin )
{
    sal_uInt32 nHelpId = 0L;
    SfxApplication *pSfxApp = SFX_APP();
    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame()->GetWorkWindow_Impl();
    sal_Bool bIsActive = sal_False;
    sal_Bool bIsTaskActive = sal_False;
    SfxDispatcher *pActDispat = pWorkWin->GetBindings().GetDispatcher_Impl();
    SfxDispatcher *pDispat = this;
    while ( pActDispat && !bIsActive )
    {
        if ( pDispat == pActDispat )
            bIsActive = sal_True;
        pActDispat = pActDispat->pImp->pParent;
    }

    if ( pImp->pParent && !pImp->bQuiet /* && bUIActive */ )
        // Das Men"u kommt immer vom obersten Dispatcher, also pAppMenu nicht weiterreichen
        nHelpId = pImp->pParent->_Update_Impl( bUIActive, bIsMDIApp, bIsIPOwner, NULL, pTaskWin );

    // Internen Zwischenspeicher ObjectBars und ChildWindows zur"ucksetzen
    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].aResId = ResId( 0,0 );
    pImp->aChildWins.Remove(0, pImp->aChildWins.Count());

    // bQuiet : eigene Shells weder f"ur UI noch f"ur SlotServer ber"ucksichtigen
    // bNoUI: eigene Shells f"ur das UI nicht ber"ucksichtigen
    if ( pImp->bQuiet || pImp->bNoUI || pImp->pFrame && pImp->pFrame->GetObjectShell()->IsPreview() )
        return nHelpId;

    sal_uInt16 nStatBarId=0;
    SfxShell *pStatusBarShell = NULL;

    sal_uInt16 nTotCount = pImp->aStack.Count();
    for ( sal_uInt16 nShell = nTotCount; nShell > 0; --nShell )
    {
        SfxShell *pShell = GetShell( nShell-1 );
        SfxInterface *pIFace = pShell->GetInterface();
        if (pShell->GetHelpId())
            nHelpId = pShell->GetHelpId();

        // Shells von Dispatcher mit Attribut "Hidden" oder "Quiet" auslassen
        sal_Bool bReadOnlyShell = IsReadOnlyShell_Impl( nShell-1 );
        SfxSlotPool& rSlotPool = pIFace->GetModule() ?
            *pIFace->GetModule()->GetSlotPool() : pShell->GetSlotPool_Impl();

        sal_uInt16 nNo;
        for ( nNo = 0; pIFace && nNo<pIFace->GetObjectBarCount(); ++nNo )
        {
            sal_uInt16 nPos = pIFace->GetObjectBarPos(nNo);
            if ( bReadOnlyShell && !( nPos & SFX_VISIBILITY_READONLYDOC ) )
                continue;

            // Soll die ObjectBar nur angezeigt werden, wenn die Shell
            // einen bestimmten UI-Modus hat?
            sal_uInt32 nFeature = pIFace->GetObjectBarFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // Auf jeden Fall eintragen, auch wenn unsichtbar. Dann kann
            // WorkWindow anbieten, wieder anzuschalten
            sal_Bool bVisible = pIFace->IsObjectBarVisible(nNo);
            if ( !bVisible )
                nPos &= SFX_POSITION_MASK;

            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos & SFX_POSITION_MASK];
            rBar.nMode = nPos;
            rBar.aResId = pIFace->GetObjectBarResId(nNo);
            const String *pName = pIFace->GetObjectBarName(nNo);
            if ( pName )
                rBar.aName = *pName;
            else
                rBar.aName.Erase();
            rBar.pIFace = pIFace;

            if ( bUIActive || bIsActive )
            {
                pWorkWin->SetObjectBar_Impl(
                    nPos, rBar.aResId, rBar.pIFace, &rBar.aName );
            }

            if ( !bVisible )
                rBar.aResId = ResId( 0,0 );
        }

        for ( nNo=0; pIFace && nNo<pIFace->GetChildWindowCount(); nNo++ )
        {
            sal_uInt32 nId = pIFace->GetChildWindowId(nNo);
            const SfxSlot *pSlot = rSlotPool.GetSlot( (sal_uInt16) nId );
            DBG_ASSERT( pSlot, "Da fehlt ein Childwindow-Slot!");
            if ( bReadOnlyShell )
            {
                // Bei read only documents nur solche ChildWindows anzeigen,
                // die daf"ur freigegeben sind
                if ( pSlot && !pSlot->IsMode( SFX_SLOT_READONLYDOC ) )
                    continue;
            }

            sal_uInt32 nFeature = pIFace->GetChildWindowFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // Bei den anderen auf CONTAINER/SERVER-Slots abfragen
            sal_uInt16 nMode = SFX_VISIBILITY_STANDARD;
            if( pSlot )
            {
                if ( pSlot->IsMode(SFX_SLOT_CONTAINER) )
                {
                    if ( !pWorkWin->IsVisible_Impl( SFX_VISIBILITY_CLIENT ) )
                        continue;
                    nMode |= SFX_VISIBILITY_CLIENT;
                }
                else
                {
                    if ( !pWorkWin->IsVisible_Impl( SFX_VISIBILITY_SERVER ) )
                        continue;
                    nMode |= SFX_VISIBILITY_SERVER;
                }
            }

            if ( bUIActive || bIsActive )
                pWorkWin->SetChildWindowVisible_Impl( nId, sal_True, nMode );
            if ( bUIActive || bIsActive || !pWorkWin->IsFloating( (sal_uInt16) ( nId & 0xFFFF ) ) )
                pImp->aChildWins.Insert( nId, pImp->aChildWins.Count());
        }

        if ( pAppMenu )
        {
            // update Object-Menus
            for ( nNo = 0; pIFace && nNo<pIFace->GetObjectMenuCount(); ++nNo )
                pAppMenu->SetObjectMenu( pIFace->GetObjectMenuPos(nNo),
                                            pIFace->GetObjectMenuResId(nNo));
        }

        if ( bIsMDIApp || bIsIPOwner )
        {
            // Bei MDIEnvironment oder internem InPlace Statuszeile setzen
            sal_uInt16 nId = pIFace->GetStatusBarResId().GetId();
            if ( nId )
            {
                nStatBarId = nId;
                pStatusBarShell =  pShell;
            }
        }
    }

    for ( sal_uInt16 nPos=0; nPos<SFX_OBJECTBAR_MAX; nPos++ )
    {
        SfxObjectBars_Impl& rFixed = pImp->aFixedObjBars[nPos];
        if ( rFixed.aResId.GetId() )
        {
            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos];
            rBar = rFixed;
            pWorkWin->SetObjectBar_Impl( rFixed.nMode,
                rFixed.aResId, rFixed.pIFace, &rFixed.aName );
        }
    }

    if ( pTaskWin && ( bIsMDIApp || bIsIPOwner ) )
    {
        SfxDispatcher *pActDispat = pTaskWin->GetBindings().GetDispatcher_Impl();
        SfxDispatcher *pDispat = this;
        if ( bIsIPOwner && pImp->pFrame )
            pDispat = pImp->pFrame->GetParentViewFrame_Impl()->GetDispatcher();

        while ( pActDispat && !bIsTaskActive )
        {
            if ( pDispat == pActDispat )
                bIsTaskActive = sal_True;
            pActDispat = pActDispat->pImp->pParent;
        }

        if ( bIsTaskActive && nStatBarId && pImp->pFrame )
        {
            // Ich kontrolliere die StatusBar einer Task auch wenn ich nicht aktiv bin, aber
            // zu einem internem InPlaceFrame oder einem anderen ViewFrame innerhalb der Task geh"ore
            SfxBindings& rBindings = pImp->pFrame->GetBindings();
            pTaskWin->SetStatusBar_Impl( nStatBarId, pStatusBarShell, rBindings );
        }
    }

    return nHelpId;
}

//--------------------------------------------------------------------
long SfxDispatcher::UpdateObjectMenus_Impl( SfxMenuBarManager *pMenuMgr )

/*  [Beschreibung]

    Hilfsmethode zum Updaten der <Object-Menus>.
*/

{
    SFX_STACK(SfxDispatcher::UpdateObjecteMenus_Impl);

    Flush();

    if (!pImp->pFrame && !IsAppDispatcher())
    {
        DBG_ERROR( "Dispatcher ohne Frame aber mit Parent?!?" );
        return 0;
    }

    // Bindings und App besorgen
    SfxApplication *pSfxApp = SFX_APP();
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->DENTERREGISTRATIONS();
    pMenuMgr->ResetObjectMenus();

    sal_uInt16 nTotCount = pImp->aStack.Count();
    sal_uInt16 nMinCount = pImp->bQuiet || pImp->bNoUI ? nTotCount : 0;

    if ( pImp->pParent ) // && !(pImp->pFrame->GetFrameType() & SFXFRAME_PLUGIN ) )
    {
        // Parents nur bei PlugIns nicht ber"ucksichtigen
        // (AppDispatcher oder parent frame)
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount += pParent->pImp->aStack.Count();
            if ( pParent->pImp->bQuiet || pParent->pImp->bNoUI )
                nMinCount = nTotCount;
            pParent = pParent->pImp->pParent;
        }
    }

    for ( sal_uInt16 nShell = nTotCount; nShell > nMinCount; --nShell )
    {
        SfxShell *pShell = GetShell( nShell-1 );
        const SfxInterface *pIFace = pShell->GetInterface();

        // update Object-Menus
        for ( sal_uInt16 nNo = 0; pIFace && nNo<pIFace->GetObjectMenuCount(); ++nNo )
            pMenuMgr->SetObjectMenu( pIFace->GetObjectMenuPos(nNo),
                                     pIFace->GetObjectMenuResId(nNo));
    }

    pMenuMgr->UpdateObjectMenus();
    if ( pBindings )
        pBindings->DLEAVEREGISTRATIONS();
    return 0;
}

//--------------------------------------------------------------------
void SfxDispatcher::FlushImpl()

/*  [Beschreibung]

    Hilfsmethode zum Ausf"uhren der ausstehenden Push- und Pop-Befehle.
*/

{
    DBG_PROFSTART(SfxDispatcherFlush);
    DBG_MEMTEST();
    SFX_STACK(SfxDispatcher::FlushImpl);


#ifdef DBG_UTIL
    ByteString aMsg( "SfxDispatcher(" );
    aMsg += ByteString::CreateFromInt32( (long) this );
    aMsg += ")::Flush()";
#endif

    pImp->aTimer.Stop();

    if ( pImp->pParent )
        pImp->pParent->Flush();

//  if ( pImp->bQuiet )
//      return;

    pImp->bFlushing = !pImp->bFlushing;
    if ( !pImp->bFlushing )
    {
        pImp->bFlushing = sal_True;
        DBG_PROFSTOP(SfxDispatcherFlush);
//!
#ifdef DBG_UTIL_MESSEHACK_AUSKOMMENT
        DBG_ERROR( "reentering SfxDispatcher::Flush()" );
        aMsg += " reentering, aborted";
        DbgTrace( aMsg.GetBuffer() );
#endif
        return;
    }

    SfxApplication *pSfxApp = SFX_APP();

    // in der 1. Runde den echten Stack 'um'bauen
    SfxToDoStack_Impl aToDoCopy;
    sal_Bool bModify = sal_False;
    short nToDo;
    for ( nToDo = pImp->aToDoStack.Count()-1; nToDo >= 0; --nToDo )
    {
        bModify = sal_True;

        SfxToDo_Impl aToDo( pImp->aToDoStack.Top(nToDo) );
        if ( aToDo.bPush )
        {
            // tats"aechlich pushen
            DBG_ASSERT( !pImp->aStack.Contains( aToDo.pCluster ),
                        "pushed SfxShell already on stack" );
            pImp->aStack.Push( aToDo.pCluster );
            aToDo.pCluster->SetDisableFlags( pImp->nDisableFlags );

            // die bewegte Shell merken
            aToDoCopy.Push( aToDo );
        }
        else
        {
            // tats"aechlich poppen
            SfxShell* pPopped = 0;
            FASTBOOL bFound = sal_False;
            do
            {
                DBG_ASSERT( pImp->aStack.Count(), "popping from empty stack" );
                pPopped = pImp->aStack.Pop();
                pPopped->SetDisableFlags( 0 );
                bFound = pPopped == aToDo.pCluster;

                // die bewegte Shell merken
                aToDoCopy.Push( SfxToDo_Impl( sal_False, aToDo.bDelete, sal_False, *pPopped) );
            }
            while ( aToDo.bUntil && !bFound );
            DBG_ASSERT( bFound, "wrong SfxShell popped" );
        }

        if ( nToDo == 0 )
            pImp->aToDoStack.Clear();
    }

    // ggf. Bindings invalidieren
    if ( !pSfxApp->IsDowning() )
    {
        if ( bModify )
        {
            pImp->pCachedServ1 = 0;
            pImp->pCachedServ2 = 0;
        }

        InvalidateBindings_Impl( bModify );
    }

    pImp->bFlushing = sal_False;
    pImp->bUpdated = sal_False; // nicht nur bei bModify, falls Doc/Template-Config
    bFlushed = sal_True;

    // in der 2. Runde die Shells aktivieren und ggf. l"oschen
    for ( nToDo = aToDoCopy.Count()-1; nToDo >= 0; --nToDo )
    {
        SfxToDo_Impl aToDo( aToDoCopy.Top(nToDo) );
        if ( aToDo.bPush )
        {
            if ( pImp->bActive )
                aToDo.pCluster->DoActivate(pImp->pFrame, sal_True);
        }
        else
            if ( pImp->bActive )
                aToDo.pCluster->DoDeactivate(pImp->pFrame, sal_True);
    }
    for ( nToDo = aToDoCopy.Count()-1; nToDo >= 0; --nToDo )
    {
        SfxToDo_Impl aToDo( aToDoCopy.Top(nToDo) );
        if ( aToDo.bDelete ) delete aToDo.pCluster;
    }
    sal_Bool bAwakeBindings = aToDoCopy.Count() != 0;
    if( bAwakeBindings )
        aToDoCopy.Clear();

    // Wenn bei Activate/Deactivate/Delete weitere Stackbewegungen erfolgt sind :
    if (!bFlushed)
        // falls jemand Push/Pop gerufen hat, wurde auch EnterReg gerufen!
        FlushImpl();

    if( bAwakeBindings && GetBindings() )
        GetBindings()->DLEAVEREGISTRATIONS();
    DBG_PROFSTOP(SfxDispatcherFlush);

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aFixedObjBars[n].aResId = ResId( 0,0 );

#ifdef DBG_UTIL
    aMsg += " done";
    DbgTrace( aMsg.GetBuffer() );
#endif
}

//--------------------------------------------------------------------
void SfxDispatcher::SetSlotFilter
(
    HACK(hier muss mal ein enum rein)
    sal_Bool            bEnable,    /*  sal_True:
                                    nur angegebene Slots enablen,
                                    alle anderen disablen

                                    sal_False:
                                    die angegebenen Slots disablen,
                                    alle anderen zun"achst enablen
                                */
    sal_uInt16          nCount,     //  Anzahl der SIDs im folgenden Array
    const sal_uInt16*   pSIDs       //  sortiertes Array von 'nCount' SIDs
)

/*  [Beschreibung]

    Mit dieser Methode wird ein Filter gesetzt, mit dem gezielt Slots
    enabled bzw. disabled werden k"onnen. Das "ubergebene Array mu\s
    bis zum Dtor bzw. n"achsten <SetSlotFilter()> erhalten bleiben, es
    wird nicht vom Dispatcher gel"oscht, es kann daher static sein.

    In ReadOnly-Dokumenten kann man mit 'bEnable==2' quasi das ReadOnlyDoc
    Flag von Slots "ubersteuern, dieser werden also wieder gefunden. Auf
    andere Slots hat das dann keine Auswirkung.


    [Beispiel]

    gezieltes disablen der Slots 1, 2 und 3:

        static sal_uInt16 __READONLY_DATA pSIDs[] = { 1, 2, 3 };
        pDisp->SetSlotFilter( sal_False, sizeof(pSIDs)/sizeof(sal_uInt16), pSIDs );

    nur die Slots 5, 6 und 7 zulassen:

        static sal_uInt16 __READONLY_DATA pSIDs[] = { 5, 6, 7 };
        pDisp->SetSlotFilter( sal_True, sizeof(pSIDs)/sizeof(sal_uInt16), pSIDs );

    Filter ausschalten:

        pDisp->SetSlotFilter();
*/

{
#ifdef DBG_UTIL
    // Array "uberpr"ufen
    for ( sal_uInt16 n = 1; n < nCount; ++n )
        DBG_ASSERT( pSIDs[n] > pSIDs[n-1], "SetSlotFilter: SIDs not sortet" );
#endif

    if ( pImp->pFilterSIDs )
        pImp->pFilterSIDs = 0;

    pImp->bFilterEnabling = bEnable;
    pImp->nFilterCount = nCount;
    pImp->pFilterSIDs = pSIDs;

    GetBindings()->InvalidateAll(sal_True);
}

//--------------------------------------------------------------------
EXTERN_C
#if defined( PM2 ) && (!defined( CSET ) && !defined ( MTW ) && !defined( WTC ))
int _stdcall
#else
#ifdef WNT
int _cdecl
#else
int
#endif
#endif

SfxCompareSIDs_Impl( const void* pSmaller, const void* pBigger )
{
    DBG_MEMTEST();
    return ( (long) *((sal_uInt16*)pSmaller) ) - ( (long) *((sal_uInt16*)pBigger) );
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::IsSlotEnabledByFilter_Impl( sal_uInt16 nSID ) const

/*  [Beschreibung]

    Sucht 'nSID' in dem mit <SetSlotFilter()> gesetzten Filter und
    liefert sal_True, falls die SID erlaubt ist, bzw. sal_False, falls sie
    durch den Filter disabled ist.

    [R"uckgabewert]
    sal_Bool                0   =>  disabled
                        1   =>  enabled
                        2   =>  enabled even if ReadOnlyDoc
*/

{
    // kein Filter?
    if ( 0 == pImp->nFilterCount )
        // => alle SIDs erlaubt
        return sal_True;

    // suchen
    sal_Bool bFound = 0 != bsearch( &nSID, pImp->pFilterSIDs, pImp->nFilterCount,
                                sizeof(sal_uInt16), SfxCompareSIDs_Impl );

    // even if ReadOnlyDoc
    if ( 2 == pImp->bFilterEnabling )
        return bFound ? 2 : 1;
    // sonst je nach Negativ/Positiv Filter
    return pImp->bFilterEnabling ? bFound : !bFound;
}

//--------------------------------------------------------------------
sal_Bool SfxDispatcher::_TryIntercept_Impl
(
    sal_uInt16              nSlot,      // zu suchende Slot-Id
    SfxSlotServer&      rServer,    // zu f"uellende <SfxSlotServer>-Instanz
    sal_Bool                bSelf
)
{
    // Eventuell geh"ort der parent auch zu einer Komponente
    SfxDispatcher *pParent = pImp->pParent;
    sal_uInt16 nLevels = pImp->aStack.Count();
    while ( pParent && pParent->pImp->pFrame )
    {
        if ( pParent->pImp->pFrame->GetFrame()->HasComponent() )
        {
            // Components d"urfen intercepten
            if ( pParent->_TryIntercept_Impl( nSlot, rServer, sal_True ) )
            {
                // Die eigenen Shells zum Shelllevel hinzuz"ahlen
                rServer.SetShellLevel( rServer.GetShellLevel() + nLevels );
                return sal_True;
            }
            else
                // Keine weitere Interception
                break;
        }
        else
            nLevels += pParent->pImp->aStack.Count();

        pParent = pParent->pImp->pParent;
    }

    if ( bSelf )
    {
        // Die ComponentViewShell befragen
        Flush();
        SfxShell *pObjShell = GetShell(0);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);

        if ( pSlot )
        {
            rServer.SetSlot(pSlot);
            rServer.SetShellLevel(0);
#ifdef DBG_UTILx
            String aMsg( nSlot );
            aMsg += " intercepted";
            DbgTrace( aMsg.GetBuffer() );
#endif
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool SfxDispatcher::_FindServer
(
    sal_uInt16              nSlot,      // zu suchende Slot-Id
    SfxSlotServer&   rServer,    // zu f"uellnde <SfxSlotServer>-Instanz
    sal_Bool                bModal      // trotz ModalMode
)

/*  [Beschreibung]

    Diese Hilfsmethode sucht den <Slot-Server>, der nSlot zur Zeit bedient.
    Als Ergebnis wird rServer entsprechend gef"ullt.

    Falls bekannt, kann das SfxInterface mitgegeben werden, von welchem
    nSlot momentan bedient wird.

    Vor der Suche nach nSlot wird der SfxDispatcher geflusht.


    [R"uckgabewert]

    sal_Bool                sal_True
                        Der Slot wurde gefunden, rServer ist g"ultig.

                        sal_False
                        Der Slot wird momentan nicht bedient, rServer
                        ist ung"ultig.
*/

{
    SFX_STACK(SfxDispatcher::_FindServer);

    // Dispatcher gelockt? (SID_BROWSE_STOP und SID_HELP_PI trotzdem durchlassen)
    SfxApplication *pSfxApp = SFX_APP();
    sal_Bool bAllLocked = pSfxApp->IsDispatcherLocked();
    if ( IsLocked(nSlot) )
    {
        if ( bAllLocked )
            pSfxApp->Get_Impl()->bInvalidateOnUnlock = sal_True;
        pImp->bInvalidateOnUnlock = sal_True;
        return sal_False;
    }

    // Anzahl der Shells auf den verkettenten Dispatchern z"ahlen
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.Count();
    if ( pImp->pParent )
    {
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount += pParent->pImp->aStack.Count();
            pParent = pParent->pImp->pParent;
        }
    }

    // Makro-Slot?
    if ( SfxMacroConfig::IsMacroSlot( nSlot ) )
    {
        rServer.SetShellLevel(nTotCount-1);
        rServer.SetSlot(pSfxApp->GetMacroConfig()->GetMacroInfo(nSlot)->GetSlot());
        return sal_True;
    }
    // Verb-Slot?
    else if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        SfxShell *pSh = 0;
        for ( sal_uInt16 nShell = 0; pSh = GetShell(nShell); ++nShell )
        {
            if ( pSh->ISA(SfxViewShell) )
            {
                rServer.SetShellLevel(nShell);
                rServer.SetSlot(pSh->GetVerbSlot_Impl(nSlot));
                return sal_True;
            }
        }

        return sal_False;
    }

    // SID gegen gesetzten Filter pr"ufen
    sal_uInt16 nSlotEnableMode=0;
    if ( pImp->pFrame )
    {
        nSlotEnableMode = IsSlotEnabledByFilter_Impl( nSlot );
        if ( 0 == nSlotEnableMode )
            return sal_False;
    }

    // im Quiet-Mode nur Parent-Dispatcher
    if ( pImp->bQuiet )
    {
        if ( pImp->pParent )
        {
            sal_Bool bRet = pImp->pParent->_FindServer( nSlot, rServer, bModal );
            rServer.SetShellLevel
                ( rServer.GetShellLevel() + pImp->aStack.Count() );
            return bRet;
        }
        else
            return sal_False;
    }

    sal_Bool bReadOnly = ( 2 != nSlotEnableMode && pImp->bReadOnly ) ||
                ( pImp->pFrame && pImp->pFrame->GetObjectShell() &&
                  pImp->pFrame->GetObjectShell()->IsLoading() );

    // durch alle Shells der verketteten Dispatchern von oben nach unten suchen
#ifdef DBG_UTILx
    String aStack( "Stack:" );
#endif
    sal_uInt16 nFirstShell = pImp->bModal && !bModal ? pImp->aStack.Count() : 0;
    for ( sal_uInt16 i = nFirstShell; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);

        if ( pSlot && pSlot->nDisableFlags && ( pSlot->nDisableFlags & pObjShell->GetDisableFlags() ) != 0 )
            return sal_False;

        if ( pSlot && !( pSlot->nFlags & SFX_SLOT_READONLYDOC ) && bReadOnly )
            return sal_False;

        if ( pSlot )
        {
            // Slot geh"ort zum Container?
            FASTBOOL bIsContainerSlot = pSlot->IsMode(SFX_SLOT_CONTAINER);
            FASTBOOL bIsInPlace = pImp->pFrame && pImp->pFrame->ISA( SfxInPlaceFrame );

            // Shell geh"ort zum Server?
            // AppDispatcher oder IPFrame-Dispatcher
            FASTBOOL bIsServerShell = !pImp->pFrame || bIsInPlace;

            // Nat"urlich sind ServerShell-Slots auch ausf"uhrbar, wenn sie auf
            // einem Container-Dispatcher ohne IPClient ausgef"uhrt werden sollen.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = pImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetIPClient();
            }

            // Shell geh"ort zum Container?
            // AppDispatcher oder kein IPFrameDispatcher
            FASTBOOL bIsContainerShell = !pImp->pFrame || !bIsInPlace;
#ifdef MBA
            if ( !bIsContainerShell )
            {
                // Bei internem InPlace auch den AppDispatcher zug"angig machen
                if ( pImp->pFrame && pImp->pFrame->GetParentViewFrame_Impl() )
                {
                    if ( i == nTotCount - 1 )
                        bIsContainerShell = sal_True;
                }
            }
#endif
            // Shell und Slot passen zusammen
            if ( !( ( bIsContainerSlot && bIsContainerShell ) ||
                    ( !bIsContainerSlot && bIsServerShell ) ) )
                pSlot = 0;
        }

#ifdef DBG_UTILx
        if ( pSlot )
        {
            String aMsg( nSlot );
            aMsg += " found in ";
            aMsg += pObjShell->GetInterface()->GetClassName();
            DbgTrace( aMsg.GetBuffer() );
        }
        else
        {
            aStack += " ";
            aStack += pObjShell->GetInterface()->GetClassName();
        }
#endif
        if ( pSlot && !IsAllowed( nSlot ) )
        {
            pSlot = NULL;
        }

        if ( pSlot )
        {
            rServer.SetSlot(pSlot);
            rServer.SetShellLevel(i);
            return sal_True;
        }
    }

#ifdef DBG_UTILx
    String aMsg( nSlot );
    aMsg += " not found in ";
    aMsg += aStack;
    DbgTrace( aMsg.GetBuffer() );
#endif
    return sal_False;
}
//--------------------------------------------------------------------
sal_Bool SfxDispatcher::_FillState
(
    const SfxSlotServer& rSvr,       // abzufragende <Slot-Server>
    SfxItemSet&             rState,     // zu f"ullendes <SfxItemSet>
    const SfxSlot*          pRealSlot   // ggf. der tats"achliche Slot
)

/*  [Beschreibung]

    Hilfsmethode zum Abfragen des Status des <Slot-Server>s rSvr.
    In rState m"ussen die gew"unschten Slots-Ids (teilweise in Which-Ids
    des betreffenden Pools umgewandelt) vorhanden sein.

    Der SfxDispatcher wird vor der Abfrage geflusht.
*/

{
    SFX_STACK(SfxDispatcher::_FillState);

    DBG_PROFSTART(SfxDispatcherFillState);

    const SfxSlot *pSlot = rSvr.GetSlot();
    SfxApplication *pSfxApp = SFX_APP();
    sal_Bool bAllLocked = pSfxApp->IsDispatcherLocked();
    if ( pSlot && IsLocked( pSlot->GetSlotId() ) )
    {
        if ( bAllLocked )
            pSfxApp->Get_Impl()->bInvalidateOnUnlock = sal_True;
        pImp->bInvalidateOnUnlock = sal_True;
        DBG_PROFSTOP(SfxDispatcherFillState);
        return sal_False;
    }

    if ( pSlot )
    {
        Flush();

        // Objekt ermitteln und Message an diesem Objekt aufrufen
        SfxShell *pSh = GetShell(rSvr.GetShellLevel());
        DBG_ASSERT(pSh, "ObjektShell nicht gefunden");

        SfxStateFunc pFunc;

        if (pRealSlot)
            pFunc = pRealSlot->GetStateFnc();
        else
            pFunc = pSlot->GetStateFnc();

        pSh->CallState( pFunc, rState );
#ifdef DBG_UTIL
        // pr"ufen, ob IDL (SlotMap) und aktuelle Items "ubereinstimmen
        if ( DbgIsAssertWarning() && rState.Count() )
        {
            SfxInterface *pIF = pSh->GetInterface();
            SfxItemIter aIter( rState );
            for ( const SfxPoolItem *pItem = aIter.FirstItem();
                  pItem;
                  pItem = aIter.NextItem() )
                if ( !IsInvalidItem(pItem) && !pItem->ISA(SfxVoidItem) )
                {
                    sal_uInt16 nSlotId = rState.GetPool()->GetSlotId(pItem->Which());
                    if ( !pItem->IsA(pIF->GetSlot(nSlotId)->GetType()->Type()) )
                    {
                        ByteString aMsg( "item-type unequal to IDL (=> no BASIC)" );
                        aMsg += "\nwith SID: ";
                        aMsg += ByteString::CreateFromInt32( nSlotId );
                        aMsg += "\nin ";
                        aMsg += pIF->GetClassName();
                        DbgOut( aMsg.GetBuffer(), DBG_OUT_ERROR, __FILE__, __LINE__);
                    }
                }
        }
#endif

        DBG_PROFSTOP(SfxDispatcherFillState);
        return sal_True;
    }

    DBG_PROFSTOP(SfxDispatcherFillState);
    return sal_False;
}

//--------------------------------------------------------------------
const SfxPoolItem* SfxDispatcher::_Execute( const SfxSlotServer &rSvr )

/*  [Beschreibung]

    Diese Methode f"uhrt einen Request "uber einen gecachten <Slot-Server>
    aus.
*/

{
    const SfxSlot *pSlot = rSvr.GetSlot();
    if ( IsLocked( pSlot->GetSlotId() ) )
        return 0;

    if ( pSlot )
    {
        Flush();

        sal_uInt16 nSlot = pSlot->GetSlotId();
        if ( SfxMacroConfig::IsMacroSlot( nSlot ) )
            SFX_APP()->GetMacroConfig()->RegisterSlotId( nSlot );

        if ( pSlot->IsMode(SFX_SLOT_ASYNCHRON) )
            //! ignoriert rSvr
        {
            SfxShell *pShell = GetShell( rSvr.GetShellLevel() );
            SfxDispatcher *pDispat = this;
            while ( pDispat )
            {
                sal_uInt16 nShellCount = pDispat->pImp->aStack.Count();
                for ( sal_uInt16 n=0; n<nShellCount; n++ )
                    if ( pShell == pDispat->pImp->aStack.Top(n) )
                    {
                        pDispat->pImp->xPoster->Post(
                            new SfxRequest( pSlot->GetSlotId(),
                                SFX_CALLMODE_RECORD, pShell->GetPool() ) );
//                        pDispat->pImp->xPoster->Post(new Executer(
//                                new SfxRequest( pSlot->GetSlotId(),
//                                    SFX_CALLMODE_RECORD, pShell->GetPool() ),
//                                pSlot, n ));
                        return 0;
                    }
            }
        }
        else
        {
            // Objekt ermitteln und Message an diesem Objekt aufrufen
            SfxShell *pSh = GetShell(rSvr.GetShellLevel());
            SfxRequest aReq( pSlot->GetSlotId(), SFX_CALLMODE_RECORD, pSh->GetPool() );
            if ( Call_Impl( *pSh, *pSlot, aReq, sal_True ) ) // von Bindings immer recorden
                return aReq.GetReturnValue();
        }
    }
    return 0;
}

//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( sal_uInt16 nConfigId,
                                  Window *pWin, const Point *pPos,
                                  const SfxPoolItem *pArg1, ... )

/*  [Beschreibung]

    Diese Methode f"uhrt das unter der Id nConfigId registrierte PopupMenu
    aus. Die Regisitrierung wird i.d.R. in <SFX_IMPL_INTERFACE> vorgenommen.

    Bei nConfigId == 0 werden die auf dem Stack des Dispatchers befindlichen
    <SfxShell>s von oben nach unten nach dem ersten registrieren PopupMenu
    durchsucht.
*/

{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;

    const SvVerbList *pVerbList = 0;

    // Nur die eigenen Shells nehmen !!
    sal_uInt16 nMaxShellLevel = rDisp.pImp->aStack.Count();

    SfxShell *pSh;
    for ( pSh = rDisp.GetShell(nShLevel);
          pSh && nShLevel < nMaxShellLevel ;
          ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {

        if (pSh->GetVerbs())
        {
            pVerbList = pSh->GetVerbs();
            break;
        }
    }

    nShLevel=0;

    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.Count();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame()->GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) ||
             ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
            SfxPopupMenuManager aPop(rResId.GetId(), *rDisp.GetBindings() );
            aPop.SetResMgr(rResId.GetResMgr());
            aPop.AddClipboardFunctions();
            aPop.Initialize();

            if (pVerbList && pVerbList->Count())
                aPop.InsertVerbs(pVerbList);

            aPop.RemoveDisabledEntries();
            sal_uInt16 nRetId = aPop.Execute(
                    pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
            return;
        }
    }
}

//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( sal_uInt16 nConfigId
                                , Window *pWin, const Point *pPos
                                    )

/*  [Beschreibung]

    Diese Methode f"uhrt das unter der Id nConfigId registrierte PopupMenu
    aus. Die Regisitrierung wird i.d.R. in <SFX_IMPL_INTERFACE> vorgenommen.

    Bei nConfigId == 0 werden die auf dem Stack des Dispatchers befindlichen
    <SfxShell>s von oben nach unten nach dem ersten registrieren PopupMenu
    durchsucht.
*/

{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    const SvVerbList *pVerbList = 0;

    // Nur die eigenen Shells nehmen !!
    sal_uInt16 nMaxShellLevel = rDisp.pImp->aStack.Count();

    SfxShell *pSh;
    for ( pSh = rDisp.GetShell(nShLevel);
          pSh && nShLevel < nMaxShellLevel ;
          ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {

        if (pSh->GetVerbs())
        {
            pVerbList = pSh->GetVerbs();
            break;
        }
    }

    nShLevel=0;

    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.Count();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame()->GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {

        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) ||
             ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
            SfxPopupMenuManager aPop(rResId.GetId(), *rDisp.GetBindings() );
            aPop.SetResMgr(rResId.GetResMgr());
            aPop.AddClipboardFunctions();
            aPop.Initialize();
            if (pVerbList && pVerbList->Count())
                aPop.InsertVerbs(pVerbList);

            aPop.RemoveDisabledEntries();
            sal_uInt16 nRetId = aPop.Execute(
                    pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
            return;
        }
    }
}

//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( const ResId &rId
                                , Window *pWin, const Point *pPos
                                    )

/*  [Beschreibung]

    Diese Methode l"adt ein PopupMenu aus der angegebenen Resource und
    f"uhrt es aus. Die Selektion wird "uber diesen SfxDispatcher ausgef"uhrt.
*/

{
    Window *pWindow = pWin ? pWin : pImp->pFrame->GetFrame()->GetWorkWindow_Impl()->GetWindow();
    SfxPopupMenuManager aPop( rId, *GetBindings() );
    aPop.AddClipboardFunctions();
    aPop.Initialize();
    aPop.RemoveDisabledEntries();
    sal_uInt16 nRetId = aPop.Execute( pPos ? *pPos : pWindow->GetPointerPosPixel(),
                                  pWindow );
}

//--------------------------------------------------------------------
void SfxDispatcher::Lock( sal_Bool bLock )

/*  [Beschreibung]

    Mit dieser Methode kann der SfxDispatcher gesperrt und freigegeben
    werden. Ein gesperrter SfxDispatcher f"uhrt keine <SfxRequest>s mehr
    aus und liefert keine Status-Informationen mehr. Er verh"alt sich
    so als w"aren alle Slots disabled.
*/

{
    SfxBindings* pBindings = GetBindings();
    if ( !bLock && pImp->bLocked && pImp->bInvalidateOnUnlock )
    {
        if ( pBindings )
            pBindings->InvalidateAll(sal_True);
        pImp->bInvalidateOnUnlock = sal_False;
    }
    else if ( pBindings )
        pBindings->InvalidateAll(sal_False);
    pImp->bLocked = bLock;
}

//--------------------------------------------------------------------
void SfxDispatcher::ShowObjectBar(sal_uInt16 nId, SfxShell *pShell) const

/*  [Beschreibung]

    Mit dieser Methode kann auf einer Objektleistenposition gezielt eine
    bestimmte Objektleiste eingeblendet werden.
*/

{
    ResId aResId(nId);
    sal_uInt16 nPos = USHRT_MAX;
    sal_uInt16 nNo;
    SfxInterface *pIFace;

    if ( pShell )
    {
        // Nur in der Shell suchen
        pIFace = pShell->GetInterface();
        for ( nNo=0; nNo<pIFace->GetObjectBarCount(); nNo++ )
        {
            if (pIFace->GetObjectBarResId(nNo).GetId() == nId)
            {
                nPos = pIFace->GetObjectBarPos(nNo);
                break;
            }
        }
    }
    else
    {
        // Alle Shells absuchen
        for ( sal_uInt16 nIdx=0; (pShell=GetShell(nIdx)); nIdx++ )
        {
            pIFace = pShell->GetInterface();
            for ( nNo=0; nNo<pIFace->GetObjectBarCount(); nNo++ )
            {
                if (pIFace->GetObjectBarResId(nNo).GetId() == nId)
                {
                    nPos = pIFace->GetObjectBarPos(nNo);
                    break;
                }
            }

            if ( nPos != USHRT_MAX )
                break;
        }
    }

    if ( nPos != USHRT_MAX )
    {
        // Auf jeden Fall eintragen, auch wenn unsichtbar. Dann kann
        // WorkWindow anbieten, wieder anzuschalten
        SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos & SFX_POSITION_MASK];
        SfxObjectBars_Impl& rFixed = pImp->aFixedObjBars[nPos & SFX_POSITION_MASK];

        sal_uInt16 nOldId = rBar.aResId.GetId();

        if ( pImp->bReadOnly && !( nPos & SFX_VISIBILITY_READONLYDOC ) )
            return;

        sal_uInt32 nFeature = pIFace->GetObjectBarFeature(nNo);
        if ( nFeature && !pShell->HasUIFeature( nFeature ) )
            return;

        sal_Bool bVisible = pIFace->IsObjectBarVisible(nNo);
        if ( !bVisible )
            // Alle Sichtbarkeitsflags ausschalten
            nPos &= SFX_POSITION_MASK;

        aResId.SetResMgr( pIFace->GetObjectBarResId(nNo).GetResMgr() );
        rBar.aResId = aResId;
        rBar.nMode = nPos;
        const String *pName = pIFace->GetObjectBarName(nNo);
        if ( pName )
            rBar.aName = *pName;
        else
            rBar.aName.Erase();
        rBar.pIFace = pIFace;

        rFixed = rBar;

        if ( nOldId != aResId.GetId() && pImp->bUpdated )
        {
            SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame()->GetWorkWindow_Impl();
            pWorkWin->SetObjectBar_Impl( nPos, aResId, pIFace, pName );
            pWorkWin->UpdateObjectBars_Impl();
        }

        if ( !bVisible )
        {
            rBar.aResId = ResId( 0,0 );
            rFixed.aResId = ResId( 0,0 );
        }

        return;
    }

//  DBG_ERROR("Objektleiste nicht bekannt!");
}

sal_uInt16 SfxDispatcher::GetObjectBarId( sal_uInt16 nPos ) const
{
    return pImp->aObjBars[nPos].aResId.GetId();
}

//--------------------------------------------------------------------
void SfxDispatcher::ResetObjectBars_Impl()

/*  [Beschreibung]

    Mit dieser Methode werden alle Objectbar-Anforderungen, die dieser
    Dispatcher an das AppWindow hat, beseitigt.
*/
{
    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].aResId = ResId( 0,0 );
    pImp->aChildWins.Remove(0, pImp->aChildWins.Count());
}


//--------------------------------------------------------------------
void SfxDispatcher::DebugOutput_Impl() const
{
#ifdef DBG_UTIL

    sal_uInt16 nOld = (sal_uInt16) DbgGetData()->nTraceOut;
    DbgGetData()->nTraceOut = DBG_OUT_FILE;

    if (pImp->bFlushed)
        DBG_TRACE("Flushed");
    if (pImp->bUpdated)
        DBG_TRACE("Updated");

    for ( sal_uInt16 nShell = pImp->aStack.Count(); nShell > 0; --nShell )
    {
        SfxShell *pShell = GetShell(nShell-1);
        const SfxInterface *pIFace = pShell->GetInterface();
        DBG_TRACE (pIFace->GetClassName());
    }

    DbgGetData()->nTraceOut = nOld;

#endif
}

void SfxDispatcher::LockUI_Impl( sal_Bool bLock )
{
    sal_Bool bWasLocked = pImp->bUILocked;
    pImp->bUILocked = bLock;
    if ( !bLock && bWasLocked )
        Update_Impl( sal_True );
}

//-------------------------------------------------------------------------
void SfxDispatcher::HideUI( sal_Bool bHide )
{
//  if ( !bHide && pImp->bReadOnly )
//      bHide = sal_True;
    sal_Bool bWasHidden = pImp->bNoUI;
    pImp->bNoUI = bHide;
    if ( bHide != bWasHidden )
        Update_Impl( sal_True );
}

void SfxDispatcher::SetReadOnly_Impl( sal_Bool bOn )
{
    pImp->bReadOnly = bOn;
//  pImp->bNoUI = bOn;
}

sal_Bool SfxDispatcher::GetReadOnly_Impl() const
{
    return pImp->bReadOnly;
}

//-------------------------------------------------------------------------
void SfxDispatcher::SetQuietMode_Impl( sal_Bool bOn )

/*  [Beschreibung]

    Bei 'bOn' stellt sich dieser Dispatcher quasi tot und leitet alles
    an den Parent-Dispatcher weiter.
*/

{
    pImp->bQuiet = bOn;
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->InvalidateAll(sal_True);
}

//-------------------------------------------------------------------------
void SfxDispatcher::SetModalMode_Impl( sal_Bool bOn )

/*  [Beschreibung]

    Bei 'bOn' werden nur noch Slots des Parent-Dispatchers gefunden.
*/

{
    pImp->bModal = bOn;
    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->InvalidateAll(sal_True);
}

void SfxDispatcher::SetExecuteMode( sal_uInt16 nMode )
{
    pImp->nStandardMode = nMode;
}

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSlot, const SfxPoolItem* &rpState )
{
    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot, &pShell, &pSlot, sal_False, sal_False ) )
    {
        rpState = pShell->GetSlotState(nSlot);
        if ( !rpState )
            return SFX_ITEM_DISABLED;
        else
            return SFX_ITEM_AVAILABLE;
    }

    return SFX_ITEM_DISABLED;
}

sal_Bool SfxDispatcher::IsReadOnlyShell_Impl( sal_uInt16 nShell ) const
{
    sal_uInt16 nShellCount = pImp->aStack.Count();
    if ( nShell < nShellCount )
    {
        SfxShell* pShell = pImp->aStack.Top( nShell );
        if( pShell->ISA( SfxModule ) || pShell->ISA( SfxApplication ) )
            return sal_False;
        else
            return pImp->bReadOnly;
    }
    else if ( pImp->pParent )
        return pImp->pParent->IsReadOnlyShell_Impl( nShell - nShellCount );
    return sal_True;
}

// Ein dirty trick, um an die Methoden der private base class von
// SfxShellStack_Impl heranzukommen
class StackAccess_Impl : public SfxShellStack_Implarr_
{};

void SfxDispatcher::InsertShell_Impl( SfxShell& rShell, sal_uInt16 nPos )
{
    Flush();

    // Der cast geht, weil SfxShellStack_Impl keine eigenen member hat
    ((StackAccess_Impl*) (&pImp->aStack))->Insert( nPos, &rShell );
    rShell.SetDisableFlags( pImp->nDisableFlags );
    rShell.DoActivate(pImp->pFrame, sal_True);

    if ( !SFX_APP()->IsDowning() )
    {
        pImp->bUpdated = sal_False;
        pImp->pCachedServ1 = 0;
        pImp->pCachedServ2 = 0;
        InvalidateBindings_Impl(sal_True);
    }
}

void SfxDispatcher::RemoveShell_Impl( SfxShell& rShell )
{
    Flush();

    // Der cast geht, weil SfxShellStack_Impl keine eigenen member hat
    StackAccess_Impl& rStack = *((StackAccess_Impl*) (&pImp->aStack));
    sal_uInt16 nCount = rStack.Count();
    for ( sal_uInt16 n=0; n<nCount; ++n )
    {
        if ( rStack[n] == &rShell )
        {
            rStack.Remove( n );
            rShell.SetDisableFlags( 0 );
            rShell.DoDeactivate(pImp->pFrame, sal_True);
            break;
        }
    }

    if ( !SFX_APP()->IsDowning() )
    {
        pImp->bUpdated = sal_False;
        pImp->pCachedServ1 = 0;
        pImp->pCachedServ2 = 0;
        InvalidateBindings_Impl(sal_True);
    }
}

sal_Bool SfxDispatcher::IsAllowed
(
    sal_uInt16 nSlot
) const
/*
    [Beschreibung]
    Die Methode prueft, ob der Zugriff auf diese Schnittstelle erlaubt ist.
    */
{
    if ( !pImp->pDisableList )
    {
        return sal_True;
    }

    // BinSearch in der DisableListe
    SvUShorts& rList = *pImp->pDisableList;
    sal_uInt16 nCount = rList.Count();
    sal_uInt16 nLow = 0, nMid, nHigh;
    sal_Bool bFound = sal_False;
    nHigh = nCount - 1;

    while ( !bFound && nLow <= nHigh )
    {
        nMid = (nLow + nHigh) >> 1;
        DBG_ASSERT( nMid < nCount, "bsearch ist buggy" );

        int nDiff = (int) nSlot - (int) rList[nMid];
        if ( nDiff < 0)
        {
            if ( nMid == 0 )
                break;
            nHigh = nMid - 1;
        }
        else if ( nDiff > 0 )
        {
            nLow = nMid + 1;
            if ( nLow == 0 )
                break;
        }
        else
            bFound = sal_True;
    }

    // Slot in der Liste gefunden ?
    sal_uInt16 nPos = bFound ? nMid : nLow;

    DBG_ASSERT( nPos <= nCount, "" );
    DBG_ASSERT( nPos == nCount || nSlot <= rList[nPos], "" );
    DBG_ASSERT( nPos == 0 || nSlot > rList[nPos-1], "" );
    DBG_ASSERT( ( (nPos+1) >= nCount ) || nSlot < rList[nPos+1], "" );

    return !bFound;
}

void SfxDispatcher::InvalidateBindings_Impl( sal_Bool bModify )
{
    // App-Dispatcher?
    if ( IsAppDispatcher() )
    {
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst();
                pFrame;
                pFrame = SfxViewFrame::GetNext( *pFrame ) )
            pFrame->GetBindings().InvalidateAll(bModify);
/*
        // alle Bindings sind betroffen
        for ( SfxInPlaceFrame *pIPFrame = (SfxInPlaceFrame*)
                    SfxViewFrame::GetFirst(0, TYPE(SfxInPlaceFrame));
                pIPFrame;
                pIPFrame = (SfxInPlaceFrame*)
                    SfxViewFrame::GetNext(*pIPFrame, 0, TYPE(SfxInPlaceFrame)) )
            pIPFrame->GetBindings().InvalidateAll(bModify);

        for ( SfxPlugInFrame *pPIFrame = (SfxPlugInFrame*)
                    SfxViewFrame::GetFirst(0, TYPE(SfxPlugInFrame));
                pPIFrame;
                pPIFrame = (SfxPlugInFrame*)
                    SfxViewFrame::GetNext(*pPIFrame, 0, TYPE(SfxPlugInFrame)) )
            pPIFrame->GetBindings().InvalidateAll(bModify);

        for ( SfxTask* pTask = SfxTask::GetFirst(); pTask;
                pTask = SfxTask::GetNext( *pTask ) )
            if ( !pTask->IsExternal() )
                pTask->GetBindings()->InvalidateAll(bModify);
*/
    }
    else
    {
        SfxDispatcher *pDisp = GetBindings()->GetDispatcher_Impl();
        while ( pDisp )
        {
            if ( pDisp == this )
            {
                GetBindings()->InvalidateAll( bModify );
                break;
            }

            pDisp = pDisp->pImp->pParent;
        }
    }
}

sal_uInt16 SfxDispatcher::GetNextToolBox_Impl( sal_uInt16 nPos, sal_uInt16 nType, String *pStr )
{
    SfxShell *pShell;
    sal_Bool bReadOnly = sal_False;
    SfxViewFrame *pFrame = pImp->pFrame;
    if ( pFrame )
    {
        SfxObjectShell *pObj = pFrame->GetObjectShell();
        bReadOnly =  !pObj || pObj->IsReadOnly();
    }

    sal_Bool bFound = sal_False;
    SfxWorkWindow *pWorkWin = GetBindings()->GetWorkWindow_Impl();

    // Wenn die aktuelle Toolbox die letzte ist, mu\s die erste zur"uckgegeben
    // werden
    sal_uInt16 nNext = 0;

    if ( nPos != 0xFFFF )
    {
        SfxDispatcher *pDispat = this;
        while ( pDispat )
        {
            SfxBindings* pBindings = pDispat->GetBindings();
            if ( !pBindings || pBindings->GetWorkWindow_Impl() != pWorkWin )
                break;

            sal_uInt16 nCount = pDispat->pImp->aStack.Count();
            // Alle interfaces auf dem Dispatcher-Stack durchsuchen
            for ( sal_uInt16 nIdx=0; nIdx<nCount; nIdx++ )
            {
                pShell = pDispat->pImp->aStack.Top(nIdx);
                const SfxInterface *pInterFace = pShell->GetInterface();
                for ( sal_uInt16 nNo = 0; pInterFace && nNo<pInterFace->GetObjectBarCount(); ++nNo )
                {
                    // Ist die Objectbar an der richtigen Position und sichtbar ?
                    sal_uInt16 nP = pInterFace->GetObjectBarPos(nNo);

                    if ( (nP & SFX_POSITION_MASK) == (nPos & SFX_POSITION_MASK) &&
                            pInterFace->IsObjectBarVisible(nNo) &&
                        ( !bReadOnly || ( nP & SFX_VISIBILITY_READONLYDOC ) ) )
                    {
                        sal_uInt16 nId = pInterFace->GetObjectBarResId(nNo).GetId();
                        if ( nId != nType )
                        {
                            // Es ist eine andere Toolbox
                            // Ist sie aktivierbar ?
                            sal_uInt32 nFeature = pInterFace->GetObjectBarFeature( nNo );
                            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                                continue;

                            // Kommt sie nach der aktuellen ?
                            if ( bFound )
                            {
                                // Ja, also ist sie die n"achste
                                if ( pStr )
                                    *pStr = *pInterFace->GetObjectBarName( nNo );
                                return nId;
                            }
                            else if ( !nNext )
                            {
                                // Nein, aber vielleicht ist es die erste ?
                                nNext = nId;
                                if ( pStr )
                                    *pStr = *pInterFace->GetObjectBarName( nNo );
                            }
                        }
                        else
                            // das ist die aktuelle!
                            bFound = sal_True;
                    }
                }
            }

            pDispat = pDispat->pImp->pParent;
        }
    }

    return nNext;
}

sal_Bool SfxDispatcher::IsUpdated_Impl() const
{
    return pImp->bUpdated;
}

void SfxDispatcher::SetDisableFlags( sal_uInt32 nFlags )
{
    pImp->nDisableFlags = nFlags;
    for ( int i = int(pImp->aStack.Count()) - 1; i >= 0; --i )
        pImp->aStack.Top( (sal_uInt16) i )->SetDisableFlags( nFlags );
}

sal_uInt32 SfxDispatcher::GetDisableFlags() const
{
    return pImp->nDisableFlags;
}

sal_Bool SfxDispatcher::HasSlot_Impl( sal_uInt16 nSlot )
{
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.Count();

    if ( pImp->pParent && !pImp->pParent->pImp->pFrame )
    {
        // the last frame also uses the AppDispatcher
        nTotCount += pImp->aStack.Count();
    }

    if ( SfxMacroConfig::IsMacroSlot( nSlot ) )
        // Makro-Slot?
        return sal_True;
    else if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        // Verb-Slot?
        SfxShell *pSh = 0;
        for ( sal_uInt16 nShell = 0; pSh = GetShell(nShell); ++nShell )
        {
            if ( pSh->ISA(SfxViewShell) )
                return sal_True;
        }

        return sal_False;
    }

    // SID gegen gesetzten Filter pr"ufen
    sal_uInt16 nSlotEnableMode=0;
    if ( pImp->pFrame )
    {
        nSlotEnableMode = IsSlotEnabledByFilter_Impl( nSlot );
        if ( 0 == nSlotEnableMode )
            return sal_False;
    }

    // im Quiet-Mode nur Parent-Dispatcher
    if ( pImp->bQuiet )
        return sal_False;

    sal_Bool bReadOnly = ( 2 != nSlotEnableMode && pImp->bReadOnly ) ||
                ( pImp->pFrame && pImp->pFrame->GetObjectShell() &&
                  pImp->pFrame->GetObjectShell()->IsLoading() );

    for ( sal_uInt16 i=0 ; i < nTotCount; ++i )
    {
        SfxShell *pObjShell = GetShell(i);
        SfxInterface *pIFace = pObjShell->GetInterface();
        const SfxSlot *pSlot = pIFace->GetSlot(nSlot);
        if ( pSlot && pSlot->nDisableFlags && ( pSlot->nDisableFlags & pObjShell->GetDisableFlags() ) != 0 )
            return sal_False;

        if ( pSlot && !( pSlot->nFlags & SFX_SLOT_READONLYDOC ) && bReadOnly )
            return sal_False;

        if ( pSlot )
        {
            // Slot geh"ort zum Container?
            FASTBOOL bIsContainerSlot = pSlot->IsMode(SFX_SLOT_CONTAINER);
            FASTBOOL bIsInPlace = pImp->pFrame && pImp->pFrame->ISA( SfxInPlaceFrame );

            // Shell geh"ort zum Server?
            // AppDispatcher oder IPFrame-Dispatcher
            FASTBOOL bIsServerShell = !pImp->pFrame || bIsInPlace;

            // Nat"urlich sind ServerShell-Slots auch ausf"uhrbar, wenn sie auf
            // einem Container-Dispatcher ohne IPClient ausgef"uhrt werden sollen.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = pImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetIPClient();
            }

            // Shell geh"ort zum Container?
            // AppDispatcher oder kein IPFrameDispatcher
            FASTBOOL bIsContainerShell = !pImp->pFrame || !bIsInPlace;

            // Shell und Slot passen zusammen
            if ( !( ( bIsContainerSlot && bIsContainerShell ) ||
                    ( !bIsContainerSlot && bIsServerShell ) ) )
                pSlot = 0;
        }

        if ( pSlot && !IsAllowed( nSlot ) )
            pSlot = NULL;

        if ( pSlot )
            return sal_True;
    }

    return sal_False;
}


