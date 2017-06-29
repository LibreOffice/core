/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <com/sun/star/uno/XInterface.hpp>

struct Foo
{
    std::unique_ptr<css::uno::XInterface> m_foo1; // expected-error {{XInterface subclass 'com::sun::star::uno::XInterface' being managed via smart pointer, should be managed via uno::Reference, parent is 'Foo' [loplugin:refcounting]}}
    std::shared_ptr<css::uno::XInterface> m_foo2; // expected-error {{XInterface subclass 'com::sun::star::uno::XInterface' being managed via smart pointer, should be managed via uno::Reference, parent is 'Foo' [loplugin:refcounting]}}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
