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
#ifndef SW_SIDEBAR_PANEL_FACTORY_HXX
#define SW_SIDEBAR_PANEL_FACTORY_HXX

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/ui/XUIElementFactory.hpp>

#include <boost/noncopyable.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace sw { namespace sidebar {

namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::ui::XUIElementFactory
        > PanelFactoryInterfaceBase;
}


class SwPanelFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    static ::rtl::OUString SAL_CALL getImplementationName(void);
    static cssu::Reference<cssu::XInterface> SAL_CALL createInstance(
        const cssu::Reference<css::lang::XMultiServiceFactory>& rxFactory);
    static cssu::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames(void);

    SwPanelFactory(void);
    virtual ~SwPanelFactory(void);

    // XUIElementFactory
    cssu::Reference<css::ui::XUIElement> SAL_CALL createUIElement(
        const ::rtl::OUString& rsResourceURL,
        const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            cssu::RuntimeException );
};


} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
