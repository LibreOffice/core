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

#include "configflush.hxx"
#include "constant.hxx"
#include <cppuhelper/supportsservice.hxx>

#include <osl/diagnose.h>

namespace filter{
    namespace config{

ConfigFlush::ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : BaseLock   (       )
    , m_xSMGR    (xSMGR  )
    , m_lListener(m_aLock)
{
}

ConfigFlush::~ConfigFlush()
{
}

OUString SAL_CALL ConfigFlush::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return impl_getImplementationName();
    
}

sal_Bool SAL_CALL ConfigFlush::supportsService(const OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ConfigFlush::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return impl_getSupportedServiceNames();
}

void SAL_CALL ConfigFlush::refresh()
    throw(css::uno::RuntimeException)
{
    
    
    
    
    
    css::lang::EventObject             aSource    (static_cast< css::util::XRefreshable* >(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)));
    if (pContainer)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                
                
                css::util::XRefreshListener* pListener = (css::util::XRefreshListener*)pIterator.next();
                pListener->refreshed(aSource);
            }
            catch(const css::uno::Exception&)
            {
                
                
                pIterator.remove();
            }
        }
    }
}


void SAL_CALL ConfigFlush::addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    
    
    m_lListener.addInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                             xListener);
}


void SAL_CALL ConfigFlush::removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    
    
    m_lListener.removeInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                                xListener);
}


OUString ConfigFlush::impl_getImplementationName()
{
    return OUString("com.sun.star.comp.filter.config.ConfigFlush");
}


css::uno::Sequence< OUString > ConfigFlush::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames[0] = "com.sun.star.document.FilterConfigRefresh";
    return lServiceNames;
}


css::uno::Reference< css::uno::XInterface > ConfigFlush::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ConfigFlush* pNew = new ConfigFlush(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::util::XRefreshable* >(pNew), css::uno::UNO_QUERY);
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
