/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/io/XStreamListener.hpp"

void test1(const css::uno::Reference<css::io::XStreamListener>& a)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    css::uno::Reference<css::uno::XInterface> b(a, css::uno::UNO_QUERY);
}

namespace test2
{
css::uno::Reference<css::io::XStreamListener> getListener();

void test()
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    css::uno::Reference<css::uno::XInterface> b(getListener(), css::uno::UNO_QUERY);
}
}

namespace test3
{
void callListener(css::uno::Reference<css::uno::XInterface> const&);

void test(css::uno::Reference<css::io::XStreamListener> const& l)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    callListener(css::uno::Reference<css::uno::XInterface>(l, css::uno::UNO_QUERY));
}
}

void test4(const css::uno::Reference<css::io::XStreamListener>& a)
{
    // no warning expected, used to reject null references
    css::uno::Reference<css::uno::XInterface> b(a, css::uno::UNO_SET_THROW);
}

// no warning expected
namespace test5
{
void test(css::uno::Reference<css::io::XStreamListener> l)
{
    css::uno::Reference<css::uno::XInterface> a = l;
}
}

namespace test6
{
void test(css::uno::Reference<css::io::XStreamListener> l)
{
    css::uno::Reference<css::uno::XInterface> a;
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l, css::uno::UNO_QUERY);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
