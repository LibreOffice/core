/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_weakbag.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 13:18:36 $
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

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/weakbag.hxx"
#include "cppuhelper/weak.hxx"
#include "cppunit/simpleheader.hxx"

namespace {

namespace css = com::sun::star;

class Test: public CppUnit::TestFixture {
public:
    void test() {
        css::uno::Reference< css::uno::XInterface > ref1(new cppu::OWeakObject);
        css::uno::Reference< css::uno::XInterface > ref2(new cppu::OWeakObject);
        css::uno::Reference< css::uno::XInterface > ref3(new cppu::OWeakObject);
        comphelper::WeakBag< css::uno::XInterface > bag;
        bag.add(ref1);
        bag.add(ref1);
        bag.add(ref2);
        bag.add(ref2);
        ref1.clear();
        bag.add(ref3);
        ref3.clear();
        CPPUNIT_ASSERT_MESSAGE("remove first ref2", bag.remove() == ref2);
        CPPUNIT_ASSERT_MESSAGE("remove second ref2", bag.remove() == ref2);
        CPPUNIT_ASSERT_MESSAGE("remove first null", !bag.remove().is());
        CPPUNIT_ASSERT_MESSAGE("remove second null", !bag.remove().is());
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
