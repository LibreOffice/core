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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_VIEWPAGERMODULE_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_VIEWPAGERMODULE_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    css::drawing::framework::XConfigurationChangeListener
    > ViewPagerModuleInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** This module is responsible for showing the ViewPager above the view in
    the center pane.
*/
class ViewPagerModule
    : private sd::MutexOwner,
      public ViewPagerModuleInterfaceBase
{
public:
    /** Create a new module that controls the view tab bar above the view
        in the specified pane.
        @param rxController
            This is the access point to the drawing framework.
        @param rxViewPagerId
            This ResourceId specifies which tab bar is to be managed by the
            new module.
    */
    ViewPagerModule (
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<
            css::drawing::framework::XResourceId>& rxViewPagerId);
    virtual ~ViewPagerModule();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewPagerId;

    /** This is the place where the view tab bar is filled.  Only missing
        buttons are added, so it is safe to call this method multiple
        times.
    */
    void UpdateViewPager (
        const css::uno::Reference<css::drawing::framework::XTabBar>& rxTabBar);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
