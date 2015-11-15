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


#include "contenthandlerfactory.hxx"
#include "querytokenizer.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/enumhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>


namespace filter{
    namespace config{

ContentHandlerFactory::ContentHandlerFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
 : m_xContext(rxContext)
{
    BaseContainer::init(rxContext                                             ,
                        ContentHandlerFactory::impl_getImplementationName()   ,
                        ContentHandlerFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_CONTENTHANDLER                         );
}



ContentHandlerFactory::~ContentHandlerFactory()
{
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstance(const OUString& sHandler)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException, std::exception)
{
    return createInstanceWithArguments(sHandler, css::uno::Sequence< css::uno::Any >());
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstanceWithArguments(const OUString&                     sHandler  ,
                                                                                                        const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException, std::exception)
{
    css::uno::Reference< css::uno::XInterface > xHandler;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    OUString sRealHandler = sHandler;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given handler name really exists ...
            Because our old implementation worked with an internal
            type name instead of a handler name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sHandler) && m_rCache->hasItem(FilterCache::E_TYPE, sHandler))
        {
            _FILTER_CONFIG_LOG_("ContentHandlerFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< OUString > lTypes { sHandler };

            css::uno::Sequence< css::beans::NamedValue > lQuery { { PROPNAME_TYPES, css::uno::makeAny(lTypes) } };

            css::uno::Reference< css::container::XEnumeration > xSet = BaseContainer::createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lHandlerProps(xSet->nextElement());
                if (!(lHandlerProps[PROPNAME_NAME] >>= sRealHandler))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But don't implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sRealHandler))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search handler on cache
    CacheItem aHandler = m_rCache->getItem(FilterCache::E_CONTENTHANDLER, sRealHandler);

    // create service instance
    xHandler = m_xContext->getServiceManager()->createInstanceWithContext(sRealHandler, m_xContext);

    // initialize filter
    css::uno::Reference< css::lang::XInitialization > xInit(xHandler, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        // format: lInitData[0] = seq<PropertyValue>, which contains all configuration properties of this handler
        //         lInitData[1] = lArguments[0]
        //         ...
        //         lInitData[n] = lArguments[n-1]
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aHandler >> lConfig;

        ::std::vector< css::uno::Any > stlArguments(comphelper::sequenceToContainer< ::std::vector< css::uno::Any > >(lArguments));
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        xInit->initialize(comphelper::containerToSequence(stlArguments));
    }

    return xHandler;
    // <- SAFE
}



css::uno::Sequence< OUString > SAL_CALL ContentHandlerFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return BaseContainer::getElementNames();
}



OUString ContentHandlerFactory::impl_getImplementationName()
{
    return OUString( "com.sun.star.comp.filter.config.ContentHandlerFactory" );
}



css::uno::Sequence< OUString > ContentHandlerFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames { "com.sun.star.frame.ContentHandlerFactory" };
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ContentHandlerFactory* pNew = new ContentHandlerFactory( comphelper::getComponentContext(xSMGR) );
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
