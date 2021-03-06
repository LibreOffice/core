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

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>

namespace sc::sidebar {

typedef ::cppu::WeakComponentImplHelper <
    css::ui::XUIElementFactory, css::lang::XServiceInfo
    > PanelFactoryInterfaceBase;

class ScPanelFactory
    : private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    // noncopyable
    ScPanelFactory(const ScPanelFactory&) = delete;
    const ScPanelFactory& operator=(const ScPanelFactory&) = delete;

    ScPanelFactory();
    virtual ~ScPanelFactory() override;

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement(
        const OUString& rsResourceURL,
        const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

} // end of namespace sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
