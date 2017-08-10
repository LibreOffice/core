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
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include <tools/debug.hxx>

#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>

#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include "window.h"

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
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/virdev.hxx>
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

    if ( (pWindow->GetType() == WindowType::RADIOBUTTON) ||
         (pWindow->GetType() == WindowType::CHECKBOX) ||
         (pWindow->GetType() == WindowType::TRISTATEBOX) ||
         (pWindow->GetType() == WindowType::PUSHBUTTON) )
        return true;

    if ( pWindow->GetType() == WindowType::FIXEDTEXT )
    {
        FixedText *pText = static_cast<FixedText*>(pWindow);
        if (pText->get_mnemonic_widget())
            return true;
        //This is the legacy pre-layout logic which we retain
        //until we can be sure we can remove it
        if (pWindow->GetStyle() & WB_NOLABEL)
            return false;
        vcl::Window* pNextWindow = pWindow->GetWindow( GetWindowType::Next );
        if ( !pNextWindow )
            return false;
        pNextWindow = pNextWindow->GetWindow( GetWindowType::Client );
        return !(!(pNextWindow->GetStyle() & WB_TABSTOP) ||
                 (pNextWindow->GetType() == WindowType::FIXEDTEXT) ||
                 (pNextWindow->GetType() == WindowType::GROUPBOX) ||
                 (pNextWindow->GetType() == WindowType::RADIOBUTTON) ||
                 (pNextWindow->GetType() == WindowType::CHECKBOX) ||
                 (pNextWindow->GetType() == WindowType::TRISTATEBOX) ||
                 (pNextWindow->GetType() == WindowType::PUSHBUTTON));
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

void Accelerator::GenerateAutoMnemonicsOnHierarchy(vcl::Window* pWindow)
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
    if ( pWindow->GetType() == WindowType::TABPAGE )
    {
        vcl::Window* pParent = pWindow->GetParent();
        if ( pParent->GetType() == WindowType::TABCONTROL )
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

static VclButtonBox* getActionArea(Dialog const *pDialog)
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

static vcl::Window* getActionAreaButtonList(Dialog const *pDialog)
{
    VclButtonBox* pButtonBox = getActionArea(pDialog);
    if (pButtonBox)
        return pButtonBox->GetWindow(GetWindowType::FirstChild);
    return pDialog->GetWindow(GetWindowType::FirstChild);
}

static PushButton* ImplGetDefaultButton( Dialog const * pDialog )
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

static PushButton* ImplGetOKButton( Dialog const * pDialog )
{
    vcl::Window* pChild = getActionAreaButtonList(pDialog);
    while ( pChild )
    {
        if ( pChild->GetType() == WindowType::OKBUTTON )
            return static_cast<PushButton*>(pChild);

        pChild = pChild->GetWindow( GetWindowType::Next );
    }

    return nullptr;
}

static PushButton* ImplGetCancelButton( Dialog const * pDialog )
{
    vcl::Window* pChild = getActionAreaButtonList(pDialog);

    while ( pChild )
    {
        if ( pChild->GetType() == WindowType::CANCELBUTTON )
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
    mpDialogRenderable = nullptr;
    mpWindowImpl->mbDialog  = true;
    mpPrevExecuteDlg        = nullptr;
    mbInExecute             = false;
    mbInClose               = false;
    mbModalMode             = false;
    mbPaintComplete         = false;
    mpContentArea.clear();
    mpActionArea.clear();
    mnMousePositioned       = 0;
    mpDialogImpl.reset(new DialogImpl);
}

vcl::Window* Dialog::GetDefaultParent(WinBits nStyle)
{
    vcl::Window* pParent = Application::GetDefDialogParent();
    if (!pParent && !(nStyle & WB_SYSTEMWINDOW))
        pParent = ImplGetSVData()->maWinData.mpAppWin;

    // If Parent is disabled, then we search for a modal dialog
    // in this frame
    if (pParent && (!pParent->IsInputEnabled() || pParent->IsInModalMode()))
    {
        ImplSVData* pSVData = ImplGetSVData();
        Dialog*     pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
        while (pExeDlg)
        {
            // only if visible and enabled
            if (pParent->ImplGetFirstOverlapWindow()->IsWindowOrChild(pExeDlg, true) &&
                pExeDlg->IsReallyVisible() &&
                pExeDlg->IsEnabled() && pExeDlg->IsInputEnabled() && !pExeDlg->IsInModalMode())
            {
                pParent = pExeDlg;
                break;
            }

            pExeDlg = pExeDlg->mpPrevExecuteDlg;
        }
    }

    return pParent;
}

void Dialog::ImplInit( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag )
{
    SystemWindowFlags nSysWinMode = Application::GetSystemWindowMode();

    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;
    nStyle |= WB_ROLLABLE;

    // Now, all Dialogs are per default system windows !!!
    nStyle |= WB_SYSTEMWINDOW;

    if (InitFlag::NoParent == eFlag || InitFlag::NoParentCentered == eFlag)
    {
        pParent = nullptr;
    }
    else if (!pParent) // parent is NULL: get the default Dialog parent
    {
        pParent = Dialog::GetDefaultParent(nStyle);
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
            SystemWindow::ImplInit( pParent, (nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_STANDALONE)) | WB_CLOSEABLE, nullptr );
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
    OUString sShareLayer("$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/config/soffice.cfg/");
    rtl::Bootstrap::expandMacros(sShareLayer);
    return sShareLayer;
}

//we can't change sizeable after the fact, so need to defer until we know and then
//do the init. Find the real parent stashed in mpDialogParent.
void Dialog::doDeferredInit(WinBits nBits)
{
    VclPtr<vcl::Window> pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits, mnInitFlag);
    mbIsDeferredInit = false;
}

Dialog::Dialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription)
    : SystemWindow(WindowType::DIALOG)
    , mnInitFlag(InitFlag::Default)
    , maID(rID)
{
    ImplInitDialogData();
    loadUI(pParent, OUStringToOString(rID, RTL_TEXTENCODING_UTF8), rUIXMLDescription);
}

Dialog::Dialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, WindowType nType, InitFlag eFlag)
    : SystemWindow(nType)
    , mnInitFlag(eFlag)
    , maID(rID)
{
    ImplInitDialogData();
    loadUI(pParent, OUStringToOString(rID, RTL_TEXTENCODING_UTF8), rUIXMLDescription);
}

Dialog::Dialog(vcl::Window* pParent, WinBits nStyle, InitFlag eFlag)
    : SystemWindow(WindowType::DIALOG)
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
    mpDialogImpl.reset();
    mpPrevExecuteDlg.clear();
    mpActionArea.clear();
    mpContentArea.clear();

    css::uno::Reference< css::uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
    css::uno::Reference<css::frame::XGlobalEventBroadcaster> xEventBroadcaster(css::frame::theGlobalEventBroadcaster::get(xContext), css::uno::UNO_QUERY_THROW);
    css::document::DocumentEvent aObject;
    aObject.EventName = "DialogClosed";
    xEventBroadcaster->documentEventOccured(aObject);
    UITestLogger::getInstance().log("DialogClosed");

    SystemWindow::dispose();
}

IMPL_LINK_NOARG(Dialog, ImplAsyncCloseHdl, void*, void)
{
    Close();
}

bool Dialog::EventNotify( NotifyEvent& rNEvt )
{
    // first call the base class due to Tab control
    bool bRet = SystemWindow::EventNotify( rNEvt );
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

    return Size(std::max<long>(w, 640 - 15),
                std::max<long>(h, 480 - 50));
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

    if (!mbModalMode && nType == StateChangedType::Visible)
    {
        css::uno::Reference< css::uno::XComponentContext > xContext(
                            comphelper::getProcessComponentContext() );
        css::uno::Reference<css::frame::XGlobalEventBroadcaster> xEventBroadcaster(css::frame::theGlobalEventBroadcaster::get(xContext), css::uno::UNO_QUERY_THROW);
        css::document::DocumentEvent aObject;
        aObject.EventName = "ModelessDialogVisible";
        xEventBroadcaster->documentEventOccured(aObject);
        UITestLogger::getInstance().log("Modeless Dialog Visible");
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
    CallEventListeners( VclEventId::WindowClose );
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
        SAL_WARN( "vcl", "Dialog::StartExecuteModal() is called in Dialog::StartExecuteModal(): "
                    << ImplGetDialogText(this) );
#endif
        return false;
    }

    switch ( Application::GetDialogCancelMode() )
    {
    case Application::DialogCancelMode::Off:
        break;
    case Application::DialogCancelMode::Silent:
        SAL_INFO(
            "vcl",
            "Dialog \"" << ImplGetDialogText(this)
                << "\"cancelled in silent mode");
        return false;
    default: // default cannot happen
    case Application::DialogCancelMode::Fatal:
        std::abort();
    }

#ifdef DBG_UTIL
    vcl::Window* pParent = GetParent();
    if ( pParent )
    {
        pParent = pParent->ImplGetFirstOverlapWindow();
        SAL_WARN_IF( !pParent->IsReallyVisible(), "vcl",
                    "Dialog::StartExecuteModal() - Parent not visible" );
        SAL_WARN_IF( !pParent->IsInputEnabled(), "vcl",
                    "Dialog::StartExecuteModal() - Parent input disabled, use another parent to ensure modality!" );
        SAL_WARN_IF(  pParent->IsInModalMode(), "vcl",
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
    ImplGetFrame()->SetModal(true);
    SetModalInputMode(true);

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

void Dialog::PrePaint(vcl::RenderContext& rRenderContext)
{
    SystemWindow::PrePaint(rRenderContext);
    mbPaintComplete = false;
}

void Dialog::PostPaint(vcl::RenderContext& rRenderContext)
{
    SystemWindow::PostPaint(rRenderContext);
    mbPaintComplete = true;
}

std::vector<OString> Dialog::getAllPageUIXMLDescriptions() const
{
    // default has no pages
    return std::vector<OString>();
}

bool Dialog::selectPageByUIXMLDescription(const OString& /*rUIXMLDescription*/)
{
    // default cannot select anything (which is okay, return true)
    return true;
}

void Dialog::registerDialogRenderable(vcl::IDialogRenderable* pDialogRenderable)
{
    if (pDialogRenderable && !mpDialogRenderable)
    {
        mpDialogRenderable = pDialogRenderable;
    }
}

void Dialog::paintDialog(VirtualDevice& rDevice)
{
    setDeferredProperties();
    ImplAdjustNWFSizes();
    Show();
    ToTop();
    ensureRepaint();

    PaintToDevice(&rDevice, Point(0, 0), Size());
}

Size Dialog::PaintActiveFloatingWindow(VirtualDevice& rDevice)
{
    Size aRet;
    ImplSVData* pSVData = ImplGetSVData();
    FloatingWindow* pFirstFloat = pSVData->maWinData.mpFirstFloat;
    if (pFirstFloat)
    {
        // TODO:: run a while loop here and check all the active floating
        // windows ( chained together, cf. pFirstFloat->mpNextFloat )
        // For now just assume that the active floating window is the one we
        // want to render
        if (pFirstFloat->GetParentDialog() == this)
        {
            pFirstFloat->PaintToDevice(&rDevice, Point(0, 0), Size());
            aRet = ::isLayoutEnabled(pFirstFloat) ? pFirstFloat->get_preferred_size() : pFirstFloat->GetSizePixel();
        }

        pFirstFloat = nullptr;
    }

    return aRet;
}

void Dialog::LogicMouseButtonDownChild(const MouseEvent& rMouseEvent)
{
    assert(comphelper::LibreOfficeKit::isActive());

    ImplSVData* pSVData = ImplGetSVData();
    FloatingWindow* pFirstFloat = pSVData->maWinData.mpFirstFloat;
    if (pFirstFloat && pFirstFloat->GetParentDialog() == this)
    {
        ImplWindowFrameProc(pFirstFloat->ImplGetBorderWindow(), SalEvent::ExternalMouseButtonDown, &rMouseEvent);
    }
}

void Dialog::LogicMouseButtonUpChild(const MouseEvent& rMouseEvent)
{
    assert(comphelper::LibreOfficeKit::isActive());

    ImplSVData* pSVData = ImplGetSVData();
    FloatingWindow* pFirstFloat = pSVData->maWinData.mpFirstFloat;
    if (pFirstFloat && pFirstFloat->GetParentDialog() == this)
    {
        ImplWindowFrameProc(pFirstFloat->ImplGetBorderWindow(), SalEvent::ExternalMouseButtonUp, &rMouseEvent);
    }
}

void Dialog::LogicMouseMoveChild(const MouseEvent& rMouseEvent)
{
    assert(comphelper::LibreOfficeKit::isActive());

    ImplSVData* pSVData = ImplGetSVData();
    FloatingWindow* pFirstFloat = pSVData->maWinData.mpFirstFloat;
    if (pFirstFloat && pFirstFloat->GetParentDialog() == this)
    {
        ImplWindowFrameProc(pFirstFloat->ImplGetBorderWindow(), SalEvent::ExternalMouseMove, &rMouseEvent);
    }
}

void Dialog::InvalidateFloatingWindow(const Point& rPos)
{
    if (comphelper::LibreOfficeKit::isActive() && mpDialogRenderable && !maID.isEmpty())
    {
        mpDialogRenderable->notifyDialogChild(maID, "invalidate", rPos);
    }
}

void Dialog::CloseFloatingWindow()
{
    if (comphelper::LibreOfficeKit::isActive() && mpDialogRenderable && !maID.isEmpty())
    {
        mpDialogRenderable->notifyDialogChild(maID, "close", Point(0, 0));
    }
}

void Dialog::LogicInvalidate(const tools::Rectangle* /*pRectangle*/)
{
    if (!comphelper::LibreOfficeKit::isDialogPainting() && mpDialogRenderable && !maID.isEmpty())
    {
        mpDialogRenderable->notifyDialog(maID, "invalidate");
    }
}

void Dialog::LogicMouseButtonDown(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    ImplWindowFrameProc(this, SalEvent::ExternalMouseButtonDown, &rMouseEvent);
}

void Dialog::LogicMouseButtonUp(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    ImplWindowFrameProc(this, SalEvent::ExternalMouseButtonUp, &rMouseEvent);
}

void Dialog::LogicMouseMove(const MouseEvent& rMouseEvent)
{
    // When we're not doing tiled rendering, then positions must be passed as pixels.
    assert(comphelper::LibreOfficeKit::isActive());

    ImplWindowFrameProc(this, SalEvent::ExternalMouseMove, &rMouseEvent);
}

void Dialog::LOKKeyInput(const KeyEvent& rKeyEvent)
{
    assert(comphelper::LibreOfficeKit::isActive());

    ImplWindowFrameProc(this, SalEvent::ExternalKeyInput, &rKeyEvent);
}

void Dialog::LOKKeyUp(const KeyEvent& rKeyEvent)
{
    assert(comphelper::LibreOfficeKit::isActive());

    ImplWindowFrameProc(this, SalEvent::ExternalKeyUp, &rKeyEvent);
}

void Dialog::ensureRepaint()
{
    // ensure repaint
    Invalidate();
    mbPaintComplete = false;

    while (!mbPaintComplete)
    {
        Application::Yield();
    }
}

Bitmap Dialog::createScreenshot()
{
    // same prerequisites as in Execute()
    setDeferredProperties();
    ImplAdjustNWFSizes();
    Show();
    ToTop();
    ensureRepaint();

    return GetBitmap(Point(), GetOutputSizePixel());
}

short Dialog::Execute()
{
#if HAVE_FEATURE_DESKTOP

    setDeferredProperties();

    if ( !ImplStartExecuteModal() )
        return 0;

    VclPtr<vcl::Window> xWindow = this;

    css::uno::Reference< css::uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
    css::uno::Reference<css::frame::XGlobalEventBroadcaster> xEventBroadcaster(css::frame::theGlobalEventBroadcaster::get(xContext), css::uno::UNO_QUERY_THROW);
    css::document::DocumentEvent aObject;
    aObject.EventName = "DialogExecute";
    xEventBroadcaster->documentEventOccured(aObject);
    UITestLogger::getInstance().log("DialogExecute");
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
        SetModalInputMode(false);
        ImplGetFrame()->SetModal(false);

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

void Dialog::EndAllDialogs( vcl::Window const * pParent )
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

bool Dialog::AreDialogsOpen()
{
    ImplSVData* pSVData = ImplGetSVData();
    Dialog* pModDialog = pSVData->maWinData.mpLastExecuteDlg;

    return (nullptr != pModDialog);
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
        pFocusControl = ImplGetDlgWindow( 0, GetDlgWindowType::First );
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
        pDev->DrawRect( tools::Rectangle( aPos, aSize ) );
    }

    if (!( GetStyle() & WB_NOBORDER ))
    {
        ScopedVclPtrInstance< ImplBorderWindow > aImplWin( this, WB_BORDER|WB_STDWORK, BorderWindowStyle::Overlap );
        aImplWin->SetText( GetText() );
        aImplWin->setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
        aImplWin->SetDisplayActive( true );
        aImplWin->InitView();

        aImplWin->Draw( pDev, aPos );
    }

    pDev->Pop();
}

void Dialog::queue_resize(StateChangedType eReason)
{
    if (IsInClose())
        return;
    SystemWindow::queue_resize(eReason);
}

bool Dialog::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "border-width")
        set_border_width(rValue.toInt32());
    else
        return SystemWindow::set_property(rKey, rValue);
    return true;
}

FactoryFunction Dialog::GetUITestFactory() const
{
    return DialogUIObject::create;
}

VclBuilderContainer::VclBuilderContainer()
    : m_pUIBuilder(nullptr)
{
}

VclBuilderContainer::~VclBuilderContainer()
{
}

ModelessDialog::ModelessDialog(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, InitFlag eFlag)
    : Dialog(pParent, rID, rUIXMLDescription, WindowType::MODELESSDIALOG, eFlag)
{
}

ModalDialog::ModalDialog( vcl::Window* pParent, WinBits nStyle ) :
    Dialog( WindowType::MODALDIALOG )
{
    ImplInit( pParent, nStyle );
}

ModalDialog::ModalDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription ) :
    Dialog(pParent, rID, rUIXMLDescription, WindowType::MODALDIALOG)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
