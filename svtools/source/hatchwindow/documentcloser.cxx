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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "documentcloser.hxx"

using namespace ::com::sun::star;


// ====================================================================
// MainThreadFrameCloserRequest
// ====================================================================

class MainThreadFrameCloserRequest
{
    uno::Reference< frame::XFrame > m_xFrame;

    public:
        MainThreadFrameCloserRequest( const uno::Reference< frame::XFrame >& xFrame )
        : m_xFrame( xFrame )
        {}

        DECL_STATIC_LINK( MainThreadFrameCloserRequest, worker, MainThreadFrameCloserRequest* );

        static void Start( MainThreadFrameCloserRequest* pRequest );
};

// --------------------------------------------------------
void MainThreadFrameCloserRequest::Start( MainThreadFrameCloserRequest* pMTRequest )
{
    if ( pMTRequest )
    {
        if ( Application::GetMainThreadIdentifier() == osl_getThreadIdentifier( NULL ) )
        {
            // this is the main thread
            worker( NULL, pMTRequest );
        }
        else
            Application::PostUserEvent( STATIC_LINK( NULL, MainThreadFrameCloserRequest, worker ), pMTRequest );
    }
}

// --------------------------------------------------------
IMPL_STATIC_LINK( MainThreadFrameCloserRequest, worker, MainThreadFrameCloserRequest*, pMTRequest )
{
    (void) pThis; // unused
    if ( pMTRequest )
    {
        if ( pMTRequest->m_xFrame.is() )
        {
            // this is the main thread, the solar mutex must be locked
            ::vos::OGuard aGuard( Application::GetSolarMutex() );

            try
            {
                uno::Reference< awt::XWindow > xWindow = pMTRequest->m_xFrame->getContainerWindow();
                uno::Reference< awt::XVclWindowPeer > xWinPeer( xWindow, uno::UNO_QUERY_THROW );

                xWindow->setVisible( sal_False );

                // reparent the window
                xWinPeer->setProperty( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginParent" ) ),
                                        uno::makeAny( (sal_Int64) 0 ) );

                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                    Dialog::EndAllDialogs( pWindow );
            }
            catch( uno::Exception& )
            {
                // ignore all the errors
            }

            try
            {
                uno::Reference< util::XCloseable > xCloseable( pMTRequest->m_xFrame, uno::UNO_QUERY_THROW );
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {
                // ignore all the errors
            }
        }

        delete pMTRequest;
    }

    return 0;
}


// ====================================================================
// ODocumentCloser
// ====================================================================

// --------------------------------------------------------
ODocumentCloser::ODocumentCloser( const uno::Reference< uno::XComponentContext >& xContext )
: m_xContext( xContext )
, m_pListenersContainer( NULL )
, m_bDisposed( sal_False )
, m_bInitialized( sal_False )
{
}

// --------------------------------------------------------
ODocumentCloser::~ODocumentCloser()
{
    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }
}

// XComponent
// --------------------------------------------------------
void SAL_CALL ODocumentCloser::dispose()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    if ( m_pListenersContainer )
        m_pListenersContainer->disposeAndClear( aSource );

    // TODO: trigger a main thread execution to close the frame
    if ( m_xFrame.is() )
    {
        // the created object will be deleted after thread execution
        MainThreadFrameCloserRequest* pCloser = new MainThreadFrameCloserRequest( m_xFrame );
        MainThreadFrameCloserRequest::Start( pCloser );
    }

    m_bDisposed = sal_True;
}

// --------------------------------------------------------
void SAL_CALL ODocumentCloser::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}

// --------------------------------------------------------
void SAL_CALL ODocumentCloser::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

// XInitialization
// --------------------------------------------------------
void SAL_CALL ODocumentCloser::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    sal_Int32 nLen = aArguments.getLength();
    if ( nLen != 1 )
        throw lang::IllegalArgumentException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Wrong count of parameters!" ) ),
                        uno::Reference< uno::XInterface >(),
                        0 );

    if ( !( aArguments[0] >>= m_xFrame ) || !m_xFrame.is() )
        throw lang::IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Nonempty reference is expected as the first argument!" ) ),
                uno::Reference< uno::XInterface >(),
                0 );

    m_bInitialized = sal_True;
}


// XServiceInfo
// --------------------------------------------------------
::rtl::OUString SAL_CALL ODocumentCloser::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return impl_staticGetImplementationName();
}

// --------------------------------------------------------
::sal_Bool SAL_CALL ODocumentCloser::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

// --------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ODocumentCloser::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return impl_staticGetSupportedServiceNames();
}

// Static methods
// --------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL ODocumentCloser::impl_staticGetSupportedServiceNames()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.DocumentCloser" ) );
    return uno::Sequence< rtl::OUString >( &aServiceName, 1 );
}

// --------------------------------------------------------
::rtl::OUString SAL_CALL ODocumentCloser::impl_staticGetImplementationName()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.embed.DocumentCloser" ) );
}

// --------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL ODocumentCloser::impl_staticCreateSelfInstance(
                                const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    uno::Reference< uno::XComponentContext > xContext;
    uno::Reference< beans::XPropertySet > xPropSet( xServiceManager, uno::UNO_QUERY );
    if ( xPropSet.is() )
        xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ) >>= xContext;

    if ( !xContext.is() )
    {
        throw uno::RuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unable to obtain component context from service manager!" ) ),
            uno::Reference< uno::XInterface >() );
    }

    return static_cast< cppu::OWeakObject * >( new ODocumentCloser( xContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
