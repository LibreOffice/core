/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


class S final {
protected:
    void f(int f) { f1 = f; }  // expected-error {{final class should not have protected members - convert them to private [loplugin:finalprotected]}}
    int f1;      // expected-error {{final class should not have protected members - convert them to private [loplugin:finalprotected]}}
public:
    void g();
    int g1;
private:
    void h();
    int h1;
};

class S2 {
protected:
    void f(int f) { f1 = f; }
    int f1;
public:
    void g();
    int g1;
private:
    void h();
    int h1;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
