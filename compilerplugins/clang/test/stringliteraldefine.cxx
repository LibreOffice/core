/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vector>

#include <rtl/ustring.hxx>
#include <sal/macros.h>

void f(OUString const&);

void f1()
{
// expected-error@+1 {{change macro 'XXX' to 'constexpr OUStringLiteral' [loplugin:stringliteraldefine]}}
#define XXX "xxx"

    // expected-note@+1 {{macro used here [loplugin:stringliteraldefine]}}
    f(OUString(XXX));

    // FIXME no warning expected
    //#define FOO f(OUString("xxx"))
    //    FOO;
}

void f2()
{
    struct DataFlavorRepresentation
    {
        OUString pMimeType;
    };

// expected-error@+1 {{change macro 'MIMETYPE_VND_SUN_XML_WRITER_ASCII' to 'constexpr OUStringLiteral' [loplugin:stringliteraldefine]}}
#define MIMETYPE_VND_SUN_XML_WRITER_ASCII "xxx"
    static const DataFlavorRepresentation aInstance[] = {
        // expected-note@+1 {{macro used here [loplugin:stringliteraldefine]}}
        { MIMETYPE_VND_SUN_XML_WRITER_ASCII },
    };
}

void f3()
{
// expected-error@+1 {{change macro 'YYY' to 'constexpr OUStringLiteral' [loplugin:stringliteraldefine]}}
#define YYY "yyy"

    // expected-note@+1 {{macro used here [loplugin:stringliteraldefine]}}
    f(YYY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
