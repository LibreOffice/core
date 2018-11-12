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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <tools/link.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace ::com::sun::star;

namespace {

// the service is implemented as a wrapper to be able to die by refcount
// the disposing mechanics is required for java related scenarios
class ODocumentCloser : public ::cppu::WeakImplHelper< css::lang::XComponent,
                                                        css::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    std::unique_ptr<::comphelper::OInterfaceContainerHelper2> m_pListenersContainer; // list of listeners

    bool m_bDisposed;

public:
    explicit ODocumentCloser(const css::uno::Sequence< css::uno::Any >& aArguments);

// XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

class MainThreadFrameCloserRequest
{
    uno::Reference< frame::XFrame > m_xFrame;

    public:
        explicit MainThreadFrameCloserRequest( const uno::Reference< frame::XFrame >& xFrame )
        : m_xFrame( xFrame )
        {}

        DECL_STATIC_LINK( MainThreadFrameCloserRequest, worker, void*, void );

        static void Start( MainThreadFrameCloserRequest* pRequest );
};


void MainThreadFrameCloserRequest::Start( MainThreadFrameCloserRequest* pMTRequest )
{
    if ( pMTRequest )
    {
        if ( Application::IsMainThread() )
        {
            // this is the main thread
            worker( nullptr, pMTRequest );
        }
        else
            Application::PostUserEvent( LINK( nullptr, MainThreadFrameCloserRequest, worker ), pMTRequest );
    }
}


IMPL_STATIC_LINK( MainThreadFrameCloserRequest, worker, void*, p, void )
{
    MainThreadFrameCloserRequest* pMTRequest = static_cast<MainThreadFrameCloserRequest*>(p);
    if ( !pMTRequest )
        return;

    if ( pMTRequest->m_xFrame.is() )
    {
        // this is the main thread, the solar mutex must be locked
        SolarMutexGuard aGuard;

        try
        {
            uno::Reference< awt::XWindow > xWindow = pMTRequest->m_xFrame->getContainerWindow();
            uno::Reference< awt::XVclWindowPeer > xWinPeer( xWindow, uno::UNO_QUERY_THROW );

            xWindow->setVisible( false );

            // reparent the window
            xWinPeer->setProperty( "PluginParent", uno::makeAny( sal_Int64(0) ) );

            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
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
            xCloseable->close( true );
        }
        catch( uno::Exception& )
        {
            // ignore all the errors
        }
    }

    delete pMTRequest;
}

ODocumentCloser::ODocumentCloser(const css::uno::Sequence< css::uno::Any >& aArguments)
: m_bDisposed( false )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    sal_Int32 nLen = aArguments.getLength();
    if ( nLen != 1 )
        throw lang::IllegalArgumentException(
                        "Wrong count of parameters!",
                        uno::Reference< uno::XInterface >(),
                        0 );

    if ( !( aArguments[0] >>= m_xFrame ) || !m_xFrame.is() )
        throw lang::IllegalArgumentException(
                "Nonempty reference is expected as the first argument!",
                uno::Reference< uno::XInterface >(),
                0 );
}


// XComponent

void SAL_CALL ODocumentCloser::dispose()
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

    m_bDisposed = true;
}


void SAL_CALL ODocumentCloser::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pListenersContainer )
        m_pListenersContainer.reset( new ::comphelper::OInterfaceContainerHelper2( m_aMutex ) );

    m_pListenersContainer->addInterface( xListener );
}


void SAL_CALL ODocumentCloser::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

// XServiceInfo
OUString SAL_CALL ODocumentCloser::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.embed.DocumentCloser" );
}

sal_Bool SAL_CALL ODocumentCloser::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL ODocumentCloser::getSupportedServiceNames()
{
    const OUString aServiceName( "com.sun.star.embed.DocumentCloser" );
    return uno::Sequence< OUString >( &aServiceName, 1 );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_embed_DocumentCloser_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new ODocumentCloser(arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
