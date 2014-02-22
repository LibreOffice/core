/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper_module.hxx>
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



void SAL_CALL OOfficeRestartManager::requestRestart( const uno::Reference< task::XInteractionHandler >& /* xInteractionHandler */ )
    throw (uno::Exception, uno::RuntimeException)
{
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    {
        ::osl::MutexGuard aGuard( m_aMutex );

        
        if ( m_bRestartRequested )
            return;

        m_bRestartRequested = true;

        
        if ( !m_bOfficeInitialized )
            return;
    }

    
    try
    {
        
        uno::Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_SET_THROW );

        uno::Reference< awt::XRequestCallback > xRequestCallback(
            xFactory->createInstanceWithContext(
                 OUString( "com.sun.star.awt.AsyncCallback" ),
                 m_xContext ),
             uno::UNO_QUERY_THROW );

        xRequestCallback->addCallback( this, uno::Any() );
    }
    catch ( uno::Exception& )
    {
        
        m_bRestartRequested = false;
    }
}


::sal_Bool SAL_CALL OOfficeRestartManager::isRestartRequested( ::sal_Bool bOfficeInitialized )
    throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( bOfficeInitialized && !m_bOfficeInitialized )
        m_bOfficeInitialized = bOfficeInitialized;

    return m_bRestartRequested;
}



void SAL_CALL OOfficeRestartManager::notify( const uno::Any& /* aData */ )
    throw ( uno::RuntimeException )
{
    try
    {
        bool bSuccess = false;

        if ( m_xContext.is() )
        {
            uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xContext);

            
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
        
        m_bRestartRequested = false;
    }
}



OUString SAL_CALL OOfficeRestartManager::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_static();
}

::sal_Bool SAL_CALL OOfficeRestartManager::supportsService( const OUString& aServiceName ) throw (uno::RuntimeException)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence< OUString > SAL_CALL OOfficeRestartManager::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

} 

void createRegistryInfo_OOfficeRestartManager()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::OOfficeRestartManager > aAutoRegistration;
    static ::comphelper::module::OSingletonRegistration< ::comphelper::OOfficeRestartManager > aSingletonRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
