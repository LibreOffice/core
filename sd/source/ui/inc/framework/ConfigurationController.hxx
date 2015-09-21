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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_CONFIGURATIONCONTROLLER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_CONFIGURATIONCONTROLLER_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XResourceFactoryManager.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfigurationController,
    css::lang::XInitialization
    > ConfigurationControllerInterfaceBase;

} // end of anonymous namespace.

namespace sd { class ViewShellBase; }

namespace sd { namespace framework {

/** The configuration controller is responsible for maintaining the current
    configuration.

    @see css::drawing::framework::XConfigurationController
        for an extended documentation.
*/
class ConfigurationController
    : private sd::MutexOwner,
      private boost::noncopyable,
      public ConfigurationControllerInterfaceBase
{
public:
    ConfigurationController() throw();
    virtual ~ConfigurationController() throw();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    void ProcessEvent();

    /** Normally the requested changes of the configuration are executed
        asynchronously.  However, there is at least one situation (searching
        with the Outliner) where the surrounding code does not cope with
        this.  So, instead of calling Reschedule until the global event loop
        executes the configuration update, this method does (almost) the
        same without the reschedules.

        Do not use this method until there is absolutely no other way.
    */
    void RequestSynchronousUpdate();

    // XConfigurationController

    virtual void SAL_CALL lock()
        throw (css::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL unlock()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL requestResourceActivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        css::drawing::framework::ResourceActivationMode eMode)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL requestResourceDeactivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL getResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL update()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual  css::uno::Reference<
        css::drawing::framework::XConfiguration>
        SAL_CALL getRequestedConfiguration()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual  css::uno::Reference<
        css::drawing::framework::XConfiguration>
        SAL_CALL getCurrentConfiguration()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL restoreConfiguration (
        const css::uno::Reference<css::drawing::framework::XConfiguration>&
        rxConfiguration)
        throw (css::uno::RuntimeException,
               std::exception) SAL_OVERRIDE;

    // XConfigurationControllerBroadcaster

    virtual void SAL_CALL addConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener,
        const OUString& rsEventType,
        const css::uno::Any& rUserData)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL notifyEvent (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XConfigurationRequestQueue

    virtual sal_Bool SAL_CALL hasPendingRequests()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL postChangeRequest (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeRequest>& rxRequest)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XResourceFactoryManager

    virtual void SAL_CALL addResourceFactory(
        const OUString& sResourceURL,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeResourceFactoryForURL(
        const OUString& sResourceURL)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeResourceFactoryForReference(
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::framework::XResourceFactory>
        SAL_CALL getResourceFactory (
        const OUString& sResourceURL)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** Use this class instead of calling lock() and unlock() directly in
        order to be exception safe.
    */
    class Lock
    {
    public:
        Lock (const css::uno::Reference<
            css::drawing::framework::XConfigurationController>& rxController);
        ~Lock();
    private:
        css::uno::Reference<
            css::drawing::framework::XConfigurationController> mxController;
    };

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;
    bool mbIsDisposed;

    /** When the called object has already been disposed this method throws
        an exception and does not return.
    */
    void ThrowIfDisposed () const
        throw (css::lang::DisposedException, css::uno::RuntimeException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
