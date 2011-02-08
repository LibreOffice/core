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
#include "precompiled_sal.hxx"

#include "sal/config.h"

#include "preextstl.h"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "postextstl.h"
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

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
