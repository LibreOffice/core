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
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2  (   WindowContentFactoryManager                     ,
                                            ::cppu::OWeakObject                             ,
                                            "com.sun.star.ui.WindowContentFactoryManager",
                                            IMPLEMENTATIONNAME_WINDOWCONTENTFACTORYMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   WindowContentFactoryManager, {} )

WindowContentFactoryManager::WindowContentFactoryManager( const uno::Reference< uno::XComponentContext >& rxContext ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_bConfigRead( sal_False )
{
    m_pConfigAccess = new ConfigurationAccess_FactoryManager( rxContext, OUString( "/org.openoffice.Office.UI.WindowContentFactories/Registered/ContentFactories" ) );
    m_pConfigAccess->acquire();
    m_xModuleManager = frame::ModuleManager::create( rxContext );
}

WindowContentFactoryManager::~WindowContentFactoryManager()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

void WindowContentFactoryManager::RetrieveTypeNameFromResourceURL( const OUString& aResourceURL, OUString& aType, OUString& aName )
{
    const sal_Int32 RESOURCEURL_PREFIX_SIZE = 17;
    const char      RESOURCEURL_PREFIX[] = "private:resource/";

    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        OUString aTmpStr( aResourceURL.copy( RESOURCEURL_PREFIX_SIZE ));
        sal_Int32 nToken = 0;
        sal_Int32 nPart  = 0;
        do
        {
            OUString sToken = aTmpStr.getToken( 0, '/', nToken);
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
    OUString                   aResourceURL;

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
    OUString aType;
    OUString aName;
    OUString aModuleId;
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
        OUString                   aImplementationName;
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
