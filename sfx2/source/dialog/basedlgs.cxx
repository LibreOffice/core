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
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/controldims.hrc>
#include <vcl/idle.hxx>

#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxhelp.hxx>
#include "workwin.hxx"
#include <sfx2/sfxresid.hxx>
#include "dialog.hrc"

using namespace ::com::sun::star::uno;

#define USERITEM_NAME "UserItem"

SingleTabDlgImpl::SingleTabDlgImpl()
        : m_pSfxPage(nullptr)
        , m_pLine(nullptr)
{
}

class SfxModelessDialog_Impl : public SfxListener
{
public:
    OString aWinState;
    SfxChildWindow* pMgr;
    bool            bConstructed;
    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    Idle            aMoveIdle;
};

void SfxModelessDialog_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        switch( pSimpleHint->GetId() )
        {
            case SFX_HINT_DYING:
                pMgr->Destroy();
                break;
        }
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
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint )
    {
        switch( pSimpleHint->GetId() )
        {
            case SFX_HINT_DYING:
                pMgr->Destroy();
                break;
        }
    }
}



void SfxModalDialog::SetDialogData_Impl()
{
    // save settings (position and user data)
    OUString sConfigId;
    if (isLayoutEnabled())
        sConfigId = OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8);
    else
    {
        SAL_WARN("sfx.config", "Dialog needs to be converted to .ui format");
        sConfigId = OUString::number(nUniqId);
    }

    SvtViewOptions aDlgOpt(E_DIALOG, sConfigId);
    aDlgOpt.SetWindowState(OStringToOUString(
        GetWindowState(WINDOWSTATE_MASK_POS), RTL_TEXTENCODING_ASCII_US));
    if ( !aExtraData.isEmpty() )
        aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( aExtraData ) );
}



void SfxModalDialog::GetDialogData_Impl()

/*  [Description]

    Helper function, reads the dialogue position from the ini file and
    puts them on the transferred window.
*/

{
    OUString sConfigId;
    if (isLayoutEnabled())
        sConfigId = OStringToOUString(GetHelpId(),RTL_TEXTENCODING_UTF8);
    else
    {
        SAL_WARN("sfx.config", "Dialog needs to be converted to .ui format");
        sConfigId = OUString::number(nUniqId);
    }

    SvtViewOptions aDlgOpt(E_DIALOG, sConfigId);
    if ( aDlgOpt.Exists() )
    {
        // load settings
        SetWindowState( OUStringToOString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );
        Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aExtraData = aTemp;
    }
}

void SfxModalDialog::init()
{
    GetDialogData_Impl();
}

SfxModalDialog::SfxModalDialog(vcl::Window *pParent, const OUString& rID, const OUString& rUIXMLDescription )
:   ModalDialog(pParent, rID, rUIXMLDescription),
    nUniqId(0), //todo: remove this member when the ResId using ctor is removed
    pInputSet(nullptr),
    pOutputSet(nullptr)
{
    init();
}

SfxModalDialog::~SfxModalDialog()
{
    disposeOnce();
}

void SfxModalDialog::dispose()
{
    SetDialogData_Impl();
    delete pOutputSet;
    ModalDialog::dispose();
}

void SfxModalDialog::CreateOutputItemSet( SfxItemPool& rPool )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
        pOutputSet = new SfxAllItemSet( rPool );
}



void SfxModalDialog::CreateOutputItemSet( const SfxItemSet& rSet )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
    {
        pOutputSet = new SfxItemSet( rSet );
        pOutputSet->ClearItem();
    }
}


void SfxModelessDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        if ( !pImp->aWinState.isEmpty() )
        {
            SetWindowState( pImp->aWinState );
        }
        else
        {
            Point aPos = GetPosPixel();
            if ( !aPos.X() )
            {
                aSize = GetSizePixel();

                Size aParentSize = GetParent()->GetOutputSizePixel();
                Size aDlgSize = GetSizePixel();
                aPos.X() += ( aParentSize.Width() - aDlgSize.Width() ) / 2;
                aPos.Y() += ( aParentSize.Height() - aDlgSize.Height() ) / 2;

                Point aPoint;
                Rectangle aRect = GetDesktopRectPixel();
                aPoint.X() = aRect.Right() - aDlgSize.Width();
                aPoint.Y() = aRect.Bottom() - aDlgSize.Height();

                aPoint = OutputToScreenPixel( aPoint );

                if ( aPos.X() > aPoint.X() )
                    aPos.X() = aPoint.X() ;
                if ( aPos.Y() > aPoint.Y() )
                    aPos.Y() = aPoint.Y();

                if ( aPos.X() < 0 ) aPos.X() = 0;
                if ( aPos.Y() < 0 ) aPos.Y() = 0;

                SetPosPixel( aPos );
            }
        }

        pImp->bConstructed = true;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SfxModelessDialog::Initialize(SfxChildWinInfo *pInfo)

/*  [Description]

    Initialization of the class SfxModelessDialog via a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor, this
    constructor should be called from the derived class or from the
    SfxChildWindows.
*/

{
    if (pInfo)
        pImp->aWinState = pInfo->aWinState;
}

void SfxModelessDialog::Resize()

/*  [Description]

    This virtual method of the class FloatingWindow keeps track if a change
    in size has been made. When this method is overridden by a derived class,
    then the SfxFloatingWindow: Resize() must also be called.
*/

{
    ModelessDialog::Resize();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveIdle.Start();
    }
}

void SfxModelessDialog::Move()
{
    ModelessDialog::Move();
    if ( pImp->bConstructed && pImp->pMgr && IsReallyVisible() )
    {
        // start timer for saving window status information
        pImp->aMoveIdle.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG_TYPED(SfxModelessDialog, TimerHdl, Idle *, void)
{
    pImp->aMoveIdle.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SfxChildIdentifier::DOCKINGWINDOW, SfxDockingConfig::ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
}

SfxModelessDialog::SfxModelessDialog(SfxBindings* pBindinx,
    SfxChildWindow *pCW, vcl::Window *pParent, const OUString& rID,
    const OUString& rUIXMLDescription)
    : ModelessDialog(pParent, rID, rUIXMLDescription)
{
    Init(pBindinx, pCW);
}

void SfxModelessDialog::Init(SfxBindings *pBindinx, SfxChildWindow *pCW)
{
    pBindings = pBindinx;
    pImp = new SfxModelessDialog_Impl;
    pImp->pMgr = pCW;
    pImp->bConstructed = false;
    SetUniqueId( GetHelpId() );
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveIdle.SetPriority(SchedulerPriority::RESIZE);
    pImp->aMoveIdle.SetIdleHdl(LINK(this,SfxModelessDialog,TimerHdl));
}

/*  [Description]

    If a ModelessDialog is enabled its ViewFrame wil be activated.
    This is necessary by PluginInFrames.
*/
bool SfxModelessDialog::Notify( NotifyEvent& rEvt )
{
    if ( pImp )
    {
        if ( rEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
            pImp->pMgr->Activate_Impl();
        }
        else if ( rEvt.GetType() == MouseNotifyEvent::LOSEFOCUS && !HasChildPathFocus() )
        {
            pBindings->SetActiveFrame( css::uno::Reference< css::frame::XFrame > () );
            pImp->pMgr->Deactivate_Impl();
        }
        else if( rEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            // First, allow KeyInput for Dialog functions ( TAB etc. )
            if ( !ModelessDialog::Notify( rEvt ) && SfxViewShell::Current() )
                // then also for valid global accelerators.
                return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
            return true;
        }
    }

    return ModelessDialog::Notify( rEvt );
}

SfxModelessDialog::~SfxModelessDialog()
{
    disposeOnce();
}

void SfxModelessDialog::dispose()
{
    if ( pImp->pMgr->GetFrame().is() && pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( nullptr );
    delete pImp;
    pImp = nullptr;
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
    SfxBoolItem aValue( pImp->pMgr->GetType(), false);
    pBindings->GetDispatcher_Impl()->Execute(
        pImp->pMgr->GetType(),
        SfxCallMode::RECORD|SfxCallMode::SYNCHRON, &aValue, 0L );
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



bool SfxFloatingWindow::Notify( NotifyEvent& rEvt )

/*  [Description]

    If a ModelessDialog is enabled, its ViewFrame will be activated.
    This necessary for the PluginInFrames.
*/

{
    if ( pImp )
    {
        if ( rEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
            pImp->pMgr->Activate_Impl();
        }
        else if ( rEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
        {
            if ( !HasChildPathFocus() )
            {
                pBindings->SetActiveFrame( nullptr );
                pImp->pMgr->Deactivate_Impl();
            }
        }
        else if( rEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            // First, allow KeyInput for Dialog functions
            if ( !FloatingWindow::Notify( rEvt ) && SfxViewShell::Current() )
                // then also for valid global accelerators.
                return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
            return true;
        }
    }

    return FloatingWindow::Notify( rEvt );
}

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        vcl::Window* pParent, WinBits nWinBits) :
    FloatingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = false;
    SetUniqueId( GetHelpId() );
    SetHelpId("");
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveIdle.SetPriority(SchedulerPriority::RESIZE);
    pImp->aMoveIdle.SetIdleHdl(LINK(this,SfxFloatingWindow,TimerHdl));
}

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        vcl::Window* pParent,
                        const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame) :
    FloatingWindow(pParent, rID, rUIXMLDescription, rFrame),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = false;

    //do we really need this odd helpid/uniqueid dance ?
    SetUniqueId( GetHelpId() );
    SetHelpId("");

    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveIdle.SetPriority(SchedulerPriority::RESIZE);
    pImp->aMoveIdle.SetIdleHdl(LINK(this,SfxFloatingWindow,TimerHdl));
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
    SfxBoolItem aValue( pImp->pMgr->GetType(), false);
    pBindings->GetDispatcher_Impl()->Execute(
            pImp->pMgr->GetType(),
            SfxCallMode::RECORD|SfxCallMode::SYNCHRON, &aValue, 0L );
    return true;
}



SfxFloatingWindow::~SfxFloatingWindow()
{
    disposeOnce();
}

void SfxFloatingWindow::dispose()
{
    if ( pImp && pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( nullptr );
    delete pImp;
    pImp = nullptr;
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
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveIdle.Start();
    }
}

void SfxFloatingWindow::Move()
{
    FloatingWindow::Move();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveIdle.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG_TYPED(SfxFloatingWindow, TimerHdl, Idle *, void)
{
    pImp->aMoveIdle.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SfxChildIdentifier::DOCKINGWINDOW, SfxDockingConfig::ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
}


void SfxFloatingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        // FloatingWindows are not centered by default
        if ( !pImp->aWinState.isEmpty() )
            SetWindowState( pImp->aWinState );
        pImp->bConstructed = true;
    }

    FloatingWindow::StateChanged( nStateChange );
}


void SfxFloatingWindow::Initialize(SfxChildWinInfo *pInfo)

/*  [Description]

    Initialization of a class SfxFloatingWindow through a SfxChildWinInfo.
    The initialization is done only in a 2nd step after the constructor and
    should be called by the constructor of the derived class or from the
    SfxChildWindows.
*/
{
    if (pInfo)
        pImp->aWinState = pInfo->aWinState;
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

IMPL_LINK_NOARG_TYPED(SfxSingleTabDialog, OKHdl_Impl, Button*, void)

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
        int nRet = pImpl->m_pSfxPage->DeactivatePage( GetOutputSetImpl() );
        if ( nRet != SfxTabPage::LEAVE_PAGE )
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
        if (sConfigId.isEmpty())
        {
            SAL_WARN("sfx.config", "Tabpage needs to be converted to .ui format");
            sConfigId = OUString::number(GetUniqId());
        }

        SvtViewOptions aPageOpt(E_TABPAGE, sConfigId);
        aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( sData ) ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog();
    return;
}



SfxSingleTabDialog::SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet& rSet,
    const OUString& rID, const OUString& rUIXMLDescription)
    : SfxModalDialog(pParent, rID, rUIXMLDescription)
    , fnGetRanges(nullptr)
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
    , fnGetRanges(nullptr)
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
    pImpl->m_pLine.disposeAndClear();
    delete pImpl;
    pOKBtn.clear();
    pCancelBtn.clear();
    pHelpBtn.clear();
    SfxModalDialog::dispose();
}

void SfxSingleTabDialog::SetTabPage(SfxTabPage* pTabPage,
    GetTabPageRanges pRangesFunc, sal_uInt32 nSettingsId)
/*  [Description]

    Insert a (new) TabPage; an existing page is deleted.
    The passed on page is initialized with the initially given Itemset
    through calling Reset().
*/

{
    SetUniqId(nSettingsId);
    pImpl->m_pSfxPage.disposeAndClear();
    pImpl->m_pSfxPage = pTabPage;
    fnGetRanges = pRangesFunc;

    if ( pImpl->m_pSfxPage )
    {
        // First obtain the user data, only then Reset()
        OUString sConfigId = OStringToOUString(pImpl->m_pSfxPage->GetConfigId(),
            RTL_TEXTENCODING_UTF8);
        if (sConfigId.isEmpty())
        {
            SAL_WARN("sfx.config", "Tabpage needs to be converted to .ui format");
            sConfigId = OUString::number(GetUniqId());
        }

        SvtViewOptions aPageOpt(E_TABPAGE, sConfigId);
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
        OString sUniqueId(pImpl->m_pSfxPage->GetUniqueId());
        if (!sUniqueId.isEmpty())
            SetUniqueId(sUniqueId);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
