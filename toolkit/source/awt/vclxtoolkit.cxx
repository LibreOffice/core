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


#include <boost/ptr_container/ptr_vector.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <stdio.h>
#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#endif
#include <com/sun/star/awt/ImageScaleMode.hpp>
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
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <osl/conditn.hxx>
#include <rtl/uuid.h>
#include <rtl/process.h>

#ifdef MACOSX
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#ifdef ANDROID
#include <sal/ByteBufferWrapper.hxx>
using org::libreoffice::touch::ByteBufferWrapper;
#endif

#include <vcl/sysdata.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxtabpagecontainer.hxx>
#include <toolkit/awt/vclxtabpagemodel.hxx>

#include <toolkit/awt/animatedimagespeer.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/unowrapper.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/btndlg.hxx>
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
#include "toolkit/awt/vclxspinbutton.hxx"
#include "toolkit/awt/scrollabledialog.hxx"
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/awt/scrollabledialog.hxx>

#define VCLWINDOW_FRAMEWINDOW               0x1000
#define VCLWINDOW_SYSTEMCHILDWINDOW         0x1001

#if (defined WNT)
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32
#elif (defined MACOSX)
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_MAC
#elif (defined UNX)
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW
#endif

TOOLKIT_DLLPUBLIC WinBits ImplGetWinBits( sal_uInt32 nComponentAttribs, sal_uInt16 nCompType )
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

    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::BORDER )
        nWinBits |= WB_BORDER;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::NOBORDER )
        nWinBits |= WB_NOBORDER;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::SIZEABLE )
        nWinBits |= WB_SIZEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::MOVEABLE )
        nWinBits |= WB_MOVEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::CLOSEABLE )
        nWinBits |= WB_CLOSEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::HSCROLL )
        nWinBits |= WB_HSCROLL;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL )
        nWinBits |= WB_VSCROLL;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::LEFT )
        nWinBits |= WB_LEFT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::CENTER )
        nWinBits |= WB_CENTER;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::RIGHT )
        nWinBits |= WB_RIGHT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::SPIN )
        nWinBits |= WB_SPIN;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::SORT )
        nWinBits |= WB_SORT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DROPDOWN )
        nWinBits |= WB_DROPDOWN;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEFBUTTON )
        nWinBits |= WB_DEFBUTTON;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::READONLY )
        nWinBits |= WB_READONLY;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::CLIPCHILDREN )
        nWinBits |= WB_CLIPCHILDREN;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::GROUP )
        nWinBits |= WB_GROUP;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::NOLABEL ) //added for issue79712
        nWinBits |= WB_NOLABEL;

    // These bits are not uniqe
    if ( bMessBox )
    {
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::OK )
            nWinBits |= WB_OK;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::OK_CANCEL )
            nWinBits |= WB_OK_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::YES_NO )
            nWinBits |= WB_YES_NO;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::YES_NO_CANCEL )
            nWinBits |= WB_YES_NO_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::RETRY_CANCEL )
            nWinBits |= WB_RETRY_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_OK )
            nWinBits |= WB_DEF_OK;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_CANCEL )
            nWinBits |= WB_DEF_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_RETRY )
            nWinBits |= WB_DEF_RETRY;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_YES )
            nWinBits |= WB_DEF_YES;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_NO )
            nWinBits |= WB_DEF_NO;
    }
    if ( nCompType == WINDOW_MULTILINEEDIT || nCompType == WINDOW_DIALOG || nCompType == WINDOW_GROUPBOX )
    {
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::AUTOHSCROLL )
            nWinBits |= WB_AUTOHSCROLL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::AUTOVSCROLL )
            nWinBits |= WB_AUTOVSCROLL;
    }


    if ( bDecoratedWindow )
    {
        if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::NODECORATION )
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
    return( strcmp( ((ComponentInfo*)pFirst)->pName,
                    ((ComponentInfo*)pSecond)->pName ) );
}
}

sal_uInt16 ImplGetComponentType( const OUString& rServiceName )
{
    static bool bSorted = false;
    if( !bSorted )
    {
        qsort(  (void*) aComponentInfos,
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

    ComponentInfo* pInf = (ComponentInfo*) bsearch( &aSearch,
                        (void*) aComponentInfos,
                        sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                        sizeof( ComponentInfo ),
                        ComponentInfoCompare );

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
        { css::awt::MessageBoxType_MAKE_FIXED_SIZE, 0, 0 }
    };

    static bool lcl_convertMessageBoxType(
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

//  ----------------------------------------------------
//  class VCLXToolkit
//  ----------------------------------------------------

static sal_Int32                            nVCLToolkitInstanceCount = 0;
static bool                                 bInitedByVCLToolkit = false;

static osl::Mutex & getInitMutex()
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

static osl::Condition & getInitCondition()
{
    static osl::Condition * pC = 0;
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
    VCLXToolkit * pTk = (VCLXToolkit *)pArgs;
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
        catch( com::sun::star::uno::Exception & )
        {
        }
        DeInitVCL();
    }
    else
    {
        JoinMainLoopThread();
    }
}
}

// contructor, which might initialize VCL
VCLXToolkit::VCLXToolkit():
    cppu::WeakComponentImplHelper2<
    ::com::sun::star::awt::XToolkitExperimental,
    ::com::sun::star::lang::XServiceInfo>( GetMutex() ),
    m_aTopWindowListeners(rBHelper.rMutex),
    m_aKeyHandlers(rBHelper.rMutex),
    m_aFocusListeners(rBHelper.rMutex),
    m_aEventListenerLink(LINK(this, VCLXToolkit, eventListenerHandler)),
    m_aKeyListenerLink(LINK(this, VCLXToolkit, keyListenerHandler)),
    m_bEventListener(false),
    m_bKeyListener(false)
{
    hSvToolsLib = NULL;
    fnSvtCreateWindow = NULL;

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
        hSvToolsLib = NULL;
        fnSvtCreateWindow = NULL;
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


::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::getDesktopWindow(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;
    // 07/00: AppWindow doesn't exist anymore...
    return xRef;
}

::com::sun::star::awt::Rectangle VCLXToolkit::getWorkArea(  ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nDisplay = Application::GetDisplayBuiltInScreen();
    Rectangle aWorkRect = Application::GetScreenPosSizePixel( nDisplay );
    com::sun::star::awt::Rectangle aNotherRect;
    aNotherRect.X = aWorkRect.getX();
    aNotherRect.Y = aWorkRect.getY();
    aNotherRect.Width = aWorkRect.getWidth();
    aNotherRect.Height = aWorkRect.getHeight();
    return aNotherRect;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::createWindow( const ::com::sun::star::awt::WindowDescriptor& rDescriptor ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return ImplCreateWindow( rDescriptor, WinBits(0) );
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXToolkit::createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    return createScreenCompatibleDeviceUsingBuffer( Width, Height, 1, 1, 0, 0, 0 );
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXToolkit::createScreenCompatibleDeviceUsingBuffer( sal_Int32 Width, sal_Int32 Height, sal_Int32 ScaleNumerator, sal_Int32 ScaleDenominator, sal_Int32 XOffset, sal_Int32 YOffset, sal_Int64 addressOfMemoryBufferForSharedArrayWrapper ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > xRef;
    VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;

    SolarMutexGuard aSolarGuard;

    VirtualDevice* pV = new VirtualDevice;
    if ( addressOfMemoryBufferForSharedArrayWrapper != 0 ) {
#if defined(ANDROID)
        ByteBufferWrapper *bbw = (ByteBufferWrapper *) (intptr_t) addressOfMemoryBufferForSharedArrayWrapper;
        pV->SetOutputSizePixelScaleOffsetAndBuffer( Size( Width, Height ), Fraction(ScaleNumerator, ScaleDenominator), Point( XOffset, YOffset), basebmp::RawMemorySharedArray( bbw->pointer(), *bbw ));
#else
        pV->SetOutputSizePixelScaleOffsetAndBuffer( Size( Width, Height ), Fraction(ScaleNumerator, ScaleDenominator), Point( XOffset, YOffset), basebmp::RawMemorySharedArray( (sal_uInt8*) (sal_uIntPtr) addressOfMemoryBufferForSharedArrayWrapper ));
#endif
    } else {
        pV->SetOutputSizePixel( Size( Width, Height ) );
    }
    pVDev->SetVirtualDevice( pV );

    xRef = pVDev;
    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion > VCLXToolkit::createRegion(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >  xRef = new VCLXRegion;
    return xRef;
}

Window* VCLXToolkit::ImplCreateWindow( VCLXWindow** ppNewComp,
    const ::com::sun::star::awt::WindowDescriptor& rDescriptor,
    Window* pParent, WinBits nWinBits )
{
    OUString aServiceName( rDescriptor.WindowServiceName );
    aServiceName = aServiceName.toAsciiLowerCase();

    Window* pNewWindow = NULL;
    sal_uInt16 nType = ImplGetComponentType( aServiceName );
    bool bFrameControl = false;
    if ( aServiceName == "frame" )
        bFrameControl = true;
    if ( aServiceName == "tabcontrolnotabs" )
    {
        nWinBits |= WB_NOBORDER;
        nType = ImplGetComponentType( OUString( "tabcontrol" ) );
    }
    if ( !pParent )
    {
        // Wenn die Component einen Parent braucht, dann NULL zurueckgeben,
        // spaeter mal ::com::sun::star::uno::Exception...
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
            if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_TOP )
                bException = false;
        }

        if ( bException )
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }

    if ( nType )
    {
        SolarMutexGuard aVclGuard;
        switch ( (WindowType)nType )
        {
            case WINDOW_CANCELBUTTON:
                pNewWindow = new CancelButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_CHECKBOX:
                 pNewWindow = new CheckBox( pParent, nWinBits );
                *ppNewComp = new VCLXCheckBox;
            break;
            case WINDOW_COMBOBOX:
                pNewWindow = new ComboBox( pParent, nWinBits|WB_AUTOHSCROLL );
                ((ComboBox*)pNewWindow)->EnableAutoSize( false );
                *ppNewComp = new VCLXComboBox;
            break;
            case WINDOW_CURRENCYBOX:
                pNewWindow = new CurrencyBox( pParent, nWinBits );
            break;
            case WINDOW_CURRENCYFIELD:
                pNewWindow = new CurrencyField( pParent, nWinBits );
                static_cast<CurrencyField*>(pNewWindow)->EnableEmptyFieldValue( sal_True );
                *ppNewComp = new VCLXNumericField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(CurrencyField*)pNewWindow );
            break;
            case WINDOW_DATEBOX:
                pNewWindow = new DateBox( pParent, nWinBits );
            break;
            case WINDOW_DATEFIELD:
                pNewWindow = new DateField( pParent, nWinBits );
                static_cast<DateField*>(pNewWindow)->EnableEmptyFieldValue( sal_True );
                *ppNewComp = new VCLXDateField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(DateField*)pNewWindow );
            break;
            case WINDOW_DOCKINGAREA:
                pNewWindow = new DockingAreaWindow( pParent );
            break;
            case WINDOW_MULTILINEEDIT:
            case WINDOW_EDIT:
                pNewWindow = new Edit( pParent, nWinBits );
                *ppNewComp = new VCLXEdit;
            break;
            case WINDOW_ERRORBOX:
                pNewWindow = new ErrorBox( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_FIXEDBITMAP:
                pNewWindow = new FixedBitmap( pParent, nWinBits );
            break;
            case WINDOW_FIXEDIMAGE:
                pNewWindow = new ImageControl( pParent, nWinBits );
                *ppNewComp = new VCLXImageControl;
            break;
            case WINDOW_FIXEDLINE:
                pNewWindow = new FixedLine( pParent, nWinBits );
            break;
            case WINDOW_FIXEDTEXT:
                pNewWindow = new FixedText( pParent, nWinBits );
                *ppNewComp = new VCLXFixedText;
            break;
            case WINDOW_FLOATINGWINDOW:
                pNewWindow = new FloatingWindow( pParent, nWinBits );
            break;
            case WINDOW_GROUPBOX:
                        {
#ifdef SCROLLABLEFRAME
                if ( bFrameControl )
                {
                    pNewWindow = new toolkit::ScrollableWrapper< GroupBox >( pParent, nWinBits | WB_VSCROLL );
                }
                else
#endif
                    pNewWindow = new GroupBox( pParent, nWinBits );
                                if ( bFrameControl )
                                {
                                    GroupBox* pGroupBox =  static_cast< GroupBox* >( pNewWindow );
                                    *ppNewComp = new VCLXFrame;
                                    // Frame control needs to receive
                                    // Mouse events
                                    pGroupBox->SetMouseTransparent( sal_False );
                                }
                        }
            break;
            case WINDOW_HELPBUTTON:
                pNewWindow = new HelpButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGEBUTTON:
                 pNewWindow = new ImageButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_INFOBOX:
                pNewWindow = new InfoBox( pParent, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_LISTBOX:
                pNewWindow = new ListBox( pParent, nWinBits|WB_SIMPLEMODE|WB_AUTOHSCROLL );
                ((ListBox*)pNewWindow)->EnableAutoSize( false );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_LONGCURRENCYBOX:
                pNewWindow = new LongCurrencyBox( pParent, nWinBits );
            break;
            case WINDOW_LONGCURRENCYFIELD:
                pNewWindow = new LongCurrencyField( pParent, nWinBits );
                *ppNewComp = new VCLXCurrencyField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(LongCurrencyField*)pNewWindow );
            break;
            case WINDOW_MENUBUTTON:
                pNewWindow = new MenuButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MESSBOX:
                pNewWindow = new MessBox( pParent, nWinBits, OUString(), OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_METRICBOX:
                pNewWindow = new MetricBox( pParent, nWinBits );
            break;
            case WINDOW_METRICFIELD:
                pNewWindow = new MetricField( pParent, nWinBits );
                *ppNewComp = new VCLXMetricField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(MetricField*)pNewWindow );
            break;
            case WINDOW_DIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
            {
                // Modal/Modeless nur durch Show/Execute
                if ( (pParent == NULL ) && ( rDescriptor.ParentIndex == -1 ) )
                    pParent = DIALOG_NO_PARENT;
                pNewWindow = new toolkit::ScrollableWrapper<Dialog>( pParent, nWinBits );
                // #i70217# Don't always create a new component object. It's possible that VCL has called
                // GetComponentInterface( sal_True ) in the Dialog ctor itself (see Window::IsTopWindow() )
                // which creates a component object.
                css::uno::Reference< css::awt::XWindowPeer > xWinPeer = pNewWindow->GetComponentInterface( sal_False );
                if ( xWinPeer.is() )
                    *ppNewComp = dynamic_cast< VCLXDialog* >( xWinPeer.get() );
                else
                    *ppNewComp = new VCLXDialog;
            }
            break;
            case WINDOW_MOREBUTTON:
                pNewWindow = new MoreButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MULTILISTBOX:
                pNewWindow = new MultiListBox( pParent, nWinBits );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_NUMERICBOX:
                pNewWindow = new NumericBox( pParent, nWinBits );
            break;
            case WINDOW_NUMERICFIELD:
                pNewWindow = new NumericField( pParent, nWinBits );
                static_cast<NumericField*>(pNewWindow)->EnableEmptyFieldValue( sal_True );
                *ppNewComp = new VCLXNumericField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(NumericField*)pNewWindow );
            break;
            case WINDOW_OKBUTTON:
                pNewWindow = new OKButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_PATTERNBOX:
                pNewWindow = new PatternBox( pParent, nWinBits );
            break;
            case WINDOW_PATTERNFIELD:
                pNewWindow = new PatternField( pParent, nWinBits );
                *ppNewComp = new VCLXPatternField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(PatternField*)pNewWindow );
            break;
            case WINDOW_PUSHBUTTON:
                pNewWindow = new PushButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_QUERYBOX:
                pNewWindow = new QueryBox( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_RADIOBUTTON:
                pNewWindow = new RadioButton( pParent, nWinBits );
                *ppNewComp = new VCLXRadioButton;

                // by default, disable RadioCheck
                // Since the VCLXRadioButton really cares for it's RadioCheck settings, this is important:
                // if we enable it, the VCLXRadioButton will use RadioButton::Check instead of RadioButton::SetState
                // This leads to a strange behaviour if the control is newly created: when settings the initial
                // state to "checked", the RadioButton::Check (called because RadioCheck=sal_True) will uncheck
                // _all_other_ radio buttons in the same group. However, at this moment the grouping of the controls
                // is not really valid: the controls are grouped after they have been created, but we're still in
                // the creation process, so the RadioButton::Check relies on invalid grouping information.
                // 07.08.2001 - #87254# - frank.schoenheit@sun.com
                static_cast<RadioButton*>(pNewWindow)->EnableRadioCheck( sal_False );
            break;
            case WINDOW_SCROLLBAR:
                pNewWindow = new ScrollBar( pParent, nWinBits );
                *ppNewComp = new VCLXScrollBar;
            break;
            case WINDOW_SCROLLBARBOX:
                pNewWindow = new ScrollBarBox( pParent, nWinBits );
            break;
            case WINDOW_SPINBUTTON:
                pNewWindow = new SpinButton( pParent, nWinBits );
                *ppNewComp = new ::toolkit::VCLXSpinButton;
            break;
            case WINDOW_SPINFIELD:
                pNewWindow = new SpinField( pParent, nWinBits );
                *ppNewComp = new VCLXNumericField;
            break;
            case WINDOW_SPLITTER:
                pNewWindow = new Splitter( pParent, nWinBits );
            break;
            case WINDOW_SPLITWINDOW:
                pNewWindow = new SplitWindow( pParent, nWinBits );
            break;
            case WINDOW_STATUSBAR:
                pNewWindow = new StatusBar( pParent, nWinBits );
            break;
            case VCLWINDOW_SYSTEMCHILDWINDOW:
                pNewWindow = new SystemChildWindow( pParent, nWinBits );
                *ppNewComp = new VCLXSystemDependentWindow();
            break;
            case WINDOW_TABCONTROL:
                pNewWindow = new TabControl( pParent, nWinBits );
                *ppNewComp = new VCLXMultiPage;
            break;
            case WINDOW_TABDIALOG:
                pNewWindow = new TabDialog( pParent, nWinBits );
            break;
            case WINDOW_TABPAGE:
                {
                    pNewWindow = new TabPage( pParent, nWinBits );
                    *ppNewComp = new VCLXTabPage;
                }
            break;
            case WINDOW_TIMEBOX:
                pNewWindow = new TimeBox( pParent, nWinBits );
            break;
            case WINDOW_TIMEFIELD:
                pNewWindow = new TimeField( pParent, nWinBits );
                static_cast<TimeField*>(pNewWindow)->EnableEmptyFieldValue( sal_True );
                *ppNewComp = new VCLXTimeField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(TimeField*)pNewWindow );
            break;
            case WINDOW_TOOLBOX:
                pNewWindow = new ToolBox( pParent, nWinBits );
                *ppNewComp = new VCLXToolBox;
            break;
            case WINDOW_TRISTATEBOX:
                pNewWindow = new TriStateBox( pParent, nWinBits );
            break;
            case WINDOW_WARNINGBOX:
                pNewWindow = new WarningBox( pParent, nWinBits, OUString() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_WORKWINDOW:
            case WINDOW_WINDOW:
            case VCLWINDOW_FRAMEWINDOW:
            case WINDOW_DOCKINGWINDOW:
                if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_TOP )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                    {
                        if ((pParent == NULL) && rDescriptor.Parent.is())
                        {
                            // try to get a system dependent window handle
                            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSystemDependentWindowPeer > xSystemDepParent(rDescriptor.Parent, ::com::sun::star::uno::UNO_QUERY);

                            if (xSystemDepParent.is())
                            {
                                sal_Int8 processID[16];

                                rtl_getGlobalProcessId( (sal_uInt8*)processID );

                                ::com::sun::star::uno::Sequence<sal_Int8> processIdSeq((sal_Int8*)processID, 16);

                                ::com::sun::star::uno::Any anyHandle = xSystemDepParent->getWindowHandle(processIdSeq, SYSTEM_DEPENDENT_TYPE);

                                // use sal_Int64 here to accommodate all int types
                                // uno::Any shift operator whill upcast if necessary
                                sal_Int64 nWindowHandle = 0;
                                sal_Bool bXEmbed = sal_False;

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
                                    pNewWindow = new WorkWindow( &aParentData );
                                }
                            }
                        }

                        if (!pNewWindow)
                            pNewWindow = new WorkWindow( pParent, nWinBits );
                    }

                    *ppNewComp = new VCLXTopWindow( pNewWindow->GetType() == WINDOW_WORKWINDOW );
                }
                else if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_CONTAINER )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                        pNewWindow = new Window( pParent, nWinBits );
                    *ppNewComp = new VCLXContainer;
                }
                else
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                        pNewWindow = new Window( pParent, nWinBits );
                    *ppNewComp = new VCLXWindow;
                }
            break;
            case WINDOW_CONTROL:
                if ( rDescriptor.WindowServiceName.equalsIgnoreAsciiCase(
                        "tabpagecontainer" ) )
                {
                    pNewWindow = new TabControl( pParent, nWinBits );
                    *ppNewComp = new VCLXTabPageContainer;
                }
                else if ( aServiceName == "animatedimages" )
                {
                    pNewWindow = new Throbber( pParent, nWinBits );
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

extern "C" Window* SAL_CALL CreateWindow( VCLXWindow** ppNewComp, const ::com::sun::star::awt::WindowDescriptor* pDescriptor, Window* pParent, WinBits nWinBits );

#endif

css::uno::Reference< css::awt::XWindowPeer > VCLXToolkit::ImplCreateWindow(
    const css::awt::WindowDescriptor& rDescriptor,
    WinBits nForceWinBits )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    SolarMutexGuard aSolarGuard;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;

    Window* pParent = NULL;
    if ( rDescriptor.Parent.is() )
    {
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( rDescriptor.Parent );

        // #103939# Don't throw assertion, may be it's a system dependend window, used in ImplCreateWindow.
        // DBG_ASSERT( pParentComponent, "ParentComponent not valid" );

        if ( pParentComponent )
            pParent = pParentComponent->GetWindow();
    }
    WinBits nWinBits = ImplGetWinBits( rDescriptor.WindowAttributes,
        ImplGetComponentType( rDescriptor.WindowServiceName ) );
    nWinBits |= nForceWinBits;

    VCLXWindow* pNewComp = NULL;

    Window* pNewWindow = NULL;
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
        OUString aLibName = ::vcl::unohelper::CreateLibraryName(
#ifdef LIBO_MERGELIBS
                                                                       "merged",
#else
                                                                       "svt",
#endif
                                                                       sal_True );
        hSvToolsLib = osl_loadModuleRelative(
            &thisModule, aLibName.pData, SAL_LOADMODULE_DEFAULT );
        if ( hSvToolsLib )
        {
            OUString aFunctionName( "CreateWindow" );
            fnSvtCreateWindow = (FN_SvtCreateWindow)osl_getFunctionSymbol( hSvToolsLib, aFunctionName.pData );
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
    DBG_ASSERTWARNING( pNewComp, "createWindow: No special Interface!" );

    if ( pNewWindow )
    {
        pNewWindow->SetCreatedWithToolkit( sal_True );
        //pNewWindow->SetPosPixel( Point() ); // do not force (0,0) position, keep default pos instead

        if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::MINSIZE )
        {
            pNewWindow->SetSizePixel( Size() );
        }
        else if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::FULLSIZE )
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
            xRef = pNewWindow->GetComponentInterface( sal_True );
        }
        else
        {
            pNewComp->SetCreatedWithToolkit( sal_True );
            xRef = pNewComp;
            pNewWindow->SetComponentInterface( xRef );
        }
        DBG_ASSERT( pNewWindow->GetComponentInterface( sal_False ) == xRef,
            "VCLXToolkit::createWindow: did #133706# resurge?" );

        if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::SHOW )
            pNewWindow->Show();
    }

    return xRef;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > VCLXToolkit::createWindows( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::WindowDescriptor >& rDescriptors ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nComponents = rDescriptors.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > aSeq( nComponents );
    for ( sal_uInt32 n = 0; n < nComponents; n++ )
    {
        ::com::sun::star::awt::WindowDescriptor aDescr = rDescriptors.getConstArray()[n];

        if ( aDescr.ParentIndex == (-1) )
            aDescr.Parent = NULL;
        else if ( ( aDescr.ParentIndex >= 0 ) && ( aDescr.ParentIndex < (short)n ) )
            aDescr.Parent = aSeq.getConstArray()[aDescr.ParentIndex];
        aSeq.getArray()[n] = createWindow( aDescr );
    }
    return aSeq;
}

// ::com::sun::star::awt::XSystemChildFactory
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::createSystemChild( const ::com::sun::star::uno::Any& Parent, const ::com::sun::star::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 nSystemType ) throw(::com::sun::star::uno::RuntimeException)
{
    Window* pChildWindow = NULL;
    if ( nSystemType == SYSTEM_DEPENDENT_TYPE )
    {
        // use sal_Int64 here to accommodate all int types
        // uno::Any shift operator whill upcast if necessary
        sal_Int64 nWindowHandle = 0;
        sal_Bool bXEmbed = sal_False;

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
                pChildWindow = new WorkWindow( &aParentData );
            }
            catch ( const ::com::sun::star::uno::RuntimeException & rEx )
            {
                // system child window could not be created
                OSL_TRACE(
                    "VCLXToolkit::createSystemChild: caught %s\n",
                    OUStringToOString(
                        rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
                pChildWindow = NULL;
            }
        }
    }
    else if (nSystemType == com::sun::star::lang::SystemDependent::SYSTEM_JAVA)
    {
        SolarMutexGuard aGuard;
        pChildWindow = new WorkWindow(0, Parent);
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer;
    if ( pChildWindow )
    {
        VCLXTopWindow* pPeer = new VCLXTopWindow(true);
        SolarMutexGuard aGuard;
        pPeer->SetWindow( pChildWindow );
        xPeer = pPeer;
    }

    return xPeer;
}

// ::com::sun::star::awt::XMessageBoxFactory
::com::sun::star::uno::Reference< ::com::sun::star::awt::XMessageBox > SAL_CALL VCLXToolkit::createMessageBox(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& aParent,
    ::com::sun::star::awt::MessageBoxType eType,
    ::sal_Int32 aButtons,
    const OUString& aTitle,
    const OUString& aMessage ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::WindowDescriptor aDescriptor;

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
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMessageBox > xMsgBox(
        ImplCreateWindow( aDescriptor, nAddWinBits ), css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xWindow( xMsgBox, css::uno::UNO_QUERY );
    if ( xMsgBox.is() && xWindow.is() )
    {
        Window * pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
        {
            SolarMutexGuard aGuard;
            xMsgBox->setCaptionText( aTitle );
            xMsgBox->setMessageText( aMessage );
        }
    }

    return xMsgBox;
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL VCLXToolkit::getDragGestureRecognizer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException)
{
    Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragGestureRecognizer();

    return ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer >();
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > SAL_CALL VCLXToolkit::getDragSource( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException)
{
    Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDragSource();

    return ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource >();
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > SAL_CALL VCLXToolkit::getDropTarget( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException)
{
    Window * pWindow = VCLUnoHelper::GetWindow( window );

    if( pWindow )
        return pWindow->GetDropTarget();

    return ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >();
}

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL VCLXToolkit::getClipboard( const OUString& clipboardName ) throw(::com::sun::star::uno::RuntimeException)
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

    return ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >();
}

// XServiceInfo
OUString VCLXToolkit::getImplementationName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString("stardiv.Toolkit.VCLXToolkit");
}

sal_Bool VCLXToolkit::supportsService( const OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString* pArray = aSNL.getConstArray();
    const OUString* pArrayEnd = aSNL.getConstArray();
    for (; pArray != pArrayEnd; ++pArray )
        if( *pArray == rServiceName )
            break;

    return pArray != pArrayEnd;
}

::com::sun::star::uno::Sequence< OUString > VCLXToolkit::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    OUString aServiceName( OUString::createFromAscii( szServiceName2_Toolkit ) );
    return ::com::sun::star::uno::Sequence< OUString >( &aServiceName, 1);
}

// css::awt::XExtendedToolkit:

// virtual
::sal_Int32 SAL_CALL VCLXToolkit::getTopWindowCount()
    throw (css::uno::RuntimeException)
{
    return static_cast< ::sal_Int32 >(::Application::GetTopWindowCount());
        // XXX  numeric overflow
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getTopWindow(::sal_Int32 nIndex)
    throw (css::uno::RuntimeException)
{
    ::Window * p = ::Application::GetTopWindow(static_cast< long >(nIndex));
        // XXX  numeric overflow
    return css::uno::Reference< css::awt::XTopWindow >(
        p == 0 ? 0 : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
css::uno::Reference< css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getActiveTopWindow() throw (css::uno::RuntimeException)
{
    ::Window * p = ::Application::GetActiveTopWindow();
    return css::uno::Reference< css::awt::XTopWindow >(
        p == 0 ? 0 : static_cast< css::awt::XWindow * >(p->GetWindowPeer()),
        css::uno::UNO_QUERY);
}

// virtual
void SAL_CALL VCLXToolkit::addTopWindowListener(
    css::uno::Reference< css::awt::XTopWindowListener > const & rListener)
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface > const &)
    throw (::com::sun::star::uno::RuntimeException)
{
}

// virtual
void SAL_CALL VCLXToolkit::fireFocusLost(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface > const &)
    throw (::com::sun::star::uno::RuntimeException)
{
}


IMPL_LINK(VCLXToolkit, eventListenerHandler, ::VclSimpleEvent const *, pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_SHOW:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowOpened);
        break;
    case VCLEVENT_WINDOW_HIDE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowClosed);
        break;
    case VCLEVENT_WINDOW_ACTIVATE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowActivated);
        break;
    case VCLEVENT_WINDOW_DEACTIVATE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowDeactivated);
        break;
    case VCLEVENT_WINDOW_CLOSE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowClosing);
        break;
    case VCLEVENT_WINDOW_GETFOCUS:
        callFocusListeners(pEvent, true);
        break;
    case VCLEVENT_WINDOW_LOSEFOCUS:
        callFocusListeners(pEvent, false);
        break;
    case VCLEVENT_WINDOW_MINIMIZE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowMinimized);
        break;
    case VCLEVENT_WINDOW_NORMALIZE:
        callTopWindowListeners(
            pEvent, &css::awt::XTopWindowListener::windowNormalized);
        break;
    }
    return 0;
}

IMPL_LINK(VCLXToolkit, keyListenerHandler, ::VclSimpleEvent const *, pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_KEYINPUT:
        return callKeyHandlers(pEvent, true);
    case VCLEVENT_WINDOW_KEYUP:
        return callKeyHandlers(pEvent, false);
    }
    return 0;
}

void VCLXToolkit::callTopWindowListeners(
    ::VclSimpleEvent const * pEvent,
    void (SAL_CALL css::awt::XTopWindowListener::* pFn)(
        css::lang::EventObject const &))
{
    ::Window * pWindow
          = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();
    if (pWindow->IsTopWindow())
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >
              aListeners(m_aTopWindowListeners.getElements());
        if (aListeners.hasElements())
        {
            css::lang::EventObject aAwtEvent(
                static_cast< css::awt::XWindow * >(pWindow->GetWindowPeer()));
            for (::sal_Int32 i = 0; i < aListeners.getLength(); ++i)
            {
                css::uno::Reference< css::awt::XTopWindowListener >
                      xListener(aListeners[i], css::uno::UNO_QUERY);
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

long VCLXToolkit::callKeyHandlers(::VclSimpleEvent const * pEvent,
                                  bool bPressed)
{
    css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >
          aHandlers(m_aKeyHandlers.getElements());

    if (aHandlers.hasElements())
    {
        ::Window * pWindow = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();

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
        for (::sal_Int32 i = 0; i < aHandlers.getLength(); ++i)
        {
            css::uno::Reference< css::awt::XKeyHandler > xHandler(
                aHandlers[i], css::uno::UNO_QUERY);
            try
            {
                if ((bPressed ? xHandler->keyPressed(aAwtEvent)
                      : xHandler->keyReleased(aAwtEvent)))
                    return 1;
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
    return 0;
}

void VCLXToolkit::callFocusListeners(::VclSimpleEvent const * pEvent,
                                     bool bGained)
{
    ::Window * pWindow
          = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();
    if (pWindow->IsTopWindow())
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >
              aListeners(m_aFocusListeners.getElements());
        if (aListeners.hasElements())
        {
            // Ignore the interior of compound controls when determining the
            // window that gets the focus next (see implementation in
            // vclxwindow.cxx for mapping between VCL and UNO AWT event):
            css::uno::Reference< css::uno::XInterface > xNext;
            ::Window * pFocus = ::Application::GetFocusWindow();
            for (::Window * p = pFocus; p != 0; p = p->GetParent())
                if (!p->IsCompoundControl())
                {
                    pFocus = p;
                    break;
                }
            if (pFocus != 0)
                xNext = pFocus->GetComponentInterface(true);
            css::awt::FocusEvent aAwtEvent(
                static_cast< css::awt::XWindow * >(pWindow->GetWindowPeer()),
                pWindow->GetGetFocusFlags(), xNext, false);
            for (::sal_Int32 i = 0; i < aListeners.getLength(); ++i)
            {
                css::uno::Reference< css::awt::XFocusListener > xListener(
                    aListeners[i], css::uno::UNO_QUERY);
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
    throw (::com::sun::star::uno::RuntimeException)
{
    Application::Reschedule(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
