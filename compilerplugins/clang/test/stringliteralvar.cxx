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

// expected-error-re@+1 {{change type of variable 'literal1' from constant character array ('const char{{ ?}}[4]') to OStringLiteral [loplugin:stringliteralvar]}}
char const literal1[] = "foo";
OString f1()
{
    // expected-note@+1 {{first passed into a 'rtl::OString' constructor here [loplugin:stringliteralvar]}}
    return literal1;
}

void f(OUString const&);
void f2()
{
    // expected-error-re@+1 {{change type of variable 'literal' from constant character array ('const char{{ ?}}[4]') to OUStringLiteral, and make it static [loplugin:stringliteralvar]}}
    char const literal[] = "foo";
    // expected-note@+1 {{first passed into a 'rtl::OUString' constructor here [loplugin:stringliteralvar]}}
    f(literal);
}

struct S3
{
    // expected-error-re@+1 {{change type of variable 'literal' from constant character array ('const char16_t{{ ?}}[4]') to OUStringLiteral [loplugin:stringliteralvar]}}
    static constexpr char16_t literal[] = u"foo";
};
void f3()
{
    // expected-note@+1 {{first passed into a 'rtl::OUString' constructor here [loplugin:stringliteralvar]}}
    f(S3::literal);
}

std::vector<OUString> f4()
{
    // expected-error-re@+1 {{change type of variable 'literal' from constant character array ('const char16_t{{ ?}}[4]') to OUStringLiteral [loplugin:stringliteralvar]}}
    static constexpr char16_t literal[] = u"foo";
    // expected-note@+1 {{first passed into a 'rtl::OUString' constructor here [loplugin:stringliteralvar]}}
    return { literal };
}

void f5()
{
    // expected-error@+1 {{variable 'literal' of type 'const rtl::OUStringLiteral<4>' with automatic storage duration most likely needs to be static [loplugin:stringliteralvar]}}
    OUStringLiteral const literal = u"foo";
    // expected-note@+1 {{first converted to 'rtl::OUString' here [loplugin:stringliteralvar]}}
    f(literal);
}

void f6()
{
    // expected-error@+1 {{variable 'literal' of type 'const rtl::OUStringLiteral<4>' with automatic storage duration most likely needs to be static [loplugin:stringliteralvar]}}
    constexpr OUStringLiteral literal = u"foo";
    // expected-note@+1 {{first converted to 'rtl::OUString' here [loplugin:stringliteralvar]}}
    f(literal);
}

void f7()
{
    static constexpr OUStringLiteral const literal = u"foo";
    f(literal);
}

void f8()
{
    static constexpr OUStringLiteral const literal = u"foo";
    // expected-error@+1 {{variable 'literal' of type 'const rtl::OUStringLiteral<4>' suspiciously used in a sizeof expression [loplugin:stringliteralvar]}}
    (void)sizeof literal;
}

void f9()
{
    // expected-error-re@+1 {{change type of variable 'literal' from constant character array ('const sal_Unicode{{ ?}}[3]'{{( \(aka 'const char16_t\[3\]'\))?}}) to OUStringLiteral [loplugin:stringliteralvar]}}
    static sal_Unicode const literal[] = { 'f', 'o', 'o' };
    // expected-note@+1 {{first passed into a 'rtl::OUString' constructor here [loplugin:stringliteralvar]}}
    f(OUString(literal, SAL_N_ELEMENTS(literal)));
}

void f10()
{
    // expected-error-re@+1 {{change type of variable 'literal' from constant character array ('const sal_Unicode{{ ?}}[3]'{{( \(aka 'const char16_t\[3\]'\))?}}) to OUStringLiteral [loplugin:stringliteralvar]}}
    static sal_Unicode const literal[] = { 'f', 'o', 'o' };
    // expected-note@+1 {{first passed into a 'rtl::OUString' constructor here [loplugin:stringliteralvar]}}
    f(OUString(literal, 3));
}

void f11(int nStreamType)
{
    // expected-error-re@+1 {{change type of variable 'sDocumentType' from constant character array ('const char{{ ?}}[4]') to OUStringLiteral, and make it static [loplugin:stringliteralvar]}}
    const char sDocumentType[] = "foo";
    OUString sStreamType;
    switch (nStreamType)
    {
        case 1:
            // expected-note@+1 {{first assigned here [loplugin:stringliteralvar]}}
            sStreamType = sDocumentType;
            break;
    }
    (void)sStreamType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
