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
};
}

struct UnoObject : public cppu::OWeakObject
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
