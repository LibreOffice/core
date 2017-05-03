/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

void f1(char *) {}
void f2(char const *) {}

enum Enum1 { X };

int main() {
    char * p1;
    char const * p2;
    p1 = nullptr;
    p2 = "";
    f1(const_cast<char *>(p1)); // expected-error {{redundant const_cast from 'char *' to 'char *' [loplugin:redundantcast]}}
    f1(const_cast<char * const>(p1)); // expected-error {{redundant const_cast from 'char *' to 'char *const' [loplugin:redundantcast]}}
    f1(const_cast<char *>(p2));
    f1(const_cast<char * const>(p2));
    f2(const_cast<char *>(p1)); // expected-error {{redundant const_cast from 'char *' to 'char *' [loplugin:redundantcast]}}
    f2(const_cast<char * const>(p1)); //  expected-error {{redundant const_cast from 'char *' to 'char *const' [loplugin:redundantcast]}}
    f2(const_cast<char const *>(p1));
    f2(const_cast<char const * const>(p1));
    f2(const_cast<char *>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'char *', result is implicitly cast to 'const char *' [loplugin:redundantcast]}}
    f2(const_cast<char * const>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'char *', result is implicitly cast to 'const char *' [loplugin:redundantcast]}}
    f2(const_cast<char const *>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'const char *' [loplugin:redundantcast]}}
    f2(const_cast<char const * const>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'const char *const' [loplugin:redundantcast]}}

    Enum1 e = (Enum1)Enum1::X; // expected-error {{redundant cstyle cast from 'Enum1' to 'Enum1' [loplugin:redundantcast]}}
    (void)e;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
