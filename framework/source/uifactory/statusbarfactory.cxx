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

#include <uifactory/menubarfactory.hxx>
#include <uielement/statusbarwrapper.hxx>

#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::ui;

using namespace framework;

namespace {

class StatusBarFactory :  public MenuBarFactory
{
public:
    explicit StatusBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.StatusBarFactory"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.ui.StatusBarFactory"_ustr};
    }

    // XUIElementFactory
    virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement( const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args ) override;
};

StatusBarFactory::StatusBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    MenuBarFactory( xContext )
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL StatusBarFactory::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
{
    Reference< css::ui::XUIElement > xStatusBar = new StatusBarWrapper(m_xContext);
    MenuBarFactory::CreateUIElement(ResourceURL, Args, u"private:resource/statusbar/", xStatusBar, m_xContext);
    return xStatusBar;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_StatusBarFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new StatusBarFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
