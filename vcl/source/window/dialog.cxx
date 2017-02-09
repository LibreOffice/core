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

#include <config_features.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include <tools/debug.hxx>

#include <tools/rc.h>
#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>

#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/dialog.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/decoview.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>
#include <salframe.hxx>

#include <iostream>

static OString ImplGetDialogText( Dialog* pDialog )
{
    OStringBuffer aErrorStr(OUStringToOString(
        pDialog->GetText(), RTL_TEXTENCODING_UTF8));

    OUString sMessage;
    if (MessBox* pMessBox = dynamic_cast<MessBox*>(pDialog))
    {
        sMessage = pMessBox->GetMessText();
    }
    else if (MessageDialog* pMessDialog = dynamic_cast<MessageDialog*>(pDialog))
    {
        sMessage = pMessDialog->get_primary_text();
    }

    if (!sMessage.isEmpty())
    {
        aErrorStr.append(", ");
        aErrorStr.append(OUStringToOString(
            sMessage, RTL_TEXTENCODING_UTF8));
    }
    return aErrorStr.makeStringAndClear();
}

static bool ImplIsMnemonicCtrl( vcl::Window* pWindow )
{
    if( ! pWindow->GetSettings().GetStyleSettings().GetAutoMnemonic() )
        return false;

    if ( (pWindow->GetType() == WINDOW_RADIOBUTTON) ||
         (pWindow->GetType() == WINDOW_CHECKBOX) ||
         (pWindow->GetType() == WINDOW_TRISTATEBOX) ||
         (pWindow->GetType() == WINDOW_PUSHBUTTON) )
        return true;

    if ( pWindow->GetType() == WINDOW_FIXEDTEXT )
    {
        FixedText *pText = static_cast<FixedText*>(pWindow);
        if (pText->get_mnemonic_widget())
            return true;
        //This is the legacy pre-layout logic which we retain
        //until we can be sure we can remove it
        if ( pWindow->GetStyle() & (WB_INFO | WB_NOLABEL) )
            return false;
        vcl::Window* pNextWindow = pWindow->GetWindow( GetWindowType::Next );
        if ( !pNextWindow )
            return false;
        pNextWindow = pNextWindow->GetWindow( GetWindowType::Client );
        if ( !(pNextWindow->GetStyle() & WB_TABSTOP) ||
             (pNextWindow->GetType() == WINDOW_FIXEDTEXT) ||
             (pNextWindow->GetType() == WINDOW_GROUPBOX) ||
             (pNextWindow->GetType() == WINDOW_RADIOBUTTON) ||
             (pNextWindow->GetType() == WINDOW_CHECKBOX) ||
             (pNextWindow->GetType() == WINDOW_TRISTATEBOX) ||
             (pNextWindow->GetType() == WINDOW_PUSHBUTTON) )
            return false;

        return true;
    }

    return false;
}

// Called by native error dialog popup implementations
void ImplHideSplash()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
            pSVData->mpIntroWindow->Hide();
}

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
vcl::Window * nextLogicalChildOfParent(vcl::Window *pTopLevel, vcl::Window *pChild)
{
    vcl::Window *pLastChild = pChild;

    if (isContainerWindow(*pChild))
        pChild = pChild->GetWindow(GetWindowType::FirstChild);
    else
        pChild = pChild->GetWindow(GetWindowType::Next);

    while (!pChild)
    {
        vcl::Window *pParent = pLastChild->GetParent();
        if (!pParent)
            return nullptr;
        if (pParent == pTopLevel)
            return nullptr;
        pLastChild = pParent;
        pChild = pParent->GetWindow(GetWindowType::Next);
    }

    if (pChild && isContainerWindow(*pChild))
        pChild = nextLogicalChildOfParent(pTopLevel, pChild);

    return pChild;
}

vcl::Window * prevLogicalChildOfParent(vcl::Window *pTopLevel, vcl::Window *pChild)
{
    vcl::Window *pLastChild = pChild;

    if (isContainerWindow(*pChild))
        pChild = pChild->GetWindow(GetWindowType::LastChild);
    else
        pChild = pChild->GetWindow(GetWindowType::Prev);

    while (!pChild)
    {
        vcl::Window *pParent = pLastChild->GetParent();
        if (!pParent)
            return nullptr;
        if (pParent == pTopLevel)
            return nullptr;
        pLastChild = pParent;
        pChild = pParent->GetWindow(GetWindowType::Prev);
    }

    if (pChild && isContainerWindow(*pChild))
        pChild = prevLogicalChildOfParent(pTopLevel, pChild);

    return pChild;
}

//Get first window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
vcl::Window * firstLogicalChildOfParent(vcl::Window *pTopLevel)
{
    vcl::Window *pChild = pTopLevel->GetWindow(GetWindowType::FirstChild);
    if (pChild && isContainerWindow(*pChild))
        pChild = nextLogicalChildOfParent(pTopLevel, pChild);
    return pChild;
}

void ImplWindowAutoMnemonic( vcl::Window* pWindow )
{
    MnemonicGenerator   aMnemonicGenerator;
    vcl::Window*                 pGetChild;
    vcl::Window*                 pChild;

    // register the assigned mnemonics
    pGetChild = pWindow->GetWindow( GetWindowType::FirstChild );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
        pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
    }

    // take the Controls of the dialog into account for TabPages
    if ( pWindow->GetType() == WINDOW_TABPAGE )
    {
        vcl::Window* pParent = pWindow->GetParent();
        if ( pParent->GetType() == WINDOW_TABCONTROL )
            pParent = pParent->GetParent();

        if ( (pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
        {
            pGetChild = pParent->GetWindow( GetWindowType::FirstChild );
            while ( pGetChild )
            {
                pChild = pGetChild->ImplGetWindow();
                aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
                pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
            }
        }
    }

    // assign mnemonics to Controls which have none
    pGetChild = pWindow->GetWindow( GetWindowType::FirstChild );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        if ( ImplIsMnemonicCtrl( pChild ) )
        {
            OUString aText = pChild->GetText();
            OUString aNewText = aMnemonicGenerator.CreateMnemonic( aText );
            if ( aText != aNewText )
                pChild->SetText( aNewText );
        }

        pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
    }
}

void ImplHandleControlAccelerator( vcl::Window* pWindow, bool bShow )
{
    Control *pControl = dynamic_cast<Control*>(pWindow->ImplGetWindow());
    if (pControl && pControl->GetText().indexOf('~') != -1)
    {
        pControl->SetShowAccelerator( bShow );
        pControl->Invalidate(InvalidateFlags::Update);
    }
}

static VclButtonBox* getActionArea(Dialog *pDialog)
{
    VclButtonBox *pButtonBox = nullptr;
    if (pDialog->isLayoutEnabled())
    {
        vcl::Window *pBox = pDialog->GetWindow(GetWindowType::FirstChild);
        vcl::Window *pChild = pBox->GetWindow(GetWindowType::LastChild);
        while (pChild)
        {
            pButtonBox = dynamic_cast<VclButtonBox*>(pChild);
            if (pButtonBox)
                break;
            pChild = pChild->GetWindow(GetWindowType::Prev);
        }
    }
    return pButtonBox;
}

static vcl::Window* getActionAreaButtonList(Dialog *pDialog)
{
    VclButtonBox* pButtonBox = getActionArea(pDialog);
    if (pButtonBox)
        return pButtonBox->GetWindow(GetWindowType::FirstChild);
    return pDialog->GetWindow(GetWindowType::FirstChild);
}

static PushButton* ImplGetDefaultButton( Dialog* pDialog )
{
    vcl::Window* pChild = getActionAreaButtonList(pDialog);
    while ( pChild )
    {
        if ( pChild->ImplIsPushButton() )
        {
            PushButton* pPushButton = static_cast<PushButton*>(pChild);
            if ( pPushButton->ImplIsDefButton() )
                return pPushButton;
        }

        pChild = pChild->GetWindow( GetWindowType::Next );
    }

    return nullptr;
}

static PushButton* ImplGetOKButton( Dialog* pDialog )
{
    vcl::Window* pChild = getActionAreaButtonList(pDialog);
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_OKBUTTON )
            return static_cast<PushButton*>(pChild);

        pChild = pChild->GetWindow( GetWindowType::Next );
    }

    return nullptr;
}

static PushButton* ImplGetCancelButton( Dialog* pDialog )
{
    vcl::Window* pChild = getActionAreaButtonList(pDialog);

    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_CANCELBUTTON )
            return static_cast<PushButton*>(pChild);

        pChild = pChild->GetWindow( GetWindowType::Next );
    }

    return nullptr;
}

static void ImplMouseAutoPos( Dialog* pDialog )
{
    MouseSettingsOptions nMouseOptions = pDialog->GetSettings().GetMouseSettings().GetOptions();
    if ( nMouseOptions & MouseSettingsOptions::AutoCenterPos )
    {
        Size aSize = pDialog->GetOutputSizePixel();
        pDialog->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
    else if ( nMouseOptions & MouseSettingsOptions::AutoDefBtnPos )
    {
        vcl::Window* pWindow = ImplGetDefaultButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetOKButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetCancelButton( pDialog );
        if ( !pWindow )
            pWindow = pDialog;
        Size aSize = pWindow->GetOutputSizePixel();
        pWindow->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
}

struct DialogImpl
{
    long    mnResult;
    bool    mbStartedModal;
    Link<Dialog&,void>  maEndDialogHdl;

    DialogImpl() : mnResult( -1 ), mbStartedModal( false ) {}
};

void Dialog::ImplInitDialogData()
{
    mpWindowImpl->mbDialog  = true;
    mpPrevExecuteDlg        = nullptr;
    mbInExecute             = false;
    mbInClose               = false;
    mbModalMode             = false;
    mpContentArea.clear();
    mpActionArea.clear();
    mnMousePositioned       = 0;
    mpDialogImpl            = new DialogImpl;
}

void Dialog::ImplInit( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag )
{
    SystemWindowFlags nSysWinMode = Application::GetSystemWindowMode();

    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;
    nStyle |= WB_ROLLABLE;

    // Now, all Dialogs are per default system windows !!!
    nStyle |= WB_SYSTEMWINDOW;

    if (eFlag == InitFlag::NoParent)
        pParent = nullptr;
    else if (!pParent) // parent is NULL: get the default Dialog parent
    {
        pParent = Application::GetDefDialogParent();
        if ( !pParent && !(nStyle & WB_SYSTEMWINDOW) )
            pParent = ImplGetSVData()->maWinData.mpAppWin;

        // If Parent is disabled, then we search for a modal dialog
        // in this frame
        if ( pParent && (!pParent->IsInputEnabled() || pParent->IsInModalMode()) )
        {
            ImplSVData* pSVData = ImplGetSVData();
            Dialog*     pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
            while ( pExeDlg )
            {
                // only if visible and enabled
                if ( pParent->ImplGetFirstOverlapWindow()->IsWindowOrChild( pExeDlg, true ) &&
                     pExeDlg->IsReallyVisible() &&
                     pExeDlg->IsEnabled() && pExeDlg->IsInputEnabled() && !pExeDlg->IsInModalMode() )
                {
                    pParent = pExeDlg;
                    break;
                }

                pExeDlg = pExeDlg->mpPrevExecuteDlg;
            }
        }
    }

    if ( !pParent || (nStyle & WB_SYSTEMWINDOW) ||
         (pParent->mpWindowImpl->mpFrameData->mbNeedSysWindow && !(nSysWinMode & SystemWindowFlags::NOAUTOMODE)) ||
         (nSysWinMode & SystemWindowFlags::DIALOG) )
    {
        // create window with a small border ?
        if ( (nStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE)) == WB_BORDER )
        {
            VclPtrInstance<ImplBorderWindow> pBorderWin( pParent, nStyle, BorderWindowStyle::Frame );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, nullptr );
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
            mpWindowImpl->mpBorderWindow  = pBorderWin;
            mpWindowImpl->mpRealParent    = pParent;
        }
        else
        {
            mpWindowImpl->mbFrame         = true;
            mpWindowImpl->mbOverlapWin    = true;
            SystemWindow::ImplInit( pParent, (nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE)) | WB_CLOSEABLE, nullptr );
            // Now set all style bits
            mpWindowImpl->mnStyle = nStyle;
        }
    }
    else
    {
        VclPtrInstance<ImplBorderWindow> pBorderWin( pParent, nStyle, BorderWindowStyle::Overlap | BorderWindowStyle::Border );
        SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, nullptr );
        pBorderWin->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        mpWindowImpl->mpRealParent    = pParent;
    }

    SetActivateMode( ActivateModeFlags::GrabFocus );

    ImplInitSettings();
}

void Dialog::ApplySettings(vcl::RenderContext& rRenderContext)
{
    if (IsControlBackground())
    {
        // user override
        SetBackground(GetControlBackground());
    }
    else if (rRenderContext.IsNativeControlSupported(ControlType::WindowBackground, ControlPart::BackgroundDialog))
    {
        // NWF background
        mpWindowImpl->mnNativeBackground = ControlPart::BackgroundDialog;
        EnableChildTransparentMode();
    }
    else
    {
        // fallback to settings color
        rRenderContext.SetBackground(GetSettings().GetStyleSettings().GetDialogColor());
    }
}

void Dialog::ImplInitSettings()
{
    // user override
    if (IsControlBackground())
        SetBackground(GetControlBackground());
    // NWF background
    else if( IsNativeControlSupported(ControlType::WindowBackground, ControlPart::BackgroundDialog))
    {
        mpWindowImpl->mnNativeBackground = ControlPart::BackgroundDialog;
        EnableChildTransparentMode();
    }
    // fallback to settings color
    else
        SetBackground(GetSettings().GetStyleSettings().GetDialogColor());
}

Dialog::Dialog( WindowType nType )
    : SystemWindow( nType )
    , mnInitFlag(InitFlag::Default)
{
    ImplInitDialogData();
}

void VclBuilderContainer::disposeBuilder()
{
    if (m_pUIBuilder)
        m_pUIBuilder->disposeBuilder();
}

OUString VclBuilderContainer::getUIRootDir()
{
    /*to-do, check if user config has an override before using shared one, etc*/
    css::uno::Reference< css::util::XPathSettings > xPathSettings = css::util::thePathSettings::get(
        ::comphelper::getProcessComponentContext() );

    OUString sShareLayer = xPathSettings->getBasePathShareLayer();

    // "UIConfig" is a "multi path" ... use first part only here!
    sal_Int32 nPos = sShareLayer.indexOf(';');
    if (nPos > 0)
        sShareLayer = sShareLayer.copy(0, nPos);

    // Note: May be an user uses URLs without a final slash! Check it ...
    if (!sShareLayer.endsWith("/"))
        sShareLayer += "/";

    sShareLayer += "soffice.cfg/";
    /*to-do, can we merge all this foo with existing soffice.cfg finding code, etc*/
    return sShareLayer;
}

//we can't change sizeable after the fact, so need to defer until we know and then
//do the init. Find the real parent stashed in mpDialogParent.
void Dialog::doDeferredInit(WinBits nBits)
{
    VclPtr<vcl::Window> pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits, mnInitFlag);
    mbIsDefferedInit = false;
}

Dialog::Dialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription)
    : SystemWindow(WINDOW_DIALOG)
    , mnInitFlag(InitFlag::Default)
{
    ImplInitDialogData();
    loadUI(pParent, OUStringToOString(rID, RTL_TEXTENCODING_UTF8), rUIXMLDescription);
}

Dialog::Dialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, WindowType nType, InitFlag eFlag)
    : SystemWindow(nType)
    , mnInitFlag(eFlag)
{
    ImplInitDialogData();
    loadUI(pParent, OUStringToOString(rID, RTL_TEXTENCODING_UTF8), rUIXMLDescription);
}

Dialog::Dialog(vcl::Window* pParent, WinBits nStyle, InitFlag eFlag)
    : SystemWindow(WINDOW_DIALOG)
    , mnInitFlag(eFlag)
{
    ImplInitDialogData();
    ImplInit( pParent, nStyle, eFlag );
}

void Dialog::set_action_area(VclButtonBox* pBox)
{
    mpActionArea.set(pBox);
}

void Dialog::set_content_area(VclBox* pBox)
{
    mpContentArea.set(pBox);
}

void Dialog::settingOptimalLayoutSize(Window *pBox)
{
    const DialogStyle& rDialogStyle =
        GetSettings().GetStyleSettings().GetDialogStyle();
    VclBox * pBox2 = static_cast<VclBox*>(pBox);
    pBox2->set_border_width(rDialogStyle.content_area_border);
    pBox2->set_spacing(pBox2->get_spacing() +
        rDialogStyle.content_area_spacing);

    VclButtonBox *pActionArea = getActionArea(this);
    if (pActionArea)
    {
        pActionArea->set_border_width(rDialogStyle.action_area_border);
        pActionArea->set_spacing(rDialogStyle.button_spacing);
    }
}

Dialog::~Dialog()
{
    disposeOnce();
}

void Dialog::dispose()
{
    delete mpDialogImpl;
    mpDialogImpl = nullptr;
    mpPrevExecuteDlg.clear();
    mpActionArea.clear();
    mpContentArea.clear();
    SystemWindow::dispose();
}

IMPL_LINK_NOARG_TYPED(Dialog, ImplAsyncCloseHdl, void*, void)
{
    Close();
}

bool Dialog::ImplHandleCmdEvent( const CommandEvent& rCEvent )
{
    if (rCEvent.GetCommand() == CommandEventId::ModKeyChange && ImplGetSVData()->maNWFData.mbAutoAccel)
    {
        const CommandModKeyData *pCData = rCEvent.GetModKeyData ();
        bool bShowAccel =  pCData && pCData->IsMod2();

        Window *pGetChild = firstLogicalChildOfParent(this);
        while (pGetChild)
        {
            if ( pGetChild->GetType() == WINDOW_TABCONTROL )
            {
                 // find currently shown tab page
                 TabControl* pTabControl = static_cast<TabControl*>( pGetChild );
                 TabPage* pTabPage = pTabControl->GetTabPage( pTabControl->GetCurPageId() );
                 vcl::Window* pTabPageChild =  firstLogicalChildOfParent( pTabPage );

                 // and go through its children
                 while ( pTabPageChild )
                 {
                     ImplHandleControlAccelerator(pTabPageChild, bShowAccel);
                     pTabPageChild = nextLogicalChildOfParent(pTabPage, pTabPageChild);
                 }
            }

            ImplHandleControlAccelerator( pGetChild, bShowAccel );
            pGetChild = nextLogicalChildOfParent(this, pGetChild);
        }
        return true;
    }
    return false;
}

bool Dialog::Notify( NotifyEvent& rNEvt )
{
    // first call the base class due to Tab control
    bool bRet = SystemWindow::Notify( rNEvt );
    if ( !bRet )
    {
        if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            vcl::KeyCode    aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16      nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) &&
                 ((GetStyle() & WB_CLOSEABLE) || ImplGetCancelButton( this ) || ImplGetOKButton( this )) )
            {
                // #i89505# for the benefit of slightly mentally challenged implementations
                // like e.g. SfxModelessDialog which destroy themselves inside Close()
                // post this Close asynchronous so we can leave our key handler before
                // we get destroyed
                PostUserEvent( LINK( this, Dialog, ImplAsyncCloseHdl ), nullptr, true);
                return true;
            }
        }
        else if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            // make sure the dialog is still modal
            // changing focus between application frames may
            // have re-enabled input for our parent
            if( mbInExecute && mbModalMode )
            {
                SetModalInputMode( false );
                SetModalInputMode( true );

                // #93022# def-button might have changed after show
                if( !mnMousePositioned )
                {
                    mnMousePositioned = 1;
                    ImplMouseAutoPos( this );
                }

            }
        }
        else if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
        {
            if (ImplHandleCmdEvent( *rNEvt.GetCommandEvent()))
                return true;
        }
    }

    return bRet;
}

//What we really want here is something that gives the available width and
//height of a users screen, taking away the space taken up the OS
//taskbar, menus, etc.
Size bestmaxFrameSizeForScreenSize(const Size &rScreenSize)
{
    long w = rScreenSize.Width();
    if (w <= 800)
        w -= 15;
    else if (w <= 1024)
        w -= 65;
    else
        w -= 115;

    long h = rScreenSize.Height();
    if (h <= 768)
        h -= 50;
    else
        h -= 100;

    return Size(w, h);
}

void Dialog::StateChanged( StateChangedType nType )
{
    if (nType == StateChangedType::InitShow)
    {
        DoInitialLayout();

        if ( !HasChildPathFocus() || HasFocus() )
            GrabFocusToFirstControl();
        if ( !(GetStyle() & WB_CLOSEABLE) )
        {
            if ( ImplGetCancelButton( this ) || ImplGetOKButton( this ) )
            {
                if ( ImplGetBorderWindow() )
                    static_cast<ImplBorderWindow*>(ImplGetBorderWindow())->SetCloseButton();
            }
        }

        ImplMouseAutoPos( this );
    }

    SystemWindow::StateChanged( nType );

    if (nType == StateChangedType::ControlBackground)
    {
        ImplInitSettings();
        Invalidate();
    }
}

void Dialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SystemWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

bool Dialog::Close()
{
    VclPtr<vcl::Window> xWindow = this;
    CallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( xWindow->IsDisposed() )
        return false;

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() && !IsInExecute() )
        return false;

    mbInClose = true;

    if ( !(GetStyle() & WB_CLOSEABLE) )
    {
        bool bRet = true;
        PushButton* pButton = ImplGetCancelButton( this );
        if ( pButton )
            pButton->Click();
        else
        {
            pButton = ImplGetOKButton( this );
            if ( pButton )
                pButton->Click();
            else
                bRet = false;
        }
        if ( xWindow->IsDisposed() )
            return true;
        return bRet;
    }

    if ( IsInExecute() )
    {
        EndDialog();
        mbInClose = false;
        return true;
    }
    else
    {
        mbInClose = false;
        return SystemWindow::Close();
    }
}

bool Dialog::ImplStartExecuteModal()
{
    if ( mbInExecute )
    {
#ifdef DBG_UTIL
        OStringBuffer aErrorStr;
        aErrorStr.append("Dialog::StartExecuteModal() is called in Dialog::StartExecuteModal(): ");
        aErrorStr.append(ImplGetDialogText(this));
        OSL_FAIL(aErrorStr.getStr());
#endif
        return false;
    }

    switch ( Application::GetDialogCancelMode() )
    {
    case Application::DIALOG_CANCEL_OFF:
        break;
    case Application::DIALOG_CANCEL_SILENT:
        SAL_INFO(
            "vcl",
            "Dialog \"" << ImplGetDialogText(this).getStr()
                << "\"cancelled in silent mode");
        return false;
    default: // default cannot happen
    case Application::DIALOG_CANCEL_FATAL:
        std::abort();
    }

#ifdef DBG_UTIL
    vcl::Window* pParent = GetParent();
    if ( pParent )
    {
        pParent = pParent->ImplGetFirstOverlapWindow();
        DBG_ASSERT( pParent->IsReallyVisible(),
                    "Dialog::StartExecuteModal() - Parent not visible" );
        DBG_ASSERT( pParent->IsInputEnabled(),
                    "Dialog::StartExecuteModal() - Parent input disabled, use another parent to ensure modality!" );
        DBG_ASSERT( ! pParent->IsInModalMode(),
                    "Dialog::StartExecuteModal() - Parent already modally disabled, use another parent to ensure modality!" );

    }
#endif

    ImplSVData* pSVData = ImplGetSVData();

     // link all dialogs which are being executed
    mpPrevExecuteDlg = pSVData->maWinData.mpLastExecuteDlg;
    pSVData->maWinData.mpLastExecuteDlg = this;

    // stop capturing, in order to have control over the dialog
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();
    EnableInput();

    if ( GetParent() )
    {
        NotifyEvent aNEvt( MouseNotifyEvent::EXECUTEDIALOG, this );
        GetParent()->CompatNotify( aNEvt );
    }
    mbInExecute = true;
    SetModalInputMode( true );

    // FIXME: no layouting, workaround some clipping issues
    ImplAdjustNWFSizes();

    Show();

    pSVData->maAppData.mnModalMode++;
    return true;
}

void Dialog::ImplEndExecuteModal()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode--;
}

short Dialog::Execute()
{
#if HAVE_FEATURE_DESKTOP

    setDeferredProperties();

    if ( !ImplStartExecuteModal() )
        return 0;

    VclPtr<vcl::Window> xWindow = this;

    // Yield util EndDialog is called or dialog gets destroyed
    // (the latter should not happen, but better safe than sorry
    while ( !xWindow->IsDisposed() && mbInExecute )
        Application::Yield();

    ImplEndExecuteModal();

#ifdef DBG_UTIL
    assert (!mpDialogParent || !mpDialogParent->IsDisposed());
#endif
    if ( !xWindow->IsDisposed() )
        xWindow.clear();
#ifdef DBG_UTIL
    else
    {
        OSL_FAIL( "Dialog::Execute() - Dialog destroyed in Execute()" );
    }
#endif

    long nRet = mpDialogImpl->mnResult;
    mpDialogImpl->mnResult = -1;
    return (short)nRet;

#else

    // touch_ui_dialog_modal was dummied out both for Android and iOS (well, TiledLibreOffice anyway)
    // For Android it returned MLODialogOK always, for iOS Cancel. Let's go with OK.
    // MLODialogResult result = touch_ui_dialog_modal(kind, ImplGetDialogText(this).getStr());
    return RET_OK;

#endif
}

// virtual
void Dialog::StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl )
{
    if ( !ImplStartExecuteModal() )
        return;

    mpDialogImpl->maEndDialogHdl = rEndDialogHdl;
    mpDialogImpl->mbStartedModal = true;
}

void Dialog::EndDialog( long nResult )
{
    if ( mbInExecute )
    {
        SetModalInputMode( false );

        // remove dialog from the list of dialogs which are being executed
        ImplSVData* pSVData = ImplGetSVData();
        Dialog* pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
        while ( pExeDlg )
        {
            if ( pExeDlg == this )
            {
                pSVData->maWinData.mpLastExecuteDlg = mpPrevExecuteDlg;
                break;
            }
            pExeDlg = pExeDlg->mpPrevExecuteDlg;
        }
        // set focus to previous modal dialogue if it is modal for
        // the same frame parent (or NULL)
        if( mpPrevExecuteDlg )
        {
            vcl::Window* pFrameParent = ImplGetFrameWindow()->ImplGetParent();
            vcl::Window* pPrevFrameParent = mpPrevExecuteDlg->ImplGetFrameWindow()->ImplGetParent();
            if( ( !pFrameParent && !pPrevFrameParent ) ||
                ( pFrameParent && pPrevFrameParent && pFrameParent->ImplGetFrame() == pPrevFrameParent->ImplGetFrame() )
                )
            {
                mpPrevExecuteDlg->GrabFocus();
            }
        }
        mpPrevExecuteDlg = nullptr;

        Hide();
        if ( GetParent() )
        {
            NotifyEvent aNEvt( MouseNotifyEvent::ENDEXECUTEDIALOG, this );
            GetParent()->CompatNotify( aNEvt );
        }

        mpDialogImpl->mnResult = nResult;

        if ( mpDialogImpl->mbStartedModal )
        {
            ImplEndExecuteModal();
            if (mpDialogImpl->maEndDialogHdl.IsSet())
            {
                mpDialogImpl->maEndDialogHdl.Call( *this );
                mpDialogImpl->maEndDialogHdl = Link<Dialog&,void>();
            }
            mpDialogImpl->mbStartedModal = false;
            mpDialogImpl->mnResult = -1;
        }
        mbInExecute = false;
    }
}

long Dialog::GetResult() const
{
    return mpDialogImpl->mnResult;
}

void Dialog::EndAllDialogs( vcl::Window* pParent )
{
    ImplSVData* pSVData = ImplGetSVData();
    Dialog* pTempModDialog;
    Dialog* pModDialog = pSVData->maWinData.mpLastExecuteDlg;
    while (pModDialog)
    {
        pTempModDialog = pModDialog->mpPrevExecuteDlg;
        if(!pParent || pParent->IsWindowOrChild(pModDialog,true))
        {
            pModDialog->EndDialog();
            pModDialog->PostUserEvent( Link<void*,void>() );
        }
        pModDialog = pTempModDialog;
    }
}

void Dialog::SetModalInputMode( bool bModal )
{
    if ( bModal == mbModalMode )
        return;

    mbModalMode = bModal;
    if ( bModal )
    {
        // Disable the prev Modal Dialog, because our dialog must close at first,
        // before the other dialog can be closed (because the other dialog
        // is on stack since our dialog returns)
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, true ) )
            mpPrevExecuteDlg->EnableInput( false, this );

        // determine next overlap dialog parent
        vcl::Window* pParent = GetParent();
        if ( pParent )
        {
            // #103716# dialogs should always be modal to the whole frame window
            // #115933# disable the whole frame hierarchy, useful if our parent
            // is a modeless dialog
            mpDialogParent = pParent->mpWindowImpl->mpFrameWindow;
            mpDialogParent->ImplIncModalCount();
        }
    }
    else
    {
        if ( mpDialogParent )
        {
            // #115933# re-enable the whole frame hierarchy again (see above)
            // note that code in getfocus assures that we do not accidentally enable
            // windows that were disabled before
            mpDialogParent->ImplDecModalCount();
        }

        // Enable the prev Modal Dialog
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, true ) )
        {
            mpPrevExecuteDlg->EnableInput( true, this );
            // ensure continued modality of prev dialog
            // do not change modality counter

            // #i119994# need find the last modal dialog before reactive it
            Dialog * pPrevModalDlg = mpPrevExecuteDlg;

            while( pPrevModalDlg && !pPrevModalDlg->IsModalInputMode() )
                pPrevModalDlg = pPrevModalDlg->mpPrevExecuteDlg;

            if( pPrevModalDlg &&
            ( pPrevModalDlg == mpPrevExecuteDlg.get()
                || !pPrevModalDlg->IsWindowOrChild( this, true ) ) )
            {
                mpPrevExecuteDlg->SetModalInputMode( false );
                mpPrevExecuteDlg->SetModalInputMode( true );
            }
        }
    }
    ImplGetFrame()->SetModal(bModal);
}

void Dialog::SetModalInputMode( bool bModal, bool bSubModalDialogs )
{
    if ( bSubModalDialogs )
    {
        vcl::Window* pOverlap = ImplGetFirstOverlapWindow();
        pOverlap = pOverlap->mpWindowImpl->mpFirstOverlap;
        while ( pOverlap )
        {
            if ( pOverlap->IsDialog() )
                static_cast<Dialog*>(pOverlap)->SetModalInputMode( bModal, true );
            pOverlap = pOverlap->mpWindowImpl->mpNext;
        }
    }

    SetModalInputMode( bModal );
}

void Dialog::GrabFocusToFirstControl()
{
    vcl::Window* pFocusControl;

    // find focus control, even if the dialog has focus
    if ( HasFocus() )
        pFocusControl = nullptr;
    else
    {
        // prefer a child window which had focus before
        pFocusControl = ImplGetFirstOverlapWindow()->mpWindowImpl->mpLastFocusWindow;
        // find the control out of the dialog control
        if ( pFocusControl )
            pFocusControl = ImplFindDlgCtrlWindow( pFocusControl );
    }
    // no control had the focus before or the control is not
    // part of the tab-control, now give focus to the
    // first control in the tab-control
    if ( !pFocusControl ||
         !(pFocusControl->GetStyle() & WB_TABSTOP) ||
         !isVisibleInLayout(pFocusControl) ||
         !isEnabledInLayout(pFocusControl) || !pFocusControl->IsInputEnabled() )
    {
        sal_uInt16 n = 0;
        pFocusControl = ImplGetDlgWindow( n, GetDlgWindowType::First );
    }
    if ( pFocusControl )
        pFocusControl->ImplControlFocus( GetFocusFlags::Init );
}

void Dialog::GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder, sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    ScopedVclPtrInstance<ImplBorderWindow> aImplWin( static_cast<vcl::Window*>(const_cast<Dialog *>(this)), WB_BORDER|WB_STDWORK, BorderWindowStyle::Overlap );
    aImplWin->GetBorder( rLeftBorder, rTopBorder, rRightBorder, rBottomBorder );
}

void Dialog::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags )
{
    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );

    Wallpaper aWallpaper = GetBackground();
    if ( !aWallpaper.IsBitmap() )
        ImplInitSettings();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetLineColor();

    if ( aWallpaper.IsBitmap() )
        pDev->DrawBitmapEx( aPos, aSize, aWallpaper.GetBitmap() );
    else
    {
        pDev->SetFillColor( aWallpaper.GetColor() );
        pDev->DrawRect( Rectangle( aPos, aSize ) );
    }

    if (!( GetStyle() & WB_NOBORDER ))
    {
        ScopedVclPtrInstance< ImplBorderWindow > aImplWin( this, WB_BORDER|WB_STDWORK, BorderWindowStyle::Overlap );
        aImplWin->SetText( GetText() );
        aImplWin->setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
        aImplWin->SetDisplayActive( true );
        aImplWin->InitView();

        aImplWin->Draw( Rectangle( aPos, aSize ), pDev, aPos );
    }

    pDev->Pop();
}

void Dialog::queue_resize(StateChangedType eReason)
{
    if (IsInClose())
        return;
    SystemWindow::queue_resize(eReason);
}

bool Dialog::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "border-width")
        set_border_width(rValue.toInt32());
    else
        return SystemWindow::set_property(rKey, rValue);
    return true;
}

VclBuilderContainer::VclBuilderContainer()
    : m_pUIBuilder(nullptr)
{
}

VclBuilderContainer::~VclBuilderContainer()
{
    delete m_pUIBuilder;
}

ModelessDialog::ModelessDialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, InitFlag eFlag)
    : Dialog(pParent, rID, rUIXMLDescription, WINDOW_MODELESSDIALOG, eFlag)
{
}

ModalDialog::ModalDialog( vcl::Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODALDIALOG )
{
    ImplInit( pParent, nStyle );
}

ModalDialog::ModalDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription ) :
    Dialog(pParent, rID, rUIXMLDescription, WINDOW_MODALDIALOG)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
