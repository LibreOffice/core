/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef NDEBUG

#include "sal/config.h"

#include <string_view>

#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "sal/types.h"

void f1a(std::string_view);
// expected-error@+1 {{replace function parameter of type 'const rtl::OString &' with 'std::string_view' [loplugin:stringviewparam]}}
char f1b(OString const& s)
{
    f1a(s);
    if (s.isEmpty())
    {
        f1a(std::string_view(s));
    }
    return s[0];
}

void f2a(std::u16string_view);
// expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
sal_Unicode f2b(OUString const& s)
{
    f2a(s);
    if (s.isEmpty())
    {
        f2a(std::u16string_view(s));
    }
    return s[0];
}

void f3a(OUString const&) {}
using F3 = void(OUString const&);
F3* f3b() { return f3a; }

SAL_DLLPUBLIC_EXPORT void f4(OUString const&) {}

template <typename T> void f5(T const&);
template <> void f5<OUString>(OUString const&) {}

void f6([[maybe_unused]] OUString const&) {}

bool f7(
    // expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
    const OUString& p1,
    // expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
    const OUString& p2)
{
    return p1 == p2;
}
// expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
bool f8(const OUString& p1, std::u16string_view p2) { return p1 == p2; }

struct Converter
{
    // expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
    static bool convertBool(bool& rBool, const OUString& rString)
    {
        rBool = rString == "true";
        return rBool || (rString == "false");
    }
};

void f9(std::u16string_view);
void f9(OUString const& s) { return f9(std::u16string_view(s)); }

struct S10
{
    S10(std::u16string_view);
    S10(OUString const& s)
        : S10(std::u16string_view(s))
    {
    }
};

// expected-error@+1 {{replace function parameter of type 'const rtl::OUString &' with 'std::u16string_view' [loplugin:stringviewparam]}}
void f11(const OUString& f11rString)
{
    OUStringBuffer buf;
    buf.append(f11rString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
