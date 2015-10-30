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

#include <config_folders.h>

#include <stdio.h>
#include <osl/file.hxx>
#include <osl/diagnose.h>
#include <ucbhelper/contentidentifier.hxx>
#include <com/sun/star/frame/XConfigManager.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>

#include "databases.hxx"
#include "provider.hxx"
#include "content.hxx"

using namespace com::sun::star;
using namespace chelp;

// ContentProvider Implementation.

ContentProvider::ContentProvider( const uno::Reference< uno::XComponentContext >& rxContext )
    : ::ucbhelper::ContentProviderImplHelper( rxContext )
    , isInitialized( false )
    , m_aScheme(MYUCP_URL_SCHEME)
    , m_pDatabases( 0 )
{
}

// virtual
ContentProvider::~ContentProvider()
{
    delete m_pDatabases;
}

// XInterface methods.
void SAL_CALL ContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< ucb::XContentProvider* >(this)),
                                               (static_cast< lang::XComponent* >(this)),
                                               (static_cast< lang::XEventListener* >(this)),
                                               (static_cast< container::XContainerListener* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.

css::uno::Sequence< sal_Int8 > SAL_CALL ContentProvider::getImplementationId()
    throw( css::uno::RuntimeException, std::exception )
{
      return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > SAL_CALL ContentProvider::getTypes()
    throw( css::uno::RuntimeException, std::exception )
{
    static cppu::OTypeCollection* pCollection = NULL;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                    cppu::UnoType<lang::XTypeProvider>::get(),
                    cppu::UnoType<lang::XServiceInfo>::get(),
                    cppu::UnoType<ucb::XContentProvider>::get(),
                    cppu::UnoType<lang::XComponent>::get(),
                    cppu::UnoType<container::XContainerListener>::get()
                );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


// XServiceInfo methods.

OUString SAL_CALL ContentProvider::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString ContentProvider::getImplementationName_Static()
{
    return OUString("CHelpContentProvider" );
}

sal_Bool SAL_CALL
ContentProvider::supportsService(const OUString& ServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL
ContentProvider::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

static uno::Reference< uno::XInterface > SAL_CALL
ContentProvider_CreateInstance(
         const uno::Reference< lang::XMultiServiceFactory> & rSMgr )
    throw( uno::Exception )
{
    lang::XServiceInfo * pX = static_cast< lang::XServiceInfo * >(
        new ContentProvider( comphelper::getComponentContext(rSMgr) ) );
    return uno::Reference< uno::XInterface >::query( pX );
}

uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 2 );
    aSNS.getArray()[ 0 ] = MYUCP_CONTENT_PROVIDER_SERVICE_NAME1;
    aSNS.getArray()[ 1 ] = MYUCP_CONTENT_PROVIDER_SERVICE_NAME2;

    return aSNS;
}

// Service factory implementation.

css::uno::Reference< css::lang::XSingleServiceFactory >
ContentProvider::createServiceFactory( const css::uno::Reference<
            css::lang::XMultiServiceFactory >& rxServiceMgr )
{
    return css::uno::Reference<
        css::lang::XSingleServiceFactory >(
            cppu::createOneInstanceFactory(
                rxServiceMgr,
                ContentProvider::getImplementationName_Static(),
                ContentProvider_CreateInstance,
                ContentProvider::getSupportedServiceNames_Static() ) );
}

// XContentProvider methods.

// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& xCanonicId )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException, std::exception )
{
    if ( !xCanonicId->getContentProviderScheme()
             .equalsIgnoreAsciiCase( m_aScheme ) )
    {   // Wrong URL-scheme
        throw ucb::IllegalIdentifierException();
    }

    {
        osl::MutexGuard aGuard( m_aMutex );
        if( !isInitialized )
            init();
    }

    if( !m_pDatabases )
        throw uno::RuntimeException();

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    xContent = new Content( m_xContext, this, xCanonicId, m_pDatabases );

    // register new content
    registerNewContent( xContent );

    // Further checks

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

void SAL_CALL
ContentProvider::dispose()
    throw ( uno::RuntimeException, std::exception)
{
    if(m_xContainer.is())
    {
        m_xContainer->removeContainerListener(this);
        m_xContainer.clear();
    }
}

void SAL_CALL
ContentProvider::elementReplaced(const container::ContainerEvent& Event)
    throw (uno::RuntimeException, std::exception)
{
    if(!m_pDatabases)
        return;

    OUString accessor;
    Event.Accessor >>= accessor;
    if(accessor != "HelpStyleSheet")
        return;

    OUString replacedElement,element;
    Event.ReplacedElement >>= replacedElement;
    Event.Element >>= element;

    if(replacedElement == element)
        return;

    m_pDatabases->changeCSS(element);
}

void ContentProvider::init()
{
    osl::MutexGuard aGuard( m_aMutex );

    isInitialized = true;
    uno::Reference< lang::XMultiServiceFactory > sProvider(
        getConfiguration() );
    uno::Reference< container::XHierarchicalNameAccess > xHierAccess(
        getHierAccess( sProvider,
                       "org.openoffice.Office.Common" ) );

    OUString instPath( getKey( xHierAccess,"Path/Current/Help" ) );
    if( instPath.isEmpty() )
        // try to determine path from default
        instPath = "$(instpath)/" LIBO_SHARE_HELP_FOLDER;
    // replace anything like $(instpath);
    subst( instPath );

    OUString stylesheet( getKey( xHierAccess,"Help/HelpStyleSheet" ) );
    try
    {
        // now adding as configuration change listener for the stylesheet
        uno::Reference< container::XNameAccess> xAccess(
            xHierAccess, uno::UNO_QUERY );
        if( xAccess.is() )
        {
            uno::Any aAny =
                xAccess->getByName("Help");
            aAny >>= m_xContainer;
            if( m_xContainer.is() )
                m_xContainer->addContainerListener( this );
        }
    }
    catch( uno::Exception const & )
    {
    }

    xHierAccess = getHierAccess( sProvider, "org.openoffice.Setup" );

    OUString setupversion(
        getKey( xHierAccess,"Product/ooSetupVersion" ) );
    OUString setupextension;

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider =
              configuration::theDefaultProvider::get( m_xContext );

        uno::Sequence < uno::Any > lParams(1);
        beans::PropertyValue                       aParam ;
        aParam.Name    = "nodepath";
        aParam.Value <<= OUString("/org.openoffice.Setup/Product");
        lParams[0] = uno::makeAny(aParam);

        // open it
        uno::Reference< uno::XInterface > xCFG( xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess",
                    lParams) );

        uno::Reference< container::XNameAccess > xDirectAccess(xCFG, uno::UNO_QUERY);
        uno::Any aRet = xDirectAccess->getByName("ooSetupExtension");

        aRet >>= setupextension;
    }
    catch ( uno::Exception& )
    {
    }

    OUString productversion( setupversion + " " + setupextension );
    bool showBasic = getBooleanKey(xHierAccess,"Help/ShowBasic");
    m_pDatabases = new Databases( showBasic,
                                  instPath,
                                  utl::ConfigManager::getProductName(),
                                  productversion,
                                  stylesheet,
                                  m_xContext );
}

uno::Reference< lang::XMultiServiceFactory >
ContentProvider::getConfiguration() const
{
    uno::Reference< lang::XMultiServiceFactory > xProvider;
    if( m_xContext.is() )
    {
        try
        {
            xProvider = configuration::theDefaultProvider::get( m_xContext );
        }
        catch( const uno::Exception& )
        {
            OSL_ENSURE( xProvider.is(), "can not instantiate configuration" );
        }
    }

    return xProvider;
}

uno::Reference< container::XHierarchicalNameAccess >
ContentProvider::getHierAccess(
    const uno::Reference< lang::XMultiServiceFactory >& sProvider,
    const char* file )
{
    uno::Reference< container::XHierarchicalNameAccess > xHierAccess;

    if( sProvider.is() )
    {
        uno::Sequence< uno::Any > seq( 1 );
        OUString sReaderService(
            OUString(
                "com.sun.star.configuration.ConfigurationAccess" ) );

        seq[ 0 ] <<= OUString::createFromAscii( file );

        try
        {
            xHierAccess =
                uno::Reference< container::XHierarchicalNameAccess >(
                    sProvider->createInstanceWithArguments(
                        sReaderService, seq ),
                    uno::UNO_QUERY );
        }
        catch( const uno::Exception& )
        {
        }
    }
    return xHierAccess;
}

OUString
ContentProvider::getKey(
    const uno::Reference< container::XHierarchicalNameAccess >& xHierAccess,
    const char* key )
{
    OUString instPath;
    if( xHierAccess.is() )
    {
        uno::Any aAny;
        try
        {
            aAny =
                xHierAccess->getByHierarchicalName(
                    OUString::createFromAscii( key ) );
        }
        catch( const container::NoSuchElementException& )
        {
        }
        aAny >>= instPath;
    }
    return instPath;
}

bool
ContentProvider::getBooleanKey(
    const uno::Reference< container::XHierarchicalNameAccess >& xHierAccess,
    const char* key )
{
  bool ret = false;
  if( xHierAccess.is() )
  {
      uno::Any aAny;
      try
      {
          aAny =
            xHierAccess->getByHierarchicalName(
                OUString::createFromAscii( key ) );
      }
      catch( const container::NoSuchElementException& )
      {
      }
      aAny >>= ret;
  }
  return ret;
}

void ContentProvider::subst( OUString& instpath )
{
    SvtPathOptions aOptions;
    instpath = aOptions.SubstituteVariable( instpath );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
