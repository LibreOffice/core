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

#include "cppu/unotype.hxx"
#include <osl/diagnose.h>
#include "osl/diagnose.hxx"
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <osl/time.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/factory.hxx>
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/compbase_ex.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include "test/testtools/bridgetest/Constructors.hpp"
#include "test/testtools/bridgetest/Constructors2.hpp"
#include "test/testtools/bridgetest/TestPolyStruct.hpp"
#include "test/testtools/bridgetest/TestPolyStruct2.hpp"
#include "test/testtools/bridgetest/XBridgeTest2.hpp"
#include "test/testtools/bridgetest/XMulti.hpp"

#include "currentcontextchecker.hxx"
#include "multi.hxx"

using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace test::testtools::bridgetest;


#define SERVICENAME     "com.sun.star.test.bridge.CppTestObject"
#define IMPLNAME        "com.sun.star.comp.bridge.CppTestObject"

namespace bridge_object
{


inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( SERVICENAME );
    return Sequence< OUString >( &aName, 1 );
}


static void assign( TestElement & rData,
                    bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const OUString& rStr,
                    const css::uno::Reference< css::uno::XInterface >& xTest,
                    const css::uno::Any& rAny )
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

static void assign( TestData & rData,
                    bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                    sal_Int16 nShort, sal_uInt16 nUShort,
                    sal_Int32 nLong, sal_uInt32 nULong,
                    sal_Int64 nHyper, sal_uInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, const OUString& rStr,
                    const css::uno::Reference< css::uno::XInterface >& xTest,
                    const css::uno::Any& rAny,
                    const css::uno::Sequence< TestElement >& rSequence )
{
    assign( (TestElement &)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny );
    rData.Sequence = rSequence;
}


class Test_Impl :
    public osl::DebugBase<Test_Impl>,
    public WeakImplHelper< XBridgeTest2, XServiceInfo , XRecursiveCall >
{
    TestData _aData, _aStructData;
    sal_Int32 m_nLastCallId;
    bool m_bFirstCall;
    bool m_bSequenceOfCallTestPassed;
    Mutex m_mutex;

    Sequence<sal_Bool> _arBool;
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
    virtual ~Test_Impl()
        {
            OSL_TRACE( "> scalar Test_Impl dtor <" );
        }

    void SAL_CALL acquire() throw () override
    {
        OWeakObject::acquire();
    }
    void SAL_CALL release() throw () override
    {
        OWeakObject::release();
     }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException, std::exception) override;

    // XLBTestBase
    virtual void SAL_CALL setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                                     sal_Int16 nShort, sal_uInt16 nUShort,
                                     sal_Int32 nLong, sal_uInt32 nULong,
                                     sal_Int64 nHyper, sal_uInt64 nUHyper,
                                     float fFloat, double fDouble,
                                     TestEnum eEnum, const OUString& rStr,
                                     const css::uno::Reference< css::uno::XInterface >& xTest,
                                     const css::uno::Any& rAny,
                                     const css::uno::Sequence<TestElement >& rSequence,
                                     const TestData& rStruct )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual TestData SAL_CALL setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                                sal_Int16& nShort, sal_uInt16& nUShort,
                                                sal_Int32& nLong, sal_uInt32& nULong,
                                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                                float& fFloat, double& fDouble,
                                                TestEnum& eEnum, OUString& rStr,
                                                css::uno::Reference< css::uno::XInterface >& xTest,
                                                css::uno::Any& rAny,
                                                css::uno::Sequence<TestElement >& rSequence,
                                                TestData& rStruct )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual TestData SAL_CALL getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                               sal_Int16& nShort, sal_uInt16& nUShort,
                                               sal_Int32& nLong, sal_uInt32& nULong,
                                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                                               float& fFloat, double& fDouble,
                                               TestEnum& eEnum, OUString& rStr,
                                               css::uno::Reference< css::uno::XInterface >& xTest,
                                               css::uno::Any& rAny,
                                               css::uno::Sequence< TestElement >& rSequence,
                                               TestData& rStruct )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual SmallStruct SAL_CALL echoSmallStruct(const SmallStruct& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual MediumStruct SAL_CALL echoMediumStruct(const MediumStruct& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual BigStruct SAL_CALL echoBigStruct(const BigStruct& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual TwoFloats SAL_CALL echoTwoFloats(const TwoFloats& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual FourFloats SAL_CALL echoFourFloats(const FourFloats& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual MixedFloatAndInteger SAL_CALL echoMixedFloatAndInteger(const MixedFloatAndInteger& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual ThreeByteStruct SAL_CALL echoThreeByteStruct(const ThreeByteStruct& rStruct) throw(css::uno::RuntimeException, std::exception) override
        { return rStruct; }
    virtual sal_Int32 SAL_CALL testPPCAlignment( sal_Int64, sal_Int64, sal_Int32, sal_Int64, sal_Int32 i2 ) throw(css::uno::RuntimeException, std::exception) override
        { return i2; }
    virtual sal_Int32 SAL_CALL testPPC64Alignment( double , double , double , sal_Int32 i1 ) throw(css::uno::RuntimeException, std::exception) override
        { return i1; }
    virtual double SAL_CALL testTenDoubles( double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, double d9, double d10 ) throw(css::uno::RuntimeException, std::exception) override
        { return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10; }
    virtual sal_Bool SAL_CALL getBool() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Bool; }
    virtual sal_Int8 SAL_CALL getByte() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Byte; }
    virtual sal_Unicode SAL_CALL getChar() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Char; }
    virtual sal_Int16 SAL_CALL getShort() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Short; }
    virtual sal_uInt16 SAL_CALL getUShort() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.UShort; }
    virtual sal_Int32 SAL_CALL getLong() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Long; }
    virtual sal_uInt32 SAL_CALL getULong() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.ULong; }
    virtual sal_Int64 SAL_CALL getHyper() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Hyper; }
    virtual sal_uInt64 SAL_CALL getUHyper() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.UHyper; }
    virtual float SAL_CALL getFloat() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Float; }
    virtual double SAL_CALL getDouble() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Double; }
    virtual TestEnum SAL_CALL getEnum() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Enum; }
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.String; }
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getInterface(  ) throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Interface; }
    virtual css::uno::Any SAL_CALL getAny() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Any; }
    virtual css::uno::Sequence< TestElement > SAL_CALL getSequence() throw(css::uno::RuntimeException, std::exception) override
        { return _aData.Sequence; }
    virtual TestData SAL_CALL getStruct() throw(css::uno::RuntimeException, std::exception) override
        { return _aStructData; }

    virtual void SAL_CALL setBool( sal_Bool _bool ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Bool = _bool; }
    virtual void SAL_CALL setByte( sal_Int8 _byte ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Byte = _byte; }
    virtual void SAL_CALL setChar( sal_Unicode _char ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Char = _char; }
    virtual void SAL_CALL setShort( sal_Int16 _short ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Short = _short; }
    virtual void SAL_CALL setUShort( sal_uInt16 _ushort ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.UShort = _ushort; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Long = _long; }
    virtual void SAL_CALL setULong( sal_uInt32 _ulong ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.ULong = _ulong; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Hyper = _hyper; }
    virtual void SAL_CALL setUHyper( sal_uInt64 _uhyper ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.UHyper = _uhyper; }
    virtual void SAL_CALL setFloat( float _float ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Float = _float; }
    virtual void SAL_CALL setDouble( double _double ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Double = _double; }
    virtual void SAL_CALL setEnum( TestEnum _enum ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Enum = _enum; }
    virtual void SAL_CALL setString( const OUString& _string ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.String = _string; }
    virtual void SAL_CALL setInterface( const css::uno::Reference< css::uno::XInterface >& _interface ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Interface = _interface; }
    virtual void SAL_CALL setAny( const css::uno::Any& _any ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Any = _any; }
    virtual void SAL_CALL setSequence( const css::uno::Sequence<TestElement >& _sequence ) throw(css::uno::RuntimeException, std::exception) override
        { _aData.Sequence = _sequence; }
    virtual void SAL_CALL setStruct( const TestData& _struct ) throw(css::uno::RuntimeException, std::exception) override
        { _aStructData = _struct; }

    virtual sal_Int32 SAL_CALL getRaiseAttr1() throw (RuntimeException, std::exception) override
    { throw RuntimeException(); }

    virtual void SAL_CALL setRaiseAttr1(sal_Int32)
        throw (IllegalArgumentException, RuntimeException, std::exception) override
    { throw IllegalArgumentException(); }

    virtual sal_Int32 SAL_CALL getRaiseAttr2()
        throw (IllegalArgumentException, RuntimeException, std::exception) override
    { throw IllegalArgumentException(); }

    virtual TestPolyStruct< sal_Bool > SAL_CALL transportPolyBoolean(
        TestPolyStruct< sal_Bool > const & arg) throw (RuntimeException, std::exception) override
    { return arg; }

    virtual void SAL_CALL transportPolyHyper(TestPolyStruct< sal_Int64 > &)
        throw (RuntimeException, std::exception) override {}

    virtual void SAL_CALL transportPolySequence(
        TestPolyStruct< Sequence< Any > > const & arg1,
        TestPolyStruct< Sequence< Any > > & arg2) throw (RuntimeException, std::exception) override
    { arg2 = arg1; }

    virtual TestPolyStruct< sal_Int32 > SAL_CALL getNullPolyLong()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< sal_Int32 >(0); /* work around MS compiler bug */ }

    virtual TestPolyStruct< OUString > SAL_CALL getNullPolyString()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< OUString >(); }

    virtual TestPolyStruct< Type > SAL_CALL getNullPolyType()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< Type >(); }

    virtual TestPolyStruct< Any > SAL_CALL getNullPolyAny()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< Any >(); }

    virtual TestPolyStruct< Sequence< sal_Bool > > SAL_CALL
    getNullPolySequence() throw (RuntimeException, std::exception) override
    { return TestPolyStruct< Sequence< sal_Bool > >(); }

    virtual TestPolyStruct< TestEnum > SAL_CALL getNullPolyEnum()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< TestEnum >(
        test::testtools::bridgetest::TestEnum_TEST);
          /* work around MS compiler bug */ }

    virtual TestPolyStruct< TestBadEnum > SAL_CALL getNullPolyBadEnum()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< TestBadEnum >(
        test::testtools::bridgetest::TestBadEnum_M);
          /* explicitly instantiate with default enumerator */ }

    virtual TestPolyStruct< TestStruct > SAL_CALL getNullPolyStruct()
        throw (RuntimeException, std::exception) override
    { return TestPolyStruct< TestStruct >(); }

    virtual TestPolyStruct< Reference< XBridgeTestBase > > SAL_CALL
    getNullPolyInterface() throw (RuntimeException, std::exception) override
    { return TestPolyStruct< Reference< XBridgeTestBase > >(); }

    virtual css::uno::Any SAL_CALL transportAny(
        const css::uno::Any& value )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL call( sal_Int32 nCallId, sal_Int32 nWaitMUSEC )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL callOneway( sal_Int32 nCallId, sal_Int32 nWaitMUSEC )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL sequenceOfCallTestPassed(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startRecursiveCall(
        const css::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual Reference< XMulti > SAL_CALL getMulti() throw (RuntimeException, std::exception) override;

    virtual OUString SAL_CALL testMulti(Reference< XMulti > const & multi)
        throw (RuntimeException, std::exception) override;

public: // XBridgeTest
    virtual TestData SAL_CALL raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xCOntext )
        throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL raiseRuntimeExceptionOneway(
        const OUString& Message, const css::uno::Reference< css::uno::XInterface >& Context )
        throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getRuntimeException() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRuntimeException( sal_Int32 _runtimeexception ) throw(css::uno::RuntimeException, std::exception) override;

    // XBridgeTest2
    virtual Sequence< sal_Bool > SAL_CALL setSequenceBool(
        const Sequence< sal_Bool >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Unicode > SAL_CALL setSequenceChar(
        const Sequence< sal_Unicode >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Int8 > SAL_CALL setSequenceByte(
        const Sequence< sal_Int8 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Int16 > SAL_CALL setSequenceShort(
        const Sequence< sal_Int16 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_uInt16 > SAL_CALL setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Int32 > SAL_CALL setSequenceLong(
        const Sequence< sal_Int32 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_uInt32 > SAL_CALL setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_Int64 > SAL_CALL setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< sal_uInt64 > SAL_CALL setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< float > SAL_CALL setSequenceFloat(
        const Sequence< float >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< double > SAL_CALL setSequenceDouble(
        const Sequence< double >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< TestEnum > SAL_CALL setSequenceEnum(
        const Sequence< TestEnum >& aSeq ) throw (RuntimeException, std::exception) override ;
    virtual Sequence< OUString > SAL_CALL setSequenceString(
        const Sequence< OUString >& aString ) throw (RuntimeException, std::exception) override;
    virtual Sequence< Reference< XInterface > > SAL_CALL setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq )
        throw (RuntimeException, std::exception) override;
    virtual Sequence<Any > SAL_CALL setSequenceAny(
        const Sequence<Any >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence<TestElement > SAL_CALL setSequenceStruct(
        const Sequence< TestElement >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< Sequence< sal_Int32 > > SAL_CALL setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq ) throw (RuntimeException, std::exception) override;
    virtual Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq )
        throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSequencesInOut(Sequence< sal_Bool >& aSeqBoolean,
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
        throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setSequencesOut( Sequence< sal_Bool >& aSeqBoolean,
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
        throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL testConstructorsService(
        Reference< XComponentContext > const & context)
        throw (RuntimeException, std::exception) override;
    virtual Reference< XCurrentContextChecker > SAL_CALL
    getCurrentContextChecker()
        throw (RuntimeException, std::exception) override;

public:
    virtual void SAL_CALL callRecursivly( const css::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall ) throw(css::uno::RuntimeException, std::exception) override;
};

//Dummy class for XComponent implementation
class Dummy : public osl::DebugBase<Dummy>,
              public WeakComponentImplHelperBase
{
public:
     Dummy(): WeakComponentImplHelperBase(*Mutex::getGlobalMutex()){}

};

Any Test_Impl::transportAny( const Any & value ) throw ( css::uno::RuntimeException, std::exception)
{
    return value;
}



namespace {

void wait(sal_Int32 microSeconds) {
    OSL_ASSERT(microSeconds >= 0 && microSeconds <= SAL_MAX_INT32 / 1000);
    TimeValue t = {
        static_cast< sal_uInt32 >(microSeconds / 1000000),
        static_cast< sal_uInt32 >(microSeconds * 1000) };
    osl_waitThread(&t);
}

}

void Test_Impl::call( sal_Int32 nCallId , sal_Int32 nWaitMUSEC ) throw(css::uno::RuntimeException, std::exception)
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


void Test_Impl::callOneway( sal_Int32 nCallId , sal_Int32 nWaitMUSEC ) throw (css::uno::RuntimeException, std::exception)
{
    wait(nWaitMUSEC);
    m_bSequenceOfCallTestPassed = m_bSequenceOfCallTestPassed && (nCallId > m_nLastCallId);
    m_nLastCallId = nCallId;
}


sal_Bool Test_Impl::sequenceOfCallTestPassed() throw (css::uno::RuntimeException, std::exception)
{
    return m_bSequenceOfCallTestPassed;
}


void SAL_CALL Test_Impl::startRecursiveCall(
    const css::uno::Reference< XRecursiveCall >& xCall, sal_Int32 nToCall )
    throw(css::uno::RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    if( nToCall )
    {
        nToCall --;
        xCall->callRecursivly( this , nToCall );
    }
}


void SAL_CALL Test_Impl::callRecursivly(
    const css::uno::Reference< XRecursiveCall >& xCall,
    sal_Int32 nToCall )
    throw(css::uno::RuntimeException, std::exception)
{
    MutexGuard guard( m_mutex );
    if( nToCall )
    {
        nToCall --;
        xCall->callRecursivly( this , nToCall );
    }
}

Reference< XMulti > Test_Impl::getMulti() throw (RuntimeException, std::exception) {
    return new testtools::bridgetest::Multi;
}

OUString Test_Impl::testMulti(Reference< XMulti > const & multi)
    throw (RuntimeException, std::exception)
{
    return testtools::bridgetest::testMulti(multi);
}


void Test_Impl::setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                           sal_Int16 nShort, sal_uInt16 nUShort,
                           sal_Int32 nLong, sal_uInt32 nULong,
                           sal_Int64 nHyper, sal_uInt64 nUHyper,
                           float fFloat, double fDouble,
                           TestEnum eEnum, const OUString& rStr,
                           const css::uno::Reference< css::uno::XInterface >& xTest,
                           const css::uno::Any& rAny,
                           const css::uno::Sequence<TestElement >& rSequence,
                           const TestData& rStruct )
    throw(css::uno::RuntimeException, std::exception)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
}

TestData Test_Impl::setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                sal_Int16& nShort, sal_uInt16& nUShort,
                                sal_Int32& nLong, sal_uInt32& nULong,
                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                float& fFloat, double& fDouble,
                                TestEnum& eEnum, OUString& rStr,
                                css::uno::Reference< css::uno::XInterface >& xTest,
                                css::uno::Any& rAny,
                                css::uno::Sequence<TestElement >& rSequence,
                                TestData& rStruct )
    throw(css::uno::RuntimeException, std::exception)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;

    TestElement elem = rSequence[ 0 ];
    rSequence[ 0 ] = rSequence[ 1 ];
    rSequence[ 1 ] = elem;

    return _aStructData;
}

TestData Test_Impl::getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                               sal_Int16& nShort, sal_uInt16& nUShort,
                               sal_Int32& nLong, sal_uInt32& nULong,
                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                               float& fFloat, double& fDouble,
                               TestEnum& eEnum, OUString& rStr,
                               css::uno::Reference< css::uno::XInterface >& xTest,
                               css::uno::Any& rAny,
                               css::uno::Sequence<TestElement >& rSequence,
                               TestData& rStruct )
    throw(css::uno::RuntimeException, std::exception)
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
    xTest = _aData.Interface;
    rAny = _aData.Any;
    rSequence = _aData.Sequence;
    rStruct = _aStructData;
    return _aStructData;
}

TestData Test_Impl::raiseException( sal_Int16 nArgumentPos, const OUString & rMsg, const Reference< XInterface > & xContext )
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    IllegalArgumentException aExc;
    aExc.ArgumentPosition = nArgumentPos;
    aExc.Message          = _aData.String = rMsg;
    aExc.Context          = _aData.Interface = xContext;
    throw aExc;
}

void Test_Impl::raiseRuntimeExceptionOneway( const OUString & rMsg, const Reference< XInterface > & xContext )
    throw(css::uno::RuntimeException, std::exception)
{
    RuntimeException aExc;
    aExc.Message          = _aData.String = rMsg;
    aExc.Context          = _aData.Interface = xContext;
    throw aExc;
}

void dothrow2(const RuntimeException& e)
{
    throw e;
}
void dothrow(const RuntimeException& e)
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
            OSL_TRACE(
                OUStringToOString(
                    exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
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
    throw (RuntimeException, std::exception)
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

void Test_Impl::setRuntimeException( sal_Int32 ) throw(css::uno::RuntimeException, std::exception)
{
    RuntimeException aExc;
    aExc.Message          = _aData.String;
    aExc.Context          = _aData.Interface;
    throwException( makeAny( aExc ) );
}

// XBridgeTest2 -------------------------------------------------------------
Sequence< sal_Bool > SAL_CALL Test_Impl::setSequenceBool(
        const Sequence< sal_Bool >& aSeq ) throw (RuntimeException, std::exception)
{
    _arBool = aSeq;
    return aSeq;
}

Sequence< sal_Unicode > SAL_CALL Test_Impl::setSequenceChar(
        const Sequence< sal_Unicode >& aSeq ) throw (RuntimeException, std::exception)
{
    _arChar = aSeq;
    return aSeq;
}

Sequence< sal_Int8 > SAL_CALL Test_Impl::setSequenceByte(
        const Sequence< sal_Int8 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arByte = aSeq;
    return aSeq;
}

Sequence< sal_Int16 > SAL_CALL Test_Impl::setSequenceShort(
        const Sequence< sal_Int16 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arShort = aSeq;
    return aSeq;
}

Sequence< sal_uInt16 > SAL_CALL Test_Impl::setSequenceUShort(
        const Sequence< sal_uInt16 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arUShort = aSeq;
    return aSeq;
}

Sequence< sal_Int32 > SAL_CALL Test_Impl::setSequenceLong(
        const Sequence< sal_Int32 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arLong = aSeq;
    return aSeq;
}

Sequence< sal_uInt32 > SAL_CALL Test_Impl::setSequenceULong(
        const Sequence< sal_uInt32 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arULong = aSeq;
    return aSeq;
}

Sequence< sal_Int64 > SAL_CALL Test_Impl::setSequenceHyper(
        const Sequence< sal_Int64 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arHyper = aSeq;
    return aSeq;
}

Sequence< sal_uInt64 > SAL_CALL Test_Impl::setSequenceUHyper(
        const Sequence< sal_uInt64 >& aSeq ) throw (RuntimeException, std::exception)
{
    _arUHyper = aSeq;
    return aSeq;
}

Sequence< float > SAL_CALL Test_Impl::setSequenceFloat(
        const Sequence< float >& aSeq ) throw (RuntimeException, std::exception)
{
    _arFloat = aSeq;
    return aSeq;
}

Sequence< double > SAL_CALL Test_Impl::setSequenceDouble(
    const Sequence< double >& aSeq ) throw (RuntimeException, std::exception)
{
    _arDouble = aSeq;
    return aSeq;
}

Sequence< TestEnum > SAL_CALL Test_Impl::setSequenceEnum(
    const Sequence< TestEnum >& aSeq ) throw (RuntimeException, std::exception)
{
    _arEnum = aSeq;
    return aSeq;
}

Sequence< OUString > SAL_CALL Test_Impl::setSequenceString(
    const Sequence< OUString >& aSeq ) throw (RuntimeException, std::exception)
{
    _arString = aSeq;
    return aSeq;
}

Sequence< Reference< XInterface > > SAL_CALL Test_Impl::setSequenceXInterface(
        const Sequence< Reference< XInterface > >& aSeq )
        throw (RuntimeException, std::exception)
{
    _arObject = aSeq;
    return aSeq;
}

Sequence<Any > SAL_CALL Test_Impl::setSequenceAny(
    const Sequence<Any >& aSeq ) throw (RuntimeException, std::exception)
{
    _arAny = aSeq;
    return aSeq;
}

Sequence<TestElement > SAL_CALL Test_Impl::setSequenceStruct(
    const Sequence< TestElement >& aSeq ) throw (RuntimeException, std::exception)
{
    _arStruct = aSeq;
    return aSeq;
}

Sequence< Sequence< sal_Int32 > > SAL_CALL Test_Impl::setDim2(
        const Sequence<Sequence< sal_Int32 > >& aSeq ) throw (RuntimeException, std::exception)
{
    _arLong2 = aSeq;
    return aSeq;
}

Sequence< Sequence< Sequence< sal_Int32 > > > SAL_CALL Test_Impl::setDim3(
        const Sequence< Sequence< Sequence< sal_Int32 > > >& aSeq )
        throw (RuntimeException, std::exception)
{
    _arLong3 = aSeq;
    return aSeq;
}

void SAL_CALL Test_Impl::setSequencesInOut(Sequence< sal_Bool >& aSeqBoolean,
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
        throw (RuntimeException, std::exception)
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

void SAL_CALL Test_Impl::setSequencesOut( Sequence< sal_Bool >& aSeqBoolean,
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
        throw (RuntimeException, std::exception)
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
    Reference< XComponentContext > const & context) throw (RuntimeException, std::exception)
{
    Sequence< sal_Bool > arg14(1); arg14[0] = true;
    Sequence< sal_Int8 > arg15(1); arg15[0] = SAL_MIN_INT8;
    Sequence< sal_Int16 > arg16(1); arg16[0] = SAL_MIN_INT16;
    Sequence< sal_uInt16 > arg17(1); arg17[0] = SAL_MAX_UINT16;
    Sequence< sal_Int32 > arg18(1); arg18[0] = SAL_MIN_INT32;
    Sequence< sal_uInt32 > arg19(1); arg19[0] = SAL_MAX_UINT32;
    Sequence< sal_Int64 > arg20(1); arg20[0] = SAL_MIN_INT64;
    Sequence< sal_uInt64 > arg21(1); arg21[0] = SAL_MAX_UINT64;
    Sequence< float > arg22(1); arg22[0] = 0.123f;
    Sequence< double > arg23(1); arg23[0] = 0.456;
    Sequence< sal_Unicode > arg24(1); arg24[0] = 'X';
    Sequence< OUString > arg25 { "test" };
    Sequence< Type > arg26(1); arg26[0] = UnoType< Any >::get();
    Sequence< Any > arg27(1); arg27[0] <<= true;
    Sequence< Sequence< sal_Bool > > arg28(1);
    arg28[0] = Sequence< sal_Bool >(1); arg28[0][0] = true;
    Sequence< Sequence< Any > > arg29(1); arg29[0] = Sequence< Any >(1);
    arg29[0][0] <<= true;
    Sequence< TestEnum > arg30(1); arg30[0] = TestEnum_TWO;
    Sequence< TestStruct > arg31(1); arg31[0].member = 10;
    Sequence< TestPolyStruct< sal_Bool > > arg32(1); arg32[0].member = true;
    Sequence< TestPolyStruct< Any > > arg33(1); arg33[0].member <<= true;
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
        "test",
        UnoType< Any >::get(),
        makeAny(true),
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
        TestPolyStruct< sal_Bool >(true),
        TestPolyStruct< Any >(makeAny(true)),
        Reference< XInterface >(nullptr));
    Sequence< Any > args(40);
    args[0] <<= true;
    args[1] <<= SAL_MIN_INT8;
    args[2] <<= SAL_MIN_INT16;
    args[3] <<= SAL_MAX_UINT16;
    args[4] <<= SAL_MIN_INT32;
    args[5] <<= SAL_MAX_UINT32;
    args[6] <<= SAL_MIN_INT64;
    args[7] <<= SAL_MAX_UINT64;
    args[8] <<= 0.123f;
    args[9] <<= 0.456;
    sal_Unicode arg10c = 'X';
    args[10].setValue(&arg10c, UnoType< UnoCharType >::get());
    args[11] <<= OUString("test");
    args[12] <<= UnoType< Any >::get();
    args[13] <<= true;
    args[14] <<= arg14;
    args[15] <<= arg15;
    args[16] <<= arg16;
    args[17] <<= arg17;
    args[18] <<= arg18;
    args[19] <<= arg19;
    args[20] <<= arg20;
    args[21] <<= arg21;
    args[22] <<= arg22;
    args[23] <<= arg23;
    args[24].setValue(&arg24, UnoType< UnoSequenceType< UnoCharType > >::get());
    args[25] <<= arg25;
    args[26] <<= arg26;
    args[27] <<= arg27;
    args[28] <<= arg28;
    args[29] <<= arg29;
    args[30] <<= arg30;
    args[31] <<= arg31;
    args[32] <<= arg32;
    args[33] <<= arg33;
    args[34] <<= arg34;
    args[35] <<= TestEnum_TWO;
    args[36] <<= TestStruct(10);
    args[37] <<= TestPolyStruct< sal_Bool >(true);
    args[38] <<= TestPolyStruct< Any >(makeAny(true));
    args[39] <<= Reference< XInterface >(nullptr);
    Constructors::create2(context, args);

    Sequence<Type> argSeq1(1); argSeq1[0] = cppu::UnoType<sal_Int32>::get();
    Sequence<Reference<XInterface> > argSeq2 { static_cast<XComponent*>(new Dummy()) };
    Sequence<Reference<XComponent> > argSeq2a { static_cast<XComponent*>(new Dummy()) };

    Sequence<TestPolyStruct2<sal_Unicode, Sequence<Any> > > argSeq3(1);
    argSeq3[0] = TestPolyStruct2<sal_Unicode, Sequence<Any> >('X', arg27);
    Sequence<TestPolyStruct2<TestPolyStruct<sal_Unicode>, Sequence<Any> > > argSeq4(1);
    argSeq4[0] = TestPolyStruct2<TestPolyStruct<sal_Unicode>, Sequence<Any> >(
        TestPolyStruct<sal_Unicode>('X'), arg27);
    Sequence<Sequence<sal_Int32> > argSeq5(1);
    argSeq5[0] = Sequence<sal_Int32>(1); argSeq5[0][0] = SAL_MIN_INT32;
    Sequence<TestPolyStruct<sal_Int32> > argSeq6(1);
    argSeq6[0] = TestPolyStruct<sal_Int32>(SAL_MIN_INT32);
    Sequence<TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> > > argSeq7(1);
    argSeq7[0] = TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> >(
        TestPolyStruct2<sal_Unicode, Any>('X', Any(true)));
    Sequence<TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > > argSeq8(1);
    argSeq8[0] = TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > (
        TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString>(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), OUString("test")));
    Sequence<TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > > > argSeq9(1);
    argSeq9[0] = TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > >(
        OUString("test"), TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >(
                     'X', TestPolyStruct<Any>(Any(true))));
    Sequence<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> > > argSeq10(1);
    argSeq10[0] = TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> >(
        TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), TestPolyStruct<sal_Unicode>('X'));
    Sequence<Sequence<TestPolyStruct<sal_Unicode > > > argSeq11(1);
    argSeq11[0] = Sequence<TestPolyStruct<sal_Unicode > >(1);
    argSeq11[0][0] = TestPolyStruct<sal_Unicode>('X');
    Sequence<Sequence<TestPolyStruct<TestPolyStruct2<sal_Unicode,Any> > > > argSeq12(1);
    argSeq12[0] = Sequence<TestPolyStruct<TestPolyStruct2<sal_Unicode,Any> > >(1);
    argSeq12[0][0] = TestPolyStruct<TestPolyStruct2<sal_Unicode,Any> >(
        TestPolyStruct2<sal_Unicode,Any>('X', Any(true)));
    Sequence<Sequence<TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString> > > > argSeq13(1);
    argSeq13[0] = Sequence<TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString> > >(1);
    argSeq13[0][0] = TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString> >(
        TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>,OUString>(
            TestPolyStruct2<sal_Unicode,Any>('X', Any(true)), OUString("test")));
    Sequence<Sequence<TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > > > > argSeq14(1);
    argSeq14[0] = Sequence<TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > > >(1);
    argSeq14[0][0] = TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> > >(
        OUString("test"), TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >(
            'X', TestPolyStruct<Any>(Any(true))));
    Sequence<Sequence<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>, TestPolyStruct<sal_Unicode> > > > argSeq15(1);
    argSeq15[0] = Sequence<TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>, TestPolyStruct<sal_Unicode> > >(1);
    argSeq15[0][0] = TestPolyStruct2<TestPolyStruct2<sal_Unicode,Any>, TestPolyStruct<sal_Unicode> >(
        TestPolyStruct2<sal_Unicode,Any>('X',Any(true)), TestPolyStruct<sal_Unicode>('X'));

    Constructors2::create1(
        context,
        TestPolyStruct<Type>(cppu::UnoType<sal_Int32>::get()),
        TestPolyStruct<Any>(Any(true)),
        TestPolyStruct<sal_Bool>(true),
        TestPolyStruct<sal_Int8>(SAL_MIN_INT8),
        TestPolyStruct<sal_Int16>(SAL_MIN_INT16),
        TestPolyStruct<sal_Int32>(SAL_MIN_INT32),
        TestPolyStruct<sal_Int64>(SAL_MIN_INT64),
        TestPolyStruct<sal_Unicode>('X'),
        TestPolyStruct<OUString>(OUString("test")),
        TestPolyStruct<float>(0.123f),
        TestPolyStruct<double>(0.456),
        TestPolyStruct<Reference<XInterface> >(static_cast<XBridgeTest2*>(this)),
        TestPolyStruct<Reference<XComponent> >(static_cast<XComponent*>(new Dummy())),
        TestPolyStruct<TestEnum>(TestEnum_TWO),
        TestPolyStruct<TestPolyStruct2<sal_Unicode, Any> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true))),
        TestPolyStruct<TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString> > (
            TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>,OUString>(
                TestPolyStruct2<sal_Unicode, Any>('X', Any(true)), OUString("test"))),
        TestPolyStruct2<OUString, TestPolyStruct2<sal_Unicode,TestPolyStruct<Any> > >(
            OUString("test"),
            TestPolyStruct2<sal_Unicode, TestPolyStruct<Any> >('X', TestPolyStruct<Any>(Any(true)))),
        TestPolyStruct2<TestPolyStruct2<sal_Unicode, Any>, TestPolyStruct<sal_Unicode> >(
            TestPolyStruct2<sal_Unicode, Any>('X', Any(true)),
            TestPolyStruct<sal_Unicode>('X')),
        TestPolyStruct<Sequence<Type> >(argSeq1),
        TestPolyStruct<Sequence<Any> >(arg27),
        TestPolyStruct<Sequence<sal_Bool> >(arg14),
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
    throw (RuntimeException, std::exception)
{
    return new testtools::bridgetest::CurrentContextChecker;
}

// XServiceInfo

OUString Test_Impl::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString( IMPLNAME );
}

sal_Bool Test_Impl::supportsService( const OUString & rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > Test_Impl::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return bridge_object::getSupportedServiceNames();
}




static Reference< XInterface > SAL_CALL Test_Impl_create(
    SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
{
    return Reference< XInterface >( static_cast<XBridgeTest *>(new Test_Impl()) );
}

}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, SAL_UNUSED_PARAMETER void * pServiceManager,
    SAL_UNUSED_PARAMETER void * )
{
    void * pRet = nullptr;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
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
