/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "sal/config.h"

#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XAdapter.hpp"
#include "com/sun/star/uno/XReference.hpp"
#include "com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

namespace {

namespace css = com::sun::star;

class Reference: public cppu::WeakImplHelper1< css::uno::XReference > {
public:
    Reference(): m_disposed(false) {}

    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException) {
        m_disposed = true;
        handleDispose();
    }

    bool isDisposed() const { return m_disposed; }

protected:
    virtual void handleDispose() {};

private:
    bool m_disposed;
};

class RuntimeExceptionReference: public Reference {
protected:
    virtual void handleDispose() {
        throw css::uno::RuntimeException();
    }
};

class DisposedExceptionReference: public Reference {
protected:
    virtual void handleDispose() {
        throw css::lang::DisposedException();
    }
};

class Test: public ::CppUnit::TestFixture {
public:
    void testReferenceDispose();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testReferenceDispose);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testReferenceDispose() {
    css::uno::Reference< css::uno::XWeak > w(new ::cppu::OWeakObject);
    css::uno::Reference< css::uno::XAdapter > a(w->queryAdapter());
    ::rtl::Reference< Reference > r1(new RuntimeExceptionReference);
    ::rtl::Reference< Reference > r2(new Reference);
    ::rtl::Reference< Reference > r3(new DisposedExceptionReference);
    a->addReference(r1.get());
    a->addReference(r2.get());
    a->addReference(r3.get());
    w.clear();
    CPPUNIT_ASSERT(r1->isDisposed());
    CPPUNIT_ASSERT(r2->isDisposed());
    CPPUNIT_ASSERT(r3->isDisposed());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
