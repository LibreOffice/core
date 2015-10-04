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



#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "comphelper/weakbag.hxx"
#include "cppuhelper/weak.hxx"
#include "gtest/gtest.h"

namespace {

namespace css = com::sun::star;

class Test: public ::testing::Test {
public:
};

TEST_F(Test, test) {
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
    ASSERT_TRUE(bag.remove() == ref2) << "remove first ref2";
    ASSERT_TRUE(bag.remove() == ref2) << "remove second ref2";
    ASSERT_TRUE(!bag.remove().is()) << "remove first null";
    ASSERT_TRUE(!bag.remove().is()) << "remove second null";
}


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
