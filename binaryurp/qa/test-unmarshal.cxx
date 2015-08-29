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



#include "sal/config.h"

#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppu/unotype.hxx"
#include "gtest/gtest.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "sal/types.h"
#include "typelib/typedescription.hxx"

#include "../source/bridge.hxx"
#include "../source/cache.hxx"
#include "../source/readerstate.hxx"
#include "../source/unmarshal.hxx"

namespace {

namespace css = com::sun::star;

class Test: public ::testing::Test {
public:
};

TEST_F(Test, testTypeOfBooleanSequence) {
    binaryurp::ReaderState state;
    css::uno::Sequence< sal_Int8 > buf(13);
    buf[0] = static_cast< sal_Int8 >(20 | 0x80); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(binaryurp::cache::ignore >> 8);
    buf[2] = static_cast< sal_Int8 >(binaryurp::cache::ignore & 0xFF);
    buf[3] = RTL_CONSTASCII_LENGTH("[]boolean");
    buf[4] = '[';
    buf[5] = ']';
    buf[6] = 'b';
    buf[7] = 'o';
    buf[8] = 'o';
    buf[9] = 'l';
    buf[10] = 'e';
    buf[11] = 'a';
    buf[12] = 'n';
    binaryurp::Unmarshal m(rtl::Reference< binaryurp::Bridge >(), state, buf);
    css::uno::TypeDescription t(m.readType());
    ASSERT_TRUE(
        t.equals(
            css::uno::TypeDescription(
                cppu::UnoType< css::uno::Sequence< bool > >::get())));
    m.done();
}

TEST_F(Test, testTypeOfVoidSequence) {
    binaryurp::ReaderState state;
    css::uno::Sequence< sal_Int8 > buf(10);
    buf[0] = static_cast< sal_Int8 >(20 | 0x80); // sequence type | cache flag
    buf[1] = static_cast< sal_Int8 >(binaryurp::cache::ignore >> 8);
    buf[2] = static_cast< sal_Int8 >(binaryurp::cache::ignore & 0xFF);
    buf[3] = RTL_CONSTASCII_LENGTH("[]void");
    buf[4] = '[';
    buf[5] = ']';
    buf[6] = 'v';
    buf[7] = 'o';
    buf[8] = 'i';
    buf[9] = 'd';
    binaryurp::Unmarshal m(rtl::Reference< binaryurp::Bridge >(), state, buf);
    try {
        m.readType();
        FAIL() << "exception expected";
    } catch (css::io::IOException &) {}
}


}
