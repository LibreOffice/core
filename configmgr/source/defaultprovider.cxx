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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

#include "configurationprovider.hxx"
#include "defaultprovider.hxx"
#include "lock.hxx"

namespace configmgr { namespace default_provider {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    osl::MutexGuard guard(*lock());
    static css::uno::Reference< css::uno::XInterface > singleton(
        configuration_provider::createDefault(context));
    return singleton;
}

OUString getImplementationName() {
    return OUString("com.sun.star.comp.configuration.DefaultProvider");
}

css::uno::Sequence< OUString > getSupportedServiceNames() {
    return css::uno::Sequence< OUString > { "com.sun.star.configuration.DefaultProvider" };
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
