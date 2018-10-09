/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "redundantinline.hxx"

S1::~S1() = default;

static inline int f8() { return 0; } // expected-error {{function has no external linkage but is explicitly declared 'inline' [loplugin:redundantinline]}}

namespace {

static inline int f9() { return 0; } // expected-error {{function has no external linkage but is explicitly declared 'inline' [loplugin:redundantinline]}}

}

int main() { return f8() + f9(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
