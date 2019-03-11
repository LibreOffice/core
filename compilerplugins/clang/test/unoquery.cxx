/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"
#include "com/sun/star/beans/XProperty.hpp"

void foo(css::uno::Reference<css::uno::XInterface> model)
{
    css::uno::Reference<css::beans::XProperty>(model, css::uno::UNO_QUERY)->getAsProperty();
    // expected-error@-1 {{calling UNO_QUERY followed by unconditional method call might result in SIGSEGV, rather use UNO_QUERY_THROW [loplugin:unoquery]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
