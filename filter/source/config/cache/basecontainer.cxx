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
#include "constant.hxx"

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/FilterConfigRefresh.hpp>
#include <com/sun/star/uno/Type.h>
#include <comphelper/enumhelper.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>


#define LOAD_IMPLICIT

namespace filter{
    namespace config{


namespace
{
    typedef ::salhelper::SingletonRef< FilterCache > FilterCacheRefHold;
    /** @short  hold at least one filter cache instance alive and
                prevent the office from unloading this cache if no filter
                is currently used.*/
    struct thePerformanceOptimizer :
        public rtl::Static<FilterCacheRefHold, thePerformanceOptimizer>
    {
    };
}

BaseContainer::BaseContainer()
    : BaseLock     (       )
    , m_rCache     (       )
    , m_pFlushCache(NULL   )
    , m_eType()
    , m_lListener  (m_aLock)
{
    m_rCache->load(FilterCache::E_CONTAINS_STANDARD);
    thePerformanceOptimizer::get();
}



BaseContainer::~BaseContainer()
{
}



void BaseContainer::init(const css::uno::Reference< css::uno::XComponentContext >&     rxContext              ,
                         const OUString&                                        sImplementationName,
                         const css::uno::Sequence< OUString >&                  lServiceNames      ,
                               FilterCache::EItemType                                  eType              )
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    m_sImplementationName = sImplementationName;
    m_lServiceNames       = lServiceNames      ;
    m_eType               = eType              ;
    m_xRefreshBroadcaster = css::document::FilterConfigRefresh::create(rxContext);
    // <- SAFE
}



void BaseContainer::impl_loadOnDemand()
{
#ifdef LOAD_IMPLICIT
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // A generic container needs all items of a set of our cache!
    // Of course it can block for a while, till the cache is realy filled.
    // Note: dont load all sets supported by the cache here!

    FilterCache::EFillState eRequiredState = FilterCache::E_CONTAINS_NOTHING;
    switch(m_eType)
    {
        case FilterCache::E_TYPE :
            eRequiredState = FilterCache::E_CONTAINS_TYPES;
            break;

        case FilterCache::E_FILTER :
            eRequiredState = FilterCache::E_CONTAINS_FILTERS;
            break;

        case FilterCache::E_DETECTSERVICE :
            eRequiredState = FilterCache::E_CONTAINS_DETECTSERVICES;
            break;

        case FilterCache::E_FRAMELOADER :
            eRequiredState = FilterCache::E_CONTAINS_FRAMELOADERS;
            break;

        case FilterCache::E_CONTENTHANDLER :
            eRequiredState = FilterCache::E_CONTAINS_CONTENTHANDLERS;
            break;
    }

    m_rCache->load(eRequiredState);
    // <- SAFE
#endif
}



void BaseContainer::impl_initFlushMode()
    throw (css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (!m_pFlushCache)
        m_pFlushCache = m_rCache->clone();
    if (!m_pFlushCache)
        throw css::uno::RuntimeException( "Cant create write copy of internal used cache on demand.",
                static_cast< OWeakObject* >(this));
    // <- SAFE
}



FilterCache* BaseContainer::impl_getWorkingCache() const
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_pFlushCache)
        return m_pFlushCache;
    else
        return &(*m_rCache);
    // <- SAFE
}



OUString SAL_CALL BaseContainer::getImplementationName()
    throw (css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_sImplementationName;
    // <- SAFE
}



sal_Bool SAL_CALL BaseContainer::supportsService(const OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

          sal_Int32        c      = m_lServiceNames.getLength();
    const OUString* pNames = m_lServiceNames.getConstArray();
    for (sal_Int32 i=0; i<c; ++i)
    {
        if (pNames[i].equals(sServiceName))
            return sal_True;
    }
    return sal_False;
    // <- SAFE
}



css::uno::Sequence< OUString > SAL_CALL BaseContainer::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_lServiceNames;
    // <- SAFE
}



void SAL_CALL BaseContainer::insertByName(const OUString& sItem ,
                                          const css::uno::Any&   aValue)
    throw (css::lang::IllegalArgumentException  ,
           css::container::ElementExistException,
           css::lang::WrappedTargetException    ,
           css::uno::RuntimeException           )
{
    if (sItem.isEmpty())
        throw css::lang::IllegalArgumentException("empty value not allowed as item name.",
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

    impl_loadOnDemand();

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    if (pCache->hasItem(m_eType, sItem))
        throw css::container::ElementExistException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);

    aLock.clear();
    // <- SAFE ----------------------------------
}



void SAL_CALL BaseContainer::removeByName(const OUString& sItem)
    throw (css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException            )
{
    impl_loadOnDemand();

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    pCache->removeItem(m_eType, sItem); // throw exceptions automaticly

    aLock.clear();
    // <- SAFE ----------------------------------
}



void SAL_CALL BaseContainer::replaceByName(const OUString& sItem ,
                                           const css::uno::Any&   aValue)
    throw (css::lang::IllegalArgumentException   ,
           css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException            )
{
    if (sItem.isEmpty())
        throw css::lang::IllegalArgumentException("empty value not allowed as item name.",
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

    impl_loadOnDemand();

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // create write copy of used cache on demand ...
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    if (!pCache->hasItem(m_eType, sItem))
        throw css::container::NoSuchElementException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);

    aLock.clear();
    // <- SAFE ----------------------------------
}



css::uno::Any SAL_CALL BaseContainer::getByName(const OUString& sItem)
    throw (css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException            )
{
    if (sItem.isEmpty())
        throw css::container::NoSuchElementException( "An empty item cant be part of this cache!",
                css::uno::Reference< css::uno::XInterface >(static_cast< css::container::XNameAccess* >(this), css::uno::UNO_QUERY));

    css::uno::Any aValue;

    impl_loadOnDemand();

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    CacheItem aItem;
    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        aItem = pCache->getItem(m_eType, sItem);
        pCache->addStatePropsToItem(m_eType, sItem, aItem); // add implicit props "Finalized"/"Mandatory"
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        // TODO invalid cache!? How should it be handled right?
        aItem.clear();
    }

    aValue <<= aItem.getAsPackedPropertyValueList();
    // <- SAFE

    return aValue;
}



css::uno::Sequence< OUString > SAL_CALL BaseContainer::getElementNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > lNames;

    impl_loadOnDemand();

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        OUStringList lKeys  = pCache->getItemNames(m_eType);
        lKeys >> lNames;
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
    throw (css::uno::RuntimeException)
{
    sal_Bool bHasOne = sal_False;

    impl_loadOnDemand();

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        bHasOne = pCache->hasItem(m_eType, sItem);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache!?
        bHasOne = sal_False;
    }

    // <- SAFE

    return bHasOne;
}



css::uno::Type SAL_CALL BaseContainer::getElementType()
    throw (css::uno::RuntimeException)
{
    // no lock necessary - because the type of our items
    // is fix! no internal call or member needed ...
    return ::getCppuType(static_cast< css::uno::Sequence< css::beans::PropertyValue >* >(NULL));
}



sal_Bool SAL_CALL BaseContainer::hasElements()
    throw (css::uno::RuntimeException)
{
    sal_Bool bHasSome = sal_False;

    impl_loadOnDemand();

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        bHasSome = pCache->hasItems(m_eType);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache?!
        bHasSome = sal_False;
    }

    // <- SAFE

    return bHasSome;
}



css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByQuery(const OUString& /* sQuery */ )
    throw (css::uno::RuntimeException)
{
    OSL_FAIL("not pure virtual ... but not realy implemented .-)");

    ::comphelper::OEnumerationByName* pEnum = new ::comphelper::OEnumerationByName(this, css::uno::Sequence< OUString >());
    return css::uno::Reference< css::container::XEnumeration >(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY);
}



css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XEnumeration > xEnum;
    OUStringList                                        lKeys;

    impl_loadOnDemand();

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        // convert the given properties first to our internal representation
        CacheItem lProps;
        lProps << lProperties;

        // search the key names of all items, where its properties match
        // the given ones in its minimum
        FilterCache* pCache = impl_getWorkingCache();
        lKeys = pCache->getMatchingItemsByProps(m_eType, lProps);
    }
    catch(const css::uno::Exception&)
    {
        // invalid cache, internal failure, wrong conversion ...!?
        // doesnt matter
        lKeys.clear();
    }

    // <- SAFE

    // create a specialized enumeration helper, which
    // provides the collected information outside.
    // It hold a reference to us ... and call our container interface directly.
    // be aware of some direct callbacks if it will be created :-)

    /* Note: Its not allowed to return NULL. Because an empty enumeration
             transport the same information but make no trouble outside.
             Further its easiear to work directly with the return value
             instaed of checking of NULL returns! */

    css::uno::Sequence< OUString > lSubSet;
    lKeys >> lSubSet;
    ::comphelper::OEnumerationByName* pEnum = new ::comphelper::OEnumerationByName(this, lSubSet);
    return css::uno::Reference< css::container::XEnumeration >(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY);
}



void SAL_CALL BaseContainer::flush()
    throw (css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_pFlushCache)
        throw css::lang::WrappedTargetRuntimeException(
                "Cant guarantee cache consistency. Special flush container does not exists!",
                static_cast< OWeakObject* >(this),
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
        m_rCache->takeOver(*m_pFlushCache);
    }
    catch(const css::uno::Exception& ex)
    {
        // Dont remove the clone. May be the outside
        // user whish to repair it now and calls flush()
        // later again ...

        throw css::lang::WrappedTargetRuntimeException( "Flush rejected by internal container.",
                static_cast< OWeakObject* >(this),
                css::uno::makeAny(ex));
    }

    delete m_pFlushCache;
    m_pFlushCache = NULL;

    css::uno::Reference< css::util::XRefreshable > xRefreshBroadcaster = m_xRefreshBroadcaster;

    aLock.clear();
    // <- SAFE

    if (xRefreshBroadcaster.is())
        xRefreshBroadcaster->refresh();

    // notify listener outside the lock!
    // The used listener helper lives if we live
    // and is threadsafe by itself.
    // Further its not a good idea to hold the own lock
    // if an outside object is called :-)
    css::lang::EventObject             aSource    (static_cast< css::util::XFlushable* >(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)));
    if (pContainer)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                // ... this pointer can be interesting to find out, where will be called as listener
                // Dont optimize it to a direct iterator cast :-)
                css::util::XFlushListener* pListener = (css::util::XFlushListener*)pIterator.next();
                pListener->flushed(aSource);
            }
            catch(const css::uno::Exception&)
            {
                // ignore any "damaged" flush listener!
                // May its remote reference is broken ...
                pIterator.remove();
            }
        }
    }
}



void SAL_CALL BaseContainer::addFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
    throw (css::uno::RuntimeException)
{
    // no locks necessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.addInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)),
                             xListener                                                                           );
}



void SAL_CALL BaseContainer::removeFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
    throw (css::uno::RuntimeException)
{
    // no locks necessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.removeInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)),
                                xListener                                                                           );
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
