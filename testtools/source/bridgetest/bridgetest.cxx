/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdio.h>
#include <string.h>
#include <osl/diagnose.h>
#include "osl/diagnose.hxx"
#include <osl/time.h>
#include <sal/types.h>
#include "typelib/typedescription.hxx"
#include <uno/dispatcher.hxx>
#include <uno/lbnames.h>
#include "uno/mapping.hxx"
#include <uno/data.h>
#include "uno/environment.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/bridge/UnoUrlResolver.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Type.hxx"

#include "test/testtools/bridgetest/BadConstructorArguments.hpp"
#include "test/testtools/bridgetest/TestPolyStruct.hpp"
#include "test/testtools/bridgetest/XBridgeTest.hpp"
#include "test/testtools/bridgetest/XBridgeTest2.hpp"
#include "test/testtools/bridgetest/XMulti.hpp"

#include "currentcontextchecker.hxx"
#include "multi.hxx"

using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::bridge;
using namespace test::testtools::bridgetest;


#define SERVICENAME     "com.sun.star.test.bridge.BridgeTest"
#define IMPLNAME        "com.sun.star.comp.bridge.BridgeTest"

#define STRING_TEST_CONSTANT "\" paco\' chorizo\\\' \"\'"

namespace bridge_test
{
template<typename T, typename U = T>
Sequence<T> cloneSequence(const Sequence<T>& val);


inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( SERVICENAME );
    return Sequence< OUString >( &aName, 1 );
}

static bool check( bool b , char const * message )
{
    if ( ! b )
        fprintf( stderr, "%s failed\n" , message );
    return b;
}

namespace {

bool checkEmpty(OUString const & string, char const * message) {
    bool ok = string.isEmpty();
    if (!ok) {
        fprintf(
            stderr, "%s failed: %s\n", message,
            OUStringToOString(string, RTL_TEXTENCODING_UTF8).getStr());
    }
    return ok;
}

}


class TestBridgeImpl : public osl::DebugBase<TestBridgeImpl>,
                       public WeakImplHelper< XMain, XServiceInfo >
{
    Reference< XComponentContext > m_xContext;

public:
    explicit TestBridgeImpl( const Reference< XComponentContext > & xContext )
        : m_xContext( xContext )
        {}
    virtual ~TestBridgeImpl()
    {
    }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException, std::exception) override;

    // XMain
    virtual sal_Int32 SAL_CALL run( const Sequence< OUString > & rArgs ) throw (RuntimeException, std::exception) override;
};


static bool equals( const TestElement & rData1, const TestElement & rData2 )
{
    check( rData1.Bool == rData2.Bool, "### bool does not match!" );
    check( rData1.Char == rData2.Char, "### char does not match!" );
    check( rData1.Byte == rData2.Byte, "### byte does not match!" );
    check( rData1.Short == rData2.Short, "### short does not match!" );
    check( rData1.UShort == rData2.UShort, "### unsigned short does not match!" );
    check( rData1.Long == rData2.Long, "### long does not match!" );
    check( rData1.ULong == rData2.ULong, "### unsigned long does not match!" );
    check( rData1.Hyper == rData2.Hyper, "### hyper does not match!" );
    check( rData1.UHyper == rData2.UHyper, "### unsigned hyper does not match!" );
    check( rData1.Float == rData2.Float, "### float does not match!" );
    check( rData1.Double == rData2.Double, "### double does not match!" );
    check( rData1.Enum == rData2.Enum, "### enum does not match!" );
    check( rData1.String == rData2.String, "### string does not match!" );
    check( rData1.Interface == rData2.Interface, "### interface does not match!" );
    check( rData1.Any == rData2.Any, "### any does not match!" );

    return (rData1.Bool == rData2.Bool &&
            rData1.Char == rData2.Char &&
            rData1.Byte == rData2.Byte &&
            rData1.Short == rData2.Short &&
            rData1.UShort == rData2.UShort &&
            rData1.Long == rData2.Long &&
            rData1.ULong == rData2.ULong &&
            rData1.Hyper == rData2.Hyper &&
            rData1.UHyper == rData2.UHyper &&
            rData1.Float == rData2.Float &&
            rData1.Double == rData2.Double &&
            rData1.Enum == rData2.Enum &&
            rData1.String == rData2.String &&
            rData1.Interface == rData2.Interface &&
            rData1.Any == rData2.Any);
}

static bool equals( const TestData & rData1, const TestData & rData2 )
{
    sal_Int32 nLen;

    if ((rData1.Sequence == rData2.Sequence) &&
        equals( (const TestElement &)rData1, (const TestElement &)rData2 ) &&
        (nLen = rData1.Sequence.getLength()) == rData2.Sequence.getLength())
    {
        // once again by hand sequence ==
        const TestElement * pElements1 = rData1.Sequence.getConstArray();
        const TestElement * pElements2 = rData2.Sequence.getConstArray();
        for ( ; nLen--; )
        {
            if (! equals( pElements1[nLen], pElements2[nLen] ))
            {
                check( false, "### sequence element did not match!" );
                return false;
            }
        }
        return true;
    }
    return false;
}

static void assign( TestElement & rData,
                    bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const OUString& rStr,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                    const ::com::sun::star::uno::Any& rAny )
{
    rData.Bool = bBool;
    rData.Char = cChar;
    rData.Byte = nByte;
    rData.Short = nShort;
    rData.UShort = nUShort;
    rData.Long = nLong;
    rData.ULong = nULong;
    rData.Hyper = nHyper;
    rData.UHyper = nUHyper;
    rData.Float = fFloat;
    rData.Double = fDouble;
    rData.Enum = eEnum;
    rData.String = rStr;
    rData.Interface = xTest;
    rData.Any = rAny;
}

namespace {

template < typename T >
bool testAny(
    T const & value, Reference< XBridgeTest > const & xLBT,
    char const * typeName = 0)
{
    Any any;
    any <<=  value;
    Any any2 = xLBT->transportAny(any);
    bool success = true;
    if (any != any2) {
        fprintf(
            stderr, "any is different after roundtrip: in %s, out %s\n",
            OUStringToOString(
                any.getValueType().getTypeName(),
                RTL_TEXTENCODING_ASCII_US).getStr(),
            OUStringToOString(
                any2.getValueType().getTypeName(),
                RTL_TEXTENCODING_ASCII_US).getStr());
        success = false;
    }
    if (typeName != 0
        && !any2.getValueType().getTypeName().equalsAscii(typeName))
    {
        fprintf(
            stderr, "any has wrong type after roundtrip: %s instead of %s\n",
            OUStringToOString(
                any2.getValueType().getTypeName(),
                RTL_TEXTENCODING_ASCII_US).getStr(),
            typeName);
        success = false;
    }
    return success;
}

}

static bool performAnyTest( const Reference< XBridgeTest > &xLBT, const TestData &data)
{
    bool bReturn = true;
    bReturn = testAny( data.Byte ,xLBT ) && bReturn;
    bReturn = testAny( data.Short,xLBT ) && bReturn;
    bReturn = testAny( data.UShort,xLBT ) && bReturn;
    bReturn = testAny( data.Long,xLBT ) && bReturn;
    bReturn = testAny( data.ULong,xLBT ) && bReturn;
    bReturn = testAny( data.Hyper,xLBT ) && bReturn;
    bReturn = testAny( data.UHyper,xLBT ) && bReturn;
    bReturn = testAny( data.Float,xLBT ) && bReturn;
    bReturn = testAny( data.Double,xLBT ) && bReturn;
    bReturn = testAny( data.Enum,xLBT ) && bReturn;
    bReturn = testAny( data.String,xLBT ) && bReturn;
    bReturn = testAny( data.Interface,xLBT ) && bReturn;
    bReturn = testAny( data, xLBT ) && bReturn;
    bReturn &= testAny(
        TestPolyStruct< sal_Unicode >(' '), xLBT,
        "test.testtools.bridgetest.TestPolyStruct<char>");

    Any a;
    {
        a.setValue( &(data.Bool) , cppu::UnoType<bool>::get() );
        OSL_ASSERT( xLBT->transportAny( a ) == a );
    }

    {
        a.setValue( &(data.Char) , cppu::UnoType<cppu::UnoCharType>::get() );
        OSL_ASSERT( xLBT->transportAny( a ) == a );
    }

    return bReturn;
}


static bool performSequenceOfCallTest( const Reference < XBridgeTest > &xLBT )
{
    sal_Int32 i,nRounds;
    sal_Int32 nGlobalIndex = 0;
    const sal_Int32 nWaitTimeSpanMUSec = 10000;
    for( nRounds = 0 ; nRounds < 10 ; nRounds ++ )
    {
        for( i = 0 ; i < nRounds ; i ++ )
        {
            // fire oneways
            xLBT->callOneway( nGlobalIndex , nWaitTimeSpanMUSec );
            nGlobalIndex ++;
        }

        // call synchron
        xLBT->call( nGlobalIndex , nWaitTimeSpanMUSec );
        nGlobalIndex ++;
    }

    return xLBT->sequenceOfCallTestPassed();
}

class ORecursiveCall : public WeakImplHelper< XRecursiveCall >
{
private:
    Mutex m_mutex;

public:
    void SAL_CALL callRecursivly(
        const ::com::sun::star::uno::Reference< XRecursiveCall >& xCall,
        sal_Int32 nToCall )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override
        {
            MutexGuard guard( m_mutex );
            if( nToCall )
            {
                nToCall --;
                xCall->callRecursivly( this , nToCall );
            }

        }
};



static bool performRecursiveCallTest( const Reference < XBridgeTest > & xLBT )
{
    xLBT->startRecursiveCall( new ORecursiveCall , 50 );
    // on failure, the test would lock up or crash
    return true;
}

class MyClass : public osl::DebugBase<MyClass>, public OWeakObject
{
public:
    MyClass();
    virtual ~MyClass();
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;
};


MyClass::MyClass()
{
}

MyClass::~MyClass()
{
}

void MyClass::acquire() throw ()
{
    OWeakObject::acquire();
}

void MyClass::release() throw ()
{
    OWeakObject::release();
}


static bool performTest(
    const Reference<XComponentContext> & xContext,
    const Reference<XBridgeTest > & xLBT,
    bool noCurrentContext )
{
    check(xLBT.is(), "### no test interface!");
    bool bRet = true;
    if (xLBT.is()) {
        // this data is never ever granted access to by calls other than
        // equals(), assign()!
        TestData aData; // test against this data
        Reference< XInterface > xI(new MyClass);
        assign(
            (TestElement &) aData, true, '@', 17, 0x1234, 0xFEDC, 0x12345678,
            0xFEDCBA98, SAL_CONST_INT64(0x123456789ABCDEF0),
            SAL_CONST_UINT64(0xFEDCBA9876543210), 17.0815f, 3.1415926359,
            TestEnum_LOLA, STRING_TEST_CONSTANT, xI,
            Any(&xI, cppu::UnoType<XInterface>::get()));
        bRet &= check(aData.Any == xI, "### unexpected any!");
        bRet &= check(!(aData.Any != xI), "### unexpected any!");
        aData.Sequence.realloc(2);
        aData.Sequence[0] = *static_cast<TestElement const *>(&aData);
        // aData.Sequence[1] is empty
        // aSetData is a manually copy of aData for first setting:
        TestData aSetData;
        assign(
            (TestElement &) aSetData, aData.Bool, aData.Char, aData.Byte,
            aData.Short, aData.UShort, aData.Long, aData.ULong, aData.Hyper,
            aData.UHyper, aData.Float, aData.Double, aData.Enum, aData.String,
            xI, Any(&xI, cppu::UnoType<XInterface>::get()));
        aSetData.Sequence.realloc(2);
        aSetData.Sequence[0] = *static_cast<TestElement const *>(&aSetData);
        // aSetData.Sequence[1] is empty
        xLBT->setValues(
            aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short,
            aSetData.UShort, aSetData.Long, aSetData.ULong, aSetData.Hyper,
            aSetData.UHyper, aSetData.Float, aSetData.Double, aSetData.Enum,
            aSetData.String, aSetData.Interface, aSetData.Any,
            aSetData.Sequence, aSetData);
        {
            TestData aRet;
            TestData aRet2;
            xLBT->getValues(
                aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
                aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float,
                aRet.Double, aRet.Enum, aRet.String, aRet.Interface, aRet.Any,
                aRet.Sequence, aRet2);
            bRet &= check(
                equals(aData, aRet) && equals(aData, aRet2), "getValues test");
            // Set last retrieved values:
            TestData aSV2ret(
                xLBT->setValues2(
                    aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
                    aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float,
                    aRet.Double, aRet.Enum, aRet.String, aRet.Interface,
                    aRet.Any, aRet.Sequence, aRet2));
            // Check inout sequence order (=> inout sequence parameter was
            // switched by test objects):
            TestElement temp(aRet.Sequence[0]);
            aRet.Sequence[0] = aRet.Sequence[1];
            aRet.Sequence[1] = temp;
            bRet &= check(
                equals(aData, aSV2ret) && equals(aData, aRet2),
                "getValues2 test");
        }
        {
            TwoFloats aIn(1.1f, 2.2f);
            TwoFloats aOut = xLBT->echoTwoFloats(aIn);
            bRet = check( memcmp(&aIn, &aOut, sizeof(TwoFloats)) == 0, "two floats struct test" ) && bRet;
        }
        {
            FourFloats aIn(3.3f, 4.4f, 5.5f, 6.6f);
            FourFloats aOut = xLBT->echoFourFloats(aIn);
            bRet = check( memcmp(&aIn, &aOut, sizeof(FourFloats)) == 0, "four floats struct test" ) && bRet;
        }
        {
            MixedFloatAndInteger aIn(7.7f, 8);
            MixedFloatAndInteger aOut = xLBT->echoMixedFloatAndInteger(aIn);
            bRet = check( memcmp(&aIn, &aOut, sizeof(MixedFloatAndInteger)) == 0, "mixed float and integer struct test" ) && bRet;
        }
        {
            ThreeByteStruct aIn(9, 10, 11);
            ThreeByteStruct aOut = xLBT->echoThreeByteStruct(aIn);
            bRet = check( memcmp(&aIn, &aOut, sizeof(ThreeByteStruct)) == 0, "three byte struct test" ) && bRet;
        }
        {
            TestData aRet;
            TestData aRet2;
            TestData aGVret(
                xLBT->getValues(
                    aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
                    aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float,
                    aRet.Double, aRet.Enum, aRet.String, aRet.Interface,
                    aRet.Any, aRet.Sequence, aRet2));
            bRet &= check(
                (equals(aData, aRet) && equals(aData, aRet2) &&
                 equals(aData, aGVret)),
                "getValues test");
            // Set last retrieved values:
            xLBT->setBool(aRet.Bool);
            xLBT->setChar(aRet.Char);
            xLBT->setByte(aRet.Byte);
            xLBT->setShort(aRet.Short);
            xLBT->setUShort(aRet.UShort);
            xLBT->setLong(aRet.Long);
            xLBT->setULong(aRet.ULong);
            xLBT->setHyper(aRet.Hyper);
            xLBT->setUHyper(aRet.UHyper);
            xLBT->setFloat(aRet.Float);
            xLBT->setDouble(aRet.Double);
            xLBT->setEnum(aRet.Enum);
            xLBT->setString(aRet.String);
            xLBT->setInterface(aRet.Interface);
            xLBT->setAny(aRet.Any);
            xLBT->setSequence(aRet.Sequence);
            xLBT->setStruct(aRet2);
        }
        {
            TestData aRet;
            aRet.Hyper = xLBT->getHyper();
            aRet.UHyper = xLBT->getUHyper();
            aRet.Float = xLBT->getFloat();
            aRet.Double = xLBT->getDouble();
            aRet.Byte = xLBT->getByte();
            aRet.Char = xLBT->getChar();
            aRet.Bool = xLBT->getBool();
            aRet.Short = xLBT->getShort();
            aRet.UShort = xLBT->getUShort();
            aRet.Long = xLBT->getLong();
            aRet.ULong = xLBT->getULong();
            aRet.Enum = xLBT->getEnum();
            aRet.String = xLBT->getString();
            aRet.Interface = xLBT->getInterface();
            aRet.Any = xLBT->getAny();
            aRet.Sequence = xLBT->getSequence();
            TestData aRet2(xLBT->getStruct());
            bRet &= check(
                equals(aData, aRet) && equals(aData, aRet2),
                "struct comparison test");
            {
                SmallStruct aIn(1, 2);
                SmallStruct aOut(xLBT->echoSmallStruct(aIn));
                bRet &= check(
                    memcmp(&aIn, &aOut, sizeof(SmallStruct)) == 0,
                    "small struct test");
            }
            {
                MediumStruct aIn(1, 2, 3, 4);
                MediumStruct aOut(xLBT->echoMediumStruct(aIn));
                bRet &= check(
                    memcmp(&aIn, &aOut, sizeof(MediumStruct)) == 0,
                    "medium struct test");
            }
            {
                BigStruct aIn(1, 2, 3, 4, 5, 6, 7, 8);
                BigStruct aOut(xLBT->echoBigStruct(aIn));
                bRet &= check(
                    memcmp(&aIn, &aOut, sizeof(BigStruct)) == 0,
                    "big struct test");
            }
            {
                sal_Int32 i2 = xLBT->testPPCAlignment(0, 0, 0, 0, 0xBEAF);
                bRet &= check(i2 == 0xBEAF, "ppc-style alignment test");
            }
            {
                sal_Int32 i1 = xLBT->testPPC64Alignment(1.0, 2.0, 3.0, 0xBEAF);
                bRet &= check(i1 == 0xBEAF, "ppc64-style alignment test");
            }
            {
                double d1 = xLBT->testTenDoubles(0.1, 0.2, 0.3, 0.4, 0.5,
                    0.6, 0.7, 0.8, 0.9, 1.0);
                bRet &= check(d1 == 5.5, "armhf doubles test");
            }
            // Test extended attributes that raise exceptions:
            try {
                xLBT->getRaiseAttr1();
                bRet &= check(false, "getRaiseAttr1 did not throw");
            } catch (const RuntimeException &) {
            } catch (...) {
                bRet &= check(false, "getRaiseAttr1 threw wrong type");
            }
            try {
                xLBT->setRaiseAttr1(0);
                bRet &= check(false, "setRaiseAttr1 did not throw");
            } catch (const IllegalArgumentException &) {
            } catch (...) {
                bRet &= check(false, "setRaiseAttr1 threw wrong type");
            }
            try {
                xLBT->getRaiseAttr2();
                bRet &= check(false, "getRaiseAttr2 did not throw");
            } catch (const IllegalArgumentException &) {
            } catch (...) {
                bRet &= check(false, "getRaiseAttr2 threw wrong type");
            }
            // Test instantiated polymorphic struct types:
            {
                bRet &= check(
                    (xLBT->transportPolyBoolean(
                        TestPolyStruct< sal_Bool >(true)).
                     member),
                    "transportPolyBoolean");
                TestPolyStruct< sal_Int64 > tps1(12345);
                xLBT->transportPolyHyper(tps1);
                bRet &= check(tps1.member == 12345, "transportPolyHyper");
                Sequence< Any > seq(2);
                seq[0] <<= static_cast< sal_uInt32 >(33);
                seq[1] <<= OUString("ABC");
                TestPolyStruct< Sequence< Any > > tps2(seq);
                TestPolyStruct< Sequence< Any > > tps3;
                xLBT->transportPolySequence(tps2, tps3);
                bRet &= check(
                    tps3.member.getLength() == 2,
                    "transportPolySequence, length");
                sal_uInt32 v0 = sal_uInt32();
                tps3.member[0] >>= v0;
                bRet &= check(v0 == 33, "transportPolySequence, element 0");
                OUString v1;
                tps3.member[1] >>= v1;
                bRet &= check( v1 == "ABC", "transportPolySequence, element 1" );
                bRet &= check(
                    xLBT->getNullPolyLong().member == 0, "getNullPolyLong");
                bRet &= check(
                    xLBT->getNullPolyString().member.isEmpty(),
                    "getNullPolyString");
                bRet &= check(
                    xLBT->getNullPolyType().member == Type(),
                    "getNullPolyType");
                Any nullAny(xLBT->getNullPolyAny().member);
                bRet &= check(
                    (((nullAny.getValueTypeName() ==
                       "com.sun.star.uno.XInterface") &&
                      !static_cast< Reference< XInterface > const * >(
                          nullAny.getValue())->is())
                     || nullAny == Any()),
                    "getNullPolyAny");
                bRet &= check(
                    xLBT->getNullPolySequence().member.getLength() == 0,
                    "getNullPolySequence");
                bRet &= check(
                    xLBT->getNullPolyEnum().member == TestEnum_TEST,
                    "getNullPolyEnum");
                bRet &= check(
                    xLBT->getNullPolyBadEnum().member == TestBadEnum_M,
                    "getNullPolyBadEnum");
                bRet &= check(
                    xLBT->getNullPolyStruct().member.member == 0,
                    "getNullPolyStruct");
                bRet &= check(
                    !xLBT->getNullPolyInterface().member.is(),
                    "getNullPolyInterface");
            }
            // Any test:
            bRet &= check(performAnyTest(xLBT , aData), "any test");
            // Sequence of call test:
            bRet &= check(
                performSequenceOfCallTest(xLBT), "sequence of call test");
            // Recursive call test:
            bRet &= check(performRecursiveCallTest(xLBT), "recursive test");
            bRet &= check(
                equals(aData, aRet) && equals(aData, aRet2),
                "recursive test results");
            // Multiple inheritance test:
            bRet &= checkEmpty(
                testtools::bridgetest::testMulti(xLBT->getMulti()),
                "remote multi");
            bRet &= checkEmpty(
                xLBT->testMulti(new testtools::bridgetest::Multi),
                "local multi");
        }
    }
    {
        Reference< XBridgeTest2 > xBT2(xLBT, UNO_QUERY);
        if (!xBT2.is()) {
            return bRet;
        }
        // Perform sequence tests (XBridgeTest2); create the sequence which is
        // compared with the results:
        sal_Bool _arBool[] = { true, false, true };
        sal_Unicode _arChar[] = { 0x0065, 0x0066, 0x0067 };
        sal_Int8 _arByte[] = { 1, 2, -1 };
        sal_Int16 _arShort[] = { -0x8000, 1, 0x7FFF };
        sal_uInt16 _arUShort[] = { 0 , 1, 0xFFFF };
        sal_Int32 _arLong[] = {
            static_cast< sal_Int32 >(0x80000000), 1, 0x7FFFFFFF };
        sal_uInt32 _arULong[] = { 0, 1, 0xFFFFFFFF };
        sal_Int64 _arHyper[] = {
            static_cast< sal_Int64 >(SAL_CONST_INT64(0x8000000000000000)), 1,
            SAL_CONST_INT64(0x7FFFFFFFFFFFFFFF) };
        sal_uInt64 _arUHyper[] = { 0, 1, SAL_CONST_UINT64(0xFFFFFFFFFFFFFFFF) };
        OUString _arString[] = {
            OUString("String 1"),
            OUString("String 2"),
            OUString("String 3") };
        sal_Bool _aBool = true;
        sal_Int32 _aInt = 0xBABEBABE;
        float _aFloat = 3.14f;
        Any _any1(&_aBool, cppu::UnoType<bool>::get());
        Any _any2(&_aInt, cppu::UnoType<sal_Int32>::get());
        Any _any3(&_aFloat, cppu::UnoType<float>::get());
        Any _arAny[] = { _any1, _any2, _any3 };
        Reference< XInterface > _arObj[3];
        _arObj[0] = new OWeakObject();
        _arObj[1] = new OWeakObject();
        _arObj[2] = new OWeakObject();
        TestEnum _arEnum[] = { TestEnum_ONE, TestEnum_TWO, TestEnum_CHECK };
        TestElement _arStruct[3];
        assign(
            _arStruct[0], true, '@', 17, 0x1234, 0xFEDC, 0x12345678, 0xFEDCBA98,
            SAL_CONST_INT64(0x123456789ABCDEF0),
            SAL_CONST_UINT64(0xFEDCBA9876543210), 17.0815f, 3.1415926359,
            TestEnum_LOLA, STRING_TEST_CONSTANT, _arObj[0],
            Any(&_arObj[0], cppu::UnoType<XInterface>::get()));
        assign(
            _arStruct[1], true, 'A', 17, 0x1234, 0xFEDC, 0x12345678, 0xFEDCBA98,
            SAL_CONST_INT64(0x123456789ABCDEF0),
            SAL_CONST_UINT64(0xFEDCBA9876543210), 17.0815f, 3.1415926359,
            TestEnum_TWO, STRING_TEST_CONSTANT, _arObj[1],
            Any(&_arObj[1], cppu::UnoType<XInterface>::get()));
        assign(
            _arStruct[2], true, 'B', 17, 0x1234, 0xFEDC, 0x12345678, 0xFEDCBA98,
            SAL_CONST_INT64(0x123456789ABCDEF0),
            SAL_CONST_UINT64(0xFEDCBA9876543210), 17.0815f, 3.1415926359,
            TestEnum_CHECK, STRING_TEST_CONSTANT, _arObj[2],
            Any(&_arObj[2], cppu::UnoType<XInterface>::get()));
        {
            float _arFloat[] = { 1.1f, 2.2f, 3.3f };
            double _arDouble[] = { 1.11, 2.22, 3.33 };
            Sequence<sal_Bool> arBool(_arBool, 3);
            Sequence<sal_Unicode> arChar( _arChar, 3);
            Sequence<sal_Int8> arByte(_arByte, 3);
            Sequence<sal_Int16> arShort(_arShort, 3);
            Sequence<sal_uInt16> arUShort(_arUShort, 3);
            Sequence<sal_Int32> arLong(_arLong, 3);
            Sequence<sal_uInt32> arULong(_arULong, 3);
            Sequence<sal_Int64> arHyper(_arHyper, 3);
            Sequence<sal_uInt64> arUHyper(_arUHyper, 3);
            Sequence<float> arFloat(_arFloat, 3);
            Sequence<double> arDouble(_arDouble, 3);
            Sequence<OUString> arString(_arString, 3);
            Sequence<Any> arAny(_arAny, 3);
            Sequence<Reference<XInterface> > arObject(_arObj, 3);
            Sequence<TestEnum> arEnum(_arEnum, 3);
            Sequence<TestElement> arStruct(_arStruct, 3);
            Sequence<Sequence<sal_Int32> > _arSeqLong2[3];
            for (int j = 0; j != 3; ++j) {
                Sequence< sal_Int32 > _arSeqLong[3];
                for (int i = 0; i != 3; ++i) {
                    _arSeqLong[i] = Sequence< sal_Int32 >(_arLong, 3);
                }
                _arSeqLong2[j] = Sequence< Sequence< sal_Int32 > >(
                    _arSeqLong, 3);
            }
            Sequence< Sequence< Sequence< sal_Int32> > > arLong3(
                _arSeqLong2, 3);
            Sequence< Sequence< sal_Int32 > > seqSeqRet(
                xBT2->setDim2(arLong3[0]));
            bRet &= check(seqSeqRet == arLong3[0], "sequence test");
            Sequence< Sequence< Sequence< sal_Int32 > > > seqSeqRet2(
                xBT2->setDim3(arLong3));
            bRet &= check(seqSeqRet2 == arLong3, "sequence test");
            Sequence< Any > seqAnyRet(xBT2->setSequenceAny(arAny));
            bRet &= check(seqAnyRet == arAny, "sequence test");
            Sequence< sal_Bool > seqBoolRet(xBT2->setSequenceBool(arBool));
            bRet &= check(seqBoolRet == arBool, "sequence test");
            Sequence< sal_Int8 > seqByteRet(xBT2->setSequenceByte(arByte));
            bRet &= check(seqByteRet == arByte, "sequence test");
            Sequence< sal_Unicode > seqCharRet(xBT2->setSequenceChar(arChar));
            bRet &= check(seqCharRet == arChar, "sequence test");
            Sequence< sal_Int16 > seqShortRet(xBT2->setSequenceShort(arShort));
            bRet &= check(seqShortRet == arShort, "sequence test");
            Sequence< sal_Int32 > seqLongRet(xBT2->setSequenceLong(arLong));
            bRet &= check(seqLongRet == arLong, "sequence test");
            Sequence< sal_Int64 > seqHyperRet(xBT2->setSequenceHyper(arHyper));
            bRet &= check(seqHyperRet == arHyper, "sequence test");
            Sequence< float > seqFloatRet(xBT2->setSequenceFloat(arFloat));
            bRet &= check(seqFloatRet == arFloat, "sequence test");
            Sequence< double > seqDoubleRet(xBT2->setSequenceDouble(arDouble));
            bRet &= check(seqDoubleRet == arDouble, "sequence test");
            Sequence< TestEnum > seqEnumRet(xBT2->setSequenceEnum(arEnum));
            bRet &= check(seqEnumRet == arEnum, "sequence test");
            Sequence< sal_uInt16 > seqUShortRet(
                xBT2->setSequenceUShort(arUShort));
            bRet &= check(seqUShortRet == arUShort, "sequence test");
            Sequence< sal_uInt32 > seqULongRet(xBT2->setSequenceULong(arULong));
            bRet &= check(seqULongRet == arULong, "sequence test");
            Sequence< sal_uInt64 > seqUHyperRet(
                xBT2->setSequenceUHyper(arUHyper));
            bRet &= check(seqUHyperRet == arUHyper, "sequence test");
            Sequence< Reference< XInterface > > seqObjectRet(
                xBT2->setSequenceXInterface(arObject));
            bRet &= check(seqObjectRet == arObject, "sequence test");
            Sequence< OUString > seqStringRet(
                xBT2->setSequenceString(arString));
            bRet &= check(seqStringRet == arString, "sequence test");
            Sequence< TestElement > seqStructRet(
                xBT2->setSequenceStruct(arStruct));
            bRet &= check(seqStructRet == arStruct, "sequence test");
            Sequence< sal_Bool > arBoolTemp(cloneSequence(arBool));
            Sequence< sal_Unicode > arCharTemp(cloneSequence<sal_Unicode, cppu::UnoCharType>(arChar));
            Sequence< sal_Int8 > arByteTemp(cloneSequence(arByte));
            Sequence< sal_Int16 > arShortTemp(cloneSequence(arShort));
            Sequence< sal_uInt16 > arUShortTemp(cloneSequence<sal_uInt16, cppu::UnoUnsignedShortType>(arUShort));
            Sequence< sal_Int32 > arLongTemp(cloneSequence(arLong));
            Sequence< sal_uInt32 > arULongTemp(cloneSequence(arULong));
            Sequence< sal_Int64 > arHyperTemp(cloneSequence(arHyper));
            Sequence< sal_uInt64 > arUHyperTemp(cloneSequence(arUHyper));
            Sequence< float > arFloatTemp(cloneSequence(arFloat));
            Sequence< double > arDoubleTemp(cloneSequence(arDouble));
            Sequence< TestEnum > arEnumTemp(cloneSequence(arEnum));
            Sequence< OUString > arStringTemp(cloneSequence(arString));
            Sequence< Reference< XInterface > > arObjectTemp(
                cloneSequence(arObject));
            Sequence< Any > arAnyTemp(cloneSequence(arAny));
            Sequence< Sequence< sal_Int32 > > arLong2Temp(arLong3[0]);
            Sequence< Sequence< Sequence< sal_Int32 > > > arLong3Temp(arLong3);
            xBT2->setSequencesInOut(
                arBoolTemp, arCharTemp, arByteTemp, arShortTemp, arUShortTemp,
                arLongTemp,arULongTemp, arHyperTemp, arUHyperTemp, arFloatTemp,
                arDoubleTemp, arEnumTemp, arStringTemp, arObjectTemp, arAnyTemp,
                arLong2Temp, arLong3Temp);
            bRet &= check(
                (arBoolTemp == arBool && arCharTemp == arChar &&
                 arByteTemp == arByte && arShortTemp == arShort &&
                 arUShortTemp == arUShort && arLongTemp == arLong &&
                 arULongTemp == arULong && arHyperTemp == arHyper &&
                 arUHyperTemp == arUHyper && arFloatTemp == arFloat &&
                 arDoubleTemp == arDouble && arEnumTemp == arEnum &&
                 arStringTemp == arString && arObjectTemp == arObject &&
                 arAnyTemp == arAny && arLong2Temp == arLong3[0] &&
                 arLong3Temp == arLong3),
                "sequence test");
            Sequence< sal_Bool > arBoolOut;
            Sequence< sal_Unicode > arCharOut;
            Sequence< sal_Int8 > arByteOut;
            Sequence< sal_Int16 > arShortOut;
            Sequence< sal_uInt16 > arUShortOut;
            Sequence< sal_Int32 > arLongOut;
            Sequence< sal_uInt32 > arULongOut;
            Sequence< sal_Int64 > arHyperOut;
            Sequence< sal_uInt64 > arUHyperOut;
            Sequence< float > arFloatOut;
            Sequence< double > arDoubleOut;
            Sequence< TestEnum > arEnumOut;
            Sequence< OUString > arStringOut;
            Sequence< Reference< XInterface > > arObjectOut;
            Sequence< Any > arAnyOut;
            Sequence< Sequence< sal_Int32 > > arLong2Out;
            Sequence< Sequence< Sequence< sal_Int32 > > > arLong3Out;
            xBT2->setSequencesOut(
                arBoolOut, arCharOut, arByteOut, arShortOut, arUShortOut,
                arLongOut,arULongOut, arHyperOut, arUHyperOut, arFloatOut,
                arDoubleOut, arEnumOut, arStringOut, arObjectOut, arAnyOut,
                arLong2Out, arLong3Out);
            bRet &= check(
                (arBoolOut == arBool && arCharOut == arChar &&
                 arByteOut == arByte && arShortOut == arShort &&
                 arUShortOut == arUShort && arLongOut == arLong &&
                 arULongOut == arULong && arHyperOut == arHyper &&
                 arUHyperOut == arUHyper && arFloatOut == arFloat &&
                 arDoubleOut == arDouble && arEnumOut == arEnum &&
                 arStringOut == arString && arObjectOut == arObject &&
                 arAnyOut == arAny && arLong2Out == arLong3[0] &&
                 arLong3Out == arLong3),
                "sequence test");
        }
        {
            // Test with empty sequences:
            Sequence< Sequence< sal_Int32 > > arLong2;
            Sequence< Sequence< sal_Int32 > > seqSeqRet(xBT2->setDim2(arLong2));
            bRet &= check(seqSeqRet == arLong2, "sequence test");
            Sequence< Sequence< Sequence< sal_Int32 > > > arLong3;
            Sequence< Sequence< Sequence< sal_Int32 > > > seqSeqRet2(
                xBT2->setDim3(arLong3));
            bRet &= check(seqSeqRet2 == arLong3, "sequence test");
            Sequence< Any > arAny;
            Sequence< Any > seqAnyRet(xBT2->setSequenceAny(arAny));
            bRet &= check(seqAnyRet == arAny, "sequence test");
            Sequence< sal_Bool > arBool;
            Sequence< sal_Bool > seqBoolRet(xBT2->setSequenceBool(arBool));
            bRet &= check(seqBoolRet == arBool, "sequence test");
            Sequence< sal_Int8 > arByte;
            Sequence< sal_Int8 > seqByteRet(xBT2->setSequenceByte(arByte));
            bRet &= check(seqByteRet == arByte, "sequence test");
            Sequence< sal_Unicode > arChar;
            Sequence< sal_Unicode > seqCharRet(xBT2->setSequenceChar(arChar));
            bRet &= check(seqCharRet == arChar, "sequence test");
            Sequence< sal_Int16 > arShort;
            Sequence< sal_Int16 > seqShortRet(xBT2->setSequenceShort(arShort));
            bRet &= check(seqShortRet == arShort, "sequence test");
            Sequence< sal_Int32 > arLong;
            Sequence< sal_Int32 > seqLongRet(xBT2->setSequenceLong(arLong));
            bRet &= check(seqLongRet == arLong, "sequence test");
            Sequence< sal_Int64 > arHyper;
            Sequence< sal_Int64 > seqHyperRet(xBT2->setSequenceHyper(arHyper));
            bRet &= check(seqHyperRet == arHyper, "sequence test");
            Sequence< float > arFloat;
            Sequence< float > seqFloatRet(xBT2->setSequenceFloat(arFloat));
            bRet &= check(seqFloatRet == arFloat, "sequence test");
            Sequence< double > arDouble;
            Sequence< double > seqDoubleRet(xBT2->setSequenceDouble(arDouble));
            bRet &= check(seqDoubleRet == arDouble, "sequence test");
            Sequence< TestEnum > arEnum;
            Sequence< TestEnum > seqEnumRet(xBT2->setSequenceEnum(arEnum));
            bRet &= check(seqEnumRet == arEnum, "sequence test");
            Sequence< sal_uInt16 > arUShort;
            Sequence< sal_uInt16 > seqUShortRet(
                xBT2->setSequenceUShort(arUShort));
            bRet &= check(seqUShortRet == arUShort, "sequence test");
            Sequence< sal_uInt32 > arULong;
            Sequence< sal_uInt32 > seqULongRet(xBT2->setSequenceULong(arULong));
            bRet &= check(seqULongRet == arULong, "sequence test");
            Sequence< sal_uInt64 > arUHyper;
            Sequence< sal_uInt64 > seqUHyperRet(
                xBT2->setSequenceUHyper(arUHyper));
            bRet &= check(seqUHyperRet == arUHyper, "sequence test");
            Sequence< Reference< XInterface > > arObject;
            Sequence< Reference< XInterface > > seqObjectRet(
                xBT2->setSequenceXInterface(arObject));
            bRet &= check(seqObjectRet == arObject, "sequence test");
            Sequence< OUString > arString;
            Sequence< OUString > seqStringRet(
                xBT2->setSequenceString(arString));
            bRet &= check(seqStringRet == arString, "sequence test");
            Sequence< TestElement > arStruct;
            Sequence< TestElement > seqStructRet(
                xBT2->setSequenceStruct(arStruct));
            bRet &= check(seqStructRet == arStruct, "sequence test");
        }
        // Issue #i60341# shows that the most interesting case is were Java
        // calls the constructors; however, since this client is currently not
        // available in Java, while the server is, the logic is reversed here:
        try {
            xBT2->testConstructorsService(xContext);
        } catch (const BadConstructorArguments &) {
            bRet = false;
        }
        if (!noCurrentContext) {
            if (!(new testtools::bridgetest::CurrentContextChecker)->perform(
                    xBT2->getCurrentContextChecker(), 0, 1))
            {
                bRet = false;
            }
            if (!(new testtools::bridgetest::CurrentContextChecker)->perform(
                    xBT2->getCurrentContextChecker(), 0, 2))
            {
                bRet = false;
            }
            if (!(new testtools::bridgetest::CurrentContextChecker)->perform(
                    xBT2->getCurrentContextChecker(), 1, 2))
            {
                bRet = false;
            }
            if (!(new testtools::bridgetest::CurrentContextChecker)->perform(
                    xBT2->getCurrentContextChecker(), 1, 3))
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

static bool raiseOnewayException( const Reference < XBridgeTest > & xLBT )
{
    bool bReturn = true;
    OUString sCompare = STRING_TEST_CONSTANT;
    Reference<XInterface> const x(xLBT->getInterface());
    try
    {
        // Note : the exception may fly or not (e.g. remote scenario).
        //        When it flies, it must contain the correct elements.
        xLBT->raiseRuntimeExceptionOneway( sCompare, x );
    }
    catch( const RuntimeException & e )
    {
        bReturn = (
#if OSL_DEBUG_LEVEL == 0
            // java stack traces trash Message
            e.Message == sCompare &&
#endif
            xLBT->getInterface() == e.Context &&
            x == e.Context );
    }
    return bReturn;
}


static bool raiseException( const Reference< XBridgeTest > & xLBT )
{
    sal_Int32 nCount = 0;
    try
    {
        try
        {
            try
            {
                TestData aRet, aRet2;
                xLBT->raiseException(
                    5, STRING_TEST_CONSTANT,
                    xLBT->getInterface() );
            }
            catch (const IllegalArgumentException &rExc)
            {
                if (rExc.ArgumentPosition == 5 &&
#if OSL_DEBUG_LEVEL == 0
                    // java stack traces trash Message
                    rExc.Message == STRING_TEST_CONSTANT &&
#endif
                    rExc.Context == xLBT->getInterface())
                {
#ifdef COMPCHECK
                    //When we check if a new compiler still works then we must not call
                    //getRuntimeException because it uses cppu::getCaughtException which
                    //does only work if all libs are build with the same runtime.
                    return true;
#else
                    ++nCount;
#endif
                }
                else
                {
                    check( false, "### unexpected exception content!" );
                }

                /** it is certain, that the RuntimeException testing will fail, if no */
                xLBT->getRuntimeException();
            }
        }
        catch (const RuntimeException & rExc)
        {
            if (rExc.Context == xLBT->getInterface()
#if OSL_DEBUG_LEVEL == 0
                    // java stack traces trash Message
                && rExc.Message == STRING_TEST_CONSTANT
#endif
                )
            {
                ++nCount;
            }
            else
            {
                check( false, "### unexpected exception content!" );
            }

            /** it is certain, that the RuntimeException testing will fail, if no */
            xLBT->setRuntimeException( 0xcafebabe );
        }
    }
    catch (const Exception & rExc)
    {
        if (rExc.Context == xLBT->getInterface()
#if OSL_DEBUG_LEVEL == 0
            // java stack traces trash Message
            && rExc.Message == STRING_TEST_CONSTANT
#endif
            )
        {
            ++nCount;
        }
        else
        {
            check( false, "### unexpected exception content!" );
        }
        return (nCount == 3);
    }
    return false;
}

/* Returns an acquired sequence
 */
uno_Sequence* cloneSequence(const uno_Sequence* val, const Type& type)
{
    TypeDescription td(type);
    td.makeComplete();
    typelib_TypeDescription* pTdRaw = td.get();
    typelib_IndirectTypeDescription* pIndirectTd =
        reinterpret_cast<typelib_IndirectTypeDescription*>(pTdRaw);

    typelib_TypeDescription* pTdElem = pIndirectTd->pType->pType;
    sal_Int8* buf = new sal_Int8[pTdElem->nSize * val->nElements];
    sal_Int8* pBufCur = buf;

    uno_Sequence* retSeq = NULL;
    switch (pTdElem->eTypeClass)
    {
    case TypeClass_SEQUENCE:
    {
        Type _tElem(pTdElem->pWeakRef);
        for (int i = 0; i < val->nElements; i++)
        {
            sal_Int8 const *pValBuf = reinterpret_cast<sal_Int8 const *>(&val->elements + i * pTdElem->nSize);

            uno_Sequence* seq = cloneSequence(
                reinterpret_cast<uno_Sequence const *>(pValBuf),
                _tElem);
            *reinterpret_cast<uno_Sequence**>(pBufCur) = seq;
            pBufCur += pTdElem->nSize;
        }
        break;
    }
    default:
        uno_type_sequence_construct(
            &retSeq, type.getTypeLibType(), const_cast<char *>(val->elements),
            val->nElements, reinterpret_cast< uno_AcquireFunc >(cpp_acquire));
        break;
    }
    delete[] buf;
    return retSeq;
}

template<typename T, typename U>
Sequence<T> cloneSequence(const Sequence<T>& val)
{
    Sequence<T> seq( cloneSequence(val.get(), cppu::UnoType<cppu::UnoSequenceType<U>>::get()), SAL_NO_ACQUIRE);
    return seq;
}

template< class T >
inline bool makeSurrogate(
    Reference< T > & rOut, Reference< T > const & rOriginal )
{
    rOut.clear();
    if (! rOriginal.is())
        return false;

    Environment aCppEnv_official;
    Environment aUnoEnv_ano;
    Environment aCppEnv_ano;

    OUString aCppEnvTypeName(
        CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    OUString aUnoEnvTypeName(
        UNO_LB_UNO );
    // official:
    uno_getEnvironment(
        reinterpret_cast< uno_Environment ** >( &aCppEnv_official ),
        aCppEnvTypeName.pData, 0 );
    // anonymous:
    uno_createEnvironment(
        reinterpret_cast< uno_Environment ** >( &aCppEnv_ano ),
        aCppEnvTypeName.pData, 0 );
    uno_createEnvironment(
        reinterpret_cast< uno_Environment ** >( &aUnoEnv_ano ),
        aUnoEnvTypeName.pData, 0 );

    UnoInterfaceReference unoI;
    Mapping cpp2uno( aCppEnv_official.get(), aUnoEnv_ano.get() );
    Mapping uno2cpp( aUnoEnv_ano.get(), aCppEnv_ano.get() );
    if (!cpp2uno.is() || !uno2cpp.is())
    {
        throw RuntimeException("cannot get C++-UNO mappings!" );
    }
    cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &unoI.m_pUnoI ),
        rOriginal.get(), cppu::UnoType<decltype(rOriginal)>::get() );
    if (! unoI.is())
    {
        throw RuntimeException(
            "mapping C++ to binary UNO failed!" );
    }
    uno2cpp.mapInterface(
        reinterpret_cast< void ** >( &rOut ),
        unoI.get(), cppu::UnoType<decltype(rOriginal)>::get() );
    if (! rOut.is())
    {
        throw RuntimeException(
            "mapping binary UNO to C++ failed!" );
    }

    return rOut.is();
}


sal_Int32 TestBridgeImpl::run( const Sequence< OUString > & rArgs )
    throw (RuntimeException, std::exception)
{
    bool bRet = false;
    try
    {
        if (! rArgs.getLength())
        {
            throw RuntimeException( "no test object specified!\n"
                                    "usage : ServiceName of test object | -u unourl of test object" );
        }

        Reference< XInterface > xOriginal;
        bool remote;
        sal_Int32 i;
        if( rArgs.getLength() > 1 && rArgs[0] == "-u" )
        {
            remote = true;
            i = 2;
        }
        else
        {
            remote = false;
            i = 1;
        }
        bool noCurrentContext = false;
        if ( i < rArgs.getLength() && rArgs[i] == "noCurrentContext" )
        {
            noCurrentContext = true;
            ++i;
        }
        bool stress = false;
        if ( i < rArgs.getLength() && rArgs[i] == "stress" )
        {
            stress = true;
            ++i;
        }

        for (;;) {
            Reference< XInterface > o;
            if (remote) {
                o = UnoUrlResolver::create(m_xContext)->resolve(rArgs[1]);
            } else {
                o = m_xContext->getServiceManager()->createInstanceWithContext(
                    rArgs[0], m_xContext);
            }
            if (!stress) {
                xOriginal = o;
                break;
            }
        }

        if (! xOriginal.is())
        {
            throw RuntimeException( "cannot get test object!" );
        }
        Reference< XBridgeTest > xTest( xOriginal, UNO_QUERY );
        if (! xTest.is())
        {
            throw RuntimeException( "test object does not implement XBridgeTest!" );
        }

        Reference<XBridgeTest > xLBT;
        bRet = check( makeSurrogate( xLBT, xTest ), "makeSurrogate" );
        bRet = check(
            performTest( m_xContext, xLBT, noCurrentContext ), "standard test" )
            && bRet;
        bRet = check( raiseException( xLBT ) , "exception test" )&& bRet;
        bRet = check( raiseOnewayException( xLBT ),
                      "oneway exception test" ) && bRet;
        if (! bRet)
        {
            throw RuntimeException( "error: test failed!" );
        }
    }
    catch (const Exception & exc)
    {
        OString cstr( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "exception occurred: %s\n", cstr.getStr() );
        throw;
    }

    return bRet ? 0 : 1;
}

// XServiceInfo

OUString TestBridgeImpl::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString( IMPLNAME );
}

sal_Bool TestBridgeImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > TestBridgeImpl::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return bridge_test::getSupportedServiceNames();
}




static Reference< XInterface > SAL_CALL TestBridgeImpl_create(
    const Reference< XComponentContext > & xContext )
{
    return Reference< XInterface >(
        static_cast< OWeakObject * >( new TestBridgeImpl( xContext ) ) );
}

}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XInterface > xFactory(
            createSingleComponentFactory(
                bridge_test::TestBridgeImpl_create,
                IMPLNAME,
                bridge_test::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
