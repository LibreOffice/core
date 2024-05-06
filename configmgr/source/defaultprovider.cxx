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

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include "configurationprovider.hxx"
#include "defaultprovider.hxx"
#include "lock.hxx"

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_configuration_DefaultProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    osl::MutexGuard guard(*configmgr::lock());
    css::uno::Reference<css::uno::XInterface> singleton(
        configmgr::configuration_provider::createDefault(context));
    singleton->acquire();
    return singleton.get();
}

namespace configmgr::default_provider
{
OUString getImplementationName() { return u"com.sun.star.comp.configuration.DefaultProvider"_ustr; }

css::uno::Sequence<OUString> getSupportedServiceNames()
{
    return { u"com.sun.star.configuration.DefaultProvider"_ustr };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
