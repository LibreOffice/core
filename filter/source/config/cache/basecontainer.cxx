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


#include "basecontainer.hxx"
#include "constant.hxx"

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/FilterConfigRefresh.hpp>
#include <com/sun/star/uno/Type.h>
#include <comphelper/enumhelper.hxx>
#include <cppuhelper/supportsservice.hxx>
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
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    m_sImplementationName = sImplementationName;
    m_lServiceNames       = lServiceNames      ;
    m_eType               = eType              ;
    m_xRefreshBroadcaster = css::document::FilterConfigRefresh::create(rxContext);
    
}



void BaseContainer::impl_loadOnDemand()
{
#ifdef LOAD_IMPLICIT
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    

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
    
#endif
}



void BaseContainer::impl_initFlushMode()
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (!m_pFlushCache)
        m_pFlushCache = m_rCache->clone();
    if (!m_pFlushCache)
        throw css::uno::RuntimeException( "Cant create write copy of internal used cache on demand.",
                static_cast< OWeakObject* >(this));
    
}



FilterCache* BaseContainer::impl_getWorkingCache() const
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_pFlushCache)
        return m_pFlushCache;
    else
        return &(*m_rCache);
    
}



OUString SAL_CALL BaseContainer::getImplementationName()
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_sImplementationName;
    
}



sal_Bool SAL_CALL BaseContainer::supportsService(const OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BaseContainer::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return m_lServiceNames;
    
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

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    if (pCache->hasItem(m_eType, sItem))
        throw css::container::ElementExistException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);

    aLock.clear();
    
}



void SAL_CALL BaseContainer::removeByName(const OUString& sItem)
    throw (css::container::NoSuchElementException,
           css::lang::WrappedTargetException     ,
           css::uno::RuntimeException            )
{
    impl_loadOnDemand();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    pCache->removeItem(m_eType, sItem); 

    aLock.clear();
    
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

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    impl_initFlushMode();

    FilterCache* pCache = impl_getWorkingCache();
    if (!pCache->hasItem(m_eType, sItem))
        throw css::container::NoSuchElementException(OUString(), static_cast< css::container::XNameContainer* >(this));
    pCache->setItem(m_eType, sItem, aItem);

    aLock.clear();
    
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

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    CacheItem aItem;
    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        aItem = pCache->getItem(m_eType, sItem);
        pCache->addStatePropsToItem(m_eType, sItem, aItem); 
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
        
        aItem.clear();
    }

    aValue <<= aItem.getAsPackedPropertyValueList();
    

    return aValue;
}



css::uno::Sequence< OUString > SAL_CALL BaseContainer::getElementNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > lNames;

    impl_loadOnDemand();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        OUStringList lKeys  = pCache->getItemNames(m_eType);
        lKeys >> lNames;
    }
    catch(const css::uno::Exception&)
    {
        
        lNames.realloc(0);
    }

    

    return lNames;
}



sal_Bool SAL_CALL BaseContainer::hasByName(const OUString& sItem)
    throw (css::uno::RuntimeException)
{
    sal_Bool bHasOne = sal_False;

    impl_loadOnDemand();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        bHasOne = pCache->hasItem(m_eType, sItem);
    }
    catch(const css::uno::Exception&)
    {
        
        bHasOne = sal_False;
    }

    

    return bHasOne;
}



css::uno::Type SAL_CALL BaseContainer::getElementType()
    throw (css::uno::RuntimeException)
{
    
    
    return ::getCppuType(static_cast< css::uno::Sequence< css::beans::PropertyValue >* >(NULL));
}



sal_Bool SAL_CALL BaseContainer::hasElements()
    throw (css::uno::RuntimeException)
{
    sal_Bool bHasSome = sal_False;

    impl_loadOnDemand();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        FilterCache* pCache = impl_getWorkingCache();
        bHasSome = pCache->hasItems(m_eType);
    }
    catch(const css::uno::Exception&)
    {
        
        bHasSome = sal_False;
    }

    

    return bHasSome;
}



css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByQuery(const OUString& /* sQuery */ )
    throw (css::uno::RuntimeException)
{
    OSL_FAIL("not pure virtual ... but not really implemented .-)");

    ::comphelper::OEnumerationByName* pEnum = new ::comphelper::OEnumerationByName(this, css::uno::Sequence< OUString >());
    return css::uno::Reference< css::container::XEnumeration >(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY);
}



css::uno::Reference< css::container::XEnumeration > SAL_CALL BaseContainer::createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XEnumeration > xEnum;
    OUStringList                                        lKeys;

    impl_loadOnDemand();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    try
    {
        
        CacheItem lProps;
        lProps << lProperties;

        
        
        FilterCache* pCache = impl_getWorkingCache();
        lKeys = pCache->getMatchingItemsByProps(m_eType, lProps);
    }
    catch(const css::uno::Exception&)
    {
        
        
        lKeys.clear();
    }

    

    
    
    
    

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
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_pFlushCache)
        throw css::lang::WrappedTargetRuntimeException(
                "Cant guarantee cache consistency. Special flush container does not exists!",
                static_cast< OWeakObject* >(this),
                css::uno::Any());

    try
    {
        m_pFlushCache->flush();
        
        
        /* TODO
            -think about me
                If the global cache gets this information via listener,
                we should remove this method!
        */
        m_rCache->takeOver(*m_pFlushCache);
    }
    catch(const css::uno::Exception& ex)
    {
        
        
        

        throw css::lang::WrappedTargetRuntimeException( "Flush rejected by internal container.",
                static_cast< OWeakObject* >(this),
                css::uno::makeAny(ex));
    }

    delete m_pFlushCache;
    m_pFlushCache = NULL;

    css::uno::Reference< css::util::XRefreshable > xRefreshBroadcaster = m_xRefreshBroadcaster;

    aLock.clear();
    

    if (xRefreshBroadcaster.is())
        xRefreshBroadcaster->refresh();

    
    
    
    
    
    css::lang::EventObject             aSource    (static_cast< css::util::XFlushable* >(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)));
    if (pContainer)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                
                
                css::util::XFlushListener* pListener = (css::util::XFlushListener*)pIterator.next();
                pListener->flushed(aSource);
            }
            catch(const css::uno::Exception&)
            {
                
                
                pIterator.remove();
            }
        }
    }
}



void SAL_CALL BaseContainer::addFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
    throw (css::uno::RuntimeException)
{
    
    
    m_lListener.addInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)),
                             xListener                                                                           );
}



void SAL_CALL BaseContainer::removeFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
    throw (css::uno::RuntimeException)
{
    
    
    m_lListener.removeInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XFlushListener >* >(NULL)),
                                xListener                                                                           );
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
