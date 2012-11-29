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

#ifndef SD_FRAMEWORK_TASK_PANEL_FACTORY_HXX
#define SD_FRAMEWORK_TASK_PANEL_FACTORY_HXX

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>

#include <boost/scoped_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
    css::lang::XInitialization,
    css::drawing::framework::XResourceFactory
    > TaskPanelFactoryInterfaceBase;

} // end of anonymous namespace.


namespace sd { class ViewShellBase; }

namespace sd { namespace framework {

/** This class creates panels for the task pane.
*/
class TaskPanelFactory
    : private ::cppu::BaseMutex,
      public TaskPanelFactoryInterfaceBase
{
public:
    TaskPanelFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~TaskPanelFactory (void);

    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxResourcesId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException);

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<
            css::drawing::framework::XResource>& rxResource)
        throw (css::uno::RuntimeException);

private:
    ViewShellBase* mpViewShellBase;
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
