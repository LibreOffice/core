/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "redundantcast.hxx"

void f1(char *) {}
void f2(char const *) {}

struct D: S {};

enum Enum1 { X };

void testConstCast() {
    char * p1;
    char const * p2;
    p1 = nullptr;
    p2 = "";
    f1(const_cast<char *>(p1)); // expected-error {{redundant const_cast from 'char *' lvalue to 'char *' prvalue [loplugin:redundantcast]}}
    f1(const_cast<char * const>(p1)); // expected-error {{redundant const_cast from 'char *' lvalue to 'char *const' prvalue [loplugin:redundantcast]}}
    f1(const_cast<char *>(p2));
    f1(const_cast<char * const>(p2));
    f2(const_cast<char *>(p1)); // expected-error {{redundant const_cast from 'char *' lvalue to 'char *' prvalue [loplugin:redundantcast]}}
    f2(const_cast<char * const>(p1)); //  expected-error {{redundant const_cast from 'char *' lvalue to 'char *const' prvalue [loplugin:redundantcast]}}
    f2(const_cast<char const *>(p1));
    f2(const_cast<char const * const>(p1));
    f2(const_cast<char *>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'char *', result is implicitly cast to 'const char *' [loplugin:redundantcast]}}
    f2(const_cast<char * const>(p2)); // expected-error {{redundant const_cast from 'const char *' to 'char *', result is implicitly cast to 'const char *' [loplugin:redundantcast]}}
    f2(const_cast<char const *>(p2)); // expected-error {{redundant const_cast from 'const char *' lvalue to 'const char *' prvalue [loplugin:redundantcast]}}
    f2(const_cast<char const * const>(p2)); // expected-error {{redundant const_cast from 'const char *' lvalue to 'const char *const' prvalue [loplugin:redundantcast]}}

    void * vp = nullptr;
    (void) const_cast<char *>(static_cast<char const *>(vp)); // expected-error {{redundant static_cast/const_cast combination from 'void *' via 'const char *' to 'char *' [loplugin:redundantcast]}}
    (void) const_cast<char *>(static_cast<char const *>(nullptr)); // expected-error {{redundant static_cast/const_cast combination from 'nullptr_t' via 'const char *' to 'char *' [loplugin:redundantcast]}}
    (void) const_cast<S &>(static_cast<S const &>(D{})); // expected-error {{redundant static_cast/const_cast combination from 'D' via 'const S &' to 'S &' [loplugin:redundantcast]}}

    S const s{};
    const_cast<S &>(s).f1();
    const_cast<S &>(s).f2(); // expected-error {{redundant const_cast from 'const S' to 'S', result is implicitly cast to 'const S' [loplugin:redundantcast]}}
    const_cast<S &>(s).f3();
    s.f3();

    // non-class lvalue, non-const:
    int ni{};
//  (void) const_cast<int>(ni);
    (void) const_cast<int &>(ni); // expected-error {{redundant const_cast from 'int' lvalue to 'int &' lvalue [loplugin:redundantcast]}}
    (void) const_cast<int &&>(ni);
//  (void) const_cast<int const>(ni);
    (void) const_cast<int const &>(ni);
    (void) const_cast<int const &&>(ni);

    // non-class lvalue, const:
    int const ci{};
//  (void) const_cast<int>(ci);
    (void) const_cast<int &>(ci);
    (void) const_cast<int &&>(ci);
//  (void) const_cast<int const>(ci);
    (void) const_cast<int const &>(ci); // expected-error {{redundant const_cast from 'const int' lvalue to 'const int &' lvalue [loplugin:redundantcast]}}
    (void) const_cast<int const &&>(ci);

    // non-class xvalue, non-const:
//  (void) const_cast<int>(nix());
//  (void) const_cast<int &>(nix());
    (void) const_cast<int &&>(nix()); // expected-error {{redundant const_cast from 'int' xvalue to 'int &&' xvalue [loplugin:redundantcast]}}
//  (void) const_cast<int const>(nix());
//  (void) const_cast<int const &>(nix());
    (void) const_cast<int const &&>(nix());

    // non-class xvalue, const:
//  (void) const_cast<int>(cix());
//  (void) const_cast<int &>(cix());
    (void) const_cast<int &&>(cix());
//  (void) const_cast<int const>(cix());
//  (void) const_cast<int const &>(cix());
    (void) const_cast<int const &&>(cix()); // expected-error {{redundant const_cast from 'const int' xvalue to 'const int &&' xvalue [loplugin:redundantcast]}}

    // non-class prvalue, non-const:
//  (void) const_cast<int>(nir());
//  (void) const_cast<int &>(nir());
//  (void) const_cast<int &&>(nir());
//  (void) const_cast<int const>(nir());
//  (void) const_cast<int const &>(nir());
//  (void) const_cast<int const &&>(nir());

    // non-class prvalue, const:
//  (void) const_cast<int>(cir());
//  (void) const_cast<int &>(cir());
//  (void) const_cast<int &&>(cir());
//  (void) const_cast<int const>(cir());
//  (void) const_cast<int const &>(cir());
//  (void) const_cast<int const &&>(cir());

    // class lvalue, non-const:
    S ns{};
//    (void) const_cast<S>(ns);
    (void) const_cast<S &>(ns); // expected-error {{redundant const_cast from 'S' lvalue to 'S &' lvalue [loplugin:redundantcast]}}
    (void) const_cast<S &&>(ns);
//  (void) const_cast<S const>(ns);
    (void) const_cast<S const &>(ns);
    (void) const_cast<S const &&>(ns);

    // class lvalue, const:
    S const cs{};
//  (void) const_cast<S>(cs);
    (void) const_cast<S &>(cs);
    (void) const_cast<S &&>(cs);
//  (void) const_cast<S const>(cs);
    (void) const_cast<S const &>(cs); // expected-error {{redundant const_cast from 'const S' lvalue to 'const S &' lvalue [loplugin:redundantcast]}}
    (void) const_cast<S const &&>(cs);

    // class xvalue, non-const:
//  (void) const_cast<S>(nsx());
//  (void) const_cast<S &>(nsx());
    (void) const_cast<S &&>(nsx()); // expected-error {{redundant const_cast from 'S' xvalue to 'S &&' xvalue [loplugin:redundantcast]}}
//  (void) const_cast<S const>(nsx());
//  (void) const_cast<S const &>(nsx());
    (void) const_cast<S const &&>(nsx());

    // class xvalue, const:
//  (void) const_cast<S>(csx());
//  (void) const_cast<S &>(csx());
    (void) const_cast<S &&>(csx());
//  (void) const_cast<S const>(csx());
//  (void) const_cast<S const &>(csx());
    (void) const_cast<S const &&>(csx()); // expected-error {{redundant const_cast from 'const S' xvalue to 'const S &&' xvalue [loplugin:redundantcast]}}

    // class prvalue, non-const:
//  (void) const_cast<S>(nsr());
//  (void) const_cast<S &>(nsr());
    (void) const_cast<S &&>(nsr());
//  (void) const_cast<S const>(nsr());
//  (void) const_cast<S const &>(nsr());
    (void) const_cast<S const &&>(nsr());

    // class prvalue, const:
//  (void) const_cast<S>(csr());
//  (void) const_cast<S &>(csr());
    (void) const_cast<S &&>(csr());
//  (void) const_cast<S const>(csr());
//  (void) const_cast<S const &>(csr());
    (void) const_cast<S const &&>(csr());
}

void testStaticCast() {
    // non-class lvalue, non-const:
    int ni{};
    (void) static_cast<int>(ni); // expected-error {{static_cast from 'int' lvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(ni);
    (void) static_cast<int &>(ni); // expected-error {{static_cast from 'int' lvalue to 'int &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int &&>(ni);
    (void) static_cast<int const>(ni); // expected-error {{in static_cast from 'int' lvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(ni); // expected-error {{static_cast from 'int' lvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(ni);
    (void) static_cast<int const &>(ni); // expected-error {{static_cast from 'int' lvalue to 'const int &' lvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<int const &>(ni);
    (void) static_cast<int const &&>(ni); // expected-error {{static_cast from 'int' lvalue to 'const int &&' xvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<int const &&>(ni);

    // non-class lvalue, const:
    int const ci{};
    (void) static_cast<int>(ci); // expected-error {{static_cast from 'const int' lvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(ci);
//  (void) static_cast<int &>(ci);
//  (void) static_cast<int &&>(ci);
    (void) static_cast<int const>(ci); // expected-error {{in static_cast from 'const int' lvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(ci); // expected-error {{static_cast from 'const int' lvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(ci);
    (void) static_cast<int const &>(ci); // expected-error {{static_cast from 'const int' lvalue to 'const int &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const &&>(ci);

    // non-class xvalue, non-const:
    (void) static_cast<int>(nix()); // expected-error {{static_cast from 'int' xvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(nix());
//  (void) static_cast<int &>(nix());
    (void) static_cast<int &&>(nix()); // expected-error {{static_cast from 'int' xvalue to 'int &&' xvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const>(nix()); // expected-error {{in static_cast from 'int' xvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(nix()); // expected-error {{static_cast from 'int' xvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    (void) static_cast<int const &>(nix());
    (void) static_cast<int const &&>(nix()); // expected-error {{static_cast from 'int' xvalue to 'const int &&' xvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<int const &&>(nix());

    // non-class xvalue, const:
    (void) static_cast<int>(cix()); // expected-error {{static_cast from 'const int' xvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(cix());
//  (void) static_cast<int &>(cix());
//  (void) static_cast<int &&>(cix());
    (void) static_cast<int const>(cix()); // expected-error {{in static_cast from 'const int' xvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(cix()); // expected-error {{static_cast from 'const int' xvalue to 'int' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) int(cix());
    (void) static_cast<int const &>(cix());
    (void) static_cast<int const &&>(cix()); // expected-error {{static_cast from 'const int' xvalue to 'const int &&' xvalue is redundant [loplugin:redundantcast]}}

    // non-class prvalue, non-const:
    (void) static_cast<int>(nir()); // expected-error {{static_cast from 'int' prvalue to 'int' prvalue is redundant [loplugin:redundantcast]}}
//  (void) static_cast<int &>(nir());
    (void) static_cast<int &&>(nir());
    (void) static_cast<int const>(nir()); // expected-error {{in static_cast from 'int' prvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(nir()); // expected-error {{static_cast from 'int' prvalue to 'int' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const &>(nir()); // expected-error {{static_cast from 'int' prvalue to 'const int &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const &&>(nir());

    // non-class prvalue, const:
    (void) static_cast<int>(cir()); // expected-error {{static_cast from 'int' prvalue to 'int' prvalue is redundant [loplugin:redundantcast]}}
//  (void) static_cast<int &>(cir());
    (void) static_cast<int &&>(cir());
    (void) static_cast<int const>(cir()); // expected-error {{in static_cast from 'int' prvalue to 'const int' prvalue, remove redundant top-level const qualifier [loplugin:redundantcast]}}
    /* => */ (void) static_cast<int>(cir()); // expected-error {{static_cast from 'int' prvalue to 'int' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const &>(cir()); // expected-error {{static_cast from 'int' prvalue to 'const int &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<int const &&>(cir());

    // class lvalue, non-const:
    S ns{};
    (void) static_cast<S>(ns); // expected-error {{static_cast from 'S' lvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(ns);
    (void) static_cast<S &>(ns); // expected-error {{static_cast from 'S' lvalue to 'S &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<S &&>(ns);
    (void) static_cast<S const>(ns); // expected-error {{static_cast from 'S' lvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ using CS = const S; (void) CS(ns);
    (void) static_cast<S const &>(ns); // expected-error {{static_cast from 'S' lvalue to 'const S &' lvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<S const &>(ns);
    (void) static_cast<S const &&>(ns); // expected-error {{static_cast from 'S' lvalue to 'const S &&' xvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<S const &&>(ns);

    // class lvalue, const:
    S const cs{};
    (void) static_cast<S>(cs); // expected-error {{static_cast from 'const S' lvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(cs);
//  (void) static_cast<S &>(cs);
//  (void) static_cast<S &&>(cs);
    (void) static_cast<S const>(cs); // expected-error {{static_cast from 'const S' lvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) CS(cs);
    (void) static_cast<S const &>(cs); // expected-error {{static_cast from 'const S' lvalue to 'const S &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<S const &&>(cs);

    // class xvalue, non-const:
    (void) static_cast<S>(nsx()); // expected-error {{static_cast from 'S' xvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(nsx());
//  (void) static_cast<S &>(nsx());
    (void) static_cast<S &&>(nsx()); // expected-error {{static_cast from 'S' xvalue to 'S &&' xvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<S const>(nsx()); // expected-error {{static_cast from 'S' xvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) CS(nsx());
    (void) static_cast<S const &>(nsx());
    (void) static_cast<S const &&>(nsx()); // expected-error {{static_cast from 'S' xvalue to 'const S &&' xvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<S const &&>(nsx());

    // class xvalue, const:
    (void) static_cast<S>(csx()); // expected-error {{static_cast from 'const S' xvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(csx());
//  (void) static_cast<S &>(csx());
//  (void) static_cast<S &&>(csx());
    (void) static_cast<S const>(csx()); // expected-error {{static_cast from 'const S' xvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) CS(csx());
    (void) static_cast<S const &>(csx());
    (void) static_cast<S const &&>(csx()); // expected-error {{static_cast from 'const S' xvalue to 'const S &&' xvalue is redundant [loplugin:redundantcast]}}

    // class prvalue, non-const:
    (void) static_cast<S>(nsr()); // expected-error {{static_cast from 'S' prvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(nsr());
//  (void) static_cast<S &>(nsr());
    (void) static_cast<S &&>(nsr());
    (void) static_cast<S const>(nsr()); // expected-error {{static_cast from 'S' prvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) CS(nsr());
    (void) static_cast<S const &>(nsr()); // expected-error {{static_cast from 'S' prvalue to 'const S &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<S const &&>(nsr()); // expected-error {{static_cast from 'S' prvalue to 'const S &&' xvalue should be written as const_cast [loplugin:redundantcast]}}
    /* => */ (void) const_cast<S const &&>(nsr());

    // class prvalue, const:
    (void) static_cast<S>(csr()); // expected-error {{static_cast from 'const S' prvalue to 'S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) S(csr());
//  (void) static_cast<S &>(csr());
//  (void) static_cast<S &&>(csr());
    (void) static_cast<S const>(csr()); // expected-error {{static_cast from 'const S' prvalue to 'const S' prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    /* => */ (void) CS(csr());
    (void) static_cast<S const &>(csr()); // expected-error {{static_cast from 'const S' prvalue to 'const S &' lvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<S const &&>(csr());
}

void testFunctionalCast() {
    (void) int(nir()); // expected-error {{redundant functional cast from 'int' to 'int' [loplugin:redundantcast]}}
    (void) S(nsr());
}

void testCStyleCast() {
    Enum1 e = (Enum1)Enum1::X; // expected-error {{redundant cstyle cast from 'Enum1' to 'Enum1' [loplugin:redundantcast]}}
    (void)e;
}

int main() {
    testConstCast();
    testStaticCast();
    testFunctionalCast();
    testCStyleCast();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
