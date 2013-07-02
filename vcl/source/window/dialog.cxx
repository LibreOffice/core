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

#include <com/sun/star/beans/XPropertySet.hpp>
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
#include <vcl/decoview.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/unowrap.hxx>
#include <iostream>

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

// =======================================================================

#ifndef ANDROID

static rtl::OString ImplGetDialogText( Dialog* pDialog )
{
    rtl::OStringBuffer aErrorStr(rtl::OUStringToOString(
        pDialog->GetText(), RTL_TEXTENCODING_UTF8));
    if ( (pDialog->GetType() == WINDOW_MESSBOX) ||
         (pDialog->GetType() == WINDOW_INFOBOX) ||
         (pDialog->GetType() == WINDOW_WARNINGBOX) ||
         (pDialog->GetType() == WINDOW_ERRORBOX) ||
         (pDialog->GetType() == WINDOW_QUERYBOX) )
    {
        aErrorStr.append(", ");
        aErrorStr.append(rtl::OUStringToOString(
            ((MessBox*)pDialog)->GetMessText(), RTL_TEXTENCODING_UTF8));
    }
    return aErrorStr.makeStringAndClear();
}

#endif

// =======================================================================

static sal_Bool ImplIsMnemonicCtrl( Window* pWindow )
{
    if( ! pWindow->GetSettings().GetStyleSettings().GetAutoMnemonic() )
        return sal_False;

    if ( (pWindow->GetType() == WINDOW_RADIOBUTTON) ||
         (pWindow->GetType() == WINDOW_CHECKBOX) ||
         (pWindow->GetType() == WINDOW_TRISTATEBOX) ||
         (pWindow->GetType() == WINDOW_PUSHBUTTON) )
        return sal_True;

    if ( pWindow->GetType() == WINDOW_FIXEDTEXT )
    {
        if ( pWindow->GetStyle() & (WB_INFO | WB_NOLABEL) )
            return sal_False;
        Window* pNextWindow = pWindow->GetWindow( WINDOW_NEXT );
        if ( !pNextWindow )
            return sal_False;
        pNextWindow = pNextWindow->GetWindow( WINDOW_CLIENT );
        if ( !(pNextWindow->GetStyle() & WB_TABSTOP) ||
             (pNextWindow->GetType() == WINDOW_FIXEDTEXT) ||
             (pNextWindow->GetType() == WINDOW_GROUPBOX) ||
             (pNextWindow->GetType() == WINDOW_RADIOBUTTON) ||
             (pNextWindow->GetType() == WINDOW_CHECKBOX) ||
             (pNextWindow->GetType() == WINDOW_TRISTATEBOX) ||
             (pNextWindow->GetType() == WINDOW_PUSHBUTTON) )
            return sal_False;

        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

// Called by native error dialog popup implementations
void ImplHideSplash()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
            pSVData->mpIntroWindow->Hide();
}

//Get next window after pChild of a pTopLevel window as
//if any intermediate layout widgets didn't exist
Window * nextLogicalChildOfParent(Window *pTopLevel, Window *pChild)
{
    Window *pLastChild = pChild;

    if (isContainerWindow(*pChild))
        pChild = pChild->GetWindow(WINDOW_FIRSTCHILD);
    else
        pChild = pChild->GetWindow(WINDOW_NEXT);

    while (!pChild)
    {
        Window *pParent = pLastChild->GetParent();
        if (!pParent)
            return NULL;
        if (pParent == pTopLevel)
            return NULL;
        pLastChild = pParent;
        pChild = pParent->GetWindow(WINDOW_NEXT);
    }

    if (pChild && isContainerWindow(*pChild))
        pChild = nextLogicalChildOfParent(pTopLevel, pChild);

    return pChild;
}

Window * prevLogicalChildOfParent(Window *pTopLevel, Window *pChild)
{
    Window *pLastChild = pChild;

    if (isContainerWindow(*pChild))
        pChild = pChild->GetWindow(WINDOW_LASTCHILD);
    else
        pChild = pChild->GetWindow(WINDOW_PREV);

    while (!pChild)
    {
        Window *pParent = pLastChild->GetParent();
        if (!pParent)
            return NULL;
        if (pParent == pTopLevel)
            return NULL;
        pLastChild = pParent;
        pChild = pParent->GetWindow(WINDOW_PREV);
    }

    if (pChild && isContainerWindow(*pChild))
        pChild = prevLogicalChildOfParent(pTopLevel, pChild);

    return pChild;
}

//Get first window of a pTopLevel window as
//if any intermediate layout widgets didn't exist
Window * firstLogicalChildOfParent(Window *pTopLevel)
{
    Window *pChild = pTopLevel->GetWindow(WINDOW_FIRSTCHILD);
    if (pChild && isContainerWindow(*pChild))
        pChild = nextLogicalChildOfParent(pTopLevel, pChild);
    return pChild;
}

// -----------------------------------------------------------------------

void ImplWindowAutoMnemonic( Window* pWindow )
{
    MnemonicGenerator   aMnemonicGenerator;
    Window*                 pGetChild;
    Window*                 pChild;

    // Die schon vergebenen Mnemonics registieren
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
        pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
    }

    // Bei TabPages auch noch die Controls vom Dialog beruecksichtigen
    if ( pWindow->GetType() == WINDOW_TABPAGE )
    {
        Window* pParent = pWindow->GetParent();
        if ( pParent->GetType() == WINDOW_TABCONTROL )
            pParent = pParent->GetParent();

        if ( (pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
        {
            pGetChild = pParent->GetWindow( WINDOW_FIRSTCHILD );
            while ( pGetChild )
            {
                pChild = pGetChild->ImplGetWindow();
                aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
                pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
            }
        }
    }

    // Die Mnemonics an die Controls vergeben, die noch keinen haben
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        if ( ImplIsMnemonicCtrl( pChild ) )
        {
            XubString aText = pChild->GetText();
            if ( aMnemonicGenerator.CreateMnemonic( aText ) )
                pChild->SetText( aText );
        }

        pGetChild = nextLogicalChildOfParent(pWindow, pGetChild);
    }
}

static VclButtonBox* getActionArea(Dialog *pDialog)
{
    VclButtonBox *pButtonBox = NULL;
    if (pDialog->isLayoutEnabled())
    {
        Window *pBox = pDialog->GetWindow(WINDOW_FIRSTCHILD);
        Window *pChild = pBox->GetWindow(WINDOW_LASTCHILD);
        while (pChild)
        {
            pButtonBox = dynamic_cast<VclButtonBox*>(pChild);
            if (pButtonBox)
                break;
            pChild = pChild->GetWindow(WINDOW_PREV);
        }
    }
    return pButtonBox;
}

static Window* getActionAreaButtonList(Dialog *pDialog)
{
    VclButtonBox* pButtonBox = getActionArea(pDialog);
    if (pButtonBox)
        return pButtonBox->GetWindow(WINDOW_FIRSTCHILD);
    return pDialog->GetWindow(WINDOW_FIRSTCHILD);
}

// =======================================================================

static PushButton* ImplGetDefaultButton( Dialog* pDialog )
{
    Window* pChild = getActionAreaButtonList(pDialog);
    while ( pChild )
    {
        if ( pChild->ImplIsPushButton() )
        {
            PushButton* pPushButton = (PushButton*)pChild;
            if ( pPushButton->ImplIsDefButton() )
                return pPushButton;
        }

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetOKButton( Dialog* pDialog )
{
    Window* pChild = getActionAreaButtonList(pDialog);
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_OKBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetCancelButton( Dialog* pDialog )
{
    Window* pChild = getActionAreaButtonList(pDialog);

    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_CANCELBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static void ImplMouseAutoPos( Dialog* pDialog )
{
    sal_uLong nMouseOptions = pDialog->GetSettings().GetMouseSettings().GetOptions();
    if ( nMouseOptions & MOUSE_OPTION_AUTOCENTERPOS )
    {
        Size aSize = pDialog->GetOutputSizePixel();
        pDialog->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
    else if ( nMouseOptions & MOUSE_OPTION_AUTODEFBTNPOS )
    {
        Window* pWindow = ImplGetDefaultButton( pDialog );
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

// =======================================================================

struct DialogImpl
{
    long    mnResult;
    bool    mbStartedModal;
    Link    maEndDialogHdl;

    DialogImpl() : mnResult( -1 ), mbStartedModal( false ) {}
};

// =======================================================================

void Dialog::ImplInitDialogData()
{
    mpWindowImpl->mbDialog  = sal_True;
    mpDialogParent          = NULL;
    mpPrevExecuteDlg        = NULL;
    mbInExecute             = sal_False;
    mbOldSaveBack           = sal_False;
    mbInClose               = sal_False;
    mbModalMode             = sal_False;
    mnMousePositioned       = 0;
    mpDialogImpl            = new DialogImpl;

    //To-Do, reuse maResizeTimer
    maLayoutTimer.SetTimeout(50);
    maLayoutTimer.SetTimeoutHdl( LINK( this, Dialog, ImplHandleLayoutTimerHdl ) );
}

// -----------------------------------------------------------------------

void Dialog::ImplInit( Window* pParent, WinBits nStyle )
{
    sal_uInt16 nSysWinMode = Application::GetSystemWindowMode();

    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;
    nStyle |= WB_ROLLABLE;

    // Now, all Dialogs are per default system windows !!!
    nStyle |= WB_SYSTEMWINDOW;


    // parent is NULL: get the default Dialog parent
    if ( !pParent )
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
                // Nur wenn er sichtbar und enabled ist
                if ( pParent->ImplGetFirstOverlapWindow()->IsWindowOrChild( pExeDlg, sal_True ) &&
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
    // DIALOG_NO_PARENT: explicitly don't have a parent for this Dialog
    else if( pParent == DIALOG_NO_PARENT )
        pParent = NULL;


    if ( !pParent || (nStyle & WB_SYSTEMWINDOW) ||
         (pParent->mpWindowImpl->mpFrameData->mbNeedSysWindow && !(nSysWinMode & SYSTEMWINDOW_MODE_NOAUTOMODE)) ||
         (nSysWinMode & SYSTEMWINDOW_MODE_DIALOG) )
    {
        // create window with a small border ?
        if ( (nStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE)) == WB_BORDER )
        {
            ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_FRAME );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
            mpWindowImpl->mpBorderWindow  = pBorderWin;
            mpWindowImpl->mpRealParent    = pParent;
        }
        else
        {
            mpWindowImpl->mbFrame         = sal_True;
            mpWindowImpl->mbOverlapWin    = sal_True;
            SystemWindow::ImplInit( pParent, (nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE)) | WB_CLOSEABLE, NULL );
            // Now set all style bits
            mpWindowImpl->mnStyle = nStyle;
        }
    }
    else
    {
        ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_OVERLAP | BORDERWINDOW_STYLE_BORDER );
        SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
        pBorderWin->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        mpWindowImpl->mpRealParent    = pParent;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void Dialog::ImplInitSettings()
{
    // user override
    if ( IsControlBackground() )
        SetBackground( GetControlBackground() );
    // NWF background
    else if( IsNativeControlSupported( CTRL_WINDOW_BACKGROUND, PART_BACKGROUND_DIALOG ) )
    {
        mpWindowImpl->mnNativeBackground = PART_BACKGROUND_DIALOG;
        EnableChildTransparentMode( sal_True );
    }
    // fallback to settings color
    else
        SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );
}

// -----------------------------------------------------------------------

Dialog::Dialog( WindowType nType )
    : SystemWindow( nType )
    , mbIsDefferedInit(false)
{
    ImplInitDialogData();
}

#define BASEPATH_SHARE_LAYER rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UIConfig"))
#define RELPATH_SHARE_LAYER rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice.cfg"))
#define SERVICENAME_PATHSETTINGS rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.PathSettings"))

rtl::OUString VclBuilderContainer::getUIRootDir()
{
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

//we can't change sizeable after the fact, so need to defer until we know and then
//do the init. Find the real parent stashed in mpDialogParent.
void Dialog::doDeferredInit(bool bResizable)
{
    WinBits nBits = WB_3DLOOK|WB_CLOSEABLE|WB_MOVEABLE;
    if (bResizable)
        nBits |= WB_SIZEABLE;
    Window *pParent = mpDialogParent;
    mpDialogParent = NULL;
    ImplInit(pParent, nBits);
    mbIsDefferedInit = false;
}

Dialog::Dialog(Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription)
    : SystemWindow( WINDOW_DIALOG )
    , mbIsDefferedInit(true)
{
    ImplInitDialogData();
    mpDialogParent = pParent; //will be unset in doDeferredInit
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID);
}

Dialog::Dialog(Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription, WindowType nType)
    : SystemWindow( nType )
    , mbIsDefferedInit(true)
{
    ImplInitDialogData();
    mpDialogParent = pParent; //will be unset in doDeferredInit
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID);
}


// -----------------------------------------------------------------------

Dialog::Dialog( Window* pParent, WinBits nStyle )
    : SystemWindow( WINDOW_DIALOG )
    , mbIsDefferedInit(false)
{
    ImplInitDialogData();
    ImplInit( pParent, nStyle );
}

VclBuilder* VclBuilderContainer::overrideResourceWithUIXML(Window *pWindow, const ResId& rResId)
{
    sal_Int32 nUIid = static_cast<sal_Int32>(rResId.GetId());

    rtl::OUString sRoot = getUIRootDir();
    rtl::OUString sPath = rtl::OUStringBuffer(
        rResId.GetResMgr()->getPrefixName()).
        append("/ui/").
        append(nUIid).
        appendAscii(".ui").
        makeStringAndClear();

    osl::File aUIFile(sRoot + sPath);
    osl::File::RC error = aUIFile.open(osl_File_OpenFlag_Read);
    //good, use the preferred GtkBuilder xml
    if (error == osl::File::E_None)
        return new VclBuilder(pWindow, sRoot, sPath, rtl::OString::valueOf(nUIid));
    return NULL;
}

WinBits Dialog::init(Window *pParent, const ResId& rResId)
{
    WinBits nStyle = ImplInitRes( rResId );

    ImplInit( pParent, nStyle );

    m_pUIBuilder = overrideResourceWithUIXML(this, rResId);

    if (m_pUIBuilder)
        loadAndSetJustHelpID(rResId);
    else
    {
        //fallback to using the binary resource file
        ImplLoadRes( rResId );
    }

    return nStyle;
}

// -----------------------------------------------------------------------

Dialog::~Dialog()
{
    maLayoutTimer.Stop();
    delete mpDialogImpl;
    mpDialogImpl = NULL;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(Dialog, ImplAsyncCloseHdl)
{
    Close();
    return 0;
}

// -----------------------------------------------------------------------

long Dialog::Notify( NotifyEvent& rNEvt )
{
    // Zuerst Basisklasse rufen wegen TabSteuerung
    long nRet = SystemWindow::Notify( rNEvt );
    if ( !nRet )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            KeyCode         aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16          nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) &&
                 ((GetStyle() & WB_CLOSEABLE) || ImplGetCancelButton( this ) || ImplGetOKButton( this )) )
            {
                // #i89505# for the benefit of slightly mentally challenged implementations
                // like e.g. SfxModelessDialog which destroy themselves inside Close()
                // post this Close asynchronous so we can leave our key handler before
                // we get destroyed
                PostUserEvent( LINK( this, Dialog, ImplAsyncCloseHdl ), this );
                return sal_True;
            }
        }
        else if ( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            // make sure the dialog is still modal
            // changing focus between application frames may
            // have re-enabled input for our parent
            if( mbInExecute && mbModalMode )
            {
                // do not change modal counter (pSVData->maAppData.mnModalDialog)
                SetModalInputMode( sal_False );
                SetModalInputMode( sal_True );

                // #93022# def-button might have changed after show
                if( !mnMousePositioned )
                {
                    mnMousePositioned = 1;
                    ImplMouseAutoPos( this );
                }

            }
        }
    }

    return nRet;
}

void Dialog::setInitialLayoutSize()
{
    maLayoutTimer.Stop();

    //resize dialog to fit requisition on initial show
    VclBox *pBox = static_cast<VclBox*>(GetWindow(WINDOW_FIRSTCHILD));

    const DialogStyle& rDialogStyle =
        GetSettings().GetStyleSettings().GetDialogStyle();
    pBox->set_border_width(rDialogStyle.content_area_border);
    pBox->set_spacing(rDialogStyle.content_area_spacing);

    VclButtonBox *pActionArea = getActionArea(this);
    if (pActionArea)
    {
        pActionArea->set_border_width(rDialogStyle.action_area_border);
        pActionArea->set_spacing(rDialogStyle.button_spacing);
    }

    Size aSize = get_preferred_size();

    Size aMax = GetOptimalSize(WINDOWSIZE_MAXIMUM);
    aSize.Width() = std::min(aMax.Width(), aSize.Width());
    aSize.Height() = std::min(aMax.Height(), aSize.Height());

    SetMinOutputSizePixel(aSize);
    SetSizePixel(aSize);
    setPosSizeOnContainee(aSize, *pBox);
}

// -----------------------------------------------------------------------

void Dialog::StateChanged( StateChangedType nType )
{
    SystemWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( GetSettings().GetStyleSettings().GetAutoMnemonic() )
            ImplWindowAutoMnemonic( this );

        if (isLayoutEnabled())
            setInitialLayoutSize();

        if ( !HasChildPathFocus() || HasFocus() )
            GrabFocusToFirstControl();
        if ( !(GetStyle() & WB_CLOSEABLE) )
        {
            if ( ImplGetCancelButton( this ) || ImplGetOKButton( this ) )
            {
                if ( ImplGetBorderWindow() )
                    ((ImplBorderWindow*)ImplGetBorderWindow())->SetCloser();
            }
        }

        ImplMouseAutoPos( this );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Dialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SystemWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

sal_Bool Dialog::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDead() )
        return sal_False;
    ImplRemoveDel( &aDelData );

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() && !IsInExecute() )
        return sal_False;

    mbInClose = sal_True;

    if ( !(GetStyle() & WB_CLOSEABLE) )
    {
        sal_Bool bRet = sal_True;
        ImplAddDel( &aDelData );
        PushButton* pButton = ImplGetCancelButton( this );
        if ( pButton )
            pButton->Click();
        else
        {
            pButton = ImplGetOKButton( this );
            if ( pButton )
                pButton->Click();
            else
                bRet = sal_False;
        }
        if ( aDelData.IsDead() )
            return sal_True;
        ImplRemoveDel( &aDelData );
        return bRet;
    }

    if ( IsInExecute() )
    {
        EndDialog( sal_False );
        mbInClose = sal_False;
        return sal_True;
    }
    else
    {
        mbInClose = sal_False;
        return SystemWindow::Close();
    }
}

// -----------------------------------------------------------------------

sal_Bool Dialog::ImplStartExecuteModal()
{
#ifdef ANDROID
    // If a non-NativeActivity app, we shouldn't be showing any dialogs
    fprintf(stderr, "%s: Should not do anything, returning false\n", __FUNCTION__);
    return sal_False;
#else

    if ( mbInExecute )
    {
#ifdef DBG_UTIL
        rtl::OStringBuffer aErrorStr;
        aErrorStr.append(RTL_CONSTASCII_STRINGPARAM("Dialog::StartExecuteModal() is called in Dialog::StartExecuteModal(): "));
        aErrorStr.append(ImplGetDialogText(this));
        OSL_FAIL(aErrorStr.getStr());
#endif
        return sal_False;
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
        return sal_False;
    default:
        assert(false); // this cannot happen
        // fall through
    case Application::DIALOG_CANCEL_FATAL:
        throw Application::DialogCancelledException(
            ImplGetDialogText(this).getStr());
    }

#ifdef DBG_UTIL
    Window* pParent = GetParent();
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

    // Dialoge, die sich in Execute befinden, miteinander verketten
    mpPrevExecuteDlg = pSVData->maWinData.mpLastExecuteDlg;
    pSVData->maWinData.mpLastExecuteDlg = this;

    // Capture beenden, damit der Dialog bedient werden kann
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();
    EnableInput( sal_True, sal_True );

    if ( GetParent() )
    {
        NotifyEvent aNEvt( EVENT_EXECUTEDIALOG, this );
        GetParent()->Notify( aNEvt );
    }
    mbInExecute = sal_True;
    SetModalInputMode( sal_True );
    mbOldSaveBack = IsSaveBackgroundEnabled();
    EnableSaveBackground();

    // FIXME: no layouting, workaround some clipping issues
    ImplAdjustNWFSizes();

    Show();

    pSVData->maAppData.mnModalMode++;
    return sal_True;
#endif
}

// -----------------------------------------------------------------------

void Dialog::ImplEndExecuteModal()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode--;
}

// -----------------------------------------------------------------------

short Dialog::Execute()
{
    if ( !ImplStartExecuteModal() )
        return 0;

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

#ifdef DBG_UTIL
    ImplDelData aParentDelData;
    Window* pDialogParent = mpDialogParent;
    if( pDialogParent )
        pDialogParent->ImplAddDel( &aParentDelData );
#endif

    // Yield util EndDialog is called or dialog gets destroyed
    // (the latter should not happen, but better safe than sorry
    while ( !aDelData.IsDead() && mbInExecute )
        Application::Yield();

    ImplEndExecuteModal();

#ifdef DBG_UTIL
    if( pDialogParent  )
    {
        if( ! aParentDelData.IsDead() )
            pDialogParent->ImplRemoveDel( &aParentDelData );
        else
            OSL_FAIL( "Dialog::Execute() - Parent of dialog destroyed in Execute()" );
    }
#endif
    if ( !aDelData.IsDead() )
        ImplRemoveDel( &aDelData );
#ifdef DBG_UTIL
    else
    {
        OSL_FAIL( "Dialog::Execute() - Dialog destroyed in Execute()" );
    }
#endif

    long nRet = mpDialogImpl->mnResult;
    mpDialogImpl->mnResult = -1;
    return (short)nRet;
}

// -----------------------------------------------------------------------

// virtual
void Dialog::StartExecuteModal( const Link& rEndDialogHdl )
{
    if ( !ImplStartExecuteModal() )
        return;

    mpDialogImpl->maEndDialogHdl = rEndDialogHdl;
    mpDialogImpl->mbStartedModal = true;
}

// -----------------------------------------------------------------------

void Dialog::EndDialog( long nResult )
{
    if ( mbInExecute )
    {
        SetModalInputMode( sal_False );

        // Dialog aus der Kette der Dialoge die in Execute stehen entfernen
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
            Window* pFrameParent = ImplGetFrameWindow()->ImplGetParent();
            Window* pPrevFrameParent = mpPrevExecuteDlg->ImplGetFrameWindow()->ImplGetParent();
            if( ( !pFrameParent && !pPrevFrameParent ) ||
                ( pFrameParent && pPrevFrameParent && pFrameParent->ImplGetFrame() == pPrevFrameParent->ImplGetFrame() )
                )
            {
                mpPrevExecuteDlg->GrabFocus();
            }
        }
        mpPrevExecuteDlg = NULL;

        Hide();
        EnableSaveBackground( mbOldSaveBack );
        if ( GetParent() )
        {
            NotifyEvent aNEvt( EVENT_ENDEXECUTEDIALOG, this );
            GetParent()->Notify( aNEvt );
        }

        mpDialogImpl->mnResult = nResult;

        if ( mpDialogImpl->mbStartedModal )
        {
            ImplEndExecuteModal();
            mpDialogImpl->maEndDialogHdl.Call( this );

            mpDialogImpl->maEndDialogHdl = Link();
            mpDialogImpl->mbStartedModal = false;
            mpDialogImpl->mnResult = -1;
        }
        mbInExecute = sal_False;
    }
}

// -----------------------------------------------------------------------

long Dialog::GetResult() const
{
    return mpDialogImpl->mnResult;
}

// -----------------------------------------------------------------------

void Dialog::EndAllDialogs( Window* pParent )
{
   ImplSVData*  pSVData = ImplGetSVData();
   Dialog*      pTempModDialog;
   Dialog*      pModDialog = pSVData->maWinData.mpLastExecuteDlg;
   while ( pModDialog )
   {
     pTempModDialog = pModDialog->mpPrevExecuteDlg;
     if( !pParent || ( pParent && pParent->IsWindowOrChild( pModDialog, sal_True ) ) )
     {
        pModDialog->EndDialog( sal_False );
        pModDialog->PostUserEvent( Link() );
     }
     pModDialog = pTempModDialog;
   }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( sal_Bool bModal )
{
    if ( bModal == mbModalMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();
    mbModalMode = bModal;
    if ( bModal )
    {
        pSVData->maAppData.mnModalDialog++;

        // Diable the prev Modal Dialog, because our dialog must close at first,
        // before the other dialog can be closed (because the other dialog
        // is on stack since our dialog returns)
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, sal_True ) )
            mpPrevExecuteDlg->EnableInput( sal_False, sal_True, sal_True, this );

        // determine next overlap dialog parent
        Window* pParent = GetParent();
        if ( pParent )
        {
            // #103716# dialogs should always be modal to the whole frame window
            // #115933# disable the whole frame hierarchie, useful if our parent
            // is a modeless dialog
            mpDialogParent = pParent->mpWindowImpl->mpFrameWindow;
            mpDialogParent->ImplIncModalCount();
        }

    }
    else
    {
        pSVData->maAppData.mnModalDialog--;

        if ( mpDialogParent )
        {
            // #115933# re-enable the whole frame hierarchie again (see above)
            // note that code in getfocus assures that we do not accidentally enable
            // windows that were disabled before
            mpDialogParent->ImplDecModalCount();
        }

        // Enable the prev Modal Dialog
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, sal_True ) )
        {
            mpPrevExecuteDlg->EnableInput( sal_True, sal_True, sal_True, this );
            // ensure continued modality of prev dialog
            // do not change modality counter
            mpPrevExecuteDlg->SetModalInputMode( sal_False );
            mpPrevExecuteDlg->SetModalInputMode( sal_True );
        }
    }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( sal_Bool bModal, sal_Bool bSubModalDialogs )
{
    if ( bSubModalDialogs )
    {
        Window* pOverlap = ImplGetFirstOverlapWindow();
        pOverlap = pOverlap->mpWindowImpl->mpFirstOverlap;
        while ( pOverlap )
        {
            if ( pOverlap->IsDialog() )
                ((Dialog*)pOverlap)->SetModalInputMode( bModal, sal_True );
            pOverlap = pOverlap->mpWindowImpl->mpNext;
        }
    }

    SetModalInputMode( bModal );
}

// -----------------------------------------------------------------------

void Dialog::GrabFocusToFirstControl()
{
    Window* pFocusControl;

    // Wenn Dialog den Focus hat, versuchen wr trotzdem
    // ein Focus-Control zu finden
    if ( HasFocus() )
        pFocusControl = NULL;
    else
    {
        // Wenn schon ein Child-Fenster mal den Focus hatte,
        // dann dieses bevorzugen
        pFocusControl = ImplGetFirstOverlapWindow()->mpWindowImpl->mpLastFocusWindow;
        // Control aus der Dialog-Steuerung suchen
        if ( pFocusControl )
            pFocusControl = ImplFindDlgCtrlWindow( pFocusControl );
    }
    // Kein Control hatte vorher den Focus, oder das Control
    // befindet sich nicht in der Tab-Steuerung, dann das erste
    // Control in der TabSteuerung den Focus geben
    if ( !pFocusControl ||
         !(pFocusControl->GetStyle() & WB_TABSTOP) ||
         !isVisibleInLayout(pFocusControl) ||
         !isEnabledInLayout(pFocusControl) || !pFocusControl->IsInputEnabled() )
    {
        sal_uInt16 n = 0;
        pFocusControl = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
    }
    if ( pFocusControl )
        pFocusControl->ImplControlFocus( GETFOCUS_INIT );
}

void Dialog::GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder, sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    ImplBorderWindow aImplWin( (Window*)this, WB_BORDER|WB_STDWORK, BORDERWINDOW_STYLE_OVERLAP );
//  aImplWin.SetText( GetText() );
//  aImplWin.SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
//  aImplWin.SetDisplayActive( sal_True );
//  aImplWin.InitView();
    aImplWin.GetBorder( rLeftBorder, rTopBorder, rRightBorder, rBottomBorder );
}


void Dialog::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong )
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
        ImplBorderWindow aImplWin( this, WB_BORDER|WB_STDWORK, BORDERWINDOW_STYLE_OVERLAP );
        aImplWin.SetText( GetText() );
        aImplWin.setPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
        aImplWin.SetDisplayActive( sal_True );
        aImplWin.InitView();

        aImplWin.Draw( Rectangle( aPos, aSize ), pDev, aPos );
    }

    pDev->Pop();
}

bool Dialog::isLayoutEnabled() const
{
    //pre dtor called, and single child is a container => we're layout enabled
    const Window *pChild = mpDialogImpl ? GetWindow(WINDOW_FIRSTCHILD) : NULL;
    return pChild && isContainerWindow(*pChild) && !pChild->GetWindow(WINDOW_NEXT);
}

Size Dialog::GetOptimalSize(WindowSizeType eType) const
{
    if (eType == WINDOWSIZE_MAXIMUM || !isLayoutEnabled())
        return SystemWindow::GetOptimalSize(eType);

    Size aSize = VclContainer::getLayoutRequisition(*GetWindow(WINDOW_FIRSTCHILD));

    sal_Int32 nBorderWidth = get_border_width();

    aSize.Height() += mpWindowImpl->mnLeftBorder + mpWindowImpl->mnRightBorder
        + 2*nBorderWidth;
    aSize.Width() += mpWindowImpl->mnTopBorder + mpWindowImpl->mnBottomBorder
        + 2*nBorderWidth;

    return Window::CalcWindowSize(aSize);
}

void Dialog::setPosSizeOnContainee(Size aSize, VclContainer &rBox)
{
    sal_Int32 nBorderWidth = get_border_width();

    aSize.Width() -= mpWindowImpl->mnLeftBorder + mpWindowImpl->mnRightBorder
        + 2 * nBorderWidth;
    aSize.Height() -= mpWindowImpl->mnTopBorder + mpWindowImpl->mnBottomBorder
        + 2 * nBorderWidth;

    Point aPos(mpWindowImpl->mnLeftBorder + nBorderWidth,
        mpWindowImpl->mnTopBorder + nBorderWidth);

    VclContainer::setLayoutAllocation(rBox, aPos, aSize);
}

IMPL_LINK( Dialog, ImplHandleLayoutTimerHdl, void*, EMPTYARG )
{
    if (!isLayoutEnabled())
    {
        SAL_WARN("vcl.layout", "Dialog has become non-layout because extra children have been added directly to it.");
        return 0;
    }

    VclBox *pBox = static_cast<VclBox*>(GetWindow(WINDOW_FIRSTCHILD));
    assert(pBox);
    setPosSizeOnContainee(GetSizePixel(), *pBox);
    return 0;
}

void Dialog::queue_layout()
{
    if (hasPendingLayout())
        return;
    if (IsInClose())
        return;
    if (!isLayoutEnabled())
        return;
    maLayoutTimer.Start();
}

void Dialog::Resize()
{
    queue_layout();
}

bool Dialog::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("border-width")))
        set_border_width(rValue.toInt32());
    else
        return SystemWindow::set_property(rKey, rValue);
    return true;
}

VclBuilderContainer::VclBuilderContainer()
    : m_pUIBuilder(NULL)
{
}

VclBuilderContainer::~VclBuilderContainer()
{
    delete m_pUIBuilder;
}

bool VclBuilderContainer::replace_buildable(Window *pParent, const ResId& rResId, Window &rReplacement)
{
    if (!pParent)
        return false;

    VclBuilderContainer *pBuilderContainer = dynamic_cast<VclBuilderContainer*>(pParent);
    if (!pBuilderContainer)
        return false;

    VclBuilder *pUIBuilder = pBuilderContainer->m_pUIBuilder;
    if (!pUIBuilder)
        return false;

    sal_Int32 nID = rResId.GetId();

    bool bFound = pUIBuilder->replace(rtl::OString::valueOf(nID), rReplacement);
    if (bFound)
    {
        rReplacement.loadAndSetJustHelpID(rResId);
    }
    else
    {
        SAL_WARN("vcl.layout", "widget " << nID << " " << &rReplacement << " not found, hiding");
        //move "missing" elements into the action area (just to have
        //a known container as an owner) and hide it
        Window* pArbitraryParent;
        if (pParent->IsDialog())
        {
            Dialog *pDialog = static_cast<Dialog*>(pParent);
            pArbitraryParent = getActionArea(pDialog);
        }
        else
            pArbitraryParent = pParent->GetWindow(WINDOW_FIRSTCHILD);
        rReplacement.ImplInit(pArbitraryParent, 0, NULL);
        rReplacement.Hide();
    }

    return true;
}

// -----------------------------------------------------------------------

ModelessDialog::ModelessDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODELESSDIALOG )
{
    rResId.SetRT( RSC_MODELESSDIALOG );

    WinBits nStyle = init( pParent, rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

ModelessDialog::ModelessDialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription ) :
    Dialog(pParent, rID, rUIXMLDescription, WINDOW_MODELESSDIALOG)
{
}

// =======================================================================

ModalDialog::ModalDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODALDIALOG )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ModalDialog::ModalDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODALDIALOG )
{
    rResId.SetRT( RSC_MODALDIALOG );
    init( pParent, rResId );
}

ModalDialog::ModalDialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription ) :
    Dialog(pParent, rID, rUIXMLDescription, WINDOW_MODALDIALOG)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
