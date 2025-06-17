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

#pragma once

#include <sddllapi.h>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <memory>

namespace sd { class DrawController; }

namespace sd::framework {

/** The configuration controller is responsible for maintaining the current
    configuration.

    @see ConfigurationController
        for an extended documentation.
*/
class SD_DLLPUBLIC ConfigurationController final
    : private cppu::BaseMutex,
      public  cppu::WeakComponentImplHelperBase
{
public:
    ConfigurationController(const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~ConfigurationController() noexcept override;
    ConfigurationController(const ConfigurationController&) = delete;
    ConfigurationController& operator=(const ConfigurationController&) = delete;

    virtual void SAL_CALL disposing() override;

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

    void lock();

    void unlock();

    void requestResourceActivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        css::drawing::framework::ResourceActivationMode eMode);

    void requestResourceDeactivation (
        const css::uno::Reference<css::drawing::framework::XResourceId>&
            rxResourceId);

    css::uno::Reference<css::drawing::framework::XResource>
        getResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);

    void update();

    css::uno::Reference<css::drawing::framework::XConfiguration>
        getRequestedConfiguration();

    css::uno::Reference<css::drawing::framework::XConfiguration>
        getCurrentConfiguration();

    void restoreConfiguration (
        const css::uno::Reference<css::drawing::framework::XConfiguration>&
        rxConfiguration);

    void addConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener,
        const OUString& rsEventType,
        const css::uno::Any& rUserData);

    void removeConfigurationChangeListener (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener);

    void notifyEvent (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent);

    /** Return whether there are pending requests for configuration changes.
        @return
            Returns `TRUE` when there is at least one request object in the
            queue that has not yet been processed.  It returns `FALSE` when
            the queue is empty.
    */
    bool hasPendingRequests();

    /** Add a request for a configuration change to the request queue.
        <p>This method should not be called from outside the drawing
        framework.  Other sub controllers of the drawing framework are typical
        callers.  They can add change requests that can not be made with the
        requestResourceActivation() and
        requestResourceDeactivation() methods.</p>
        @param xRequest
            The configuration change represented by this request object must only
            be committed to the configuration when the
            com::sun::star::drawing::framework::XConfigurationChangeRequest::execute()
            method of the xRequest object is called.
    */
    void postChangeRequest (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeRequest>& rxRequest);

    void addResourceFactory(
        const OUString& sResourceURL,
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory);

    void removeResourceFactoryForURL(
        const OUString& sResourceURL);

    void removeResourceFactoryForReference(
        const css::uno::Reference<css::drawing::framework::XResourceFactory>& rxResourceFactory);

    css::uno::Reference<css::drawing::framework::XResourceFactory>
        getResourceFactory (
        const OUString& sResourceURL);

    /** Use this class instead of calling lock() and unlock() directly in
        order to be exception safe.
    */
    class Lock
    {
    public:
        Lock (const rtl::Reference<ConfigurationController>& rxController);
        ~Lock();
    private:
        rtl::Reference<ConfigurationController> mxController;
    };

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImplementation;
    bool mbIsDisposed;

    /** When the called object has already been disposed this method throws
        an exception and does not return.

        @throws css::lang::DisposedException
        @throws css::uno::RuntimeException
    */
    void ThrowIfDisposed () const;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
