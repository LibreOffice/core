/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Any.hxx>

void func1();

int main()
{
    // no warning excepted
    try
    {
        func1();
    }
    catch (css::uno::Exception const&)
    {
        css::uno::Any ex(cppu::getCaughtException());
        SAL_WARN("avmedia", "exception: " << exceptionToString(ex));
    }

    try
    {
        func1();
    }
    catch (css::uno::Exception const& ex)
    {
        SAL_WARN("xmloff", "message " << ex);
        // expected-error@-1 {{use TOOLS_WARN_EXCEPTION/TOOLS_INFO_EXCEPTION/exceptionToString to print exception nicely [loplugin:logexceptionnicely]}}
    }

    try
    {
        func1();
    }
    catch (const css::lang::IndexOutOfBoundsException& ex)
    {
        SAL_WARN("xmloff", "message " << ex);
        // expected-error@-1 {{use TOOLS_WARN_EXCEPTION/TOOLS_INFO_EXCEPTION/exceptionToString to print exception nicely [loplugin:logexceptionnicely]}}
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
