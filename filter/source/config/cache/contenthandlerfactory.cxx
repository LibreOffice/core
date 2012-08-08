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


#include "contenthandlerfactory.hxx"
#include "querytokenizer.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/enumhelper.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions



ContentHandlerFactory::ContentHandlerFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                                 ,
                        ContentHandlerFactory::impl_getImplementationName()   ,
                        ContentHandlerFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_CONTENTHANDLER                         );
}



ContentHandlerFactory::~ContentHandlerFactory()
{
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstance(const ::rtl::OUString& sHandler)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sHandler, css::uno::Sequence< css::uno::Any >());
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstanceWithArguments(const ::rtl::OUString&                     sHandler  ,
                                                                                                        const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xHandler;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString sRealHandler = sHandler;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given handler name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a handler name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sHandler) && m_rCache->hasItem(FilterCache::E_TYPE, sHandler))
        {
            _FILTER_CONFIG_LOG_("ContentHandlerFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< ::rtl::OUString > lTypes(1);
            lTypes[0] = sHandler;

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPES;
            lQuery[0].Value <<= lTypes;

            css::uno::Reference< css::container::XEnumeration > xSet = createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lHandlerProps(xSet->nextElement());
                if (!(lHandlerProps[PROPNAME_NAME] >>= sRealHandler))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But dont implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sRealHandler))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search handler on cache
    CacheItem aHandler = m_rCache->getItem(FilterCache::E_CONTENTHANDLER, sRealHandler);

    // create service instance
    xHandler = m_xSMGR->createInstance(sRealHandler);

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

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xHandler;
    // <- SAFE
}



css::uno::Sequence< ::rtl::OUString > SAL_CALL ContentHandlerFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return getElementNames();
}



::rtl::OUString ContentHandlerFactory::impl_getImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.filter.config.ContentHandlerFactory" ));
}



css::uno::Sequence< ::rtl::OUString > ContentHandlerFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ContentHandlerFactory" ));
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ContentHandlerFactory* pNew = new ContentHandlerFactory(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
