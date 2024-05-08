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


#include "frameloaderfactory.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/sequence.hxx>


namespace filter::config{

FrameLoaderFactory::FrameLoaderFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
    BaseContainer::init(u"com.sun.star.comp.filter.config.FrameLoaderFactory"_ustr  ,
                        { u"com.sun.star.frame.FrameLoaderFactory"_ustr },
                        FilterCache::E_FRAMELOADER                         );
}


FrameLoaderFactory::~FrameLoaderFactory()
{
}


css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstance(const OUString& sLoader)
{
    return createInstanceWithArguments(sLoader, css::uno::Sequence< css::uno::Any >());
}


css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstanceWithArguments(const OUString&                     sLoader  ,
                                                                                                     const css::uno::Sequence< css::uno::Any >& lArguments)
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    auto & cache = GetTheFilterCache();

    // search loader on cache
    CacheItem aLoader = cache.getItem(m_eType, sLoader);

    // create service instance
    css::uno::Reference< css::uno::XInterface > xLoader = m_xContext->getServiceManager()->createInstanceWithContext(sLoader, m_xContext);

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

        ::std::vector< css::uno::Any > stlArguments(comphelper::sequenceToContainer< ::std::vector<css::uno::Any> >(lArguments));
        stlArguments.insert(stlArguments.begin(), css::uno::Any(lConfig));

        xInit->initialize(comphelper::containerToSequence(stlArguments));
    }

    return xLoader;
    // <- SAFE
}


css::uno::Sequence< OUString > SAL_CALL FrameLoaderFactory::getAvailableServiceNames()
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return BaseContainer::getElementNames();
}

} // namespace filter::config


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_FrameLoaderFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new filter::config::FrameLoaderFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
