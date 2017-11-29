/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <rtl/ustring.hxx>

#define MACRO (1)

bool foo(int);

enum class EFoo { Bar };

struct S { operator bool(); };

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
};

struct S2 {
    S2& GetText();
    void toChar();
};
void func2(S2 *p)
{
    (p->GetText()).toChar(); // expected-error {{unnecessary parentheses around member expr [loplugin:unnecessaryparen]}}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
