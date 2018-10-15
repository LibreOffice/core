/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
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
