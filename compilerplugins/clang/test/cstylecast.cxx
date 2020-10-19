/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/types.h>

namespace N
{
enum E
{
    E1
};

using T = unsigned int;
}

void FunctionalCast(void* p)
{
    // expected-error-re@+1 {{Function-style cast from 'void *' to 'sal_IntPtr' (aka '{{.+}}') (performs: reinterpret_cast) (PointerToIntegral) [loplugin:cstylecast]}}
    auto n = sal_IntPtr(p);
    (void(n)); // no warning expected (outer parens to disambiguate expr vs. decl)
}

static const int C
    = (int)0; // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}

int main()
{
    constexpr int c1 = 0;
    int const c2 = 0;
    int n
        = (int)0; // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (signed int)0; // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)~0; // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)-0; // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)+0; // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)!0; // expected-error {{C-style cast from 'bool' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (int) // expected-error {{C-style cast from 'int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        (0 << 0);
    n = (int) // expected-error {{C-style cast from 'const int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        c1;
    n = (int) // expected-error {{C-style cast from 'const int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        c2;
    n = (int) // expected-error {{C-style cast from 'const int' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        C;
    n = (int) // expected-error {{C-style cast from 'N::E' to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        N::E1;
    n = (N::E) // expected-error {{C-style cast from 'N::E' to 'N::E' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        N::E1;
    n = (enum // expected-error {{C-style cast from 'N::E' to 'enum N::E' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
         N::E)N::E1;
    n = (N::T)0; // expected-error {{C-style cast from 'int' to 'N::T' (aka 'unsigned int') (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    n = (int) // expected-error-re {{C-style cast from {{.*}} to 'int' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
        sizeof(int);
    n = (int) // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
        n;
    n = (int)~n; // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)-n; // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)+n; // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
    n = (int)!n; // expected-error {{C-style cast from 'bool' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
    n = (int) // expected-error {{C-style cast from 'int' to 'int' (performs: static_cast) (NoOp) [loplugin:cstylecast]}}
        (0 << n);
    n = (double)0; // expected-error {{C-style cast from 'int' to 'double' (performs: functional cast) (NoOp) [loplugin:cstylecast]}}
    (void)n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
