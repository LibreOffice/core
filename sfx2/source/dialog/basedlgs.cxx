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
#include <svtools/fixedhyper.hxx>
#include <svtools/controldims.hrc>

#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/viewsh.hxx>
#include "sfx2/sfxhelp.hxx"
#include "workwin.hxx"
#include "sfx2/sfxresid.hxx"
#include "dialog.hrc"

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define USERITEM_NAME OUString("UserItem")

class SfxModelessDialog_Impl : public SfxListener
{
public:
    rtl::OString aWinState;
    SfxChildWindow* pMgr;
    sal_Bool            bConstructed;
    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    Timer           aMoveTimer;
};

void SfxModelessDialog_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
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
    rtl::OString aWinState;
    SfxChildWindow* pMgr;
    sal_Bool            bConstructed;
    Timer           aMoveTimer;

    void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

void SfxFloatingWindow_Impl::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_DYING:
                pMgr->Destroy();
                break;
        }
    }
}

// class SfxModalDefParentHelper -----------------------------------------

SfxModalDefParentHelper::SfxModalDefParentHelper( Window *pWindow)
{
    pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pWindow );
}

// -----------------------------------------------------------------------

SfxModalDefParentHelper::~SfxModalDefParentHelper()
{
    Application::SetDefDialogParent( pOld );
}

// -----------------------------------------------------------------------

void SfxModalDialog::SetDialogData_Impl()
{
    // save settings (position and user data)
    SvtViewOptions aDlgOpt( E_DIALOG, String::CreateFromInt32( nUniqId ) );
    aDlgOpt.SetWindowState(OStringToOUString(
        GetWindowState(WINDOWSTATE_MASK_POS), RTL_TEXTENCODING_ASCII_US));
    if ( aExtraData.Len() )
        aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aExtraData ) ) );
}

// -----------------------------------------------------------------------

void SfxModalDialog::GetDialogData_Impl()

/*  [Description]

    Helper function, reads the dialogue position from the ini file and
    puts them on the transfered window.
*/

{
    SvtViewOptions aDlgOpt( E_DIALOG, String::CreateFromInt32( nUniqId ) );
    if ( aDlgOpt.Exists() )
    {
        // load settings
        SetWindowState( rtl::OUStringToOString( aDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );
        Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            aExtraData = String( aTemp );
    }
}

// -----------------------------------------------------------------------

void SfxModalDialog::init()
{
    GetDialogData_Impl();
}

#define BASEPATH_SHARE_LAYER rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UIConfig"))
#define RELPATH_SHARE_LAYER rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice.cfg"))
#define SERVICENAME_PATHSETTINGS rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSettings"))

rtl::OUString getUIRootDir()
{
    namespace css = ::com::sun::star;

    /*to-do, check if user config has an override before using shared one, etc*/
    css::uno::Reference< css::beans::XPropertySet > xPathSettings(
        ::comphelper::getProcessServiceFactory()->createInstance(SERVICENAME_PATHSETTINGS),
                css::uno::UNO_QUERY_THROW);

    ::rtl::OUString sShareLayer;
    xPathSettings->getPropertyValue(BASEPATH_SHARE_LAYER) >>= sShareLayer;

    // "UIConfig" is a "multi path" ... use first part only here!
    sal_Int32 nPos = sShareLayer.indexOf(';');
    if (nPos > 0)
        sShareLayer = sShareLayer.copy(0, nPos);

    // Note: May be an user uses URLs without a final slash! Check it ...
    nPos = sShareLayer.lastIndexOf('/');
    if (nPos != sShareLayer.getLength()-1)
        sShareLayer += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

    sShareLayer += RELPATH_SHARE_LAYER; // folder
    sShareLayer += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    /*to-do, can we merge all this foo with existing soffice.cfg finding code, etc*/
    return sShareLayer;
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog(Window* pParent, const ResId &rResId )

/*  [Description]

    Constructor of the general base class for modal Dialoge;
    ResId is used as ID in ini-file. The saved position from there is set.
*/

:   ModalDialog(pParent, rResId),
    nUniqId(rResId.GetId()),
    pInputSet(0),
    pOutputSet(0)
{
    sal_Int32 nUIid = static_cast<sal_Int32>(nUniqId);
    rtl::OUString sPath = rtl::OUStringBuffer(getUIRootDir()).
        append(rResId.GetResMgr()->getPrefixName()).
        append("/ui/").
        append(nUIid).
        appendAscii(".ui").
        makeStringAndClear();
    fprintf(stderr, "path %s id %d\n", rtl::OUStringToOString(sPath, RTL_TEXTENCODING_UTF8).getStr(), nUniqId);

    osl::File aUIFile(sPath);
    osl::File::RC error = aUIFile.open(osl_File_OpenFlag_Read);
    if (error == osl::File::E_None)
        m_pUIBuilder = new VclBuilder(this, sPath, rtl::OString::valueOf(nUIid));
    init();
}

SfxModalDialog::SfxModalDialog(Window *pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription )
:   ModalDialog(pParent, 0), //todo
    nUniqId(0), //todo
    pInputSet(0),
    pOutputSet(0)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir() + rUIXMLDescription, rID);
    init();
}

// -----------------------------------------------------------------------

SfxModalDialog::SfxModalDialog(Window* pParent,
                               sal_uInt32 nUniqueId,
                               WinBits nWinStyle) :
/*  [Description]

    Constructor of the general base class for modal Dialoge;
    The ID for the ini-file wird explicilty handed over.
    The saved position from there is set.
*/

    ModalDialog(pParent, nWinStyle),
    nUniqId(nUniqueId),
    pInputSet(0),
    pOutputSet(0)
{
    init();
}

// -----------------------------------------------------------------------

SfxModalDialog::~SfxModalDialog()

/*  [Description]

    Destructor; writes the Dialog position in the ini-file.
*/

{
    SetDialogData_Impl();
    delete pOutputSet;
}

void SfxModalDialog::CreateOutputItemSet( SfxItemPool& rPool )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
        pOutputSet = new SfxAllItemSet( rPool );
}

// -----------------------------------------------------------------------

void SfxModalDialog::CreateOutputItemSet( const SfxItemSet& rSet )
{
    DBG_ASSERT( !pOutputSet, "Double creation of OutputSet!" );
    if (!pOutputSet)
    {
        pOutputSet = new SfxItemSet( rSet );
        pOutputSet->ClearItem();
    }
}

//-------------------------------------------------------------------------
void SfxModelessDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
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

        pImp->bConstructed = sal_True;
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
        pImp->aMoveTimer.Start();
    }
}

void SfxModelessDialog::Move()
{
    ModelessDialog::Move();
    if ( pImp->bConstructed && pImp->pMgr && IsReallyVisible() )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG(SfxModelessDialog, TimerHdl)
{
    pImp->aMoveTimer.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SFX_CHILDWIN_DOCKINGWINDOW, SFX_ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
    return 0;
}

// -----------------------------------------------------------------------

SfxModelessDialog::SfxModelessDialog( SfxBindings *pBindinx,
                        SfxChildWindow *pCW, Window *pParent,
                        const ResId& rResId ) :
    ModelessDialog(pParent, rResId),
    pBindings(pBindinx),
    pImp( new SfxModelessDialog_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = sal_False;
    SetUniqueId( GetHelpId() );
    SetHelpId("");
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxModelessDialog,TimerHdl));
}

// -----------------------------------------------------------------------

long SfxModelessDialog::Notify( NotifyEvent& rEvt )

/*  [Description]

    If a ModelessDialog is enabled its ViewFrame wil be activated.
    This is necessary by PluginInFrames.
*/

{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
        pImp->pMgr->Activate_Impl();
        Window* pWindow = rEvt.GetWindow();
        rtl::OString sHelpId;
        while ( sHelpId.isEmpty() && pWindow )
        {
            sHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( !sHelpId.isEmpty() )
            SfxHelp::OpenHelpAgent( &pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), sHelpId );
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS && !HasChildPathFocus() )
    {
        pBindings->SetActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
        pImp->pMgr->Deactivate_Impl();
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // First, allow KeyInput for Dialog functions ( TAB etc. )
        if ( !ModelessDialog::Notify( rEvt ) && SfxViewShell::Current() )
            // then also for valid global accelerators.
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return sal_True;
    }

    return ModelessDialog::Notify( rEvt );
}

// -----------------------------------------------------------------------

SfxModelessDialog::~SfxModelessDialog()

/*  [Description]

    Destructor
*/

{
    if ( pImp->pMgr->GetFrame().is() && pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );
    delete pImp;
}

//-------------------------------------------------------------------------

sal_Bool SfxModelessDialog::Close()

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots. If this is method is overridden by a derived class
    method, then the SfxModelessDialogWindow: Close() must be called afterwards
    if the Close() was not cancelled with "return sal_False".
*/

{
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    SfxBoolItem aValue( pImp->pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
        pImp->pMgr->GetType(),
        SFX_CALLMODE_RECORD|SFX_CALLMODE_SYNCHRON, &aValue, 0L );
    return sal_True;
}

//-------------------------------------------------------------------------

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
        rInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
}

// -----------------------------------------------------------------------

long SfxFloatingWindow::Notify( NotifyEvent& rEvt )

/*  [Description]

    If a ModelessDialog is enabled, its ViewFrame will be activated.
    This necessary for the PluginInFrames.
*/

{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
    {
        pBindings->SetActiveFrame( pImp->pMgr->GetFrame() );
        pImp->pMgr->Activate_Impl();
        Window* pWindow = rEvt.GetWindow();
        rtl::OString sHelpId;
        while ( sHelpId.isEmpty() && pWindow )
        {
            sHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( !sHelpId.isEmpty() )
            SfxHelp::OpenHelpAgent( &pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame(), sHelpId );
    }
    else if ( rEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !HasChildPathFocus() )
        {
            pBindings->SetActiveFrame( NULL );
            pImp->pMgr->Deactivate_Impl();
        }
    }
    else if( rEvt.GetType() == EVENT_KEYINPUT )
    {
        // First, allow KeyInput for Dialog functions
        if ( !FloatingWindow::Notify( rEvt ) && SfxViewShell::Current() )
            // then also for valid global accelerators.
            return SfxViewShell::Current()->GlobalKeyInput_Impl( *rEvt.GetKeyEvent() );
        return sal_True;
    }

    return FloatingWindow::Notify( rEvt );
}

// -----------------------------------------------------------------------

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent, WinBits nWinBits) :
    FloatingWindow (pParent, nWinBits),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = sal_False;
    SetUniqueId( GetHelpId() );
    SetHelpId("");
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxFloatingWindow,TimerHdl));
}

// -----------------------------------------------------------------------

SfxFloatingWindow::SfxFloatingWindow( SfxBindings *pBindinx,
                        SfxChildWindow *pCW,
                        Window* pParent,
                        const ResId& rResId) :
    FloatingWindow(pParent, rResId),
    pBindings(pBindinx),
    pImp( new SfxFloatingWindow_Impl )
{
    pImp->pMgr = pCW;
    pImp->bConstructed = sal_False;
    SetUniqueId( GetHelpId() );
    SetHelpId("");
    if ( pBindinx )
        pImp->StartListening( *pBindinx );
    pImp->aMoveTimer.SetTimeout(50);
    pImp->aMoveTimer.SetTimeoutHdl(LINK(this,SfxFloatingWindow,TimerHdl));
}

//-------------------------------------------------------------------------

sal_Bool SfxFloatingWindow::Close()

/*  [Description]

    The window is closed when the ChildWindow is destroyed by running the
    ChildWindow-slots. If this is method is overridden by a derived class
    method, then the SfxModelessDialogWindow: Close) must be called afterwards
    if the Close() was not cancelled with "return sal_False".
*/

{
    // Execute with Parameters, since Toggle is ignored by some ChildWindows.
    SfxBoolItem aValue( pImp->pMgr->GetType(), sal_False);
    pBindings->GetDispatcher_Impl()->Execute(
            pImp->pMgr->GetType(),
            SFX_CALLMODE_RECORD|SFX_CALLMODE_SYNCHRON, &aValue, 0L );
    return sal_True;
}

// -----------------------------------------------------------------------

SfxFloatingWindow::~SfxFloatingWindow()

/*  [Description]

    Destructor
*/

{
    if ( pImp->pMgr->GetFrame() == pBindings->GetActiveFrame() )
        pBindings->SetActiveFrame( NULL );
    delete pImp;
}

//-------------------------------------------------------------------------

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
        pImp->aMoveTimer.Start();
    }
}

void SfxFloatingWindow::Move()
{
    FloatingWindow::Move();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        // start timer for saving window status information
        pImp->aMoveTimer.Start();
    }
}

/*
    Implements a timer event that is triggered by a move or resize of the window
    This will save config information to Views.xcu with a small delay
*/
IMPL_LINK_NOARG(SfxFloatingWindow, TimerHdl)
{
    pImp->aMoveTimer.Stop();
    if ( pImp->bConstructed && pImp->pMgr )
    {
        if ( !IsRollUp() )
            aSize = GetSizePixel();
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        pImp->aWinState = GetWindowState( nMask );
        GetBindings().GetWorkWindow_Impl()->ConfigChild_Impl( SFX_CHILDWIN_DOCKINGWINDOW, SFX_ALIGNDOCKINGWINDOW, pImp->pMgr->GetType() );
    }
    return 0;
}

//-------------------------------------------------------------------------
void SfxFloatingWindow::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        // FloatingWindows are not centered by default
        if ( !pImp->aWinState.isEmpty() )
            SetWindowState( pImp->aWinState );
        pImp->bConstructed = sal_True;
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
    pImp->aWinState = pInfo->aWinState;
}

//-------------------------------------------------------------------------

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
        rInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
}

// SfxSingleTabDialog ----------------------------------------------------

IMPL_LINK_NOARG(SfxSingleTabDialog, OKHdl_Impl)

/*  [Description]

    Ok_Handler; FillItemSet() is called for setting of Page.
*/

{
    if ( !GetInputItemSet() )
    {
        // TabPage without ItemSet
        EndDialog( RET_OK );
        return 1;
    }

    if ( !GetOutputItemSet() )
    {
        CreateOutputItemSet( *GetInputItemSet() );
    }
    sal_Bool bModified = sal_False;

    if ( pImpl->m_pSfxPage->HasExchangeSupport() )
    {
        int nRet = pImpl->m_pSfxPage->DeactivatePage( GetOutputSetImpl() );
        if ( nRet != SfxTabPage::LEAVE_PAGE )
            return 0;
        else
            bModified = ( GetOutputItemSet()->Count() > 0 );
    }
    else
        bModified = pImpl->m_pSfxPage->FillItemSet( *GetOutputSetImpl() );

    if ( bModified )
    {
        // Save user data in IniManager.
        pImpl->m_pSfxPage->FillUserData();
        String sData( pImpl->m_pSfxPage->GetUserData() );
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( GetUniqId() ) );
        aPageOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( sData ) ) );
        EndDialog( RET_OK );
    }
    else
        EndDialog( RET_CANCEL );
    return 0;
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window *pParent,
    const SfxItemSet& rSet,
    sal_uInt16 nUniqueId
) :

/*  [Description]

    Constructor of the general base class for SingleTab-Dialoge;
    ID for the ini-file is handed over.
*/

    SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pImpl           ( new SingleTabDlgImpl )
{
    DBG_WARNING( "please use the constructor with ViewFrame" );
    SetInputSet( &rSet );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::SfxSingleTabDialog
(
    Window* pParent,
    sal_uInt16 nUniqueId,
    const SfxItemSet* pInSet
)

/*  [Description]

    Constructor of the general base class for SingleTab-Dialoge;
    ID for the ini-file is handed over.
    Deprecated.
 */

:   SfxModalDialog( pParent, nUniqueId, WinBits( WB_STDMODAL | WB_3DLOOK ) ),

    pOKBtn          ( 0 ),
    pCancelBtn      ( 0 ),
    pHelpBtn        ( 0 ),
    pImpl           ( new SingleTabDlgImpl )
{
    DBG_WARNING( "please use the constructor with ViewFrame" );
    SetInputSet( pInSet );
}

// -----------------------------------------------------------------------

SfxSingleTabDialog::~SfxSingleTabDialog()
{
    delete pOKBtn;
    delete pCancelBtn;
    delete pHelpBtn;
    delete pImpl->m_pTabPage;
    delete pImpl->m_pSfxPage;
    delete pImpl->m_pLine;
    delete pImpl;
}

// -----------------------------------------------------------------------

void SfxSingleTabDialog::SetTabPage( SfxTabPage* pTabPage,
                                     GetTabPageRanges pRangesFunc )
/*  [Description]

    Insert a (new) TabPage; an existing page is deleted.
    The passed on page is initialized with the initially given Itemset
    through calling Reset().
*/

{
    if ( !pOKBtn )
    {
        pOKBtn = new OKButton( this, WB_DEFBUTTON );
        pOKBtn->SetClickHdl( LINK( this, SfxSingleTabDialog, OKHdl_Impl ) );
    }
    if ( !pCancelBtn )
        pCancelBtn = new CancelButton( this );
    if ( !pHelpBtn )
        pHelpBtn = new HelpButton( this );

    if ( pImpl->m_pTabPage )
        delete pImpl->m_pTabPage;
    if ( pImpl->m_pSfxPage )
        delete pImpl->m_pSfxPage;
    pImpl->m_pSfxPage = pTabPage;
    fnGetRanges = pRangesFunc;

    if ( pImpl->m_pSfxPage )
    {
        // First obtain the user data, only then Reset()
        SvtViewOptions aPageOpt( E_TABPAGE, String::CreateFromInt32( GetUniqId() ) );
        String sUserData;
        Any aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
        OUString aTemp;
        if ( aUserItem >>= aTemp )
            sUserData = String( aTemp );
        pImpl->m_pSfxPage->SetUserData( sUserData );
        pImpl->m_pSfxPage->Reset( *GetInputItemSet() );
        pImpl->m_pSfxPage->Show();

        // Adjust size and position
        pImpl->m_pSfxPage->SetPosPixel( Point() );
        Size aOutSz( pImpl->m_pSfxPage->GetSizePixel() );
        Size aBtnSiz = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );
        Point aPnt( aOutSz.Width(), LogicToPixel( Point( 0, 6 ), MAP_APPFONT ).Y() );
        aOutSz.Width() += aBtnSiz.Width() + LogicToPixel( Size( 6, 0 ), MAP_APPFONT ).Width();
        SetOutputSizePixel( aOutSz );
        pOKBtn->SetPosSizePixel( aPnt, aBtnSiz );
        pOKBtn->Show();
        aPnt.Y() = LogicToPixel( Point( 0, 23 ), MAP_APPFONT ).Y();
        pCancelBtn->SetPosSizePixel( aPnt, aBtnSiz );
        pCancelBtn->Show();
        aPnt.Y() = LogicToPixel( Point( 0, 43 ), MAP_APPFONT ).Y();
        pHelpBtn->SetPosSizePixel( aPnt, aBtnSiz );

        if ( Help::IsContextHelpEnabled() )
            pHelpBtn->Show();

        // Set TabPage text in the Dialog
        SetText( pImpl->m_pSfxPage->GetText() );

        // Dialog recieves the HelpId of TabPage
        SetHelpId( pImpl->m_pSfxPage->GetHelpId() );
        SetUniqueId( pImpl->m_pSfxPage->GetUniqueId() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
