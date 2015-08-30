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
#include "precompiled_sal.hxx"

#include "sal/config.h"

#include "osl/conditn.hxx"
#include "osl/thread.hxx"
#include "osl/time.h"
#include "sal/types.h"
#include "gtest/gtest.h"

namespace {

osl::Condition global;

class Thread: public osl::Thread {
public:
    explicit Thread(osl::Condition & cond): m_cond(cond) {}

private:
    virtual void SAL_CALL run() {}

    virtual void SAL_CALL onTerminated() {
        m_cond.set();
        ASSERT_EQ(osl::Condition::result_ok, global.wait());
    }

    osl::Condition & m_cond;
};

class Test: public ::testing::Test {
public:
};

// Nondeterministic, best effort test that an osl::Thread can be destroyed
// (and in particular osl_destroyThread---indirectly---be called) before the
// corresponding thread has terminated:
TEST_F(Test, test) {
    for (int i = 0; i < 50; ++i) {
        osl::Condition c;
        Thread t(c);
        ASSERT_TRUE(t.create());
        // Make sure virtual Thread::run/onTerminated are called before
        // Thread::~Thread:
        ASSERT_EQ(osl::Condition::result_ok, c.wait());
    }
    // Make sure Thread::~Thread is called before each spawned thread
    // terminates:
    global.set();
    // Give the spawned threads enough time to terminate:
    TimeValue const twentySeconds = { 20, 0 };
    osl::Thread::wait(twentySeconds);
}


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

