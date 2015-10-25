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

#ifndef INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERHELPER_HXX
#define INCLUDED_SD_SOURCE_UI_PRESENTER_PRESENTERHELPER_HXX

#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <boost/noncopyable.hpp>

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper<
        css::lang::XInitialization,
        css::drawing::XPresenterHelper
    > PresenterHelperInterfaceBase;
}

/** Implementation of the XPresenterHelper interface: functionality that can
    not be implemented in an extension.
*/
class PresenterHelper
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterHelperInterfaceBase
{
public:
    explicit PresenterHelper (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterHelper();

    // XInitialize

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw(css::uno::Exception,css::uno::RuntimeException, std::exception) override;

    // XPresenterHelper

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        sal_Bool bCreateSystemChildWindow,
        sal_Bool bInitiallyVisible,
        sal_Bool bEnableChildTransparentMode,
        sal_Bool bEnableParentClip)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::rendering::XCanvas> SAL_CALL createSharedCanvas (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxUpdateCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxUpdateWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxSharedCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
        const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::rendering::XCanvas> SAL_CALL createCanvas (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        sal_Int16 nRequestedCanvasFeatures,
        const OUString& rsOptionalCanvasServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL toTop (
        const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL loadBitmap (
        const OUString& rsURL,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL captureMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL releaseMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getWindowExtentsRelative (
        const css::uno::Reference<css::awt::XWindow>& rxChildWindow,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
