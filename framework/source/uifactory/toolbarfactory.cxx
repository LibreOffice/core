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

#include <com/sun/star/frame/ModuleManager.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <uielement/toolbarwrapper.hxx>
#include <uifactory/menubarfactory.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;
using namespace framework;

namespace {

class ToolBarFactory :  public MenuBarFactory
{
public:
    explicit ToolBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.ToolBarFactory");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.ui.ToolBarFactory" };
        return aSeq;
    }

    // XUIElementFactory
    virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement(
            const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args )
        throw ( css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
};

ToolBarFactory::ToolBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    MenuBarFactory( xContext )
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL ToolBarFactory::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception )
{
    Reference< css::ui::XUIElement > xToolBar(
           static_cast<OWeakObject *>(new ToolBarWrapper(m_xContext)), UNO_QUERY);
    CreateUIElement(ResourceURL, Args, "PopupMode", "private:resource/toolbar/", xToolBar, m_xContext);
    return xToolBar;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ToolBarFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ToolBarFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
