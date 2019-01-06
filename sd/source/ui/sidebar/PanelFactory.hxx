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
#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_PANELFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_PANELFACTORY_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/ui/XUIElementFactory.hpp>

namespace sd { namespace sidebar {

typedef ::cppu::WeakComponentImplHelper <
    css::ui::XUIElementFactory
    > PanelFactoryInterfaceBase;

class PanelFactory
    : private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    explicit PanelFactory ();
    virtual ~PanelFactory() override;
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;

    virtual void SAL_CALL disposing() override;

    // XUIElementFactory

    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const OUString& rsResourceURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
