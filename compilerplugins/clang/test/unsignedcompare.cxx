/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

bool f(int i1, unsigned i2)
{
    // expected-error@+1 {{explicit cast from 'int' to 'unsigned int' (of equal rank) in comparison against 'unsigned int': if the cast value is known to be non-negative, use o3tl::make_unsigned instead of the cast [loplugin:unsignedcompare]}}
    return unsigned(i1) < i2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
