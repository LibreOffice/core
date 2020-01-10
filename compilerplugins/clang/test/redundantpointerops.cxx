/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <tools/ref.hxx>

struct Struct1 {
    int x;
};

void function1(Struct1& s)
{
    (&s)->x = 1; // expected-error {{'&' followed by '->' operating on 'Struct1', rather use '.' [loplugin:redundantpointerops]}}
};

struct Struct2 {
    int x;
    Struct2* operator&() { return this; }
};

void function2(Struct2 s)
{
    (&s)->x = 1; // expected-error {{'&' followed by '->' operating on 'Struct2', rather use '.' [loplugin:redundantpointerops]}}
};

void function3(Struct1& s)
{
    (*(&s)).x = 1; // expected-error {{'&' followed by '*' operating on 'Struct1', rather use '.' [loplugin:redundantpointerops]}}
};

//void function4(Struct1* s)
//{
//    (*s).x = 1; // xxexpected-error {{'*' followed by '.', rather use '->' [loplugin:redundantpointerops]}}
//};

int function5(std::unique_ptr<int> x)
{
    return *x.get(); // expected-error-re {{'*' followed by '.get()' operating on '{{.*}}unique_ptr{{.*}}', just use '*' [loplugin:redundantpointerops]}}
};

void function6(std::shared_ptr<int> x)
{
    (void) *x.get(); // expected-error-re {{'*' followed by '.get()' operating on '{{.*}}shared_ptr{{.*}}', just use '*' [loplugin:redundantpointerops]}}
}

void function6b(std::shared_ptr<Struct1> x)
{
    x.get()->x = 1; // expected-error-re {{'get()' followed by '->' operating on '{{.*}}shared_ptr{{.*}}', just use '->' [loplugin:redundantpointerops]}}
}

void function7(rtl::Reference<css::uno::XInterface> x)
{
    (void) *x.get(); // expected-error {{'*' followed by '.get()' operating on 'rtl::Reference<css::uno::XInterface>', just use '*' [loplugin:redundantpointerops]}}
}

void function8(css::uno::Reference<css::uno::XInterface> x)
{
    (void) *x.get(); // expected-error {{'*' followed by '.get()' operating on 'css::uno::Reference<css::uno::XInterface>', just use '*' [loplugin:redundantpointerops]}}
}

void function9(tools::SvRef<SvRefBase> x)
{
    (void) *x.get(); // expected-error {{'*' followed by '.get()' operating on 'tools::SvRef<SvRefBase>', just use '*' [loplugin:redundantpointerops]}}
}

struct DerivedRtlReference: public rtl::Reference<css::uno::XInterface> {};

void function10(DerivedRtlReference x)
{
    (void) *x.get(); // expected-error {{'*' followed by '.get()' operating on 'DerivedRtlReference', just use '*' [loplugin:redundantpointerops]}}
}

struct DerivedUnoReference: public css::uno::Reference<css::uno::XInterface> {};

void function11(DerivedUnoReference x)
{
    (void) *x.get(); // expected-error {{'*' followed by '.get()' operating on 'DerivedUnoReference', just use '*' [loplugin:redundantpointerops]}}
}

// tools::SvRef is final

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
