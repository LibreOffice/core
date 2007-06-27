/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_thread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:24:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
#include "precompiled_sal.hxx"

#include "sal/config.h"

#include "cppunit/simpleheader.hxx"
#include "osl/conditn.hxx"
#include "osl/thread.hxx"
#include "osl/time.h"
#include "sal/types.h"

namespace {

osl::Condition global;

class Thread: public osl::Thread {
public:
    explicit Thread(osl::Condition & cond): m_cond(cond) {}

private:
    virtual void SAL_CALL run() {}

    virtual void SAL_CALL onTerminated() {
        m_cond.set();
        CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, global.wait());
    }

    osl::Condition & m_cond;
};

class Test: public CppUnit::TestFixture {
public:
    // Nondeterministic, best effort test that an osl::Thread can be destroyed
    // (and in particular osl_destroyThread---indirectly---be called) before the
    // corresponding thread has terminated:
    void test() {
        for (int i = 0; i < 50; ++i) {
            osl::Condition c;
            Thread t(c);
            CPPUNIT_ASSERT(t.create());
            // Make sure virtual Thread::run/onTerminated are called before
            // Thread::~Thread:
            CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, c.wait());
        }
        // Make sure Thread::~Thread is called before each spawned thread
        // terminates:
        global.set();
        // Give the spawned threads enough time to terminate:
        TimeValue const twentySeconds = { 20, 0 };
        osl::Thread::wait(twentySeconds);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
