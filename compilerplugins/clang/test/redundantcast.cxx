/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstddef>

#include <sal/types.h>

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

template<typename T>
struct EnumItemInterface {
    T GetValue() { return static_cast<T>(0); }
};
class Enum1Item : public EnumItemInterface<Enum1> {
};
bool testCStyleCastOfTemplateMethodResult(Enum1Item* item) {
    return (Enum1)item->GetValue() == Enum1::X; // expected-error {{redundant cstyle cast from 'Enum1' to 'Enum1' [loplugin:redundantcast]}}
}

using T1 = int;
T1 nt1r() { return 0; }
void testArithmeticTypedefs() {
    (void) static_cast<T1>(nir());
    (void) T1(nir());
    (void) (T1) nir();
    (void) static_cast<int>(nt1r());
    (void) int(nt1r());
    (void) (int) nt1r();
    using T2 = T1;
    (void) static_cast<T2>(nt1r());
    (void) T2(nt1r());
    (void) (T2) nt1r();
    (void) static_cast<T1>(nt1r()); // expected-error {{redundant}}
    (void) T1(nt1r()); // expected-error {{redundant}}
    (void) (T1) nt1r(); // expected-error {{redundant}}

    T1 const c{};
    (void) static_cast<T1>(c); // expected-error {{redundant}}
}

void testReinterpretConstCast() {
    int n = 0;
    (void) reinterpret_cast<std::size_t>((const_cast<int const *>(&n))); // expected-error-re {{redundant const_cast from 'int *' to 'const int *' within reinterpret_cast to fundamental type 'std::size_t' (aka 'unsigned {{.+}}') [loplugin:redundantcast]}}
}

void testDynamicCast() {

    struct S1 { virtual ~S1(); };
    struct S2 final: S1 {};
    struct S3: S1 {};

    S1 * s1 = nullptr;
    S2 * s2 = nullptr;
    S3 * s3 = nullptr;

    (void) dynamic_cast<S2 *>(s1);
    (void) dynamic_cast<S1 *>(s2); // expected-error {{redundant dynamic upcast from 'S2 *' to 'S1 *' [loplugin:redundantcast]}}
    (void) dynamic_cast<S2 *>(s2); // expected-error {{redundant dynamic cast from 'S2 *' to 'S2 *' [loplugin:redundantcast]}}
    (void) dynamic_cast<S3 *>(s2);
    (void) dynamic_cast<const S2 *>(s2); // expected-error {{redundant dynamic cast from 'S2 *' to 'const S2 *' [loplugin:redundantcast]}}
    (void) dynamic_cast<S1 *>(s3); // expected-error {{redundant dynamic upcast from 'S3 *' to 'S1 *' [loplugin:redundantcast]}}
}

void overload(int);
void overload(long);
void nonOverload();

struct Overload {
    int overload();
    long overload() const;
    void nonOverload();
};

void testOverloadResolution() {
    (void) static_cast<void (*)(long)>(overload);
    (void) static_cast<void (*)(long)>((overload));
    (void) static_cast<void (*)(long)>(&overload);
    (void) static_cast<void (*)(long)>((&overload));
    (void) static_cast<void (*)(long)>(&((overload)));
    (void) static_cast<void (*)()>(nonOverload); // expected-error {{static_cast from 'void (*)()' prvalue to 'void (*)()' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<void (*)()>((nonOverload)); // expected-error {{static_cast from 'void (*)()' prvalue to 'void (*)()' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<void (*)()>(&nonOverload); // expected-error {{static_cast from 'void (*)()' prvalue to 'void (*)()' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<void (*)()>((&nonOverload)); // expected-error {{static_cast from 'void (*)()' prvalue to 'void (*)()' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<void (*)()>(&((nonOverload))); // expected-error {{static_cast from 'void (*)()' prvalue to 'void (*)()' prvalue is redundant [loplugin:redundantcast]}}
    (void) static_cast<long (Overload::*)() const>(&Overload::overload);
    (void) static_cast<void (Overload::*)()>(&Overload::nonOverload); // expected-error {{static_cast from 'void (Overload::*)()' prvalue to 'void (Overload::*)()' prvalue is redundant [loplugin:redundantcast]}}

    using OverloadFn = void (*)(long);
    (void) OverloadFn(overload);
    using NonOverloadFn = void (*)();
    (void) NonOverloadFn(nonOverload); // expected-error {{redundant functional cast from 'void (*)()' to 'NonOverloadFn' (aka 'void (*)()') [loplugin:redundantcast]}}
    using OverloadMemFn = long (Overload::*)() const;
    (void) OverloadMemFn(&Overload::overload);
    using NonOverloadMemFn = void (Overload::*)();
    (void) NonOverloadMemFn(&Overload::nonOverload); // expected-error {{redundant functional cast from 'void (Overload::*)()' to 'NonOverloadMemFn' (aka 'void (Overload::*)()') [loplugin:redundantcast]}}
};

void testIntermediaryStaticCast() {
    int n = 0;
    n = static_cast<double>(n); // expected-error {{suspicious static_cast from 'int' to 'double', result is implicitly cast to 'int' [loplugin:redundantcast]}}
    n = double(n); // expected-error {{suspicious functional cast from 'int' to 'double', result is implicitly cast to 'int' [loplugin:redundantcast]}}
    double d = 0.0;
    d = static_cast<int>(d) + 1.0; // expected-error {{suspicious static_cast from 'double' to 'int', result is implicitly cast to 'double' [loplugin:redundantcast]}}
    d = int(d) + 1.0; // expected-error {{suspicious functional cast from 'double' to 'int', result is implicitly cast to 'double' [loplugin:redundantcast]}}
};

void testArrayDecay() {
    (void) static_cast<char const *>(""); // expected-error {{redundant static_cast from 'const char [1]' to 'const char *' [loplugin:redundantcast]}}
    (void) reinterpret_cast<char const *>(""); // expected-error {{redundant reinterpret_cast from 'const char [1]' to 'const char *' [loplugin:redundantcast]}}
    (void) reinterpret_cast<char const *>(u8"");
}

void testNew() {
    class A {};
    class B : public A {};
    A* p = static_cast<A*>(new B); // expected-error {{redundant static_cast from 'B *' to 'A *' [loplugin:redundantcast]}}
    (void)p;
    // no warning expected for resolving-ambiguity cast
    class C : public A {};
    class D : public B, public C {};
    p = static_cast<B*>(new D);
    // no warning expected for down-cast
    auto p2 = static_cast<B*>(p);
    (void)p2;
}

using F = void (*)();
auto testNullFunctionPointer(int i, F p) {
    switch (i) {
    case 0:
        return static_cast<F>(nullptr);
    case 1:
        return F(nullptr);
    default:
        return p;
    }
}

void testSalIntTypes() {
    sal_Int16 const n = 0;
    (void) static_cast<sal_Int16>(n); // expected-error-re {{static_cast from 'const sal_Int16' (aka 'const {{.+}}') lvalue to 'sal_Int16' (aka '{{.+}}') prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    (void) static_cast<::sal_Int16>(n); // expected-error-re {{static_cast from 'const sal_Int16' (aka 'const {{.+}}') lvalue to '::sal_Int16' (aka '{{.+}}') prvalue is redundant or should be written as an explicit construction of a temporary [loplugin:redundantcast]}}
    (void) static_cast<short>(n); // doesn't warn, even if 'sal_Int16' is 'short'
    using Other = sal_Int16;
    (void) static_cast<Other>(n); // doesn't warn either
}

int main() {
    testConstCast();
    testStaticCast();
    testFunctionalCast();
    testCStyleCast();
    testCStyleCastOfTemplateMethodResult(nullptr);
    testReinterpretConstCast();
    testDynamicCast();
    testIntermediaryStaticCast();
    testArrayDecay();
    testSalIntTypes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
