/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <thread>
#include <utility>

#include <cppu/unotype.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <osl/diagnose.hxx>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/RuntimeException.hpp>
#include <cpo/uno/Sequence.hxx>

#include <test/testtools/bridgetest/Constructors.hpp>
#include <test/testtools/bridgetest/Constructors2.hpp>
#include <test/testtools/bridgetest/TestPolyStruct.hpp>
#include <test/testtools/bridgetest/TestPolyStruct2.hpp>
#include <test/testtools/bridgetest/XBridgeTest2.hpp>
#include <test/testtools/bridgetest/XMulti.hpp>

#include "currentcontextchecker.hxx"
#include "multi.hxx"

using namespace osl;
using namespace cppu;
using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace test::testtools::bridgetest;

#ifdef _MSC_VER
#pragma warning (disable : 4503) // irrelevant for test code
#endif

constexpr OUString SERVICENAME = u"com.sun.star.test.bridge.CppTestObject"_ustr;
constexpr OUString IMPLNAME = u"com.sun.star.comp.bridge.CppTestObject"_ustr;

namespace bridge_object
{


static Sequence< OUString > getSupportedServiceNames()
{
    return { SERVICENAME };
}


static void assign( TestElement & rData,
                    bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const OUString& rStr,
                    sal_Int8 nByte2, sal_Int16 nShort2,
                    const cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                    const cpo::uno::Any& rAny )
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
    rData.Byte2 = nByte2;
    rData.Short2 = nShort2;
    rData.Interface = xTest;
    rData.Any = rAny;
}

static void assign( TestData & rData,
                    bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const OUString& rStr,
                    sal_Int8 nByte2, sal_Int16 nShort2,
                    const cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                    const cpo::uno::Any& rAny,
                    const cpo::uno::Sequence< TestElement >& rSequence )
{
    assign( static_cast<TestElement &>(rData),
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, nByte2, nShort2, xTest, rAny );
    rData.Sequence = rSequence;
}

namespace {

class Test_Impl :
    public osl::DebugBase<Test_Impl>,
    public WeakImplHelper< XBridgeTest2, XServiceInfo , XRecursiveCall >
{
    TestData _aData, _aStructData;
    sal_Int32 m_nLastCallId;
    bool m_bFirstCall;
    bool m_bSequenceOfCallTestPassed;
    Mutex m_mutex;

    Sequence<bool> _arBool;
    Sequence<sal_Unicode> _arChar;
    Sequence<sal_Int8> _arByte;
    Sequence<sal_Int16> _arShort;
    Sequence<sal_uInt16> _arUShort;
    Sequence<sal_Int32> _arLong;
    Sequence<sal_uInt32> _arULong;
    Sequence<sal_Int64> _arHyper;
    Sequence<sal_uInt64> _arUHyper;
    Sequence<OUString> _arString;
    Sequence<float> _arFloat;
    Sequence<double> _arDouble;
    Sequence<TestEnum> _arEnum;
    Sequence<Reference<XInterface> > _arObject;
    Sequence<Sequence<sal_Int32> > _arLong2;
    Sequence<Sequence<Sequence<sal_Int32> > > _arLong3;
    Sequence<Any> _arAny;
    Sequence<TestElement> _arStruct;

public:
    Test_Impl() : m_nLastCallId( 0 ),
                  m_bFirstCall( true ),
                  m_bSequenceOfCallTestPassed( true )
        {}

    void acquire() noexcept override
    {
        OWeakObject::acquire();
    }
    void release() noexcept override
    {
        OWeakObject::release();
     }

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString & rServiceName ) override;
    virtual Sequence< OUString > getSupportedServiceNames() override;

    // XLBTestBase
    virtual void setValues( bool bBool,
                                     sal_Unicode cChar,
                                     sal_Int8 nByte,
                                     sal_Int16 nShort,
                                     sal_uInt16 nUShort,
                                     sal_Int32 nLong,
                                     sal_uInt32 nULong,
                                     sal_Int64 nHyper,
                                     sal_uInt64 nUHyper,
                                     float fFloat,
                                     double fDouble,
                                     TestEnum eEnum,
                                     const OUString& rStr,
                                     sal_Int8 nByte2,
                                     sal_Int16 nShort2,
                                     const cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                                     const cpo::uno::Any& rAny,
                                     const cpo::uno::Sequence<TestElement >& rSequence,
                                     const ::test::testtools::bridgetest::TestDataElements& rStruct ) override;

    virtual ::test::testtools::bridgetest::TestDataElements setValues2( bool& bBool,
                                                                                 sal_Unicode& cChar,
                                                                                 sal_Int8& nByte,
                                                                                 sal_Int16& nShort,
                                                                                 sal_uInt16& nUShort,
                                                                                 sal_Int32& nLong,
                                                                                 sal_uInt32& nULong,
                                                                                 sal_Int64& nHyper,
                                                                                 sal_uInt64& nUHyper,
                                                                                 float& fFloat,
                                                                                 double& fDouble,
                                                                                 TestEnum& eEnum,
                                                                                 OUString& rStr,
                                                                                 sal_Int8& nByte2,
                                                                                 sal_Int16& nShort2,
                                                                                 cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                                                                                 cpo::uno::Any& rAny,
                                                                                 cpo::uno::Sequence<TestElement >& rSequence,
                                                                                 ::test::testtools::bridgetest::TestDataElements& rStruct ) override;

    virtual ::test::testtools::bridgetest::TestDataElements getValues( bool& bBool,
                                                                                sal_Unicode& cChar,
                                                                                sal_Int8& nByte,
                                                                                sal_Int16& nShort,
                                                                                sal_uInt16& nUShort,
                                                                                sal_Int32& nLong,
                                                                                sal_uInt32& nULong,
                                                                                sal_Int64& nHyper,
                                                                                sal_uInt64& nUHyper,
                                                                                float& fFloat,
                                                                                double& fDouble,
                                                                                TestEnum& eEnum,
                                                                                OUString& rStr,
                                                                                sal_Int8& nByte2,
                                                                                sal_Int16& nShort2,
                                                                                cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                                                                                cpo::uno::Any& rAny,
                                                                                cpo::uno::Sequence< TestElement >& rSequence,
                                                                                ::test::testtools::bridgetest::TestDataElements& rStruct ) override;

    virtual SmallStruct echoSmallStruct(const SmallStruct& rStruct) override
        { return rStruct; }
    virtual MediumStruct echoMediumStruct(const MediumStruct& rStruct) override
        { return rStruct; }
    virtual BigStruct echoBigStruct(const BigStruct& rStruct) override
        { return rStruct; }
    virtual TwoFloats echoTwoFloats(const TwoFloats& rStruct) override
        { return rStruct; }
    virtual FourFloats echoFourFloats(const FourFloats& rStruct) override
        { return rStruct; }
    virtual MixedFloatAndInteger echoMixedFloatAndInteger(const MixedFloatAndInteger& rStruct) override
        { return rStruct; }
    virtual DoubleHyper echoDoubleHyper(DoubleHyper const & s) override { return s; }
    virtual HyperDouble echoHyperDouble(HyperDouble const & s) override { return s; }
    virtual FloatFloatLongByte echoFloatFloatLongByte(FloatFloatLongByte const & s)
        override
    { return s; }
    virtual ThreeByteStruct echoThreeByteStruct(const ThreeByteStruct& rStruct) override
        { return rStruct; }
    virtual sal_Int32 testPPCAlignment( sal_Int64, sal_Int64, sal_Int32, sal_Int64, sal_Int32 i2 ) override
        { return i2; }
    virtual sal_Int32 testPPC64Alignment( double , double , double , sal_Int32 i1 ) override
        { return i1; }
    virtual double testTenDoubles( double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, double d9, double d10 ) override
        { return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10; }
    virtual bool getBool() override
        { return _aData.Bool; }
    virtual sal_Int8 getByte() override
        { return _aData.Byte; }
    virtual sal_Unicode getChar() override
        { return _aData.Char; }
    virtual sal_Int16 getShort() override
        { return _aData.Short; }
    virtual sal_uInt16 getUShort() override
        { return _aData.UShort; }
    virtual sal_Int32 getLong() override
        { return _aData.Long; }
    virtual sal_uInt32 getULong() override
        { return _aData.ULong; }
    virtual sal_Int64 getHyper() override
        { return _aData.Hyper; }
    virtual sal_uInt64 getUHyper() override
        { return _aData.UHyper; }
    virtual float getFloat() override
        { return _aData.Float; }
    virtual double getDouble() override
        { return _aData.Double; }
    virtual TestEnum getEnum() override
        { return _aData.Enum; }
    virtual OUString getString() override
        { return _aData.String; }
    virtual sal_Int8 getByte2() override
        { return _aData.Byte2; }
    virtual sal_Int16 getShort2() override
        { return _aData.Short2; }
    virtual cpo::uno::Reference< cpo::uno::XInterface > getInterface(  ) override
        { return _aData.Interface; }
    virtual cpo::uno::Any getAny() override
        { return _aData.Any; }
    virtual cpo::uno::Sequence< TestElement > getSequence() override
        { return _aData.Sequence; }
    virtual ::test::testtools::bridgetest::TestDataElements getStruct() override
        { return _aStructData; }

    virtual void setBool( bool _bool ) override
        { _aData.Bool = _bool; }
    virtual void setByte( sal_Int8 _byte ) override
        { _aData.Byte = _byte; }
    virtual void setChar( sal_Unicode _char ) override
        { _aData.Char = _char; }
    virtual void setShort( sal_Int16 _short ) override
        { _aData.Short = _short; }
    virtual void setUShort( sal_uInt16 _ushort ) override
        { _aData.UShort = _ushort; }
    virtual void setLong( sal_Int32 _long ) override
        { _aData.Long = _long; }
    virtual void setULong( sal_uInt32 _ulong ) override
        { _aData.ULong = _ulong; }
    virtual void setHyper( sal_Int64 _hyper ) override
        { _aData.Hyper = _hyper; }
    virtual void setUHyper( sal_uInt64 _uhyper ) override
        { _aData.UHyper = _uhyper; }
    virtual void setFloat( float _float ) override
        { _aData.Float = _float; }
    virtual void setDouble( double _double ) override
        { _aData.Double = _double; }
    virtual void setEnum( TestEnum _enum ) override
        { _aData.Enum = _enum; }
    virtual void setString( const OUString& _string ) override
        { _aData.String = _string; }
    virtual void setByte2( sal_Int8 _byte ) override
        { _aData.Byte2 = _byte; }
    virtual void setShort2( sal_Int16 _short ) override
        { _aData.Short2 = _short; }
    virtual void setInterface( const cpo::uno::Reference< cpo::uno::XInterface >& _interface ) override
        { _aData.Interface = _interface; }
    virtual void setAny( const cpo::uno::Any& _any ) override
        { _aData.Any = _any; }
    virtual void setSequence( const cpo::uno::Sequence<TestElement >& _sequence ) override
        { _aData.Sequence = _sequence; }
    virtual void setStruct( const ::test::testtools::bridgetest::TestDataElements& _struct ) override
        { _aStructData = _struct; }

    virtual sal_Int32 getRaiseAttr1() override
    { throw RuntimeException(); }

    virtual void setRaiseAttr1(sal_Int32) override
    { throw IllegalArgumentException(); }

    virtual sal_Int32 getRaiseAttr2() override
    { throw IllegalArgumentException(); }

    virtual TestPolyStruct< bool > transportPolyBoolean(
        TestPolyStruct< bool > const & arg) override
    { return arg; }

    virtual void transportPolyHyper(TestPolyStruct< sal_Int64 > &) override {}

    virtual void transportPolySequence(
        TestPolyStruct< Sequence< Any > > const & arg1,
        TestPolyStruct< Sequence< Any > > & arg2) override
    { arg2 = arg1; }

    virtual TestPolyStruct< sal_Int32 > getNullPolyLong() override
    { return TestPolyStruct< sal_Int32 >(0); /* work around MS compiler bug */ }

    virtual TestPolyStruct< OUString > getNullPolyString() override
    { return TestPolyStruct< OUString >(); }

    virtual TestPolyStruct< Type > getNullPolyType() override
    { return TestPolyStruct< Type >(); }

    virtual TestPolyStruct< Any > getNullPolyAny() override
    { return TestPolyStruct< Any >(); }

    virtual TestPolyStruct< Sequence< bool > >
    getNullPolySequence() override
    { return TestPolyStruct< Sequence< bool > >(); }

    virtual TestPolyStruct< TestEnum > getNullPolyEnum() override
    { return TestPolyStruct< TestEnum >(
        test::testtools::bridgetest::TestEnum_TEST);
          /* work around MS compiler bug */ }

    virtual TestPolyStruct< TestBadEnum > getNullPolyBadEnum() override
    { return TestPolyStruct< TestBadEnum >(
        test::testtools::bridgetest::TestBadEnum_M);
          /* explicitly instantiate with default enumerator */ }

    virtual TestPolyStruct< TestStruct > getNullPolyStruct() override
    { return TestPolyStruct< TestStruct >(); }

    virtual TestPolyStruct< Reference< XBridgeTestBase > >
    getNullPolyInterface() override
    { return TestPolyStruct< Reference< XBridgeTestBase > >(); }

    virtual cpo::uno::Any transportAny(
        const cpo::uno::Any& value ) override;

    virtual void call( sal_Int32 nCallId, sal_Int32 nWaitMUSEC ) override;
    virtual void callOneway( sal_Int32 nCallId, sal_Int32 nWaitMUSEC ) override;
    virtual bool sequenceOfCallTestPassed(  ) override;
    virtual void startRecursiveCall(
        const cpo::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall ) override;

    virtual Reference< XMulti > getMulti() override;

    virtual OUString testMulti(Reference< XMulti > const & multi) override;

public: // XBridgeTest
    virtual ::test::testtools::bridgetest::TestDataElements raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xCOntext ) override;

    virtual void raiseRuntimeExceptionOneway(
        const OUString& Message, const cpo::uno::Reference< cpo::uno::XInterface >& Context ) override;

    virtual sal_Int32 getRuntimeException() override;
    virtual void setRuntimeException( sal_Int32 _runtimeexception ) override;

    // XBridgeTest2
    virtual Sequence< bool > setSequenceBool(
        const Sequence< bool >& aSeq ) override;
    virtual Sequence< sal_Unicode > setSequenceChar(
        const Sequence< sal_Unicode >& aSeq ) override;
    virtual Sequence< sal_Int8 > setSequenceByte(
        const Sequence< sal_Int8 >& aSeq ) override;
    virtual Sequence< sal_Int16 > setSequenceShort(
        const Sequence< sal_Int16 >& aSeq ) override;
    virtual Sequence< sal_uInt16 > setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq ) override;
    virtual Sequence< sal_Int32 > setSequenceLong(
        const Sequence< sal_Int32 >& aSeq ) override;
    virtual Sequence< sal_uInt32 > setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq ) override;
    virtual Sequence< sal_Int64 > setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq ) override;
    virtual Sequence< sal_uInt64 > setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq ) override;
    virtual Sequence< float > setSequenceFloat(
        const Sequence< float >& aSeq ) override;
    virtual Sequence< double > setSequenceDouble(
        const Sequence< double >& aSeq ) override;
    virtual Sequence< TestEnum > setSequenceEnum(
        const Sequence< TestEnum >& aSeq ) override ;
    virtual Sequence< OUString > setSequenceString(
        const Sequence< OUString >& aString ) override;
    virtual Sequence< Reference< XInterface > > setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq ) override;
    virtual Sequence<Any > setSequenceAny(
        const Sequence<Any >& aSeq ) override;
    virtual Sequence<TestElement > setSequenceStruct(
        const Sequence< TestElement >& aSeq ) override;
    virtual Sequence< Sequence< sal_Int32 > > setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq ) override;
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq ) override;
    virtual void setSequencesInOut(Sequence< bool >& aSeqBoolean,
                                Sequence< sal_Unicode >& aSeqChar,
                                Sequence< sal_Int8 >& aSeqByte,
                                Sequence< sal_Int16 >& aSeqShort,
                                Sequence< sal_uInt16 >& aSeqUShort,
                                Sequence< sal_Int32 >& aSeqLong,
                                Sequence< sal_uInt32 >& aSeqULong,
                                Sequence< sal_Int64 >& aSeqHyper,
                                Sequence< sal_uInt64 >& aSeqUHyper,
                                Sequence< float >& aSeqFloat,
                                Sequence< double >& aSeqDouble,
                                Sequence< TestEnum >& aSeqTestEnum,
                                Sequence< OUString >& aSeqString,
                                Sequence<Reference<XInterface > >& aSeqXInterface,
                                Sequence< Any >& aSeqAny,
                                Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                                Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 ) override;
    virtual void setSequencesOut( Sequence< bool >& aSeqBoolean,
                             Sequence< sal_Unicode >& aSeqChar,
                             Sequence< sal_Int8 >& aSeqByte,
                             Sequence< sal_Int16 >& aSeqShort,
                             Sequence< sal_uInt16 >& aSeqUShort,
                             Sequence< sal_Int32 >& aSeqLong,
                             Sequence< sal_uInt32 >& aSeqULong,
                             Sequence< sal_Int64 >& aSeqHyper,
                             Sequence< sal_uInt64 >& aSeqUHyper,
                             Sequence< float >& aSeqFloat,
                             Sequence< double >& aSeqDouble,
                             Sequence< TestEnum >& aSeqEnum,
                             Sequence< OUString >& aSeqString,
                             Sequence< Reference< XInterface > >& aSeqXInterface,
                             Sequence< Any >& aSeqAny,
                             Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                             Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 ) override;
    virtual void testConstructorsService(
        Reference< XComponentContext > const & context) override;
    virtual Reference< XCurrentContextChecker >
    getCurrentContextChecker() override;

public:
    virtual void callRecursivly( const cpo::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall ) override;
};

//Dummy class for XComponent implementation
class Dummy : public osl::DebugBase<Dummy>,
              public WeakComponentImplHelperBase
{
public:
     Dummy(): WeakComponentImplHelperBase(*Mutex::getGlobalMutex()){}

};

}

Any Test_Impl::transportAny( const Any & value )
{
    return value;
}


namespace {

void wait(sal_Int32 microSeconds) {
    OSL_ASSERT(microSeconds >= 0 && microSeconds <= SAL_MAX_INT32 / 1000);
    std::this_thread::sleep_for(std::chrono::microseconds(microSeconds));
}

}

void Test_Impl::call( sal_Int32 nCallId , sal_Int32 nWaitMUSEC )
{
    wait(nWaitMUSEC);
    if( m_bFirstCall )
    {
        m_bFirstCall = false;
    }
    else
    {
        m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed && (nCallId > m_nLastCallId);
    }
    m_nLastCallId = nCallId;
}


void Test_Impl::callOneway( sal_Int32 nCallId , sal_Int32 nWaitMUSEC )
{
    wait(nWaitMUSEC);
    m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed && (nCallId > m_nLastCallId);
    m_nLastCallId = nCallId;
}


bool Test_Impl::sequenceOfCallTestPassed()
{
    return m_bSequenceOfCallTestPassed;
}


void Test_Impl::startRecursiveCall(
    const cpo::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall )
{
    MutexGuard guard( m_mutex );
    if( nToCall )
    {
        nToCall --;
        xCall->callRecursivly( this , nToCall );
    }
}


void Test_Impl::callRecursivly(
    const cpo::uno::Reference< XRecursiveCall >& xCall,
    sal_Int32 nToCall )
{
    MutexGuard guard( m_mutex );
    if( nToCall )
    {
        nToCall --;
        xCall->callRecursivly( this , nToCall );
    }
}

Reference< XMulti > Test_Impl::getMulti() {
    return new testtools::bridgetest::Multi;
}

OUString Test_Impl::testMulti(Reference< XMulti > const & multi)
{
    return testtools::bridgetest::testMulti(multi);
}


void Test_Impl::setValues( bool bBool,
                           sal_Unicode cChar,
                           sal_Int8 nByte,
                           sal_Int16 nShort,
                           sal_uInt16 nUShort,
                           sal_Int32 nLong,
                           sal_uInt32 nULong,
                           sal_Int64 nHyper,
                           sal_uInt64 nUHyper,
                           float fFloat,
                           double fDouble,
                           TestEnum eEnum,
                           const OUString& rStr,
                           sal_Int8 nByte2,
                           sal_Int16 nShort2,
                           const cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                           const cpo::uno::Any& rAny,
                           const cpo::uno::Sequence<TestElement >& rSequence,
                           const ::test::testtools::bridgetest::TestDataElements& rStruct )
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, nByte2, nShort2, xTest, rAny, rSequence );
    _aStructData = rStruct;
}

::test::testtools::bridgetest::TestDataElements Test_Impl::setValues2( bool& bBool,
                                                                       sal_Unicode& cChar,
                                                                       sal_Int8& nByte,
                                                                       sal_Int16& nShort,
                                                                       sal_uInt16& nUShort,
                                                                       sal_Int32& nLong,
                                                                       sal_uInt32& nULong,
                                                                       sal_Int64& nHyper,
                                                                       sal_uInt64& nUHyper,
                                                                       float& fFloat,
                                                                       double& fDouble,
                                                                       TestEnum& eEnum,
                                                                       OUString& rStr,
                                                                       sal_Int8& nByte2,
                                                                       sal_Int16& nShort2,
                                                                       cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                                                                       cpo::uno::Any& rAny,
                                                                       cpo::uno::Sequence<TestElement >& rSequence,
                                                                       ::test::testtools::bridgetest::TestDataElements& rStruct )
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, nByte2, nShort2, xTest, rAny, rSequence );
    _aStructData = rStruct;

    auto pSequence = rSequence.getArray();
    std::swap(pSequence[ 0 ], pSequence[ 1 ]);

    return _aStructData;
}

::test::testtools::bridgetest::TestDataElements Test_Impl::getValues( bool& bBool,
                                                                      sal_Unicode& cChar,
                                                                      sal_Int8& nByte,
                                                                      sal_Int16& nShort,
                                                                      sal_uInt16& nUShort,
                                                                      sal_Int32& nLong,
                                                                      sal_uInt32& nULong,
                                                                      sal_Int64& nHyper,
                                                                      sal_uInt64& nUHyper,
                                                                      float& fFloat,
                                                                      double& fDouble,
                                                                      TestEnum& eEnum,
                                                                      OUString& rStr,
                                                                      sal_Int8& nByte2,
                                                                      sal_Int16& nShort2,
                                                                      cpo::uno::Reference< cpo::uno::XInterface >& xTest,
                                                                      cpo::uno::Any& rAny,
                                                                      cpo::uno::Sequence<TestElement >& rSequence,
                                                                      ::test::testtools::bridgetest::TestDataElements& rStruct )
{
    bBool = _aData.Bool;
    cChar = _aData.Char;
    nByte = _aData.Byte;
    nShort = _aData.Short;
    nUShort = _aData.UShort;
    nLong = _aData.Long;
    nULong = _aData.ULong;
    nHyper = _aData.Hyper;
    nUHyper = _aData.UHyper;
    fFloat = _aData.Float;
    fDouble = _aData.Double;
    eEnum = _aData.Enum;
    rStr = _aData.String;
    nByte2 = _aData.Byte2;
    nShort2 = _aData.Short2;
    xTest = _aData.Interface;
    rAny = _aData.Any;
    rSequence = _aData.Sequence;
    rStruct = _aStructData;
    return _aStructData;
}

::test::testtools::bridgetest::TestDataElements Test_Impl::raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xContext )
{
    _aData.String = rMsg;
    _aData.Interface = xContext;
    throw IllegalArgumentException(rMsg, xContext, nArgumentPos);
}

void Test_Impl::raiseRuntimeExceptionOneway( const OUString & rMsg, const Reference< XInterface > & xContext )
{
    _aData.String = rMsg;
    _aData.Interface = xContext;
    throw RuntimeException(rMsg, xContext);
}

static void dothrow2(const RuntimeException& e)
{
    throw e;
}
static void dothrow(const RuntimeException& e)
{
#if defined _MSC_VER
    // currently only for MSVC:
    // just to test whether all bridges fall back to a RuntimeException
    // in case of a thrown non-UNO exception:
    try
    {
        throw ::std::bad_alloc();
    }
    catch (...)
    {
        try
        {
            Any a( getCaughtException() );
            RuntimeException exc;
            OSL_VERIFY( a >>= exc );
        }
        catch (...) // never throws anything
        {
            fprintf( stderr, "\ngetCaughtException() failed!\n" );
            exit( 1 );
        }
    }
#endif
    dothrow2( e );
}

sal_Int32 Test_Impl::getRuntimeException()
{
    try
    {
        dothrow( RuntimeException( _aData.String, _aData.Interface ) );
    }
    catch (Exception &)
    {
        Any a( getCaughtException() );
        throwException( a );
    }
    return 0; // for dummy
}

void Test_Impl::setRuntimeException( sal_Int32 )
{
    RuntimeException aExc(_aData.String, _aData.Interface);
    throwException( Any( aExc ) );
}

// XBridgeTest2 -------------------------------------------------------------
Sequence< bool > Test_Impl::setSequenceBool(
        const Sequence< bool >& aSeq )
{
    _arBool = aSeq;
    return aSeq;
}

Sequence< sal_Unicode > Test_Impl::setSequenceChar(
        const Sequence< sal_Unicode >& aSeq )
{
    _arChar = aSeq;
    return aSeq;
}

Sequence< sal_Int8 > Test_Impl::setSequenceByte(
        const Sequence< sal_Int8 >& aSeq )
{
    _arByte = aSeq;
    return aSeq;
}

Sequence< sal_Int16 > Test_Impl::setSequenceShort(
        const Sequence< sal_Int16 >& aSeq )
{
    _arShort = aSeq;
    return aSeq;
}

Sequence< sal_uInt16 > Test_Impl::setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq )
{
    _arUShort = aSeq;
    return aSeq;
}

Sequence< sal_Int32 > Test_Impl::setSequenceLong(
        const Sequence< sal_Int32 >& aSeq )
{
    _arLong = aSeq;
    return aSeq;
}

Sequence< sal_uInt32 > Test_Impl::setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq )
{
    _arULong = aSeq;
    return aSeq;
}

Sequence< sal_Int64 > Test_Impl::setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq )
{
    _arHyper = aSeq;
    return aSeq;
}

Sequence< sal_uInt64 > Test_Impl::setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq )
{
    _arUHyper = aSeq;
    return aSeq;
}

Sequence< float > Test_Impl::setSequenceFloat(
        const Sequence< float >& aSeq )
{
    _arFloat = aSeq;
    return aSeq;
}

Sequence< double > Test_Impl::setSequenceDouble(
    const Sequence< double >& aSeq )
{
    _arDouble = aSeq;
    return aSeq;
}

Sequence< TestEnum > Test_Impl::setSequenceEnum(
    const Sequence< TestEnum >& aSeq )
{
    _arEnum = aSeq;
    return aSeq;
}

Sequence< OUString > Test_Impl::setSequenceString(
    const Sequence< OUString >& aSeq )
{
    _arString = aSeq;
    return aSeq;
}

Sequence< Reference< XInterface > > Test_Impl::setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq )
{
    _arObject = aSeq;
    return aSeq;
}

Sequence<Any > Test_Impl::setSequenceAny(
    const Sequence<Any >& aSeq )
{
    _arAny = aSeq;
    return aSeq;
}

Sequence<TestElement > Test_Impl::setSequenceStruct(
    const Sequence< TestElement >& aSeq )
{
    _arStruct = aSeq;
    return aSeq;
}

Sequence< Sequence< sal_Int32 > > Test_Impl::setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq )
{
    _arLong2 = aSeq;
    return aSeq;
}

Sequence< Sequence< Sequence< sal_Int32 > > > Test_Impl::setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq )
{
    _arLong3 = aSeq;
    return aSeq;
}

void Test_Impl::setSequencesInOut(Sequence< bool >& aSeqBoolean,
                                Sequence< sal_Unicode >& aSeqChar,
                                Sequence< sal_Int8 >& aSeqByte,
                                Sequence< sal_Int16 >& aSeqShort,
                                Sequence< sal_uInt16 >& aSeqUShort,
                                Sequence< sal_Int32 >& aSeqLong,
                                Sequence< sal_uInt32 >& aSeqULong,
                                Sequence< sal_Int64 >& aSeqHyper,
                                Sequence< sal_uInt64 >& aSeqUHyper,
                                Sequence< float >& aSeqFloat,
                                Sequence< double >& aSeqDouble,
                                Sequence< TestEnum >& aSeqTestEnum,
                                Sequence< OUString >& aSeqString,
                                Sequence<Reference<XInterface > >& aSeqXInterface,
                                Sequence< Any >& aSeqAny,
                                Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                                Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
{
    _arBool = aSeqBoolean;
    _arChar = aSeqChar;
    _arByte = aSeqByte;
    _arShort = aSeqShort;
    _arUShort = aSeqUShort;
    _arLong = aSeqLong;
    _arULong = aSeqULong;
    _arHyper  = aSeqHyper;
    _arUHyper = aSeqUHyper;
    _arFloat = aSeqFloat;
    _arDouble = aSeqDouble;
    _arEnum = aSeqTestEnum;
    _arString = aSeqString;
    _arObject = aSeqXInterface;
    _arAny = aSeqAny;
    _arLong2 = aSeqDim2;
    _arLong3 = aSeqDim3;
}

void Test_Impl::setSequencesOut( Sequence< bool >& aSeqBoolean,
                             Sequence< sal_Unicode >& aSeqChar,
                             Sequence< sal_Int8 >& aSeqByte,
                             Sequence< sal_Int16 >& aSeqShort,
                             Sequence< sal_uInt16 >& aSeqUShort,
                             Sequence< sal_Int32 >& aSeqLong,
                             Sequence< sal_uInt32 >& aSeqULong,
                             Sequence< sal_Int64 >& aSeqHyper,
                             Sequence< sal_uInt64 >& aSeqUHyper,
                             Sequence< float >& aSeqFloat,
                             Sequence< double >& aSeqDouble,
                             Sequence< TestEnum >& aSeqEnum,
                             Sequence< OUString >& aSeqString,
                             Sequence< Reference< XInterface > >& aSeqXInterface,
                             Sequence< Any >& aSeqAny,
                             Sequence< Sequence< sal_Int32 > >& aSeqDim2,
                             Sequence< Sequence< Sequence< sal_Int32 > > >& aSeqDim3 )
{
    aSeqBoolean = _arBool;
    aSeqChar = _arChar;
    aSeqByte = _arByte;
    aSeqShort = _arShort;
    aSeqUShort = _arUShort;
    aSeqLong = _arLong;
    aSeqULong = _arULong;
    aSeqHyper = _arHyper;
    aSeqUHyper = _arUHyper;
    aSeqFloat = _arFloat;
    aSeqDouble = _arDouble;
    aSeqEnum = _arEnum;
    aSeqString = _arString;
    aSeqXInterface = _arObject;
    aSeqAny = _arAny;
    aSeqDim2 = _arLong2;
    aSeqDim3 = _arLong3;
}

void Test_Impl::testConstructorsService(
    Reference< XComponentContext > const & context)
{
    Sequence< bool > arg14{ true };
    Sequence< sal_Int8 > arg15{ SAL_MIN_INT8 };
    Sequence< sal_Int16 > arg16{ SAL_MIN_INT16 };
    Sequence< sal_uInt16 > arg17{ SAL_MAX_UINT16 };
    Sequence< sal_Int32 > arg18{ SAL_MIN_INT32 };
    Sequence< sal_uInt32 > arg19{ SAL_MAX_UINT32 };
    Sequence< sal_Int64 > arg20{ SAL_MIN_INT64 };
    Sequence< sal_uInt64 > arg21{ SAL_MAX_UINT64 };
    Sequence< float > arg22{ 0.123f };
    Sequence< double > arg23{ 0.456 };
    Sequence< sal_Unicode > arg24{ 'X' };
    Sequence< OUString > arg25 { u"test"_ustr };
    Sequence< Type > arg26{ UnoType< Any >::get() };
    Sequence< Any > arg27{ Any(true) };
    Sequence< Sequence< bool > > arg28{ { true } };
    Sequence< Sequence< Any > > arg29{ { Any(true) } };
    Sequence< TestEnum > arg30{ TestEnum_TWO };
    Sequence< TestStruct > arg31(1); arg31.getArray()[0].member = 10;
    Sequence< TestPolyStruct< bool > > arg32{ { true } };
    Sequence< TestPolyStruct< Any > > arg33(1); arg33.getArray()[0].member <<= true;
    Sequence< Reference< XInterface > > arg34(1);
    Constructors::create1(context,
        true,
        SAL_MIN_INT8,
        SAL_MIN_INT16,
        SAL_MAX_UINT16,
        SAL_MIN_INT32,
        SAL_MAX_UINT32,
        SAL_MIN_INT64,
        SAL_MAX_UINT64,
        0.123f,
        0.456,
        'X',
        u"test"_ustr,
        UnoType< Any >::get(),
        Any(true),
        arg14,
        arg15,
        arg16,
        arg17,
        arg18,
        arg19,
        arg20,
        arg21,
        arg22,
        arg23,
        arg24,
        arg25,
        arg26,
        arg27,
        arg28,
        arg29,
        arg30,
        arg31,
        arg32,
        arg33,
        arg34,
        TestEnum_TWO,
        TestStruct(10),
        TestPolyStruct< bool >(true),
        TestPolyStruct< Any >(Any(true)),
        Reference< XInterface >(nullptr));
    Sequence< Any > args{
        Any(true),
        Any(SAL_MIN_INT8),
        Any(SAL_MIN_INT16),
        Any(SAL_MAX_UINT16),
        Any(SAL_MIN_INT32),
        Any(SAL_MAX_UINT32),
        Any(SAL_MIN_INT64),
        Any(SAL_MAX_UINT64),
        Any(0.123f),
        Any(0.456),
        Any(u'X'),
        Any(u"test"_ustr),
        Any(UnoType< Any >::get()),
        Any(true),
        Any(arg14),
        Any(arg15),
        Any(arg16),
        Any(arg17),
        Any(arg18),
        Any(arg19),
        Any(arg20),
        Any(arg21),
        Any(arg22),
        Any(arg23),
        Any(arg24),
        Any(arg25),
        Any(arg26),
        Any(arg27),
        Any(arg28),
        Any(arg29),
        Any(arg30),
        Any(arg31),
        Any(arg32),
        Any(arg33),
        Any(arg34),
        Any(TestEnum_TWO),
        Any(TestStruct(10)),
        Any(TestPolyStruct< bool >(true)),
        Any(TestPolyStruct< Any >(Any(true))),
        Any(Reference< XInterface >(nullptr))
    };
    assert(args.getLength() == 40);
    Constructors::create2(context, args);

    Sequence<Type> argSeq1{ cppu::UnoType<sal_Int32>::get() };
    Sequence<Reference<XInterface> > argSeq2 { static_cast<XComponent*>(new Dummy()) };
    Sequence<Reference<XComponent> > argSeq2a { static_cast<XComponent*>(new Dummy()) };

    Sequence<TestPolyStruct2<sal_Unicode, Sequence<Any> > > argSeq3
        { TestPolyStruct2<sal_Unicode, Sequence<Any> >('X', arg27) };
    Sequence<TestPolyStruct2<TestPolyStruct<sal_Unicode>, Sequence<Any> > > argSeq4
        { TestPolyStruct2<TestPolyStruct<sal_Unicode>, Sequence<Any> >(
            TestPolyStruct<sal_Unicode>('X'), arg27) };
    Sequence<Sequence<sal_Int32> > argSeq5{ { SAL_MIN_INT32 } };
    Sequence<TestPolyStruct<sal_Int32> > argSeq6{ TestPolyStruct<sal_Int32>(SAL_MIN_INT32) };
    Sequence<TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> > > argSeq7
        { TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true))) };
    Sequence<TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > > argSeq8
        { TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > (
            TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString>(
                TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), u"test"_ustr)) };
    Sequence<TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > > > argSeq9
        { TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > >(
            u"test"_ustr, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >(
                                  'X', TestPolyStruct<Any>(Any(true)))) };
    Sequence<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> > > argSeq10
        { TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), TestPolyStruct<sal_Unicode>('X')) };
    Sequence<Sequence<TestPolyStruct<sal_Unicode > > > argSeq11
        { { TestPolyStruct<sal_Unicode>('X') } };
    Sequence<Sequence<TestPolyStruct<TestPolyStruct2<sal_Unicode,Any> > > > argSeq12
        { { TestPolyStruct<TestPolyStruct2<sal_Unicode,Any> >(
            TestPolyStruct2<sal_Unicode,Any>('X', Any(true))) } };
    Sequence<Sequence<TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString> > > > argSeq13
        { {TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString> >(
            TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString>(
                TestPolyStruct2<sal_Unicode,Any>('X', Any(true)), u"test"_ustr))} };
    Sequence<Sequence<TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > > > > argSeq14
        { { TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > >(
            u"test"_ustr, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >(
                'X', TestPolyStruct<Any>(Any(true)))) } };
    Sequence<Sequence<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>, TestPolyStruct<sal_Unicode> > > > argSeq15
        { { TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>, TestPolyStruct<sal_Unicode> >(
            TestPolyStruct2<sal_Unicode,Any>('X',Any(true)), TestPolyStruct<sal_Unicode>('X')) } };

    Constructors2::create1(
        context,
        TestPolyStruct<Type>(cppu::UnoType<sal_Int32>::get()),
        TestPolyStruct<Any>(Any(true)),
        TestPolyStruct<bool>(true),
        TestPolyStruct<sal_Int8>(SAL_MIN_INT8),
        TestPolyStruct<sal_Int16>(SAL_MIN_INT16),
        TestPolyStruct<sal_Int32>(SAL_MIN_INT32),
        TestPolyStruct<sal_Int64>(SAL_MIN_INT64),
        TestPolyStruct<sal_Unicode>('X'),
        TestPolyStruct<OUString>(u"test"_ustr),
        TestPolyStruct<float>(0.123f),
        TestPolyStruct<double>(0.456),
        TestPolyStruct<Reference<XInterface> >(static_cast<XBridgeTest2*>(this)),
        TestPolyStruct<Reference<XComponent> >(static_cast<XComponent*>(new Dummy())),
        TestPolyStruct<TestEnum>(TestEnum_TWO),
        TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true))),
        TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > (
            TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString>(
                TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), u"test"_ustr)),
        TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode,TestPolyStruct<Any> > >(
            u"test"_ustr,
            TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >('X', TestPolyStruct<Any>(Any(true)))),
        TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true)),
            TestPolyStruct<sal_Unicode>('X')),
        TestPolyStruct<Sequence<Type> >(argSeq1),
        TestPolyStruct<Sequence<Any> >(arg27),
        TestPolyStruct<Sequence<bool> >(arg14),
        TestPolyStruct<Sequence<sal_Int8> >(arg15),
        TestPolyStruct<Sequence<sal_Int16> >(arg16),
        TestPolyStruct<Sequence<sal_Int32> >(arg18),
        TestPolyStruct<Sequence<sal_Int64> >(arg20),
        TestPolyStruct<Sequence<sal_Unicode> >(arg24),
        TestPolyStruct<Sequence<OUString> >(arg25),
        TestPolyStruct<Sequence<float> >(arg22),
        TestPolyStruct<Sequence<double> >(arg23),
        TestPolyStruct<Sequence<Reference<XInterface> > >(argSeq2),
        TestPolyStruct<Sequence<Reference<XComponent> > >(argSeq2a),
        TestPolyStruct<Sequence<TestEnum> >(arg30),
        TestPolyStruct<Sequence<TestPolyStruct2<sal_Unicode, Sequence<Any> > > >(argSeq3),
        TestPolyStruct<Sequence<TestPolyStruct2<TestPolyStruct<sal_Unicode>, Sequence<Any> > > > (argSeq4),
        TestPolyStruct<Sequence<Sequence<sal_Int32> > >(argSeq5),
        argSeq6,
        argSeq7,
        argSeq8,
        argSeq9,
        argSeq10,
        argSeq11,
        argSeq12,
        argSeq13,
        argSeq14,
        argSeq15);
}

Reference< XCurrentContextChecker > Test_Impl::getCurrentContextChecker()
{
    return new testtools::bridgetest::CurrentContextChecker;
}

// XServiceInfo

OUString Test_Impl::getImplementationName()
{
    return IMPLNAME;
}

bool Test_Impl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > Test_Impl::getSupportedServiceNames()
{
    return bridge_object::getSupportedServiceNames();
}


static Reference< XInterface > Test_Impl_create(
    SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
{
    return Reference< XInterface >( static_cast<XBridgeTest *>(new Test_Impl()) );
}

}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * component_getFactory(
    const char * pImplName, SAL_UNUSED_PARAMETER void * pServiceManager,
    SAL_UNUSED_PARAMETER void * )
{
    void * pRet = nullptr;

    if (pServiceManager && o3tl::equalsAscii(IMPLNAME, pImplName))
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            static_cast< XMultiServiceFactory * >( pServiceManager ),
            IMPLNAME,
            bridge_object::Test_Impl_create,
            bridge_object::getSupportedServiceNames() ) );

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
