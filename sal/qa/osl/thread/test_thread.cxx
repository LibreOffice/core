/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "osl/conditn.hxx"
#include "osl/thread.hxx"
#include "osl/time.h"

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
    
    
    
    void test() {
        for (int i = 0; i < 50; ++i) {
            osl::Condition c;
            Thread t(c);
            CPPUNIT_ASSERT(t.create());
            
            
            CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, c.wait());
        }
        
        
        global.set();
        
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
