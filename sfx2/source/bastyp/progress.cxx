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
#include <sfx2/sfxresid.hxx>
#include "bastyp.hrc"
#include <sfx2/msg.hxx>
#include "sfxslots.hxx"
#include "sfxbasecontroller_internal.hxx"
#include <time.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::task;

struct SfxProgress_Impl
{
    Reference < XStatusIndicator > xStatusInd;
    OUString                aText, aStateText;
    sal_uIntPtr             nMax;
    clock_t                 nCreate;
    clock_t                 nNextReschedule;
    bool                    bLocked;
    bool                    bWaitMode;
    bool                    bAllowRescheduling;
    bool                    bRunning;

    SfxProgress*            pActiveProgress;
    SfxObjectShellRef       xObjSh;
    SfxWorkWindow*          pWorkWin;
    SfxViewFrame*           pView;

    explicit                SfxProgress_Impl( const OUString& );
    void                    Enable_Impl();

};


void SfxProgress_Impl::Enable_Impl()
{
    SfxObjectShell* pDoc = xObjSh.get();
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pDoc);
    while ( pFrame )
    {
        pFrame->Enable(true/*bEnable*/);
        pFrame->GetDispatcher()->Lock( false );
        pFrame = SfxViewFrame::GetNext(*pFrame, pDoc);
    }

    if ( pView )
    {
        pView->Enable( true/*bEnable*/ );
        pView->GetDispatcher()->Lock( false );
    }

    if ( !pDoc )
        SfxGetpApp()->GetAppDispatcher_Impl()->Lock( false );
}


SfxProgress_Impl::SfxProgress_Impl( const OUString &/*rTitle*/ )
    : nMax(0)
    , nCreate(0)
    , nNextReschedule(0)
    , bLocked(false)
    , bWaitMode(false)
    , bAllowRescheduling(false)
    , bRunning(false)
    , pActiveProgress(nullptr)
    , pWorkWin(nullptr)
    , pView(nullptr)
{
}


SfxProgress::SfxProgress
(
    SfxObjectShell*     pObjSh, /*  The action is performed on the
                                    SfxObjectShell which can be NULL.
                                    When it is then the application will be
                                    used */

    const OUString&     rText,  /* Text, which appears before the Statusmonitor
                                  in the status line */

    sal_uInt32          nRange, /* Max value for range  */

    bool                bWait    /* Activate the wait-Pointer initially (TRUE) */
)

/*  [Description]

    The constructor of the class SfxProgress switches the SfxObjectShell
    passed as parameter and SfxViewFrames which display this document in
    a progress mode. Ie as long as one of those SfxViewFrame instances is
    active the associated SfxDispatcher and associated Window is disabled.
    A progress-bar will be displayed in the status bar,
*/

:   pImpl( new SfxProgress_Impl( rText ) ),
    nVal(0),
    bSuspended(true)
{
    pImpl->bRunning = true;
    pImpl->bAllowRescheduling = Application::IsInExecute();

    pImpl->xObjSh = pObjSh;
    pImpl->aText = rText;
    pImpl->nMax = nRange;
    pImpl->bLocked = false;
    pImpl->bWaitMode = bWait;
    pImpl->nCreate = Get10ThSec();
    pImpl->nNextReschedule = pImpl->nCreate;
    SAL_INFO(
        "sfx.bastyp",
        "SfxProgress: created for '" << rText << "' at " << pImpl->nCreate
            << "ds");
    pImpl->pWorkWin = nullptr;
    pImpl->pView = nullptr;

    pImpl->pActiveProgress = GetActiveProgress( pObjSh );
    if ( pObjSh )
        pObjSh->SetProgress_Impl(this);
    else if( !pImpl->pActiveProgress )
        SfxGetpApp()->SetProgress_Impl(this);
    Resume();
}


SfxProgress::~SfxProgress()

/*  [Description]

    The destructor of the class SfxProgress restores the old status,
    the documents are released again and the status bar shows the items again.
*/

{
    Stop();
    if ( pImpl->xStatusInd.is() )
        pImpl->xStatusInd->end();
}


void SfxProgress::Stop()

/*  [Description]

    Early Exit of <SfxProgress>.
*/

{
    if( pImpl->pActiveProgress )
    {
        if ( pImpl->xObjSh.is() && pImpl->xObjSh->GetProgress() == this )
            pImpl->xObjSh->SetProgress_Impl(nullptr);
        return;
    }

    if ( !pImpl->bRunning )
        return;
    pImpl->bRunning = false;
    SAL_INFO(
        "sfx.bastyp", "SfxProgress: destroyed at " << Get10ThSec() << "ds");

    Suspend();
    if ( pImpl->xObjSh.is() )
        pImpl->xObjSh->SetProgress_Impl(nullptr);
    else
        SfxGetpApp()->SetProgress_Impl(nullptr);
    if ( pImpl->bLocked )
        pImpl->Enable_Impl();
}

void SfxProgress::SetStateText
(
    sal_uInt32       nNewVal,     /* New value for the progress-bar */
    const OUString& rNewVal     /* Status as Text */
)

{
    pImpl->aStateText = rNewVal;
    SetState( nNewVal );
}

void SfxProgress::SetState
(
    sal_uInt32   nNewVal,    /* new value for the progress bar */

    sal_uInt32   nNewRange   /* new maximum value, 0 for retaining the old */
)
/*  [Description]

    Setting the current status, after a time delay Reschedule is called.
*/

{
    if( pImpl->pActiveProgress ) return;

    nVal = nNewVal;

    // new Range?
    if ( nNewRange && nNewRange != pImpl->nMax )
    {
        SAL_INFO(
            "sfx.bastyp",
            "SfxProgress: range changed from " << pImpl->nMax << " to "
                << nNewRange);
        pImpl->nMax = nNewRange;
    }

    if ( !pImpl->xStatusInd.is() )
    {
        // get the active ViewFrame of the document this progress is working on
        // if it doesn't work on a document, take the current ViewFrame
        SfxObjectShell* pObjSh = pImpl->xObjSh.get();
        pImpl->pView = SfxViewFrame::Current();
        DBG_ASSERT( pImpl->pView || pObjSh, "Can't make progress bar!");
        if ( pObjSh && ( !pImpl->pView || pObjSh != pImpl->pView->GetObjectShell() ) )
        {
            // current document does not belong to current ViewFrame; take it's first visible ViewFrame
            SfxViewFrame* pDocView = SfxViewFrame::GetFirst( pObjSh );
            if ( pDocView )
                pImpl->pView = pDocView;
            else
            {
                // don't show status indicator for hidden documents (only valid while loading)
                SfxMedium* pMedium = pObjSh->GetMedium();
                const SfxBoolItem* pHiddenItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_HIDDEN, false);
                if ( !pHiddenItem || !pHiddenItem->GetValue() )
                {
                    {
                        const SfxUnoAnyItem* pIndicatorItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pMedium->GetItemSet(), SID_PROGRESS_STATUSBAR_CONTROL, false);
                        Reference< XStatusIndicator > xInd;
                        if ( pIndicatorItem && (pIndicatorItem->GetValue()>>=xInd) )
                            pImpl->xStatusInd = xInd;
                    }
                }
            }
        }
        else if ( pImpl->pView )
        {
            pImpl->pWorkWin = SfxGetpApp()->GetWorkWindow_Impl( pImpl->pView );
            if ( pImpl->pWorkWin )
                pImpl->xStatusInd = pImpl->pWorkWin->GetStatusIndicator();
        }

        if ( pImpl->xStatusInd.is() )
        {
            pImpl->xStatusInd->start( pImpl->aText, pImpl->nMax );
            pImpl->pView = nullptr;
        }
    }

    if ( pImpl->xStatusInd.is() )
    {
        pImpl->xStatusInd->setValue( nNewVal );
    }
}


void SfxProgress::Resume()

/*  [Description]

    Resumed the status of the display after an interrupt.

    [Cross-reference]

    <SfxProgress::Suspend()>
*/

{
    if( pImpl->pActiveProgress ) return;
    if ( bSuspended )
    {
        SAL_INFO("sfx.bastyp", "SfxProgress: resumed");
        if ( pImpl->xStatusInd.is() )
        {
            pImpl->xStatusInd->start( pImpl->aText, pImpl->nMax );
            pImpl->xStatusInd->setValue( nVal );
        }

        if ( pImpl->bWaitMode )
        {
            if ( pImpl->xObjSh.is() )
            {
                for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImpl->xObjSh.get() );
                        pFrame;
                        pFrame = SfxViewFrame::GetNext( *pFrame, pImpl->xObjSh.get() ) )
                    pFrame->GetWindow().EnterWait();
            }
        }

        if ( pImpl->xObjSh.is() )
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst(pImpl->xObjSh.get());
            if ( pFrame )
                pFrame->GetBindings().ENTERREGISTRATIONS();
        }

        bSuspended = false;
    }
}


void SfxProgress::Suspend()

/*  [Description]

    Interrupts the status of the display

    [Cross-reference]

    <SfxProgress::Resume()>
*/

{
    if( pImpl->pActiveProgress ) return;
    if ( !bSuspended )
    {
        SAL_INFO("sfx.bastyp", "SfxProgress: suspended");
        bSuspended = true;

        if ( pImpl->xStatusInd.is() )
        {
            pImpl->xStatusInd->reset();
        }

        if ( pImpl->xObjSh.is() )
        {
            for ( SfxViewFrame *pFrame =
                    SfxViewFrame::GetFirst(pImpl->xObjSh.get());
                    pFrame;
                    pFrame = SfxViewFrame::GetNext( *pFrame, pImpl->xObjSh.get() ) )
                pFrame->GetWindow().LeaveWait();
        }
        if ( pImpl->xObjSh.is() )
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pImpl->xObjSh.get() );
            if ( pFrame )
                pFrame->GetBindings().LEAVEREGISTRATIONS();
        }
    }
}


void SfxProgress::UnLock()
{
    if( pImpl->pActiveProgress ) return;
    if ( !pImpl->bLocked )
        return;

    SAL_INFO("sfx.bastyp", "SfxProgress: unlocked");
    pImpl->bLocked = false;
    pImpl->Enable_Impl();
}


void SfxProgress::Reschedule()

/*  [Description]

    Reschedule, callable from the outside
*/

{
    SFX_STACK(SfxProgress::Reschedule);

    if( pImpl->pActiveProgress ) return;
    SfxApplication* pApp = SfxGetpApp();
    if ( pImpl->bLocked && 0 == pApp->Get_Impl()->nRescheduleLocks )
    {
        SfxAppData_Impl *pAppData = pApp->Get_Impl();
        ++pAppData->nInReschedule;
        Application::Reschedule();
        --pAppData->nInReschedule;
    }
}


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
        return nullptr;

    SfxProgress *pProgress = nullptr;
    if ( pDocSh )
        pProgress = pDocSh->GetProgress();
    if ( !pProgress )
        pProgress = SfxGetpApp()->GetProgress();
    return pProgress;
}


void SfxProgress::EnterLock()
{
    SfxGetpApp()->Get_Impl()->nRescheduleLocks++;
}


void SfxProgress::LeaveLock()
{
    SfxAppData_Impl *pImp = SfxGetpApp()->Get_Impl();
    DBG_ASSERT( 0 != pImp->nRescheduleLocks, "SFxProgress::LeaveLock but no locks" );
    pImp->nRescheduleLocks--;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
