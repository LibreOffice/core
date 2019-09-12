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
    auto str1 = "str1" + OUString::number( 10 );
    // expected-error@-1 {{creating a variable of type 'rtl::OUStringConcat<char const[5], rtl::OUString>' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    OUString str2 = "str2" + OUString::number( 20 ) + "ing";
    const auto& str3 = "str3" + OUString::number( 30 );
    // expected-error@-1 {{creating a variable of type 'const rtl::OUStringConcat<char const[5], rtl::OUString> &' will make it reference temporaries}}
    // expected-note@-2 {{use OUString instead}}
    const auto str4 = "str4" + OString::number( 40 );
    // expected-error@-1 {{creating a variable of type 'const rtl::OStringConcat<char const[5], rtl::OString>' will make it reference temporaries}}
    // expected-note@-2 {{use OString instead}}
    (void) str1;
    (void) str2;
    (void) str3;
    (void) str4;
}

struct A
{
    auto bar()
    // expected-error@-1 {{returning a variable of type 'rtl::OStringConcat<char const[4], rtl::OString>' will make it reference temporaries}}
    // expected-note@-2 {{use OString instead}}
    {
        return "bar" + OString::number( 110 );
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
