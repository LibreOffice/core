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
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star::uno;

#define USERITEM_NAME "UserItem"

class SfxModelessDialog_Impl : public SfxListener
{
public:
    OString aWinState;
    SfxChildWindow* pMgr;
    bool            bClosing;
    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    Idle            aMoveIdle;
};

void SfxModelessDialog_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (pMgr && rHint.GetId() == SfxHintId::Dying) {
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

void SfxModelessDialogController::Init(SfxBindings *pBindinx, SfxChildWindow *pCW)
{
    m_pBindings = pBindinx;
    m_xImpl.reset(new SfxModelessDialog_Impl);
    m_xImpl->pMgr = pCW;
    m_xImpl->bClosing = false;
    if (pBindinx)
        m_xImpl->StartListening( *pBindinx );
}

/*  [Description]

    If a ModelessDialog is enabled its ViewFrame will be activated.
    This is necessary by PluginInFrames.
*/
IMPL_LINK_NOARG(SfxDialogController, FocusChangeHdl, weld::Widget&, void)
{
    if (m_xDialog->has_toplevel_focus())
        Activate();
    else
        Deactivate();
}

void SfxModelessDialogController::Activate()
{
    if (!m_xImpl)
        return;
    m_pBindings->SetActiveFrame(m_xImpl->pMgr->GetFrame());
    m_xImpl->pMgr->Activate_Impl();
}

void SfxModelessDialogController::Deactivate()
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

void SfxDialogController::EndDialog()
{
    if (!m_xDialog->get_visible())
        return;
    response(RET_CLOSE);
}

bool SfxModelessDialogController::IsClosing() const
{
    return m_xImpl->bClosing;
}

void SfxModelessDialogController::EndDialog()
{
    if (m_xImpl->bClosing)
        return;
    // In the case of async dialogs, the call to SfxDialogController::EndDialog
    // may delete this object, so keep myself alive for the duration of this
    // stack frame.
    auto aHoldSelf = shared_from_this();
    m_xImpl->bClosing = true;
    SfxDialogController::EndDialog();
    if (!m_xImpl)
        return;
    m_xImpl->bClosing = false;
}

void SfxModelessDialogController::ChildWinDispose()
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

SfxDialogController::SfxDialogController(weld::Widget* pParent, const OUString& rUIFile,
                                         const OString& rDialogId)
    : GenericDialogController(pParent, rUIFile, rDialogId,
                                    comphelper::LibreOfficeKit::isActive()
                                    && comphelper::LibreOfficeKit::isMobilePhone(SfxLokHelper::getView()))
{
    m_xDialog->SetInstallLOKNotifierHdl(LINK(this, SfxDialogController, InstallLOKNotifierHdl));
    m_xDialog->connect_toplevel_focus_changed(LINK(this, SfxDialogController, FocusChangeHdl));
}

IMPL_STATIC_LINK_NOARG(SfxDialogController, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return SfxViewShell::Current();
}

SfxSingleTabDialogController::SfxSingleTabDialogController(weld::Widget *pParent, const SfxItemSet* pSet,
    const OUString& rUIXMLDescription, const OString& rID)
    : SfxOkDialogController(pParent, rUIXMLDescription, rID)
    , m_pInputSet(pSet)
    , m_xContainer(m_xDialog->weld_content_area())
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
    , m_xHelpBtn(m_xBuilder->weld_button("help"))
{
    m_xOKBtn->connect_clicked(LINK(this, SfxSingleTabDialogController, OKHdl_Impl));
}

SfxSingleTabDialogController::~SfxSingleTabDialogController()
{
}

/*  [Description]

    Insert a (new) TabPage; an existing page is deleted.
    The passed on page is initialized with the initially given Itemset
    through calling Reset().
*/
void SfxSingleTabDialogController::SetTabPage(std::unique_ptr<SfxTabPage> xTabPage)
{
    m_xSfxPage = std::move(xTabPage);
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
    OUString sTitle(m_xSfxPage->GetPageTitle());
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
