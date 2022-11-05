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
    // expected-error-re@-1 {{creating a variable of type {{.+}} will make it reference temporaries}}
    // expected-note@-2 {{use O(U)String instead}}
    OUString str2 = "str2" + OUString::number(20) + "ing";
    const auto& str3 = "str3" + OUString::number(30);
    // expected-error-re@-1 {{creating a variable of type {{.+}} will make it reference temporaries}}
    // expected-note@-2 {{use O(U)String instead}}
    const auto str4 = "str4" + OString::number(40);
    // expected-error-re@-1 {{creating a variable of type {{.+}} will make it reference temporaries}}
    // expected-note@-2 {{use O(U)String instead}}
    auto str5 = OUString::number(50);
    auto str6 = OUString::number(50).toAsciiUpperCase();
    (void)str1;
    (void)str2;
    (void)str3;
    (void)str4;
    (void)str5;
    (void)str6;
}

struct A
{
    auto bar()
    // expected-error-re@-1 {{returning a variable of type {{.+}} will make it reference temporaries}}
    // expected-note@-2 {{use O(U)String instead}}
    {
        return "bar" + OString::number(110);
    }
    auto baz() { return OString::number(120); }
    auto baz2() { return OString::number(120).toAsciiUpperCase(); }
};

template <typename T> void fun(const T& par)
// parameters are without warnings
{
    const T& var = par;
    // expected-error-re@-1 {{creating a variable of type 'const rtl::StringConcat<{{.*}}> &' will make it reference temporaries}}
    // expected-note@-2 {{use O(U)String instead}}
    (void)var;
}

void testfun() { fun("fun" + OUString::number(200)); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
