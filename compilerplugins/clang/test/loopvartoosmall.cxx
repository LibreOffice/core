/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdint>

std::int32_t size() { return 1; }

int main() {
    for (std::int16_t i = 0; i < size(); ++i) {} // expected-error {{[loplugin:loopvartoosmall]}}
    for (std::int16_t i = 0; i <= size(); ++i) {} // expected-error {{[loplugin:loopvartoosmall]}}
    for (std::int16_t i = 0; i != size(); ++i) {} // expected-error {{[loplugin:loopvartoosmall]}}
    std::int16_t j;
    for (j = 0; j < size(); ++j) {} // expected-error {{[loplugin:loopvartoosmall]}}
    for (j = 0; j <= size(); ++j) {} // expected-error {{[loplugin:loopvartoosmall]}}
    for (j = 0; j != size(); ++j) {} // expected-error {{[loplugin:loopvartoosmall]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
