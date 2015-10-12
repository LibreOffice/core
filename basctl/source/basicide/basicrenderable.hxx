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
#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BASICRENDERABLE_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BASICRENDERABLE_HXX

#include <com/sun/star/view/XRenderable.hpp>
#include <cppuhelper/compbase.hxx>

#include <vcl/print.hxx>

namespace basctl
{

class BaseWindow;

class Renderable :
    public cppu::WeakComponentImplHelper< css::view::XRenderable >,
    public vcl::PrinterOptionsHelper
{
    VclPtr<BaseWindow>  mpWindow;
    osl::Mutex          maMutex;

    VclPtr<Printer> getPrinter();
public:
    explicit Renderable (BaseWindow*);
    virtual ~Renderable();

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount (
        const css::uno::Any& aSelection,
        const css::uno::Sequence<css::beans::PropertyValue >& xOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getRenderer (
        sal_Int32 nRenderer,
        const css::uno::Any& rSelection,
        const css::uno::Sequence<css::beans::PropertyValue>& rxOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL render (
        sal_Int32 nRenderer,
        const css::uno::Any& rSelection,
        const css::uno::Sequence<css::beans::PropertyValue>& rxOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BASICRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
