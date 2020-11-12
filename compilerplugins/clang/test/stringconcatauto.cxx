/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include <rtl/ustring.hxx>

void foo()
{
    auto str1 = "str1" + OUString::number(10);
    // expected-error-re@-1 {{creating a variable of type 'rtl::OUStringConcat<{{.*}}>' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    OUString str2 = "str2" + OUString::number(20) + "ing";
    const auto& str3 = "str3" + OUString::number(30);
    // expected-error-re@-1 {{creating a variable of type 'const rtl::OUStringConcat<{{.*}}> &' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    const auto str4 = "str4" + OString::number(40);
    // expected-error-re@-1 {{creating a variable of type 'const rtl::OStringConcat<{{.*}}>' will make it reference temporaries}}
    // expected-note@-2 {{use OString instead}}
    auto str5 = OUString::number(50);
    // expected-error-re@-1 {{creating a variable of type 'rtl::OUStringNumber<{{.*}}>' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    (void)str1;
    (void)str2;
    (void)str3;
    (void)str4;
    (void)str5;
}

struct A
{
    auto bar()
    // expected-error-re@-1 {{returning a variable of type 'rtl::OStringConcat<{{.*}}>' will make it reference temporaries}}
    // expected-note@-2 {{use OString instead}}
    {
        return "bar" + OString::number(110);
    }
    auto baz()
    // expected-error-re@-1 {{returning a variable of type 'rtl::OStringNumber<{{.*}}>' will make it reference temporaries}}
    // expected-note@-2 {{use OString instead}}
    {
        return OString::number(120);
    }
};

template <typename T> void fun(const T& par)
// parameters are without warnings
{
    const T& var = par;
    // expected-error-re@-1 {{creating a variable of type 'const rtl::OUStringConcat<{{.*}}> &' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    (void)var;
}

void testfun() { fun("fun" + OUString::number(200)); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
