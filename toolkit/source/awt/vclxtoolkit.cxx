/*************************************************************************
 *
 *  $RCSfile: vclxtoolkit.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 07:43:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _SVWIN_HXX
#include <tools/svwin.h>
#endif
#include <stdio.h>

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWCLASS_HPP_
#include <com/sun/star/awt/WindowClass.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_SYSTEMDEPENDENT_HPP_
#include <com/sun/star/lang/SystemDependent.hpp>
#endif
#include "com/sun/star/awt/FocusEvent.hpp"
#include "com/sun/star/awt/KeyEvent.hpp"
#include "com/sun/star/awt/KeyModifier.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"

#include <cppuhelper/typeprovider.hxx>
#include <osl/conditn.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <rtl/process.h>

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
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


#include <vcl/sysdata.hxx>

#include <tools/debug.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

namespace css = ::com::sun::star;

#define VCLWINDOW_FRAMEWINDOW               0x1000
#define VCLWINDOW_SYSTEMCHILDWINDOW         0x1001

#ifdef UNX
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW
#elif (defined WNT)
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32
#elif (defined OS2)
#define SYSTEM_DEPENDENT_TYPE ::com::sun::star::lang::SystemDependent::SYSTEM_OS2
#endif

sal_uInt32 ImplGetWinBits( sal_uInt32 nComponentAttribs, sal_uInt16 nCompType )
{
    sal_uInt32 nWinBits = 0;

    sal_Bool bMessBox = sal_False;
    if ( ( nCompType == WINDOW_INFOBOX ) ||
         ( nCompType == WINDOW_MESSBOX ) ||
         ( nCompType == WINDOW_QUERYBOX ) ||
         ( nCompType == WINDOW_WARNINGBOX ) ||
         ( nCompType == WINDOW_ERRORBOX ) )
    {
        bMessBox = sal_True;
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

    return nWinBits;
}

struct ComponentInfo
{
    const char*     pName;
    WindowType      nWinType;
};

static ComponentInfo __FAR_DATA aComponentInfos [] =
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
    { "helpbutton",         WINDOW_HELPBUTTON },
    { "imagebutton",        WINDOW_IMAGEBUTTON },
    { "imageradiobutton",   WINDOW_IMAGERADIOBUTTON },
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
    { "workwindow",         WINDOW_WORKWINDOW }
};

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
     ComponentInfoCompare( const void* pFirst, const void* pSecond)
{
    return( strcmp( ((ComponentInfo*)pFirst)->pName,
                    ((ComponentInfo*)pSecond)->pName ) );
}

sal_uInt16 ImplGetComponentType( const String& rServiceName )
{
    static sal_Bool bSorted = sal_False;
    if( !bSorted )
    {
        qsort(  (void*) aComponentInfos,
                sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                sizeof( ComponentInfo ),
                ComponentInfoCompare );
        bSorted = sal_True;
    }


    ComponentInfo aSearch;
    ByteString aServiceName( rServiceName, gsl_getSystemTextEncoding() );
    aServiceName.ToLowerAscii();
    if ( aServiceName.Len() )
        aSearch.pName = aServiceName.GetBuffer();
    else
        aSearch.pName = "window";

    ComponentInfo* pInf = (ComponentInfo*) bsearch( &aSearch,
                        (void*) aComponentInfos,
                        sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                        sizeof( ComponentInfo ),
                        ComponentInfoCompare );

    return pInf ? pInf->nWinType : 0;
}


//  ----------------------------------------------------
//  class VCLXToolkit
//  ----------------------------------------------------

static sal_Int32                            nVCLToolkitInstanceCount = 0;
static BOOL                                 bInitedByVCLToolkit = sal_False;
//static cppu::OInterfaceContainerHelper *  pToolkits = 0;

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

struct ToolkitThreadData
{
    VCLXToolkit * pTk;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMgr;

    ToolkitThreadData( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr, VCLXToolkit * pTk_ )
        : pTk( pTk_ )
        , xSMgr( rSMgr )
    {
    }
};

static void SAL_CALL ToolkitWorkerFunction( void* pArgs )
{
    ToolkitThreadData * pTTD = (ToolkitThreadData *)pArgs;
    bInitedByVCLToolkit = InitVCL( pTTD->xSMgr );
    if( bInitedByVCLToolkit )
    {
        UnoWrapper* pUnoWrapper = new UnoWrapper( pTTD->pTk );
        Application::SetUnoWrapper( pUnoWrapper );
    }
    getInitCondition().set();
    if( bInitedByVCLToolkit )
    {
        {
        osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
        Application::Execute();
        }
        try
        {
            pTTD->pTk->dispose();
        }
        catch( com::sun::star::uno::Exception & )
        {
        }
        /*
        if( pToolkits )
        {
            cppu::OInterfaceIteratorHelper aIt( *pToolkits );
            ::com::sun::star::uno::XInterface * pI;
            while( pI = aIt.next() )
                ((::com::sun::star::lang::XComponent *)pI)->dispose();

            // delete toolkit container
            osl::Guard< osl::Mutex > aGuard( getInitMutex() );
            delete pToolkits;
            pToolkits = 0;
        }
        */
        DeInitVCL();
    }
    else
    {
        JoinMainLoopThread();
    }
    delete pTTD;
}


// contructor, which might initialize VCL
VCLXToolkit::VCLXToolkit( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr ):
    cppu::WeakComponentImplHelper5<
    ::com::sun::star::awt::XToolkit, ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::awt::XSystemChildFactory,
    ::com::sun::star::awt::XDataTransferProviderAccess,
    ::com::sun::star::awt::XExtendedToolkit >( GetMutex() ),
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
        CreateMainLoopThread( ToolkitWorkerFunction, new ToolkitThreadData( rSMgr, this ) );
        getInitCondition().wait();
        /*
        if( bInitedByVCLToolkit )
        {
            // insert in disposing list
            if( !pToolkits )
                pToolkits = new cppu::OInterfaceContainerHelper( getInitMutex() );
            pToolkits->addInterface( (::com::sun::star::lang::XComponent *)this );
        }
        */
    }
}

VCLXToolkit::~VCLXToolkit()
{
}


void SAL_CALL VCLXToolkit::disposing()
{
    if ( hSvToolsLib )
    {
        osl_unloadModule( hSvToolsLib );
        hSvToolsLib = NULL;
        fnSvtCreateWindow = NULL;
    }

    {
        osl::Guard< osl::Mutex > aGuard( getInitMutex() );
        if( --nVCLToolkitInstanceCount == 0 )
        {
            if( bInitedByVCLToolkit )
            {
                Application::Quit();
                JoinMainLoopThread();
                bInitedByVCLToolkit = sal_False;
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
    ::css::lang::EventObject aEvent(
        static_cast< ::cppu::OWeakObject * >(this));
    m_aTopWindowListeners.disposeAndClear(aEvent);
    m_aKeyHandlers.disposeAndClear(aEvent);
    m_aFocusListeners.disposeAndClear(aEvent);

/*
    osl::Guard< osl::Mutex > aGuard( getInitMutex() );
    // insert in disposing list
    if( pToolkits )
    {
        // remove from the disposing list
        pToolkits->removeInterface( (::com::sun::star::lang::XComponent *)this );
    }
*/
}


::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::getDesktopWindow(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;
    // 07/00: AppWindow doesn't exist anymore...
    return xRef;
}

::com::sun::star::awt::Rectangle VCLXToolkit::getWorkArea(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Rectangle aRect;
    // 07/00: AppWindow doesn't exist anymore...
    return aRect;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::createWindow( const ::com::sun::star::awt::WindowDescriptor& rDescriptor ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    osl::Guard< vos::IMutex > aSolarGuard( Application::GetSolarMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;

    Window* pParent = NULL;
    if ( rDescriptor.Parent.is() )
    {
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( rDescriptor.Parent );

        // #103939# Don't through assertion, may be it's a system dependend window, used in ImplCreateWindow.
        // DBG_ASSERT( pParentComponent, "ParentComponent not valid" );

        if ( pParentComponent )
            pParent = pParentComponent->GetWindow();
    }

    sal_uInt32 nWinBits = ImplGetWinBits( rDescriptor.WindowAttributes,
        ImplGetComponentType( rDescriptor.WindowServiceName ) );

    VCLXWindow* pNewComp = NULL;

    Window* pNewWindow = NULL;
    // Try to create the window with SvTools
    // (do this _before_ creating it on our own: The old mechanism (extended toolkit in SvTools) did it this way,
    // and we need to stay compatible)
    // try to load the lib
    if ( !fnSvtCreateWindow && !hSvToolsLib )
    {
        ::rtl::OUString aLibName = ::vcl::unohelper::CreateLibraryName( "svt", TRUE );
        hSvToolsLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_DEFAULT );
        if ( hSvToolsLib )
        {
            ::rtl::OUString aFunctionName( RTL_CONSTASCII_USTRINGPARAM( "CreateWindow" ) );
            fnSvtCreateWindow = (FN_SvtCreateWindow)osl_getSymbol( hSvToolsLib, aFunctionName.pData );
        }
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
            pNewComp->SetCreatedWithToolkit( TRUE );
            pNewWindow->SetComponentInterface( pNewComp );
            xRef = pNewComp;
        }

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

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXToolkit::createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > xRef;
    VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;

    osl::Guard< vos::IMutex > aSolarGuard( Application::GetSolarMutex() );

    VirtualDevice* pV = new VirtualDevice;
    pV->SetOutputSizePixel( Size( Width, Height ) );
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
    const ::com::sun::star::awt::WindowDescriptor& rDescriptor, Window* pParent, sal_uInt32 nWinBits )
{
    String aServiceName( rDescriptor.WindowServiceName );
    aServiceName.ToLowerAscii();

    Window* pNewWindow = NULL;
    sal_uInt16 nType = ImplGetComponentType( aServiceName );

    if ( !pParent )
    {
        // Wenn die Component einen Parent braucht, dann NULL zurueckgeben,
        // spaeter mal ::com::sun::star::uno::Exception...
        sal_Bool bException = sal_True;
        if ( ( nType == WINDOW_DIALOG ) || ( nType == WINDOW_MODALDIALOG ) || ( nType == WINDOW_MODELESSDIALOG ) )
            bException = sal_False;
        else if ( ( nType == WINDOW_WINDOW ) ||
                  ( nType == WINDOW_WORKWINDOW ) ||
                  ( nType == VCLWINDOW_FRAMEWINDOW ) )
        {
            if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_TOP )
                bException = sal_False;
        }

        if ( bException )
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }

    if ( nType )
    {
        NAMESPACE_VOS(OGuard) aVclGuard( Application::GetSolarMutex()  );
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
                ((ComboBox*)pNewWindow)->EnableAutoSize( sal_False );
                *ppNewComp = new VCLXComboBox;
            break;
            case WINDOW_CURRENCYBOX:
                pNewWindow = new CurrencyBox( pParent, nWinBits );
            break;
            case WINDOW_CURRENCYFIELD:
                pNewWindow = new CurrencyField( pParent, nWinBits );
                static_cast<CurrencyField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXNumericField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(CurrencyField*)pNewWindow );
            break;
            case WINDOW_DATEBOX:
                pNewWindow = new DateBox( pParent, nWinBits );
            break;
            case WINDOW_DATEFIELD:
                pNewWindow = new DateField( pParent, nWinBits );
                static_cast<DateField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXDateField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(DateField*)pNewWindow );
            break;
            case WINDOW_EDIT:
                pNewWindow = new Edit( pParent, nWinBits );
                *ppNewComp = new VCLXEdit;
            break;
            case WINDOW_ERRORBOX:
                pNewWindow = new ErrorBox( pParent, nWinBits, String() );
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
                pNewWindow = new GroupBox( pParent, nWinBits );
            break;
            case WINDOW_HELPBUTTON:
                pNewWindow = new HelpButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGEBUTTON:
                 pNewWindow = new ImageButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGERADIOBUTTON:
                pNewWindow = new ImageRadioButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_INFOBOX:
                pNewWindow = new InfoBox( pParent, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_LISTBOX:
                pNewWindow = new ListBox( pParent, nWinBits|WB_SIMPLEMODE|WB_AUTOHSCROLL );
                ((ListBox*)pNewWindow)->EnableAutoSize( sal_False );
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
                pNewWindow = new MessBox( pParent, nWinBits, String(), String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_METRICBOX:
                pNewWindow = new MetricBox( pParent, nWinBits );
            break;
            case WINDOW_METRICFIELD:
                pNewWindow = new MetricField( pParent, nWinBits );
                *ppNewComp = new VCLXSpinField;
            break;
            case WINDOW_DIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
                // Modal/Modeless nur durch Show/Execute
                pNewWindow = new Dialog( pParent, nWinBits );
                *ppNewComp = new VCLXDialog;
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
                static_cast<NumericField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
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
                pNewWindow = new QueryBox( pParent, nWinBits, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_RADIOBUTTON:
                pNewWindow = new RadioButton( pParent, nWinBits );
                *ppNewComp = new VCLXRadioButton;

                // by default, disable RadioCheck
                // Since the VCLXRadioButton really cares for it's RadioCheck settings, this is important:
                // if we enable it, the VCLXRadioButton will use RadioButton::Check instead of RadioButton::SetState
                // This leads to a strange behaviour if the control is newly created: when settings the initial
                // state to "checked", the RadioButton::Check (called because RadioCheck=TRUE) will uncheck
                // _all_other_ radio buttons in the same group. However, at this moment the grouping of the controls
                // is not really valid: the controls are grouped after they have been created, but we're still in
                // the creation process, so the RadioButton::Check relies on invalid grouping information.
                // 07.08.2001 - #87254# - frank.schoenheit@sun.com
                static_cast<RadioButton*>(pNewWindow)->EnableRadioCheck( FALSE );
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
                *ppNewComp = new VCLXButton;
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
            break;
            case WINDOW_TABDIALOG:
                pNewWindow = new TabDialog( pParent, nWinBits );
            break;
            case WINDOW_TABPAGE:
                pNewWindow = new TabPage( pParent, nWinBits );
            break;
            case WINDOW_TIMEBOX:
                pNewWindow = new TimeBox( pParent, nWinBits );
            break;
            case WINDOW_TIMEFIELD:
                pNewWindow = new TimeField( pParent, nWinBits );
                static_cast<TimeField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
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
                pNewWindow = new WarningBox( pParent, nWinBits, String() );
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

                                ::com::sun::star::uno::Sequence<sal_Int8> processIdSeq(processID, 16);

                                ::com::sun::star::uno::Any anyHandle = xSystemDepParent->getWindowHandle(processIdSeq, SYSTEM_DEPENDENT_TYPE);

#if defined UNX
                                sal_Int32 x11_id;

                                            if (anyHandle >>= x11_id)
                                            {
                                                            printf("x11_id = %ld\n", x11_id);
                                                            SystemParentData aParentData;
                                                            aParentData.nSize   = sizeof( aParentData );
                                                            aParentData.aWindow = x11_id;
                                                            pNewWindow = new WorkWindow( &aParentData );
                                            }
#elif defined WNT

                                sal_Int32 hWnd;

                                            if (anyHandle >>= hWnd)
                                            {
                                                            printf("hWnd = %ld\n", hWnd);
                                                            SystemParentData aParentData;
                                                            aParentData.nSize   = sizeof( aParentData );
                                                            aParentData.hWnd    = (HWND)hWnd;
                                                            pNewWindow = new WorkWindow( &aParentData );
                                            }
#endif
                            }
                        }

                        if (!pNewWindow)
                            pNewWindow = new WorkWindow( pParent, nWinBits );
                    }
                    *ppNewComp = new VCLXTopWindow;
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
            default:    DBG_ERROR( "UNO3!" );
        }
    }

    return pNewWindow;
}

// ::com::sun::star::awt::XSystemChildFactory
::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::createSystemChild( const ::com::sun::star::uno::Any& Parent, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 nSystemType ) throw(::com::sun::star::uno::RuntimeException)
{
    Window* pChildWindow = NULL;
    if ( nSystemType == SYSTEM_DEPENDENT_TYPE )
    {
#if defined UNX
            sal_Int32 x11_id;
            if ( Parent >>= x11_id )
            {
                printf("x11_id = %ld\n", x11_id);
                SystemParentData aParentData;
                aParentData.nSize = sizeof( aParentData );
                aParentData.aWindow = x11_id;
                osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
                pChildWindow = new WorkWindow( &aParentData );
            }
#elif defined WNT
            sal_Int32 hWnd;
            if ( Parent >>= hWnd)
            {
                printf("hWnd = %ld\n", hWnd);
                SystemParentData aParentData;
                aParentData.nSize = sizeof( aParentData );
                aParentData.hWnd = (HWND)hWnd;
                osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
                pChildWindow = new WorkWindow( &aParentData );
            }
#endif
    }
    else if (nSystemType == com::sun::star::lang::SystemDependent::SYSTEM_JAVA)
    {
        osl::Guard< vos::IMutex > aGuard(Application::GetSolarMutex());
        pChildWindow = new WorkWindow(0, Parent);
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer;
    if ( pChildWindow )
    {
        VCLXTopWindow* pPeer = new VCLXTopWindow;
        osl::Guard< vos::IMutex > aGuard( Application::GetSolarMutex() );
        pPeer->SetWindow( pChildWindow );
        xPeer = pPeer;
    }

    return xPeer;
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

::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL VCLXToolkit::getClipboard( const ::rtl::OUString& clipboardName ) throw(::com::sun::star::uno::RuntimeException)
{
    if( clipboardName.getLength() == 0 )
    {
        if( !mxClipboard.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
            if ( xFactory.is() )
            {
                // remember clipboard here
                mxClipboard = ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > (
                    xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ), ::com::sun::star::uno::UNO_QUERY );
            }
        }

        return mxClipboard;
    }

    else if( clipboardName.equals( ::rtl::OUString::createFromAscii("Selection") ) )
    {
        if( !mxSelection.is() )
            ;

        return mxSelection;
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >();
}

// XServiceInfo
::rtl::OUString VCLXToolkit::getImplementationName() throw(::com::sun::star::uno::RuntimeException)
{
    return rtl::OUString::createFromAscii( "stardiv.Toolkit.VCLXToolkit" );
}

sal_Bool VCLXToolkit::supportsService( const ::rtl::OUString& rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString* pArray = aSNL.getConstArray();
    const ::rtl::OUString* pArrayEnd = aSNL.getConstArray();
    for (; pArray != pArrayEnd; ++pArray )
        if( *pArray == rServiceName )
            break;

    return pArray != pArrayEnd;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > VCLXToolkit::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( szServiceName2_Toolkit ) );
    return ::com::sun::star::uno::Sequence< ::rtl::OUString >( &aServiceName, 1);
}

// css::awt::XExtendedToolkit:

// virtual
::sal_Int32 SAL_CALL VCLXToolkit::getTopWindowCount()
    throw (::css::uno::RuntimeException)
{
    return static_cast< ::sal_Int32 >(::Application::GetTopWindowCount());
        // XXX  numeric overflow
}

// virtual
::css::uno::Reference< ::css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getTopWindow(::sal_Int32 nIndex)
    throw (::css::uno::RuntimeException)
{
    ::Window * p = ::Application::GetTopWindow(static_cast< long >(nIndex));
        // XXX  numeric overflow
    return ::css::uno::Reference< ::css::awt::XTopWindow >(
        p == 0 ? 0 : static_cast< ::css::awt::XWindow * >(p->GetWindowPeer()),
        ::css::uno::UNO_QUERY);
}

// virtual
::css::uno::Reference< ::css::awt::XTopWindow > SAL_CALL
VCLXToolkit::getActiveTopWindow() throw (::css::uno::RuntimeException)
{
    ::Window * p = ::Application::GetActiveTopWindow();
    return ::css::uno::Reference< ::css::awt::XTopWindow >(
        p == 0 ? 0 : static_cast< ::css::awt::XWindow * >(p->GetWindowPeer()),
        ::css::uno::UNO_QUERY);
}

// virtual
void SAL_CALL VCLXToolkit::addTopWindowListener(
    ::css::uno::Reference< ::css::awt::XTopWindowListener > const & rListener)
    throw (::css::uno::RuntimeException)
{
    OSL_ENSURE(rListener.is(), "Null rListener");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rListener->disposing(
            ::css::lang::EventObject(
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
    ::css::uno::Reference< ::css::awt::XTopWindowListener > const & rListener)
    throw (::css::uno::RuntimeException)
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
    ::css::uno::Reference< ::css::awt::XKeyHandler > const & rHandler)
    throw (::css::uno::RuntimeException)
{
    OSL_ENSURE(rHandler.is(), "Null rHandler");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rHandler->disposing(
            ::css::lang::EventObject(
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
    ::css::uno::Reference< ::css::awt::XKeyHandler > const & rHandler)
    throw (::css::uno::RuntimeException)
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
    ::css::uno::Reference< ::css::awt::XFocusListener > const & rListener)
    throw (::css::uno::RuntimeException)
{
    OSL_ENSURE(rListener.is(), "Null rListener");
    ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        rListener->disposing(
            ::css::lang::EventObject(
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
    ::css::uno::Reference< ::css::awt::XFocusListener > const & rListener)
    throw (::css::uno::RuntimeException)
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
    ::com::sun::star::uno::XInterface > const & source)
    throw (::com::sun::star::uno::RuntimeException)
{
}

// virtual
void SAL_CALL VCLXToolkit::fireFocusLost(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface > const & source)
    throw (::com::sun::star::uno::RuntimeException)
{
}


IMPL_LINK(VCLXToolkit, eventListenerHandler, ::VclSimpleEvent const *, pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_SHOW:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowOpened);
        break;
    case VCLEVENT_WINDOW_HIDE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowClosed);
        break;
    case VCLEVENT_WINDOW_ACTIVATE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowActivated);
        break;
    case VCLEVENT_WINDOW_DEACTIVATE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowDeactivated);
        break;
    case VCLEVENT_WINDOW_CLOSE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowClosing);
        break;
    case VCLEVENT_WINDOW_GETFOCUS:
        callFocusListeners(pEvent, true);
        break;
    case VCLEVENT_WINDOW_LOSEFOCUS:
        callFocusListeners(pEvent, false);
        break;
    case VCLEVENT_WINDOW_MINIMIZE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowMinimized);
        break;
    case VCLEVENT_WINDOW_NORMALIZE:
        callTopWindowListeners(
            pEvent, &::css::awt::XTopWindowListener::windowNormalized);
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
    void (SAL_CALL ::css::awt::XTopWindowListener::* pFn)(
        ::css::lang::EventObject const &))
{
    ::Window * pWindow
          = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();
    if (pWindow->IsTopWindow())
    {
        ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >
              aListeners(m_aTopWindowListeners.getElements());
        if (aListeners.hasElements())
        {
            ::css::lang::EventObject aAwtEvent(
                static_cast< ::css::awt::XWindow * >(pWindow->GetWindowPeer()));
            for (::sal_Int32 i = 0; i < aListeners.getLength(); ++i)
            {
                ::css::uno::Reference< ::css::awt::XTopWindowListener >
                      xListener(aListeners[i], ::css::uno::UNO_QUERY);
                try
                {
                    (xListener.get()->*pFn)(aAwtEvent);
                }
                catch (::css::uno::RuntimeException & rEx)
                {
                    OSL_TRACE(
                        "VCLXToolkit::callTopWindowListeners: caught %s\n",
                        ::rtl::OUStringToOString(
                            rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
    }
}

long VCLXToolkit::callKeyHandlers(::VclSimpleEvent const * pEvent,
                                  bool bPressed)
{
    ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >
          aHandlers(m_aKeyHandlers.getElements());

    if (aHandlers.hasElements())
    {
        ::Window * pWindow = static_cast< ::VclWindowEvent const * >(pEvent)->GetWindow();

        // See implementation in vclxwindow.cxx for mapping between VCL and UNO AWT event
        ::KeyEvent * pKeyEvent = static_cast< ::KeyEvent * >(
            static_cast< ::VclWindowEvent const * >(pEvent)->GetData());
        ::css::awt::KeyEvent aAwtEvent(
            static_cast< ::css::awt::XWindow * >(pWindow->GetWindowPeer()),
            (pKeyEvent->GetKeyCode().IsShift()
             ? ::css::awt::KeyModifier::SHIFT : 0)
            | (pKeyEvent->GetKeyCode().IsMod1()
               ? ::css::awt::KeyModifier::MOD1 : 0)
            | (pKeyEvent->GetKeyCode().IsMod2()
               ? ::css::awt::KeyModifier::MOD2 : 0),
            pKeyEvent->GetKeyCode().GetCode(), pKeyEvent->GetCharCode(),
            pKeyEvent->GetKeyCode().GetFunction());
        for (::sal_Int32 i = 0; i < aHandlers.getLength(); ++i)
        {
            ::css::uno::Reference< ::css::awt::XKeyHandler > xHandler(
                aHandlers[i], ::css::uno::UNO_QUERY);
            try
            {
                if ((bPressed ? xHandler->keyPressed(aAwtEvent)
                      : xHandler->keyReleased(aAwtEvent)))
                    return 1;
            }
            catch (::css::uno::RuntimeException & rEx)
            {
                OSL_TRACE(
                    "VCLXToolkit::callKeyHandlers: caught %s\n",
                    ::rtl::OUStringToOString(
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
        ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > >
              aListeners(m_aFocusListeners.getElements());
        if (aListeners.hasElements())
        {
            // Ignore the interior of compound controls when determining the
            // window that gets the focus next (see implementation in
            // vclxwindow.cxx for mapping between VCL and UNO AWT event):
            ::css::uno::Reference< css::uno::XInterface > xNext;
            ::Window * pFocus = ::Application::GetFocusWindow();
            for (::Window * p = pFocus; p != 0; p = p->GetParent())
                if (!p->IsCompoundControl())
                {
                    pFocus = p;
                    break;
                }
            if (pFocus != 0)
                xNext = pFocus->GetComponentInterface(true);
            ::css::awt::FocusEvent aAwtEvent(
                static_cast< ::css::awt::XWindow * >(pWindow->GetWindowPeer()),
                pWindow->GetGetFocusFlags(), xNext, false);
            for (::sal_Int32 i = 0; i < aListeners.getLength(); ++i)
            {
                ::css::uno::Reference< ::css::awt::XFocusListener > xListener(
                    aListeners[i], ::css::uno::UNO_QUERY);
                try
                {
                    bGained ? xListener->focusGained(aAwtEvent)
                        : xListener->focusLost(aAwtEvent);
                }
                catch (::css::uno::RuntimeException & rEx)
                {
                    OSL_TRACE(
                        "VCLXToolkit::callFocusListeners: caught %s\n",
                        ::rtl::OUStringToOString(
                            rEx.Message, RTL_TEXTENCODING_UTF8).getStr());
                }
            }
        }
    }
}
