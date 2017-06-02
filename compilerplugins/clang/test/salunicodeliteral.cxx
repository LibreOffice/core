/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "sal/types.h"

#include "salunicodeliteral.hxx"

#define TEST1 'x'
#define TEST2 sal_Unicode('x')

namespace {

void f(sal_Unicode) {}

}

void test() {
    f(sal_Unicode('x')); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    f(static_cast<sal_Unicode>('x')); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    using T = sal_Unicode const;
    f(static_cast<T>('x')); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'T' (aka 'const char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    f((sal_Unicode) 'x'); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    f(sal_Unicode(('x'))); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    f(sal_Unicode(120)); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    f(sal_Unicode(TEST1));
    f(TEST2); // expected-error {{in LIBO_INTERNAL_ONLY code, replace literal cast to 'sal_Unicode' (aka 'char16_t') with a u'...' char16_t character literal [loplugin:salunicodeliteral]}}
    char c = 'x';
    f(sal_Unicode(c));
    f(char16_t('x'));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
