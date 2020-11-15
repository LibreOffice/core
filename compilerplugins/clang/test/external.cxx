/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vector>

#include "external.hxx"

int n0; // no warning, see external.hxx

// expected-error@+1 {{externally available entity 'n1' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int n1 = 0;
// expected-note@+1 {{another declaration is here [loplugin:external]}}
extern int n1;

int const n2 = 0; // no warning, internal linkage

constexpr int n3 = 0; // no warning, internal linkage

static int n4; // no warning, internal linkage

// expected-note@+1 {{another declaration is here [loplugin:external]}}
extern int n5;
// expected-error@+1 {{externally available entity 'n5' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int n5;

// expected-note@+1 {{another declaration is here [loplugin:external]}}
extern "C" int n6;
// expected-error@+1 {{externally available entity 'n6' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int n6;

extern "C" {
// expected-error@+1 {{externally available entity 'n7' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int n7;
}

namespace
{
int u1; // no warning, internal linkage

static int u2; // no warning, internal linkage

extern "C" int u3;
int u3; // no warning, see the comment about DR1113 in compilerplugins/clang/external.cxx

extern "C" {
int u4; // no warning, internal linkage
}
}

namespace N
{
int v1; // no warning, see external.hxx

// expected-error@+1 {{externally available entity 'v2' is not previously declared in an included file (if it is only used in this translation unit, make it static or put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
int v2;

static int v3; // no warning, internal linkage
}

struct S
{
    static int f()
    {
        static int s = 0;
        return s;
    }

    static int m;
};

int S::m = 0; // no warning

int f(int a) // no warning about parameters
{
    static int s = 0; // no warning about local static variables
    ++s;
    int b = a + s; // no warning about local variables
    return b;
}

// expected-error@+1 {{externally available entity 'S1' is not previously declared in an included file (if it is only used in this translation unit, put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
struct S1
{
    friend void f1() {} // no warning for injected function (no place where to mark it `static`)
    template <typename> friend void ft1() {} // ...nor for injected function template
    // expected-error@+1 {{externally available entity 'f2' is not previously declared in an included file (if it is only used in this translation unit, make it static; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
    friend void f2() {}
};

// expected-error@+1 {{externally available entity 'S2' is not previously declared in an included file (if it is only used in this translation unit, put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
struct S2
{
    friend void f1();
    template <typename> friend void ft1();
    // expected-note@+1 {{another declaration is here [loplugin:external]}}
    friend void f2();
};

static void g()
{
    void f1();
    // expected-note@+1 {{another declaration is here [loplugin:external]}}
    void f2();
}

// expected-note@+1 {{another declaration is here [loplugin:external]}}
void f2();

namespace N
{
inline namespace I1
{
extern "C++" {
// expected-note@+1 {{another declaration is here [loplugin:external]}}
enum E : int;

// expected-error@+1 {{externally available entity 'E' is not previously declared in an included file (if it is only used in this translation unit, put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
enum E : int
{
};
}
}

// expected-note-re@+1 {{a function associating 'N{{(::I1)?}}::E' is declared here [loplugin:external]}}
static void g(std::vector<E>)
{
    // expected-note@+1 {{another declaration is here [loplugin:external]}}
    void f(E const*);
}

// expected-note-re@+1 {{a function associating 'N{{(::I1)?}}::E' is declared here [loplugin:external]}}
void f(E const*);

extern "C++" {
// expected-note-re@+1 {{a function associating 'N{{(::I1)?}}::E' is declared here [loplugin:external]}}
void fc(E const*);
}

// expected-error@+1 {{externally available entity 'S1' is not previously declared in an included file (if it is only used in this translation unit, put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
struct S1
{
    struct S2;
    // No note about associating function; injected friend function not found by ADL:
    friend void f2(E const*);
    // expected-note@+1 {{a function associating 'N::S1' is declared here [loplugin:external]}}
    friend void h(S1);
};

// expected-error@+1 {{externally available entity 'S3' is not previously declared in an included file (if it is only used in this translation unit, put it in an unnamed namespace; otherwise, provide a declaration of it in an included file) [loplugin:external]}}
struct S3
{
    // expected-note@+1 {{another declaration is here [loplugin:external]}}
    friend void h(S1);
};

inline namespace I2
{
// expected-note-re@+1 {{a function associating 'N{{(::I1)?}}::E' is declared here [loplugin:external]}}
void f3(E);

inline namespace I3
{
// expected-note-re@+1 {{a function associating 'N{{(::I1)?}}::E' is declared here [loplugin:external]}}
void f4(E);
}
}
}

struct N::S1::S2
{
    // expected-note@+1 {{another declaration is here [loplugin:external]}}
    friend void f(E const*);
};

int main()
{
    (void)n2;
    (void)n3;
    (void)n4;
    (void)u1;
    (void)u2;
    (void)N::v3;
    g();
    (void)&N::g;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
