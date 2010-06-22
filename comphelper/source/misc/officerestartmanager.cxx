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
#include "precompiled_comphelper.hxx"

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper_module.hxx>
#include "officerestartmanager.hxx"

using namespace ::com::sun::star;

namespace comphelper
{

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOfficeRestartManager::getSupportedServiceNames_static()
{
    uno::Sequence< rtl::OUString > aResult( 1 );
    aResult[0] = getServiceName_static();
    return aResult;
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OOfficeRestartManager::getImplementationName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.task.OfficeRestartManager" ) );
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OOfficeRestartManager::getSingletonName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.OfficeRestartManager" ) );
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OOfficeRestartManager::getServiceName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.task.OfficeRestartManager" ) );
}

// ----------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOfficeRestartManager::Create( const uno::Reference< uno::XComponentContext >& rxContext )
{
    return static_cast< cppu::OWeakObject* >( new OOfficeRestartManager( rxContext ) );
}

// XRestartManager
// ----------------------------------------------------------
void SAL_CALL OOfficeRestartManager::requestRestart( const uno::Reference< task::XInteractionHandler >& /* xInteractionHandler */ )
    throw (uno::Exception, uno::RuntimeException)
{
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // if the restart already running there is no need to trigger it again
        if ( m_bRestartRequested )
            return;

        m_bRestartRequested = sal_True;

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
                 ::rtl::OUString::createFromAscii("com.sun.star.awt.AsyncCallback"),
                 m_xContext ),
             uno::UNO_QUERY_THROW );

        xRequestCallback->addCallback( this, uno::Any() );
    }
    catch ( uno::Exception& )
    {
        // the try to request restart has failed
        m_bRestartRequested = sal_False;
    }
}

// ----------------------------------------------------------
::sal_Bool SAL_CALL OOfficeRestartManager::isRestartRequested( ::sal_Bool bOfficeInitialized )
    throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( bOfficeInitialized && !m_bOfficeInitialized )
        m_bOfficeInitialized = bOfficeInitialized;

    return m_bRestartRequested;
}

// XCallback
// ----------------------------------------------------------
void SAL_CALL OOfficeRestartManager::notify( const uno::Any& /* aData */ )
    throw ( uno::RuntimeException )
{
    try
    {
        sal_Bool bSuccess = sal_False;

        if ( m_xContext.is() )
        {
            uno::Reference< lang::XMultiComponentFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_SET_THROW );
            uno::Reference< frame::XDesktop > xDesktop(
                xFactory->createInstanceWithContext(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ), m_xContext ),
                uno::UNO_QUERY_THROW );

            // Turn Quickstarter veto off
            uno::Reference< beans::XPropertySet > xPropertySet( xDesktop, uno::UNO_QUERY_THROW );
            ::rtl::OUString aVetoPropName( RTL_CONSTASCII_USTRINGPARAM( "SuspendQuickstartVeto" ) );
            uno::Any aValue;
            aValue <<= (sal_Bool)sal_True;
            xPropertySet->setPropertyValue( aVetoPropName, aValue );

            try
            {
                bSuccess = xDesktop->terminate();
            } catch( uno::Exception& )
            {}

            if ( !bSuccess )
            {
                aValue <<= (sal_Bool)sal_False;
                xPropertySet->setPropertyValue( aVetoPropName, aValue );
            }
        }

        if ( !bSuccess )
            m_bRestartRequested = sal_False;
    }
    catch( uno::Exception& )
    {
        // the try to restart has failed
        m_bRestartRequested = sal_False;
    }
}

// XServiceInfo
// ----------------------------------------------------------
::rtl::OUString SAL_CALL OOfficeRestartManager::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_static();
}

// ----------------------------------------------------------
::sal_Bool SAL_CALL OOfficeRestartManager::supportsService( const ::rtl::OUString& aServiceName ) throw (uno::RuntimeException)
{
    const uno::Sequence< rtl::OUString > & aSupportedNames = getSupportedServiceNames_static();
    for ( sal_Int32 nInd = 0; nInd < aSupportedNames.getLength(); nInd++ )
    {
        if ( aSupportedNames[ nInd ].equals( aServiceName ) )
            return sal_True;
    }

    return sal_False;
}

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOfficeRestartManager::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

} // namespace comphelper

void createRegistryInfo_OOfficeRestartManager()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::OOfficeRestartManager > aAutoRegistration;
    static ::comphelper::module::OSingletonRegistration< ::comphelper::OOfficeRestartManager > aSingletonRegistration;
}
