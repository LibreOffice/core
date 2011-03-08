/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <basic/sbx.hxx>

#include <svl/eitem.hxx>
#include <tools/time.hxx>

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
#include "sfxresid.hxx"
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
    SfxWorkWindow*          pWorkWin;
    SfxViewFrame*           pView;

                            SfxProgress_Impl( const String& );
    void                    Enable_Impl( BOOL );

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
extern sal_uInt32 Get10ThSec();

// -----------------------------------------------------------------------

void SfxProgress_Impl::Enable_Impl( BOOL bEnable )
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
    SfxObjectShell*     pObjSh, /*  The action is performed on the
                                    SfxObjectShell which can be NULL.
                                    When it is then the application will be
                                    used */

    const String&       rText,  /* Text, which appears before the Statusmonitor
                                  in the status line */

    ULONG               nRange, /* Max value for range  */

    BOOL                bAll    /* Disable all documents or only the document of the ViewFram */
    ,BOOL               bWait   /* Aktivate the wait-Pointer initially (TRUE) */
)

/*  [Description]

    The constructor of the class SfxProgress switches the SfxObjectShell
    passed as parameter and SfxViewFrames which display this document in
    a progress mode. Ie as long as one of those SfxViewFrame instances is
    active the associated SfxDispatcher and associated Window is disabled.
    A progress-bar will be displayed in the status bar,
*/

:       pImp( new SfxProgress_Impl( rText ) ),
    nVal(0),
    bSuspended(TRUE)
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

/*  [Description]

    The destructor of the class SfxProgress restores the old status,
    the documents are released again and the status bar shows the items again.
*/

{
    Stop();
    if ( pImp->xStatusInd.is() )
        pImp->xStatusInd->end();

    if( pImp->bIsStatusText == TRUE )
        GetpApp()->HideStatusText( );
    delete pImp;
}

// -----------------------------------------------------------------------

void SfxProgress::Stop()

/*  [Description]

    Early Exit of <SfxProgress>.
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
    pImp->bRunning = FALSE;
    DBG( DbgOutf( "SfxProgress: destroyed at %luds", Get10ThSec() ) );

    Suspend();
    if ( pImp->xObjSh.Is() )
        pImp->xObjSh->SetProgress_Impl(0);
    else
        SFX_APP()->SetProgress_Impl(0);
    if ( pImp->bLocked )
        pImp->Enable_Impl(TRUE);
}

// -----------------------------------------------------------------------

void SfxProgress::SetText
(
    const String&       /*      new Text */
)

/*  [Description]

    Changes the text that appears to the left next to progress bar.
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

// Required in App data
static ULONG nLastTime = 0;

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

BOOL SfxProgress::SetStateText
(
    ULONG          nNewVal,     /* New value for the progress-bar */
    const String&  rNewVal,     /* Status as Text */
    ULONG          nNewRange    /* new maximum value, 0 for retaining the old */
)

{
    pImp->aStateText = rNewVal;
    return SetState( nNewVal, nNewRange );
}

// -----------------------------------------------------------------------

BOOL SfxProgress::SetState
(
    ULONG   nNewVal,    /* new value for the progress bar */

    ULONG   nNewRange   /* new maximum value, 0 for retaining the old */
)
/*  [Description]

    Setting the current status, after a time delay Reschedule is called.

    [Return value]

    BOOL                TRUE
                        Proceed with the action

                        FALSE
                        Cancel action
*/

{
    if( pImp->pActiveProgress ) return TRUE;

    nVal = nNewVal;

    // new Range?
    if ( nNewRange && nNewRange != pImp->nMax )
    {
        DBG( DbgOutf( "SfxProgress: range changed from %lu to %lu",
                      pImp->nMax, nNewRange ) );
        pImp->nMax = nNewRange;
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
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, FALSE );
                if ( !pHiddenItem || !pHiddenItem->GetValue() )
                {
                    {
                        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, FALSE );
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

    return TRUE;
}

// -----------------------------------------------------------------------

void SfxProgress::Resume()

/*  [Description]

    Resumed the status of the display after an interrupt.

    [Cross-reference]

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

        bSuspended = FALSE;
    }
}

// -----------------------------------------------------------------------

void SfxProgress::Suspend()

/*  [Description]

    Interrupts the status of the display

    [Cross-reference]

    <SfxProgress::Resume()>
*/

{
    if( pImp->pActiveProgress ) return;
    if ( !bSuspended )
    {
        DBG( DbgOutf( "SfxProgress: suspended" ) );
        bSuspended = TRUE;

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
    // No Reschedule for Embedded-Objects,
    // because we are defenseless against the OLE protocol
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

    pImp->Enable_Impl( FALSE );

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
    pImp->Enable_Impl(TRUE);
}

// -----------------------------------------------------------------------

void SfxProgress::Reschedule()

/*  [Description]

    Reschedule, callable from the outside
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
    BOOL        bWait  /*   TRUE  Wait-cursor is used

                            FALSE Wait-cursor not used */
)

/*  [Description]

    Switch Wait-mode.
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

BOOL SfxProgress::GetWaitMode() const

/* [Description]

   Get Wait-cursor mode.
*/

{
    return pImp->bWaitMode;
}

// -----------------------------------------------------------------------

SfxProgress* SfxProgress::GetActiveProgress
(
    SfxObjectShell* pDocSh        /*  the <SfxObjectShell>, which should be
                                      queried after a current <SfxProgress>,
                                      or 0 if an current SfxProgress for the
                                      entire application should be obtained.
                                      The pointer only needs at the time of
                                      the call to be valid.
                                  */
)

/*  [Description]

    This method is used to check whether and which <SfxProgress> is currently
    active for a specific instance of SfxObjectShell or even an entire
    application. This can for example be used to check for Time-Out-Events, etc.

    Instead of a pointer to the SfxProgress the SfxObjectShell may be
    pointed at the SfxProgress of the application, with the query
    'SfxProgress:: GetActiveProgress (pMyDocSh)' thus the current
    SfxProgress of 'pMyDocSh' is delivered, otherwise the SfxProgress of
    the application or a 0-pointer.

    [Note]

    If no SfxProgress is running in the application and also not at the
    specified SfxObjectShell, then this method will always return 0,
    even if one SfxProgress runs on another SfxObjectShell.

    [Cross-reference]

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

bool SfxProgress::StatusBarManagerGone_Impl
(
    SfxStatusBarManager *       // This <SfxStatusBarManager> will be destroyed
)

/*  [Description]

    Internal method for notifying the SfxProgress that the specified
    SfxStatusBarManger will be destroyed so that the Progress can let go of it.
*/

{
    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
