/*************************************************************************
 *
 *  $RCSfile: progress.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:28 $
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
#include "appdata.hxx"

#ifndef _SBX_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#pragma hdrstop

#include "progress.hxx"
#include "viewfrm.hxx"
#include "ipfrm.hxx"
#include "viewsh.hxx"
#include "objsh.hxx"
#include "app.hxx"
#include "stbmgr.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "docfile.hxx"
#include <workwin.hxx>

#include <time.h>

struct SfxProgress_Impl :
    public SfxCancellable
{
    String                  aText, aStateText;
    ULONG                   nMax;
    clock_t                 nCreate;
    clock_t                 nNextReschedule;
    BOOL                    bLocked, bAllDocs;
    BOOL                    bWaitMode;
    BOOL                    bAllowRescheduling;
    BOOL                    bRunning;
    BOOL                    bIsStatusText;

    SfxProgress*            pActiveProgress;
    SfxObjectShellRef       xObjSh;
    SfxStatusBarManager*    pMgr;
    SfxApplication*         pApp;
    SfxWorkWindow*          pWorkWin;

                            SfxProgress_Impl( const String& );

};

//========================================================================

#define TIMEOUT_PROGRESS         5L /* 10th s */
#define MAXPERCENT_PROGRESS     33

#define TIMEOUT_RESCHEDULE      10L /* 10th s */
#define MAXPERCENT_RESCHEDULE   50

#define Progress
#include "sfxslots.hxx"

#define aTypeLibInfo aProgressTypeLibImpl

//========================================================================

inline ULONG Get10ThSec()
{
#if defined (MSC) && defined (WIN)
    ULONG n10Ticks = 10 * (ULONG)GetTickCount();
#else
    ULONG n10Ticks = 10 * (ULONG)clock();
#endif

    return n10Ticks / CLOCKS_PER_SEC;
}

// -----------------------------------------------------------------------

void Enable_Impl(SfxObjectShell *pDoc, BOOL bEnable)
{
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst(pDoc);
    while ( pFrame )
    {
        pFrame->Enable(bEnable);
        if ( pDoc )
            pFrame->GetDispatcher()->Lock( !bEnable );
        pFrame = SfxViewFrame::GetNext(*pFrame, pDoc);
    }

    if ( pDoc )
    {
        SfxFrame* pFrm = pDoc->GetMedium()->GetLoadTargetFrame();
        pFrame = pFrm ? pFrm->GetCurrentViewFrame() : NULL;
        if ( pFrame )
        {
            pFrame->Enable( bEnable );
            pFrame->GetDispatcher()->Lock( !bEnable );
        }
    }
    else
        SFX_APP()->LockDispatcher( !bEnable );
}

// -----------------------------------------------------------------------

SfxProgress_Impl::SfxProgress_Impl( const String &rTitle )
    :   SfxCancellable( ( pApp = SFX_APP(), pApp->GetCancelManager() ), rTitle ),
        pActiveProgress( 0 )

{
}

// -----------------------------------------------------------------------

SfxProgress::SfxProgress
(
    SfxObjectShell* pObjSh, /* SfxObjectShell, an der die Aktion ausgef"uhrt
                               wird. Kann NULL sein, dann wird die Applikation
                               verwendet */

    const String&   rText,  /* Text, der in der Statuszeile vor den Statusmonitor
                               erscheint */

    ULONG           nRange, /* Maximalwert des Bereiches */

    BOOL            bAll    /* alle Dokumente oder nur das Dokument des ViewFrames
                               disablen (FALSE) */
    ,BOOL           bWait   /* initial den Wait-Pointer aktivieren (TRUE) */
)

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxProgress schaltet den als Parameter
    "ubergebenen SfxObjectShell und SfxViewFrames, welche dieses Dokument
    anzeigen in einen Progress-Mode. D.h. solange eine dieser SfxViewFrame
    Instanzen aktiv ist, ist der dazugeh"orige SfxDispatcher und das
    dazugeh"orige Window disabled. In der Statuszeile wird ein Balken zur
    Fortschritts-Anzeige angezeigt.
*/

:   nVal(0),
    bSuspended(TRUE),
    pImp( new SfxProgress_Impl( rText ) )
{
    pImp->bRunning = TRUE;
    pImp->bAllowRescheduling = Application::IsInExecute();;

    pImp->xObjSh = pObjSh;
    pImp->aText = rText;
    pImp->nMax = nRange;
    pImp->bLocked = FALSE;
    pImp->bWaitMode = bWait;
    pImp->bIsStatusText = FALSE;
    pImp->nCreate = Get10ThSec();
    pImp->nNextReschedule = pImp->nCreate;
    DBG( DbgOutf( "SfxProgress: created for '%s' at %luds",
                  rText.GetBuffer(), pImp->nCreate ) );
    pImp->bAllDocs = bAll;
    //pImp->pApp = SFX_APP();
    pImp->pMgr = 0;
    pImp->pWorkWin = 0;

    pImp->pActiveProgress = GetActiveProgress( pObjSh );
    if ( pObjSh )
        pObjSh->SetProgress_Impl(this);
    else if( !pImp->pActiveProgress )
        pImp->pApp->SetProgress_Impl(this);
    Resume();
}

// -----------------------------------------------------------------------

SfxProgress::~SfxProgress()

/*  [Beschreibung]

    Der Destruktor der Klasse SfxProgress restauriert den alten Zustand;
    die Dokumente werden wieder freigeschaltet und die Statuszeile zeigt
    wieder Items an.
*/

{
    Stop();
    if( pImp->bIsStatusText == TRUE )
        GetpApp()->HideStatusText( );
    delete pImp;
}

// -----------------------------------------------------------------------

void SfxProgress::Stop()

/*  [Beschreibung]

    Vorzeitiges Beenden des <SfxProgress>.
*/

{
    if( pImp->pActiveProgress ) return;
    if ( !pImp->bRunning )
        return;
    pImp->bRunning = FALSE;
    DBG( DbgOutf( "SfxProgress: destroyed at %luds", Get10ThSec() ) );

    Suspend();
    if ( pImp->xObjSh.Is() )
        pImp->xObjSh->SetProgress_Impl(0);
    else
        pImp->pApp->SetProgress_Impl(0);
    if ( pImp->bLocked )
    {
        if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
        {
            Enable_Impl(pImp->xObjSh, TRUE);
        }
        else
        {
            Enable_Impl(0, TRUE);
        }
    }
}

// -----------------------------------------------------------------------

void SfxProgress::SetText
(
    const String&   rText   /*  neuer Text */
)

/*  [Beschreibung]

    "Andert den Text, der links neben dem Fortschritts-Balken
    angezeigt wird.
*/

{
    if( pImp->pActiveProgress ) return;
    if ( pImp->pMgr && pImp->pMgr->IsProgressMode() )
    {
        pImp->pMgr->EndProgressMode();
        pImp->aText = rText;
        pImp->pMgr->StartProgressMode( pImp->aText, pImp->nMax );
    }
}

// -----------------------------------------------------------------------

const String& SfxProgress::GetStateText_Impl() const
{
    return pImp->aStateText;
}

// -----------------------------------------------------------------------
/*
IMPL_STATIC_LINK( SfxProgress, SetStateHdl, PlugInLoadStatus*, pStatus )
{
    INetRequest* pReq = 0;
    const INetHint *pHint = PTR_CAST( INetHint, pStatus->pHint );
    pReq = PTR_CAST( INetRequest, pStatus->pBC );

    String aString;
    if( pReq )
        aString = SfxMedium::GetStatusString( pHint->GetId(), pReq, pHint );
    if( aString.Len() )
    {
        GetpApp()->ShowStatusText( aString );
        if( pThis )
            pThis->pImp->bIsStatusText = TRUE;
    }
    return 0;
}
*/

// -----------------------------------------------------------------------

// muss in AppDaten
static ULONG nLastTime = 0;

long TimeOut_Impl( void* pThis, void* pArgV )
{
    Timer *pArg = (Timer*)pArgV;
    if( Time::GetSystemTicks() - nLastTime > 3000 )
    {
        GetpApp()->HideStatusText();
        nLastTime = 0;
        delete pArg;
    }
    else pArg->Start();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( SfxProgress, DefaultBindingProgress, SvProgressArg*, pArg )
{
    if( !nLastTime )
    {
        Timer *pTimer = new Timer();
        pTimer->SetTimeout( 3000 );
        pTimer->SetTimeoutHdl( Link( 0, TimeOut_Impl ) );
        pTimer->Start();
    }
    if( Time::GetSystemTicks( ) - nLastTime > 100 )
    {
        nLastTime = Time::GetSystemTicks();
        String aString = SfxMedium::GetStatusString( pArg );
        if( aString.Len() )
            GetpApp()->ShowStatusText( aString );
    }
    return 0;
}

// -----------------------------------------------------------------------

BOOL SfxProgress::SetStateText
(
    ULONG           nNewVal,    /* neuer Wert f"ur die Fortschritts-Anzeige */
    const String&   rNewVal,    /* Status als Text */
    ULONG           nNewRange   /* neuer Maximalwert, 0 f"ur Beibehaltung des alten */
)

{
    pImp->aStateText = rNewVal;
    return SetState( nNewVal, nNewRange );
}

// -----------------------------------------------------------------------

BOOL SfxProgress::SetState
(
    ULONG   nNewVal,    /* neuer Wert f"ur die Fortschritts-Anzeige */

    ULONG   nNewRange   /* neuer Maximalwert, 0 f"ur Beibehaltung des alten */
)
/*  [Beschreibung]

    Setzen des aktuellen Status; nach einem zeitlichen Versatz
    wird Reschedule aufgerufen.


    [R"uckgabewert]

    BOOL                TRUE
                        Fortfahren mit der Aktion

                        FALSE
                        Abbrechen der Aktion
*/

{
    // wurde via Stop-Button angehalten?
    if ( pImp->IsCancelled() )
        return FALSE;

    if( pImp->pActiveProgress ) return TRUE;

    // neuen Wert "ubernehmen
    BOOL bOver=FALSE;
    nVal = nNewVal;

    // neuer Range?
    if ( nNewRange && nNewRange != pImp->nMax )
    {
        DBG( DbgOutf( "SfxProgress: range changed from %lu to %lu",
                      pImp->nMax, nNewRange ) );
        pImp->nMax = nNewRange;
        bOver = TRUE;
    }

    // noch kein StbManager eingestellt?
    if ( !pImp->pMgr )
    {
        ULONG nTime = Get10ThSec();
        ULONG nTimeDiff = nTime - pImp->nCreate;
        ULONG nPercent = pImp->nMax ? nNewVal * 100 / pImp->nMax : 0;
        DBG( DbgOutf( "SfxProgress: SetState invisible at %luds (%luds/%luds), %ld%%/%d%%",
                      nTime, nTimeDiff, TIMEOUT_PROGRESS,
                      nPercent, MAXPERCENT_PROGRESS ) );
        if ( nTimeDiff > TIMEOUT_PROGRESS && nPercent <= MAXPERCENT_PROGRESS )
        {
            pImp->pWorkWin = pImp->pApp->GetWorkWindow_Impl( SfxViewFrame::Current() );
            if( pImp->pWorkWin )
            {
                pImp->pWorkWin->SetTempStatusBar_Impl( TRUE );
                pImp->pMgr = pImp->pWorkWin->GetStatusBarManager_Impl();
            }
            DBG( DbgOutf( "SfxProgress: visible" ) );
        }
    }

    // schon ein StbManager?
    if ( pImp->pMgr )
    {
//      if ( ( !pImp->xObjSh.Is() || &pImp->xObjSh == SfxObjectShell::Current() ) )
        {
            // Rescheduling noch nicht aktiv?
            if ( !pImp->bLocked && pImp->bAllowRescheduling )
            {
                ULONG nTime = Get10ThSec();
                ULONG nTimeDiff = nTime - pImp->nCreate;
                ULONG nPercent = pImp->nMax ? nNewVal * 100 / pImp->nMax : 0;
                DBG( DbgOutf( "SfxProgress: SetState unlocked at %luds (%luds/%luds), %ld%%/%d%%",
                              nTime, nTimeDiff, TIMEOUT_RESCHEDULE,
                              nPercent, MAXPERCENT_RESCHEDULE ) );
                // Zeitpunkt zum verz"ogerten Rescheduling erreicht?
                if ( nTimeDiff > TIMEOUT_RESCHEDULE && nPercent <= MAXPERCENT_RESCHEDULE )
                    Lock();
            }

            if ( !bSuspended )
            {
                // reiner Text-Progress?
                if ( !pImp->nMax )
                    GetpApp()->ShowStatusText( pImp->aStateText );
                else
                {
                    // Progress-Fortschritt anzeigen
                    if ( bOver )
                        pImp->pMgr->SetProgressMaxValue( pImp->nMax );
                    if ( !pImp->pMgr->IsProgressMode() )
                        pImp->pMgr->StartProgressMode( pImp->aText, pImp->nMax );
                    pImp->pMgr->SetProgressState(nNewVal);
                }
            }
        }

        Reschedule();
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void SfxProgress::Resume()

/*  [Beschreibung]

    Nimmt die Anzeige des Status nach einer Unterbrechung wieder auf.

    [Querverweise]
    <SfxProgress::Suspend()>
*/

{
    if( pImp->pActiveProgress ) return;
    if ( bSuspended )
    {
        DBG( DbgOutf( "SfxProgress: resumed" ) );
        if ( pImp->pMgr && pImp->nMax )
        {
            pImp->pMgr->StartProgressMode( pImp->aText, pImp->nMax );
            pImp->pMgr->SetProgressState( nVal );
        }
        if ( pImp->bWaitMode )
        {
            if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
            {
                for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImp->xObjSh);
                        pFrame;
                        pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                    pFrame->GetWindow().EnterWait();
                SfxFrame* pFrm = pImp->xObjSh->GetMedium()->GetLoadTargetFrame();
                if ( pFrm )
                    pFrm->GetWindow().EnterWait();
            }
//(mba)/task
/*
            else if ( Application::GetAppWindow() )
                Application::GetAppWindow()->EnterWait();
 */
        }

        SfxBindings *pBindings = 0;
        if ( pImp->xObjSh )
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImp->xObjSh);
            if ( pFrame )
                pFrame->GetBindings().ENTERREGISTRATIONS();
        }

        bSuspended = FALSE;
    }
}

// -----------------------------------------------------------------------

void SfxProgress::Suspend()

/*  [Beschreibung]

    Unterbricht die Anzeige des Status

    [Querverweise]
    <SfxProgress::Resume()>
*/

{
    if( pImp->pActiveProgress ) return;
    if ( !bSuspended )
    {
        DBG( DbgOutf( "SfxProgress: suspended" ) );
        bSuspended = TRUE;
        if ( pImp->pMgr )
        {
            if ( pImp->pMgr->IsProgressMode() )
                pImp->pMgr->EndProgressMode();
            pImp->pMgr->ShowItems();
            pImp->pWorkWin->SetTempStatusBar_Impl( FALSE );
        }

        if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
        {
            for ( SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst(pImp->xObjSh);
                    pFrame;
                    pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                pFrame->GetWindow().LeaveWait();
                SfxFrame* pFrm = pImp->xObjSh->GetMedium()->GetLoadTargetFrame();
                if ( pFrm )
                    pFrm->GetWindow().LeaveWait();
        }
//(mba)/task
/*
        else if ( Application::GetAppWindow() )
            Application::GetAppWindow()->LeaveWait();
*/
        SfxBindings *pBindings = 0;
        if ( pImp->xObjSh.Is() )
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImp->xObjSh);
            if ( pFrame )
                pFrame->GetBindings().LEAVEREGISTRATIONS();
        }
    }
}

// -----------------------------------------------------------------------

void SfxProgress::Lock()
{
    if( pImp->pActiveProgress ) return;
    // kein Reschedule bei Embedded-Objekten,
    // da wir gegen das OLE Protokoll wehrlos sind
    if ( !pImp->xObjSh.Is() )
    {
        for ( SfxObjectShell *pDocSh = SfxObjectShell::GetFirst();
              pDocSh;
              pDocSh = SfxObjectShell::GetNext(*pDocSh) )
        {
            SfxObjectCreateMode eMode = pDocSh->GetCreateMode();
            if ( ( eMode == SFX_CREATE_MODE_EMBEDDED ) ||
                 ( eMode == SFX_CREATE_MODE_PREVIEW ) )
            {
                DBG( DbgOutf( "SfxProgress: not locked because EMBEDDED/PREVIEW found" ) );
                pImp->bAllowRescheduling = FALSE;
            }
        }
    }
    else
    {
        SfxObjectCreateMode eMode = pImp->xObjSh->GetCreateMode();
        if ( ( eMode == SFX_CREATE_MODE_EMBEDDED ) ||
             ( eMode == SFX_CREATE_MODE_PREVIEW ) )
        {
            DBG( DbgOutf( "SfxProgress: not locked because ObjectShell is EMBEDDED/PREVIEW" ) );
            pImp->bAllowRescheduling = FALSE;
        }
    }

    if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
    {
        Enable_Impl(pImp->xObjSh, FALSE);
    }
    else
    {
        Enable_Impl(0, FALSE);
    }

    DBG( DbgOutf( "SfxProgress: locked" ) );
    pImp->bLocked = TRUE;
}

// -----------------------------------------------------------------------

void SfxProgress::UnLock()
{
    if( pImp->pActiveProgress ) return;
    if ( !pImp->bLocked )
        return;

    DBG( DbgOutf( "SfxProgress: unlocked" ) );
    pImp->bLocked = FALSE;
    if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
    {
        Enable_Impl(pImp->xObjSh, TRUE);
    }
    else
    {
        Enable_Impl(0, TRUE);
    }
}

// -----------------------------------------------------------------------

void SfxProgress::Reschedule()

/*  [Beschreibung]

    Reschedule von au"sen rufbar

*/

{
    SFX_STACK(SfxProgress::Reschedule);

    if( pImp->pActiveProgress ) return;
    if ( pImp->bLocked && 0 == pImp->pApp->Get_Impl()->nRescheduleLocks )
    {
        DBG_ASSERTWARNING( pImp->pApp->IsInAsynchronCall_Impl(),
                            "Reschedule in synchron-call-stack" );

        SfxAppData_Impl *pAppData = pImp->pApp->Get_Impl();
        ++pAppData->nInReschedule;
        Application::Reschedule();
        --pAppData->nInReschedule;
    }
}

// -----------------------------------------------------------------------

void SfxProgress::SetWaitMode
(
    BOOL    bWait       /*  TRUE
                            Wartecursor wird verwendet

                            FALSE
                            Es wird kein Wartecursor verwendet */
)

/*  [Beschreibung]

    Wartecursor-Modus umschalten.

*/

{
    if( pImp->pActiveProgress ) return;
    if ( !bSuspended && pImp->bWaitMode != bWait )
    {
        if ( bWait )
        {
            if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
            {
                for ( SfxViewFrame *pFrame =
                        SfxViewFrame::GetFirst(pImp->xObjSh);
                        pFrame;
                        pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                    pFrame->GetWindow().EnterWait();
                SfxFrame* pFrm = pImp->xObjSh->GetMedium()->GetLoadTargetFrame();
                if ( pFrm )
                    pFrm->GetWindow().EnterWait();
            }
//(mba)/task
/*
            else if ( Application::GetAppWindow() )
                Application::GetAppWindow()->EnterWait();
 */
        }
        else
        {
            if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
            {
                for ( SfxViewFrame *pFrame =
                        SfxViewFrame::GetFirst(pImp->xObjSh);
                        pFrame;
                        pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                    pFrame->GetWindow().LeaveWait();
                SfxFrame* pFrm = pImp->xObjSh->GetMedium()->GetLoadTargetFrame();
                if ( pFrm )
                    pFrm->GetWindow().LeaveWait();
            }
//(mba)/task
            /*

            else if ( Application::GetAppWindow() )
                Application::GetAppWindow()->LeaveWait();
             */
        }
    }

    pImp->bWaitMode = bWait;
}

// -----------------------------------------------------------------------

BOOL SfxProgress::GetWaitMode() const

/*  [Beschreibung]

    Wartecursor-Modus abfragen.

*/

{
    return pImp->bWaitMode;
}

// -----------------------------------------------------------------------

SfxProgress* SfxProgress::GetActiveProgress
(
    SfxObjectShell* pDocSh    /*  <SfxObjectShell>, die nach einem laufenden
                                    <SfxProgress> gefragt werden soll, oder
                                    0, wenn ein f"ur die gesamte Applikation
                                    laufender SfxProgress erfragt werden soll.
                                    Der Pointer braucht nur zum Zeitpunkt des
                                    Aufrufs g"ultig zu sein. */
)

/*  [Beschreibung]

    Mit dieser Methode kann erfragt werden, ob und welcher <SfxProgress>-
    f"ur eine bestimmte Instanz von SfxObjectShell oder gar die gesamte
    Applikation zur Zeit aktiv ist. Dies kann z.B. zum Abfangen von
    Time-Out-Events etc. verwendet werden.

    Anstelle eines Pointer auf den SfxProgress der SfxObjectShell wird
    ggf. der auf den SfxProgress der Applikation geliefert, mit der
    Abfrage 'SfxProgress::GetActiveProgress(pMyDocSh)' wird also
    insofern vorhanden der SfxProgress von 'pMyDocSh' geliefert,
    sonst der SfxProgress der Applikation bzw. ein 0-Pointer.


    [Anmerkung]

    "auft kein SfxProgress an der Applikation und ebenfalls keiner an
    der angegebenen SfxObjectShell, dann wird immer 0 zur"uckgeliefert,
    auch wenn an einer anderen SfxObjectShell ein SfxProgress l"uft.


    [Querverweise]

    <SfxApplication::GetProgress()const>
    <SfxObjectShell::GetProgress()const>
*/

{
    SfxProgress *pProgress = 0;
    if ( pDocSh )
        pProgress = pDocSh->GetProgress();
    if ( !pProgress )
        pProgress = SFX_APP()->GetProgress();
    return pProgress;
}

// -----------------------------------------------------------------------

void SfxProgress::EnterLock()
{
    SFX_APP()->Get_Impl()->nRescheduleLocks++;
}

// -----------------------------------------------------------------------

void SfxProgress::LeaveLock()
{
    SfxAppData_Impl *pImp = SFX_APP()->Get_Impl();
    DBG_ASSERT( 0 != pImp->nRescheduleLocks, "SFxProgress::LeaveLock but no locks" )
    pImp->nRescheduleLocks--;
}

// -----------------------------------------------------------------------

FASTBOOL SfxProgress::StatusBarManagerGone_Impl
(
    SfxStatusBarManager *pStb   // dieser <SfxStatusBarManager> wird zerst"ort
)

/*  [Beschreibung]

    Interne Methode zum Benachrichtigen des SfxProgress, da\s der angegebene
    SfxStatusBarManger zerst"ort wird. Damit der Progress ihn loslassen
    kann.
*/

{
    if ( pImp->pMgr != pStb )
        return FALSE;

    DBG( DbgOutf( "SfxProgress: StatusBarManager gone" ) );
    pImp->pMgr = 0;
    return TRUE;
}

