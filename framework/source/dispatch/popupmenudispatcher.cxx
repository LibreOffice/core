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
#include <services.h>
#include <properties.h>

#include <com/sun/star/frame/XLayoutManager2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/ui/XUIElement.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>

namespace framework{

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::cppu;
using namespace ::osl;

PopupMenuDispatcher::PopupMenuDispatcher(
    const uno::Reference< XComponentContext >& xContext )
        :   m_xContext              ( xContext                       )
        ,   m_bAlreadyDisposed      ( false                      )
        ,   m_bActivateListener     ( false                      )
{
}

PopupMenuDispatcher::~PopupMenuDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

OUString SAL_CALL PopupMenuDispatcher::getImplementationName()
{
    return "com.sun.star.comp.framework.PopupMenuControllerDispatcher";
}

sal_Bool SAL_CALL PopupMenuDispatcher::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PopupMenuDispatcher::getSupportedServiceNames()
{
    return { SERVICENAME_PROTOCOLHANDLER };
}

void SAL_CALL PopupMenuDispatcher::initialize( const css::uno::Sequence< css::uno::Any >& lArguments )
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    SolarMutexGuard g;
    for (int a=0; a<lArguments.getLength(); ++a)
    {
        if (a==0)
        {
            lArguments[a] >>= xFrame;
            m_xWeakFrame = xFrame;

            m_bActivateListener = true;
            uno::Reference< css::frame::XFrameActionListener > xFrameActionListener(
                static_cast<OWeakObject *>(this), css::uno::UNO_QUERY );
            xFrame->addFrameActionListener( xFrameActionListener );
        }
    }
}

css::uno::Reference< css::frame::XDispatch >
SAL_CALL PopupMenuDispatcher::queryDispatch(
    const css::util::URL&  rURL    ,
    const OUString& sTarget ,
    sal_Int32              nFlags  )
{
    css::uno::Reference< css::frame::XDispatch > xDispatch;

    if ( rURL.Complete.startsWith( "vnd.sun.star.popup:" ) )
    {
        // --- SAFE ---
        SolarMutexClearableGuard aGuard;
        impl_RetrievePopupControllerQuery();
        if ( !m_xUriRefFactory.is() )
            m_xUriRefFactory = css::uri::UriReferenceFactory::create( m_xContext );

        css::uno::Reference< css::container::XNameAccess > xPopupCtrlQuery( m_xPopupCtrlQuery );
        aGuard.clear();
        // --- SAFE ---

        if ( xPopupCtrlQuery.is() )
        {
            try
            {
                // Just use the main part of the URL for popup menu controllers
                sal_Int32     nSchemePart( 0 );
                OUString aBaseURL( "vnd.sun.star.popup:" );
                OUString aURL( rURL.Complete );

                nSchemePart = aURL.indexOf( ':' );
                if (( nSchemePart > 0 ) &&
                    ( aURL.getLength() > ( nSchemePart+1 )))
                {
                    sal_Int32 nQueryPart  = aURL.indexOf( '?', nSchemePart );
                    if ( nQueryPart > 0 )
                        aBaseURL += aURL.copy( nSchemePart+1, nQueryPart-(nSchemePart+1) );
                    else if ( nQueryPart == -1 )
                        aBaseURL += aURL.copy( nSchemePart+1 );
                }

                css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider;

                // Find popup menu controller using the base URL
                xPopupCtrlQuery->getByName( aBaseURL ) >>= xDispatchProvider;
                aGuard.clear();

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
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

void SAL_CALL PopupMenuDispatcher::dispatch( const URL& /*aURL*/, const Sequence< PropertyValue >& /*seqProperties*/ )
{
}

void SAL_CALL PopupMenuDispatcher::addStatusListener( const uno::Reference< XStatusListener >& /*xControl*/,
                                                      const URL& /*aURL*/ )
{
}

void SAL_CALL PopupMenuDispatcher::removeStatusListener( const uno::Reference< XStatusListener >& /*xControl*/,
                                                         const URL& /*aURL*/ )
{
}

void SAL_CALL PopupMenuDispatcher::frameAction( const FrameActionEvent& aEvent )
{
    SolarMutexGuard g;
    if (( aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING ) ||
        ( aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ))
    {
        // Reset query reference to requery it again next time
        m_xPopupCtrlQuery.clear();
    }
}

void SAL_CALL PopupMenuDispatcher::disposing( const EventObject& )
{
    SolarMutexGuard g;
    // Safe impossible cases
    SAL_WARN_IF( m_bAlreadyDisposed, "fwk", "MenuDispatcher::disposing(): Object already disposed .. don't call it again!" );

    if( m_bAlreadyDisposed )
        return;

    m_bAlreadyDisposed = true;

    if ( m_bActivateListener )
    {
        uno::Reference< XFrame > xFrame( m_xWeakFrame.get(), UNO_QUERY );
        if ( xFrame.is() )
        {
            xFrame->removeFrameActionListener( uno::Reference< XFrameActionListener >( static_cast<OWeakObject *>(this), UNO_QUERY ));
            m_bActivateListener = false;
        }
    }

    // Forget our factory.
    m_xContext.clear();
}

void PopupMenuDispatcher::impl_RetrievePopupControllerQuery()
{
    if ( m_xPopupCtrlQuery.is() )
        return;

    css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
    css::uno::Reference< css::frame::XFrame > xFrame( m_xWeakFrame );

    if ( !xFrame.is() )
        return;

    css::uno::Reference< css::beans::XPropertySet > xPropSet( xFrame, css::uno::UNO_QUERY );
    if ( !xPropSet.is() )
        return;

    try
    {
        xPropSet->getPropertyValue( FRAME_PROPNAME_ASCII_LAYOUTMANAGER ) >>= xLayoutManager;

        if ( xLayoutManager.is() )
        {
            css::uno::Reference< css::ui::XUIElement > xMenuBar = xLayoutManager->getElement( "private:resource/menubar/menubar" );

            m_xPopupCtrlQuery.set( xMenuBar, css::uno::UNO_QUERY );
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

} //  namespace framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_PopupMenuDispatcher_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::PopupMenuDispatcher(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
