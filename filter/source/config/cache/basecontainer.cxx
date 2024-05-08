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


#include "basecontainer.hxx"

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/uno/Type.h>
#include <comphelper/enumhelper.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

namespace filter::config{

BaseContainer::BaseContainer()
    : m_eType()
{
    GetTheFilterCache().load(FilterCache::E_CONTAINS_STANDARD);
}


BaseContainer::~BaseContainer()
{
}


void BaseContainer::init(const OUString&                                        sImplementationName,
                         const css::uno::Sequence< OUString >&                  lServiceNames      ,
                               FilterCache::EItemType                                  eType              )
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    m_sImplementationName = sImplementationName;
    m_lServiceNames       = lServiceNames      ;
    m_eType               = eType              ;
    // <- SAFE
}


void BaseContainer::impl_loadOnDemand(std::unique_lock<std::mutex>& /*rGuard*/)
{
    // A generic container needs all items of a set of our cache!
    // Of course it can block for a while, till the cache is really filled.
    // Note: don't load all sets supported by the cache here!

    FilterCache::EFillState eRequiredState = FilterCache::E_CONTAINS_NOTHING;
    switch(m_eType)
    {
        case FilterCache::E_TYPE :
            eRequiredState = FilterCache::E_CONTAINS_TYPES;
            break;

        case FilterCache::E_FILTER :
            eRequiredState = FilterCache::E_CONTAINS_FILTERS;
            break;

        case FilterCache::E_FRAMELOADER :
            eRequiredState = FilterCache::E_CONTAINS_FRAMELOADERS;
            break;

        case FilterCache::E_CONTENTHANDLER :
            eRequiredState = FilterCache::E_CONTAINS_CONTENTHANDLERS;
            break;
    }

    GetTheFilterCache().load(eRequiredState);
}


void BaseContainer::impl_initFlushMode(std::unique_lock<std::mutex>& /*rGuard*/)
{
    if (!m_pFlushCache)
        m_pFlushCache = GetTheFilterCache().clone();
    if (!m_pFlushCache)
        throw css::uno::RuntimeException( u"Can not create write copy of internal used cache on demand."_ustr,
                getXWeak());
}


FilterCache* BaseContainer::impl_getWorkingCache(std::unique_lock<std::mutex>& /*rGuard*/) const
{
    if (m_pFlushCache)
        return m_pFlushCache.get();
    else
        return &GetTheFilterCache();
}


OUString SAL_CALL BaseContainer::getImplementationName()
{
    return m_sImplementationName;
}


sal_Bool SAL_CALL BaseContainer::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BaseContainer::getSupportedServiceNames()
{
    return m_lServiceNames;
}


void SAL_CALL BaseContainer::insertByName(const OUString& sItem ,
                                          const css::uno::Any&   aValue)
{
    if (sItem.isEmpty())
        throw css::lang::IllegalArgumentException(u"empty value not allowed as item name."_ustr,
            static_cast< css::container::XNameContainer* >(this),
            1);

    CacheItem aItem;
    try
    {
        aItem << aValue;
    }
    catch(const css::uno::Exception& ex)
    {
        throw css::lang::IllegalArgumentException(ex.Message, static_cast< css::container::XNameContainer* >(this), 2);
    }

    // SAFE -> ----------------------------------
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode(aLock);

    FilterCache* pCache = impl_getWorkingCache(aLock);
    if (pCache->hasItem(m_eType, sItem))
        throw css::container::ElementExistException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);
    // <- SAFE ----------------------------------
}


void SAL_CALL BaseContainer::removeByName(const OUString& sItem)
{
    // SAFE -> ----------------------------------
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode(aLock);

    FilterCache* pCache = impl_getWorkingCache(aLock);
    pCache->removeItem(m_eType, sItem); // throw exceptions automatically
    // <- SAFE ----------------------------------
}


void SAL_CALL BaseContainer::replaceByName(const OUString& sItem ,
                                           const css::uno::Any&   aValue)
{
    if (sItem.isEmpty())
        throw css::lang::IllegalArgumentException(u"empty value not allowed as item name."_ustr,
            static_cast< css::container::XNameContainer* >(this),
            1);

    CacheItem aItem;
    try
    {
        aItem << aValue;
    }
    catch(const css::uno::Exception& ex)
    {
        throw css::lang::IllegalArgumentException(ex.Message, static_cast< css::container::XNameContainer* >(this), 2);
    }

    // SAFE -> ----------------------------------
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode(aLock);

    FilterCache* pCache = impl_getWorkingCache(aLock);
    if (!pCache->hasItem(m_eType, sItem))
        throw css::container::NoSuchElementException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);
    // <- SAFE ----------------------------------
}


css::uno::Any SAL_CALL BaseContainer::getByName(const OUString& sItem)
{
    if (sItem.isEmpty())
        throw css::container::NoSuchElementException( u"An empty item can't be part of this cache!"_ustr,
                static_cast< css::container::XNameAccess* >(this));

    css::uno::Any aValue;

    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache(aLock);
        aValue = pCache->getItemWithStateProps(m_eType, sItem);
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        // TODO invalid cache!? How should it be handled right?
    }

    // <- SAFE

    return aValue;
}


css::uno::Sequence< OUString > SAL_CALL BaseContainer::getElementNames()
{
    css::uno::Sequence< OUString > lNames;

    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache(aLock);
        std::vector<OUString> lKeys  = pCache->getItemNames(m_eType);
        lNames = comphelper::containerToSequence(lKeys);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache!?
        lNames.realloc(0);
    }

    // <- SAFE

    return lNames;
}


sal_Bool SAL_CALL BaseContainer::hasByName(const OUString& sItem)
{
    bool bHasOne = false;

    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache(aLock);
        bHasOne = pCache->hasItem(m_eType, sItem);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache!?
        bHasOne = false;
    }

    // <- SAFE

    return bHasOne;
}


css::uno::Type SAL_CALL BaseContainer::getElementType()
{
    // no lock necessary - because the type of our items
    // is fix! no internal call or member needed ...
    return cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get();
}


sal_Bool SAL_CALL BaseContainer::hasElements()
{
    bool bHasSome = false;

    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache(aLock);
        bHasSome = pCache->hasItems(m_eType);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache?!
        bHasSome = false;
    }

    // <- SAFE

    return bHasSome;
}


css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByQuery(const OUString& /* sQuery */ )
{
    OSL_FAIL("not pure virtual ... but not really implemented .-)");

    return new ::comphelper::OEnumerationByName(this, {});
}


css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
{
    std::vector<OUString>                               lKeys;

    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    impl_loadOnDemand(aLock);

    try
    {
        // search the key names of all items, where its properties match
        // the given ones in its minimum
        FilterCache* pCache = impl_getWorkingCache(aLock);
        lKeys = pCache->getMatchingItemsByProps(m_eType, std::span<const css::beans::NamedValue>( lProperties.getConstArray(), lProperties.getLength() ));
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache, internal failure, wrong conversion ...!?
        // doesn't matter
        lKeys.clear();
    }

    // <- SAFE

    // create a specialized enumeration helper, which
    // provides the collected information outside.
    // It hold a reference to us ... and call our container interface directly.
    // be aware of some direct callbacks if it will be created :-)

    /* Note: It's not allowed to return NULL. Because an empty enumeration
             transport the same information but make no trouble outside.
             Further its easier to work directly with the return value
             instead of checking of NULL returns! */

    return new ::comphelper::OEnumerationByName(this, std::move(lKeys));
}


void SAL_CALL BaseContainer::flush()
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    if (!m_pFlushCache)
        throw css::lang::WrappedTargetRuntimeException(
                u"Can not guarantee cache consistency. Special flush container does not exists!"_ustr,
                getXWeak(),
                css::uno::Any());

    try
    {
        m_pFlushCache->flush();
        // Take over all changes into the global cache and
        // forget the clone.
        /* TODO
            -think about me
                If the global cache gets this information via listener,
                we should remove this method!
        */
        GetTheFilterCache().takeOver(*m_pFlushCache);
    }
    catch(const css::uno::Exception& ex)
    {
        // Don't remove the clone. May be the outside
        // user wish to repair it now and calls flush()
        // later again ...

        throw css::lang::WrappedTargetRuntimeException( u"Flush rejected by internal container."_ustr,
                getXWeak(),
                css::uno::Any(ex));
    }

    m_pFlushCache.reset();

    css::lang::EventObject             aSource    (static_cast< css::util::XFlushable* >(this));
    m_lListener.notifyEach( aLock, &css::util::XFlushListener::flushed, aSource);

    // <- SAFE
}


void SAL_CALL BaseContainer::addFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
{
    std::unique_lock g(m_aMutex);
    m_lListener.addInterface(g, xListener);
}


void SAL_CALL BaseContainer::removeFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
{
    std::unique_lock g(m_aMutex);
    m_lListener.removeInterface(g, xListener);
}

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
