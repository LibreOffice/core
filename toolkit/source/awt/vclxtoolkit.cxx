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
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>
#include <rtl/process.h>
#include <tools/link.hxx>
#include <tools/fract.hxx>
#include <tools/wintypes.hxx>

#ifdef MACOSX
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <vcl/sysdata.hxx>

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
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/group.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/morebtn.hxx>
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
#include "toolkit/awt/vclxspinbutton.hxx"
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/awt/scrollabledialog.hxx>

#include "helper/unowrapper.hxx"

#define VCLWINDOW_FRAMEWINDOW               0x1000
#define VCLWINDOW_SYSTEMCHILDWINDOW         0x1001

#if defined(_WIN32)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_WIN32
#elif defined(MACOSX)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_MAC
#elif defined(UNX)
#define SYSTEM_DEPENDENT_TYPE css::lang::SystemDependent::SYSTEM_XWINDOW
#endif

namespace {

extern "C" typedef vcl::Window* (SAL_CALL *FN_SvtCreateWindow)(
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
        SetPriority(SchedulerPriority::HIGHEST);
        Start();
    }

    virtual ~Pause() override
    {
    }

    virtual void Invoke() override
    {
        SolarMutexGuard aSolarGuard;
        osl::Thread::wait(std::chrono::milliseconds(m_nPauseMilliseconds));
        Stop();
        delete this;
    }

    sal_Int32 m_nPauseMilliseconds;
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
    ::Link<VclSimpleEvent&,void> m_aEventListenerLink;
    ::Link<VclWindowEvent&,bool> m_aKeyListenerLink;
    bool m_bEventListener;
    bool m_bKeyListener;

    DECL_LINK_TYPED(eventListenerHandler, ::VclSimpleEvent&, void);

    DECL_LINK_TYPED(keyListenerHandler, ::VclWindowEvent&, bool);

    void callTopWindowListeners(
        ::VclSimpleEvent const * pEvent,
        void (SAL_CALL css::awt::XTopWindowListener::* pFn)(
            css::lang::EventObject const &));

    bool callKeyHandlers(::VclSimpleEvent const * pEvent, bool bPressed);

    void callFocusListeners(::VclSimpleEvent const * pEvent, bool bGained);

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }

    virtual void SAL_CALL disposing() override;

    static vcl::Window* ImplCreateWindow( VCLXWindow** ppNewComp, const css::awt::WindowDescriptor& rDescriptor, vcl::Window* pParent, WinBits nWinBits );
    css::uno::Reference< css::awt::XWindowPeer > ImplCreateWindow( const css::awt::WindowDescriptor& Descriptor, WinBits nWinBits );

public:

    VCLXToolkit();
    virtual ~VCLXToolkit() override;

    // css::awt::XToolkitExperimental
    virtual void SAL_CALL processEventsToIdle()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int64 SAL_CALL getOpenGLBufferSwapCounter()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setDeterministicScheduling(sal_Bool bDeterministicMode)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL pause(sal_Int32 nMilliseconds)
        throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XToolkit
    css::uno::Reference< css::awt::XWindowPeer >  SAL_CALL getDesktopWindow(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::awt::Rectangle                                        SAL_CALL getWorkArea(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XWindowPeer >  SAL_CALL createWindow( const css::awt::WindowDescriptor& Descriptor ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > SAL_CALL createWindows( const css::uno::Sequence< css::awt::WindowDescriptor >& Descriptors ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XDevice >      SAL_CALL createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw
(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XRegion >      SAL_CALL createRegion(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XSystemChildFactory
    css::uno::Reference< css::awt::XWindowPeer > SAL_CALL createSystemChild( const css::uno::Any& Parent, const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XMessageBoxFactory
    virtual css::uno::Reference< css::awt::XMessageBox > SAL_CALL createMessageBox( const css::uno::Reference< css::awt::XWindowPeer >& aParent, css::awt::MessageBoxType eType, ::sal_Int32 aButtons, const OUString& aTitle, const OUString& aMessage ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XDataTransfer
    css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL getDragGestureRecognizer( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::datatransfer::dnd::XDragSource > SAL_CALL getDragSource( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::datatransfer::dnd::XDropTarget > SAL_CALL getDropTarget( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const OUString& clipboardName ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XExtendedToolkit:

    virtual ::sal_Int32 SAL_CALL getTopWindowCount()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::awt::XTopWindow >
    SAL_CALL getTopWindow(::sal_Int32 nIndex)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::awt::XTopWindow >
    SAL_CALL getActiveTopWindow()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addTopWindowListener(
        css::uno::Reference<
        css::awt::XTopWindowListener > const & rListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeTopWindowListener(
        css::uno::Reference<
        css::awt::XTopWindowListener > const & rListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addKeyHandler(
        css::uno::Reference<
        css::awt::XKeyHandler > const & rHandler)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeKeyHandler(
        css::uno::Reference<
        css::awt::XKeyHandler > const & rHandler)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addFocusListener(
        css::uno::Reference<
        css::awt::XFocusListener > const & rListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeFocusListener(
        css::uno::Reference<
        css::awt::XFocusListener > const & rListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL fireFocusGained(
        css::uno::Reference<
        css::uno::XInterface > const & source)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL fireFocusLost(
        css::uno::Reference<
        css::uno::XInterface > const & source)
        throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XReschedule:
    virtual void SAL_CALL reschedule()
        throw (css::uno::RuntimeException, std::exception) override;

    // css:awt:XToolkitRobot
    virtual void SAL_CALL keyPress( const css::awt::KeyEvent & aKeyEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL keyRelease( const css::awt::KeyEvent & aKeyEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mousePress( const css::awt::MouseEvent & aMouseEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseRelease( const css::awt::MouseEvent & aMouseEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseMove( const css::awt::MouseEvent & aMouseEvent )
        throw (css::uno::RuntimeException, std::exception) override;

};

WinBits ImplGetWinBits( sal_uInt32 nComponentAttribs, sal_uInt16 nCompType )
{
    WinBits nWinBits = 0;

    bool bMessBox = false;
    if ( ( nCompType == WINDOW_INFOBOX ) ||
         ( nCompType == WINDOW_MESSBOX ) ||
         ( nCompType == WINDOW_QUERYBOX ) ||
         ( nCompType == WINDOW_WARNINGBOX ) ||
         ( nCompType == WINDOW_ERRORBOX ) )
    {
        bMessBox = true;
    }

    bool bDecoratedWindow = false;
    if  (   bMessBox
        ||  ( nCompType == WINDOW_DIALOG )
        ||  ( nCompType == WINDOW_MODELESSDIALOG )
        ||  ( nCompType == WINDOW_MODALDIALOG )
        ||  ( nCompType == WINDOW_SYSTEMDIALOG )
        ||  ( nCompType == WINDOW_PATHDIALOG )
        ||  ( nCompType == WINDOW_FILEDIALOG )
        ||  ( nCompType == WINDOW_PRINTERSETUPDIALOG )
        ||  ( nCompType == WINDOW_PRINTDIALOG )
        ||  ( nCompType == WINDOW_COLORDIALOG )
        ||  ( nCompType == WINDOW_FONTDIALOG )
        ||  ( nCompType == WINDOW_DOCKINGWINDOW )
        ||  ( nCompType == WINDOW_TABDIALOG )
        ||  ( nCompType == WINDOW_BUTTONDIALOG )
        ||  ( nCompType == WINDOW_SYSTEMCHILDWINDOW )
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

    // These bits are not uniqe
    if ( bMessBox )
    {
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::OK )
            nWinBits |= WB_OK;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::OK_CANCEL )
            nWinBits |= WB_OK_CANCEL;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::YES_NO )
            nWinBits |= WB_YES_NO;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::YES_NO_CANCEL )
            nWinBits |= WB_YES_NO_CANCEL;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::RETRY_CANCEL )
            nWinBits |= WB_RETRY_CANCEL;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_OK )
            nWinBits |= WB_DEF_OK;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_CANCEL )
            nWinBits |= WB_DEF_CANCEL;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_RETRY )
            nWinBits |= WB_DEF_RETRY;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_YES )
            nWinBits |= WB_DEF_YES;
        if( nComponentAttribs & css::awt::VclWindowPeerAttribute::DEF_NO )
            nWinBits |= WB_DEF_NO;
    }
    if ( nCompType == WINDOW_MULTILINEEDIT || nCompType == WINDOW_DIALOG || nCompType == WINDOW_GROUPBOX )
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

    return nWinBits;
}

struct ComponentInfo
{
    const char*     pName;
    WindowType      nWinType;
};

static ComponentInfo aComponentInfos [] =
{
    { "buttondialog",       WINDOW_BUTTONDIALOG },
    { "cancelbutton",       WINDOW_CANCELBUTTON },
    { "checkbox",           WINDOW_CHECKBOX },
    { "combobox",           WINDOW_COMBOBOX },
    { "control",            WINDOW_CONTROL },
    { "currencybox",        WINDOW_CURRENCYBOX },
    { "currencyfield",      WINDOW_CURRENCYFIELD },
    { "datebox",            WINDOW_DATEBOX },
    { "datefield",          WINDOW_DATEFIELD },
    { "dialog",             WINDOW_DIALOG },
    { "dockingarea",        WINDOW_DOCKINGAREA },
    { "dockingwindow",      WINDOW_DOCKINGWINDOW },
    { "edit",               WINDOW_EDIT },
    { "errorbox",           WINDOW_ERRORBOX },
    { "fixedbitmap",        WINDOW_FIXEDBITMAP },
    { "fixedimage",         WINDOW_FIXEDIMAGE },
    { "fixedline",          WINDOW_FIXEDLINE },
    { "fixedtext",          WINDOW_FIXEDTEXT },
    { "floatingwindow",     WINDOW_FLOATINGWINDOW },
    { "framewindow",        VCLWINDOW_FRAMEWINDOW },
    { "groupbox",           WINDOW_GROUPBOX },
    { "frame",          WINDOW_GROUPBOX },
    { "helpbutton",         WINDOW_HELPBUTTON },
    { "imagebutton",        WINDOW_IMAGEBUTTON },
    { "infobox",            WINDOW_INFOBOX },
    { "listbox",            WINDOW_LISTBOX },
    { "longcurrencybox",    WINDOW_LONGCURRENCYBOX },
    { "longcurrencyfield",  WINDOW_LONGCURRENCYFIELD },
    { "menubutton",         WINDOW_MENUBUTTON },
    { "messbox",            WINDOW_MESSBOX },
    { "metricbox",          WINDOW_METRICBOX },
    { "metricfield",        WINDOW_METRICFIELD },
    { "modaldialog",        WINDOW_MODALDIALOG },
    { "modelessdialog",     WINDOW_MODELESSDIALOG },
    { "morebutton",         WINDOW_MOREBUTTON },
    { "multilineedit",      WINDOW_MULTILINEEDIT },
    { "multilistbox",       WINDOW_MULTILISTBOX },
    { "numericbox",         WINDOW_NUMERICBOX },
    { "numericfield",       WINDOW_NUMERICFIELD },
    { "okbutton",           WINDOW_OKBUTTON },
    { "patternbox",         WINDOW_PATTERNBOX },
    { "patternfield",       WINDOW_PATTERNFIELD },
    { "pushbutton",         WINDOW_PUSHBUTTON },
    { "querybox",           WINDOW_QUERYBOX },
    { "radiobutton",        WINDOW_RADIOBUTTON },
    { "scrollbar",          WINDOW_SCROLLBAR },
    { "scrollbarbox",       WINDOW_SCROLLBARBOX },
    { "animatedimages",     WINDOW_CONTROL },
    { "spinbutton",         WINDOW_SPINBUTTON },
    { "spinfield",          WINDOW_SPINFIELD },
    { "splitter",           WINDOW_SPLITTER },
    { "splitwindow",        WINDOW_SPLITWINDOW },
    { "statusbar",          WINDOW_STATUSBAR },
    { "systemchildwindow",  VCLWINDOW_SYSTEMCHILDWINDOW },
    { "tabcontrol",         WINDOW_TABCONTROL },
    { "tabdialog",          WINDOW_TABDIALOG },
    { "tabpage",            WINDOW_TABPAGE },
    { "timebox",            WINDOW_TIMEBOX },
    { "timefield",          WINDOW_TIMEFIELD },
    { "toolbox",            WINDOW_TOOLBOX },
    { "tristatebox",        WINDOW_TRISTATEBOX },
    { "warningbox",         WINDOW_WARNINGBOX },
    { "window",             WINDOW_WINDOW },
    { "workwindow",         WINDOW_WORKWINDOW },
    { "tabpagecontainer",   WINDOW_CONTROL },
    { "tabpagemodel",       WINDOW_TABPAGE }
};

extern "C"
{
static int SAL_CALL ComponentInfoCompare( const void* pFirst, const void* pSecond)
{
    return( strcmp( static_cast<ComponentInfo const *>(pFirst)->pName,
                    static_cast<ComponentInfo const *>(pSecond)->pName ) );
}
}

sal_uInt16 ImplGetComponentType( const OUString& rServiceName )
{
    static bool bSorted = false;
    if( !bSorted )
    {
        qsort(  static_cast<void*>(aComponentInfos),
                sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                sizeof( ComponentInfo ),
                ComponentInfoCompare );
        bSorted = true;
    }


    ComponentInfo aSearch;
    OString aServiceName(OUStringToOString(rServiceName, osl_getThreadTextEncoding()).toAsciiLowerCase());
    if ( !aServiceName.isEmpty() )
        aSearch.pName = aServiceName.getStr();
    else
        aSearch.pName = "window";

    ComponentInfo* pInf = static_cast<ComponentInfo*>(bsearch( &aSearch,
                        static_cast<void*>(aComponentInfos),
                        sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                        sizeof( ComponentInfo ),
                        ComponentInfoCompare ));

    return pInf ? pInf->nWinType : 0;
}


namespace
{
    struct MessageBoxTypeInfo
    {
        css::awt::MessageBoxType eType;
        const sal_Char          *pName;
        sal_Int32                nLen;
    };

    static const MessageBoxTypeInfo aMessageBoxTypeInfo[] =
    {
        { css::awt::MessageBoxType_MESSAGEBOX,      RTL_CONSTASCII_STRINGPARAM("messbox") },
        { css::awt::MessageBoxType_INFOBOX,         RTL_CONSTASCII_STRINGPARAM("infobox") },
        { css::awt::MessageBoxType_WARNINGBOX,      RTL_CONSTASCII_STRINGPARAM("warningbox") },
        { css::awt::MessageBoxType_ERRORBOX,        RTL_CONSTASCII_STRINGPARAM("errorbox") },
        { css::awt::MessageBoxType_QUERYBOX,        RTL_CONSTASCII_STRINGPARAM("querybox") },
        { css::awt::MessageBoxType_MAKE_FIXED_SIZE, nullptr, 0 }
    };

    bool lcl_convertMessageBoxType(
        rtl::OUString &sType,
        css::awt::MessageBoxType eType )
    {
        const MessageBoxTypeInfo *pMap = aMessageBoxTypeInfo;
        css::awt::MessageBoxType eVal = css::awt::MessageBoxType_MAKE_FIXED_SIZE;

        while ( pMap->pName )
        {
            if ( pMap->eType == eType )
            {
                eVal = eType;
                sType = rtl::OUString( pMap->pName, pMap->nLen, RTL_TEXTENCODING_ASCII_US );
                break;
            }
            pMap++;
        }

        return ( eVal != css::awt::MessageBoxType_MAKE_FIXED_SIZE );
    }
}

static sal_Int32                            nVCLToolkitInstanceCount = 0;
static bool                                 bInitedByVCLToolkit = false;

osl::Mutex & getInitMutex()
{
    static osl::Mutex * pM;
    if( !pM )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pM )
        {
            static osl::Mutex aMutex;
            pM = &aMutex;
        }
    }
    return *pM;
}

osl::Condition & getInitCondition()
{
    static osl::Condition * pC = nullptr;
    if( !pC )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pC )
        {
            static osl::Condition aCondition;
            pC = &aCondition;
        }
    }
    return *pC;
}

extern "C"
{
static void SAL_CALL ToolkitWorkerFunction( void* pArgs )
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
        Application::SetUnoWrapper( pUnoWrapper );
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

VCLXToolkit::~VCLXToolkit()
{
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


css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::getDesktopWindow(  ) throw(css::uno::RuntimeException, std::exception)
{
    css::uno::Reference< css::awt::XWindowPeer > xRef;
    // 07/00: AppWindow doesn't exist anymore...
    return xRef;
}

css::awt::Rectangle VCLXToolkit::getWorkArea(  ) throw(css::uno::RuntimeException, std::exception)
{
    sal_Int32 nDisplay = Application::GetDisplayBuiltInScreen();
    Rectangle aWorkRect = Application::GetScreenPosSizePixel( nDisplay );
    css::awt::Rectangle aNotherRect;
    aNotherRect.X = aWorkRect.getX();
    aNotherRect.Y = aWorkRect.getY();
    aNotherRect.Width = aWorkRect.getWidth();
    aNotherRect.Height = aWorkRect.getHeight();
    return aNotherRect;
}

css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::createWindow( const css::awt::WindowDescriptor& rDescriptor ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    return ImplCreateWindow( rDescriptor, WinBits(0) );
}

css::uno::Reference< css::awt::XDevice > VCLXToolkit::createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw(css::uno::RuntimeException, std::exception)
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

css::uno::Reference< css::awt::XRegion > VCLXToolkit::createRegion(  ) throw(css::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    css::uno::Reference< css::awt::XRegion >  xRef = new VCLXRegion;
    return xRef;
}

vcl::Window* VCLXToolkit::ImplCreateWindow( VCLXWindow** ppNewComp,
    const css::awt::WindowDescriptor& rDescriptor,
    vcl::Window* pParent, WinBits nWinBits )
{
    OUString aServiceName( rDescriptor.WindowServiceName );
    aServiceName = aServiceName.toAsciiLowerCase();

    vcl::Window* pNewWindow = nullptr;
    sal_uInt16 nType = ImplGetComponentType( aServiceName );
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
        // Wenn die Component einen Parent braucht, dann NULL zurueckgeben,
        // spaeter mal css::uno::Exception...
        bool bException = true;
        if  (   ( nType == WINDOW_DIALOG )
            ||  ( nType == WINDOW_MODALDIALOG )
            ||  ( nType == WINDOW_MODELESSDIALOG )
            ||  ( nType == WINDOW_MESSBOX )
            ||  ( nType == WINDOW_INFOBOX )
            ||  ( nType == WINDOW_WARNINGBOX )
            ||  ( nType == WINDOW_ERRORBOX )
            ||  ( nType == WINDOW_QUERYBOX )
            )
            bException = false;
        else if ( ( nType == WINDOW_WINDOW ) ||
                  ( nType == WINDOW_WORKWINDOW ) ||
                  ( nType == VCLWINDOW_FRAMEWINDOW ) )
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

    if ( nType )
    {
        SolarMutexGuard aVclGuard;
        switch ( (WindowType)nType )
        {
            case WINDOW_CANCELBUTTON:
                pNewWindow = VclPtr<CancelButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_CHECKBOX:
                 pNewWindow = VclPtr<CheckBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXCheckBox;
            break;
            case WINDOW_COMBOBOX:
                pNewWindow = VclPtr<ComboBox>::Create( pParent, nWinBits|WB_AUTOHSCROLL );
                static_cast<ComboBox*>(pNewWindow)->EnableAutoSize( false );
                *ppNewComp = new VCLXComboBox;
            break;
            case WINDOW_CURRENCYBOX:
                pNewWindow = VclPtr<CurrencyBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_CURRENCYFIELD:
                pNewWindow = VclPtr<CurrencyField>::Create( pParent, nWinBits );
                static_cast<CurrencyField*>(pNewWindow)->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXNumericField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<CurrencyField*>(pNewWindow)) );
            break;
            case WINDOW_DATEBOX:
                pNewWindow = VclPtr<DateBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_DATEFIELD:
                pNewWindow = VclPtr<DateField>::Create( pParent, nWinBits );
                static_cast<DateField*>(pNewWindow)->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXDateField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<DateField*>(pNewWindow)) );
            break;
            case WINDOW_DOCKINGAREA:
                pNewWindow = VclPtr<DockingAreaWindow>::Create( pParent );
            break;
            case WINDOW_MULTILINEEDIT:
            case WINDOW_EDIT:
                pNewWindow = VclPtr<Edit>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXEdit;
            break;
            case WINDOW_ERRORBOX:
                pNewWindow = VclPtr<ErrorBox>::Create( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_FIXEDBITMAP:
                pNewWindow = VclPtr<FixedBitmap>::Create( pParent, nWinBits );
            break;
            case WINDOW_FIXEDIMAGE:
                pNewWindow = VclPtr<ImageControl>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXImageControl;
            break;
            case WINDOW_FIXEDLINE:
                pNewWindow = VclPtr<FixedLine>::Create( pParent, nWinBits );
            break;
            case WINDOW_FIXEDTEXT:
                pNewWindow = VclPtr<FixedText>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXFixedText;
            break;
            case WINDOW_FLOATINGWINDOW:
                pNewWindow = VclPtr<FloatingWindow>::Create( pParent, nWinBits );
            break;
            case WINDOW_GROUPBOX:
                pNewWindow = VclPtr<GroupBox>::Create( pParent, nWinBits );
                if ( bFrameControl )
                {
                    GroupBox* pGroupBox =  static_cast< GroupBox* >( pNewWindow );
                    *ppNewComp = new VCLXFrame;
                    // Frame control needs to receive
                    // Mouse events
                pGroupBox->SetMouseTransparent( false );
                }
            break;
            case WINDOW_HELPBUTTON:
                pNewWindow = VclPtr<HelpButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGEBUTTON:
                 pNewWindow = VclPtr<ImageButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_INFOBOX:
                pNewWindow = VclPtr<InfoBox>::Create( pParent, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_LISTBOX:
                pNewWindow = VclPtr<ListBox>::Create( pParent, nWinBits|WB_SIMPLEMODE|WB_AUTOHSCROLL );
                static_cast<ListBox*>(pNewWindow)->EnableAutoSize( false );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_LONGCURRENCYBOX:
                pNewWindow = VclPtr<LongCurrencyBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_LONGCURRENCYFIELD:
                pNewWindow = VclPtr<LongCurrencyField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXCurrencyField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<LongCurrencyField*>(pNewWindow)) );
            break;
            case WINDOW_MENUBUTTON:
                pNewWindow = VclPtr<MenuButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MESSBOX:
                pNewWindow = VclPtr<MessBox>::Create( pParent, nWinBits, OUString(), OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_METRICBOX:
                pNewWindow = VclPtr<MetricBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_METRICFIELD:
                pNewWindow = VclPtr<MetricField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXMetricField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<MetricField*>(pNewWindow)) );
            break;
            case WINDOW_DIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
            {
                // Modal/Modeless nur durch Show/Execute
                if ( (pParent == nullptr ) && ( rDescriptor.ParentIndex == -1 ) )
                    pNewWindow = VclPtr<toolkit::ScrollableWrapper<Dialog>>::Create( nullptr, nWinBits, Dialog::InitFlag::NoParent );
                else
                    pNewWindow = VclPtr<toolkit::ScrollableWrapper<Dialog>>::Create( pParent, nWinBits );
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
            case WINDOW_MOREBUTTON:
                pNewWindow = VclPtr<MoreButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MULTILISTBOX:
                pNewWindow = VclPtr<MultiListBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_NUMERICBOX:
                pNewWindow = VclPtr<NumericBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_NUMERICFIELD:
                pNewWindow = VclPtr<NumericField>::Create( pParent, nWinBits );
                static_cast<NumericField*>(pNewWindow)->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXNumericField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<NumericField*>(pNewWindow)) );
            break;
            case WINDOW_OKBUTTON:
                pNewWindow = VclPtr<OKButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_PATTERNBOX:
                pNewWindow = VclPtr<PatternBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_PATTERNFIELD:
                pNewWindow = VclPtr<PatternField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXPatternField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<PatternField*>(pNewWindow)) );
            break;
            case WINDOW_PUSHBUTTON:
                pNewWindow = VclPtr<PushButton>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_QUERYBOX:
                pNewWindow = VclPtr<QueryBox>::Create( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_RADIOBUTTON:
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
                static_cast<RadioButton*>(pNewWindow)->EnableRadioCheck( false );
            break;
            case WINDOW_SCROLLBAR:
                pNewWindow = VclPtr<ScrollBar>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXScrollBar;
            break;
            case WINDOW_SCROLLBARBOX:
                pNewWindow = VclPtr<ScrollBarBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_SPINBUTTON:
                pNewWindow = VclPtr<SpinButton>::Create( pParent, nWinBits );
                *ppNewComp = new ::toolkit::VCLXSpinButton;
            break;
            case WINDOW_SPINFIELD:
                pNewWindow = VclPtr<SpinField>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXNumericField;
            break;
            case WINDOW_SPLITTER:
                pNewWindow = VclPtr<Splitter>::Create( pParent, nWinBits );
            break;
            case WINDOW_SPLITWINDOW:
                pNewWindow = VclPtr<SplitWindow>::Create( pParent, nWinBits );
            break;
            case WINDOW_STATUSBAR:
                pNewWindow = VclPtr<StatusBar>::Create( pParent, nWinBits );
            break;
            case VCLWINDOW_SYSTEMCHILDWINDOW:
                pNewWindow = VclPtr<SystemChildWindow>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXSystemDependentWindow();
            break;
            case WINDOW_TABCONTROL:
                pNewWindow = VclPtr<TabControl>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXMultiPage;
            break;
            case WINDOW_TABDIALOG:
                pNewWindow = VclPtr<TabDialog>::Create( pParent, nWinBits );
            break;
            case WINDOW_TABPAGE:
                {
                    pNewWindow = VclPtr<TabPage>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXTabPage;
                }
            break;
            case WINDOW_TIMEBOX:
                pNewWindow = VclPtr<TimeBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_TIMEFIELD:
                pNewWindow = VclPtr<TimeField>::Create( pParent, nWinBits );
                static_cast<TimeField*>(pNewWindow)->EnableEmptyFieldValue( true );
                *ppNewComp = new VCLXTimeField;
                static_cast<VCLXFormattedSpinField*>(*ppNewComp)->SetFormatter( static_cast<FormatterBase*>(static_cast<TimeField*>(pNewWindow)) );
            break;
            case WINDOW_TOOLBOX:
                pNewWindow = VclPtr<ToolBox>::Create( pParent, nWinBits );
                *ppNewComp = new VCLXToolBox;
            break;
            case WINDOW_TRISTATEBOX:
                pNewWindow = VclPtr<TriStateBox>::Create( pParent, nWinBits );
            break;
            case WINDOW_WARNINGBOX:
                pNewWindow = VclPtr<WarningBox>::Create( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_WORKWINDOW:
            case WINDOW_WINDOW:
            case VCLWINDOW_FRAMEWINDOW:
            case WINDOW_DOCKINGWINDOW:
                if ( rDescriptor.Type == css::awt::WindowClass_TOP )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
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

                    *ppNewComp = new VCLXTopWindow( pNewWindow->GetType() == WINDOW_WORKWINDOW );
                }
                else if ( rDescriptor.Type == css::awt::WindowClass_CONTAINER )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = VclPtr<DockingWindow>::Create( pParent, nWinBits );
                    else
                        pNewWindow = VclPtr<vcl::Window>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXContainer;
                }
                else
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = VclPtr<DockingWindow>::Create( pParent, nWinBits );
                    else
                        pNewWindow = VclPtr<vcl::Window>::Create( pParent, nWinBits );
                    *ppNewComp = new VCLXWindow;
                }
            break;
            case WINDOW_CONTROL:
                if ( rDescriptor.WindowServiceName.equalsIgnoreAsciiCase(
                        "tabpagecontainer" ) )
                {
                    pNewWindow = VclPtr<TabControl>::Create( pParent, nWinBits );
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

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" vcl::Window* SAL_CALL CreateWindow( VCLXWindow** ppNewComp, const css::awt::WindowDescriptor* pDescriptor, vcl::Window* pParent, WinBits nWinBits );

#endif

css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::ImplCreateWindow(
    const css::awt::WindowDescriptor& rDescriptor,
    WinBits nForceWinBits )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SolarMutexGuard aSolarGuard;

    css::uno::Reference< css::awt::XWindowPeer > xRef;

    vcl::Window* pParent = nullptr;
    if ( rDescriptor.Parent.is() )
    {
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( rDescriptor.Parent );

        // #103939# Don't throw assertion, may be it's a system dependent window, used in ImplCreateWindow.
        // DBG_ASSERT( pParentComponent, "ParentComponent not valid" );

        if ( pParentComponent )
            pParent = pParentComponent->GetWindow();
    }
    WinBits nWinBits = ImplGetWinBits( rDescriptor.WindowAttributes,
        ImplGetComponentType( rDescriptor.WindowServiceName ) );
    nWinBits |= nForceWinBits;

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
        pNewWindow = ImplCreateWindow( &pNewComp, rDescriptor, pParent, nWinBits );

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
            Rectangle aRect = VCLRectangle( rDescriptor.Bounds );
            pNewWindow->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        }

        if ( !pNewComp )
        {
            // Default-Interface
            xRef = pNewWindow->GetComponentInterface();
        }
        else
        {
            pNewComp->SetCreatedWithToolkit( true );
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

css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > VCLXToolkit::createWindows( const css::uno::Sequence< css::awt::WindowDescriptor >& rDescriptors ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nComponents = rDescriptors.getLength();
    css::uno::Sequence< css::uno::Reference< css::awt::XWindowPeer > > aSeq( nComponents );
    for ( sal_uInt32 n = 0; n < nComponents; n++ )
    {
        css::awt::WindowDescriptor aDescr = rDescriptors.getConstArray()[n];

        if ( aDescr.ParentIndex == (-1) )
            aDescr.Parent = nullptr;
        else if ( ( aDescr.ParentIndex >= 0 ) && ( aDescr.ParentIndex < (short)n ) )
            aDescr.Parent = aSeq.getConstArray()[aDescr.ParentIndex];
        aSeq.getArray()[n] = createWindow( aDescr );
    }
    return aSeq;
}

// css::awt::XSystemChildFactory
css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::createSystemChild( const css::uno::Any& Parent, const css::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 nSystemType ) throw(css::uno::RuntimeException, std::exception)
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
            catch ( const css::uno::RuntimeException & rEx )
            {
                // system child window could not be created
                OSL_TRACE(
                    "VCLXToolkit::createSystemChild: caught %s\n",
                    OUStringToOString(
                        rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
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
        VCLXTopWindow* pPeer = new VCLXTopWindow(true);
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
    const OUString& aMessage ) throw (css::uno::RuntimeException, std::exception)
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
    WinBits nAddWinBits( 0 );
    if (( aButtons & 0x0000ffffL ) == css::awt::MessageBoxButtons::BUTTONS_ABORT_IGNORE_RETRY )
        nAddWinBits |= WB_ABORT_RETRY_IGNORE;
    if ( sal_Int32( aButtons & 0xffff0000L ) == css::awt::MessageBoxButtons::DEFAULT_BUTTON_IGNORE )
        nAddWinBits |= WB_DEF_IGNORE;

    rtl::OUString aType;
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
        vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
        {
            SolarMutexGuard aGuard;
            xMsgBox->setCaptionText( aTitle );
            xMsgBox->setMessageText( aMessage );
        }
    }

    return xMsgBox;
}

css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL VCLXToolkit::getDragGestureRecognizer( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragGestureRecognizer();

    return css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer >();
}

css::uno::Reference< css::datatransfer::dnd::XDragSource > SAL_CALL VCLXToolkit::getDragSource( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragSource();

    return css::uno::Reference< css::datatransfer::dnd::XDragSource >();
}

css::uno::Reference< css::datatransfer::dnd::XDropTarget > SAL_CALL VCLXToolkit::getDropTarget( const css::uno::Reference< css::awt::XWindow >& window ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDropTarget();

    return css::uno::Reference< css::datatransfer::dnd::XDropTarget >();
}

css::uno::Reference< css::datatransfer::clipboard::XClipboard > SAL_CALL VCLXToolkit::getClipboard( const OUString& clipboardName ) throw(css::uno::RuntimeException, std::exception)
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
OUString VCLXToolkit::getImplementationName() throw(css::uno::RuntimeException, std::exception)
{
    return OUString("stardiv.Toolkit.VCLXToolkit");
}

sal_Bool VCLXToolkit::supportsService( const OUString& rServiceName ) throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > VCLXToolkit::getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{
        "com.sun.star.awt.Toolkit", "stardiv.vcl.VclToolkit"};
}

// css::awt::XExtendedToolkit:

// virtual
::sal_Int32 SAL_CALL VCLXToolkit::getTopWindowCount()
    throw (css::uno::RuntimeException, std::exception)
{
    return static_cast< ::sal_Int32 >(::Application::GetTopWindowCount());
        // XXX  numeric overflow
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getTopWindow(::sal_Int32 nIndex)
    throw (css::uno::RuntimeException, std::exception)
{
    vcl::Window * p = ::Application::GetTopWindow(static_cast< long >(nIndex));
        // XXX  numeric overflow
    return css::uno::Reference< css::awt::XTopWindow >(
        p == nullptr ? nullptr : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getActiveTopWindow() throw (css::uno::RuntimeException, std::exception)
{
    vcl::Window * p = ::Application::GetActiveTopWindow();
    return css::uno::Reference< css::awt::XTopWindow >(
        p == nullptr ? nullptr : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
void SAL_CALL VCLXToolkit::addTopWindowListener(
    css::uno::Reference< css::awt::XTopWindowListener > const & rListener)
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
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
    throw (css::uno::RuntimeException, std::exception)
{
}

// virtual
void SAL_CALL VCLXToolkit::fireFocusLost(
    css::uno::Reference<
    css::uno::XInterface > const &)
    throw (css::uno::RuntimeException, std::exception)
{
}


IMPL_LINK_TYPED(VCLXToolkit, eventListenerHandler, ::VclSimpleEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
    case VCLEVENT_WINDOW_SHOW:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowOpened);
        break;
    case VCLEVENT_WINDOW_HIDE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowClosed);
        break;
    case VCLEVENT_WINDOW_ACTIVATE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowActivated);
        break;
    case VCLEVENT_WINDOW_DEACTIVATE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowDeactivated);
        break;
    case VCLEVENT_WINDOW_CLOSE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowClosing);
        break;
    case VCLEVENT_WINDOW_GETFOCUS:
        callFocusListeners(&rEvent, true);
        break;
    case VCLEVENT_WINDOW_LOSEFOCUS:
        callFocusListeners(&rEvent, false);
        break;
    case VCLEVENT_WINDOW_MINIMIZE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowMinimized);
        break;
    case VCLEVENT_WINDOW_NORMALIZE:
        callTopWindowListeners(
            &rEvent, &css::awt::XTopWindowListener::windowNormalized);
        break;
    }
}

IMPL_LINK_TYPED(VCLXToolkit, keyListenerHandler, ::VclWindowEvent&, rEvent, bool)
{
    switch (rEvent.GetId())
    {
    case VCLEVENT_WINDOW_KEYINPUT:
        return callKeyHandlers(&rEvent, true);
    case VCLEVENT_WINDOW_KEYUP:
        return callKeyHandlers(&rEvent, false);
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
                catch (const css::uno::RuntimeException & rEx)
                {
                    OSL_TRACE(
                        "VCLXToolkit::callTopWindowListeners: caught %s\n",
                        OUStringToOString(
                            rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
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
                if ((bPressed ? xHandler->keyPressed(aAwtEvent)
                      : xHandler->keyReleased(aAwtEvent)))
                    return true;
            }
            catch (const css::uno::RuntimeException & rEx)
            {
                OSL_TRACE(
                    "VCLXToolkit::callKeyHandlers: caught %s\n",
                    OUStringToOString(
                       rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
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
                catch (const css::uno::RuntimeException & rEx)
                {
                    OSL_TRACE(
                        "VCLXToolkit::callFocusListeners: caught %s\n",
                        OUStringToOString(
                            rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
    }
}

// css::awt::XReschedule:

void SAL_CALL VCLXToolkit::reschedule()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    Application::Reschedule(true);
}

// css::awt::XToolkitExperimental

void SAL_CALL VCLXToolkit::processEventsToIdle()
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    Scheduler::ProcessAllPendingEvents();
}

sal_Int64 SAL_CALL VCLXToolkit::getOpenGLBufferSwapCounter()
    throw (css::uno::RuntimeException, std::exception)
{
#if HAVE_FEATURE_OPENGL
    return OpenGLWrapper::getBufferSwapCounter();
#else
    return 0;
#endif
}

void SAL_CALL VCLXToolkit::setDeterministicScheduling(sal_Bool bDeterministicMode)
    throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    Scheduler::SetDeterministicMode(bDeterministicMode);
}

void SAL_CALL VCLXToolkit::pause(sal_Int32 nMilliseconds)
    throw (css::uno::RuntimeException, std::exception)
{
    new Pause(nMilliseconds);
}

// css:awt:XToolkitRobot

void SAL_CALL VCLXToolkit::keyPress( const css::awt::KeyEvent & aKeyEvent )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aKeyEvent.Source, css::uno::UNO_QUERY );
    if( !xWindow.is() )
        throw css::uno::RuntimeException( "invalid event source" );

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::KeyEvent aVCLKeyEvent = VCLUnoHelper::createVCLKeyEvent( aKeyEvent );
    ::Application::PostKeyEvent( VCLEVENT_WINDOW_KEYINPUT, pWindow, &aVCLKeyEvent );
}

void SAL_CALL VCLXToolkit::keyRelease( const css::awt::KeyEvent & aKeyEvent )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aKeyEvent.Source, css::uno::UNO_QUERY );
    if( !xWindow.is() )
        throw css::uno::RuntimeException( "invalid event source" );

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::KeyEvent aVCLKeyEvent = VCLUnoHelper::createVCLKeyEvent( aKeyEvent );
    ::Application::PostKeyEvent( VCLEVENT_WINDOW_KEYUP, pWindow, &aVCLKeyEvent );
}


void SAL_CALL VCLXToolkit::mousePress( const css::awt::MouseEvent & aMouseEvent )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY );
    if( !xWindow.is() )
        throw css::uno::RuntimeException( "invalid event source" );

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pWindow, &aVCLMouseEvent );
}

void SAL_CALL VCLXToolkit::mouseRelease( const css::awt::MouseEvent & aMouseEvent )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY );
    if( !xWindow.is() )
        throw css::uno::RuntimeException( "invalid event source" );

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, pWindow, &aVCLMouseEvent );
}

void SAL_CALL VCLXToolkit::mouseMove( const css::awt::MouseEvent & aMouseEvent )
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::awt::XWindow> xWindow ( aMouseEvent.Source, css::uno::UNO_QUERY );
    if( !xWindow.is() )
        throw css::uno::RuntimeException( "invalid event source" );

    vcl::Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
    if( !pWindow )
        throw css::uno::RuntimeException( "invalid event source" );

    ::MouseEvent aVCLMouseEvent = VCLUnoHelper::createVCLMouseEvent( aMouseEvent );
    ::Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, pWindow, &aVCLMouseEvent );
}


}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_VCLXToolkit_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXToolkit());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
