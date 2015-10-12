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
#include <rtl/ref.hxx>
#include "framework/Pane.hxx"

#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <boost/noncopyable.hpp>
#include <map>
#include <memory>

namespace sd {
    class ViewShellBase;
}

namespace sd { namespace sidebar {

namespace
{
    typedef ::cppu::WeakComponentImplHelper <
        css::ui::XUIElementFactory
        > PanelFactoryInterfaceBase;
}

class PanelFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    static css::uno::Reference<css::uno::XInterface> SAL_CALL createInstance (
        const css::uno::Reference<css::lang::XMultiServiceFactory>& rxFactory);

    explicit PanelFactory (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PanelFactory();

    virtual void SAL_CALL disposing() override;

    // XUIElementFactory

    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const ::rtl::OUString& rsResourceURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
