/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <utility>

struct S
{
};

void f(S const& s1, S s2)
{
    // expected-error-re@+1 {{suspicious 'std::move' from 'const S' to const-qualified '{{.+}}' (aka 'const S') [loplugin:constmove]}}
    S v1(std::move(s1));
    (void)v1;
    S v2(std::move(s2));
    (void)v2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
