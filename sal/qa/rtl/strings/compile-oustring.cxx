/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/string.h>
#include <rtl/ustring.hxx>

// expected-note@rtl/ustring.hxx:* 1+ {{}}

void checkExtraIntArgument()
{
    // This makes sure that using by mistake RTL_CONSTASCII_STRINGPARAM does not trigger a different
    // overload, i.e. the second argument to match() in this case is the indexFrom argument,
    // but with the macro it would contain the length of the string. Therefore
    // match( RTL_CONSTASCII_STRINGPARAM( "bar" )) would be match( "bar", 3 ), which would be
    // true when called for OUString( "foobar" ). But this should not happen because of the
    // &foo[0] trick in the RTL_CONSTASCII_STRINGPARAM macro.
    // expected-error@+1 {{}}
    OUString("foobar").match(RTL_CONSTASCII_STRINGPARAM("bar"));
}

void checkNonconstChar()
{
    // check that non-const char[] data do not trigger string literal overloads
    char test[] = "test";
    char bar[] = "bar";
    const char consttest[] = "test";
    const char constbar[] = "bar";
    // expected-error@+1 {{}}
    (void)OUString("footest").replaceAll(test, bar);
    // expected-error@+1 {{}}
    (void)OUString("footest").replaceAll(consttest, bar);
    // expected-error@+1 {{}}
    (void)OUString("footest").replaceAll(test, constbar);
    (void)OUString("footest").replaceAll(consttest, constbar);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
