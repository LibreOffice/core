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
#include <rtl/ref.hxx>
#include <boost/intrusive_ptr.hpp>
#include <com/sun/star/uno/XInterface.hpp>

namespace cppu
{
class OWeakObject
{
public:
    void acquire();
    void release();
};
}

struct UnoObject : public cppu::OWeakObject
{
};
struct UnoSubObject : public UnoObject
{
};

//
// Note, getting duplicate warnings for some reason I cannot fathom
//

struct Foo
{
    // expected-error@+2 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    std::unique_ptr<UnoObject> m_foo1;
    // expected-error@+2 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    std::shared_ptr<UnoObject> m_foo2;
    // expected-error@+2 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    boost::intrusive_ptr<UnoObject> m_foo3;
    rtl::Reference<UnoObject> m_foo4; // no warning expected
};

// expected-error@+2 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
// expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
std::unique_ptr<UnoObject> foo1();
rtl::Reference<UnoObject> foo2(); // no warning expected
// expected-error@+2 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
// expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via smart pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
void foo3(std::unique_ptr<UnoObject> p);

void test2(UnoObject* pUnoObject)
{
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being deleted via delete, should be managed via rtl::Reference [loplugin:refcounting]}}
    delete pUnoObject;
}

template <typename T> struct Dependent : T
{
    void f() { delete this; }
    //TODO: missing expected error@+1 {{cppu::OWeakObject subclass 'Dependent<UnoObject>' being deleted via delete, should be managed via rtl::Reference [loplugin:refcounting]}}
    void g() { delete this; }
};
struct Dummy
{
};
void dummy(Dependent<Dummy>* p1, Dependent<UnoObject>* p2)
{
    p1->f();
    p2->g();
}

void foo4()
{
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via raw pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    UnoObject* p = new UnoObject;
    (void)p;
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoObject' being managed via raw pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    p = new UnoObject;
}

UnoObject* foo5()
{
    // expected-error@+1 {{new object of cppu::OWeakObject subclass 'UnoObject' being returned via raw pointer, should be returned by via rtl::Reference [loplugin:refcounting]}}
    return new UnoObject;
}
rtl::Reference<UnoObject> foo6()
{
    // no warning expected
    return new UnoObject;
}
const rtl::Reference<UnoObject>& getConstRef();
void foo7()
{
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoSubObject' being managed via raw pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    UnoSubObject* p1 = static_cast<UnoSubObject*>(foo6().get());
    (void)p1;
    // expected-error@+1 {{cppu::OWeakObject subclass 'UnoSubObject' being managed via raw pointer, should be managed via rtl::Reference [loplugin:refcounting]}}
    p1 = static_cast<UnoSubObject*>(foo6().get());

    rtl::Reference<UnoObject> u2;
    // no warning expected
    UnoSubObject* p2 = static_cast<UnoSubObject*>(u2.get());
    (void)p2;
    p2 = static_cast<UnoSubObject*>(u2.get());
    // no warning expected
    UnoSubObject* p3 = static_cast<UnoSubObject*>(getConstRef().get());
    (void)p3;
    p3 = static_cast<UnoSubObject*>(getConstRef().get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
