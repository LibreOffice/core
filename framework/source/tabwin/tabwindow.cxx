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

#include <tabwin/tabwindow.hxx>
#include <properties.h>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/sequenceashashmap.hxx>

//  Defines

using namespace com::sun::star;

namespace framework
{

//  XInterface, XTypeProvider, XServiceInfo

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

TabWindow::TabWindow( const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aMutex )
    , ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
    , m_bInitialized( false )
    , m_bDisposed( false )
    , m_nNextTabID( 1 )
    , m_aTitlePropName( "Title" )
    , m_aPosPropName( "Position" )
    , m_xContext( xContext )
    , m_aListenerContainer( m_aMutex )
{
}

TabWindow::~TabWindow()
{
}

// Helper

void TabWindow::implts_LayoutWindows() const
{
    const sal_Int32 nTabControlHeight = 30;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    css::uno::Reference< css::awt::XDevice > xDevice( m_xTopWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xWindow( m_xTopWindow, css::uno::UNO_QUERY );
    css::uno::Reference< css::awt::XWindow > xTabControlWindow( m_xTabControlWindow );
    css::uno::Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Convert relative size to output size.
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
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( rTabControlWindow );
    if ( pWindow )
        return static_cast<TabControl *>(pWindow.get());
    else
        return nullptr;
}

void TabWindow::impl_SetTitle( const OUString& rTitle )
{
    if ( m_xTopWindow.is() )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(
                            css::uno::Reference< css::awt::XWindow >(
                                m_xTopWindow, css::uno::UNO_QUERY ));
        if ( pWindow )
            pWindow->SetText( rTitle );
    }
}

void TabWindow::implts_SendNotification( Notification eNotify, sal_Int32 ID ) const
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer(
                                                        cppu::UnoType<css::awt::XTabListener>::get());
    if (pContainer!=nullptr)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                switch ( eNotify )
                {
                    case NOTIFY_INSERTED:
                        static_cast<css::awt::XTabListener*>(pIterator.next())->inserted( ID );
                        break;
                    case NOTIFY_REMOVED:
                        static_cast<css::awt::XTabListener*>(pIterator.next())->removed( ID );
                        break;
                    case NOTIFY_ACTIVATED:
                        static_cast<css::awt::XTabListener*>(pIterator.next())->activated( ID );
                        break;
                    case NOTIFY_DEACTIVATED:
                        static_cast<css::awt::XTabListener*>(pIterator.next())->deactivated( ID );
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
                                                        cppu::UnoType<css::awt::XTabListener>::get());
    if (pContainer!=nullptr)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                switch ( eNotify )
                {
                    case NOTIFY_CHANGED:
                        static_cast<css::awt::XTabListener*>(pIterator.next())->changed( ID, rSeq );
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

// Links

IMPL_LINK( TabWindow, Activate, TabControl*, pTabControl, void )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;

    sal_Int32 nPageId = pTabControl->GetCurPageId();

    OUString aTitle = pTabControl->GetPageText( sal_uInt16( nPageId ));
    impl_SetTitle( aTitle );
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_ACTIVATED, nPageId );
}

IMPL_LINK( TabWindow, Deactivate, TabControl*, pTabControl, bool )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    sal_Int32 nPageId = pTabControl->GetCurPageId();
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_DEACTIVATED, nPageId );

    return true;
}

// XInitialization

void SAL_CALL TabWindow::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    const OUString aTopWindowArgName( "TopWindow" );
    const OUString aSizeArgName( "Size" );

    css::awt::Size aDefaultSize( 500, 500 );
    css::awt::Size aSize( aDefaultSize );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aLock;
    bool                                               bInitalized( m_bInitialized );
    css::uno::Reference< css::uno::XComponentContext >     xContext( m_xContext );
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( !bInitalized )
    {
        css::beans::PropertyValue                   aPropValue;
        css::uno::Reference< css::awt::XTopWindow > xTopWindow;
        css::uno::Reference< css::awt::XToolkit2 >  xToolkit;
        css::awt::WindowDescriptor                  aDescriptor;

        if ( xContext.is() )
        {
            try
            {
                xToolkit = css::awt::Toolkit::create( xContext );
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
                aDescriptor.Parent.clear();
                aDescriptor.Bounds              =   css::awt::Rectangle( 0, 0, aSize.Width, aSize.Height );
                aDescriptor.WindowAttributes    =   0;

                try
                {
                    xTopWindow.set( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );
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
                aLock.reset();
                m_bInitialized = true;
                aLock.clear();
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
                aDescriptor.Parent.set( xTopWindow, css::uno::UNO_QUERY );
                aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0);
                aDescriptor.WindowAttributes    =   0;

                xContainerWindow.set( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );

                // create a tab control window properties
                aDescriptor.Type                = css::awt::WindowClass_SIMPLE;
                aDescriptor.WindowServiceName   = "tabcontrol";
                aDescriptor.ParentIndex         = -1;
                aDescriptor.Parent.set( xTopWindow, css::uno::UNO_QUERY );
                aDescriptor.Bounds              = css::awt::Rectangle( 0,0,0,0 );
                aDescriptor.WindowAttributes    = 0;

                xTabControl.set( xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );

                if ( xContainerWindow.is() && xTabControl.is() )
                {
                    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                    aLock.reset();
                    m_xTopWindow = xTopWindow;
                    m_xContainerWindow = xContainerWindow;
                    m_xTabControlWindow = xTabControl;
                    aLock.clear();
                    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

                    xWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );

                    SolarMutexGuard aGuard;
                    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if( pWindow )
                        pWindow->Show();

                    pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
                    if ( pWindow )
                        pWindow->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate  );

                    pWindow = VCLUnoHelper::GetWindow( xTabControl );
                    if ( pWindow )
                    {
                        pWindow->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate  );
                        TabControl* pTabControl = static_cast<TabControl *>(pWindow.get());
                        pTabControl->SetActivatePageHdl( LINK( this, TabWindow, Activate ));
                        pTabControl->SetDeactivatePageHdl( LINK( this, TabWindow, Deactivate ));
                    }

                    implts_LayoutWindows();
                }
            }
        }
    }
}

//  XComponent

void SAL_CALL TabWindow::dispose()
{
    // Send message to all listener and forget her references.
    css::uno::Reference< css::lang::XComponent > xThis(
        static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
    css::lang::EventObject aEvent( xThis );

    m_aListenerContainer.disposeAndClear( aEvent );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexResettableGuard aLock;
    css::uno::Reference< css::awt::XWindow > xTabControlWindow( m_xTabControlWindow );
    css::uno::Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    css::uno::Reference< css::awt::XTopWindow > xTopWindow( m_xTopWindow );
    m_xTabControlWindow.clear();
    m_xContainerWindow.clear();
    m_xTopWindow.clear();
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    css::uno::Reference< css::lang::XComponent > xComponent( xTabControlWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    xComponent.set( xContainerWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    xComponent.set( xTopWindow, css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    aLock.reset();
    m_bDisposed = true;
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL TabWindow::addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    if ( m_bDisposed )
        return;
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.addInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

void SAL_CALL TabWindow::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    if ( m_bDisposed )
        return;
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.removeInterface( cppu::UnoType<css::lang::XEventListener>::get(), xListener );
}

// XEventListener

void SAL_CALL TabWindow::disposing( const css::lang::EventObject& )
{
}

// XWindowListener

void SAL_CALL TabWindow::windowResized( const css::awt::WindowEvent& )
{
    implts_LayoutWindows();
}

void SAL_CALL TabWindow::windowMoved( const css::awt::WindowEvent& )
{
}

void SAL_CALL TabWindow::windowShown( const css::lang::EventObject& )
{
    SolarMutexGuard g;

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->Show();

    if ( m_xContainerWindow.is() )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pWindow )
            pWindow->Show();
    }
}

void SAL_CALL TabWindow::windowHidden( const css::lang::EventObject& )
{
    SolarMutexGuard g;
    if ( m_xContainerWindow.is() )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pWindow )
            pWindow->Hide();
    }

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->Hide();
}

// XTopWindowListener

void SAL_CALL TabWindow::windowOpened( const css::lang::EventObject& )
{
}

void SAL_CALL TabWindow::windowClosing( const css::lang::EventObject& )
{
    css::uno::Reference< css::lang::XComponent > xComponent( static_cast<OWeakObject *>(this), css::uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
}

void SAL_CALL TabWindow::windowClosed( const css::lang::EventObject& )
{
}

void SAL_CALL TabWindow::windowMinimized( const css::lang::EventObject& )
{
}

void SAL_CALL TabWindow::windowNormalized( const css::lang::EventObject& )
{
}

void SAL_CALL TabWindow::windowActivated( const css::lang::EventObject& )
{
}

void SAL_CALL TabWindow::windowDeactivated( const css::lang::EventObject& )
{
}

//  XSimpleTabController

::sal_Int32 SAL_CALL TabWindow::insertTab()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    sal_Int32 nNextTabID( m_nNextTabID++ );

    OUString aTitle;
    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
        pTabControl->InsertPage( sal_uInt16( nNextTabID ), aTitle );
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_SendNotification( NOTIFY_INSERTED, nNextTabID );

    return nNextTabID;
}

void SAL_CALL TabWindow::removeTab( ::sal_Int32 ID )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    TabControl* pTabControl = impl_GetTabControl( m_xTabControlWindow );
    if ( pTabControl )
    {
        sal_uInt16 nPos      = pTabControl->GetPagePos( sal_uInt16( ID ));
        if ( nPos == TAB_PAGE_NOTFOUND )
            throw css::lang::IndexOutOfBoundsException();
        pTabControl->RemovePage( sal_uInt16( ID ));
        sal_uInt16 nCurTabId = pTabControl->GetCurPageId();
        aLock.clear();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_SendNotification( NOTIFY_REMOVED, ID );

        // activate new tab if old tab was active!
        nPos = pTabControl->GetPagePos( sal_uInt16( nCurTabId ));
        if ( nPos != TAB_PAGE_NOTFOUND && nCurTabId != ID )
            activateTab( nCurTabId );
    }
}

void SAL_CALL TabWindow::setTabProps( ::sal_Int32 ID, const css::uno::Sequence< css::beans::NamedValue >& Properties )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;

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

            OUString aTitle  = pTabControl->GetPageText( sal_uInt16( ID ));
            sal_Int32       nNewPos = nPos;

            aTitle = aSeqHashMap.getUnpackedValueOrDefault< OUString >(
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
            aLock.clear();

            css::uno::Sequence< css::beans::NamedValue > aNamedValueSeq = getTabProps( ID );
            implts_SendNotification( NOTIFY_CHANGED, ID, aNamedValueSeq );
        }
    }
}

css::uno::Sequence< css::beans::NamedValue > SAL_CALL TabWindow::getTabProps( ::sal_Int32 ID )
{
    SolarMutexGuard g;

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
            OUString aTitle = pTabControl->GetPageText( sal_uInt16( ID ));
                          nPos   = pTabControl->GetPagePos( sal_uInt16( ID ));

            css::uno::Sequence< css::beans::NamedValue > aSeq
            {
                { m_aTitlePropName, css::uno::makeAny( aTitle ) },
                { m_aPosPropName,   css::uno::makeAny( sal_Int32( nPos )) }
            };
            return aSeq;
        }
    }

    return aNamedValueSeq;
}

void SAL_CALL TabWindow::activateTab( ::sal_Int32 ID )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;

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
            OUString aTitle = pTabControl->GetPageText( sal_uInt16( ID ));
            pTabControl->SetCurPageId( sal_uInt16( ID ));
            pTabControl->SelectTabPage( sal_uInt16( ID ));
            impl_SetTitle( aTitle );

            aLock.clear();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */

            if ( nOldID != TAB_PAGE_NOTFOUND )
                implts_SendNotification( NOTIFY_DEACTIVATED, nOldID );
            implts_SendNotification( NOTIFY_ACTIVATED, ID );
        }
    }
}

::sal_Int32 SAL_CALL TabWindow::getActiveTabID()
{
    SolarMutexGuard g;

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
}

void SAL_CALL TabWindow::addTabListener(
    const css::uno::Reference< css::awt::XTabListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    if ( m_bDisposed )
        return;
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.addInterface(
        cppu::UnoType<css::awt::XTabListener>::get(), xListener );
}

void SAL_CALL TabWindow::removeTabListener( const css::uno::Reference< css::awt::XTabListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aLock;
    if ( m_bDisposed )
        return;
    aLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    m_aListenerContainer.removeInterface(
        cppu::UnoType<css::awt::XTabListener>::get(), xListener );
}

//  OPropertySetHelper

// XPropertySet helper
sal_Bool SAL_CALL TabWindow::convertFastPropertyValue( css::uno::Any&       aConvertedValue ,
                                                       css::uno::Any&       aOldValue       ,
                                                       sal_Int32            nHandle         ,
                                                       const css::uno::Any& aValue             )
{
    //  Initialize state with sal_False !!!
    //  (Handle can be invalid)
    bool bReturn = false;

    switch( nHandle )
    {
        case TABWINDOW_PROPHANDLE_PARENTWINDOW :
            bReturn = PropHelper::willPropertyBeChanged(
                        css::uno::makeAny( m_xContainerWindow ),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;

        case TABWINDOW_PROPHANDLE_TOPWINDOW :
            bReturn = PropHelper::willPropertyBeChanged(
                        css::uno::makeAny( m_xTopWindow ),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn;
}

void SAL_CALL TabWindow::setFastPropertyValue_NoBroadcast( sal_Int32,
                                                           const css::uno::Any&)
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
    static ::cppu::OPropertyArrayHelper* pInfoHelper = nullptr;

    if( pInfoHelper == nullptr )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );

        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == nullptr )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            // "impl_getStaticPropertyDescriptor" is a non exported and static function, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), true );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL TabWindow::getPropertySetInfo()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static css::uno::Reference< css::beans::XPropertySetInfo >* pInfo = nullptr;

    if( pInfo == nullptr )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == nullptr )
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

    const css::beans::Property pProperties[] =
    {
        css::beans::Property( TABWINDOW_PROPNAME_PARENTWINDOW,
                                         TABWINDOW_PROPHANDLE_PARENTWINDOW,
                                         cppu::UnoType<css::awt::XWindow>::get(),
                                         css::beans::PropertyAttribute::READONLY  ),
        css::beans::Property( TABWINDOW_PROPNAME_TOPWINDOW,
                                         TABWINDOW_PROPHANDLE_TOPWINDOW,
                                         cppu::UnoType<css::awt::XWindow>::get(),
                                         css::beans::PropertyAttribute::READONLY  )
    };  // Use it to initialize sequence!
    const css::uno::Sequence< css::beans::Property > lPropertyDescriptor( pProperties, TABWINDOW_PROPCOUNT );

    // Return "PropertyDescriptor"
    return lPropertyDescriptor;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
