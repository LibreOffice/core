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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERFRAMEWORKOBSERVER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERFRAMEWORKOBSERVER_HXX

#include <framework/ConfigurationChangeListener.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <functional>

namespace sd::framework { class ConfigurationController; }

namespace sdext::presenter {

/** Watch the drawing framework for changes and run callbacks when a certain
    change takes place.
*/
class PresenterFrameworkObserver
    : public sd::framework::ConfigurationChangeListener
{
public:
    typedef ::std::function<void (bool)> Action;

    PresenterFrameworkObserver(const PresenterFrameworkObserver&) = delete;
    PresenterFrameworkObserver& operator=(const PresenterFrameworkObserver&) = delete;

    static void RunOnUpdateEnd (
        const rtl::Reference<::sd::framework::ConfigurationController>& rxController,
        const Action& rAction);

    virtual void disposing(std::unique_lock<std::mutex>&) override;
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;
    virtual void notifyConfigurationChange (
        const sd::framework::ConfigurationChangeEvent& rEvent) override;

private:
    rtl::Reference<sd::framework::ConfigurationController> mxConfigurationController;
    Action maAction;

    /** Create a new PresenterFrameworkObserver object.
        @param rPredicate
            This functor tests whether the action is to be executed or not.
        @param rAction
            The functor to execute when the predicate returns true,
            e.g. when some resource has been created.
    */
    PresenterFrameworkObserver (
        rtl::Reference<sd::framework::ConfigurationController> xController,
        const Action& rAction);
    virtual ~PresenterFrameworkObserver() override;

    void Shutdown();
};

}  // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
