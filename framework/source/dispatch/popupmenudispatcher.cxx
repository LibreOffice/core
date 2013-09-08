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

#include <dispatch/popupmenudispatcher.hxx>
#include <general.h>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#include <ucbhelper/content.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star                ;
using namespace ::com::sun::star::awt           ;
using namespace ::com::sun::star::beans         ;
using namespace ::com::sun::star::container     ;
using namespace ::com::sun::star::frame         ;
using namespace ::com::sun::star::lang          ;
using namespace ::com::sun::star::uno           ;
using namespace ::com::sun::star::util          ;
using namespace ::cppu                          ;
using namespace ::osl                           ;
using namespace ::rtl                           ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
PopupMenuDispatcher::PopupMenuDispatcher(
    const uno::Reference< XComponentContext >& xContext )
        //  Init baseclasses first
        :   ThreadHelpBase          ( &Application::GetSolarMutex()  )
        // Init member
        ,   m_xContext              ( xContext                       )
        ,   m_aListenerContainer    ( m_aLock.getShareableOslMutex() )
        ,   m_bAlreadyDisposed      ( sal_False                      )
        ,   m_bActivateListener     ( sal_False                      )
{
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
PopupMenuDispatcher::~PopupMenuDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

OUString SAL_CALL PopupMenuDispatcher::getImplementationName() throw( css::uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

sal_Bool SAL_CALL PopupMenuDispatcher::supportsService( const OUString& sServiceName )
  throw( css::uno::RuntimeException )
{
    return ::comphelper::findValue(getSupportedServiceNames(), sServiceName, sal_True).getLength() != 0;
}

css::uno::Sequence< OUString > SAL_CALL PopupMenuDispatcher::getSupportedServiceNames()
  throw( css::uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

css::uno::Sequence< OUString > PopupMenuDispatcher::impl_getStaticSupportedServiceNames()
{
    css::uno::Sequence< OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = SERVICENAME_PROTOCOLHANDLER;
    return seqServiceNames;
}

OUString PopupMenuDispatcher::impl_getStaticImplementationName()
{
    return IMPLEMENTATIONNAME_POPUPMENUDISPATCHER;
}

css::uno::Reference< css::uno::XInterface >
SAL_CALL PopupMenuDispatcher::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )
throw( css::uno::Exception )
{
    /* create new instance of service */
    PopupMenuDispatcher* pClass = new PopupMenuDispatcher( comphelper::getComponentContext(xServiceManager) );
    /* hold it alive by increasing his ref count!!! */
    css::uno::Reference< css::uno::XInterface > xService( static_cast< ::cppu::OWeakObject* >(pClass), css::uno::UNO_QUERY );
    /* initialize new service instance ... he can use his own refcount ... we hold it! */
    pClass->impl_initService();
    /* return new created service as reference */
    return xService;
}

css::uno::Reference< css::lang::XSingleServiceFactory >
PopupMenuDispatcher::impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager )
{
    css::uno::Reference< css::lang::XSingleServiceFactory > xReturn (
       cppu::createSingleFactory ( xServiceManager,
                                PopupMenuDispatcher::impl_getStaticImplementationName()   ,
                                PopupMenuDispatcher::impl_createInstance                  ,
                                PopupMenuDispatcher::impl_getStaticSupportedServiceNames() )
                                                                    );
    return xReturn;
}

DEFINE_INIT_SERVICE(PopupMenuDispatcher,
{
    /*Attention
    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
    to create a new instance of this class by our own supported service factory.
    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
    */
}
)

//*****************************************************************************************************************
//  XInitialization
//*****************************************************************************************************************
void SAL_CALL PopupMenuDispatcher::initialize(
    const css::uno::Sequence< css::uno::Any >& lArguments )
throw( css::uno::Exception, css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    for (int a=0; a<lArguments.getLength(); ++a)
    {
        if (a==0)
        {
            lArguments[a] >>= xFrame;
            m_xWeakFrame = xFrame;

            m_bActivateListener = sal_True;
            uno::Reference< css::frame::XFrameActionListener > xFrameActionListener(
                (OWeakObject *)this, css::uno::UNO_QUERY );
            xFrame->addFrameActionListener( xFrameActionListener );
        }
    }

    aWriteLock.unlock();
    /* } SAFE */
}

//*****************************************************************************************************************
//  XDispatchProvider
//*****************************************************************************************************************
css::uno::Reference< css::frame::XDispatch >
SAL_CALL PopupMenuDispatcher::queryDispatch(
    const css::util::URL&  rURL    ,
    const OUString& sTarget ,
    sal_Int32              nFlags  )
throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( rURL.Complete.startsWith( "vnd.sun.star.popup:" ) )
    {
        // --- SAFE ---
        ResetableGuard aGuard( m_aLock );
        impl_RetrievePopupControllerQuery();
        impl_CreateUriRefFactory();

        css::uno::Reference< css::container::XNameAccess > xPopupCtrlQuery( m_xPopupCtrlQuery );
        css::uno::Reference< css::uri::XUriReferenceFactory > xUriRefFactory( m_xUriRefFactory );
        aGuard.unlock();
        // --- SAFE ---

        if ( xPopupCtrlQuery.is() )
        {
            try
            {
                // Just use the main part of the URL for popup menu controllers
                sal_Int32     nQueryPart( 0 );
                sal_Int32     nSchemePart( 0 );
                OUString aBaseURL( "vnd.sun.star.popup:" );
                OUString aURL( rURL.Complete );

                nSchemePart = aURL.indexOf( ':' );
                if (( nSchemePart > 0 ) &&
                    ( aURL.getLength() > ( nSchemePart+1 )))
                {
                    nQueryPart  = aURL.indexOf( '?', nSchemePart );
                    if ( nQueryPart > 0 )
                        aBaseURL += aURL.copy( nSchemePart+1, nQueryPart-(nSchemePart+1) );
                    else if ( nQueryPart == -1 )
                        aBaseURL += aURL.copy( nSchemePart+1 );
                }

                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider;

                // Find popup menu controller using the base URL
                xPopupCtrlQuery->getByName( aBaseURL ) >>= xDispatchProvider;
                aGuard.unlock();

                // Ask popup menu dispatch provider for dispatch object
                if ( xDispatchProvider.is() )
                    xDispatch = xDispatchProvider->queryDispatch( rURL, sTarget, nFlags );
            }
            catch ( const RuntimeException& )
            {
                throw;
            }
            catch ( const Exception& )
            {
            }
        }
    }
    return xDispatch;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
PopupMenuDispatcher::queryDispatches(
    const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = this->queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::dispatch(
    const URL&                        /*aURL*/            ,
    const Sequence< PropertyValue >&  /*seqProperties*/ )
throw( RuntimeException )
{
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::addStatusListener(
    const uno::Reference< XStatusListener >& xControl,
    const URL&                          aURL    )
throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Add listener to container.
    m_aListenerContainer.addInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//  XDispatch
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::removeStatusListener(
    const uno::Reference< XStatusListener >& xControl,
    const URL&                          aURL    )
throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    // Add listener to container.
    m_aListenerContainer.removeInterface( aURL.Complete, xControl );
}

//*****************************************************************************************************************
//   XFrameActionListener
//*****************************************************************************************************************

void
SAL_CALL PopupMenuDispatcher::frameAction(
    const FrameActionEvent& aEvent )
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if (( aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING ) ||
        ( aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ))
    {
        // Reset query reference to requery it again next time
        m_xPopupCtrlQuery.clear();
    }
}

//*****************************************************************************************************************
//   XEventListener
//*****************************************************************************************************************
void
SAL_CALL PopupMenuDispatcher::disposing( const EventObject& ) throw( RuntimeException )
{
    // Ready for multithreading
    ResetableGuard aGuard( m_aLock );
    // Safe impossible cases
    SAL_WARN_IF( m_bAlreadyDisposed, "fwk", "MenuDispatcher::disposing(): Object already disposed .. don't call it again!" );

    if( m_bAlreadyDisposed == sal_False )
    {
        m_bAlreadyDisposed = sal_True;

        if ( m_bActivateListener )
        {
            uno::Reference< XFrame > xFrame( m_xWeakFrame.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                xFrame->removeFrameActionListener( uno::Reference< XFrameActionListener >( (OWeakObject *)this, UNO_QUERY ));
                m_bActivateListener = sal_False;
            }
        }

        // Forget our factory.
        m_xContext = uno::Reference< XComponentContext >();
    }
}

void PopupMenuDispatcher::impl_RetrievePopupControllerQuery()
{
    if ( !m_xPopupCtrlQuery.is() )
    {
        css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
        css::uno::Reference< css::frame::XFrame > xFrame( m_xWeakFrame );

        if ( xFrame.is() )
        {
            css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
            if ( xPropSet.is() )
            {
                try
                {
                    xPropSet->getPropertyValue( FRAME_PROPNAME_LAYOUTMANAGER ) >>= xLayoutManager;

                    if ( xLayoutManager.is() )
                    {
                        css::uno::Reference< css::ui::XUIElement > xMenuBar;
                        OUString aMenuBar( "private:resource/menubar/menubar" );
                        xMenuBar = xLayoutManager->getElement( aMenuBar );

                        m_xPopupCtrlQuery = css::uno::Reference< css::container::XNameAccess >(
                                                xMenuBar, css::uno::UNO_QUERY );
                    }
                }
                catch ( const css::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( const css::uno::Exception& )
                {
                }
            }
        }
    }
}

void PopupMenuDispatcher::impl_CreateUriRefFactory()
{
    if ( !m_xUriRefFactory.is() )
    {
        m_xUriRefFactory = css::uri::UriReferenceFactory::create( m_xContext );
    }
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
