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
#include "precompiled_cppu.hxx"

#include "sal/config.h"

#include <stdlib.h> // wntmsci10 does not like <cstdlib>

#include "Enum1.hpp"
#include "Enum2.hpp"
#include "Exception1.hpp"
#include "Exception2.hpp"
#include "Exception2a.hpp"
#include "Exception2b.hpp"
#include "Interface1.hpp"
#include "Interface2.hpp"
#include "Interface2a.hpp"
#include "Interface2b.hpp"
#include "Interface3.hpp"
#include "Poly.hpp"
#include "Struct1.hpp"
#include "Struct2.hpp"
#include "Struct2a.hpp"
#include "Struct2b.hpp"
#include "boost/type_traits/is_same.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "gtest/gtest.h"

namespace {

namespace css = com::sun::star;

class Base {
public:
    Base(): m_count(0) {}

    void acquire() {
        if (osl_incrementInterlockedCount(&m_count) == SAL_MAX_INT32) {
            abort();
        }
    }

    void release() {
        if (osl_decrementInterlockedCount(&m_count) == 0) {
            delete this;
        }
    }

protected:
    virtual ~Base() {}

private:
    Base(Base &); // not declared
    void operator =(Base &); // not declared

    oslInterlockedCount m_count;
};

class Impl1: public Interface1, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException)
    {
        if (type
            == getCppuType< css::uno::Reference< css::uno::XInterface > >())
        {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(this));
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface1 > >()) {
            css::uno::Reference< Interface1 > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () {
        Base::release();
    }
};

class Impl2: public Interface2a, public Interface3, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException)
    {
        if (type
            == getCppuType< css::uno::Reference< css::uno::XInterface > >())
        {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(
                    static_cast< Interface2a * >(this)));
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface2 > >()) {
            css::uno::Reference< Interface2 > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface2a > >())
        {
            css::uno::Reference< Interface2a > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface3 > >()) {
            css::uno::Reference< Interface3 > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () {
        Base::release();
    }
};

class Impl2b: public Interface2b, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException)
    {
        if (type
            == getCppuType< css::uno::Reference< css::uno::XInterface > >())
        {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(
                    static_cast< Interface2a * >(this)));
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface2 > >()) {
            css::uno::Reference< Interface2 > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface2a > >())
        {
            css::uno::Reference< Interface2a > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == getCppuType< css::uno::Reference< Interface2b > >())
        {
            css::uno::Reference< Interface2b > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () {
        Base::release();
    }
};

class Test: public ::testing::Test {
public:
};

TEST_F(Test, testVoid) {
    css::uno::Any a;
    ASSERT_TRUE(a.getValueType() == getVoidCppuType());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(
            !(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) << "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(
            !(a >>= b) && b == getCppuType< rtl::OUString >()) << "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) << "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testBoolean) {
    css::uno::Any a(false);
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Bool >());
    {
        bool b = true;
        ASSERT_TRUE((a >>= b) && !b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE((a >>= b) && !b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        if (boost::is_same< sal_uInt8, sal_Bool >::value) {
            ASSERT_TRUE((a >>= b) && b == 0) << "@sal_uInt8";
        } else {
            ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
        }
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testByte) {
    css::uno::Any a(static_cast< sal_Int8 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Int8 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            ASSERT_TRUE((a >>= b) && b == 1) << "@sal_Unicode";
        } else {
            ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
        }
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testShort) {
    css::uno::Any a(static_cast< sal_Int16 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Int16 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            ASSERT_TRUE((a >>= b) && b == 1) << "@sal_Unicode";
        } else {
            ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
        }
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testUnsignedShort) {
    sal_uInt16 n = 1;
    css::uno::Any a(&n, getCppuType(static_cast< sal_uInt16 const * >(0)));
    ASSERT_TRUE(
        a.getValueType() == getCppuType(static_cast< sal_uInt16 const * >(0)));
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            ASSERT_TRUE((a >>= b) && b == 1) << "@sal_Unicode";
        } else {
            ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
        }
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testLong) {
    css::uno::Any a(static_cast< sal_Int32 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Int32 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testUnsignedLong) {
    css::uno::Any a(static_cast< sal_uInt32 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_uInt32 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testHyper) {
    css::uno::Any a(static_cast< sal_Int64 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Int64 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testUnsignedHyper) {
    css::uno::Any a(static_cast< sal_uInt64 >(1));
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_uInt64 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testFloat) {
    css::uno::Any a(1.f);
    ASSERT_TRUE(a.getValueType() == getCppuType< float >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testDouble) {
    css::uno::Any a(1.);
    ASSERT_TRUE(a.getValueType() == getCppuType< double >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE((a >>= b) && b == 1) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testChar) {
    sal_Unicode c = '1';
    css::uno::Any a(&c, getCppuType< sal_Unicode >());
    ASSERT_TRUE(a.getValueType() == getCppuType< sal_Unicode >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            ASSERT_TRUE(!(a >>= b) && b == '2') << "@sal_Unicode";
        } else {
            ASSERT_TRUE((a >>= b) && b == '1') << "sal_Unicode";
        }
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testString) {
    css::uno::Any a(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1")));
    ASSERT_TRUE(a.getValueType() == getCppuType< rtl::OUString >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE((a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("1"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testType) {
    css::uno::Any a(getCppuType< sal_Int32 >());
    ASSERT_TRUE(a.getValueType() == getCppuType< css::uno::Type >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE((a >>= b) && b == getCppuType< sal_Int32 >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testSequence) {
    sal_Int32 n = 1;
    css::uno::Any a(css::uno::Sequence< sal_Int32 >(&n, 1));
    ASSERT_TRUE(
        a.getValueType() == getCppuType< css::uno::Sequence< sal_Int32 > >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        css::uno::Sequence< sal_Int32 > b(2);
        ASSERT_TRUE((a >>= b) && b.getLength() == 1 && b[0] == 1) <<
            "css::uno::Sequence<sal_Int32>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testEnum) {
    css::uno::Any a(Enum2_M1);
    ASSERT_TRUE(a.getValueType() == getCppuType< Enum2 >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Enum2 b = Enum2_M2;
        ASSERT_TRUE((a >>= b) && b == Enum2_M1) << "Enum2";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testStruct) {
    css::uno::Any a(Struct2a(1, 3));
    ASSERT_TRUE(a.getValueType() == getCppuType< Struct2a >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Struct2 b(2);
        ASSERT_TRUE((a >>= b) && b.member == 1) << "Struct2";
    }
    {
        Struct2a b(2, 2);
        ASSERT_TRUE((a >>= b) && b.member == 1 && b.member2 == 3) <<
            "Struct2a";
    }
    {
        Struct2b b(2, 2, 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct2b";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testPoly) {
    css::uno::Any a;
    a <<= Poly< css::uno::Sequence< ::sal_Unicode > >();
    ASSERT_TRUE(a.getValueType().getTypeName().equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("Poly<[]char>"))) <<
        "type name";
    ASSERT_TRUE(a == css::uno::Any(Poly< css::uno::Sequence< ::sal_Unicode > >())) <<
        "constructor";
}

TEST_F(Test, testException) {
    css::uno::Any a(
        Exception2a(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 1,
            3));
    ASSERT_TRUE(a.getValueType() == getCppuType< Exception2a >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        Exception2 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE((a >>= b) && b.member == 1) << "Exception2";
    }
    {
        Exception2a b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2,
            2);
        ASSERT_TRUE((a >>= b) && b.member == 1 && b.member2 == 3) <<
            "Exception2a";
    }
    {
        Exception2b b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2,
            2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception2b";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
}

TEST_F(Test, testInterface) {
    css::uno::Reference< Interface2a > i2(new Impl2);
    css::uno::Any a(i2);
    ASSERT_TRUE(
        a.getValueType()
        == getCppuType< css::uno::Reference< Interface2a > >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface1";
    }
    {
        css::uno::Reference< Interface2 > b(new Impl2);
        ASSERT_TRUE((a >>= b) && b == i2) << "Interface2";
    }
    {
        css::uno::Reference< Interface2a > b(new Impl2);
        ASSERT_TRUE((a >>= b) && b == i2) << "Interface2a";
    }
    {
        css::uno::Reference< Interface2b > i(new Impl2b);
        css::uno::Reference< Interface2b > b(i);
        ASSERT_TRUE(!(a >>= b) && b == i) << "Interface2b";
    }
    {
        css::uno::Reference< Interface3 > b(new Impl2);
        ASSERT_TRUE((a >>= b) && b == i2) << "Interface3";
    }
}

TEST_F(Test, testNull) {
    css::uno::Any a = css::uno::Any(css::uno::Reference< Interface2a >());
    ASSERT_TRUE(
        a.getValueType()
        == getCppuType< css::uno::Reference< Interface2a > >());
    {
        bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "bool";
    }
    {
        sal_Bool b = true;
        ASSERT_TRUE(!(a >>= b) && b) << "sal_Bool";
    }
    {
        sal_Int8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int8";
    }
    {
        sal_uInt8 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt8";
    }
    {
        sal_Int16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int16";
    }
    {
        sal_uInt16 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt16";
    }
    {
        sal_Int32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int32";
    }
    {
        sal_uInt32 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt32";
    }
    {
        sal_Int64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_Int64";
    }
    {
        sal_uInt64 b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "sal_uInt64";
    }
    {
        float b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "float";
    }
    {
        double b = 2;
        ASSERT_TRUE(!(a >>= b) && b == 2) << "double";
    }
    {
        sal_Unicode b = '2';
        ASSERT_TRUE(!(a >>= b) && b == '2') << "sal_Unicode";
    }
    {
        rtl::OUString b(RTL_CONSTASCII_USTRINGPARAM("2"));
        ASSERT_TRUE(!(a >>= b) && b.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("2"))) <<
            "rtl::OUString";
    }
    {
        css::uno::Type b(getCppuType< rtl::OUString >());
        ASSERT_TRUE(!(a >>= b) && b == getCppuType< rtl::OUString >()) <<
            "css::uno::Type";
    }
    {
        css::uno::Any b(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")));
        ASSERT_TRUE((a >>= b) && b == a) << "css::uno::Any";
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        ASSERT_TRUE(!(a >>= b) && b.getLength() == 2) <<
            "css::uno::Sequence<rtl::OUString>";
    }
    {
        Enum1 b = Enum1_M2;
        ASSERT_TRUE(!(a >>= b) && b == Enum1_M2) << "Enum1";
    }
    {
        Struct1 b(2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Struct1";
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        ASSERT_TRUE(!(a >>= b) && b.member == 2) << "Exception1";
    }
    {
        css::uno::Reference< Interface1 > b(new Impl1);
        ASSERT_TRUE((a >>= b) && !b.is()) <<
            "Interface1";
    }
    {
        css::uno::Reference< Interface2 > b(new Impl2);
        ASSERT_TRUE((a >>= b) && !b.is()) <<
            "Interface2";
    }
    {
        css::uno::Reference< Interface2a > b(new Impl2);
        ASSERT_TRUE((a >>= b) && !b.is()) << "Interface2a";
    }
    {
        css::uno::Reference< Interface2b > b(new Impl2b);
        ASSERT_TRUE((a >>= b) && !b.is()) <<
            "Interface2b";
    }
    {
        css::uno::Reference< Interface3 > b(new Impl2);
        ASSERT_TRUE((a >>= b) && !b.is()) <<
            "Interface3";
    }
}

}
