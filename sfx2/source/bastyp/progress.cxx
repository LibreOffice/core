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

#include <sfx2/progress.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>

#ifndef _SBX_HXX //autogen
#include <basic/sbx.hxx>
#endif

#include <svl/eitem.hxx>
#include <tools/time.hxx>

// wg. nRescheduleLocks
#include "appdata.hxx"
#include <sfx2/request.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/docfile.hxx>
#include "workwin.hxx"
#include "sfx2/sfxresid.hxx"
#include "bastyp.hrc"
#include <sfx2/msg.hxx>

#include <time.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

void AddNumber_Impl( String& aNumber, sal_uInt32 nArg )
{
    if ( nArg >= 10240 )
    {
        aNumber += String::CreateFromInt32( (sal_uInt16)( ( nArg + 512 ) / 1024 ) );
        aNumber += ' ';
        aNumber += SfxResId( STR_KB );
    }
    else
    {
        aNumber += String::CreateFromInt32( nArg );
        aNumber += ' ';
        aNumber += SfxResId( STR_BYTES );
    }
}

struct SfxProgress_Impl
{
    Reference < XStatusIndicator > xStatusInd;
    String                  aText, aStateText;
    sal_uIntPtr                 nMax;
    clock_t                 nCreate;
    clock_t                 nNextReschedule;
    sal_Bool                    bLocked, bAllDocs;
    sal_Bool                    bWaitMode;
    sal_Bool                    bAllowRescheduling;
    sal_Bool                    bRunning;
    sal_Bool                    bIsStatusText;

    SfxProgress*            pActiveProgress;
    SfxObjectShellRef       xObjSh;
    SfxWorkWindow*          pWorkWin;
    SfxViewFrame*           pView;

                            SfxProgress_Impl( const String& );
    void                    Enable_Impl( sal_Bool );

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
extern sal_uIntPtr Get10ThSec();

// -----------------------------------------------------------------------

void SfxProgress_Impl::Enable_Impl( sal_Bool bEnable )
{
    SfxObjectShell* pDoc = bAllDocs ? NULL : (SfxObjectShell*) xObjSh;
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst(pDoc);
    while ( pFrame )
    {
        pFrame->Enable(bEnable);
        pFrame->GetDispatcher()->Lock( !bEnable );
        pFrame = SfxViewFrame::GetNext(*pFrame, pDoc);
    }

    if ( pView )
    {
        pView->Enable( bEnable );
        pView->GetDispatcher()->Lock( !bEnable );
    }

    if ( !pDoc )
        SFX_APP()->GetAppDispatcher_Impl()->Lock( !bEnable );
}

// -----------------------------------------------------------------------

SfxProgress_Impl::SfxProgress_Impl( const String &/*rTitle*/ )
    :   pActiveProgress( 0 )
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

    sal_uIntPtr         nRange, /* Maximalwert des Bereiches */

    sal_Bool            bAll    /* alle Dokumente oder nur das Dokument des ViewFrames
                               disablen (sal_False) */
    ,sal_Bool           bWait   /* initial den Wait-Pointer aktivieren (sal_True) */
)

/*  [Beschreibung]

    Der Konstruktor der Klasse SfxProgress schaltet den als Parameter
    "ubergebenen SfxObjectShell und SfxViewFrames, welche dieses Dokument
    anzeigen in einen Progress-Mode. D.h. solange eine dieser SfxViewFrame
    Instanzen aktiv ist, ist der dazugeh"orige SfxDispatcher und das
    dazugeh"orige Window disabled. In der Statuszeile wird ein Balken zur
    Fortschritts-Anzeige angezeigt.
*/

:   pImp( new SfxProgress_Impl( rText ) ),
    nVal(0),
    bSuspended(sal_True)
{
    pImp->bRunning = sal_True;
    pImp->bAllowRescheduling = Application::IsInExecute();;

    pImp->xObjSh = pObjSh;
    pImp->aText = rText;
    pImp->nMax = nRange;
    pImp->bLocked = sal_False;
    pImp->bWaitMode = bWait;
    pImp->bIsStatusText = sal_False;
    pImp->nCreate = Get10ThSec();
    pImp->nNextReschedule = pImp->nCreate;
    DBG( DbgOutf( "SfxProgress: created for '%s' at %luds",
                  rText.GetBuffer(), pImp->nCreate ) );
    pImp->bAllDocs = bAll;
    pImp->pWorkWin = 0;
    pImp->pView = 0;

    pImp->pActiveProgress = GetActiveProgress( pObjSh );
    if ( pObjSh )
        pObjSh->SetProgress_Impl(this);
    else if( !pImp->pActiveProgress )
        SFX_APP()->SetProgress_Impl(this);
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
    if ( pImp->xStatusInd.is() )
        pImp->xStatusInd->end();

    if( pImp->bIsStatusText == sal_True )
        GetpApp()->HideStatusText( );
    delete pImp;
}

// -----------------------------------------------------------------------

void SfxProgress::Stop()

/*  [Beschreibung]

    Vorzeitiges Beenden des <SfxProgress>.
*/

{
    if( pImp->pActiveProgress )
    {
        if ( pImp->xObjSh.Is() && pImp->xObjSh->GetProgress() == this )
            pImp->xObjSh->SetProgress_Impl(0);
        return;
    }

    if ( !pImp->bRunning )
        return;
    pImp->bRunning = sal_False;
    DBG( DbgOutf( "SfxProgress: destroyed at %luds", Get10ThSec() ) );

    Suspend();
    if ( pImp->xObjSh.Is() )
        pImp->xObjSh->SetProgress_Impl(0);
    else
        SFX_APP()->SetProgress_Impl(0);
    if ( pImp->bLocked )
        pImp->Enable_Impl(sal_True);
}

// -----------------------------------------------------------------------

void SfxProgress::SetText
(
    const String&   /*  neuer Text */
)

/*  [Beschreibung]

    "Andert den Text, der links neben dem Fortschritts-Balken
    angezeigt wird.
*/

{
    if( pImp->pActiveProgress ) return;
    if ( pImp->xStatusInd.is() )
    {
        pImp->xStatusInd->reset();
        pImp->xStatusInd->start( pImp->aText, pImp->nMax );
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
            pThis->pImp->bIsStatusText = sal_True;
    }
    return 0;
}
*/

// -----------------------------------------------------------------------

// muss in AppDaten
static sal_uIntPtr nLastTime = 0;

long TimeOut_Impl( void*, void* pArgV )
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

sal_Bool SfxProgress::SetStateText
(
    sal_uLong           nNewVal,    /* neuer Wert f"ur die Fortschritts-Anzeige */
    const String&   rNewVal,    /* Status als Text */
    sal_uLong           nNewRange   /* neuer Maximalwert, 0 f"ur Beibehaltung des alten */
)

{
    pImp->aStateText = rNewVal;
    return SetState( nNewVal, nNewRange );
}

// -----------------------------------------------------------------------

sal_Bool SfxProgress::SetState
(
    sal_uLong   nNewVal,    /* neuer Wert f"ur die Fortschritts-Anzeige */

    sal_uLong   nNewRange   /* neuer Maximalwert, 0 f"ur Beibehaltung des alten */
)
/*  [Beschreibung]

    Setzen des aktuellen Status; nach einem zeitlichen Versatz
    wird Reschedule aufgerufen.


    [R"uckgabewert]

    sal_Bool                TRUE
                        Fortfahren mit der Aktion

                        FALSE
                        Abbrechen der Aktion
*/

{
    // wurde via Stop-Button angehalten?
//  if ( pImp->IsCancelled() )
//      return sal_False;

    if( pImp->pActiveProgress ) return sal_True;

    // neuen Wert "ubernehmen
    sal_Bool bOver=sal_False;
    nVal = nNewVal;

    // neuer Range?
    if ( nNewRange && nNewRange != pImp->nMax )
    {
        DBG( DbgOutf( "SfxProgress: range changed from %lu to %lu",
                      pImp->nMax, nNewRange ) );
        pImp->nMax = nNewRange;
        bOver = sal_True;
    }

    if ( !pImp->xStatusInd.is() )
    {
        // get the active ViewFrame of the document this progress is working on
        // if it doesn't work on a document, take the current ViewFrame
        SfxObjectShell* pObjSh = pImp->xObjSh;
        pImp->pView = SfxViewFrame::Current();
        DBG_ASSERT( pImp->pView || pObjSh, "Can't make progress bar!");
        if ( pObjSh && ( !pImp->pView || pObjSh != pImp->pView->GetObjectShell() ) )
        {
            // current document does not belong to current ViewFrame; take it's first visible ViewFrame
            SfxViewFrame* pDocView = SfxViewFrame::GetFirst( pObjSh );
            if ( pDocView )
                pImp->pView = pDocView;
            else
            {
                // don't show status indicator for hidden documents (only valid while loading)
                SfxMedium* pMedium = pObjSh->GetMedium();
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False );
                if ( !pHiddenItem || !pHiddenItem->GetValue() )
                {
                    // not in a view, perhaps it's just loading
                    //SfxFrame* pFrame = pMedium->GetLoadTargetFrame();
                    //if ( pFrame && pFrame->GetCurrentViewFrame() )
                    //{
                        // recycling frame
                        //pImp->pView = pFrame->GetCurrentViewFrame();
                    //}
                    //else
                    {
                        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
                        Reference< XStatusIndicator > xInd;
                        if ( pIndicatorItem && (pIndicatorItem->GetValue()>>=xInd) )
                            pImp->xStatusInd = xInd;
                    }
                }
            }
        }
        else if ( pImp->pView )
        {
            pImp->pWorkWin = SFX_APP()->GetWorkWindow_Impl( pImp->pView );
            if ( pImp->pWorkWin )
                pImp->xStatusInd = pImp->pWorkWin->GetStatusIndicator();
        }

        if ( pImp->xStatusInd.is() )
        {
            pImp->xStatusInd->start( pImp->aText, pImp->nMax );
            pImp->pView = NULL;
        }
    }

    if ( pImp->xStatusInd.is() )
    {
        pImp->xStatusInd->setValue( nNewVal );
    }

    return sal_True;
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
        if ( pImp->xStatusInd.is() )
        {
            pImp->xStatusInd->start( pImp->aText, pImp->nMax );
            pImp->xStatusInd->setValue( nVal );
        }

        if ( pImp->bWaitMode )
        {
            if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
            {
                for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImp->xObjSh);
                        pFrame;
                        pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                    pFrame->GetWindow().EnterWait();
            }
        }

        if ( pImp->xObjSh )
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImp->xObjSh);
            if ( pFrame )
                pFrame->GetBindings().ENTERREGISTRATIONS();
        }

        bSuspended = sal_False;
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
        bSuspended = sal_True;

        if ( pImp->xStatusInd.is() )
        {
            pImp->xStatusInd->reset();
        }

        if ( pImp->xObjSh.Is() && !pImp->bAllDocs )
        {
            for ( SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst(pImp->xObjSh);
                    pFrame;
                    pFrame = SfxViewFrame::GetNext( *pFrame, pImp->xObjSh ) )
                pFrame->GetWindow().LeaveWait();
        }
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
                pImp->bAllowRescheduling = sal_False;
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
            pImp->bAllowRescheduling = sal_False;
        }
    }

    pImp->Enable_Impl( sal_False );

    DBG( DbgOutf( "SfxProgress: locked" ) );
    pImp->bLocked = sal_True;
}

// -----------------------------------------------------------------------

void SfxProgress::UnLock()
{
    if( pImp->pActiveProgress ) return;
    if ( !pImp->bLocked )
        return;

    DBG( DbgOutf( "SfxProgress: unlocked" ) );
    pImp->bLocked = sal_False;
    pImp->Enable_Impl(sal_True);
}

// -----------------------------------------------------------------------

void SfxProgress::Reschedule()

/*  [Beschreibung]

    Reschedule von au"sen rufbar

*/

{
    SFX_STACK(SfxProgress::Reschedule);

    if( pImp->pActiveProgress ) return;
    SfxApplication* pApp = SFX_APP();
    if ( pImp->bLocked && 0 == pApp->Get_Impl()->nRescheduleLocks )
    {
        DBG_ASSERTWARNING( pApp->IsInAsynchronCall_Impl(),
                            "Reschedule in synchron-call-stack" );

        SfxAppData_Impl *pAppData = pApp->Get_Impl();
        ++pAppData->nInReschedule;
        Application::Reschedule();
        --pAppData->nInReschedule;
    }
}

// -----------------------------------------------------------------------

void SfxProgress::SetWaitMode
(
    sal_Bool    bWait       /*  TRUE
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
            }
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
            }
        }
    }

    pImp->bWaitMode = bWait;
}

// -----------------------------------------------------------------------

sal_Bool SfxProgress::GetWaitMode() const

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
    if ( !SfxApplication::Get() )
        return 0;

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
    DBG_ASSERT( 0 != pImp->nRescheduleLocks, "SFxProgress::LeaveLock but no locks" );
    pImp->nRescheduleLocks--;
}

// -----------------------------------------------------------------------

FASTBOOL SfxProgress::StatusBarManagerGone_Impl
(
    SfxStatusBarManager *   // dieser <SfxStatusBarManager> wird zerst"ort
)

/*  [Beschreibung]

    Interne Methode zum Benachrichtigen des SfxProgress, da\s der angegebene
    SfxStatusBarManger zerst"ort wird. Damit der Progress ihn loslassen
    kann.
*/

{
    return sal_True;
}

