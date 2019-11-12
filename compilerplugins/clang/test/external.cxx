/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// expected-error@+1 {{externally available entity 'n1' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int n1 = 0;

int const n2 = 0; // no warning, internal linkage

constexpr int n3 = 0; // no warning, internal linkage

int main()
{
    (void)n2;
    (void)n3;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
