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


#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper_module.hxx>
#include <comphelper_services.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "officerestartmanager.hxx"

using namespace ::com::sun::star;

namespace comphelper
{


uno::Sequence< OUString > SAL_CALL OOfficeRestartManager::getSupportedServiceNames_static()
{
    uno::Sequence< OUString > aResult( 1 );
    aResult[0] = getServiceName_static();
    return aResult;
}


OUString SAL_CALL OOfficeRestartManager::getImplementationName_static()
{
    return OUString( "com.sun.star.comp.task.OfficeRestartManager" );
}


OUString SAL_CALL OOfficeRestartManager::getSingletonName_static()
{
    return OUString( "com.sun.star.task.OfficeRestartManager" );
}


OUString SAL_CALL OOfficeRestartManager::getServiceName_static()
{
    return OUString( "com.sun.star.comp.task.OfficeRestartManager" );
}


uno::Reference< uno::XInterface > SAL_CALL OOfficeRestartManager::Create( const uno::Reference< uno::XComponentContext >& rxContext )
{
    return static_cast< cppu::OWeakObject* >( new OOfficeRestartManager( rxContext ) );
}

// XRestartManager

void SAL_CALL OOfficeRestartManager::requestRestart( const uno::Reference< task::XInteractionHandler >& /* xInteractionHandler */ )
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // if the restart already running there is no need to trigger it again
        if ( m_bRestartRequested )
            return;

        m_bRestartRequested = true;

        // the office is still not initialized, no need to terminate, changing the state is enough
        if ( !m_bOfficeInitialized )
            return;
    }

    // TODO: use InteractionHandler to report errors
    try
    {
        // register itself as a job that should be executed asynchronously
        uno::Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_SET_THROW );

        uno::Reference< awt::XRequestCallback > xRequestCallback(
            xFactory->createInstanceWithContext(
                 "com.sun.star.awt.AsyncCallback",
                 m_xContext ),
             uno::UNO_QUERY_THROW );

        xRequestCallback->addCallback( this, uno::Any() );
    }
    catch ( uno::Exception& )
    {
        // the try to request restart has failed
        m_bRestartRequested = false;
    }
}


sal_Bool SAL_CALL OOfficeRestartManager::isRestartRequested( sal_Bool bOfficeInitialized )
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( bOfficeInitialized && !m_bOfficeInitialized )
        m_bOfficeInitialized = bOfficeInitialized;

    return m_bRestartRequested;
}

// XCallback

void SAL_CALL OOfficeRestartManager::notify( const uno::Any& /* aData */ )
    throw ( uno::RuntimeException, std::exception )
{
    try
    {
        bool bSuccess = false;

        if ( m_xContext.is() )
        {
            uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xContext);

            // Turn Quickstarter veto off
            uno::Reference< beans::XPropertySet > xPropertySet( xDesktop, uno::UNO_QUERY_THROW );
            OUString aVetoPropName( "SuspendQuickstartVeto" );
            uno::Any aValue;
            aValue <<= true;
            xPropertySet->setPropertyValue( aVetoPropName, aValue );

            try
            {
                bSuccess = xDesktop->terminate();
            } catch( uno::Exception& )
            {}

            if ( !bSuccess )
            {
                aValue <<= false;
                xPropertySet->setPropertyValue( aVetoPropName, aValue );
            }
        }

        if ( !bSuccess )
            m_bRestartRequested = false;
    }
    catch( uno::Exception& )
    {
        // the try to restart has failed
        m_bRestartRequested = false;
    }
}

// XServiceInfo

OUString SAL_CALL OOfficeRestartManager::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return getImplementationName_static();
}

sal_Bool SAL_CALL OOfficeRestartManager::supportsService( const OUString& aServiceName ) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence< OUString > SAL_CALL OOfficeRestartManager::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}

} // namespace comphelper

void createRegistryInfo_OOfficeRestartManager()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::OOfficeRestartManager > aAutoRegistration;
    static ::comphelper::module::OSingletonRegistration< ::comphelper::OOfficeRestartManager > aSingletonRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
