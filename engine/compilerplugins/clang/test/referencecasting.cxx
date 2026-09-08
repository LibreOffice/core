/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "cpo/uno/Sequence.hxx"
#include "cpo/uno/XInterface.hpp"
#include "com/sun/star/io/XStreamListener.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "cppuhelper/implbase.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ref.hxx"

void test1(const cpo::uno::Reference<css::io::XStreamListener>& a)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    cpo::uno::Reference<css::lang::XEventListener> b(a, cpo::uno::UNO_QUERY);
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    auto c = cpo::uno::Reference<css::lang::XEventListener>::query(a);
}

namespace test2
{
cpo::uno::Reference<css::io::XStreamListener> getListener();

void test()
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    cpo::uno::Reference<css::lang::XEventListener> b(getListener(), cpo::uno::UNO_QUERY);
}
}

namespace test3
{
void callListener(cpo::uno::Reference<cpo::uno::XInterface> const&);

void test(cpo::uno::Reference<css::io::XStreamListener> const& l)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    callListener(cpo::uno::Reference<css::lang::XEventListener>(l, cpo::uno::UNO_QUERY));
}
}

void test4(const cpo::uno::Reference<css::io::XStreamListener>& a)
{
    // no warning expected, used to reject null references
    cpo::uno::Reference<css::lang::XEventListener> b(a, cpo::uno::UNO_SET_THROW);
}

// no warning expected
namespace test5
{
void test(cpo::uno::Reference<css::io::XStreamListener> l)
{
    cpo::uno::Reference<cpo::uno::XInterface> a = l;
}
}

namespace test6
{
void test(cpo::uno::Reference<css::io::XStreamListener> l)
{
    cpo::uno::Reference<css::lang::XEventListener> a;
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l, cpo::uno::UNO_QUERY);
}
}

namespace test7
{
void test(cpo::uno::Reference<css::io::XStreamListener> l)
{
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    cpo::uno::Reference<css::lang::XEventListener> a(l.get(), cpo::uno::UNO_QUERY);
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    a.set(l.get(), cpo::uno::UNO_QUERY);
}

class FooStream : public css::io::XStreamListener
{
    virtual ~FooStream();
};
void test(rtl::Reference<FooStream> l)
{
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    cpo::uno::Reference<css::io::XStreamListener> a(l.get());
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l.get(), cpo::uno::UNO_QUERY);
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    a.set(l.get());
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    cpo::uno::Reference<css::io::XStreamListener> b(l.get(), cpo::uno::UNO_QUERY);
    // no warning expected
    cpo::uno::Reference<css::lang::XTypeProvider> c(l.get(), cpo::uno::UNO_QUERY);
    // no warning expected
    cpo::uno::Reference<css::io::XStreamListener> a2 = l;
    (void)a2;
}
cpo::uno::Sequence<cpo::uno::Reference<css::io::XStreamListener>> getContinuations()
{
    rtl::Reference<FooStream> noel1;
    // expected-error@+1 {{unnecessary get() call [loplugin:referencecasting]}}
    return { noel1.get() };
}
}

namespace test8
{
void test(css::io::XStreamListener* l)
{
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    cpo::uno::Reference<css::lang::XEventListener> a(l, cpo::uno::UNO_QUERY);
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    a.set(l, cpo::uno::UNO_QUERY);
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
    cpo::uno::Reference<css::io::XStreamListener> xController;
    // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
    xController.set(static_cast<::cppu::OWeakObject*>(pController), cpo::uno::UNO_QUERY);
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
        cpo::uno::Reference<css::lang::XEventListener> xSource(
            static_cast<css::lang::XTypeProvider*>(this), cpo::uno::UNO_QUERY);
    }
};
}

// no warning expected for SAL_NO_ACQUIRE
namespace test11
{
void test(css::io::XStreamListener* l)
{
    cpo::uno::Reference<css::lang::XEventListener> a(l, SAL_NO_ACQUIRE);
    a.set(l, SAL_NO_ACQUIRE);
}
}

// no warning expected: querying for XInterface (instead of doing an upcast) has special semantics,
// to check for UNO object equivalence.
void test12(const cpo::uno::Reference<css::io::XStreamListener>& a)
{
    cpo::uno::Reference<cpo::uno::XInterface> b(a, cpo::uno::UNO_QUERY);
}

// no warning expected: querying for XInterface (instead of doing an upcast) has special semantics,
// to check for UNO object equivalence.
struct Test13
{
    cpo::uno::Reference<cpo::uno::XInterface> m_xNormalizedIFace;
    void newObject(const cpo::uno::Reference<cpo::uno::XInterface>& _rxIFace)
    {
        m_xNormalizedIFace.set(_rxIFace, cpo::uno::UNO_QUERY);
    }
};

void test14(cpo::uno::Sequence<cpo::uno::Reference<css::io::XStreamListener>> seq)
{
    for (sal_Int32 i = 0; i < seq.getLength(); ++i)
    {
        // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
        cpo::uno::Reference<css::io::XStreamListener> xDataSeries(seq[i], cpo::uno::UNO_QUERY);
    }
}

namespace test15
{
class Foo : public cppu::WeakImplHelper<css::lang::XComponent, css::io::XInputStream>
{
    virtual ~Foo();
    cpo::uno::Reference<css::lang::XTypeProvider> bar()
    {
        // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
        return cpo::uno::Reference<css::lang::XTypeProvider>(
            static_cast<css::lang::XTypeProvider*>(this), cpo::uno::UNO_QUERY);
    }
    cpo::uno::Reference<css::io::XInputStream> bar2()
    {
        // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
        return cpo::uno::Reference<css::io::XInputStream>(static_cast<css::io::XInputStream*>(this),
                                                          cpo::uno::UNO_QUERY);
    }
    cpo::uno::Reference<css::io::XInputStream> bar3()
    {
        // expected-error@+1 {{the source reference is already a subtype of the destination reference, just use = [loplugin:referencecasting]}}
        return cpo::uno::Reference<css::io::XInputStream>(*this, cpo::uno::UNO_QUERY);
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
