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
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>


namespace filter::config{

ConfigFlush::ConfigFlush()
{
}

ConfigFlush::~ConfigFlush()
{
}

OUString SAL_CALL ConfigFlush::getImplementationName()
{
    return u"com.sun.star.comp.filter.config.ConfigFlush"_ustr;
    // <- SAFE
}

sal_Bool SAL_CALL ConfigFlush::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ConfigFlush::getSupportedServiceNames()
{
    return  { u"com.sun.star.document.FilterConfigRefresh"_ustr };
}

void SAL_CALL ConfigFlush::refresh()
{
    // notify listener outside the lock!
    // The used listener helper lives if we live
    // and is threadsafe by itself.
    // Further it's not a good idea to hold the own lock
    // if an outside object is called :-)
    std::unique_lock g(m_aMutex);
    if (!m_aRefreshListeners.getLength(g))
        return;
    css::lang::EventObject aSource(static_cast< css::util::XRefreshable* >(this));
    m_aRefreshListeners.notifyEach(g, &css::util::XRefreshListener::refreshed, aSource);
}


void SAL_CALL ConfigFlush::addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
{
    std::unique_lock g(m_aMutex);
    m_aRefreshListeners.addInterface(g, xListener);
}


void SAL_CALL ConfigFlush::removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
{
    std::unique_lock g(m_aMutex);
    m_aRefreshListeners.removeInterface(g, xListener);
}


} // namespace filter::config

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_ConfigFlush_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new filter::config::ConfigFlush());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
