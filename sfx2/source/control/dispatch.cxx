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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <svl/itempool.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#ifndef _SFXEITEM_HXX //autogen
#include <svl/eitem.hxx>
#endif
#include <svl/undo.hxx>
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <svtools/ttprops.hxx>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>             // wg. bsearch

#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#include <svtools/helpopt.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#ifndef GCC
#endif

// wg. nAutoPageID
#include "appdata.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/minstack.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/hintpost.hxx>
#include "slotserv.hxx"
#include <sfx2/ipclient.hxx>
#include "sfxtypes.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/mnumgr.hxx>
#include "workwin.hxx"

namespace css = ::com::sun::star;

//==================================================================
DBG_NAME(SfxDispatcherFlush)
DBG_NAME(SfxDispatcherFillState)

//==================================================================
typedef SfxRequest* SfxRequestPtr;
SV_IMPL_PTRARR( SfxItemPtrArray, SfxPoolItemPtr );
SV_DECL_PTRARR_DEL( SfxRequestPtrArray, SfxRequestPtr, 4, 4 )
SV_IMPL_PTRARR( SfxRequestPtrArray, SfxRequestPtr );

DECL_PTRSTACK(SfxShellStack_Impl, SfxShell*, 8, 4 );
//==================================================================

struct SfxToDo_Impl
{
    SfxShell*           pCluster;
    bool                bPush;
    bool                bDelete;
    bool                bUntil;

    SfxToDo_Impl()
        : pCluster(0)
        , bPush(false)
        , bDelete(false)
        , bUntil(false)
                {}
    SfxToDo_Impl( bool bOpPush, bool bOpDelete, bool bOpUntil, SfxShell& rCluster )
        : pCluster(&rCluster)
        , bPush(bOpPush)
        , bDelete(bOpDelete)
        , bUntil(bOpUntil)
                {}
    ~SfxToDo_Impl(){}

    bool operator==( const SfxToDo_Impl& rWith ) const
    { return pCluster==rWith.pCluster && bPush==rWith.bPush; }
};

DECL_OBJSTACK(SfxToDoStack_Impl, SfxToDo_Impl, 8, 4);
IMPL_OBJSTACK(SfxToDoStack_Impl, SfxToDo_Impl);

struct SfxObjectBars_Impl
{
    sal_uInt32          nResId; // Resource - und ConfigId der Toolbox
    sal_uInt16          nMode;  // spezielle Sichtbarkeitsflags
    String              aName;
    SfxInterface*       pIFace;

    SfxObjectBars_Impl() :
        nResId( 0 )
    {}
};

//------------------------------------------------------------------

struct SfxDispatcher_Impl
{
    SfxRequestPtrArray      aReqArr;
    const SfxSlotServer* pCachedServ1;       // zuletzt gerufene Message
    const SfxSlotServer* pCachedServ2;       // vorletzt gerufene Message
    SfxShellStack_Impl      aStack;             // aktive Funktionalitaet
    Timer                   aTimer;             // fuers flushen
    SfxToDoStack_Impl       aToDoStack;         // nicht abgearb. Push/Pop
    SfxViewFrame*           pFrame;             // 0 oder zugeh"or. Frame
    SfxDispatcher*          pParent;            // z.B. AppDispatcher, ggf. 0
    SfxHintPosterRef        xPoster;            // asynchrones Execute
    sal_Bool                    bFlushing;          // sal_True waehrend Flush //?
    sal_Bool                    bUpdated;           // Update_Impl gelaufen
    sal_Bool                    bLocked;            // kein Execute
    sal_Bool                    bInvalidateOnUnlock;// da fragte jemand
    sal_Bool                    bActive;            // nicht verwechseln mit gesetzt!
    sal_Bool*                   pInCallAliveFlag;   // dem Stack den Dtor anzeigen
    SfxObjectBars_Impl      aObjBars[SFX_OBJECTBAR_MAX];
    SfxObjectBars_Impl      aFixedObjBars[SFX_OBJECTBAR_MAX];
    SvULongs                aChildWins;
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
sal_Bool SfxDispatcher::IsLocked( sal_uInt16 ) const

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
    return pImp->bLocked;
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
    if ( rSlot.IsMode(SFX_SLOT_FASTCALL) || rShell.CanExecuteSlot_Impl(rSlot) )
    {
        if ( GetFrame() )
        {
            // ggf. Recording anwerfen
            com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame(
                    GetFrame()->GetFrame().GetFrameInterface(),
                    com::sun::star::uno::UNO_QUERY);

            com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xSet(
                    xFrame,
                    com::sun::star::uno::UNO_QUERY);

            if ( xSet.is() )
            {
                com::sun::star::uno::Any aProp = xSet->getPropertyValue(::rtl::OUString::createFromAscii("DispatchRecorderSupplier"));
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorderSupplier > xSupplier;
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder;
                aProp >>= xSupplier;
                if(xSupplier.is())
                    xRecorder = xSupplier->getDispatchRecorder();

                if ( bRecord && xRecorder.is() && !rSlot.IsMode(SFX_SLOT_NORECORD) )
                    rReq.Record_Impl( rShell, rSlot, xRecorder, GetFrame() );
            }
        }

        // Alles holen, was gebraucht wird, da der Slot den Execute evtl. nicht
        // "uberlebt, falls es ein 'Pseudoslot' f"ur Macros oder Verben ist
        sal_Bool bAutoUpdate = rSlot.IsMode(SFX_SLOT_AUTOUPDATE);

        // API-Call-Klammerung und Document-Lock w"ahrend des Calls
        {
            // 'this' mu\s im Dtor bescheid sagen
            sal_Bool bThisDispatcherAlive = sal_True;
            sal_Bool *pOldInCallAliveFlag = pImp->pInCallAliveFlag;
            pImp->pInCallAliveFlag = &bThisDispatcherAlive;

            SfxViewFrame* pView = GetFrame();
            if ( !pView )
                pView = SfxViewFrame::Current();
            if ( pView )
            {
                rtl::OString aCmd(".uno:");
                aCmd += rSlot.GetUnoName();
                SfxHelp::OpenHelpAgent( &pView->GetFrame(), aCmd );
            }

            SfxExecFunc pFunc = rSlot.GetExecFnc();
            rShell.CallExec( pFunc, rReq );

            // falls 'this' noch lebt
            if ( bThisDispatcherAlive )
                pImp->pInCallAliveFlag = pOldInCallAliveFlag;
            else
            {
                if ( pOldInCallAliveFlag )
                {
                    // auch verschachtelte Stack-Frames sch"utzen
                    *pOldInCallAliveFlag = sal_False;
                }

                // do nothing after this object is dead
                return rReq.IsDone();
            }
        }

        if ( rReq.IsDone() )
        {
            SfxBindings *pBindings = GetBindings();

            // bei AutoUpdate sofort updaten; "Pseudoslots" d"urfen nicht
            // Autoupdate sein!
            if ( bAutoUpdate && pBindings )
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

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].nResId = 0;

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
    sTemp += ByteString::CreateFromInt64( (sal_uIntPtr)this );
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

    bool bDelete = (nMode & SFX_SHELL_POP_DELETE) == SFX_SHELL_POP_DELETE;
    bool bUntil = (nMode & SFX_SHELL_POP_UNTIL) == SFX_SHELL_POP_UNTIL;
    bool bPush = (nMode & SFX_SHELL_PUSH) == SFX_SHELL_PUSH;

    SfxApplication *pSfxApp = SFX_APP();

#ifdef DBG_UTIL
    ByteString aMsg( "-SfxDispatcher(" );
    aMsg += ByteString::CreateFromInt64( (sal_uIntPtr) this );
    aMsg += bPush ? ")::Push(" : ")::Pop(";
    if ( rShell.GetInterface() )
        aMsg += rShell.GetInterface()->GetClassName();
    else
        aMsg += ByteString::CreateFromInt64( (sal_uIntPtr) &rShell );
    aMsg += bDelete ? ") with delete" : ")";
    if ( bUntil ) aMsg += " (up to)";
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
            DBG_TRACE("Unflushed dispatcher!");
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
    (void)pvoid; // unused
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


    [Rueckgabewert]

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
void SfxDispatcher::DoActivate_Impl( sal_Bool bMDI, SfxViewFrame* /* pOld */ )

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
        sTemp += ByteString::CreateFromInt64( (sal_uIntPtr) this );
        DBG_TRACE(sTemp.GetBuffer());
        DBG_ASSERT( !pImp->bActive, "Activate-Fehler" );
        #endif
        pImp->bActive = sal_True;
        pImp->bUpdated = sal_False;
        SfxBindings* pBindings = GetBindings();
        if ( pBindings )
        {
            pBindings->SetDispatcher(this);
            pBindings->SetActiveFrame( pImp->pFrame->GetFrame().GetFrameInterface() );
        }
    }
    else
    {
        #ifdef DBG_UTIL
        ByteString sTemp("Non-MDI-Activate Dispatcher");
        sTemp += ByteString::CreateFromInt64( (sal_uIntPtr) this );
        DBG_TRACE( sTemp.GetBuffer() );
        #endif
    }

    if ( IsAppDispatcher() )
        return;

    for ( int i = int(pImp->aStack.Count()) - 1; i >= 0; --i )
        pImp->aStack.Top( (sal_uInt16) i )->DoActivate_Impl(pImp->pFrame, bMDI);

    if ( bMDI && pImp->pFrame )
    {
        //SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxBindings *pBind = GetBindings();
        while ( pBind )
        {
            pBind->HidePopupCtrls_Impl( sal_False );
            pBind = pBind->GetSubBindings_Impl();
        }

        pImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( sal_False, sal_False, 1 );
    }

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
void SfxDispatcher::DoDeactivate_Impl( sal_Bool bMDI, SfxViewFrame* pNew )

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
        DBG_TRACE(ByteString("Deactivate Dispatcher ").Append(ByteString::CreateFromInt64( (sal_uIntPtr) this )).GetBuffer());
        DBG_ASSERT( pImp->bActive, "Deactivate-Fehler" );
        pImp->bActive = sal_False;

        if ( pImp->pFrame && !(pImp->pFrame->GetObjectShell()->IsInPlaceActive() ) )
        {
            SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
            if ( pWorkWin )
            {
                for (sal_uInt16 n=0; n<pImp->aChildWins.Count();)
                {
                    SfxChildWindow *pWin = pWorkWin->GetChildWindow_Impl( (sal_uInt16) ( pImp->aChildWins[n] & 0xFFFF ) );
                    if (!pWin || (pWin && pWin->GetAlignment() == SFX_ALIGN_NOALIGNMENT))
                        pImp->aChildWins.Remove(n);
                    else
                        n++;
                }
            }
        }
    }
    else {
        DBG_TRACE( ByteString ("Non-MDI-DeActivate Dispatcher").Append(ByteString::CreateFromInt64( (sal_uIntPtr) this )).GetBuffer() );
    }

    if ( IsAppDispatcher() && !pSfxApp->IsDowning() )
        return;

    for ( sal_uInt16 i = 0; i < pImp->aStack.Count(); ++i )
        pImp->aStack.Top(i)->DoDeactivate_Impl(pImp->pFrame, bMDI);

    sal_Bool bHidePopups = bMDI && pImp->pFrame;
    if ( pNew && pImp->pFrame )
    {
        com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xOldFrame(
            pNew->GetFrame().GetFrameInterface()->getCreator(), com::sun::star::uno::UNO_QUERY );

        com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xMyFrame(
            GetFrame()->GetFrame().GetFrameInterface(), com::sun::star::uno::UNO_QUERY );

        if ( xOldFrame == xMyFrame )
            bHidePopups = sal_False;
    }

    if ( bHidePopups )
    {
        //SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
        SfxBindings *pBind = GetBindings();
        while ( pBind )
        {
            pBind->HidePopupCtrls_Impl( sal_True );
            pBind = pBind->GetSubBindings_Impl();
        }

        pImp->pFrame->GetFrame().GetWorkWindow_Impl()->HidePopups_Impl( sal_True, sal_False, 1 );
    }

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
    sal_Bool            bModal,     // trotz ModalMode
    sal_Bool        bRealSlot
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
        if ( 0 == (*ppSlot)->GetExecFnc() && bRealSlot )
            *ppSlot = (*ppShell)->GetInterface()->GetRealSlot(*ppSlot);
        // Check only real slots as enum slots don't have an execute function!
        if ( bRealSlot && ((0 == *ppSlot) || (0 == (*ppSlot)->GetExecFnc()) ))
            return sal_False;

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
                    if ( eCallMode & SFX_CALLMODE_RECORD )
                        rReq.AllowRecording( sal_True );
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
    const SfxSlot *pSlot = GetSlot( rCommand );
    if ( pSlot )
        return pSlot->GetSlotId();
    return 0;
}

const SfxSlot* SfxDispatcher::GetSlot( const String& rCommand )
{
    // Anzahl der Shells auf den verkettenten Dispatchern z"ahlen
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.Count();
    if ( pImp->pParent )
    {
        SfxDispatcher *pParent = pImp->pParent;
        while ( pParent )
        {
            nTotCount = nTotCount + pParent->pImp->aStack.Count();
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
            return pSlot;
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
SfxPoolItem* SfxExecuteItem::Clone( SfxItemPool* ) const
{
    return new SfxExecuteItem( *this );
}

//--------------------------------------------------------------------
SfxExecuteItem::SfxExecuteItem( const SfxExecuteItem& rArg )
    : SfxItemPtrArray(), SfxPoolItem( rArg ), nModifier( 0 )
{
    eCall = rArg.eCall;
    nSlot = rArg.nSlot;
    sal_uInt16 nCount = rArg.Count();
    for( sal_uInt16 nPos = 0; nPos < nCount; nPos++ )
        Insert( rArg[ nPos ]->Clone(), nPos );
}

//--------------------------------------------------------------------
SfxExecuteItem::SfxExecuteItem(
    sal_uInt16 nWhichId, sal_uInt16 nSlotP, SfxCallMode eModeP,
    const SfxPoolItem*  pArg1, ... ) :
    SfxPoolItem( nWhichId ), nSlot( nSlotP ), eCall( eModeP ), nModifier( 0 )
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
    sal_uInt16 nWhichId, sal_uInt16 nSlotP, SfxCallMode eModeP )
    : SfxPoolItem( nWhichId ), nSlot( nSlotP ), eCall( eModeP ), nModifier( 0 )
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

    delete [] (SfxPoolItem**)pPtr;

    return pRet;
}

//--------------------------------------------------------------------
const SfxPoolItem*  SfxDispatcher::Execute(
    sal_uInt16 nSlot,
    SfxCallMode nCall,
    SfxItemSet* pArgs,
    SfxItemSet* pInternalArgs,
    sal_uInt16 nModi)
{
    if ( IsLocked(nSlot) )
        return 0;

    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSlot,  &pShell, &pSlot, sal_False,
                               SFX_CALLMODE_MODAL==(nCall&SFX_CALLMODE_MODAL) ) )
    {
        SfxAllItemSet aSet( pShell->GetPool() );
        if ( pArgs )
        {
            SfxItemIter aIter(*pArgs);
            for ( const SfxPoolItem *pArg = aIter.FirstItem();
                pArg;
                pArg = aIter.NextItem() )
                MappedPut_Impl( aSet, *pArg );
        }
        SfxRequest aReq( nSlot, nCall, aSet );
        if (pInternalArgs)
            aReq.SetInternalArgs_Impl( *pInternalArgs );
        aReq.SetModifier( nModi );

        _Execute( *pShell, *pSlot, aReq, nCall );
        return aReq.GetReturnValue();
    }
    return 0;
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
    return Execute( nSlot, eCall, 0, rArgs );
}

//--------------------------------------------------------------------
const SfxPoolItem*  SfxDispatcher::Execute
(
    sal_uInt16 nSlot,
    SfxCallMode eCall,
    sal_uInt16 nModi,
    const SfxItemSet &rArgs
)
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
        aReq.SetModifier( nModi );
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
                Call_Impl( *pSh, *pSlot, *pReq, pReq->AllowsRecording() ); //! woher bRecord?
//                Call_Impl( *pShell, *pExec->pSlot, *pReq, sal_True ); //! woher bRecord?
                DBG( pSfxApp->LeaveAsynchronCall_Impl() );
            }

//            delete pExec;
        }
        else
        {
//            pImp->xPoster->Post(pExec);
            if ( pImp->bLocked )
                pImp->aReqArr.Insert( new SfxRequest(*pReq), pImp->aReqArr.Count() );
            else
                pImp->xPoster->Post(new SfxRequest(*pReq));
        }
    }
//    else
//        delete pExec;

    delete pReq;
    return 0;
}
//--------------------------------------------------------------------
void SfxDispatcher::SetMenu_Impl()
{
    if ( pImp->pFrame )
    {
        SfxViewFrame* pTop = pImp->pFrame->GetTopViewFrame();
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxFrame& rFrame = pTop->GetFrame();
            if ( rFrame.IsMenuBarOn_Impl() )
            {
                com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), com::sun::star::uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    com::sun::star::uno::Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                    {
                        rtl::OUString aMenuBarURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" ));
                        if ( !xLayoutManager->isElementVisible( aMenuBarURL ) )
                            xLayoutManager->createElement( aMenuBarURL );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------
void SfxDispatcher::Update_Impl( sal_Bool bForce )
{
    SFX_STACK(SfxDispatcher::Update_Impl);

    Flush();

    if ( !pImp->pFrame || pImp->bUILocked )
        return;

    SFX_APP();  // -Wall is this required???
    SfxDispatcher *pDisp = this;
    sal_Bool bUpdate = bForce;
    while ( pDisp && pDisp->pImp->pFrame )
    {
        SfxWorkWindow *pWork = pDisp->pImp->pFrame->GetFrame().GetWorkWindow_Impl();
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

    if ( !bUpdate || pImp->pFrame->GetFrame().IsClosing_Impl() )
        return;

    SfxViewFrame* pTop = pImp->pFrame ? pImp->pFrame->GetTopViewFrame() : NULL;
    sal_Bool bUIActive = pTop && pTop->GetBindings().GetDispatcher() == this;

    if ( !bUIActive && pTop && GetBindings() == &pTop->GetBindings() )
        // keep own tools internally for collecting
        GetBindings()->GetDispatcher()->pImp->bUpdated = sal_False;

    SfxBindings* pBindings = GetBindings();
    if ( pBindings )
        pBindings->DENTERREGISTRATIONS();

    com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame = pBindings->GetActiveFrame();
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, com::sun::star::uno::UNO_QUERY );
    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            com::sun::star::uno::Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )) );
            aValue >>= xLayoutManager;
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    if ( xLayoutManager.is() )
        xLayoutManager->lock();

    sal_Bool bIsIPActive = pImp->pFrame && pImp->pFrame->GetObjectShell()->IsInPlaceActive();
    SfxInPlaceClient *pClient = pImp->pFrame ? pImp->pFrame->GetViewShell()->GetUIActiveClient() : NULL;
    if ( bUIActive && /* !bIsIPActive && */ ( !pClient || !pClient->IsObjectUIActive() ) )
        SetMenu_Impl();

    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
    SfxWorkWindow *pTaskWin = pImp->pFrame->GetTopFrame().GetWorkWindow_Impl();
    pTaskWin->ResetStatusBar_Impl();

    SfxDispatcher *pDispat = this;
    while ( pDispat )
    {
        SfxWorkWindow *pWork = pDispat->pImp->pFrame->GetFrame().GetWorkWindow_Impl();
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

    _Update_Impl( bUIActive, !bIsIPActive, bIsIPActive, pTaskWin );
    if ( bUIActive || bIsActive )
        pWorkWin->UpdateObjectBars_Impl();

    if ( pBindings )
        pBindings->DLEAVEREGISTRATIONS();

    if ( xLayoutManager.is() )
        xLayoutManager->unlock();

    return;
}

void SfxDispatcher::_Update_Impl( sal_Bool bUIActive, sal_Bool bIsMDIApp, sal_Bool bIsIPOwner, SfxWorkWindow *pTaskWin )
{
    SFX_APP();
    SfxWorkWindow *pWorkWin = pImp->pFrame->GetFrame().GetWorkWindow_Impl();
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
        pImp->pParent->_Update_Impl( bUIActive, bIsMDIApp, bIsIPOwner, pTaskWin );

    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].nResId = 0;
    pImp->aChildWins.Remove(0, pImp->aChildWins.Count());

    // bQuiet : own shells aren't considered for UI and SlotServer
    // bNoUI: own Shells aren't considered fors UI
    if ( pImp->bQuiet || pImp->bNoUI || (pImp->pFrame && pImp->pFrame->GetObjectShell()->IsPreview()) )
        return;

    sal_uInt32 nStatBarId=0;
    SfxShell *pStatusBarShell = NULL;

    SfxSlotPool* pSlotPool = &SfxSlotPool::GetSlotPool( GetFrame() );
    sal_uInt16 nTotCount = pImp->aStack.Count();
    for ( sal_uInt16 nShell = nTotCount; nShell > 0; --nShell )
    {
        SfxShell *pShell = GetShell( nShell-1 );
        SfxInterface *pIFace = pShell->GetInterface();

        // don't consider shells if "Hidden" oder "Quiet"
        sal_Bool bReadOnlyShell = IsReadOnlyShell_Impl( nShell-1 );
        sal_uInt16 nNo;
        for ( nNo = 0; pIFace && nNo<pIFace->GetObjectBarCount(); ++nNo )
        {
            sal_uInt16 nPos = pIFace->GetObjectBarPos(nNo);
            if ( bReadOnlyShell && !( nPos & SFX_VISIBILITY_READONLYDOC ) )
                continue;

            // check wether toolbar needs activation of a special feature
            sal_uInt32 nFeature = pIFace->GetObjectBarFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // check for toolboxes that are exclusively for a viewer
            if ( pImp->pFrame)
            {
                sal_Bool bViewerTbx = SFX_VISIBILITY_VIEWER == ( nPos & SFX_VISIBILITY_VIEWER );
                SfxObjectShell* pSh = pImp->pFrame->GetObjectShell();
                SFX_ITEMSET_ARG( pSh->GetMedium()->GetItemSet(), pItem, SfxBoolItem, SID_VIEWONLY, sal_False );
                sal_Bool bIsViewer = pItem && pItem->GetValue();
                if ( bIsViewer != bViewerTbx )
                    continue;
            }

            // always register toolbars, allows to switch them on
            sal_Bool bVisible = pIFace->IsObjectBarVisible(nNo);
            if ( !bVisible )
                nPos &= SFX_POSITION_MASK;

            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos & SFX_POSITION_MASK];
            rBar.nMode = nPos;
            rBar.nResId = pIFace->GetObjectBarResId(nNo).GetId();
            const String *pName = pIFace->GetObjectBarName(nNo);
            if ( pName )
                rBar.aName = *pName;
            else
                rBar.aName.Erase();
            rBar.pIFace = pIFace;

            if ( bUIActive || bIsActive )
            {
                pWorkWin->SetObjectBar_Impl(
                    nPos, rBar.nResId, rBar.pIFace, &rBar.aName );
            }

            if ( !bVisible )
                rBar.nResId = 0;
        }

        for ( nNo=0; pIFace && nNo<pIFace->GetChildWindowCount(); nNo++ )
        {
            sal_uInt32 nId = pIFace->GetChildWindowId(nNo);
            const SfxSlot *pSlot = pSlotPool->GetSlot( (sal_uInt16) nId );
            DBG_ASSERT( pSlot, "Childwindow slot missing!");
            if ( bReadOnlyShell )
            {
                // only show ChildWindows if their slot is allowed for readonly documents
                if ( pSlot && !pSlot->IsMode( SFX_SLOT_READONLYDOC ) )
                    continue;
            }

            sal_uInt32 nFeature = pIFace->GetChildWindowFeature(nNo);
            if ( nFeature && !pShell->HasUIFeature( nFeature ) )
                continue;

            // slot decides wether a ChildWindow is shown when document is OLE server or OLE client
            sal_uInt16 nMode = SFX_VISIBILITY_STANDARD;
            if( pSlot )
            {
                if ( pSlot->IsMode(SFX_SLOT_CONTAINER) )
                {
                    if ( pWorkWin->IsVisible_Impl( SFX_VISIBILITY_CLIENT ) )
                        nMode |= SFX_VISIBILITY_CLIENT;
                }
                else
                {
                    if ( pWorkWin->IsVisible_Impl( SFX_VISIBILITY_SERVER ) )
                        nMode |= SFX_VISIBILITY_SERVER;
                }
            }

            if ( bUIActive || bIsActive )
                pWorkWin->SetChildWindowVisible_Impl( nId, sal_True, nMode );
            if ( bUIActive || bIsActive || !pWorkWin->IsFloating( (sal_uInt16) ( nId & 0xFFFF ) ) )
                pImp->aChildWins.Insert( nId, pImp->aChildWins.Count());
        }

        if ( bIsMDIApp || bIsIPOwner )
        {
            sal_uInt32 nId = pIFace->GetStatusBarResId().GetId();
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
        if ( rFixed.nResId )
        {
            SfxObjectBars_Impl& rBar = pImp->aObjBars[nPos];
            rBar = rFixed;
            pWorkWin->SetObjectBar_Impl( rFixed.nMode,
                rFixed.nResId, rFixed.pIFace, &rFixed.aName );
        }
    }

    if ( pTaskWin && ( bIsMDIApp || bIsIPOwner ) )
    {
        SfxDispatcher *pActDispatcher = pTaskWin->GetBindings().GetDispatcher_Impl();
        SfxDispatcher *pDispatcher = this;
        while ( pActDispatcher && !bIsTaskActive )
        {
            if ( pDispatcher == pActDispatcher )
                bIsTaskActive = sal_True;
            pActDispatcher = pActDispatcher->pImp->pParent;
        }

        if ( bIsTaskActive && nStatBarId && pImp->pFrame )
        {
            // internal frames also may control statusbar
            SfxBindings& rBindings = pImp->pFrame->GetBindings();
            pImp->pFrame->GetFrame().GetWorkWindow_Impl()->SetStatusBar_Impl( nStatBarId, pStatusBarShell, rBindings );
        }
    }
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

    DBG_TRACE("Flushing dispatcher!");

#ifdef DBG_UTIL
    ByteString aMsg( "SfxDispatcher(" );
    aMsg += ByteString::CreateFromInt64( (sal_uIntPtr) this );
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
            bool bFound = sal_False;
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
    DBG_TRACE("Successfully flushed dispatcher!");

    // in der 2. Runde die Shells aktivieren und ggf. l"oschen
    for ( nToDo = aToDoCopy.Count()-1; nToDo >= 0; --nToDo )
    {
        SfxToDo_Impl aToDo( aToDoCopy.Top(nToDo) );
        if ( aToDo.bPush )
        {
            if ( pImp->bActive )
                aToDo.pCluster->DoActivate_Impl(pImp->pFrame, sal_True);
        }
        else
            if ( pImp->bActive )
                aToDo.pCluster->DoDeactivate_Impl(pImp->pFrame, sal_True);
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
        pImp->aFixedObjBars[n].nResId = 0;

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
        if ( pParent->pImp->pFrame->GetFrame().HasComponent() )
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
            nLevels = nLevels + pParent->pImp->aStack.Count();

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

    // Dispatcher gelockt? (SID_HELP_PI trotzdem durchlassen)
    if ( IsLocked(nSlot) )
    {
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
            nTotCount = nTotCount + pParent->pImp->aStack.Count();
            pParent = pParent->pImp->pParent;
        }
    }

    // Verb-Slot?
    if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        for ( sal_uInt16 nShell = 0;; ++nShell )
        {
            SfxShell *pSh = GetShell(nShell);
            if ( pSh == NULL )
                return false;
            if ( pSh->ISA(SfxViewShell) )
            {
                const SfxSlot* pSlot = pSh->GetVerbSlot_Impl(nSlot);
                if ( pSlot )
                {
                    rServer.SetShellLevel(nShell);
                    rServer.SetSlot( pSlot );
                    return true;
                }
            }
        }
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

    sal_Bool bReadOnly = ( 2 != nSlotEnableMode && pImp->bReadOnly );
//              ( pImp->pFrame && pImp->pFrame->GetObjectShell() );
//                pImp->pFrame->GetObjectShell()->IsLoading() );

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
            bool bIsContainerSlot = pSlot->IsMode(SFX_SLOT_CONTAINER);
            bool bIsInPlace = pImp->pFrame && pImp->pFrame->GetObjectShell()->IsInPlaceActive();

            // Shell geh"ort zum Server?
            // AppDispatcher oder IPFrame-Dispatcher
            bool bIsServerShell = !pImp->pFrame || bIsInPlace;

            // Nat"urlich sind ServerShell-Slots auch ausf"uhrbar, wenn sie auf
            // einem Container-Dispatcher ohne IPClient ausgef"uhrt werden sollen.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = pImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetUIActiveClient();
            }

            // Shell geh"ort zum Container?
            // AppDispatcher oder kein IPFrameDispatcher
            bool bIsContainerShell = !pImp->pFrame || !bIsInPlace;
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

sal_Bool SfxDispatcher::HasSlot_Impl( sal_uInt16 nSlot )
{
    Flush();
    sal_uInt16 nTotCount = pImp->aStack.Count();

    if ( pImp->pParent && !pImp->pParent->pImp->pFrame )
    {
        // the last frame also uses the AppDispatcher
        nTotCount = nTotCount + pImp->aStack.Count();
    }

    if (nSlot >= SID_VERB_START && nSlot <= SID_VERB_END)
    {
        // Verb-Slot?
        for ( sal_uInt16 nShell = 0;; ++nShell )
        {
            SfxShell *pSh = GetShell(nShell);
            if ( pSh == NULL )
                return false;
            if ( pSh->ISA(SfxViewShell) )
                return true;
        }
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

    sal_Bool bReadOnly = ( 2 != nSlotEnableMode && pImp->bReadOnly );
//              ( pImp->pFrame && pImp->pFrame->GetObjectShell());
//                pImp->pFrame->GetObjectShell()->IsLoading() );

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
            bool bIsContainerSlot = pSlot->IsMode(SFX_SLOT_CONTAINER);
            bool bIsInPlace = pImp->pFrame && pImp->pFrame->GetObjectShell()->IsInPlaceActive();

            // Shell geh"ort zum Server?
            // AppDispatcher oder IPFrame-Dispatcher
            bool bIsServerShell = !pImp->pFrame || bIsInPlace;

            // Nat"urlich sind ServerShell-Slots auch ausf"uhrbar, wenn sie auf
            // einem Container-Dispatcher ohne IPClient ausgef"uhrt werden sollen.
            if ( !bIsServerShell )
            {
                SfxViewShell *pViewSh = pImp->pFrame->GetViewShell();
                bIsServerShell = !pViewSh || !pViewSh->GetUIActiveClient();
            }

            // Shell geh"ort zum Container?
            // AppDispatcher oder kein IPFrameDispatcher
            bool bIsContainerShell = !pImp->pFrame || !bIsInPlace;

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
    if ( pSlot && IsLocked( pSlot->GetSlotId() ) )
    {
        pImp->bInvalidateOnUnlock = sal_True;
        DBG_PROFSTOP(SfxDispatcherFillState);
        return sal_False;
    }

    if ( pSlot )
    {
        DBG_ASSERT(bFlushed, "Dispatcher not flushed after retrieving slot servers!");
        if ( !bFlushed )
            return sal_False;
        // Flush();

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
                                  const SfxPoolItem *, ... )
{
    ExecutePopup( nConfigId, pWin, pPos );
}

SfxPopupMenuManager* SfxDispatcher::Popup( sal_uInt16 nConfigId,Window *pWin, const Point *pPos )
{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    SfxShell *pSh;
    nShLevel=0;

    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.Count();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) || ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
                return SfxPopupMenuManager::Popup( rResId, rDisp.GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
        }
    }
    return 0;
}


//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( sal_uInt16 nConfigId, Window *pWin, const Point *pPos )
{
    SfxDispatcher &rDisp = *SFX_APP()->GetDispatcher_Impl();
    sal_uInt16 nShLevel = 0;
    SfxShell *pSh;
/*
    const SvVerbList *pVerbList = 0;
    sal_uInt16 nMaxShellLevel = rDisp.pImp->aStack.Count();
    for ( pSh = rDisp.GetShell(nShLevel);
          pSh && nShLevel < nMaxShellLevel ;
          ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        if ( pSh->GetVerbs() )
        {
            pVerbList = pSh->GetVerbs();
            break;
        }
    }
*/
    nShLevel=0;
    if ( rDisp.pImp->bQuiet )
    {
        nConfigId = 0;
        nShLevel = rDisp.pImp->aStack.Count();
    }

    Window *pWindow = pWin ? pWin : rDisp.pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
    for ( pSh = rDisp.GetShell(nShLevel); pSh; ++nShLevel, pSh = rDisp.GetShell(nShLevel) )
    {
        const ResId& rResId = pSh->GetInterface()->GetPopupMenuResId();
        if ( ( nConfigId == 0 && rResId.GetId() ) || ( nConfigId != 0 && rResId.GetId() == nConfigId ) )
        {
            //SfxPopupMenuManager aPop( rResId.GetId(), *rDisp.GetBindings() );
            //aPop.SetResMgr(rResId.GetResMgr());
            //aPop.AddClipboardFunctions();
            //aPop.Initialize();
            //if ( pVerbList && pVerbList->Count() )
            //    aPop.InsertVerbs(pVerbList);
            //aPop.RemoveDisabledEntries();
            //aPop.Execute( pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
            SfxPopupMenuManager::ExecutePopup( rResId, rDisp.GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
            return;
        }
    }
}

//----------------------------------------------------------------------
void SfxDispatcher::ExecutePopup( const ResId &rId, Window *pWin, const Point *pPos )
{
    Window *pWindow = pWin ? pWin : pImp->pFrame->GetFrame().GetWorkWindow_Impl()->GetWindow();
/*
    SfxPopupMenuManager aPop( rId, *GetBindings() );
    aPop.AddClipboardFunctions();
    aPop.Initialize();
    aPop.RemoveDisabledEntries();
    aPop.Execute( pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
*/
    SfxPopupMenuManager::ExecutePopup( rId, GetFrame(), pPos ? *pPos : pWindow->GetPointerPosPixel(), pWindow );
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
    if ( !bLock )
    {
        sal_uInt16 nCount = pImp->aReqArr.Count();
        for ( sal_uInt16 i=0; i<nCount; i++ )
            pImp->xPoster->Post( pImp->aReqArr[i] );
        pImp->aReqArr.Remove( 0, nCount );
    }
}

sal_uInt32 SfxDispatcher::GetObjectBarId( sal_uInt16 nPos ) const
{
    return pImp->aObjBars[nPos].nResId;
}

//--------------------------------------------------------------------
void SfxDispatcher::ResetObjectBars_Impl()

/*  [Beschreibung]

    Mit dieser Methode werden alle Objectbar-Anforderungen, die dieser
    Dispatcher an das AppWindow hat, beseitigt.
*/
{
    for (sal_uInt16 n=0; n<SFX_OBJECTBAR_MAX; n++)
        pImp->aObjBars[n].nResId = 0;
    pImp->aChildWins.Remove(0, pImp->aChildWins.Count());
}


//--------------------------------------------------------------------
void SfxDispatcher::DebugOutput_Impl() const
{
#ifdef DBG_UTIL

    sal_uInt16 nOld = (sal_uInt16) DbgGetData()->nTraceOut;
    DbgGetData()->nTraceOut = DBG_OUT_FILE;

    if (bFlushed)
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
    if ( pImp->pFrame )
    {
        SfxViewFrame* pTop = pImp->pFrame->GetTopViewFrame();
        if ( pTop && pTop->GetBindings().GetDispatcher() == this )
        {
            SfxFrame& rFrame = pTop->GetFrame();
            if ( rFrame.IsMenuBarOn_Impl() )
            {
                com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > xPropSet( rFrame.GetFrameInterface(), com::sun::star::uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                    com::sun::star::uno::Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                        xLayoutManager->setVisible( !bHide );
                }
            }
        }
    }

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

SfxItemState SfxDispatcher::QueryState( sal_uInt16 nSID, ::com::sun::star::uno::Any& rAny )
{
    SfxShell *pShell = 0;
    const SfxSlot *pSlot = 0;
    if ( GetShellAndSlot_Impl( nSID, &pShell, &pSlot, sal_False, sal_False ) )
    {
        const SfxPoolItem* pItem( 0 );

        pItem = pShell->GetSlotState( nSID );
        if ( !pItem )
            return SFX_ITEM_DISABLED;
        else
        {
            ::com::sun::star::uno::Any aState;
            if ( !pItem->ISA(SfxVoidItem) )
            {
                sal_uInt16 nSubId( 0 );
                SfxItemPool& rPool = pShell->GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( nSID );
                if ( rPool.GetMetric( nWhich ) == SFX_MAPUNIT_TWIP )
                    nSubId |= CONVERT_TWIPS;
                pItem->QueryValue( aState, (sal_uInt8)nSubId );
            }
            rAny = aState;

            return SFX_ITEM_AVAILABLE;
        }
    }

    return SFX_ITEM_DISABLED;
}

sal_Bool SfxDispatcher::IsReadOnlyShell_Impl( sal_uInt16 nShell ) const
{
    sal_uInt16 nShellCount = pImp->aStack.Count();
    if ( nShell < nShellCount )
    {
        SfxShell* pShell = pImp->aStack.Top( nShell );
        if( pShell->ISA( SfxModule ) || pShell->ISA( SfxApplication ) || pShell->ISA( SfxViewFrame ) )
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
    rShell.DoActivate_Impl(pImp->pFrame, sal_True);

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
            rShell.DoDeactivate_Impl(pImp->pFrame, sal_True);
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
    sal_uInt16 nLow = 0, nMid = 0, nHigh;
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

#ifdef _DEBUG
    // Slot in der Liste gefunden ?
    sal_uInt16 nPos = bFound ? nMid : nLow;

    DBG_ASSERT( nPos <= nCount, "" );
    DBG_ASSERT( nPos == nCount || nSlot <= rList[nPos], "" );
    DBG_ASSERT( nPos == 0 || nSlot > rList[nPos-1], "" );
    DBG_ASSERT( ( (nPos+1) >= nCount ) || nSlot < rList[nPos+1], "" );
#endif

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

SfxModule* SfxDispatcher::GetModule() const
{
    for ( sal_uInt16 nShell = 0;; ++nShell )
    {
        SfxShell *pSh = GetShell(nShell);
        if ( pSh == NULL )
            return 0;
        if ( pSh->ISA(SfxModule) )
            return (SfxModule*) pSh;
    }
}
