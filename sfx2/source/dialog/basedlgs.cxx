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

#include <stdlib.h>

#include <comphelper/lok.hxx>

#include <osl/file.hxx>
#include <vcl/help.hxx>
#include <svl/eitem.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/controldims.hxx>
#include <vcl/idle.hxx>

#include <sfx2/basedlgs.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxhelp.hxx>
#include <workwin.hxx>

using namespace ::com::sun::star::uno;

#define USERITEM_NAME "UserItem"

SingleTabDlgImpl::SingleTabDlgImpl()
    : m_pSfxPage(nullptr)
{
}

class SfxModelessDialog_Impl : public SfxListener
{
public:
    OString aWinState;
    SfxChildWindow* pMgr;
    bool            bConstructed;
    bool            bClosing;
    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    Idle            aMoveIdle;
};

void SfxModelessDialog_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Dying) {
        pMgr->Destroy();
    }
}

class SfxFloatingWindow_Impl : public SfxListener
{
public:
    OString aWinState;
    SfxChildWindow* pMgr;
    bool            bConstructed;
    Idle            aMoveIdle;

    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

void SfxFloatingWindow_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if( rHint.GetId() == SfxHintId::Dying) {
        pMgr->Destroy();
    }
}


void SfxModalDialog::SetDialogData_Impl()
{
    // save settings (position and user data)
    OUString sConfigId = OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8);
    SvtViewOptions aDlgOpt(EViewType::Dialog, sConfigId);
    aDlgOpt.SetWindowState(OStringToOUString(
        GetWindowState(WindowStateMask::Pos), RTL_TEXTENCODING_ASCII_US));
    if ( !aExtraData.isEmpty() )
        aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( aExtraData ) );
}


void SfxModalDialog::GetDialogData_Impl()

/*  [Description]

    Helper function, reads the dialogue position from the ini file and
    puts them on the transferred window.
*/

{
    OUString sConfigId = OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8);
    SvtViewOptions aDlgOpt(EViewType::Dialog, sConfigId);
    if ( aDlgOpt.Exists() )
    {
        // load settings
        SetWindowState( OUStringToOString( aDlgOpt.GetWindowState(), RTL_TEXTENCODING_ASCII_US ) );
        Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aExtraData = aTemp;
    }
}

SfxModalDialog::SfxModalDialog(vcl::Window *pParent, const OUString& rID, const OUString& rUIXMLDescription )
:   ModalDialog(pParent, rID, rUIXMLDescription),
    pInputSet(nullptr)
{
    SetInstallLOKNotifierHdl(LINK(this, SfxModalDialog, InstallLOKNotifierHdl));
    GetDialogData_Impl();
}

IMPL_STATIC_LINK_NOARG(SfxModalDialog, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return SfxViewShell::Current();
}

SfxModalDialog::~SfxModalDialog()
{
    disposeOnce();
}

void SfxModalDialog::dispose()
{
    SetDialogData_Impl();
    pOutputSet.reset();

    ModalDialog::dispose();
}

void SfxModalDialog::CreateOutputItemSet( const SfxItemSet& rSet )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
    {
        pOutputSet.reset(new SfxItemSet( rSet ));
        pOutputSet->ClearItem();
    }
}

void SfxModelessDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        if ( !pImpl->aWinState.isEmpty() )
        {
            SetWindowState( pImpl->aWinState );
        }
        else
        {
            Point aPos = GetPosPixel();
            if ( !aPos.X() )
            {
                aSize = GetSizePixel();

                Size aParentSize = GetParent()->GetOutputSizePixel();
                Size aDlgSize = GetSizePixel();
                aPos.AdjustX(( aParentSize.Width() - aDlgSize.Width() ) / 2 );
                aPos.AdjustY(( aParentSize.Height() - aDlgSize.Height() ) / 2 );

                Point aPoint;
                tools::Rectangle aRect = GetDesktopRectPixel();
                aPoint.setX( aRect.Right() - aDlgSize.Width() );
                aPoint.setY( aRect.Bottom() - aDlgSize.Height() );

                aPoint = OutputToScreenPixel( aPoint );

                if ( aPos.X() > aPoint.X() )
                    aPos.setX( aPoint.X() ) ;
                if ( aPos.Y() > aPoint.Y() )
                    aPos.setY( aPoint.Y() );

                if ( aPos.X() < 0 ) aPos.setX( 0 );
                if ( aPos.Y() < 0 ) aPos.setY( 0 );

                SetPosPixel( aPos );
            }
        }

        pImpl->bConstructed = true;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SfxModelessDialog::Initialize(SfxChildWinInfo const *pInfo)

/*  [Description]

    Initialization of the class SfxModelessDialog via a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor, this
    constructor should be called from the derived class or from the
    SfxChildWindows.
*/

{
    if (pInfo)
        pImpl->aWinState = pInfo->aWinState;
}

void SfxModelessDialog::Resize()

/*  [Description]

    This virtual method of the class FloatingWindow keeps track if a change
    in size has been made. When this method is overridden by a derived class,
    then the SfxFloatingWindow: Resize() must also be called.
*/

{
    ModelessDialog::Resize();
    if ( pImpl->bConstructed && pImpl->pMgr )
    {
        // start timer for saving window status information
        pImpl->aMoveIdle.Start();
    }
}

void SfxModelessDialog::Move()
{
    ModelessDialog::Move();
    if ( pImpl->bConstructed && pImpl->pMgr && IsReallyVisible() )
    {
        // start timer for saving window status information
        pImpl->aMoveIdle.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG(SfxModelessDialog, TimerHdl, Timer *, void)
{
    pImpl->aMoveIdle.Stop();
    if ( pImpl->bConstructed && pImpl->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        WindowStateMask nMask = WindowStateMask::Pos | WindowStateMask::State;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= WindowStateMask::Width | WindowStateMask::Height;
        pImpl->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SfxChildIdentifier::DOCKINGWINDOW, SfxDockingConfig::ALIGNDOCKINGWINDOW, pImpl->pMgr->GetType() );
    }
}

SfxModelessDialog::SfxModelessDialog(SfxBindings* pBindinx,
    SfxChildWindow *pCW, vcl::Window *pParent, const OUString& rID,
    const OUString& rUIXMLDescription)
    : ModelessDialog(pParent, rID, rUIXMLDescription)
{
    SetInstallLOKNotifierHdl(LINK(this, SfxModelessDialog, InstallLOKNotifierHdl));
    Init(pBindinx, pCW);
}

IMPL_STATIC_LINK_NOARG(SfxModelessDialog, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return SfxViewShell::Current();
}

void SfxModelessDialog::Init(SfxBindings *pBindinx, SfxChildWindow *pCW)
{
    pBindings = pBindinx;
    pImpl.reset(new SfxModelessDialog_Impl);
    pImpl->pMgr = pCW;
    pImpl->bConstructed = false;
    pImpl->bClosing = false;
    if ( pBindinx )
        pImpl->StartListening( *pBindinx );
    pImpl->aMoveIdle.SetPriority(TaskPriority::RESIZE);
    pImpl->aMoveIdle.SetInvokeHandler(LINK(this,SfxModelessDialog,TimerHdl));
}

/*  [Description]

    If a ModelessDialog is enabled its ViewFrame will be activated.
    This is necessary by PluginInFrames.
*/
bool SfxModelessDialog::EventNotify( NotifyEvent& rEvt )
{
    if ( pImpl )
    {
        if ( rEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            pBindings->SetActiveFrame( pImpl->pMgr->GetFrame() );
            pImpl->pMgr->Activate_Impl();
        }
        else if ( rEvt.GetType() == MouseNotifyEvent::LOSEFOCUS && !HasChildPathFocus() )
        {
            pBindings->SetActiveFrame( css::uno::Reference< css::frame::XFrame > () );
        }
        else if( rEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            // First, allow KeyInput for Dialog functions ( TAB etc. )
            if (!ModelessDialog::EventNotify(rEvt) && SfxViewShell::Current())
            {
                // then also for valid global accelerators.
                return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
            }
            return true;
        }
    }

    return ModelessDialog::EventNotify( rEvt );
}

SfxModelessDialog::~SfxModelessDialog()
{
    disposeOnce();
}

void SfxModelessDialog::dispose()
{
    if ( pImpl->pMgr->GetFrame().is() && pImpl->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( nullptr );
    pImpl.reset();

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if (comphelper::LibreOfficeKit::isActive() && pViewShell)
    {
        pViewShell->notifyWindow(GetLOKWindowId(), "close");
        ReleaseLOKNotifier();
    }

    ModelessDialog::dispose();
}


bool SfxModelessDialog::Close()

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots. If this is method is overridden by a derived class
    method, then the SfxModelessDialogWindow: Close() must be called afterwards
    if the Close() was not cancelled with "return sal_False".
*/

{
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    SfxBoolItem aValue( pImpl->pMgr->GetType(), false);
    pBindings->GetDispatcher_Impl()->ExecuteList(
        pImpl->pMgr->GetType(),
        SfxCallMode::RECORD|SfxCallMode::SYNCHRON, { &aValue } );
    return true;
}


void SfxModelessDialog::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Description]

    Fills a SfxChildWinInfo with specific data from SfxModelessDialog,
    so that it can be written in the INI file. It is assumed that rinfo
    receives all other possible relevant data in the ChildWindow class.
    ModelessDialogs have no specific information, so that the base
    implementation does nothing and therefore must not be called.
*/

{
    rInfo.aSize  = aSize;
    if ( IsRollUp() )
        rInfo.nFlags |= SfxChildWindowFlags::ZOOMIN;
}

void SfxModelessDialogController::Initialize(SfxChildWinInfo const *pInfo)

/*  [Description]

    Initialization of the class SfxModelessDialog via a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor, this
    constructor should be called from the derived class or from the
    SfxChildWindows.
*/

{
    if (!pInfo)
        return;
    m_xImpl->aWinState = pInfo->aWinState;
    if (m_xImpl->aWinState.isEmpty())
        return;
    m_xDialog->set_window_state(m_xImpl->aWinState);
}

SfxModelessDialogController::SfxModelessDialogController(SfxBindings* pBindinx,
    SfxChildWindow *pCW, weld::Window *pParent, const OUString& rUIXMLDescription,
    const OString& rID)
    : SfxDialogController(pParent, rUIXMLDescription, rID)
{
    Init(pBindinx, pCW);
    m_xDialog->connect_focus_in(LINK(this, SfxModelessDialogController, FocusInHdl));
    m_xDialog->connect_focus_out(LINK(this, SfxModelessDialogController, FocusOutHdl));
}

void SfxModelessDialogController::Init(SfxBindings *pBindinx, SfxChildWindow *pCW)
{
    m_pBindings = pBindinx;
    m_xImpl.reset(new SfxModelessDialog_Impl);
    m_xImpl->pMgr = pCW;
    m_xImpl->bConstructed = true;
    m_xImpl->bClosing = false;
    if (pBindinx)
        m_xImpl->StartListening( *pBindinx );
}

void SfxModelessDialogController::DeInit()
{
    if (m_xImpl->pMgr)
    {
        WindowStateMask nMask = WindowStateMask::Pos | WindowStateMask::State;
        if (m_xDialog->get_resizable())
            nMask |= WindowStateMask::Width | WindowStateMask::Height;
        m_xImpl->aWinState = m_xDialog->get_window_state(nMask);
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SfxChildIdentifier::DOCKINGWINDOW, SfxDockingConfig::ALIGNDOCKINGWINDOW, m_xImpl->pMgr->GetType() );
    }

    m_xImpl->pMgr = nullptr;
}

/*  [Description]

    If a ModelessDialog is enabled its ViewFrame will be activated.
    This is necessary by PluginInFrames.
*/
IMPL_LINK_NOARG(SfxModelessDialogController, FocusInHdl, weld::Widget&, void)
{
    if (!m_xImpl)
        return;
    m_pBindings->SetActiveFrame(m_xImpl->pMgr->GetFrame());
    m_xImpl->pMgr->Activate_Impl();
    Activate();
}

IMPL_LINK_NOARG(SfxModelessDialogController, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xImpl)
        return;
    m_pBindings->SetActiveFrame(css::uno::Reference< css::frame::XFrame>());
}

SfxModelessDialogController::~SfxModelessDialogController()
{
    if (!m_xImpl->pMgr)
        return;
    auto xFrame = m_xImpl->pMgr->GetFrame();
    if (!xFrame)
        return;
    if (xFrame == m_pBindings->GetActiveFrame())
        m_pBindings->SetActiveFrame(nullptr);
}

void SfxModelessDialogController::EndDialog()
{
    if (!m_xDialog->get_visible())
        return;
    m_xImpl->bClosing = true;
    response(RET_CLOSE);
    m_xImpl->bClosing = false;
}

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots.
*/
void SfxModelessDialogController::Close()
{
    if (m_xImpl->bClosing)
        return;
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    SfxBoolItem aValue(m_xImpl->pMgr->GetType(), false);
    m_pBindings->GetDispatcher_Impl()->ExecuteList(
        m_xImpl->pMgr->GetType(),
        SfxCallMode::RECORD|SfxCallMode::SYNCHRON, { &aValue } );
}

/*  [Description]

    Fills a SfxChildWinInfo with specific data from SfxModelessDialog,
    so that it can be written in the INI file. It is assumed that rinfo
    receives all other possible relevant data in the ChildWindow class.
    ModelessDialogs have no specific information, so that the base
    implementation does nothing and therefore must not be called.
*/
void SfxModelessDialogController::FillInfo(SfxChildWinInfo& rInfo) const
{
    rInfo.aSize = m_xDialog->get_size();
}

bool SfxFloatingWindow::EventNotify( NotifyEvent& rEvt )

/*  [Description]

    If a ModelessDialog is enabled, its ViewFrame will be activated.
    This necessary for the PluginInFrames.
*/

{
    if ( pImpl )
    {
        if ( rEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            pBindings->SetActiveFrame( pImpl->pMgr->GetFrame() );
            pImpl->pMgr->Activate_Impl();
        }
        else if ( rEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
        {
            if ( !HasChildPathFocus() )
            {
                pBindings->SetActiveFrame( nullptr );
            }
        }
        else if( rEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            // First, allow KeyInput for Dialog functions
            if (!FloatingWindow::EventNotify(rEvt) && SfxViewShell::Current())
            {
                // then also for valid global accelerators.
                return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
            }
            return true;
        }
    }

    return FloatingWindow::EventNotify( rEvt );
}

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        vcl::Window* pParent, WinBits nWinBits) :
    FloatingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pImpl( new SfxFloatingWindow_Impl )
{
    pImpl->pMgr = pCW;
    pImpl->bConstructed = false;
    if ( pBindinx )
        pImpl->StartListening( *pBindinx );
    pImpl->aMoveIdle.SetPriority(TaskPriority::RESIZE);
    pImpl->aMoveIdle.SetInvokeHandler(LINK(this,SfxFloatingWindow,TimerHdl));
}

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        vcl::Window* pParent,
                        const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame) :
    FloatingWindow(pParent, rID, rUIXMLDescription, rFrame),
    pBindings(pBindinx),
    pImpl( new SfxFloatingWindow_Impl )
{
    pImpl->pMgr = pCW;
    pImpl->bConstructed = false;

    if ( pBindinx )
        pImpl->StartListening( *pBindinx );
    pImpl->aMoveIdle.SetPriority(TaskPriority::RESIZE);
    pImpl->aMoveIdle.SetInvokeHandler(LINK(this,SfxFloatingWindow,TimerHdl));
}

bool SfxFloatingWindow::Close()

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots. If this is method is overridden by a derived class
    method, then the SfxModelessDialogWindow: Close) must be called afterwards
    if the Close() was not cancelled with "return sal_False".
*/

{
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    SfxBoolItem aValue( pImpl->pMgr->GetType(), false);
    pBindings->GetDispatcher_Impl()->ExecuteList(
            pImpl->pMgr->GetType(),
            SfxCallMode::RECORD|SfxCallMode::SYNCHRON, { &aValue });

    return true;
}


SfxFloatingWindow::~SfxFloatingWindow()
{
    disposeOnce();
}

void SfxFloatingWindow::dispose()
{
    if ( pImpl && pImpl->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( nullptr );
    pImpl.reset();
    FloatingWindow::dispose();
}

void SfxFloatingWindow::Resize()

/*  [Description]

    This virtual method of the class FloatingWindow keeps track if a change
    in size has been made. When this method is overridden by a derived class,
    then the SfxFloatingWindow: Resize() must also be called.
*/

{
    FloatingWindow::Resize();
    if ( pImpl->bConstructed && pImpl->pMgr )
    {
        // start timer for saving window status information
        pImpl->aMoveIdle.Start();
    }
}

void SfxFloatingWindow::Move()
{
    FloatingWindow::Move();
    if ( pImpl->bConstructed && pImpl->pMgr )
    {
        // start timer for saving window status information
        pImpl->aMoveIdle.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG(SfxFloatingWindow, TimerHdl, Timer *, void)
{
    pImpl->aMoveIdle.Stop();
    if ( pImpl->bConstructed && pImpl->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        WindowStateMask nMask = WindowStateMask::Pos | WindowStateMask::State;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= WindowStateMask::Width | WindowStateMask::Height;
        pImpl->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SfxChildIdentifier::DOCKINGWINDOW, SfxDockingConfig::ALIGNDOCKINGWINDOW, pImpl->pMgr->GetType() );
    }
}


void SfxFloatingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        // FloatingWindows are not centered by default
        if ( !pImpl->aWinState.isEmpty() )
            SetWindowState( pImpl->aWinState );
        pImpl->bConstructed = true;
    }

    FloatingWindow::StateChanged( nStateChange );
}


void SfxFloatingWindow::Initialize(SfxChildWinInfo const *pInfo)

/*  [Description]

    Initialization of a class SfxFloatingWindow through a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor and
    should be called by the constructor of the derived class or from the
    SfxChildWindows.
*/
{
    if (pInfo)
        pImpl->aWinState = pInfo->aWinState;
}


void SfxFloatingWindow::FillInfo(SfxChildWinInfo& rInfo) const

/*  [Description]

    Fills a SfxChildWinInfo with specific data from SfxFloatingWindow,
    so that it can be written in the INI file. It is assumed that rinfo
    receives all other possible relevant data in the ChildWindow class.
    Insertions are marked with size and the ZoomIn flag.
    If this method is overridden, the base implementation must be called first.
*/

{
    rInfo.aSize  = aSize;
    if ( IsRollUp() )
        rInfo.nFlags |= SfxChildWindowFlags::ZOOMIN;
}

// SfxSingleTabDialog ----------------------------------------------------

IMPL_LINK_NOARG(SfxSingleTabDialog, OKHdl_Impl, Button*, void)

/*  [Description]

    Ok_Handler; FillItemSet() is called for setting of Page.
*/

{
    if ( !GetInputItemSet() )
    {
        // TabPage without ItemSet
        EndDialog( RET_OK );
        return;
    }

    if ( !GetOutputItemSet() )
    {
        CreateOutputItemSet( *GetInputItemSet() );
    }
    bool bModified = false;

    if ( pImpl->m_pSfxPage->HasExchangeSupport() )
    {
        DeactivateRC nRet = pImpl->m_pSfxPage->DeactivatePage( GetOutputSetImpl() );
        if ( nRet != DeactivateRC::LeavePage )
            return;
        else
            bModified = ( GetOutputItemSet()->Count() > 0 );
    }
    else
        bModified = pImpl->m_pSfxPage->FillItemSet( GetOutputSetImpl() );

    if ( bModified )
    {
        // Save user data in IniManager.
        pImpl->m_pSfxPage->FillUserData();
        OUString sData( pImpl->m_pSfxPage->GetUserData() );

        OUString sConfigId = OStringToOUString(pImpl->m_pSfxPage->GetConfigId(),
            RTL_TEXTENCODING_UTF8);
        SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
        aPageOpt.SetUserItem( USERITEM_NAME, makeAny( sData ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog();
}


SfxSingleTabDialog::SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet& rSet,
    const OUString& rID, const OUString& rUIXMLDescription)
    : SfxModalDialog(pParent, rID, rUIXMLDescription)
    , pImpl(new SingleTabDlgImpl)
{
    get(pOKBtn, "ok");
    pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    get(pCancelBtn, "cancel");
    get(pHelpBtn, "help");
    SetInputSet( &rSet );
}

SfxSingleTabDialog::SfxSingleTabDialog(vcl::Window* pParent, const SfxItemSet* pInSet,
    const OUString& rID, const OUString& rUIXMLDescription)
    : SfxModalDialog(pParent, rID, rUIXMLDescription)
    , pImpl(new SingleTabDlgImpl)
{
    get(pOKBtn, "ok");
    pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    get(pCancelBtn, "cancel");
    get(pHelpBtn, "help");
    SetInputSet( pInSet );
}

SfxSingleTabDialog::~SfxSingleTabDialog()
{
    disposeOnce();
}

void SfxSingleTabDialog::dispose()
{
    pImpl->m_pSfxPage.disposeAndClear();
    pImpl.reset();
    pOKBtn.clear();
    pCancelBtn.clear();
    pHelpBtn.clear();
    SfxModalDialog::dispose();
}

/*  [Description]

    Insert a (new) TabPage; an existing page is deleted.
    The passed on page is initialized with the initially given Itemset
    through calling Reset().
*/
void SfxSingleTabDialog::SetTabPage(SfxTabPage* pTabPage)
{
    pImpl->m_pSfxPage.disposeAndClear();
    pImpl->m_pSfxPage = pTabPage;

    if ( !pImpl->m_pSfxPage )
        return;

    // First obtain the user data, only then Reset()
    OUString sConfigId = OStringToOUString(pImpl->m_pSfxPage->GetConfigId(), RTL_TEXTENCODING_UTF8);
    SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
    Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
    OUString sUserData;
    aUserItem >>= sUserData;
    pImpl->m_pSfxPage->SetUserData(sUserData);
    pImpl->m_pSfxPage->Reset( GetInputItemSet() );
    pImpl->m_pSfxPage->Show();

    pHelpBtn->Show(Help::IsContextHelpEnabled());

    // Set TabPage text in the Dialog if there is any
    OUString sTitle(pImpl->m_pSfxPage->GetText());
    if (!sTitle.isEmpty())
        SetText(sTitle);

    // Dialog receives the HelpId of TabPage if there is any
    OString sHelpId(pImpl->m_pSfxPage->GetHelpId());
    if (!sHelpId.isEmpty())
        SetHelpId(sHelpId);
}

SfxDialogController::SfxDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                         const OString& rDialogId)
    : GenericDialogController(pParent, rUIFile, rDialogId)
{
    m_xDialog->SetInstallLOKNotifierHdl(LINK(this, SfxDialogController, InstallLOKNotifierHdl));
}

IMPL_STATIC_LINK_NOARG(SfxDialogController, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return SfxViewShell::Current();
}

SfxSingleTabDialogController::SfxSingleTabDialogController(weld::Widget *pParent, const SfxItemSet& rSet,
    const OUString& rUIXMLDescription, const OString& rID)
    : SfxOkDialogController(pParent, rUIXMLDescription, rID)
    , m_pInputSet(&rSet)
    , m_xContainer(m_xDialog->weld_content_area())
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xHelpBtn(m_xBuilder->weld_button("help"))
{
    m_xOKBtn->connect_clicked(LINK(this, SfxSingleTabDialogController, OKHdl_Impl));
}

SfxSingleTabDialogController::~SfxSingleTabDialogController()
{
    m_xSfxPage.disposeAndClear();
}

/*  [Description]

    Insert a (new) TabPage; an existing page is deleted.
    The passed on page is initialized with the initially given Itemset
    through calling Reset().
*/
void SfxSingleTabDialogController::SetTabPage(SfxTabPage* pTabPage)
{
    m_xSfxPage.disposeAndClear();
    m_xSfxPage = pTabPage;

    if (!m_xSfxPage)
        return;

    // First obtain the user data, only then Reset()
    OUString sConfigId = OStringToOUString(m_xSfxPage->GetConfigId(), RTL_TEXTENCODING_UTF8);
    SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
    Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
    OUString sUserData;
    aUserItem >>= sUserData;
    m_xSfxPage->SetUserData(sUserData);
    m_xSfxPage->Reset(GetInputItemSet());

    m_xHelpBtn->set_visible(Help::IsContextHelpEnabled());

    // Set TabPage text in the Dialog if there is any
    OUString sTitle(m_xSfxPage->GetText());
    if (!sTitle.isEmpty())
        m_xDialog->set_title(sTitle);

    // Dialog receives the HelpId of TabPage if there is any
    OString sHelpId(m_xSfxPage->GetHelpId());
    if (!sHelpId.isEmpty())
        m_xDialog->set_help_id(sHelpId);
}

/*  [Description]

    Ok_Handler; FillItemSet() is called for setting of Page.
*/
IMPL_LINK_NOARG(SfxSingleTabDialogController, OKHdl_Impl, weld::Button&, void)
{
    const SfxItemSet* pInputSet = GetInputItemSet();
    if (!pInputSet)
    {
        // TabPage without ItemSet
        m_xDialog->response(RET_OK);
        return;
    }

    if (!GetOutputItemSet())
    {
        CreateOutputItemSet(*pInputSet);
    }

    bool bModified = false;

    if (m_xSfxPage->HasExchangeSupport())
    {
        DeactivateRC nRet = m_xSfxPage->DeactivatePage(m_xOutputSet.get());
        if (nRet != DeactivateRC::LeavePage)
            return;
        else
            bModified = m_xOutputSet->Count() > 0;
    }
    else
        bModified = m_xSfxPage->FillItemSet(m_xOutputSet.get());

    if (bModified)
    {
        // Save user data in IniManager.
        m_xSfxPage->FillUserData();
        OUString sData(m_xSfxPage->GetUserData());

        OUString sConfigId = OStringToOUString(m_xSfxPage->GetConfigId(),
            RTL_TEXTENCODING_UTF8);
        SvtViewOptions aPageOpt(EViewType::TabPage, sConfigId);
        aPageOpt.SetUserItem( USERITEM_NAME, makeAny( sData ) );
        m_xDialog->response(RET_OK);
    }
    else
        m_xDialog->response(RET_CANCEL);
}

void SfxSingleTabDialogController::CreateOutputItemSet(const SfxItemSet& rSet)
{
    assert(!m_xOutputSet && "Double creation of OutputSet!");
    m_xOutputSet.reset(new SfxItemSet(rSet));
    m_xOutputSet->ClearItem();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
