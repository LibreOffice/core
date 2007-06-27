/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_weak.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:21:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
#include "cppunit/simpleheader.hxx"
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
