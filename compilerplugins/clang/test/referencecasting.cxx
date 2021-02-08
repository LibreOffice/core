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
#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "cppuhelper/weak.hxx"

void test1(const css::uno::Reference<css::io::XStreamListener>& a)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    css::uno::Reference<css::lang::XEventListener> b(a, css::uno::UNO_QUERY);
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    auto c = css::uno::Reference<css::lang::XEventListener>::query(a);
}

namespace test2
{
css::uno::Reference<css::io::XStreamListener> getListener();

void test()
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    css::uno::Reference<css::lang::XEventListener> b(getListener(), css::uno::UNO_QUERY);
}
}

namespace test3
{
void callListener(css::uno::Reference<css::uno::XInterface> const&);

void test(css::uno::Reference<css::io::XStreamListener> const& l)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    callListener(css::uno::Reference<css::lang::XEventListener>(l, css::uno::UNO_QUERY));
}
}

void test4(const css::uno::Reference<css::io::XStreamListener>& a)
{
    // no warning expected, used to reject null references
    css::uno::Reference<css::lang::XEventListener> b(a, css::uno::UNO_SET_THROW);
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
    css::uno::Reference<css::lang::XEventListener> a;
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l, css::uno::UNO_QUERY);
}
}

namespace test7
{
void test(css::uno::Reference<css::io::XStreamListener> l)
{
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    css::uno::Reference<css::lang::XEventListener> a(l.get(), css::uno::UNO_QUERY);
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    a.set(l.get(), css::uno::UNO_QUERY);
}
}

namespace test8
{
void test(css::io::XStreamListener* l)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    css::uno::Reference<css::lang::XEventListener> a(l, css::uno::UNO_QUERY);
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l, css::uno::UNO_QUERY);
}
}

// check for looking through casts
namespace test9
{
class StatusbarController : public css::io::XStreamListener, public ::cppu::OWeakObject
{
};

void test(StatusbarController* pController)
{
    css::uno::Reference<css::io::XStreamListener> xController;
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    xController.set(static_cast<::cppu::OWeakObject*>(pController), css::uno::UNO_QUERY);
}
}

// no warning expected when we have an ambiguous base
namespace test10
{
class Foo : public css::lang::XTypeProvider, public css::lang::XComponent
{
    virtual ~Foo();
    void bar()
    {
        css::uno::Reference<css::lang::XEventListener> xSource(
            static_cast<css::lang::XTypeProvider*>(this), css::uno::UNO_QUERY);
    }
};
}

// no warning expected for SAL_NO_ACQUIRE
namespace test11
{
void test(css::io::XStreamListener* l)
{
    css::uno::Reference<css::lang::XEventListener> a(l, SAL_NO_ACQUIRE);
    a.set(l, SAL_NO_ACQUIRE);
}
}

// no warning expected: querying for XInterface (instead of doing an upcast) has special semantics,
// to check for UNO object equivalence.
void test12(const css::uno::Reference<css::io::XStreamListener>& a)
{
    css::uno::Reference<css::uno::XInterface> b(a, css::uno::UNO_QUERY);
}

// no warning expected: querying for XInterface (instead of doing an upcast) has special semantics,
// to check for UNO object equivalence.
struct Test13
{
    css::uno::Reference<css::uno::XInterface> m_xNormalizedIFace;
    void newObject(const css::uno::Reference<css::uno::XInterface>& _rxIFace)
    {
        m_xNormalizedIFace.set(_rxIFace, css::uno::UNO_QUERY);
    }
};

void test14(css::uno::Sequence<css::uno::Reference<css::io::XStreamListener>> seq)
{
    for (sal_Int32 i = 0; i < seq.getLength(); ++i)
    {
        // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
        css::uno::Reference<css::io::XStreamListener> xDataSeries(seq[i], css::uno::UNO_QUERY);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
