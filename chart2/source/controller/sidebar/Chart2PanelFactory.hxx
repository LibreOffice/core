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
#ifndef INCLUDED_SC_INC_SCPANELFACTORY_HXX
#define INCLUDED_SC_INC_SCPANELFACTORY_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <boost/noncopyable.hpp>


namespace chart { namespace sidebar {

namespace
{
    typedef ::cppu::WeakComponentImplHelper <
        css::ui::XUIElementFactory, css::lang::XServiceInfo
        > PanelFactoryInterfaceBase;
}

class ChartPanelFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    ChartPanelFactory();
    virtual ~ChartPanelFactory();

    // XUIElementFactory
    virtual css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement(
        const ::rtl::OUString& rsResourceURL,
        const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

} } // end of namespace sc::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
