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


#include "configflush.hxx"
#include "constant.hxx"

#include <osl/diagnose.h>


namespace css = ::com::sun::star;

namespace filter{
    namespace config{


//-----------------------------------------------
ConfigFlush::ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : BaseLock   (       )
    , m_xSMGR    (xSMGR  )
    , m_lListener(m_aLock)
{
}

//-----------------------------------------------
ConfigFlush::~ConfigFlush()
{
}

//-----------------------------------------------
::rtl::OUString SAL_CALL ConfigFlush::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return impl_getImplementationName();
    // <- SAFE
}

//-----------------------------------------------
sal_Bool SAL_CALL ConfigFlush::supportsService(const ::rtl::OUString& sServiceName)
    throw (css::uno::RuntimeException)
{
          css::uno::Sequence< ::rtl::OUString > lServiceNames = impl_getSupportedServiceNames();
          sal_Int32                             c             = lServiceNames.getLength();
    const ::rtl::OUString*                      pNames        = lServiceNames.getConstArray();
    for (sal_Int32 i=0; i<c; ++i)
    {
        if (pNames[i].equals(sServiceName))
            return sal_True;
    }
    return sal_False;
}

//-----------------------------------------------
css::uno::Sequence< ::rtl::OUString > SAL_CALL ConfigFlush::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return impl_getSupportedServiceNames();
}

//-----------------------------------------------
void SAL_CALL ConfigFlush::refresh()
    throw(css::uno::RuntimeException)
{
    // notify listener outside the lock!
    // The used listener helper lives if we live
    // and is threadsafe by itself.
    // Further its not a good idea to hold the own lock
    // if an outside object is called :-)
    css::lang::EventObject             aSource    (static_cast< css::util::XRefreshable* >(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_lListener.getContainer(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)));
    if (pContainer)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                // ... this pointer can be interesting to find out, where will be called as listener
                // Dont optimize it to a direct iterator cast :-)
                css::util::XRefreshListener* pListener = (css::util::XRefreshListener*)pIterator.next();
                pListener->refreshed(aSource);
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

//-----------------------------------------------
void SAL_CALL ConfigFlush::addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    // no locks neccessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.addInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                             xListener);
}

//-----------------------------------------------
void SAL_CALL ConfigFlush::removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
    throw(css::uno::RuntimeException)
{
    // no locks neccessary
    // used helper lives if we live and is threadsafe by itself ...
    m_lListener.removeInterface(::getCppuType(static_cast< css::uno::Reference< css::util::XRefreshListener >* >(NULL)),
                                xListener);
}

//-----------------------------------------------
::rtl::OUString ConfigFlush::impl_getImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.filter.config.ConfigFlush");
}

//-----------------------------------------------
css::uno::Sequence< ::rtl::OUString > ConfigFlush::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = SERVICE_FILTERCONFIGREFRESH;
    return lServiceNames;
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigFlush::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ConfigFlush* pNew = new ConfigFlush(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::util::XRefreshable* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
