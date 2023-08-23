/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstddef>
#include <memory>
#include <string>
#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>

#define MACRO (1)

bool foo(int);

enum class EFoo { Bar };

struct S { operator bool(); };

enum class BrowseMode
{
    Modules = 0x01,
    Top = 0x02,
    Bottom = 0x04,
    Left = 0x08,
};
namespace o3tl
{
template <> struct typed_flags<BrowseMode> : is_typed_flags<BrowseMode, 0xf>
{
};
}

void f(char const *);
char const * operator ""_s1(char const *, std::size_t);
#if __cplusplus >= 202002L
struct Str { constexpr Str(char const *) {} };
template<Str> char const * operator ""_s2();
#endif

int main()
{
    int x = 1;
    x = ((2)); // expected-error {{parentheses around parentheses [loplugin:unnecessaryparen]}}

    if ((foo(1))) foo(2); // expected-error {{parentheses immediately inside if statement [loplugin:unnecessaryparen]}}

    foo((1)); // expected-error {{parentheses immediately inside single-arg call [loplugin:unnecessaryparen]}}

    int y = (x); // expected-error {{parentheses immediately inside vardecl statement [loplugin:unnecessaryparen]}}
    (void)y;

    EFoo efoo = EFoo::Bar;
    switch (efoo) {
        case (EFoo::Bar): break; // expected-error {{parentheses immediately inside case statement [loplugin:unnecessaryparen]}}
    }

    int z = (y) ? 1 : 0; // expected-error {{unnecessary parentheses around identifier [loplugin:unnecessaryparen]}}
    (void)z;

    int v1 = (static_cast<short>(1)) + 1; // expected-error {{unnecessary parentheses around cast [loplugin:unnecessaryparen]}}
    (void)v1;

    // No warnings, used to silence -Wunreachable-code:
    if ((false)) {
        return 0;
    }
    x = (true) ? 0 : 1;

    // More "no warnings", at least potentially used to silence -Wunreachable-code:
    while ((false)) {
        return 0;
    }
    for (; (false);) {
        return 0;
    }
    x = foo(0) && (false) ? 0 : 1;
    x = MACRO < (0) ? 0 : 1;
        // cf. odd Clang -Wunreachable-code--suppression mechanism when the macro itself contains
        // parentheses, causing the issue that lead to c421ac3f9432f2e9468d28447dc4c2e45b6f4da3
        // "Revert loplugin:unnecessaryparen warning around integer literals"

    int v2 = (1); // expected-error {{parentheses immediately inside vardecl statement [loplugin:unnecessaryparen]}}
    (void)v2;

    std::string v3;
    v3 = (std::string("xx") + "xx"); // expected-error {{parentheses immediately inside assignment [loplugin:unnecessaryparen]}}
    (void)v3;

    S s1;
    if ((s1)) { // expected-error {{parentheses immediately inside if statement [loplugin:unnecessaryparen]}}
        return 0;
    }
    S s2;
    if ((s2 = s1)) {
        return 0;
    }

    (void) sizeof (int);
    (void) sizeof (x); // expect no warning (for whatever reason; for symmetry with above case?)

    // Expecting just one error, not reported twice during TraverseInitListExpr:
    int a[] = {(x)}; // expected-error {{unnecessary parentheses around identifier [loplugin:unnecessaryparen]}}
    (void) a;

    (void) (+1); // expected-error {{unnecessary parentheses around signed numeric literal [loplugin:unnecessaryparen]}}
    (void) (-1); // expected-error {{unnecessary parentheses around signed numeric literal [loplugin:unnecessaryparen]}}

    // For simplicity's sake, even warn about pathological cases that would require adding
    // whitespace when removing the parentheses (as is also necessary in other cases anyway, like
    // "throw(x);"); it is unlikely that there are any actual occurrences of code like "-(-1)" that
    // would benefit from the parentheses readability-wise, compared to "- -1":
    (void) -(-1); // expected-error {{unnecessary parentheses around signed numeric literal [loplugin:unnecessaryparen]}}

    char *p = nullptr;
    delete (p); // expected-error {{parentheses immediately inside delete expr [loplugin:unnecessaryparen]}}

    BrowseMode nBits = ( BrowseMode::Modules | BrowseMode::Top ); // expected-error {{parentheses immediately inside vardecl statement [loplugin:unnecessaryparen]}}
    (void)nBits;

    OUString::number((v2+1)); // expected-error {{parentheses immediately inside single-arg call [loplugin:unnecessaryparen]}}

    (void) ("foo"); // expected-error {{unnecessary parentheses around single-token string literal [loplugin:unnecessaryparen]}}
    (void) ("foo" "bar");
    f(("foo")); // expected-error {{parentheses immediately inside single-arg call [loplugin:unnecessaryparen]}}
    f(("foo" "bar"));
    (void) ("foo"_s1); // expected-error {{unnecessary parentheses around single-token string literal [loplugin:unnecessaryparen]}}
    (void) ("foo" "bar"_s1);
    f(("foo"_s1)); // expected-error {{parentheses immediately inside single-arg call [loplugin:unnecessaryparen]}}
    f(("foo" "bar"_s1));
#if __cplusplus >= 202002L
    (void) ("foo"_s2); //TODO: expected error {{unnecessary parentheses around single-token string literal [loplugin:unnecessaryparen]}}
    (void) ("foo" "bar"_s2);
    f(("foo"_s2)); // TODO: expected error {{parentheses immediately inside single-arg call [loplugin:unnecessaryparen]}}
    f(("foo" "bar"_s2));
#endif
};

struct B { operator bool() const; };

struct N { bool operator !(); };

class Foo2
{
    int* p;
    B b;
    N n;

    int foo2()
    {
        return (p) ? 1 : 0; // expected-error {{unnecessary parentheses around member expr [loplugin:unnecessaryparen]}}
    }

    static int foo3(Foo2 & foo) {
        (void) !(foo.p);
        (void) !(foo.b);
        (void) !(foo.n);
        return (foo.p) ? 1 : 0;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
