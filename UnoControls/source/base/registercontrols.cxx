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

#include <sal/config.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>

#include <framecontrol.hxx>
#include <progressbar.hxx>
#include <progressmonitor.hxx>
#include <statusindicator.hxx>

namespace {

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> FrameControl_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &
        rServiceManager)
{
    return static_cast<cppu::OWeakObject *>(
        new unocontrols::FrameControl(
            comphelper::getComponentContext(rServiceManager)));
}

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface> ProgressBar_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &
        rServiceManager)
{
    return static_cast<cppu::OWeakObject *>(
        new unocontrols::ProgressBar(
            comphelper::getComponentContext(rServiceManager)));
}

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface>
ProgressMonitor_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &
        rServiceManager)
{
    return static_cast<cppu::OWeakObject *>(
        new unocontrols::ProgressMonitor(
            comphelper::getComponentContext(rServiceManager)));
}

/// @throws css::uno::Exception
css::uno::Reference<css::uno::XInterface>
StatusIndicator_createInstance(
    css::uno::Reference<css::lang::XMultiServiceFactory> const &
        rServiceManager)
{
    return static_cast<cppu::OWeakObject *>(
        new unocontrols::StatusIndicator(
            comphelper::getComponentContext(rServiceManager)));
}

}

extern "C" SAL_DLLPUBLIC_EXPORT void * ctl_component_getFactory(
    char const * pImplName, void * pServiceManager, SAL_UNUSED_PARAMETER void *)
{
    css::uno::Reference<css::lang::XMultiServiceFactory > smgr(
        static_cast<css::lang::XMultiServiceFactory *>(pServiceManager));
    css::uno::Reference<css::lang::XSingleServiceFactory> fac;
    if (unocontrols::FrameControl::impl_getStaticImplementationName()
        .equalsAscii(pImplName))
    {
        fac = cppu::createSingleFactory(
            smgr, unocontrols::FrameControl::impl_getStaticImplementationName(),
            &FrameControl_createInstance,
            unocontrols::FrameControl::impl_getStaticSupportedServiceNames());
    } else if (unocontrols::ProgressBar::impl_getStaticImplementationName()
               .equalsAscii(pImplName))
    {
        fac = cppu::createSingleFactory(
            smgr, unocontrols::ProgressBar::impl_getStaticImplementationName(),
            &ProgressBar_createInstance,
            unocontrols::ProgressBar::impl_getStaticSupportedServiceNames());
    } else if (unocontrols::ProgressMonitor::impl_getStaticImplementationName()
               .equalsAscii(pImplName))
    {
        fac = cppu::createSingleFactory(
            smgr,
            unocontrols::ProgressMonitor::impl_getStaticImplementationName(),
            &ProgressMonitor_createInstance,
            unocontrols::ProgressMonitor::impl_getStaticSupportedServiceNames());
    } else if (unocontrols::StatusIndicator::impl_getStaticImplementationName()
               .equalsAscii(pImplName))
    {
        fac = cppu::createSingleFactory(
            smgr,
            unocontrols::StatusIndicator::impl_getStaticImplementationName(),
            &StatusIndicator_createInstance,
            unocontrols::StatusIndicator::impl_getStaticSupportedServiceNames());
    }
    if (fac.is()) {
        fac->acquire();
    }
    return fac.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
