/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined _WIN32 //TODO, see corresponding TODO in compilerplugins/clang/writeonlyvars.cxx
// expected-no-diagnostics
#else

#include <com/sun/star/uno/Any.hxx>

namespace test1
{
int const aFormalArgs[] = { 1, 2 };
// expected-error@+1 {{static var can be const [loplugin:constvars]}}
static sal_uInt16 nMediaArgsCount = SAL_N_ELEMENTS(aFormalArgs);
sal_uInt16 foo()
{
    (void)aFormalArgs;
    return nMediaArgsCount;
}
};

// no warning expected
namespace test2
{
static char const* ar[] = { "xxxx" };
static const char* lcl_DATA_OTHERS = "localedata_others";
void foo()
{
    (void)ar;
    (void)lcl_DATA_OTHERS;
}
};

// no warning expected
namespace test3
{
static sal_uInt16 nMediaArgsCount = 1; // loplugin:constvars:ignore
sal_uInt16 foo() { return nMediaArgsCount; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
