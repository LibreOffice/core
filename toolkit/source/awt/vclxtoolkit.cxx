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

#include <stdio.h>
#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#endif
#include <config_features.h>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/MessageBoxButtons.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/FocusEvent.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XToolkitExperimental.hpp>
#include <com/sun/star/awt/XToolkitRobot.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/conditn.hxx>
#include <osl/module.h>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>
#include <rtl/process.h>
#include <sal/log.hxx>
#include <tools/link.hxx>
#include <tools/fract.hxx>
#include <tools/wintypes.hxx>

#ifdef MACOSX
#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#endif

#include <vcl/sysdata.hxx>
#include <vcl/textrectinfo.hxx>
#include <vcl/vclmedit.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxtabpagecontainer.hxx>

#include <toolkit/awt/animatedimagespeer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/edit.hxx>
#include <vcl/event.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/toolkit/group.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/toolkit/morebtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/spin.hxx>
#include <vcl/split.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/throbber.hxx>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLWrapper.hxx>
#endif
#include <toolkit/awt/vclxspinbutton.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>

#include <helper/msgbox.hxx>
#include <helper/scrollabledialog.hxx>
#include <helper/unowrapper.hxx>

#if defined(_WIN32)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_WIN32
#elif defined(MACOSX)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_MAC
#elif defined(UNX)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_XWINDOW
#endif

void MessBox::ImplInitButtons()
{
    ButtonDialogFlags nOKFlags = ButtonDialogFlags::OK;
    ButtonDialogFlags nCancelFlags = ButtonDialogFlags::Cancel;
    ButtonDialogFlags nRetryFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nYesFlags = ButtonDialogFlags::NONE;
    ButtonDialogFlags nNoFlags = ButtonDialogFlags::NONE;

    if ( mnMessBoxStyle & MessBoxStyle::OkCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultOk
            nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::YesNo )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultYes )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultNo
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        nNoFlags |= ButtonDialogFlags::Cancel;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::YesNoCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultYes )
            nYesFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultNo )
            nNoFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Yes, RET_YES, nYesFlags );
        AddButton( StandardButtonType::No, RET_NO, nNoFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::RetryCancel )
    {
        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nCancelFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else // MessBoxStyle::DefaultRetry
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Cancel, RET_CANCEL, nCancelFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::AbortRetryIgnore )
    {
        ButtonDialogFlags nAbortFlags = ButtonDialogFlags::NONE;
        ButtonDialogFlags nIgnoreFlags = ButtonDialogFlags::NONE;

        if ( mnMessBoxStyle & MessBoxStyle::DefaultCancel )
            nAbortFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultRetry )
            nRetryFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;
        else if ( mnMessBoxStyle & MessBoxStyle::DefaultIgnore )
            nIgnoreFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::Abort, RET_CANCEL, nAbortFlags );
        AddButton( StandardButtonType::Retry, RET_RETRY, nRetryFlags );
        AddButton( StandardButtonType::Ignore, RET_IGNORE, nIgnoreFlags );
    }
    else if ( mnMessBoxStyle & MessBoxStyle::Ok )
    {
        nOKFlags |= ButtonDialogFlags::Default | ButtonDialogFlags::Focus;

        AddButton( StandardButtonType::OK, RET_OK, nOKFlags );
    }
}

MessBox::MessBox(vcl::Window* pParent, MessBoxStyle nMessBoxStyle, WinBits nWinBits,
                 const OUString& rTitle, const OUString& rMessage) :
    ButtonDialog( WindowType::MESSBOX ),
    mbHelpBtn( false ),
    mnMessBoxStyle( nMessBoxStyle ),
    maMessText( rMessage )
{
    ImplLOKNotifier(pParent);
    ImplInit(pParent, nWinBits | WB_MOVEABLE | WB_HORZ | WB_CENTER);
    ImplInitButtons();

    if ( !rTitle.isEmpty() )
        SetText( rTitle );
}

MessBox::~MessBox()
{
    disposeOnce();
}

void MessBox::dispose()
{
    mpVCLMultiLineEdit.disposeAndClear();
    mpFixedImage.disposeAndClear();
    ButtonDialog::dispose();
}

void MessBox::ImplPosControls()
{
    if ( !GetHelpId().isEmpty() )
    {
        if ( !mbHelpBtn )
        {
            AddButton( StandardButtonType::Help, RET_HELP, ButtonDialogFlags::Help, 3 );
            mbHelpBtn = true;
        }
    }
    else
    {
        if ( mbHelpBtn )
        {
            RemoveButton( RET_HELP );
            mbHelpBtn = false;
        }
    }

    TextRectInfo    aTextInfo;
    tools::Rectangle       aRect( 0, 0, 30000, 30000 );
    tools::Rectangle       aFormatRect;
    Point           aTextPos( IMPL_DIALOG_OFFSET, IMPL_DIALOG_OFFSET+IMPL_MSGBOX_OFFSET_EXTRA_Y );
    Size            aImageSize;
    Size            aPageSize;
    Size            aMEditSize;
    long            nTitleWidth;
    long            nButtonSize = ImplGetButtonSize();
    long            nMaxWidth = GetDesktopRectPixel().GetWidth()-8;
    long            nMaxLineWidth;
    long            nWidth;
    WinBits         nWinStyle = WB_LEFT | WB_NOLABEL;
    DrawTextFlags   nTextStyle = DrawTextFlags::MultiLine | DrawTextFlags::Top | DrawTextFlags::Left;

    mpVCLMultiLineEdit.disposeAndClear();
    mpFixedImage.disposeAndClear();

    // Clean up message text with tabs
    OUString aMessText(maMessText.replaceAll("\t", "    "));

    //If window too small, we make dialog box be wider
    nMaxWidth = 630 * GetDPIScaleFactor();

    // MessagBox should be at least as wide as to see the title
    // Extra-Width for Close button, because Close button is set after this call
    nTitleWidth = CalcTitleWidth();

    nMaxWidth -= (IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2);

    // for an image, get its size, create a suitable control and position it
    aImageSize = maImage.GetSizePixel();
    if ( aImageSize.Width() )
    {
        aImageSize.AdjustWidth(4 );
        aImageSize.AdjustHeight(4 );
        aTextPos.AdjustX(aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE );
        mpFixedImage = VclPtr<FixedImage>::Create( this );
        mpFixedImage->SetPosSizePixel( Point( IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_X,
                                              IMPL_DIALOG_OFFSET-2+IMPL_MSGBOX_OFFSET_EXTRA_Y ),
                                       aImageSize );
        mpFixedImage->SetImage( maImage );
        mpFixedImage->Show();
        nMaxWidth -= aImageSize.Width()+IMPL_SEP_MSGBOX_IMAGE;
    }
    else
        aTextPos.AdjustX(IMPL_MSGBOX_OFFSET_EXTRA_X );

    // Determine maximum line length without wordbreak
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    nMaxLineWidth = aFormatRect.GetWidth();
    nTextStyle |= DrawTextFlags::WordBreak;

    // Determine the width for text formatting
    if ( nMaxLineWidth > 450 )
        nWidth = 450;
    else if ( nMaxLineWidth > 300 )
        nWidth = nMaxLineWidth+5;
    else
        nWidth = 300;

    nWidth *= GetDPIScaleFactor();

    if ( nButtonSize > nWidth )
        nWidth = nButtonSize-(aTextPos.X()-IMPL_DIALOG_OFFSET);
    if ( nWidth > nMaxWidth )
        nWidth = nMaxWidth;

    aRect.SetRight( nWidth );
    aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    if ( aTextInfo.GetMaxLineWidth() > nWidth )
    {
        nWidth = aTextInfo.GetMaxLineWidth()+8;
        aRect.SetRight( nWidth );
        aFormatRect = GetTextRect( aRect, aMessText, nTextStyle, &aTextInfo );
    }

    // get Style for VCLMultiLineEdit
    aMEditSize.setWidth( aTextInfo.GetMaxLineWidth()+1 );
    aMEditSize.setHeight( aFormatRect.GetHeight() );
    aPageSize.setWidth( aImageSize.Width() );
    if ( aMEditSize.Height() < aImageSize.Height() )
    {
        nWinStyle |= WB_VCENTER;
        aPageSize.setHeight( aImageSize.Height() );
        aMEditSize.setHeight( aImageSize.Height() );
    }
    else
    {
        nWinStyle |= WB_TOP;
        aPageSize.setHeight( aMEditSize.Height() );
    }
    if ( aImageSize.Width() )
        aPageSize.AdjustWidth(IMPL_SEP_MSGBOX_IMAGE );
    aPageSize.AdjustWidth((IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_X*2) );
    aPageSize.AdjustWidth(aMEditSize.Width()+1 );
    aPageSize.AdjustHeight((IMPL_DIALOG_OFFSET*2)+(IMPL_MSGBOX_OFFSET_EXTRA_Y*2) );

    if ( aPageSize.Width() < IMPL_MINSIZE_MSGBOX_WIDTH )
        aPageSize.setWidth( IMPL_MINSIZE_MSGBOX_WIDTH );
    if ( aPageSize.Width() < nTitleWidth )
        aPageSize.setWidth( nTitleWidth );

    mpVCLMultiLineEdit = VclPtr<VclMultiLineEdit>::Create( this, nWinStyle );
    mpVCLMultiLineEdit->SetText( aMessText );
    mpVCLMultiLineEdit->SetPosSizePixel( aTextPos, aMEditSize );
    mpVCLMultiLineEdit->Show();
    mpVCLMultiLineEdit->SetPaintTransparent(true);
    mpVCLMultiLineEdit->EnableCursor(false);
    SetPageSizePixel( aPageSize );
}

void MessBox::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
    {
        ImplPosControls();
    }
    ButtonDialog::StateChanged( nType );
}

Size MessBox::GetOptimalSize() const
{
    // FIXME: base me on the font size ?
    return Size( 250, 100 );
}


namespace {

extern "C" typedef vcl::Window* (*FN_SvtCreateWindow)(
        VCLXWindow** ppNewComp,
        const css::awt::WindowDescriptor* pDescriptor,
        vcl::Window* pParent,
        WinBits nWinBits );

class Pause : public Idle
{
public:
    explicit Pause(sal_Int32 nPauseMilliseconds) :
        Idle("pause"),
        m_nPauseMilliseconds(nPauseMilliseconds)
    {
        SetPriority(TaskPriority::HIGHEST);
        Start();
    }

    virtual void Invoke() override
    {
        SolarMutexGuard aSolarGuard;
        osl::Thread::wait(std::chrono::milliseconds(m_nPauseMilliseconds));
        Stop();
        delete this;
    }

    sal_Int32 const m_nPauseMilliseconds;
};

class VCLXToolkitMutexHelper
{
protected:
    ::osl::Mutex    maMutex;
};

class VCLXToolkit : public VCLXToolkitMutexHelper,
                    public cppu::WeakComponentImplHelper<
                    css::awt::XToolkitExperimental,
                    css::awt::XToolkitRobot,
                    css::lang::XServiceInfo >
{
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > mxClipboard;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > mxSelection;

    oslModule           hSvToolsLib;
    FN_SvtCreateWindow  fnSvtCreateWindow;

    ::comphelper::OInterfaceContainerHelper2 m_aTopWindowListeners;
    ::comphelper::OInterfaceContainerHelper2 m_aKeyHandlers;
    ::comphelper::OInterfaceContainerHelper2 m_aFocusListeners;
    ::Link<VclSimpleEvent&,void> const m_aEventListenerLink;
    ::Link<VclWindowEvent&,bool> const m_aKeyListenerLink;
    bool m_bEventListener;
    bool m_bKeyListener;

    DECL_LINK(eventListenerHandler, ::VclSimpleEvent&, void);

    DECL_LINK(keyListenerHandler, ::VclWindowEvent&, bool);

    void callTopWindowListeners(
        ::VclSimpleEvent const * pEvent,
        void (SAL_CALL css::awt::XTopWindowListener::* pFn)(
            css::lang::EventObject const &));

    bool callKeyHandlers(::VclSimpleEvent const * pEvent, bool bPressed);

    void callFocusListeners(::VclSimpleEvent const * pEvent, bool bGained);

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }

    virtual void SAL_CALL disposing() override;

    static vcl::Window* ImplCreateWindow( VCLXWindow** ppNewComp, const css::awt::WindowDescriptor& rDescriptor, vcl::Window* pParent,
                             WinBits nWinBits, MessBoxStyle nMessBoxStyle );
    css::uno::Reference< css::awt::XWindowPeer > ImplCreateWindow( const css::awt::WindowDescriptor& Descriptor,
                             MessBoxStyle nForceMessBoxStyle );

public:

    VCLXToolkit();

    // css::awt::XToolkitExperimental
    virtual void SAL_CALL processEventsToIdle() override;

    virtual sal_Int64 SAL_CALL getOpenGLBufferSwapCounter() override;

    virtual void SAL_CALL setDeterministicScheduling(sal_Bool bDeterministicMode) override;

    virtual void SAL_CALL pause(sal_Int32 nMilliseconds) override;

    virtual void SAL_CALL startRecording() override;

    virtual void SAL_CALL stopRecording() override;

    css::uno::Sequence< OUString > SAL_CALL getRecordingAndClear() override;

    // css::awt::XToolkit
    css::uno::Reference< css::awt::XWindowPeer >  SAL_CALL getDesktopWindow(  ) override;
    css::awt::Rectangle                                        SAL_CALL getWorkArea(  ) override;
    css::uno::Reference< css::awt::XWindowPeer >  SAL_CALL createWindow( const css::awt::WindowDescriptor& Descriptor ) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > SAL_CALL createWindows( const css::uno::Sequence< css::awt::WindowDescriptor >& Descriptors ) override;
    css::uno::Reference< css::awt::XDevice >      SAL_CALL createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) override;
    css::uno::Reference< css::awt::XRegion >      SAL_CALL createRegion(  ) override;

    // css::awt::XSystemChildFactory
    css::uno::Reference< css::awt::XWindowPeer > SAL_CALL createSystemChild( const css::uno::Any& Parent, const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) override;

    // css::awt::XMessageBoxFactory
    virtual css::uno::Reference< css::awt::XMessageBox > SAL_CALL createMessageBox( const css::uno::Reference< css::awt::XWindowPeer >& aParent, css::awt::MessageBoxType eType, ::sal_Int32 aButtons, const OUString& aTitle, const OUString& aMessage ) override;

    // css::awt::XDataTransfer
    css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL getDragGestureRecognizer( const css::uno::Reference< css::awt::XWindow >& window ) override;
    css::uno::Reference< css::datatransfer::dnd::XDragSource > SAL_CALL getDragSource( const css::uno::Reference< css::awt::XWindow >& window ) override;
    css::uno::Reference< css::datatransfer::dnd::XDropTarget > SAL_CALL getDropTarget( const css::uno::Reference< css::awt::XWindow >& window ) override;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const OUString& clipboardName ) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // css::awt::XExtendedToolkit:

    virtual ::sal_Int32 SAL_CALL getTopWindowCount() override;

    virtual css::uno::Reference< css::awt::XTopWindow >
    SAL_CALL getTopWindow(::sal_Int32 nIndex) override;

    virtual css::uno::Reference< css::awt::XTopWindow >
    SAL_CALL getActiveTopWindow() override;

    virtual void SAL_CALL addTopWindowListener(
        css::uno::Reference<
        css::awt::XTopWindowListener > const & rListener) override;

    virtual void SAL_CALL removeTopWindowListener(
        css::uno::Reference<
        css::awt::XTopWindowListener > const & rListener) override;

    virtual void SAL_CALL addKeyHandler(
        css::uno::Reference<
        css::awt::XKeyHandler > const & rHandler) override;

    virtual void SAL_CALL removeKeyHandler(
        css::uno::Reference<
        css::awt::XKeyHandler > const & rHandler) override;

    virtual void SAL_CALL addFocusListener(
        css::uno::Reference<
        css::awt::XFocusListener > const & rListener) override;

    virtual void SAL_CALL removeFocusListener(
        css::uno::Reference<
        css::awt::XFocusListener > const & rListener) override;

    virtual void SAL_CALL fireFocusGained(
        css::uno::Reference<
        css::uno::XInterface > const & source) override;

    virtual void SAL_CALL fireFocusLost(
        css::uno::Reference<
        css::uno::XInterface > const & source) override;

    // css::awt::XReschedule:
    virtual void SAL_CALL reschedule() override;

    // css:awt:XToolkitRobot
    virtual void SAL_CALL keyPress( const css::awt::KeyEvent & aKeyEvent ) override;

    virtual void SAL_CALL keyRelease( const css::awt::KeyEvent & aKeyEvent ) override;

    virtual void SAL_CALL mousePress( const css::awt::MouseEvent & aMouseEvent ) override;

    virtual void SAL_CALL mouseRelease( const css::awt::MouseEvent & aMouseEvent ) override;

    virtual void SAL_CALL mouseMove( const css::awt::MouseEvent & aMouseEvent ) override;

};

std::pair<WinBits,MessBoxStyle> ImplGetWinBits( sal_uInt32 nComponentAttribs, WindowType nCompType )
{
    WinBits nWinBits = 0;
    MessBoxStyle nStyle = MessBoxStyle::NONE;

    bool bMessBox = false;
    if ( ( nCompType == WindowType::INFOBOX ) ||
         ( nCompType == WindowType::MESSBOX ) ||
         ( nCompType == WindowType::QUERYBOX ) ||
         ( nCompType == WindowType::WARNINGBOX ) ||
         ( nCompType == WindowType::ERRORBOX ) )
    {
        bMessBox = true;
    }

    bool bDecoratedWindow = false;
    if  (   bMessBox
        ||  ( nCompType == WindowType::DIALOG )
        ||  ( nCompType == WindowType::MODELESSDIALOG )
        ||  ( nCompType == WindowType::MODALDIALOG )
        ||  ( nCompType == WindowType::DOCKINGWINDOW )
        ||  ( nCompType == WindowType::TABDIALOG )
        ||  ( nCompType == WindowType::BUTTONDIALOG )
        ||  ( nCompType == WindowType::SYSTEMCHILDWINDOW )
        )
    {
        bDecoratedWindow = true;
    }

    if( nComponentAttribs & css::awt::WindowAttribute::BORDER )
        nWinBits |= WB_BORDER;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::NOBORDER )
        nWinBits |= WB_NOBORDER;
    if( nComponentAttribs & css::awt::WindowAttribute::SIZEABLE )
        nWinBits |= WB_SIZEABLE;
    if( nComponentAttribs & css::awt::WindowAttribute::MOVEABLE )
        nWinBits |= WB_MOVEABLE;
    if( nComponentAttribs & css::awt::WindowAttribute::CLOSEABLE )
        nWinBits |= WB_CLOSEABLE;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::HSCROLL )
        nWinBits |= WB_HSCROLL;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::VSCROLL )
        nWinBits |= WB_VSCROLL;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::LEFT )
        nWinBits |= WB_LEFT;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::CENTER )
        nWinBits |= WB_CENTER;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::RIGHT )
        nWinBits |= WB_RIGHT;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::SPIN )
        nWinBits |= WB_SPIN;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::SORT )
        nWinBits |= WB_SORT;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DROPDOWN )
        nWinBits |= WB_DROPDOWN;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEFBUTTON )
        nWinBits |= WB_DEFBUTTON;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::READONLY )
        nWinBits |= WB_READONLY;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::CLIPCHILDREN )
        nWinBits |= WB_CLIPCHILDREN;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::GROUP )
        nWinBits |= WB_GROUP;
    if( nComponentAttribs & css::awt::VclWindowPeerAttribute::NOLABEL ) //added for issue79712
        nWinBits |= WB_NOLABEL;

    // These bits are not unique
    if ( bMessBox )
    {
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::OK )
            nStyle |= MessBoxStyle::Ok;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::OK_CANCEL )
            nStyle |= MessBoxStyle::OkCancel;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::YES_NO )
            nStyle |= MessBoxStyle::YesNo;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::YES_NO_CANCEL )
            nStyle |= MessBoxStyle::YesNoCancel;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::RETRY_CANCEL )
            nStyle |= MessBoxStyle::RetryCancel;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_OK )
            nStyle |= MessBoxStyle::DefaultOk;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_CANCEL )
            nStyle |= MessBoxStyle::DefaultCancel;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_RETRY )
            nStyle |= MessBoxStyle::DefaultRetry;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_YES )
            nStyle |= MessBoxStyle::DefaultYes;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_NO )
            nStyle |= MessBoxStyle::DefaultNo;
    }
    if ( nCompType == WindowType::MULTILINEEDIT || nCompType == WindowType::DIALOG || nCompType == WindowType::GROUPBOX )
    {
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::AUTOHSCROLL )
            nWinBits |= WB_AUTOHSCROLL;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::AUTOVSCROLL )
            nWinBits |= WB_AUTOVSCROLL;
    }


    if ( bDecoratedWindow )
    {
        if( nComponentAttribs & css::awt::WindowAttribute::NODECORATION )
        {
            // No decoration removes several window attributes and must
            // set WB_NOBORDER!
            nWinBits &= ~WB_BORDER;
            nWinBits &= ~WB_SIZEABLE;
            nWinBits &= ~WB_MOVEABLE;
            nWinBits &= ~WB_CLOSEABLE;
            nWinBits |= WB_NOBORDER;
        }
    }

    return { nWinBits, nStyle };
}

struct ComponentInfo
{
    OUStringLiteral sName;
    WindowType      nWinType;
};

static ComponentInfo const aComponentInfos [] =
{
    { OUStringLiteral("animatedimages"),     WindowType::CONTROL },
    { OUStringLiteral("buttondialog"),       WindowType::BUTTONDIALOG },
    { OUStringLiteral("cancelbutton"),       WindowType::CANCELBUTTON },
    { OUStringLiteral("checkbox"),           WindowType::CHECKBOX },
    { OUStringLiteral("combobox"),           WindowType::COMBOBOX },
    { OUStringLiteral("control"),            WindowType::CONTROL },
    { OUStringLiteral("currencybox"),        WindowType::CURRENCYBOX },
    { OUStringLiteral("currencyfield"),      WindowType::CURRENCYFIELD },
    { OUStringLiteral("datebox"),            WindowType::DATEBOX },
    { OUStringLiteral("datefield"),          WindowType::DATEFIELD },
    { OUStringLiteral("dialog"),             WindowType::DIALOG },
    { OUStringLiteral("dockingarea"),        WindowType::DOCKINGAREA },
    { OUStringLiteral("dockingwindow"),      WindowType::DOCKINGWINDOW },
    { OUStringLiteral("edit"),               WindowType::EDIT },
    { OUStringLiteral("errorbox"),           WindowType::ERRORBOX },
    { OUStringLiteral("fixedbitmap"),        WindowType::FIXEDBITMAP },
    { OUStringLiteral("fixedimage"),         WindowType::FIXEDIMAGE },
    { OUStringLiteral("fixedline"),          WindowType::FIXEDLINE },
    { OUStringLiteral("fixedtext"),          WindowType::FIXEDTEXT },
    { OUStringLiteral("floatingwindow"),     WindowType::FLOATINGWINDOW },
    { OUStringLiteral("frame"),              WindowType::GROUPBOX },
    { OUStringLiteral("framewindow"),        WindowType::TOOLKIT_FRAMEWINDOW },
    { OUStringLiteral("groupbox"),           WindowType::GROUPBOX },
    { OUStringLiteral("helpbutton"),         WindowType::HELPBUTTON },
    { OUStringLiteral("imagebutton"),        WindowType::IMAGEBUTTON },
    { OUStringLiteral("infobox"),            WindowType::INFOBOX },
    { OUStringLiteral("listbox"),            WindowType::LISTBOX },
    { OUStringLiteral("longcurrencybox"),    WindowType::LONGCURRENCYBOX },
    { OUStringLiteral("longcurrencyfield"),  WindowType::LONGCURRENCYFIELD },
    { OUStringLiteral("menubutton"),         WindowType::MENUBUTTON },
    { OUStringLiteral("messbox"),            WindowType::MESSBOX },
    { OUStringLiteral("metricbox"),          WindowType::METRICBOX },
    { OUStringLiteral("metricfield"),        WindowType::METRICFIELD },
    { OUStringLiteral("modaldialog"),        WindowType::MODALDIALOG },
    { OUStringLiteral("modelessdialog"),     WindowType::MODELESSDIALOG },
    { OUStringLiteral("morebutton"),         WindowType::MOREBUTTON },
    { OUStringLiteral("multilineedit"),      WindowType::MULTILINEEDIT },
    { OUStringLiteral("multilistbox"),       WindowType::MULTILISTBOX },
    { OUStringLiteral("numericbox"),         WindowType::NUMERICBOX },
    { OUStringLiteral("numericfield"),       WindowType::NUMERICFIELD },
    { OUStringLiteral("okbutton"),           WindowType::OKBUTTON },
    { OUStringLiteral("patternbox"),         WindowType::PATTERNBOX },
    { OUStringLiteral("patternfield"),       WindowType::PATTERNFIELD },
    { OUStringLiteral("pushbutton"),         WindowType::PUSHBUTTON },
    { OUStringLiteral("querybox"),           WindowType::QUERYBOX },
    { OUStringLiteral("radiobutton"),        WindowType::RADIOBUTTON },
    { OUStringLiteral("scrollbar"),          WindowType::SCROLLBAR },
    { OUStringLiteral("scrollbarbox"),       WindowType::SCROLLBARBOX },
    { OUStringLiteral("spinbutton"),         WindowType::SPINBUTTON },
    { OUStringLiteral("spinfield"),          WindowType::SPINFIELD },
    { OUStringLiteral("splitter"),           WindowType::SPLITTER },
    { OUStringLiteral("splitwindow"),        WindowType::SPLITWINDOW },
    { OUStringLiteral("statusbar"),          WindowType::STATUSBAR },
    { OUStringLiteral("systemchildwindow"),  WindowType::TOOLKIT_SYSTEMCHILDWINDOW },
    { OUStringLiteral("tabcontrol"),         WindowType::TABCONTROL },
    { OUStringLiteral("tabdialog"),          WindowType::TABDIALOG },
    { OUStringLiteral("tabpage"),            WindowType::TABPAGE },
    { OUStringLiteral("tabpagecontainer"),   WindowType::CONTROL },
    { OUStringLiteral("tabpagemodel"),       WindowType::TABPAGE },
    { OUStringLiteral("timebox"),            WindowType::TIMEBOX },
    { OUStringLiteral("timefield"),          WindowType::TIMEFIELD },
    { OUStringLiteral("toolbox"),            WindowType::TOOLBOX },
    { OUStringLiteral("tristatebox"),        WindowType::TRISTATEBOX },
    { OUStringLiteral("warningbox"),         WindowType::WARNINGBOX },
    { OUStringLiteral("window"),             WindowType::WINDOW },
    { OUStringLiteral("workwindow"),         WindowType::WORKWINDOW }
};

bool ComponentInfoFindCompare( const ComponentInfo & lhs, const OUString & s)
{
    return rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(s.pData->buffer, s.pData->length,
                lhs.sName.data, lhs.sName.size) > 0;
}

WindowType ImplGetComponentType( const OUString& rServiceName )
{
    static bool bSorted = false;
    if( !bSorted )
    {
        assert( std::is_sorted( std::begin(aComponentInfos), std::end(aComponentInfos),
                    [](const ComponentInfo & lhs, const ComponentInfo & rhs) {
                        return
                            rtl_str_compare_WithLength(
                                lhs.sName.data, lhs.sName.size, rhs.sName.data, rhs.sName.size)
                            < 0;
                    } ) );
        bSorted = true;
    }

    OUString sSearch;
    if ( !rServiceName.isEmpty() )
        sSearch = rServiceName;
    else
        sSearch = "window";

    auto it = std::lower_bound( std::begin(aComponentInfos), std::end(aComponentInfos), sSearch,
                                ComponentInfoFindCompare );
    if (it != std::end(aComponentInfos) &&
        rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(sSearch.pData->buffer, sSearch.pData->length, it->sName.data, it->sName.size) == 0)
        return it->nWinType;
    return WindowType::NONE;
}

struct MessageBoxTypeInfo
{
    css::awt::MessageBoxType const eType;
    const sal_Char          *pName;
    sal_Int32 const          nLen;
};

static const MessageBoxTypeInfo aMessageBoxTypeInfo[] =
{
    { css::awt::MessageBoxType_MESSAGEBOX,      RTL_CONSTASCII_STRINGPARAM("messbox") },
    { css::awt::MessageBoxType_INFOBOX,         RTL_CONSTASCII_STRINGPARAM("infobox") },
    { css::awt::MessageBoxType_WARNINGBOX,      RTL_CONSTASCII_STRINGPARAM("warningbox") },
    { css::awt::MessageBoxType_ERRORBOX,        RTL_CONSTASCII_STRINGPARAM("errorbox") },
    { css::awt::MessageBoxType_QUERYBOX,        RTL_CONSTASCII_STRINGPARAM("querybox") },
    { css::awt::MessageBoxType::MessageBoxType_MAKE_FIXED_SIZE, nullptr, 0 }
};

bool lcl_convertMessageBoxType(
    OUString &sType,
    css::awt::MessageBoxType eType )
{
    const MessageBoxTypeInfo *pMap = aMessageBoxTypeInfo;
    css::awt::MessageBoxType eVal = css::awt::MessageBoxType::MessageBoxType_MAKE_FIXED_SIZE;

    while ( pMap->pName )
    {
        if ( pMap->eType == eType )
        {
            eVal = eType;
            sType = OUString( pMap->pName, pMap->nLen, RTL_TEXTENCODING_ASCII_US );
            break;
        }
        pMap++;
    }

    return ( eVal != css::awt::MessageBoxType::MessageBoxType_MAKE_FIXED_SIZE );
}

static sal_Int32                            nVCLToolkitInstanceCount = 0;
static bool                                 bInitedByVCLToolkit = false;

osl::Mutex & getInitMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}

osl::Condition & getInitCondition()
{
    static osl::Condition aCondition;
    return aCondition;
}

extern "C"
{
static void ToolkitWorkerFunction( void* pArgs )
{
    osl_setThreadName("VCLXToolkit VCL main thread");

    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager;
    try
    {
        xServiceManager = ::comphelper::getProcessServiceFactory();
    }
    catch (const css::uno::DeploymentException&)
    {
    }
    if (!xServiceManager.is())
    {
        css::uno::Reference<css::uno::XComponentContext> xContext =
            ::cppu::defaultBootstrap_InitialComponentContext();

        xServiceManager.set( xContext->getServiceManager(), css::uno::UNO_QUERY_THROW );
        // set global process service factory used by unotools config helpers
        ::comphelper::setProcessServiceFactory( xServiceManager );
    }

    VCLXToolkit * pTk = static_cast<VCLXToolkit *>(pArgs);
    bInitedByVCLToolkit = InitVCL();
    if( bInitedByVCLToolkit )
    {
        UnoWrapper* pUnoWrapper = new UnoWrapper( pTk );
        UnoWrapperBase::SetUnoWrapper( pUnoWrapper );
    }
    getInitCondition().set();
    if( bInitedByVCLToolkit )
    {
        {
            SolarMutexGuard aGuard;
            Application::Execute();
        }
        try
        {
            pTk->dispose();
        }
        catch( css::uno::Exception & )
        {
        }
        DeInitVCL();
    }
    else
    {
        // having the thread join itself is pretty stupid.
        // but we can't get the osl_Thread to destroy here so just leak it.
    }
}
}

// constructor, which might initialize VCL
VCLXToolkit::VCLXToolkit():
    cppu::WeakComponentImplHelper<
    css::awt::XToolkitExperimental,
    css::awt::XToolkitRobot,
    css::lang::XServiceInfo>( GetMutex() ),
    m_aTopWindowListeners(rBHelper.rMutex),
    m_aKeyHandlers(rBHelper.rMutex),
    m_aFocusListeners(rBHelper.rMutex),
    m_aEventListenerLink(LINK(this, VCLXToolkit, eventListenerHandler)),
    m_aKeyListenerLink(LINK(this, VCLXToolkit, keyListenerHandler)),
    m_bEventListener(false),
    m_bKeyListener(false)
{
    hSvToolsLib = nullptr;
    fnSvtCreateWindow = nullptr;

    osl::Guard< osl::Mutex > aGuard( getInitMutex() );
    nVCLToolkitInstanceCount++;
    if( ( nVCLToolkitInstanceCount == 1 ) && ( !Application::IsInMain() ) )
    {
        // setup execute thread
        CreateMainLoopThread( ToolkitWorkerFunction, this );
        getInitCondition().wait();
    }
}

void SAL_CALL VCLXToolkit::disposing()
{
#ifndef DISABLE_DYNLOADING
    if ( hSvToolsLib )
    {
        osl_unloadModule( hSvToolsLib );
        hSvToolsLib = nullptr;
        fnSvtCreateWindow = nullptr;
    }
#endif

    {
        osl::Guard< osl::Mutex > aGuard( getInitMutex() );
        if( --nVCLToolkitInstanceCount == 0 )
        {
            if( bInitedByVCLToolkit )
            {
                Application::Quit();
                JoinMainLoopThread();
                bInitedByVCLToolkit = false;
            }
        }
    }

    if (m_bEventListener)
    {
        ::Application::RemoveEventListener(m_aEventListenerLink);
        m_bEventListener = false;
    }
    if (m_bKeyListener)
    {
        ::Application::RemoveKeyListener(m_aKeyListenerLink);
        m_bKeyListener = false;
    }
    css::lang::EventObject aEvent(
        static_cast< ::cppu::OWeakObject * >(this));
    m_aTopWindowListeners.disposeAndClear(aEvent);
    m_aKeyHandlers.disposeAndClear(aEvent);
    m_aFocusListeners.disposeAndClear(aEvent);
}


css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::getDesktopWindow(  )
{
    css::uno::Reference< css::awt::XWindowPeer > xRef;
    // 07/00: AppWindow doesn't exist anymore...
    return xRef;
}

css::awt::Rectangle VCLXToolkit::getWorkArea(  )
{
    sal_Int32 nDisplay = Application::GetDisplayBuiltInScreen();
    tools::Rectangle aWorkRect = Application::GetScreenPosSizePixel( nDisplay );
    css::awt::Rectangle aNotherRect;
    aNotherRect.X = aWorkRect.getX();
    aNotherRect.Y = aWorkRect.getY();
    aNotherRect.Width = aWorkRect.getWidth();
    aNotherRect.Height = aWorkRect.getHeight();
    return aNotherRect;
}

css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::createWindow( const css::awt::WindowDescriptor& rDescriptor )
{
    return ImplCreateWindow( rDescriptor, MessBoxStyle::NONE );
}

css::uno::Reference< css::awt::XDevice > VCLXToolkit::createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::awt::XDevice > xRef;
    VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;

    SolarMutexGuard aSolarGuard;

    VclPtrInstance<VirtualDevice> pV;
    pV->SetOutputSizePixel( Size( Width, Height ) );
    pVDev->SetVirtualDevice( pV );

    xRef = pVDev;
    return xRef;
}

css::uno::Reference< css::awt::XRegion > VCLXToolkit::createRegion(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::awt::XRegion >  xRef = new VCLXRegion;
    return xRef;
}

class InfoBox : public MessBox
{
public:
    InfoBox(vcl::Window* pParent, const OUString& rMessage)
        : MessBox(pParent, MessBoxStyle::Ok | MessBoxStyle::DefaultOk, 0, OUString(), rMessage)
    {
        // Default Text is the display title from the application
        if (GetText().isEmpty())
            SetText(GetStandardInfoBoxText());
        SetImage(GetStandardInfoBoxImage());
    }
};

class ErrorBox : public MessBox
{
public:
    ErrorBox(vcl::Window* pParent, MessBoxStyle nStyle, WinBits nWinBits, const OUString& rMessage)
        : MessBox(pParent, nStyle, nWinBits, OUString(), rMessage)
    {
        // Default Text is the display title from the application
        if (GetText().isEmpty())
            SetText(GetStandardErrorBoxText());
        SetImage(GetStandardErrorBoxImage());
    }
};

class QueryBox : public MessBox
{
public:
    QueryBox(vcl::Window* pParent, MessBoxStyle nStyle, WinBits nWinBits, const OUString& rMessage)
        : MessBox(pParent, nStyle, nWinBits, OUString(), rMessage)
    {
        // Default Text is the display title from the application
        if (GetText().isEmpty())
            SetText(GetStandardQueryBoxText());
        SetImage(GetStandardQueryBoxImage());
    }
};

class WarningBox : public MessBox
{
public:
    WarningBox(vcl::Window* pParent, MessBoxStyle nStyle, WinBits nWinBits, const OUString& rMessage)
        : MessBox(pParent, nStyle, nWinBits, OUString(), rMessage)
    {
        // Default Text is the display title from the application
        if (GetText().isEmpty())
            SetText(GetStandardWarningBoxText());
        SetImage(GetStandardWarningBoxImage());
    }
};

vcl::Window* VCLXToolkit::ImplCreateWindow( VCLXWindow** ppNewComp,
    const css::awt::WindowDescriptor& rDescriptor,
    vcl::Window* pParent, WinBits nWinBits, MessBoxStyle nMessBoxStyle )
{
    OUString aServiceName( rDescriptor.WindowServiceName );
    aServiceName = aServiceName.toAsciiLowerCase();

    VclPtr<vcl::Window> pNewWindow;
    WindowType nType = ImplGetComponentType( aServiceName );
    bool bFrameControl = false;
    if ( aServiceName == "frame" )
        bFrameControl = true;
    if ( aServiceName == "tabcontrolnotabs" )
    {
        nWinBits |= WB_NOBORDER;
        nType = ImplGetComponentType( "tabcontrol" );
    }
    if ( !pParent )
    {
        // If the component needs a parent, then return NULL,
        // some time later css::uno::Exception...
        bool bException = true;
        if  (   ( nType == WindowType::DIALOG )
            ||  ( nType == WindowType::MODALDIALOG )
            ||  ( nType == WindowType::MODELESSDIALOG )
            ||  ( nType == WindowType::MESSBOX )
            ||  ( nType == WindowType::INFOBOX )
            ||  ( nType == WindowType::WARNINGBOX )
            ||  ( nType == WindowType::ERRORBOX )
            ||  ( nType == WindowType::QUERYBOX )
            )
            bException = false;
        else if ( ( nType == WindowType::WINDOW ) ||
                  ( nType == WindowType::WORKWINDOW ) ||
                  ( nType == WindowType::TOOLKIT_FRAMEWINDOW ) )
        {
            if ( rDescriptor.Type == css::awt::WindowClass_TOP )
                bException = false;
        }

        if ( bException )
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }

    if ( nType != WindowType::NONE )
    {
        SolarMutexGuard aVclGuard;
        switch ( nType )
        {
            case WindowType::CANCELBUTTON:
                pNewWindow = VclPtr<CancelButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::CHECKBOX:
                 pNewWindow = VclPtr<CheckBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXCheckBox;
            break;
            case WindowType::COMBOBOX:
                pNewWindow = VclPtr<ComboBox>::Create( pParent, nWinBits|WB_AUTOHSCROLL );
                static_cast<ComboBox*>(pNewWindow.get())->EnableAutoSize( false );
                *ppNewComp = new VCLXComboBox;
            break;
            case WindowType::CURRENCYBOX:
                pNewWindow = VclPtr<CurrencyBox>::Create( pParent, nWinBits );
            break;
            case WindowType::CURRENCYFIELD:
                pNewWindow = VclPtr<CurrencyField>::Create( pParent, nWinBits );
                static_cast<CurrencyField*>(pNewWindow.get())->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXNumericField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<CurrencyField*>(pNewWindow.get())) );
            break;
            case WindowType::DATEBOX:
                pNewWindow = VclPtr<DateBox>::Create( pParent, nWinBits );
            break;
            case WindowType::DATEFIELD:
                pNewWindow = VclPtr<DateField>::Create( pParent, nWinBits );
                static_cast<DateField*>(pNewWindow.get())->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXDateField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<DateField*>(pNewWindow.get())) );
            break;
            case WindowType::DOCKINGAREA:
                pNewWindow = VclPtr<DockingAreaWindow>::Create( pParent );
            break;
            case WindowType::MULTILINEEDIT:
            case WindowType::EDIT:
                pNewWindow = VclPtr<Edit>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXEdit;
            break;
            case WindowType::ERRORBOX:
                pNewWindow = VclPtr<ErrorBox>::Create( pParent, nMessBoxStyle, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WindowType::FIXEDBITMAP:
                pNewWindow = VclPtr<FixedBitmap>::Create( pParent, nWinBits );
            break;
            case WindowType::FIXEDIMAGE:
                pNewWindow = VclPtr<ImageControl>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXImageControl;
            break;
            case WindowType::FIXEDLINE:
                pNewWindow = VclPtr<FixedLine>::Create( pParent, nWinBits );
            break;
            case WindowType::FIXEDTEXT:
                pNewWindow = VclPtr<FixedText>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXFixedText;
            break;
            case WindowType::FLOATINGWINDOW:
                pNewWindow = VclPtr<FloatingWindow>::Create( pParent, nWinBits );
            break;
            case WindowType::GROUPBOX:
                pNewWindow = VclPtr<GroupBox>::Create( pParent, nWinBits );
                if ( bFrameControl )
                {
                    GroupBox* pGroupBox =  static_cast< GroupBox* >( pNewWindow.get() );
                    *ppNewComp = new VCLXFrame;
                    // Frame control needs to receive
                    // Mouse events
                    pGroupBox->SetMouseTransparent( false );
                }
            break;
            case WindowType::HELPBUTTON:
                pNewWindow = VclPtr<HelpButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::IMAGEBUTTON:
                 pNewWindow = VclPtr<ImageButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::INFOBOX:
                pNewWindow = VclPtr<InfoBox>::Create( pParent, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WindowType::LISTBOX:
                pNewWindow = VclPtr<ListBox>::Create( pParent, nWinBits|WB_SIMPLEMODE|WB_AUTOHSCROLL );
                static_cast<ListBox*>(pNewWindow.get())->EnableAutoSize( false );
                *ppNewComp = new VCLXListBox;
            break;
            case WindowType::LONGCURRENCYBOX:
                pNewWindow = VclPtr<LongCurrencyBox>::Create( pParent, nWinBits );
            break;
            case WindowType::LONGCURRENCYFIELD:
                pNewWindow = VclPtr<LongCurrencyField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXCurrencyField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<LongCurrencyField*>(pNewWindow.get())) );
            break;
            case WindowType::MENUBUTTON:
                pNewWindow = VclPtr<MenuButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::MESSBOX:
                pNewWindow = VclPtr<MessBox>::Create( pParent, nMessBoxStyle, nWinBits, OUString(), OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WindowType::METRICBOX:
                pNewWindow = VclPtr<MetricBox>::Create( pParent, nWinBits );
            break;
            case WindowType::METRICFIELD:
                pNewWindow = VclPtr<MetricField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXMetricField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<MetricField*>(pNewWindow.get())) );
            break;
            case WindowType::DIALOG:
            case WindowType::MODALDIALOG:
            case WindowType::MODELESSDIALOG:
            {
                // Modal/Modeless only via Show/Execute
                if ( (pParent == nullptr ) && ( rDescriptor.ParentIndex == -1 ) )
                    pNewWindow = VclPtr<toolkit::ScrollableDialog>::Create( nullptr, nWinBits, Dialog::InitFlag::NoParent );
                else
                    pNewWindow = VclPtr<toolkit::ScrollableDialog>::Create( pParent, nWinBits );
                // #i70217# Don't always create a new component object. It's possible that VCL has called
                // GetComponentInterface( sal_True ) in the Dialog ctor itself (see Window::IsTopWindow() )
                // which creates a component object.
                css::uno::Reference< css::awt::XWindowPeer > xWinPeer = pNewWindow->GetComponentInterface( false );
                if ( xWinPeer.is() )
                    *ppNewComp = dynamic_cast< VCLXDialog* >( xWinPeer.get() );
                else
                    *ppNewComp = new VCLXDialog;
            }
            break;
            case WindowType::MOREBUTTON:
                pNewWindow = VclPtr<MoreButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::MULTILISTBOX:
                pNewWindow = VclPtr<MultiListBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXListBox;
            break;
            case WindowType::NUMERICBOX:
                pNewWindow = VclPtr<NumericBox>::Create( pParent, nWinBits );
            break;
            case WindowType::NUMERICFIELD:
                pNewWindow = VclPtr<NumericField>::Create( pParent, nWinBits );
                static_cast<NumericField*>(pNewWindow.get())->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXNumericField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<NumericField*>(pNewWindow.get())) );
            break;
            case WindowType::OKBUTTON:
                pNewWindow = VclPtr<OKButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::PATTERNBOX:
                pNewWindow = VclPtr<PatternBox>::Create( pParent, nWinBits );
            break;
            case WindowType::PATTERNFIELD:
                pNewWindow = VclPtr<PatternField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXPatternField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<PatternField*>(pNewWindow.get())) );
            break;
            case WindowType::PUSHBUTTON:
                pNewWindow = VclPtr<PushButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WindowType::QUERYBOX:
                pNewWindow = VclPtr<QueryBox>::Create( pParent, nMessBoxStyle, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WindowType::RADIOBUTTON:
                pNewWindow = VclPtr<RadioButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXRadioButton;

                // by default, disable RadioCheck
                // Since the VCLXRadioButton really cares for its RadioCheck settings, this is important:
                // if we enable it, the VCLXRadioButton will use RadioButton::Check instead of RadioButton::SetState
                // This leads to a strange behaviour if the control is newly created: when settings the initial
                // state to "checked", the RadioButton::Check (called because RadioCheck=sal_True) will uncheck
                // _all_other_ radio buttons in the same group. However, at this moment the grouping of the controls
                // is not really valid: the controls are grouped after they have been created, but we're still in
                // the creation process, so the RadioButton::Check relies on invalid grouping information.
                // 07.08.2001 - #87254# - frank.schoenheit@sun.com
                static_cast<RadioButton*>(pNewWindow.get())->EnableRadioCheck( false );
            break;
            case WindowType::SCROLLBAR:
                pNewWindow = VclPtr<ScrollBar>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXScrollBar;
            break;
            case WindowType::SCROLLBARBOX:
                pNewWindow = VclPtr<ScrollBarBox>::Create( pParent, nWinBits );
            break;
            case WindowType::SPINBUTTON:
                pNewWindow = VclPtr<SpinButton>::Create( pParent, nWinBits );
                *ppNewComp = new ::toolkit::VCLXSpinButton;
            break;
            case WindowType::SPINFIELD:
                pNewWindow = VclPtr<SpinField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXNumericField;
            break;
            case WindowType::SPLITTER:
                pNewWindow = VclPtr<Splitter>::Create( pParent, nWinBits );
            break;
            case WindowType::SPLITWINDOW:
                pNewWindow = VclPtr<SplitWindow>::Create( pParent, nWinBits );
            break;
            case WindowType::STATUSBAR:
                pNewWindow = VclPtr<StatusBar>::Create( pParent, nWinBits );
            break;
            case WindowType::TOOLKIT_SYSTEMCHILDWINDOW:
                pNewWindow = VclPtr<SystemChildWindow>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXSystemDependentWindow();
            break;
            case WindowType::TABCONTROL:
                pNewWindow = VclPtr<TabControl>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXMultiPage;
            break;
            case WindowType::TABDIALOG:
                pNewWindow = VclPtr<TabDialog>::Create( pParent, nWinBits );
            break;
            case WindowType::TABPAGE:
                {
                    pNewWindow = VclPtr<TabPage>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXTabPage;
                }
            break;
            case WindowType::TIMEBOX:
                pNewWindow = VclPtr<TimeBox>::Create( pParent, nWinBits );
            break;
            case WindowType::TIMEFIELD:
                pNewWindow = VclPtr<TimeField>::Create( pParent, nWinBits );
                static_cast<TimeField*>(pNewWindow.get())->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXTimeField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<TimeField*>(pNewWindow.get())) );
            break;
            case WindowType::TOOLBOX:
                pNewWindow = VclPtr<ToolBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXToolBox;
            break;
            case WindowType::TRISTATEBOX:
                pNewWindow = VclPtr<TriStateBox>::Create( pParent, nWinBits );
            break;
            case WindowType::WARNINGBOX:
                pNewWindow = VclPtr<WarningBox>::Create( pParent, nMessBoxStyle, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WindowType::WORKWINDOW:
            case WindowType::WINDOW:
            case WindowType::TOOLKIT_FRAMEWINDOW:
            case WindowType::DOCKINGWINDOW:
                if ( rDescriptor.Type == css::awt::WindowClass_TOP )
                {
                    if (nType == WindowType::DOCKINGWINDOW )
                        pNewWindow = VclPtr<DockingWindow>::Create( pParent, nWinBits );
                    else
                    {
                        if ((pParent == nullptr) && rDescriptor.Parent.is())
                        {
                            // try to get a system dependent window handle
                            css::uno::Reference< css::awt::XSystemDependentWindowPeer > xSystemDepParent(rDescriptor.Parent, css::uno::UNO_QUERY);

                            if (xSystemDepParent.is())
                            {
                                sal_Int8 processID[16];

                                rtl_getGlobalProcessId( reinterpret_cast<sal_uInt8*>(processID) );

                                css::uno::Sequence<sal_Int8> processIdSeq(processID, 16);

                                css::uno::Any anyHandle = xSystemDepParent->getWindowHandle(processIdSeq, SYSTEM_DEPENDENT_TYPE);

                                // use sal_Int64 here to accommodate all int types
                                // uno::Any shift operator whill upcast if necessary
                                sal_Int64 nWindowHandle = 0;
                                bool bXEmbed = false;

                                bool bUseParentData = true;
                                if( ! (anyHandle >>= nWindowHandle) )
                                {
                                    css::uno::Sequence< css::beans::NamedValue > aProps;
                                    if( anyHandle >>= aProps )
                                    {
                                        const int nProps = aProps.getLength();
                                        const css::beans::NamedValue* pProps = aProps.getConstArray();
                                        for( int i = 0; i < nProps; i++ )
                                        {
                                            if ( pProps[i].Name == "WINDOW" )
                                                pProps[i].Value >>= nWindowHandle;
                                            else if ( pProps[i].Name == "XEMBED" )
                                                pProps[i].Value >>= bXEmbed;
                                        }
                                    }
                                    else
                                        bUseParentData = false;
                                }

                                if( bUseParentData )
                                {
                                    SystemParentData aParentData;
                                    aParentData.nSize   = sizeof( aParentData );
                                    #if defined MACOSX
                                    aParentData.pView   = reinterpret_cast<NSView*>(nWindowHandle);
                                    #elif defined ANDROID
                                    // Nothing
                                    #elif defined IOS
                                    // Nothing
                                    #elif defined UNX
                                    aParentData.aWindow = nWindowHandle;
                                    aParentData.bXEmbedSupport = bXEmbed;
                                    #elif defined WNT
                                    aParentData.hWnd = reinterpret_cast<HWND>(nWindowHandle);
                                    #endif
                                    pNewWindow = VclPtr<WorkWindow>::Create( &aParentData );
                                }
                            }
                        }

                        if (!pNewWindow)
                            pNewWindow = VclPtr<WorkWindow>::Create( pParent, nWinBits );
                    }

                    *ppNewComp = new VCLXTopWindow();
                }
                else if ( rDescriptor.Type == css::awt::WindowClass_CONTAINER )
                {
                    if (nType == WindowType::DOCKINGWINDOW )
                        pNewWindow = VclPtr<DockingWindow>::Create( pParent, nWinBits );
                    else
                        pNewWindow = VclPtr<vcl::Window>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXContainer;
                }
                else
                {
                    if (nType == WindowType::DOCKINGWINDOW )
                        pNewWindow = VclPtr<DockingWindow>::Create( pParent, nWinBits );
                    else
                        pNewWindow = VclPtr<vcl::Window>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXWindow;
                }
            break;
            case WindowType::CONTROL:
                if ( rDescriptor.WindowServiceName.equalsIgnoreAsciiCase(
                        "tabpagecontainer" ) )
                {
                    // TabControl has a special case for tabs without border: they are displayed
                    // in a different way, so we need to ensure that this style is not set, so
                    // we can guarantee normal tab behavior
                    pNewWindow = VclPtr<TabControl>::Create( pParent, nWinBits & (~WB_NOBORDER));
                    *ppNewComp = new VCLXTabPageContainer;
                }
                else if ( aServiceName == "animatedimages" )
                {
                    pNewWindow = VclPtr<Throbber>::Create( pParent, nWinBits );
                    *ppNewComp = new ::toolkit::AnimatedImagesPeer;
                }
            break;
            default:
                OSL_ENSURE( false, "VCLXToolkit::ImplCreateWindow: unknown window type!" );
                break;
        }
    }

    return pNewWindow;
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void thisModule() {} }

#else

extern "C" vcl::Window* SAL_CALL CreateWindow( VCLXWindow** ppNewComp, const css::awt::WindowDescriptor* pDescriptor, vcl::Window* pParent, WinBits nWinBits );

#endif

css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::ImplCreateWindow(
    const css::awt::WindowDescriptor& rDescriptor,
    MessBoxStyle nForceMessBoxStyle )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SolarMutexGuard aSolarGuard;

    css::uno::Reference< css::awt::XWindowPeer > xRef;

    VclPtr<vcl::Window> pParent;
    if ( rDescriptor.Parent.is() )
    {
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( rDescriptor.Parent );

        // #103939# Don't throw assertion, may be it's a system dependent window, used in ImplCreateWindow.
        // DBG_ASSERT( pParentComponent, "ParentComponent not valid" );

        if ( pParentComponent )
            pParent = pParentComponent->GetWindow();
    }
    std::pair<WinBits, MessBoxStyle> aPair = ImplGetWinBits( rDescriptor.WindowAttributes,
        ImplGetComponentType( rDescriptor.WindowServiceName ) );
    WinBits nWinBits = aPair.first;
    aPair.second |= nForceMessBoxStyle;

    VCLXWindow* pNewComp = nullptr;

    vcl::Window* pNewWindow = nullptr;
    // Try to create the window with SvTools
    // (do this _before_ creating it on our own: The old mechanism (extended toolkit in SvTools) did it this way,
    // and we need to stay compatible)
    // try to load the lib
    if ( !fnSvtCreateWindow
#ifndef DISABLE_DYNLOADING
         && !hSvToolsLib
#endif
         )
    {
#ifndef DISABLE_DYNLOADING
        OUString aLibName(SVT_DLL_NAME);
        hSvToolsLib = osl_loadModuleRelative(
            &thisModule, aLibName.pData, SAL_LOADMODULE_DEFAULT );
        if ( hSvToolsLib )
        {
            OUString aFunctionName( "CreateWindow" );
            fnSvtCreateWindow = reinterpret_cast<FN_SvtCreateWindow>(osl_getFunctionSymbol( hSvToolsLib, aFunctionName.pData ));
        }
#else
        fnSvtCreateWindow = CreateWindow;
#endif
    }
    // ask the SvTool creation function
    if ( fnSvtCreateWindow )
        pNewWindow = fnSvtCreateWindow( &pNewComp, &rDescriptor, pParent, nWinBits );

    // if SvTools could not provide a window, create it ourself
    if ( !pNewWindow )
        pNewWindow = ImplCreateWindow( &pNewComp, rDescriptor, pParent, nWinBits, aPair.second );

    DBG_ASSERT( pNewWindow, "createWindow: Unknown Component!" );
    SAL_INFO_IF( !pNewComp, "toolkit", "createWindow: No special Interface!" );

    if ( pNewWindow )
    {
        pNewWindow->SetCreatedWithToolkit( true );
        //pNewWindow->SetPosPixel( Point() ); // do not force (0,0) position, keep default pos instead

        if ( rDescriptor.WindowAttributes & css::awt::WindowAttribute::MINSIZE )
        {
            pNewWindow->SetSizePixel( Size() );
        }
        else if ( rDescriptor.WindowAttributes & css::awt::WindowAttribute::FULLSIZE )
        {
            if ( pParent )
                pNewWindow->SetSizePixel( pParent->GetOutputSizePixel() );
        }
        else if ( !VCLUnoHelper::IsZero( rDescriptor.Bounds ) )
        {
            tools::Rectangle aRect = VCLRectangle( rDescriptor.Bounds );
            pNewWindow->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        }

        if ( !pNewComp )
        {
            // Default-Interface
            xRef = pNewWindow->GetComponentInterface();
        }
        else
        {
            xRef = pNewComp;
            pNewWindow->SetComponentInterface( xRef );
        }
        DBG_ASSERT( pNewWindow->GetComponentInterface( false ) == xRef,
            "VCLXToolkit::createWindow: did #133706# resurge?" );

        if ( rDescriptor.WindowAttributes & css::awt::WindowAttribute::SHOW )
            pNewWindow->Show();
    }

    return xRef;
}

css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > VCLXToolkit::createWindows( const css::uno::Sequence< css::awt::WindowDescriptor >& rDescriptors )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nComponents = rDescriptors.getLength();
    css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > aSeq( nComponents );
    for ( sal_uInt32 n = 0; n < nComponents; n++ )
    {
        css::awt::WindowDescriptor aDescr = rDescriptors.getConstArray()[n];

        if ( aDescr.ParentIndex == -1 )
            aDescr.Parent = nullptr;
        else if ( ( aDescr.ParentIndex >= 0 ) && ( aDescr.ParentIndex < static_cast<short>(n) ) )
            aDescr.Parent = aSeq.getConstArray()[aDescr.ParentIndex];
        aSeq.getArray()[n] = createWindow( aDescr );
    }
    return aSeq;
}

// css::awt::XSystemChildFactory
css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::createSystemChild( const css::uno::Any& Parent, const css::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 nSystemType )
{
    VclPtr<vcl::Window> pChildWindow;
    if ( nSystemType == SYSTEM_DEPENDENT_TYPE )
    {
        // use sal_Int64 here to accommodate all int types
        // uno::Any shift operator whill upcast if necessary
        sal_Int64 nWindowHandle = 0;
        bool bXEmbed = false;

        bool bUseParentData = true;
        if( ! (Parent >>= nWindowHandle) )
        {
            css::uno::Sequence< css::beans::NamedValue > aProps;
            if( Parent >>= aProps )
            {
                const int nProps = aProps.getLength();
                const css::beans::NamedValue* pProps = aProps.getConstArray();
                for( int i = 0; i < nProps; i++ )
                {
                    if ( pProps[i].Name == "WINDOW" )
                        pProps[i].Value >>= nWindowHandle;
                    else if ( pProps[i].Name == "XEMBED" )
                        pProps[i].Value >>= bXEmbed;
                }
            }
            else
                bUseParentData = false;
        }

        if( bUseParentData )
        {
            SystemParentData aParentData;
            aParentData.nSize   = sizeof( aParentData );
            #if defined MACOSX
            aParentData.pView   = reinterpret_cast<NSView*>(nWindowHandle);
            #elif defined ANDROID
            // Nothing
            #elif defined IOS
            // Nothing
            #elif defined UNX
            aParentData.aWindow = nWindowHandle;
            aParentData.bXEmbedSupport = bXEmbed;
            #elif defined WNT
            aParentData.hWnd = reinterpret_cast<HWND>(nWindowHandle);
            #endif
            SolarMutexGuard aGuard;
            try
            {
                pChildWindow.reset( VclPtr<WorkWindow>::Create( &aParentData ) );
            }
            catch ( const css::uno::RuntimeException & )
            {
                // system child window could not be created
                DBG_UNHANDLED_EXCEPTION("toolkit");
                pChildWindow.clear();
            }
        }
    }
    else if (nSystemType == css::lang::SystemDependent::SYSTEM_JAVA)
    {
        SolarMutexGuard aGuard;
        pChildWindow.reset(VclPtr<WorkWindow>::Create(nullptr, Parent));
    }

    css::uno::Reference< css::awt::XWindowPeer > xPeer;
    if ( pChildWindow )
    {
        VCLXTopWindow* pPeer = new VCLXTopWindow;
        SolarMutexGuard aGuard;
        pPeer->SetWindow( pChildWindow );
        xPeer = pPeer;
    }

    return xPeer;
}

// css::awt::XMessageBoxFactory
css::uno::Reference< css::awt::XMessageBox > SAL_CALL VCLXToolkit::createMessageBox(
    const css::uno::Reference< css::awt::XWindowPeer >& aParent,
    css::awt::MessageBoxType eType,
    ::sal_Int32 aButtons,
    const OUString& aTitle,
    const OUString& aMessage )
{
    css::awt::WindowDescriptor aDescriptor;

    sal_Int32 nWindowAttributes = css::awt::WindowAttribute::BORDER|css::awt::WindowAttribute::MOVEABLE|css::awt::WindowAttribute::CLOSEABLE;

    // Map button definitions to window attributes
    if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_OK )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::OK;
    else if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_OK_CANCEL )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::OK_CANCEL;
    else if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_YES_NO )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::YES_NO;
    else if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_YES_NO_CANCEL )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::YES_NO_CANCEL;
    else if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_RETRY_CANCEL )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::RETRY_CANCEL;

    // Map default button definitions to window attributes
    if (sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_OK )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::DEF_OK;
    else if (sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_CANCEL )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::DEF_CANCEL;
    else if (sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_YES )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::DEF_YES;
    else if (sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_NO )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::DEF_NO;
    else if (sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_RETRY )
        nWindowAttributes |= css::awt::VclWindowPeerAttribute::DEF_RETRY;

    // No more bits for VclWindowPeerAttribute possible. Mapping must be
    // done explicitly using VCL methods
    MessBoxStyle nAddWinBits = MessBoxStyle::NONE;
    if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_ABORT_IGNORE_RETRY )
        nAddWinBits |= MessBoxStyle::AbortRetryIgnore;
    if ( sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_IGNORE )
        nAddWinBits |= MessBoxStyle::DefaultIgnore;

    OUString aType;
    lcl_convertMessageBoxType( aType, eType );

    aDescriptor.Type              = css::awt::WindowClass_MODALTOP;
    aDescriptor.WindowServiceName = aType;
    aDescriptor.ParentIndex       = -1;
    aDescriptor.Parent            = aParent;
    aDescriptor.WindowAttributes  = nWindowAttributes;
    css::uno::Reference< css::awt::XMessageBox > xMsgBox(
        ImplCreateWindow( aDescriptor, nAddWinBits ), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xWindow( xMsgBox, css::uno::UNO_QUERY );
    if ( xMsgBox.is() && xWindow.is() )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
        {
            SolarMutexGuard aGuard;
            xMsgBox->setCaptionText( aTitle );
            xMsgBox->setMessageText( aMessage );
        }
    }

    return xMsgBox;
}

css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL VCLXToolkit::getDragGestureRecognizer( const css::uno::Reference< css::awt::XWindow >& window )
{
    SolarMutexGuard g;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragGestureRecognizer();

    return css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer >();
}

css::uno::Reference< css::datatransfer::dnd::XDragSource > SAL_CALL VCLXToolkit::getDragSource( const css::uno::Reference< css::awt::XWindow >& window )
{
    SolarMutexGuard g;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragSource();

    return css::uno::Reference< css::datatransfer::dnd::XDragSource >();
}

css::uno::Reference< css::datatransfer::dnd::XDropTarget > SAL_CALL VCLXToolkit::getDropTarget( const css::uno::Reference< css::awt::XWindow >& window )
{
    SolarMutexGuard g;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDropTarget();

    return css::uno::Reference< css::datatransfer::dnd::XDropTarget >();
}

css::uno::Reference< css::datatransfer::clipboard::XClipboard > SAL_CALL VCLXToolkit::getClipboard( const OUString& clipboardName )
{
    if( clipboardName.isEmpty() )
    {
        if( !mxClipboard.is() )
        {
            // remember clipboard here
            mxClipboard = css::datatransfer::clipboard::SystemClipboard::create(
                comphelper::getProcessComponentContext());
        }

        return mxClipboard;
    }

    else if( clipboardName == "Selection" )
    {
        return mxSelection;
    }

    return css::uno::Reference< css::datatransfer::clipboard::XClipboard >();
}

// XServiceInfo
OUString VCLXToolkit::getImplementationName()
{
    return OUString("stardiv.Toolkit.VCLXToolkit");
}

sal_Bool VCLXToolkit::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > VCLXToolkit::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.awt.Toolkit", "stardiv.vcl.VclToolkit"};
}

// css::awt::XExtendedToolkit:

// virtual
::sal_Int32 SAL_CALL VCLXToolkit::getTopWindowCount()
{
    return static_cast< ::sal_Int32 >(::Application::GetTopWindowCount());
        // XXX  numeric overflow
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getTopWindow(::sal_Int32 nIndex)
{
    vcl::Window * p = ::Application::GetTopWindow(static_cast< long >(nIndex));
        // XXX  numeric overflow
    return css::uno::Reference< css::awt::XTopWindow >(
        p == nullptr ? nullptr : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getActiveTopWindow()
{
    vcl::Window * p = ::Application::GetActiveTopWindow();
    return css::uno::Reference< css::awt::XTopWindow >(
        p == nullptr ? nullptr : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
void SAL_CALL VCLXToolkit::addTopWindowListener(
    css::uno::Reference< css::awt::XTopWindowListener > const & rListener)
{
    OSL_ENSURE(rListener.is(), "Null rListener");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rListener->disposing(
            css::lang::EventObject(
                static_cast< ::cppu::OWeakObject * >(this)));
    }
    else if (m_aTopWindowListeners.addInterface(rListener) == 1
             && !m_bEventListener)
    {
        m_bEventListener = true;
        ::Application::AddEventListener(m_aEventListenerLink);
    }
}

// virtual
void SAL_CALL VCLXToolkit::removeTopWindowListener(
    css::uno::Reference< css::awt::XTopWindowListener > const & rListener)
{
    ::osl::MutexGuard aGuard(rBHelper.rMutex);
    if (!(rBHelper.bDisposed || rBHelper.bInDispose)
        && m_aTopWindowListeners.removeInterface(rListener) == 0
        && m_aFocusListeners.getLength() == 0 && m_bEventListener)
    {
        ::Application::RemoveEventListener(m_aEventListenerLink);
        m_bEventListener = false;
    }
}

// virtual
void SAL_CALL VCLXToolkit::addKeyHandler(
    css::uno::Reference< css::awt::XKeyHandler > const & rHandler)
{
    OSL_ENSURE(rHandler.is(), "Null rHandler");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rHandler->disposing(
            css::lang::EventObject(
                static_cast< ::cppu::OWeakObject * >(this)));
    }
    else if (m_aKeyHandlers.addInterface(rHandler) == 1 && !m_bKeyListener)
    {
        m_bKeyListener = true;
        ::Application::AddKeyListener(m_aKeyListenerLink);
    }
}

// virtual
void SAL_CALL VCLXToolkit::removeKeyHandler(
    css::uno::Reference< css::awt::XKeyHandler > const & rHandler)
{
    ::osl::MutexGuard aGuard(rBHelper.rMutex);
    if (!(rBHelper.bDisposed || rBHelper.bInDispose)
        && m_aKeyHandlers.removeInterface(rHandler) == 0 && m_bKeyListener)
    {
        ::Application::RemoveKeyListener(m_aKeyListenerLink);
        m_bKeyListener = false;
    }
}

// virtual
void SAL_CALL VCLXToolkit::addFocusListener(
    css::uno::Reference< css::awt::XFocusListener > const & rListener)
{
    OSL_ENSURE(rListener.is(), "Null rListener");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rListener->disposing(
            css::lang::EventObject(
                static_cast< ::cppu::OWeakObject * >(this)));
    }
    else if (m_aFocusListeners.addInterface(rListener) == 1
             && !m_bEventListener)
    {
        m_bEventListener = true;
        ::Application::AddEventListener(m_aEventListenerLink);
    }
}

// virtual
void SAL_CALL VCLXToolkit::removeFocusListener(
    css::uno::Reference< css::awt::XFocusListener > const & rListener)
{
    ::osl::MutexGuard aGuard(rBHelper.rMutex);
    if (!(rBHelper.bDisposed || rBHelper.bInDispose)
        && m_aFocusListeners.removeInterface(rListener) == 0
        && m_aTopWindowListeners.getLength() == 0 && m_bEventListener)
    {
        ::Application::RemoveEventListener(m_aEventListenerLink);
        m_bEventListener = false;
    }
}

// virtual
void SAL_CALL VCLXToolkit::fireFocusGained(
    css::uno::Reference<
    css::uno::XInterface > const &)
{
}

// virtual
void SAL_CALL VCLXToolkit::fireFocusLost(
    css::uno::Reference<
    css::uno::XInterface > const &)
{
}


IMPL_LINK(VCLXToolkit, eventListenerHandler, ::VclSimpleEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
    case VclEventId::WindowShow:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowOpened);
        break;
    case VclEventId::WindowHide:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowClosed);
        break;
    case VclEventId::WindowActivate:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowActivated);
        break;
    case VclEventId::WindowDeactivate:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowDeactivated);
        break;
    case VclEventId::WindowClose:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowClosing);
        break;
    case VclEventId::WindowGetFocus:
        callFocusListeners(&rEvent, true);
        break;
    case VclEventId::WindowLoseFocus:
        callFocusListeners(&rEvent, false);
        break;
    case VclEventId::WindowMinimize:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowMinimized);
        break;
    case VclEventId::WindowNormalize:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowNormalized);
        break;
    default: break;
    }
}

IMPL_LINK(VCLXToolkit, keyListenerHandler, ::VclWindowEvent&, rEvent, bool)
{
    switch (rEvent.GetId())
    {
    case VclEventId::WindowKeyInput:
        return callKeyHandlers(&rEvent, true);
    case VclEventId::WindowKeyUp:
        return callKeyHandlers(&rEvent, false);
    default: break;
    }
    return false;
}

void VCLXToolkit::callTopWindowListeners(
    ::VclSimpleEvent const * pEvent,
    void (SAL_CALL css::awt::XTopWindowListener::* pFn)(
        css::lang::EventObject const &))
{
    vcl::Window * pWindow
          = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();
    if (pWindow->IsTopWindow())
    {
        std::vector< css::uno::Reference< css::uno::XInterface > >
              aListeners(m_aTopWindowListeners.getElements());
        if (!aListeners.empty())
        {
            css::lang::EventObject aAwtEvent(
                static_cast< css::awt::XWindow * >(pWindow->GetWindowPeer()));
            for (css::uno::Reference<XInterface> & i : aListeners)
            {
                css::uno::Reference< css::awt::XTopWindowListener >
                      xListener(i, css::uno::UNO_QUERY);
                try
                {
                    (xListener.get()->*pFn)(aAwtEvent);
                }
                catch (const css::uno::RuntimeException &)
                {
                    DBG_UNHANDLED_EXCEPTION("toolkit");
                }
            }
        }
    }
}

bool VCLXToolkit::callKeyHandlers(::VclSimpleEvent const * pEvent,
                                  bool bPressed)
{
    std::vector< css::uno::Reference< css::uno::XInterface > >
          aHandlers(m_aKeyHandlers.getElements());

    if (!aHandlers.empty())
    {
        vcl::Window * pWindow = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();

        // See implementation in vclxwindow.cxx for mapping between VCL and UNO AWT event
        ::KeyEvent * pKeyEvent = static_cast< ::KeyEvent * >(
            static_cast< ::VclWindowEvent const * >(pEvent)->GetData());
        css::awt::KeyEvent aAwtEvent(
            static_cast< css::awt::XWindow * >(pWindow->GetWindowPeer()),
            (pKeyEvent->GetKeyCode().IsShift()
             ? css::awt::KeyModifier::SHIFT : 0)
            | (pKeyEvent->GetKeyCode().IsMod1()
               ? css::awt::KeyModifier::MOD1 : 0)
            | (pKeyEvent->GetKeyCode().IsMod2()
               ? css::awt::KeyModifier::MOD2 : 0)
            | (pKeyEvent->GetKeyCode().IsMod3()
               ? css::awt::KeyModifier::MOD3 : 0),
            pKeyEvent->GetKeyCode().GetCode(), pKeyEvent->GetCharCode(),
            sal::static_int_cast< sal_Int16 >(
                pKeyEvent->GetKeyCode().GetFunction()));
        for (css::uno::Reference<XInterface> & i : aHandlers)
        {
            css::uno::Reference< css::awt::XKeyHandler > xHandler(
                i, css::uno::UNO_QUERY);
            try
            {
                if (bPressed ? xHandler->keyPressed(aAwtEvent)
                             : xHandler->keyReleased(aAwtEvent))
                    return true;
            }
            catch (const css::uno::RuntimeException &)
            {
                DBG_UNHANDLED_EXCEPTION("toolkit");
            }
        }
    }
    return false;
}

void VCLXToolkit::callFocusListeners(::VclSimpleEvent const * pEvent,
                                     bool bGained)
{
    vcl::Window * pWindow
          = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();
    if (pWindow->IsTopWindow())
    {
        std::vector< css::uno::Reference< css::uno::XInterface > >
              aListeners(m_aFocusListeners.getElements());
        if (!aListeners.empty())
        {
            // Ignore the interior of compound controls when determining the
            // window that gets the focus next (see implementation in
            // vclxwindow.cxx for mapping between VCL and UNO AWT event):
            css::uno::Reference< css::uno::XInterface > xNext;
            vcl::Window * pFocus = ::Application::GetFocusWindow();
            for (vcl::Window * p = pFocus; p != nullptr; p = p->GetParent())
                if (!p->IsCompoundControl())
                {
                    pFocus = p;
                    break;
                }
            if (pFocus != nullptr)
                xNext = pFocus->GetComponentInterface();
            css::awt::FocusEvent aAwtEvent(
                static_cast< css::awt::XWindow * >(pWindow->GetWindowPeer()),
                static_cast<sal_Int16>(pWindow->GetGetFocusFlags()),
                xNext, false);
            for (css::uno::Reference<XInterface> & i : aListeners)
            {
                css::uno::Reference< css::awt::XFocusListener > xListener(
                    i, css::uno::UNO_QUERY);
                try
                {
                    bGained ? xListener->focusGained(aAwtEvent)
                        : xListener->focusLost(aAwtEvent);
                }
                catch (const css::uno::RuntimeException &)
                {
                    DBG_UNHANDLED_EXCEPTION("toolkit");
                }
            }
        }
    }
}

// css::awt::XReschedule:

void SAL_CALL VCLXToolkit::reschedule()
{
    SolarMutexGuard aSolarGuard;
    Application::Reschedule(true);
}

// css::awt::XToolkitExperimental

void SAL_CALL VCLXToolkit::processEventsToIdle()
{
    SolarMutexGuard aSolarGuard;
    comphelper::ProfileZone aZone("processEvents");
    Scheduler::ProcessEventsToIdle();
}

sal_Int64 SAL_CALL VCLXToolkit::getOpenGLBufferSwapCounter()
{
#if HAVE_FEATURE_OPENGL
    return OpenGLWrapper::getBufferSwapCounter();
#else
    return 0;
#endif
}

void SAL_CALL VCLXToolkit::setDeterministicScheduling(sal_Bool bDeterministicMode)
{
    SolarMutexGuard aSolarGuard;
    Scheduler::SetDeterministicMode(bDeterministicMode);
}

void SAL_CALL VCLXToolkit::pause(sal_Int32 nMilliseconds)
{
    new Pause(nMilliseconds);
}

void SAL_CALL VCLXToolkit::startRecording()
{
    ::comphelper::ProfileRecording::startRecording(true);
}

void SAL_CALL VCLXToolkit::stopRecording()
{
    ::comphelper::ProfileRecording::startRecording( false );
}

css::uno::Sequence< OUString > VCLXToolkit::getRecordingAndClear()
{
    return ::comphelper::ProfileRecording::getRecordingAndClear();
}

// css:awt:XToolkitRobot

void SAL_CALL VCLXToolkit::keyPress( const css::awt::KeyEvent & aKeyEvent )
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aKeyEvent.Source, css::uno::UNO_QUERY_THROW );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::KeyEvent aVCLKeyEvent = VCLUnoHelper::createVCLKeyEvent( aKeyEvent );
    ::Application::PostKeyEvent( VclEventId::WindowKeyInput, pWindow, &aVCLKeyEvent );
}

void SAL_CALL VCLXToolkit::keyRelease( const css::awt::KeyEvent & aKeyEvent )
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aKeyEvent.Source, css::uno::UNO_QUERY_THROW );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::KeyEvent aVCLKeyEvent = VCLUnoHelper::createVCLKeyEvent( aKeyEvent );
    ::Application::PostKeyEvent( VclEventId::WindowKeyUp, pWindow, &aVCLKeyEvent );
}


void SAL_CALL VCLXToolkit::mousePress( const css::awt::MouseEvent & aMouseEvent )
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY_THROW );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VclEventId::WindowMouseButtonDown, pWindow, &aVCLMouseEvent );
}

void SAL_CALL VCLXToolkit::mouseRelease( const css::awt::MouseEvent & aMouseEvent )
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY_THROW );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VclEventId::WindowMouseButtonUp, pWindow, &aVCLMouseEvent );
}

void SAL_CALL VCLXToolkit::mouseMove( const css::awt::MouseEvent & aMouseEvent )
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY_THROW );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VclEventId::WindowMouseMove, pWindow, &aVCLMouseEvent );
}


}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_VCLXToolkit_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXToolkit());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
