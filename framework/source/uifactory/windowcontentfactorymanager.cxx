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

#include <uifactory/windowcontentfactorymanager.hxx>
#include <uifactory/uielementfactorymanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   WindowContentFactoryManager                     ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_WINDOWCONTENTFACTORYMANAGER         ,
                                            IMPLEMENTATIONNAME_WINDOWCONTENTFACTORYMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   WindowContentFactoryManager, {} )

WindowContentFactoryManager::WindowContentFactoryManager( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_FactoryManager( m_xServiceManager,rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories" )) );
    m_pConfigAccess->acquire();
    m_xModuleManager = frame::ModuleManager::create( comphelper::getComponentContext(m_xServiceManager) );
}

WindowContentFactoryManager::~WindowContentFactoryManager()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

void WindowContentFactoryManager::RetrieveTypeNameFromResourceURL( const rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName )
{
    const sal_Int32 RESOURCEURL_PREFIX_SIZE = 17;
    const char      RESOURCEURL_PREFIX[] = "private:resource/";

    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        rtl::OUString aTmpStr( aResourceURL.copy( RESOURCEURL_PREFIX_SIZE ));
        sal_Int32 nToken = 0;
        sal_Int32 nPart  = 0;
        do
        {
            ::rtl::OUString sToken = aTmpStr.getToken( 0, '/', nToken);
            if ( !sToken.isEmpty() )
            {
                if ( nPart == 0 )
                    aType = sToken;
                else if ( nPart == 1 )
                    aName = sToken;
                else
                    break;
                nPart++;
            }
        }
        while( nToken >=0 );
    }
}

// XSingleComponentFactory
uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithContext(
    const uno::Reference< uno::XComponentContext >& /*xContext*/ )
throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xWindow;
    return xWindow;
}

uno::Reference< uno::XInterface > SAL_CALL WindowContentFactoryManager::createInstanceWithArgumentsAndContext(
    const uno::Sequence< uno::Any >& Arguments, const uno::Reference< uno::XComponentContext >& Context )
throw (uno::Exception, uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xWindow;
    uno::Reference< frame::XFrame >   xFrame;
    ::rtl::OUString                   aResourceURL;

    for (sal_Int32 i=0; i < Arguments.getLength(); i++ )
    {
        beans::PropertyValue aPropValue;
        if ( Arguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name == "Frame" )
                aPropValue.Value >>= xFrame;
            else if ( aPropValue.Name == "ResourceURL" )
                aPropValue.Value >>= aResourceURL;
        }
    }

    uno::Reference< frame::XModuleManager2 > xModuleManager;
    // SAFE
    {
        ResetableGuard aLock( m_aLock );
        xModuleManager = m_xModuleManager;
    }
    // UNSAFE

    // Determine the module identifier
    ::rtl::OUString aType;
    ::rtl::OUString aName;
    ::rtl::OUString aModuleId;
    try
    {
        if ( xFrame.is() && xModuleManager.is() )
            aModuleId = xModuleManager->identify( uno::Reference< uno::XInterface >( xFrame, uno::UNO_QUERY ) );
    }
    catch ( const frame::UnknownModuleException& )
    {
    }

    RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );
    if ( !aType.isEmpty() &&
         !aName.isEmpty() &&
         !aModuleId.isEmpty() )
    {
        ::rtl::OUString                   aImplementationName;
        uno::Reference< uno::XInterface > xHolder( static_cast<cppu::OWeakObject*>(this), uno::UNO_QUERY );

        // Detetmine the implementation name of the window content factory dependent on the
        // module identifier, user interface element type and name
        // SAFE
        ResetableGuard aLock( m_aLock );

        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }

        aImplementationName = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
        if ( !aImplementationName.isEmpty() )
        {
            aLock.unlock();
            // UNSAFE

            uno::Reference< lang::XMultiServiceFactory > xServiceManager( Context->getServiceManager(), uno::UNO_QUERY );
            if ( xServiceManager.is() )
            {
                uno::Reference< lang::XSingleComponentFactory > xFactory(
                    xServiceManager->createInstance( aImplementationName ), uno::UNO_QUERY );
                if ( xFactory.is() )
                {
                    // Be careful: We call external code. Therefore here we have to catch all exceptions
                    try
                    {
                        xWindow = xFactory->createInstanceWithArgumentsAndContext( Arguments, Context );
                    }
                    catch ( uno::Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }

    // UNSAFE
    if ( !xWindow.is())
    {
        // Fallback: Use internal factory code to create a toolkit dialog as a content window
        xWindow = createInstanceWithContext(Context);
    }

    return xWindow;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
