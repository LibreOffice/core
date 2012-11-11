/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <tabwin/tabwindow.hxx>
#include <properties.h>

#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <toolkit/unohlp.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using ::rtl::OUString;
using namespace com::sun::star;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_11                    (   TabWindow                                                                          ,
                                            ::cppu::OWeakObject                                                                ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                        ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                         ),
                                            DIRECT_INTERFACE( css::lang::XInitialization                                      ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                           ),
                                            DIRECT_INTERFACE( css::awt::XWindowListener                                       ),
                                            DIRECT_INTERFACE( css::awt::XTopWindowListener                                    ),
                                            DIRECT_INTERFACE( css::awt::XSimpleTabController                                  ),
                                            DERIVED_INTERFACE( css::lang::XEventListener, css::awt::XWindowListener           ),
                                            DIRECT_INTERFACE( css::beans::XMultiPropertySet                                   ),
                                            DIRECT_INTERFACE( css::beans::XFastPropertySet                                    ),
                                            DIRECT_INTERFACE( css::beans::XPropertySet                                        )
                                        )

DEFINE_XTYPEPROVIDER_11                 (   TabWindow                               ,
                                            css::lang::XTypeProvider                ,
                                            css::lang::XServiceInfo                 ,
                                            css::lang::XInitialization              ,
                                            css::lang::XComponent                   ,
                                            css::awt::XWindowListener               ,
                                            css::awt::XTopWindowListener            ,
                                            css::awt::XSimpleTabController          ,
                                            css::lang::XEventListener               ,
                                            css::beans::XMultiPropertySet           ,
                                            css::beans::XFastPropertySet            ,
                                            css::beans::XPropertySet
                                        )

PRIVATE_DEFINE_XSERVICEINFO_BASE        (   TabWindow                           ,
                                            ::cppu::OWeakObject                 ,
                                            SERVICENAME_TABWINDOW               ,
                                            IMPLEMENTATIONNAME_TABWINDOW
                                        )

DEFINE_INIT_SERVICE                     (   TabWindow, {} )

TabWindow::TabWindow( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex() )
    , ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    , m_bInitialized( sal_False )
    , m_bDisposed( sal_False )
    , m_nNextTabID( 1 )
    , m_aTitlePropName( RTL_CONSTASCII_USTRINGPARAM( "Title" ))
    , m_aPosPropName( RTL_CONSTASCII_USTRINGPARAM( "Position" ))
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
}

TabWindow::~TabWindow()
{
}

//---------------------------------------------------------------------------------------------------------
// Helper
//---------------------------------------------------------------------------------------------------------

void TabWindow::implts_LayoutWindows() const
{
    const sal_Int32 nTabControlHeight = 30;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    css::uno::Reference< css::awt::XDevice > xDevice( m_xTopWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xWindow( m_xTopWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xTabControlWindow( m_xTabControlWindow );
    css::uno::Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Convert relativ size to output size.
    if ( xWindow.is() && xDevice.is() )
    {
        css::awt::Rectangle  aRectangle  = xWindow->getPosSize();
        css::awt::DeviceInfo aInfo       = xDevice->getInfo();
        css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                            aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

        css::awt::Size  aContainerWindowSize;
        css::awt::Size  aTabControlSize;

        aContainerWindowSize.Width = aSize.Width;
        aTabControlSize.Width = aSize.Width;

        aContainerWindowSize.Height = std::max( sal_Int32( 0 ), aSize.Height - nTabControlHeight );
        aTabControlSize.Height = nTabControlHeight;

        xContainerWindow->setPosSize( 0, 0,
                                      aContainerWindowSize.Width, aContainerWindowSize.Height,
                                      css::awt::PosSize::POSSIZE );
        xTabControlWindow->setPosSize( 0, std::max( nTabControlHeight, sal_Int32( aSize.Height - nTabControlHeight)),
                                       aTabControlSize.Width, aTabControlSize.Height,
                                       css::awt::PosSize::POSSIZE );
    }
}

TabControl* TabWindow::impl_GetTabControl( const css::uno::Reference< css::awt::XWindow >& rTabControlWindow ) const
{
    Window* pWindow = VCLUnoHelper::GetWindow( rTabControlWindow );
    if ( pWindow )
        return (TabControl *)pWindow;
    else
        return NULL;
}

void TabWindow::impl_SetTitle( const ::rtl::OUString& rTitle )
{
    if ( m_xTopWindow.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow(
                            css::uno::Reference< css::awt::XWindow >(
                                m_xTopWindow, css::uno::UNO_QUERY ));
        if ( pWindow )
            pWindow->SetText( rTitle );
    }
}

void TabWindow::implts_SendNotification( Notification eNotify, sal_Int32 ID ) const
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer(
                                                        ::getCppuType( ( const css::uno::Reference< css::awt::XTabListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                switch ( eNotify )
                {
                    case NOTIFY_INSERTED:
                        ((css::awt::XTabListener*)pIterator.next())->inserted( ID );
                        break;
                    case NOTIFY_REMOVED:
                        ((css::awt::XTabListener*)pIterator.next())->removed( ID );
                        break;
                    case NOTIFY_ACTIVATED:
                        ((css::awt::XTabListener*)pIterator.next())->activated( ID );
                        break;
                    case NOTIFY_DEACTIVATED:
                        ((css::awt::XTabListener*)pIterator.next())->deactivated( ID );
                        break;
                    default:
                        break;
                }
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

void TabWindow::implts_SendNotification( Notification eNotify, sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& rSeq ) const
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer(
                                                        ::getCppuType( ( const css::uno::Reference< css::awt::XTabListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                switch ( eNotify )
                {
                    case NOTIFY_CHANGED:
                        ((css::awt::XTabListener*)pIterator.next())->changed( ID, rSeq );
                        break;
                    default:
                        break;
                }
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------
// Links
//---------------------------------------------------------------------------------------------------------

IMPL_LINK( TabWindow, Activate, TabControl*, pTabControl )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    sal_Int32 nPageId = pTabControl->GetCurPageId();

    rtl::OUString aTitle = pTabControl->GetPageText( sal_uInt16( nPageId ));
    impl_SetTitle( aTitle );
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_ACTIVATED, nPageId );

    return 1;
}

IMPL_LINK( TabWindow, Deactivate, TabControl*, pTabControl )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    sal_Int32 nPageId = pTabControl->GetCurPageId();
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_DEACTIVATED, nPageId );

    return 1;
}

//---------------------------------------------------------------------------------------------------------
// XInitilization
//---------------------------------------------------------------------------------------------------------

void SAL_CALL TabWindow::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
throw (css::uno::Exception, css::uno::RuntimeException)
{
    const rtl::OUString aTopWindowArgName( RTL_CONSTASCII_USTRINGPARAM( "TopWindow" ));
    const rtl::OUString aSizeArgName( RTL_CONSTASCII_USTRINGPARAM( "Size" ));

    css::awt::Size aDefaultSize( 500, 500 );
    css::awt::Size aSize( aDefaultSize );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    sal_Bool                                               bInitalized( m_bInitialized );
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xServiceManager );
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( !bInitalized )
    {
        css::beans::PropertyValue                   aPropValue;
        css::uno::Reference< css::awt::XTopWindow > xTopWindow;
        css::uno::Reference< css::awt::XToolkit2 >  xToolkit;
        css::awt::WindowDescriptor                  aDescriptor;

        if ( xSMGR.is() )
        {
            try
            {
                xToolkit = css::awt::Toolkit::create( comphelper::getComponentContext(xSMGR) );
            }
            catch ( const css::uno::RuntimeException& )
            {
                throw;
            }
            catch ( const css::uno::Exception& )
            {
            }
        }

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name == aTopWindowArgName )
                    aPropValue.Value >>= xTopWindow;
                else if ( aPropValue.Name == aSizeArgName )
                {
                    aPropValue.Value >>= aSize;
                    if ( aSize.Width <= 0 )
                        aSize.Width = aDefaultSize.Width;
                    if ( aSize.Height <= 0 )
                        aSize.Height = aDefaultSize.Height;
                }
            }
        }

        if ( xToolkit.is() )
        {
            if ( !xTopWindow.is() )
            {
                // describe top window properties.
                aDescriptor.Type                =   css::awt::WindowClass_TOP;
                aDescriptor.ParentIndex         =   -1;
                aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >();
                aDescriptor.Bounds              =   css::awt::Rectangle( 0, 0, aSize.Width, aSize.Height );
                aDescriptor.WindowAttributes    =   0;

                try
                {
                    xTopWindow = css::uno::Reference< css::awt::XTopWindow >( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );
                }
                catch ( const css::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( const css::uno::Exception& )
                {
                }
            }

            if ( xTopWindow.is() )
            {
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                aLock.lock();
                m_bInitialized = sal_True;
                aLock.unlock();
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */

                css::uno::Reference< css::awt::XWindow > xWindow( xTopWindow, css::uno::UNO_QUERY );
                xWindow->addWindowListener( css::uno::Reference< css::awt::XWindowListener >(
                    static_cast< ::cppu::OWeakObject* >( this ), css::uno::UNO_QUERY_THROW ));

                xTopWindow->addTopWindowListener( css::uno::Reference< css::awt::XTopWindowListener >(
                    static_cast< ::cppu::OWeakObject* >( this ), css::uno::UNO_QUERY_THROW ));

                css::uno::Reference< css::awt::XWindow > xContainerWindow;
                css::uno::Reference< css::awt::XWindow > xTabControl;

                // describe container window properties.
                aDescriptor.Type                =   css::awt::WindowClass_SIMPLE;
                aDescriptor.ParentIndex         =   -1;
                aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >( xTopWindow, css::uno::UNO_QUERY );
                aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0);
                aDescriptor.WindowAttributes    =   0;

                xContainerWindow = css::uno::Reference< css::awt::XWindow >( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );

                // create a tab control window properties
                aDescriptor.Type                = css::awt::WindowClass_SIMPLE;
                aDescriptor.WindowServiceName   = DECLARE_ASCII("tabcontrol");
                aDescriptor.ParentIndex         = -1;
                aDescriptor.Parent              = css::uno::Reference< css::awt::XWindowPeer >( xTopWindow, css::uno::UNO_QUERY );
                aDescriptor.Bounds              = css::awt::Rectangle( 0,0,0,0 );
                aDescriptor.WindowAttributes    = 0;

                xTabControl = css::uno::Reference< css::awt::XWindow >( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );

                if ( xContainerWindow.is() && xTabControl.is() )
                {
                    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                    aLock.lock();
                    m_xTopWindow = xTopWindow;
                    m_xContainerWindow = xContainerWindow;
                    m_xTabControlWindow = xTabControl;
                    aLock.unlock();
                    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

                    xWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );

                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if( pWindow )
                        pWindow->Show( sal_True );

                    pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
                    if ( pWindow )
                        pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE  );

                    pWindow = VCLUnoHelper::GetWindow( xTabControl );
                    if ( pWindow )
                    {
                        pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE  );
                        TabControl* pTabControl = (TabControl *)pWindow;
                        pTabControl->SetActivatePageHdl( LINK( this, TabWindow, Activate ));
                        pTabControl->SetDeactivatePageHdl( LINK( this, TabWindow, Deactivate ));
                    }

                    implts_LayoutWindows();
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//  XComponent
//---------------------------------------------------------------------------------------------------------
void SAL_CALL TabWindow::dispose() throw (css::uno::RuntimeException)
{
    // Send message to all listener and forget her references.
    css::uno::Reference< css::lang::XComponent > xThis(
        static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject aEvent( xThis );

    m_aListenerContainer.disposeAndClear( aEvent );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    css::uno::Reference< css::awt::XWindow > xTabControlWindow( m_xTabControlWindow );
    css::uno::Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    css::uno::Reference< css::awt::XTopWindow > xTopWindow( m_xTopWindow );
    m_xTabControlWindow.clear();
    m_xContainerWindow.clear();
    m_xTopWindow.clear();
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    css::uno::Reference< css::lang::XComponent > xComponent( xTabControlWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    xComponent = css::uno::Reference< css::lang::XComponent >( xContainerWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    xComponent = css::uno::Reference< css::lang::XComponent >( xTopWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    aLock.lock();
    m_bDisposed = sal_True;
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL TabWindow::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        return;
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.addInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

void SAL_CALL TabWindow::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        return;
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.removeInterface( ::getCppuType( ( const css::uno::Reference< css::lang::XEventListener >* ) NULL ), xListener );
}

//---------------------------------------------------------------------------------------------------------
// XEventListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL TabWindow::disposing( const css::lang::EventObject& )
throw( css::uno::RuntimeException )
{
}

//---------------------------------------------------------------------------------------------------------
// XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL TabWindow::windowResized( const css::awt::WindowEvent& )
throw( css::uno::RuntimeException )
{
    implts_LayoutWindows();
}

void SAL_CALL TabWindow::windowMoved( const css::awt::WindowEvent& )
throw( css::uno::RuntimeException )
{
}

void SAL_CALL TabWindow::windowShown( const css::lang::EventObject& )
throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->Show();

    if ( m_xContainerWindow.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pWindow )
            pWindow->Show();
    }
}

void SAL_CALL TabWindow::windowHidden( const css::lang::EventObject& )
throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    if ( m_xContainerWindow.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pWindow )
            pWindow->Hide();
    }

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->Hide();
}

//---------------------------------------------------------------------------------------------------------
// XTopWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL TabWindow::windowOpened( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL TabWindow::windowClosing( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XComponent > xComponent( (OWeakObject *)this, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}

void SAL_CALL TabWindow::windowClosed( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL TabWindow::windowMinimized( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL TabWindow::windowNormalized( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL TabWindow::windowActivated( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

void SAL_CALL TabWindow::windowDeactivated( const css::lang::EventObject& )
throw (css::uno::RuntimeException)
{
}

//---------------------------------------------------------------------------------------------------------
//  XSimpleTabController
//---------------------------------------------------------------------------------------------------------

::sal_Int32 SAL_CALL TabWindow::insertTab()
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    sal_Int32 nNextTabID( m_nNextTabID++ );

    rtl::OUString aTitle;
    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->InsertPage( sal_uInt16( nNextTabID ), aTitle );
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_INSERTED, nNextTabID );

    return nNextTabID;
}

void SAL_CALL TabWindow::removeTab( ::sal_Int32 ID )
throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nCurTabId = pTabControl->GetCurPageId();
        sal_uInt16 nPos      = pTabControl->GetPagePos( sal_uInt16( ID ));
        if ( nPos == TAB_PAGE_NOTFOUND )
            throw css::lang::IndexOutOfBoundsException();
        else
        {
            pTabControl->RemovePage( sal_uInt16( ID ));
            nCurTabId = pTabControl->GetCurPageId();
        }
        aLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_SendNotification( NOTIFY_REMOVED, ID );

        // activate new tab if old tab was active!
        nPos = pTabControl->GetPagePos( sal_uInt16( nCurTabId ));
        if ( nPos != TAB_PAGE_NOTFOUND && nCurTabId != ID )
            activateTab( nCurTabId );
    }
}

void SAL_CALL TabWindow::setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties )
throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nPos = pTabControl->GetPagePos( sal_uInt16( ID ));
        if ( nPos == TAB_PAGE_NOTFOUND )
            throw css::lang::IndexOutOfBoundsException();
        else
        {
            comphelper::SequenceAsHashMap aSeqHashMap( Properties );

            ::rtl::OUString aTitle  = pTabControl->GetPageText( sal_uInt16( ID ));
            sal_Int32       nNewPos = nPos;

            aTitle = aSeqHashMap.getUnpackedValueOrDefault< ::rtl::OUString >(
                                    m_aTitlePropName, aTitle );
            pTabControl->SetPageText( sal_uInt16( ID ), aTitle );
            nNewPos = aSeqHashMap.getUnpackedValueOrDefault< sal_Int32 >(
                                    m_aPosPropName, nNewPos );
            if ( nNewPos != sal_Int32( nPos ))
            {
                nPos = sal_uInt16( nNewPos );
                if ( nPos >= pTabControl->GetPageCount() )
                    nPos = TAB_APPEND;

                pTabControl->RemovePage( sal_uInt16( ID ));
                pTabControl->InsertPage( sal_uInt16( ID ), aTitle, nPos );
            }

            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            aLock.unlock();

            css::uno::Sequence< css::beans::NamedValue > aNamedValueSeq = getTabProps( ID );
            implts_SendNotification( NOTIFY_CHANGED, ID, aNamedValueSeq );
        }
    }
}

css::uno::Sequence< css::beans::NamedValue > SAL_CALL TabWindow::getTabProps( ::sal_Int32 ID )
throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    css::uno::Sequence< css::beans::NamedValue > aNamedValueSeq;

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nPos = pTabControl->GetPagePos( sal_uInt16( ID ));
        if ( nPos == TAB_PAGE_NOTFOUND )
            throw css::lang::IndexOutOfBoundsException();
        else
        {
            rtl::OUString aTitle = pTabControl->GetPageText( sal_uInt16( ID ));
                          nPos   = pTabControl->GetPagePos( sal_uInt16( ID ));

            css::uno::Sequence< css::beans::NamedValue > aSeq( 2 );
            aSeq[0].Name  = m_aTitlePropName;
            aSeq[0].Value = css::uno::makeAny( aTitle );
            aSeq[1].Name  = m_aPosPropName;
            aSeq[1].Value = css::uno::makeAny( sal_Int32( nPos ));
            return aSeq;
        }
    }
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    return aNamedValueSeq;
}

void SAL_CALL TabWindow::activateTab( ::sal_Int32 ID )
throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nPos = pTabControl->GetPagePos( sal_uInt16( ID ));
        if ( nPos == TAB_PAGE_NOTFOUND )
            throw css::lang::IndexOutOfBoundsException();
        else
        {
            sal_Int32 nOldID     = pTabControl->GetCurPageId();
            rtl::OUString aTitle = pTabControl->GetPageText( sal_uInt16( ID ));
            pTabControl->SetCurPageId( sal_uInt16( ID ));
            pTabControl->SelectTabPage( sal_uInt16( ID ));
            impl_SetTitle( aTitle );

            aLock.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */

            if ( nOldID != TAB_PAGE_NOTFOUND )
                implts_SendNotification( NOTIFY_DEACTIVATED, nOldID );
            implts_SendNotification( NOTIFY_ACTIVATED, ID );
        }
    }
}

::sal_Int32 SAL_CALL TabWindow::getActiveTabID()
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nID = pTabControl->GetCurPageId();
        if ( nID == TAB_PAGE_NOTFOUND )
            return -1;
        else
            return sal_Int32( nID );
    }

    return -1;
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL TabWindow::addTabListener(
    const css::uno::Reference< css::awt::XTabListener >& xListener )
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        return;
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.addInterface(
        ::getCppuType( ( const css::uno::Reference< css::awt::XTabListener >* ) NULL ), xListener );
}

void SAL_CALL TabWindow::removeTabListener( const css::uno::Reference< css::awt::XTabListener >& xListener )
throw (css::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    if ( m_bDisposed )
        return;
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.removeInterface(
        ::getCppuType( ( const css::uno::Reference< css::awt::XTabListener >* ) NULL ), xListener );
}

//---------------------------------------------------------------------------------------------------------
//  OPropertySetHelper
//---------------------------------------------------------------------------------------------------------

// XPropertySet helper
sal_Bool SAL_CALL TabWindow::convertFastPropertyValue( css::uno::Any&       aConvertedValue ,
                                                       css::uno::Any&       aOldValue       ,
                                                       sal_Int32            nHandle         ,
                                                       const css::uno::Any& aValue             )
throw( css::lang::IllegalArgumentException )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case TABWINDOW_PROPHANDLE_PARENTWINDOW :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny( m_xContainerWindow ),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case TABWINDOW_PROPHANDLE_TOPWINDOW :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny( m_xTopWindow ),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL TabWindow::setFastPropertyValue_NoBroadcast( sal_Int32,
                                                           const css::uno::Any&)
throw( css::uno::Exception )
{
}

void SAL_CALL TabWindow::getFastPropertyValue( css::uno::Any& aValue  ,
                                               sal_Int32      nHandle    ) const
{
    switch( nHandle )
    {
        case TABWINDOW_PROPHANDLE_PARENTWINDOW:
            aValue <<= m_xContainerWindow;
            break;
        case TABWINDOW_PROPHANDLE_TOPWINDOW:
            aValue <<= m_xTopWindow;
            break;
    }
}

::cppu::IPropertyArrayHelper& SAL_CALL TabWindow::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL TabWindow::getPropertySetInfo()
throw ( css::uno::RuntimeException )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

const css::uno::Sequence< css::beans::Property > TabWindow::impl_getStaticPropertyDescriptor()
{
    // Create property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( TABWINDOW_PROPNAME_PARENTWINDOW,
                                         TABWINDOW_PROPHANDLE_PARENTWINDOW,
                                         ::getCppuType((const css::uno::Reference< css::awt::XWindow >*)NULL),
                                         com::sun::star::beans::PropertyAttribute::READONLY  ),
        com::sun::star::beans::Property( TABWINDOW_PROPNAME_TOPWINDOW,
                                         TABWINDOW_PROPHANDLE_TOPWINDOW,
                                         ::getCppuType((const css::uno::Reference< css::awt::XWindow >*)NULL),
                                         com::sun::star::beans::PropertyAttribute::READONLY  )
    };  // Use it to initialize sequence!
    const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, TABWINDOW_PROPCOUNT );

    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
