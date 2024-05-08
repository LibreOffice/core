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

#include "NameContainer.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

namespace {

class Implementation:
    public cppu::ImplInheritanceHelper<
        NameContainer<css::uno::Reference<css::beans::XPropertySet>>,
        css::lang::XServiceInfo>
{
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.XForms"_ustr; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.xforms.XForms"_ustr};
    }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_form_XForms_get_implementation(uno::XComponentContext*,
        uno::Sequence<uno::Any> const &)
{
    return cppu::acquire(new Implementation);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
