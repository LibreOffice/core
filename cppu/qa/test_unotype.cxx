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

#include <typeinfo>

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "gtest/gtest.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace {

namespace css = com::sun::star;

struct DerivedStruct1: css::lang::EventObject {};

struct DerivedStruct2: css::beans::PropertyChangeEvent {};

struct DerivedException1: css::uno::Exception {};

struct DerivedException2: css::uno::RuntimeException {};

struct DerivedInterface1: css::uno::XInterface {};

struct DerivedInterface2: css::uno::XComponentContext {};

class Test: public ::testing::Test {
public:
};

TEST_F(Test, testUnoType) {
    css::uno::Type t;
    t = ::cppu::UnoType< ::cppu::UnoVoidType >::get();
    ASSERT_EQ(css::uno::TypeClass_VOID, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")), t.getTypeName());
    t = ::cppu::UnoType< bool >::get();
    ASSERT_EQ(css::uno::TypeClass_BOOLEAN, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("boolean")),
        t.getTypeName());
    ASSERT_TRUE(::cppu::UnoType< ::sal_Bool >::get() == t);
    t = ::cppu::UnoType< ::sal_Int8 >::get();
    ASSERT_EQ(css::uno::TypeClass_BYTE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("byte")), t.getTypeName());
    t = ::cppu::UnoType< ::sal_Int16 >::get();
    ASSERT_EQ(css::uno::TypeClass_SHORT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("short")), t.getTypeName());
    t = ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get();
    ASSERT_EQ(css::uno::TypeClass_UNSIGNED_SHORT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unsigned short")),
        t.getTypeName());
    t = ::cppu::UnoType< ::sal_Int32 >::get();
    ASSERT_EQ(css::uno::TypeClass_LONG, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("long")), t.getTypeName());
    t = ::cppu::UnoType< ::sal_uInt32 >::get();
    ASSERT_EQ(css::uno::TypeClass_UNSIGNED_LONG, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unsigned long")),
        t.getTypeName());
    t = ::cppu::UnoType< ::sal_Int64 >::get();
    ASSERT_EQ(css::uno::TypeClass_HYPER, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("hyper")), t.getTypeName());
    t = ::cppu::UnoType< ::sal_uInt64 >::get();
    ASSERT_EQ(css::uno::TypeClass_UNSIGNED_HYPER, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unsigned hyper")),
        t.getTypeName());
    t = ::cppu::UnoType< float >::get();
    ASSERT_EQ(css::uno::TypeClass_FLOAT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("float")), t.getTypeName());
    t = ::cppu::UnoType< double >::get();
    ASSERT_EQ(css::uno::TypeClass_DOUBLE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("double")),
        t.getTypeName());
    t = ::cppu::UnoType< ::cppu::UnoCharType >::get();
    ASSERT_EQ(css::uno::TypeClass_CHAR, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("char")), t.getTypeName());
    t = ::cppu::UnoType< ::rtl::OUString >::get();
    ASSERT_EQ(css::uno::TypeClass_STRING, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("string")),
        t.getTypeName());
    t = ::cppu::UnoType< css::uno::Type >::get();
    ASSERT_EQ(css::uno::TypeClass_TYPE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("type")), t.getTypeName());
    t = ::cppu::UnoType< css::uno::Any >::get();
    ASSERT_EQ(css::uno::TypeClass_ANY, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any")), t.getTypeName());
    t = ::cppu::UnoType< ::cppu::UnoSequenceType< ::sal_Int8 > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]byte")),
        t.getTypeName());
    ASSERT_TRUE(
        ::cppu::UnoType< css::uno::Sequence< ::sal_Int8 > >::get() == t);
    t = ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]unsigned short")),
        t.getTypeName());
    t = ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]char")),
        t.getTypeName());
    t = ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::sal_Int8 > > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[][]byte")),
        t.getTypeName());
    ASSERT_TRUE(
        ::cppu::UnoType<
        css::uno::Sequence< css::uno::Sequence< ::sal_Int8 > > >::get() == t);
    t = ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoUnsignedShortType > > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[][]unsigned short")),
        t.getTypeName());
    t = ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoCharType > > >::get();
    ASSERT_EQ(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[][]char")),
        t.getTypeName());
    t = ::cppu::UnoType< css::uno::TypeClass >::get();
    ASSERT_EQ(css::uno::TypeClass_ENUM, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.TypeClass")),
        t.getTypeName());
    t = ::cppu::UnoType< css::lang::EventObject >::get();
    ASSERT_EQ(css::uno::TypeClass_STRUCT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.EventObject")),
        t.getTypeName());
    ASSERT_TRUE(::cppu::UnoType< DerivedStruct1 >::get() == t);
    t = ::cppu::UnoType< css::beans::PropertyChangeEvent >::get();
    ASSERT_EQ(css::uno::TypeClass_STRUCT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.beans.PropertyChangeEvent")),
        t.getTypeName());
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(::cppu::UnoType< DerivedStruct2 >::get() == t);
#endif
    t = ::cppu::UnoType< css::beans::Optional< ::sal_Int8 > >::get();
    ASSERT_EQ(css::uno::TypeClass_STRUCT, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.Optional<byte>")),
        t.getTypeName());
    t = ::cppu::UnoType< css::uno::Exception >::get();
    ASSERT_EQ(css::uno::TypeClass_EXCEPTION, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception")),
        t.getTypeName());
    ASSERT_TRUE(::cppu::UnoType< DerivedException1 >::get() == t);
    t = ::cppu::UnoType< css::uno::RuntimeException >::get();
    ASSERT_EQ(css::uno::TypeClass_EXCEPTION, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.RuntimeException")),
        t.getTypeName());
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(::cppu::UnoType< DerivedException2 >::get() == t);
#endif
    t = ::cppu::UnoType< css::uno::XInterface >::get();
    ASSERT_EQ(css::uno::TypeClass_INTERFACE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface")),
        t.getTypeName());
    ASSERT_TRUE(
        ::cppu::UnoType< css::uno::Reference< css::uno::XInterface > >::get() ==
        t);
    ASSERT_TRUE(::cppu::UnoType< DerivedInterface1 >::get() == t);
    ASSERT_TRUE(
        ::cppu::UnoType< css::uno::Reference< DerivedInterface1 > >::get() ==
        t);
    t = ::cppu::UnoType< css::uno::XComponentContext >::get();
    ASSERT_EQ(css::uno::TypeClass_INTERFACE, t.getTypeClass());
    ASSERT_EQ(
        ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XComponentContext")),
        t.getTypeName());
    ASSERT_TRUE(
        ::cppu::UnoType<
        css::uno::Reference< css::uno::XComponentContext > >::get() == t);
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(::cppu::UnoType< DerivedInterface2 >::get() == t);
    ASSERT_TRUE(
        ::cppu::UnoType< css::uno::Reference< DerivedInterface2 > >::get() ==
        t);
#endif
}

TEST_F(Test, testGetTypeFavourUnsigned) {
    ASSERT_TRUE(typeid(::sal_Unicode) == typeid(::sal_uInt16));
    ASSERT_TRUE(
        ::getCppuType(static_cast< ::sal_Unicode * >(0)) ==
        ::getCppuType(static_cast< ::sal_uInt16 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< ::cppu::UnoVoidType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoVoidType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< bool * >(0)) ==
        ::cppu::UnoType< bool >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< bool * >(0)) ==
        ::getCppuType(static_cast< bool * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Bool * >(0)) ==
        ::cppu::UnoType< bool >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Bool * >(0)) ==
        ::getCppuType(static_cast< ::sal_Bool * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int8 * >(0)) ==
        ::cppu::UnoType< ::sal_Int8 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int8 * >(0)) ==
        ::getCppuType(static_cast< ::sal_Int8 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int16 * >(0)) ==
        ::cppu::UnoType< ::sal_Int16 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int16 * >(0)) ==
        ::getCppuType(static_cast< ::sal_Int16 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< ::cppu::UnoUnsignedShortType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt16 * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt16 * >(0)) ==
        ::getCppuType(static_cast< ::sal_uInt16 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int32 * >(0)) ==
        ::cppu::UnoType< ::sal_Int32 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int32 * >(0)) ==
        ::getCppuType(static_cast< ::sal_Int32 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt32 * >(0)) ==
        ::cppu::UnoType< ::sal_uInt32 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt32 * >(0)) ==
        ::getCppuType(static_cast< ::sal_uInt32 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int64 * >(0)) ==
        ::cppu::UnoType< ::sal_Int64 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Int64 * >(0)) ==
        ::getCppuType(static_cast< ::sal_Int64 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt64 * >(0)) ==
        ::cppu::UnoType< ::sal_uInt64 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_uInt64 * >(0)) ==
        ::getCppuType(static_cast< ::sal_uInt64 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< float * >(0)) ==
        ::cppu::UnoType< float >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< float * >(0)) ==
        ::getCppuType(static_cast< float * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< double * >(0)) ==
        ::cppu::UnoType< double >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< double * >(0)) ==
        ::getCppuType(static_cast< double * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< ::cppu::UnoCharType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoCharType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Unicode * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::sal_Unicode * >(0)) ==
        ::getCppuType(static_cast< ::sal_Unicode * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::rtl::OUString * >(0)) ==
        ::cppu::UnoType< ::rtl::OUString >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< ::rtl::OUString * >(0)) ==
        ::getCppuType(static_cast< ::rtl::OUString * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< css::uno::Type * >(0)) ==
        ::cppu::UnoType< css::uno::Type >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< css::uno::Type * >(0)) ==
        ::getCppuType(static_cast< css::uno::Type * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< css::uno::Any * >(0)) ==
        ::cppu::UnoType< css::uno::Any >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< css::uno::Any * >(0)) ==
        ::getCppuType(static_cast< css::uno::Any * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast<
            ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< ::sal_uInt16 > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< ::sal_uInt16 > * >(0)) ==
        ::getCppuType(static_cast< css::uno::Sequence< ::sal_uInt16 > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
            ::cppu::UnoUnsignedShortType > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoUnsignedShortType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_uInt16 > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoUnsignedShortType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_uInt16 > > * >(0)) ==
        ::getCppuType(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_uInt16 > > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< ::sal_Unicode > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< ::sal_Unicode > * >(0)) ==
        ::getCppuType(static_cast< css::uno::Sequence< ::sal_Unicode > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_Unicode > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoUnsignedShortType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_Unicode > > * >(0)) ==
        ::getCppuType(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_Unicode > > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::TypeClass * >(0)) ==
        ::cppu::UnoType< css::uno::TypeClass >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::TypeClass * >(0)) ==
        ::getCppuType(static_cast< css::uno::TypeClass * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::lang::EventObject * >(0)) ==
        ::cppu::UnoType< css::lang::EventObject >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::lang::EventObject * >(0)) ==
        ::getCppuType(static_cast< css::lang::EventObject * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedStruct1 * >(0)) ==
        ::cppu::UnoType< css::lang::EventObject >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedStruct1 * >(0)) ==
        ::getCppuType(static_cast< DerivedStruct1 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::beans::PropertyChangeEvent * >(0)) ==
        ::cppu::UnoType< css::beans::PropertyChangeEvent >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::beans::PropertyChangeEvent * >(0)) ==
        ::getCppuType(static_cast< css::beans::PropertyChangeEvent * >(0)));
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedStruct2 * >(0)) ==
        ::cppu::UnoType< css::beans::PropertyChangeEvent >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedStruct2 * >(0)) ==
        ::getCppuType(static_cast< DerivedStruct2 * >(0)));
#endif
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::beans::Optional< ::sal_Int8 > * >(0)) ==
        ::cppu::UnoType< css::beans::Optional< ::sal_Int8 > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::beans::Optional< ::sal_Int8 > * >(0)) ==
        ::getCppuType(static_cast< css::beans::Optional< ::sal_Int8 > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Exception * >(0)) ==
        ::cppu::UnoType< css::uno::Exception >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Exception * >(0)) ==
        ::getCppuType(static_cast< css::uno::Exception * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedException1 * >(0)) ==
        ::cppu::UnoType< css::uno::Exception >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedException1 * >(0)) ==
        ::getCppuType(static_cast< DerivedException1 * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::RuntimeException * >(0)) ==
        ::cppu::UnoType< css::uno::RuntimeException >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::RuntimeException * >(0)) ==
        ::getCppuType(static_cast< css::uno::RuntimeException * >(0)));
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedException2 * >(0)) ==
        ::cppu::UnoType< css::uno::RuntimeException >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedException2 * >(0)) ==
        ::getCppuType(static_cast< DerivedException2 * >(0)));
#endif
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::XInterface * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Reference< css::uno::XInterface > * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Reference< css::uno::XInterface > * >(0)) ==
        ::getCppuType(
            static_cast< css::uno::Reference< css::uno::XInterface > * >(0)));
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedInterface1 * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Reference< DerivedInterface1 > * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::XComponentContext * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast<
            css::uno::Reference< css::uno::XComponentContext > * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast<
            css::uno::Reference< css::uno::XComponentContext > * >(0)) ==
        ::getCppuType(
            static_cast<
            css::uno::Reference< css::uno::XComponentContext > * >(0)));
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(static_cast< DerivedInterface2 * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourUnsigned(
            static_cast< css::uno::Reference< DerivedInterface2 > * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
#endif
}

TEST_F(Test, testGetTypeFavourChar) {
    ASSERT_TRUE(typeid(::sal_Unicode) == typeid(::sal_uInt16));
    ASSERT_TRUE(
        ::getCppuType< ::sal_Unicode >() == ::getCppuType< ::sal_uInt16 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::cppu::UnoVoidType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoVoidType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< bool * >(0)) ==
        ::cppu::UnoType< bool >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< bool * >(0)) ==
        ::getCppuType< bool >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Bool * >(0)) ==
        ::cppu::UnoType< bool >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Bool * >(0)) ==
        ::getCppuType< ::sal_Bool >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int8 * >(0)) ==
        ::cppu::UnoType< ::sal_Int8 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int8 * >(0)) ==
        ::getCppuType< ::sal_Int8 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int16 * >(0)) ==
        ::cppu::UnoType< ::sal_Int16 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int16 * >(0)) ==
        ::getCppuType< ::sal_Int16 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< ::cppu::UnoUnsignedShortType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_uInt16 * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoCharType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int32 * >(0)) ==
        ::cppu::UnoType< ::sal_Int32 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int32 * >(0)) ==
        ::getCppuType< ::sal_Int32 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_uInt32 * >(0)) ==
        ::cppu::UnoType< ::sal_uInt32 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_uInt32 * >(0)) ==
        ::getCppuType< ::sal_uInt32 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int64 * >(0)) ==
        ::cppu::UnoType< ::sal_Int64 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Int64 * >(0)) ==
        ::getCppuType< ::sal_Int64 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_uInt64 * >(0)) ==
        ::cppu::UnoType< ::sal_uInt64 >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_uInt64 * >(0)) ==
        ::getCppuType< ::sal_uInt64 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< float * >(0)) ==
        ::cppu::UnoType< float >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< float * >(0)) ==
        ::getCppuType< float >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< double * >(0)) ==
        ::cppu::UnoType< double >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< double * >(0)) ==
        ::getCppuType< double >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::cppu::UnoCharType * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoCharType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Unicode * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoCharType >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::sal_Unicode * >(0)) ==
        ::getCppuType< ::sal_Unicode >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::rtl::OUString * >(0)) ==
        ::cppu::UnoType< ::rtl::OUString >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< ::rtl::OUString * >(0)) ==
        ::getCppuType< ::rtl::OUString >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Type * >(0)) ==
        ::cppu::UnoType< css::uno::Type >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Type * >(0)) ==
        ::getCppuType< css::uno::Type >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Any * >(0)) ==
        ::cppu::UnoType< css::uno::Any >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Any * >(0)) ==
        ::getCppuType< css::uno::Any >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast<
            ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoUnsignedShortType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Sequence< ::sal_uInt16 > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
            ::cppu::UnoUnsignedShortType > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoUnsignedShortType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_uInt16 > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoCharType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Sequence< ::sal_Unicode > * >(0)) ==
        ::cppu::UnoType<
        ::cppu::UnoSequenceType< ::cppu::UnoCharType > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Sequence< css::uno::Sequence<
            ::sal_Unicode > > * >(0)) ==
        ::cppu::UnoType< ::cppu::UnoSequenceType< ::cppu::UnoSequenceType<
        ::cppu::UnoCharType > > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::TypeClass * >(0)) ==
        ::cppu::UnoType< css::uno::TypeClass >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::TypeClass * >(0)) ==
        ::getCppuType< css::uno::TypeClass >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::lang::EventObject * >(0)) ==
        ::cppu::UnoType< css::lang::EventObject >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::lang::EventObject * >(0)) ==
        ::getCppuType< css::lang::EventObject >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedStruct1 * >(0)) ==
        ::cppu::UnoType< css::lang::EventObject >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedStruct1 * >(0)) ==
        ::getCppuType< DerivedStruct1 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::beans::PropertyChangeEvent * >(0)) ==
        ::cppu::UnoType< css::beans::PropertyChangeEvent >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::beans::PropertyChangeEvent * >(0)) ==
        ::getCppuType< css::beans::PropertyChangeEvent >());
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedStruct2 * >(0)) ==
        ::cppu::UnoType< css::beans::PropertyChangeEvent >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedStruct2 * >(0)) ==
        ::getCppuType< DerivedStruct2 >());
#endif
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::beans::Optional< ::sal_Int8 > * >(0)) ==
        ::cppu::UnoType< css::beans::Optional< ::sal_Int8 > >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::beans::Optional< ::sal_Int8 > * >(0)) ==
        ::getCppuType< css::beans::Optional< ::sal_Int8 > >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Exception * >(0)) ==
        ::cppu::UnoType< css::uno::Exception >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< css::uno::Exception * >(0)) ==
        ::getCppuType< css::uno::Exception >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedException1 * >(0)) ==
        ::cppu::UnoType< css::uno::Exception >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedException1 * >(0)) ==
        ::getCppuType< DerivedException1 >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::RuntimeException * >(0)) ==
        ::cppu::UnoType< css::uno::RuntimeException >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::RuntimeException * >(0)) ==
        ::getCppuType< css::uno::RuntimeException >());
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedException2 * >(0)) ==
        ::cppu::UnoType< css::uno::RuntimeException >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedException2 * >(0)) ==
        ::getCppuType< DerivedException2 >());
#endif
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::XInterface * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Reference< css::uno::XInterface > * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Reference< css::uno::XInterface > * >(0)) ==
        ::getCppuType< css::uno::Reference< css::uno::XInterface > >());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedInterface1 * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Reference< DerivedInterface1 > * >(0)) ==
        ::cppu::UnoType< css::uno::XInterface >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::XComponentContext * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast<
            css::uno::Reference< css::uno::XComponentContext > * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast<
            css::uno::Reference< css::uno::XComponentContext > * >(0)) ==
        ::getCppuType< css::uno::Reference< css::uno::XComponentContext > >());
#if !(defined __SUNPRO_CC && __SUNPRO_CC <= 0x550) // erroneous ambiguity stated
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(static_cast< DerivedInterface2 * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
    ASSERT_TRUE(
        ::cppu::getTypeFavourChar(
            static_cast< css::uno::Reference< DerivedInterface2 > * >(0)) ==
        ::cppu::UnoType< css::uno::XComponentContext >::get());
#endif
}


}
