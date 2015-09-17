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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_PRESENTATIONFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_PRESENTATIONFACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > PresentationFactoryInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** This factory creates a marker view whose existence in a configuration
    indicates that a slideshow is running (in another but associated
    application window).
*/
class PresentationFactory
    : private sd::MutexOwner,
      public PresentationFactoryInterfaceBase
{
public:
    static const OUString msPresentationViewURL;

    PresentationFactory (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~PresentationFactory();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxViewId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;

    void ThrowIfDisposed() const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
