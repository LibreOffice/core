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

#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
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
{
}

// virtual
ContentProvider::~ContentProvider()
{
}

// XInterface methods.
void SAL_CALL ContentProvider::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    noexcept
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< lang::XTypeProvider* >(this),
                                               static_cast< lang::XServiceInfo* >(this),
                                               static_cast< ucb::XContentProvider* >(this),
                                               static_cast< lang::XComponent* >(this),
                                               static_cast< lang::XEventListener* >(this),
                                               static_cast< container::XContainerListener* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.

css::uno::Sequence< sal_Int8 > SAL_CALL ContentProvider::getImplementationId()
{
      return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > SAL_CALL ContentProvider::getTypes()
{
    static cppu::OTypeCollection ourTypeCollection(
                    cppu::UnoType<lang::XTypeProvider>::get(),
                    cppu::UnoType<lang::XServiceInfo>::get(),
                    cppu::UnoType<ucb::XContentProvider>::get(),
                    cppu::UnoType<lang::XComponent>::get(),
                    cppu::UnoType<container::XContainerListener>::get()
                );

    return ourTypeCollection.getTypes();
}


// XServiceInfo methods.

OUString SAL_CALL ContentProvider::getImplementationName()
{
    return "CHelpContentProvider";
}

sal_Bool SAL_CALL
ContentProvider::supportsService(const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL
ContentProvider::getSupportedServiceNames()
{
    return { "com.sun.star.help.XMLHelp", "com.sun.star.ucb.HelpContentProvider" };
}

// XContentProvider methods.

// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& xCanonicId )
{
    if ( !xCanonicId->getContentProviderScheme()
             .equalsIgnoreAsciiCase( MYUCP_URL_SCHEME ) )
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
        = queryExistingContent( xCanonicId );
    if ( xContent.is() )
        return xContent;

    xContent = new Content( m_xContext, this, xCanonicId, m_pDatabases.get() );

    // register new content
    registerNewContent( xContent );

    // Further checks

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

void SAL_CALL
ContentProvider::dispose()
{
    if(m_xContainer.is())
    {
        m_xContainer->removeContainerListener(this);
        m_xContainer.clear();
    }
}

void SAL_CALL
ContentProvider::elementReplaced(const container::ContainerEvent& Event)
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

    OUString instPath(
        officecfg::Office::Common::Path::Current::Help::get(m_xContext));
    if( instPath.isEmpty() )
        // try to determine path from default
        instPath = "$(instpath)/" LIBO_SHARE_HELP_FOLDER;
    // replace anything like $(instpath);
    subst( instPath );

    OUString stylesheet(
        officecfg::Office::Common::Help::HelpStyleSheet::get(m_xContext));

    // now adding as configuration change listener for the stylesheet
    m_xContainer.set(
        officecfg::Office::Common::Help::get(m_xContext),
        css::uno::UNO_QUERY_THROW);
    m_xContainer->addContainerListener( this );

    OUString setupversion(
        officecfg::Setup::Product::ooSetupVersion::get(m_xContext));
    OUString setupextension(
        officecfg::Setup::Product::ooSetupExtension::get(m_xContext));
    OUString productversion( setupversion + " " + setupextension );

    bool showBasic = officecfg::Office::Common::Help::ShowBasic::get(
        m_xContext);
    m_pDatabases.reset( new Databases( showBasic,
                                  instPath,
                                  utl::ConfigManager::getProductName(),
                                  productversion,
                                  stylesheet,
                                  m_xContext ) );
}

void ContentProvider::subst( OUString& instpath )
{
    SvtPathOptions aOptions;
    instpath = aOptions.SubstituteVariable( instpath );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
CHelpContentProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ContentProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
