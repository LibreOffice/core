/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include "sal/config.h"

#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XAdapter.hpp"
#include "com/sun/star/uno/XReference.hpp"
#include "com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "testshl/simpleheader.hxx"
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

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
