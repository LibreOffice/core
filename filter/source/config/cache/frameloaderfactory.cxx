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


#include "frameloaderfactory.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/enumhelper.hxx>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;




FrameLoaderFactory::FrameLoaderFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                              ,
                        FrameLoaderFactory::impl_getImplementationName()   ,
                        FrameLoaderFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_FRAMELOADER                         );
}



FrameLoaderFactory::~FrameLoaderFactory()
{
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstance(const ::rtl::OUString& sLoader)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sLoader, css::uno::Sequence< css::uno::Any >());
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstanceWithArguments(const ::rtl::OUString&                     sLoader  ,
                                                                                                     const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString sRealLoader = sLoader;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given loader name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a loader name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sLoader) && m_rCache->hasItem(FilterCache::E_TYPE, sLoader))
        {
            _FILTER_CONFIG_LOG_("FrameLoaderFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< ::rtl::OUString > lTypes(1);
            lTypes[0] = sLoader;

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPES;
            lQuery[0].Value <<= lTypes;

            css::uno::Reference< css::container::XEnumeration > xSet = createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lLoaderProps(xSet->nextElement());
                if (!(lLoaderProps[PROPNAME_NAME] >>= sRealLoader))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But dont implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sRealLoader))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search loader on cache
    CacheItem aLoader = m_rCache->getItem(m_eType, sRealLoader);

    // create service instance
    css::uno::Reference< css::uno::XInterface > xLoader = m_xSMGR->createInstance(sRealLoader);

    // initialize filter
    css::uno::Reference< css::lang::XInitialization > xInit(xLoader, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        // format: lInitData[0] = seq<PropertyValue>, which contains all configuration properties of this loader
        //         lInitData[1] = lArguments[0]
        //         ...
        //         lInitData[n] = lArguments[n-1]
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aLoader >> lConfig;

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xLoader;
    // <- SAFE
}



css::uno::Sequence< ::rtl::OUString > SAL_CALL FrameLoaderFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return getElementNames();
}



::rtl::OUString FrameLoaderFactory::impl_getImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.filter.config.FrameLoaderFactory" ));
}



css::uno::Sequence< ::rtl::OUString > FrameLoaderFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.FrameLoaderFactory" ));
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    FrameLoaderFactory* pNew = new FrameLoaderFactory(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
