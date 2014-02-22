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


#include "frameloaderfactory.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/enumhelper.hxx>
#include <comphelper/processfactory.hxx>


namespace filter{
    namespace config{

FrameLoaderFactory::FrameLoaderFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
    BaseContainer::init(rxContext                                              ,
                        FrameLoaderFactory::impl_getImplementationName()   ,
                        FrameLoaderFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_FRAMELOADER                         );
}



FrameLoaderFactory::~FrameLoaderFactory()
{
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstance(const OUString& sLoader)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sLoader, css::uno::Sequence< css::uno::Any >());
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstanceWithArguments(const OUString&                     sLoader  ,
                                                                                                     const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    OUString sRealLoader = sLoader;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given loader name really exist ...
            Because our old implementation worked with an internal
            type name instead of a loader name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sLoader) && m_rCache->hasItem(FilterCache::E_TYPE, sLoader))
        {
            _FILTER_CONFIG_LOG_("FrameLoaderFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< OUString > lTypes(1);
            lTypes[0] = sLoader;

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPES;
            lQuery[0].Value <<= lTypes;

            css::uno::Reference< css::container::XEnumeration > xSet = BaseContainer::createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lLoaderProps(xSet->nextElement());
                if (!(lLoaderProps[PROPNAME_NAME] >>= sRealLoader))
                    continue;
            }

            
            
            if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sRealLoader))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif 

    
    CacheItem aLoader = m_rCache->getItem(m_eType, sRealLoader);

    
    css::uno::Reference< css::uno::XInterface > xLoader = m_xContext->getServiceManager()->createInstanceWithContext(sRealLoader, m_xContext);

    
    css::uno::Reference< css::lang::XInitialization > xInit(xLoader, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        
        
        
        
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aLoader >> lConfig;

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xLoader;
    
}



css::uno::Sequence< OUString > SAL_CALL FrameLoaderFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    
    return BaseContainer::getElementNames();
}



OUString FrameLoaderFactory::impl_getImplementationName()
{
    return OUString( "com.sun.star.comp.filter.config.FrameLoaderFactory" );
}



css::uno::Sequence< OUString > FrameLoaderFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames[0] = "com.sun.star.frame.FrameLoaderFactory";
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    FrameLoaderFactory* pNew = new FrameLoaderFactory( comphelper::getComponentContext(xSMGR) );
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
