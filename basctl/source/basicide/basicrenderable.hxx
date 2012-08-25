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
#ifndef BASCTL_BASICRENDERABLE_HXX
#define BASCTL_BASICRENDERABLE_HXX

#include "com/sun/star/view/XRenderable.hpp"
#include "cppuhelper/compbase1.hxx"

#include "vcl/print.hxx"

namespace basctl
{

class BaseWindow;

class Renderable :
    public cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >,
    public vcl::PrinterOptionsHelper
{
    BaseWindow*         mpWindow;
    osl::Mutex          maMutex;

    Printer* getPrinter();
public:
    Renderable (BaseWindow*);
    virtual ~Renderable();

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount (
        const com::sun::star::uno::Any& aSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue >& xOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> SAL_CALL getRenderer (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL render (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

};

} // namespace basctl

#endif // BASCTL_BASICRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
