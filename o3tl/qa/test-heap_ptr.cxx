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



#include "preextstl.h"
#include "gtest/gtest.h"
#include "postextstl.h"

#include <o3tl/heap_ptr.hxx>




using o3tl::heap_ptr;


class Help
{
  public:
    explicit            Help(
                            int                 i_n )
                                                : n(i_n) { ++nInstanceCount_; }
                        ~Help()                 { --nInstanceCount_; }
    int                 Value() const           { return n; }
    static int          InstanceCount_()        { return nInstanceCount_; }

  private:
    int                 n;
    static int          nInstanceCount_;
};
int Help::nInstanceCount_ = 0;


class heap_ptr_test : public ::testing::Test
{
  public:
}; // class heap_ptr_test

TEST_F(heap_ptr_test, global)
{
    // Construction
    heap_ptr<Help>
        t_empty;
    const heap_ptr<Help>
        t0( new Help(7000) );
    heap_ptr<Help>
        t1( new Help(10) );
    heap_ptr<Help>
        t2( new Help(20) );

    int nHelpCount = 3;

    ASSERT_TRUE(! t_empty.is()) << "ctor1";
    ASSERT_TRUE(t0.is()) << "ctor2";
    ASSERT_TRUE((*t0).Value() == 7000) << "ctor3";
    ASSERT_TRUE(t0->Value() == 7000) << "ctor4";
    ASSERT_TRUE(t0.get() == t0.operator->()) << "ctor5";
    ASSERT_TRUE(t0.get() == &(*t0)) << "ctor6";

    ASSERT_TRUE(t1.is()) << "ctor7";
    ASSERT_TRUE((*t1).Value() == 10) << "ctor8";
    ASSERT_TRUE(t1->Value() == 10) << "ctor9";
    ASSERT_TRUE(t1.get() == t1.operator->()) << "ctor10";
    ASSERT_TRUE(t1.get() == &(*t1)) << "ctor11";

    ASSERT_TRUE(t2.operator*().Value() == 20) << "ctor12";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "ctor13";


    // Operator safe_bool() / bool()
    ASSERT_TRUE(! t_empty) << "bool1";
    ASSERT_TRUE(t0) << "bool2";
    ASSERT_TRUE(t1.is() == static_cast<bool>(t1)) << "bool3";


    // Assignment, reset() and release()
        // Release
    Help * hp = t1.release();
    ASSERT_TRUE(! t1.is()) << "release1";
    ASSERT_TRUE(t1.get() == 0) << "release2";
    ASSERT_TRUE(t1.operator->() == 0) << "release3";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "release4";

        // operator=()
    t_empty = hp;
    ASSERT_TRUE(t_empty.is()) << "assign1";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "assign2";

    t1 = t_empty.release();
    ASSERT_TRUE(t1.is()) << "assign3";
    ASSERT_TRUE(! t_empty.is()) << "assign4";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "assign5";

        // reset()
    hp = new Help(30);
    ++nHelpCount;

    t_empty.reset(hp);
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "reset1";
    ASSERT_TRUE(t_empty.is()) << "reset2";
    ASSERT_TRUE(t_empty.get() == hp) << "reset3";

        // Ignore second assignment
    t_empty = hp;
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "selfassign1";
    ASSERT_TRUE(t_empty.is()) << "selfassign2";
    ASSERT_TRUE(t_empty.get() == hp) << "selfassign3";

    t_empty.reset(0);
    hp = 0;
    --nHelpCount;
    ASSERT_TRUE(! t_empty.is()) << "reset4";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "reset5";


    // swap
    t1.swap(t2);
    ASSERT_TRUE(t1->Value() == 20) << "swap1";
    ASSERT_TRUE(t2->Value() == 10) << "swap2";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "swap3";

    o3tl::swap(t1,t2);
    ASSERT_TRUE(t1->Value() == 10) << "swap4";
    ASSERT_TRUE(t2->Value() == 20) << "swap5";
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "swap6";

    // RAII
    {
        heap_ptr<Help>
            t_raii( new Help(55) );
        ASSERT_TRUE(Help::InstanceCount_() == nHelpCount + 1) << "raii1";
    }
    ASSERT_TRUE(Help::InstanceCount_() == nHelpCount) << "raii2";
}


