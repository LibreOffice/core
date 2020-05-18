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

#include <sal/main.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/weak.hxx>

#include <test/XLanguageBindingTest.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/implbase.hxx>


using namespace test;
using namespace cppu;
using namespace osl;
using namespace css::uno;
using namespace css::script;
using namespace css::beans;
using namespace css::lang;
using namespace css::reflection;
using namespace css::registry;


sal_Bool equals( const test::TestElement & rData1, const test::TestElement & rData2 )
{
    OSL_ENSURE( rData1.Bool == rData2.Bool, "### bool does not match!" );
    OSL_ENSURE( rData1.Char == rData2.Char, "### char does not match!" );
    OSL_ENSURE( rData1.Byte == rData2.Byte, "### byte does not match!" );
    OSL_ENSURE( rData1.Short == rData2.Short, "### short does not match!" );
    OSL_ENSURE( rData1.UShort == rData2.UShort, "### unsigned short does not match!" );
    OSL_ENSURE( rData1.Long == rData2.Long, "### long does not match!" );
    OSL_ENSURE( rData1.ULong == rData2.ULong, "### unsigned long does not match!" );
    OSL_ENSURE( rData1.Hyper == rData2.Hyper, "### hyper does not match!" );
    OSL_ENSURE( rData1.UHyper == rData2.UHyper, "### unsigned hyper does not match!" );
    OSL_ENSURE( rData1.Float == rData2.Float, "### float does not match!" );
    OSL_ENSURE( rData1.Double == rData2.Double, "### double does not match!" );
    OSL_ENSURE( rData1.Enum == rData2.Enum, "### enum does not match!" );
    OSL_ENSURE( rData1.String == rData2.String, "### string does not match!" );
    OSL_ENSURE( rData1.Interface == rData2.Interface, "### interface does not match!" );
    OSL_ENSURE( rData1.Any == rData2.Any, "### any does not match!" );

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

sal_Bool equals( const test::TestData & rData1, const test::TestData & rData2 )
{
    sal_Int32 nLen;

    if ((rData1.Sequence == rData2.Sequence) &&
        equals( (const test::TestElement &)rData1, (const test::TestElement &)rData2 ) &&
        (nLen = rData1.Sequence.getLength()) == rData2.Sequence.getLength())
    {
        // once again by hand sequence ==
        const test::TestElement * pElements1 = rData1.Sequence.getConstArray();
        const test::TestElement * pElements2 = rData2.Sequence.getConstArray();
        for ( ; nLen--; )
        {
            if (! equals( pElements1[nLen], pElements2[nLen] ))
            {
                OSL_FAIL( "### sequence element did not match!" );
                return sal_False;
            }
        }
        return sal_True;
    }
    return sal_False;
}

void assign( test::TestElement & rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test::TestEnum eEnum, const OUString& rStr,
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

void assign( test::TestData & rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test::TestEnum eEnum, const OUString& rStr,
             const css::uno::Reference< css::uno::XInterface >& xTest,
             const css::uno::Any& rAny,
             const css::uno::Sequence< test::TestElement >& rSequence )
{
    assign( (test::TestElement &)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny );
    rData.Sequence = rSequence;
}


class Test_Impl : public WeakImplHelper< XLanguageBindingTest >
{
    test::TestData _aData, _aStructData;

public:
    virtual ~Test_Impl()
        { SAL_INFO("stoc", "> scalar Test_Impl dtor <" ); }

    // XLBTestBase
    virtual void SAL_CALL setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                                     sal_Int16 nShort, sal_uInt16 nUShort,
                                     sal_Int32 nLong, sal_uInt32 nULong,
                                     sal_Int64 nHyper, sal_uInt64 nUHyper,
                                     float fFloat, double fDouble,
                                     test::TestEnum eEnum, const OUString& rStr,
                                     const css::uno::Reference< css::uno::XInterface >& xTest,
                                     const css::uno::Any& rAny,
                                     const css::uno::Sequence<test::TestElement >& rSequence,
                                     const test::TestData& rStruct )
        throw(css::uno::RuntimeException);

    virtual test::TestData SAL_CALL setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                                sal_Int16& nShort, sal_uInt16& nUShort,
                                                sal_Int32& nLong, sal_uInt32& nULong,
                                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                                float& fFloat, double& fDouble,
                                                test::TestEnum& eEnum, OUString& rStr,
                                                css::uno::Reference< css::uno::XInterface >& xTest,
                                                css::uno::Any& rAny,
                                                css::uno::Sequence<test::TestElement >& rSequence,
                                                test::TestData& rStruct )
        throw(css::uno::RuntimeException);

    virtual test::TestData SAL_CALL getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                               sal_Int16& nShort, sal_uInt16& nUShort,
                                               sal_Int32& nLong, sal_uInt32& nULong,
                                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                                               float& fFloat, double& fDouble,
                                               test::TestEnum& eEnum, OUString& rStr,
                                               css::uno::Reference< css::uno::XInterface >& xTest,
                                               css::uno::Any& rAny,
                                               css::uno::Sequence< test::TestElement >& rSequence,
                                               test::TestData& rStruct )
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getBool() throw(css::uno::RuntimeException)
        { return _aData.Bool; }
    virtual sal_Int8 SAL_CALL getByte() throw(css::uno::RuntimeException)
        { return _aData.Byte; }
    virtual sal_Unicode SAL_CALL getChar() throw(css::uno::RuntimeException)
        { return _aData.Char; }
    virtual sal_Int16 SAL_CALL getShort() throw(css::uno::RuntimeException)
        { return _aData.Short; }
    virtual sal_uInt16 SAL_CALL getUShort() throw(css::uno::RuntimeException)
        { return _aData.UShort; }
    virtual sal_Int32 SAL_CALL getLong() throw(css::uno::RuntimeException)
        { return _aData.Long; }
    virtual sal_uInt32 SAL_CALL getULong() throw(css::uno::RuntimeException)
        { return _aData.ULong; }
    virtual sal_Int64 SAL_CALL getHyper() throw(css::uno::RuntimeException)
        { return _aData.Hyper; }
    virtual sal_uInt64 SAL_CALL getUHyper() throw(css::uno::RuntimeException)
        { return _aData.UHyper; }
    virtual float SAL_CALL getFloat() throw(css::uno::RuntimeException)
        { return _aData.Float; }
    virtual double SAL_CALL getDouble() throw(css::uno::RuntimeException)
        { return _aData.Double; }
    virtual test::TestEnum SAL_CALL getEnum() throw(css::uno::RuntimeException)
        { return _aData.Enum; }
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException)
        { return _aData.String; }
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getInterface(  ) throw(css::uno::RuntimeException)
        { return _aData.Interface; }
    virtual css::uno::Any SAL_CALL getAny() throw(css::uno::RuntimeException)
        { return _aData.Any; }
    virtual css::uno::Sequence< test::TestElement > SAL_CALL getSequence() throw(css::uno::RuntimeException)
        { return _aData.Sequence; }
    virtual test::TestData SAL_CALL getStruct() throw(css::uno::RuntimeException)
        { return _aStructData; }

    virtual void SAL_CALL setBool( sal_Bool _bool ) throw(css::uno::RuntimeException)
        { _aData.Bool = _bool; }
    virtual void SAL_CALL setByte( sal_Int8 _byte ) throw(css::uno::RuntimeException)
        { _aData.Byte = _byte; }
    virtual void SAL_CALL setChar( sal_Unicode _char ) throw(css::uno::RuntimeException)
        { _aData.Char = _char; }
    virtual void SAL_CALL setShort( sal_Int16 _short ) throw(css::uno::RuntimeException)
        { _aData.Short = _short; }
    virtual void SAL_CALL setUShort( sal_uInt16 _ushort ) throw(css::uno::RuntimeException)
        { _aData.UShort = _ushort; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(css::uno::RuntimeException)
        { _aData.Long = _long; }
    virtual void SAL_CALL setULong( sal_uInt32 _ulong ) throw(css::uno::RuntimeException)
        { _aData.ULong = _ulong; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(css::uno::RuntimeException)
        { _aData.Hyper = _hyper; }
    virtual void SAL_CALL setUHyper( sal_uInt64 _uhyper ) throw(css::uno::RuntimeException)
        { _aData.UHyper = _uhyper; }
    virtual void SAL_CALL setFloat( float _float ) throw(css::uno::RuntimeException)
        { _aData.Float = _float; }
    virtual void SAL_CALL setDouble( double _double ) throw(css::uno::RuntimeException)
        { _aData.Double = _double; }
    virtual void SAL_CALL setEnum( test::TestEnum _enum ) throw(css::uno::RuntimeException)
        { _aData.Enum = _enum; }
    virtual void SAL_CALL setString( const OUString& _string ) throw(css::uno::RuntimeException)
        { _aData.String = _string; }
    virtual void SAL_CALL setInterface( const css::uno::Reference< css::uno::XInterface >& _interface ) throw(css::uno::RuntimeException)
        { _aData.Interface = _interface; }
    virtual void SAL_CALL setAny( const css::uno::Any& _any ) throw(css::uno::RuntimeException)
        { _aData.Any = _any; }
    virtual void SAL_CALL setSequence( const css::uno::Sequence<test::TestElement >& _sequence ) throw(css::uno::RuntimeException)
        { _aData.Sequence = _sequence; }
    virtual void SAL_CALL setStruct( const test::TestData& _struct ) throw(css::uno::RuntimeException)
        { _aStructData = _struct; }

    // XLanguageBindingTest
    virtual test::TestData SAL_CALL raiseException( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte, sal_Int16& nShort, sal_uInt16& nUShort, sal_Int32& nLong, sal_uInt32& nULong, sal_Int64& nHyper, sal_uInt64& nUHyper, float& fFloat, double& fDouble, test::TestEnum& eEnum, OUString& aString, css::uno::Reference< css::uno::XInterface >& xInterface, css::uno::Any& aAny, css::uno::Sequence<test::TestElement >& aSequence,test::TestData& aStruct )
        throw(css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getRuntimeException() throw(css::uno::RuntimeException);
    virtual void SAL_CALL setRuntimeException( sal_Int32 _runtimeexception ) throw(css::uno::RuntimeException);
};

class XLB_Invocation : public WeakImplHelper< XInvocation >
{
    Reference< XLanguageBindingTest > _xLBT;

public:
    XLB_Invocation( const Reference< XMultiServiceFactory > & /*xMgr*/,
                    const Reference< XLanguageBindingTest > & xLBT )
        : _xLBT( xLBT )
        {}

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() throw(css::uno::RuntimeException)
        { return Reference< XIntrospectionAccess >(); }
    virtual Any SAL_CALL invoke( const OUString & rFunctionName,
                                 const Sequence< Any > & rParams,
                                 Sequence< sal_Int16 > & rOutParamIndex,
                                 Sequence< Any > & rOutParam ) throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const OUString & rPropertyName, const Any & rValue ) throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException);
    virtual Any SAL_CALL getValue( const OUString & rPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasMethod( const OUString & rName ) throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasProperty( const OUString & rName ) throw(css::uno::RuntimeException);
};

Any XLB_Invocation::invoke( const OUString & rFunctionName,
                            const Sequence< Any > & rParams,
                            Sequence< sal_Int16 > & rOutParamIndex,
                            Sequence< Any > & rOutParam )
    throw(css::lang::IllegalArgumentException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException)
{
    bool bImplementedMethod = true;

    Any aRet;

    OSL_ASSERT( rOutParam.getLength() == 0 );
    OSL_ASSERT( rOutParamIndex.getLength() == 0 );

    try
    {
        sal_Bool aBool;
        sal_Unicode aChar;
        sal_Int8 nByte;
        sal_Int16 nShort;
        sal_uInt16 nUShort;
        sal_Int32 nLong;
        sal_uInt32 nULong;
        sal_Int64 nHyper;
        sal_uInt64 nUHyper;
        float fFloat = 0.0;
        double fDouble = 0.0;
        TestEnum eEnum;
        OUString aString;
        Reference< XInterface > xInterface;
        Any aAny;
        Sequence< TestElement > aSeq;
        TestData aData;

        if ( rFunctionName == "setValues" )
        {
            OSL_ASSERT( rParams.getLength() == 17 );
            aBool = *(sal_Bool *)rParams[0].getValue();
            aChar = *(sal_Unicode *)rParams[1].getValue();
            rParams[2] >>= nByte;
            rParams[3] >>= nShort;
            rParams[4] >>= nUShort;
            rParams[5] >>= nLong;
            rParams[6] >>= nULong;
            rParams[7] >>= nHyper;
            rParams[8] >>= nUHyper;
            rParams[9] >>= fFloat;
            rParams[10] >>= fDouble;
            rParams[11] >>= eEnum;
            rParams[12] >>= aString;
            rParams[13] >>= xInterface;
            rParams[14] >>= aAny;
            rParams[15] >>= aSeq;
            rParams[16] >>= aData;

            _xLBT->setValues( aBool, aChar, nByte, nShort, nUShort, nLong, nULong,
                              nHyper, nUHyper, fFloat, fDouble, eEnum, aString, xInterface,
                              aAny, aSeq, aData );

            rOutParamIndex.realloc( 0 );
            rOutParam.realloc( 0 );
        }
        else if ( rFunctionName == "setValues2" )
        {
            aBool = *(sal_Bool *)rParams[0].getValue();
            aChar = *(sal_Unicode *)rParams[1].getValue();
            rParams[2] >>= nByte;
            rParams[3] >>= nShort;
            rParams[4] >>= nUShort;
            rParams[5] >>= nLong;
            rParams[6] >>= nULong;
            rParams[7] >>= nHyper;
            rParams[8] >>= nUHyper;
            rParams[9] >>= fFloat;
            rParams[10] >>= fDouble;
            rParams[11] >>= eEnum;
            rParams[12] >>= aString;
            rParams[13] >>= xInterface;
            rParams[14] >>= aAny;
            rParams[15] >>= aSeq;
            rParams[16] >>= aData;

            aRet <<= _xLBT->setValues2( aBool, aChar, nByte, nShort, nUShort, nLong, nULong,
                                        nHyper, nUHyper, fFloat, fDouble, eEnum, aString, xInterface,
                                        aAny, aSeq, aData );

            rOutParamIndex.realloc( 17 );
            rOutParamIndex[0] = 0;
            rOutParamIndex[1] = 1;
            rOutParamIndex[2] = 2;
            rOutParamIndex[3] = 3;
            rOutParamIndex[4] = 4;
            rOutParamIndex[5] = 5;
            rOutParamIndex[6] = 6;
            rOutParamIndex[7] = 7;
            rOutParamIndex[8] = 8;
            rOutParamIndex[9] = 9;
            rOutParamIndex[10] = 10;
            rOutParamIndex[11] = 11;
            rOutParamIndex[12] = 12;
            rOutParamIndex[13] = 13;
            rOutParamIndex[14] = 14;
            rOutParamIndex[15] = 15;
            rOutParamIndex[16] = 16;

            rOutParam.realloc( 17 );
            rOutParam[0].setValue( &aBool, cppu::UnoType<bool>::get() );
            rOutParam[1].setValue( &aChar, cppu::UnoType<cppu::UnoCharType>::get() );
            rOutParam[2] <<= nByte;
            rOutParam[3] <<= nShort;
            rOutParam[4] <<= nUShort;
            rOutParam[5] <<= nLong;
            rOutParam[6] <<= nULong;
            rOutParam[7] <<= nHyper;
            rOutParam[8] <<= nUHyper;
            rOutParam[9] <<= fFloat;
            rOutParam[10] <<= fDouble;
            rOutParam[11] <<= eEnum;
            rOutParam[12] <<= aString;
            rOutParam[13] <<= xInterface;
            rOutParam[14] <<= aAny;
            rOutParam[15] <<= aSeq;
            rOutParam[16] <<= aData;
        }
        else if ( rFunctionName == "getValues" )
        {
            aRet <<= _xLBT->getValues( aBool, aChar, nByte, nShort, nUShort, nLong, nULong,
                                       nHyper, nUHyper, fFloat, fDouble, eEnum, aString, xInterface,
                                       aAny, aSeq, aData );

            rOutParamIndex.realloc( 17 );
            rOutParamIndex[0] = 0;
            rOutParamIndex[1] = 1;
            rOutParamIndex[2] = 2;
            rOutParamIndex[3] = 3;
            rOutParamIndex[4] = 4;
            rOutParamIndex[5] = 5;
            rOutParamIndex[6] = 6;
            rOutParamIndex[7] = 7;
            rOutParamIndex[8] = 8;
            rOutParamIndex[9] = 9;
            rOutParamIndex[10] = 10;
            rOutParamIndex[11] = 11;
            rOutParamIndex[12] = 12;
            rOutParamIndex[13] = 13;
            rOutParamIndex[14] = 14;
            rOutParamIndex[15] = 15;
            rOutParamIndex[16] = 16;

            rOutParam.realloc( 17 );
            rOutParam[0].setValue( &aBool, cppu::UnoType<bool>::get() );
            rOutParam[1].setValue( &aChar, cppu::UnoType<cppu::UnoCharType>::get() );
            rOutParam[2] <<= nByte;
            rOutParam[3] <<= nShort;
            rOutParam[4] <<= nUShort;
            rOutParam[5] <<= nLong;
            rOutParam[6] <<= nULong;
            rOutParam[7] <<= nHyper;
            rOutParam[8] <<= nUHyper;
            rOutParam[9] <<= fFloat;
            rOutParam[10] <<= fDouble;
            rOutParam[11] <<= eEnum;
            rOutParam[12] <<= aString;
            rOutParam[13] <<= xInterface;
            rOutParam[14] <<= aAny;
            rOutParam[15] <<= aSeq;
            rOutParam[16] <<= aData;
        }
        else if ( rFunctionName == "raiseException" )
        {
            aRet <<= _xLBT->raiseException( aBool, aChar, nByte, nShort, nUShort, nLong, nULong,
                                            nHyper, nUHyper, fFloat, fDouble, eEnum, aString, xInterface,
                                            aAny, aSeq, aData );

            rOutParamIndex.realloc( 17 );
            rOutParamIndex[0] = 0;
            rOutParamIndex[1] = 1;
            rOutParamIndex[2] = 2;
            rOutParamIndex[3] = 3;
            rOutParamIndex[4] = 4;
            rOutParamIndex[5] = 5;
            rOutParamIndex[6] = 6;
            rOutParamIndex[7] = 7;
            rOutParamIndex[8] = 8;
            rOutParamIndex[9] = 9;
            rOutParamIndex[10] = 10;
            rOutParamIndex[11] = 11;
            rOutParamIndex[12] = 12;
            rOutParamIndex[13] = 13;
            rOutParamIndex[14] = 14;
            rOutParamIndex[15] = 15;
            rOutParamIndex[16] = 16;

            rOutParam.realloc( 17 );
            rOutParam[0].setValue( &aBool, cppu::UnoType<bool>::get() );
            rOutParam[1].setValue( &aChar, cppu::UnoType<cppu::UnoCharType>::get() );
            rOutParam[2] <<= nByte;
            rOutParam[3] <<= nShort;
            rOutParam[4] <<= nUShort;
            rOutParam[5] <<= nLong;
            rOutParam[6] <<= nULong;
            rOutParam[7] <<= nHyper;
            rOutParam[8] <<= nUHyper;
            rOutParam[9] <<= fFloat;
            rOutParam[10] <<= fDouble;
            rOutParam[11] <<= eEnum;
            rOutParam[12] <<= aString;
            rOutParam[13] <<= xInterface;
            rOutParam[14] <<= aAny;
            rOutParam[15] <<= aSeq;
            rOutParam[16] <<= aData;
        }
        else
        {
            bImplementedMethod = false;
        }
    }
    catch (const IllegalArgumentException & rExc)
    {
        // thrown by raiseException() call
        InvocationTargetException aExc;
        aExc.TargetException <<= rExc;
        throw aExc;
    }
    catch (Exception &)
    {
        OSL_FAIL( "### unexpected exception caught!" );
        throw;
    }

    if (! bImplementedMethod)
    {
        throw IllegalArgumentException(
            OUString( "not an implemented method!" ),
            (OWeakObject *)this, 0 );
    }

    return aRet;
}

void XLB_Invocation::setValue( const OUString & rName, const Any & rValue )
    throw(css::beans::UnknownPropertyException, css::script::CannotConvertException, css::reflection::InvocationTargetException, css::uno::RuntimeException)
{
    if ( rName == "Bool" )
        _xLBT->setBool( *(const sal_Bool *)rValue.getValue() );
    else if ( rName == "Byte" )
        _xLBT->setByte( *(const sal_Int8 *)rValue.getValue() );
    else if ( rName == "Char" )
        _xLBT->setChar( *(const sal_Unicode *)rValue.getValue() );
    else if ( rName == "Short" )
        _xLBT->setShort( *(const sal_Int16 *)rValue.getValue() );
    else if ( rName == "UShort" )
        _xLBT->setUShort( *(const sal_uInt16 *)rValue.getValue() );
    else if ( rName == "Long" )
        _xLBT->setLong( *(const sal_Int32 *)rValue.getValue() );
    else if ( rName == "ULong" )
        _xLBT->setULong( *(const sal_uInt32 *)rValue.getValue() );
    else if ( rName == "Hyper" )
        _xLBT->setHyper( *(const sal_Int64 *)rValue.getValue() );
    else if ( rName == "UHyper" )
        _xLBT->setUHyper( *(const sal_uInt64 *)rValue.getValue() );
    else if ( rName == "Float" )
        _xLBT->setFloat( *(const float *)rValue.getValue() );
    else if ( rName == "Double" )
        _xLBT->setDouble( *(const double *)rValue.getValue() );
    else if ( rName == "Enum" )
        _xLBT->setEnum( *(const TestEnum *)rValue.getValue() );
    else if ( rName == "String" )
        _xLBT->setString( *(const OUString *)rValue.getValue() );
    else if ( rName == "Interface" )
        _xLBT->setInterface( *(const Reference< XInterface > *)rValue.getValue() );
    else if ( rName == "Any" )
        _xLBT->setAny( rValue );
    else if ( rName == "Sequence" )
        _xLBT->setSequence( *(const Sequence< TestElement > *)rValue.getValue() );
    else if ( rName == "Struct" )
        _xLBT->setStruct( *(const TestData *)rValue.getValue() );
    else if ( rName == "RuntimeException" )
        _xLBT->setRuntimeException( *(const sal_Int32 *)rValue.getValue() );
}

Any XLB_Invocation::getValue( const OUString & rName )
    throw(css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    Any aRet;
    if ( rName == "Bool" )
    {
        sal_Bool aBool = _xLBT->getBool();
        aRet.setValue( &aBool, cppu::UnoType<bool>::get() );
    }
    else if ( rName == "Byte" )
        aRet <<= _xLBT->getByte();
    else if ( rName == "Char" )
    {
        sal_Unicode aChar = _xLBT->getChar();
        aRet.setValue( &aChar, cppu::UnoType<cppu::UnoCharType>::get() );
    }
    else if ( rName == "Short" )
        aRet <<= _xLBT->getShort();
    else if ( rName == "UShort" )
        aRet <<= _xLBT->getUShort();
    else if ( rName == "Long" )
        aRet <<= _xLBT->getLong();
    else if ( rName == "ULong" )
        aRet <<= _xLBT->getULong();
    else if ( rName == "Hyper" )
        aRet <<= _xLBT->getHyper();
    else if ( rName == "UHyper" )
        aRet <<= _xLBT->getUHyper();
    else if ( rName == "Float" )
        aRet <<= _xLBT->getFloat();
    else if ( rName == "Double" )
        aRet <<= _xLBT->getDouble();
    else if ( rName == "Enum" )
        aRet <<= _xLBT->getEnum();
    else if ( rName == "String" )
        aRet <<= _xLBT->getString();
    else if ( rName == "Interface" )
        aRet <<= _xLBT->getInterface();
    else if ( rName == "Any" )
        aRet <<= _xLBT->getAny();
    else if ( rName == "Sequence" )
        aRet <<= _xLBT->getSequence();
    else if ( rName == "Struct" )
        aRet <<= _xLBT->getStruct();
    else if ( rName == "RuntimeException" )
        aRet <<= _xLBT->getRuntimeException();
    return aRet;
}

sal_Bool XLB_Invocation::hasMethod( const OUString & rName )
    throw(css::uno::RuntimeException)
{
    return (rName == "raiseException" ||
            rName == "getValues" ||
            rName == "setValues2" ||
            rName == "setValues" ||
            rName == "acquire" ||
            rName == "release" ||
            rName == "queryInterface" );
}

sal_Bool XLB_Invocation::hasProperty( const OUString & rName )
    throw(css::uno::RuntimeException)
{
    return (rName == "Bool" ||
            rName == "Byte" ||
            rName == "Char" ||
            rName == "Short" ||
            rName == "UShort" ||
            rName == "Long" ||
            rName == "ULong" ||
            rName == "Hyper" ||
            rName == "UHyper" ||
            rName == "Float" ||
            rName == "Double" ||
            rName == "Enum" ||
            rName == "String" ||
            rName == "Interface" ||
            rName == "Any" ||
            rName == "Sequence" ||
            rName == "Struct" ||
            rName == "RuntimeException" );
}


void Test_Impl::setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                           sal_Int16 nShort, sal_uInt16 nUShort,
                           sal_Int32 nLong, sal_uInt32 nULong,
                           sal_Int64 nHyper, sal_uInt64 nUHyper,
                           float fFloat, double fDouble,
                           test::TestEnum eEnum, const OUString& rStr,
                           const css::uno::Reference< css::uno::XInterface >& xTest,
                           const css::uno::Any& rAny,
                           const css::uno::Sequence<test::TestElement >& rSequence,
                           const test::TestData& rStruct )
    throw(css::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
}

test::TestData Test_Impl::setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                      sal_Int16& nShort, sal_uInt16& nUShort,
                                      sal_Int32& nLong, sal_uInt32& nULong,
                                      sal_Int64& nHyper, sal_uInt64& nUHyper,
                                      float& fFloat, double& fDouble,
                                      test::TestEnum& eEnum, OUString& rStr,
                                      css::uno::Reference< css::uno::XInterface >& xTest,
                                      css::uno::Any& rAny,
                                      css::uno::Sequence<test::TestElement >& rSequence,
                                      test::TestData& rStruct )
    throw(css::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
    return _aStructData;
}

test::TestData Test_Impl::getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                     sal_Int16& nShort, sal_uInt16& nUShort,
                                     sal_Int32& nLong, sal_uInt32& nULong,
                                     sal_Int64& nHyper, sal_uInt64& nUHyper,
                                     float& fFloat, double& fDouble,
                                     test::TestEnum& eEnum, OUString& rStr,
                                     css::uno::Reference< css::uno::XInterface >& xTest,
                                     css::uno::Any& rAny,
                                     css::uno::Sequence<test::TestElement >& rSequence,
                                     test::TestData& rStruct )
    throw(css::uno::RuntimeException)
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


sal_Bool performTest( const Reference<XLanguageBindingTest > & xLBT )
{
    OSL_ENSURE( xLBT.is(), "### no test interface!" );
    if (xLBT.is())
    {
        // this data is never ever granted access to by calls other than equals(), assign()!
        test::TestData aData; // test against this data

        Reference<XInterface > xI( *new OWeakObject() );

        assign( (test::TestElement &)aData,
                sal_True, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
                SAL_CONST_INT64(0x123456789abcdef0),
                SAL_CONST_UINT64(0xfedcba9876543210),
                (float)17.0815, 3.1415926359, TestEnum_LOLA, OUString("dumdidum"), xI,
                Any( &xI, cppu::UnoType<XInterface>::get()) );

        OSL_ENSURE( aData.Any == xI, "### unexpected any!" );
        OSL_ENSURE( !(aData.Any != xI), "### unexpected any!" );

        aData.Sequence = Sequence<test::TestElement >( (const test::TestElement *)&aData, 1 );
        // aData complete


        // this is a manually copy of aData for first setting...
        test::TestData aSetData;

        assign( (test::TestElement &)aSetData,
                aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort,
                aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, aData.Double,
                aData.Enum, aData.String, xI,
                Any( &xI, cppu::UnoType<XInterface>::get()) );

        aSetData.Sequence = Sequence<test::TestElement >( (const test::TestElement *)&aSetData, 1 );

        xLBT->setValues(
            aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short, aSetData.UShort,
            aSetData.Long, aSetData.ULong, aSetData.Hyper, aSetData.UHyper, aSetData.Float, aSetData.Double,
            aSetData.Enum, aSetData.String, aSetData.Interface, aSetData.Any, aSetData.Sequence, aSetData );

        {
        test::TestData aRet, aRet2;
        xLBT->getValues(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) );

        // set last retrieved values
        test::TestData aSV2ret = xLBT->setValues2(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        OSL_ASSERT( equals( aData, aSV2ret ) && equals( aData, aRet2 ) );
        }
        {
        test::TestData aRet, aRet2;
        test::TestData aGVret = xLBT->getValues(
            aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
            aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
            aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );

        OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) && equals( aData, aGVret ) );

        // set last retrieved values
        xLBT->setBool( aRet.Bool );
        xLBT->setChar( aRet.Char );
        xLBT->setByte( aRet.Byte );
        xLBT->setShort( aRet.Short );
        xLBT->setUShort( aRet.UShort );
        xLBT->setLong( aRet.Long );
        xLBT->setULong( aRet.ULong );
        xLBT->setHyper( aRet.Hyper );
        xLBT->setUHyper( aRet.UHyper );
        xLBT->setFloat( aRet.Float );
        xLBT->setDouble( aRet.Double );
        xLBT->setEnum( aRet.Enum );
        xLBT->setString( aRet.String );
        xLBT->setInterface( aRet.Interface );
        xLBT->setAny( aRet.Any );
        xLBT->setSequence( aRet.Sequence );
        xLBT->setStruct( aRet2 );
        }
        {
        test::TestData aRet, aRet2;
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
        aRet2 = xLBT->getStruct();

        return (equals( aData, aRet ) && equals( aData, aRet2 ));
        }
    }
    return sal_False;
}


test::TestData Test_Impl::raiseException( sal_Bool& /*bBool*/, sal_Unicode& /*cChar*/, sal_Int8& /*nByte*/, sal_Int16& /*nShort*/, sal_uInt16& /*nUShort*/, sal_Int32& /*nLong*/, sal_uInt32& /*nULong*/, sal_Int64& /*nHyper*/, sal_uInt64& /*nUHyper*/, float& /*fFloat*/, double& /*fDouble*/, test::TestEnum& /*eEnum*/, OUString& /*aString*/, css::uno::Reference< css::uno::XInterface >& /*xInterface*/, css::uno::Any& /*aAny*/, css::uno::Sequence< test::TestElement >& /*aSequence*/, test::TestData& /*aStruct*/ )
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    IllegalArgumentException aExc;
    aExc.ArgumentPosition = 5;
    aExc.Message          = "dum dum dum I dance around the circle...";
    aExc.Context          = *this;
    throw aExc;
}

sal_Int32 Test_Impl::getRuntimeException() throw(css::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = "dum dum dum I dance around the circle...";
    aExc.Context          = *this;
    throw aExc;
}

void Test_Impl::setRuntimeException( sal_Int32 /*_runtimeexception*/ ) throw(css::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = "dum dum dum I dance around the circle...";
    aExc.Context          = *this;
    throw aExc;
}


sal_Bool raiseException( const Reference<XLanguageBindingTest > & xLBT )
{
    try
    {
        try
        {
            try
            {
                test::TestData aRet, aRet2;
                xLBT->raiseException(
                    aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
                    aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
                    aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );
                return sal_False;
            }
            catch (const IllegalArgumentException &aExc)
            {
                OSL_ENSURE( aExc.ArgumentPosition == 5 &&
                             aExc.Message.startsWith("dum dum dum I dance around the circle..."),
                             "### unexpected exception content!" );

                Reference<XLanguageBindingTest > xLBT2(
                    Reference<XLanguageBindingTest >::query( aExc.Context ) );

                OSL_ENSURE( xLBT2.is(), "### unexpected source of exception!" );
                if (xLBT2.is())
                    xLBT2->getRuntimeException();
                else
                    return sal_False;
            }
        }
        catch (const RuntimeException & rExc)
        {
            OSL_ENSURE( rExc.Message.startsWith("dum dum dum I dance around the circle..."),
                        "### unexpected exception content!" );

            Reference<XLanguageBindingTest > xLBT2(
                Reference<XLanguageBindingTest >::query( rExc.Context ) );

            OSL_ENSURE( xLBT2.is(), "### unexpected source of exception!" );
            if (xLBT2.is())
                xLBT2->setRuntimeException( 0xcafebabe );
            else
                return sal_False;
        }
    }
    catch (const Exception & aExc)
    {
        OSL_ENSURE( aExc.Message.startsWith("dum dum dum I dance around the circle..."),
                     "### unexpected exception content!" );
        return aExc.Message.startsWith("dum dum dum I dance around the circle...");
    }
    return sal_False;
}


static sal_Bool test_adapter( const Reference< XMultiServiceFactory > & xMgr )
{
    Reference< XInvocationAdapterFactory > xAdapFac(
        xMgr->createInstance("com.sun.star.script.InvocationAdapterFactory"), UNO_QUERY );
    Reference< XInvocationAdapterFactory2 > xAdapFac2( xAdapFac, UNO_QUERY_THROW );

    Reference< XLanguageBindingTest > xOriginal( (XLanguageBindingTest *)new Test_Impl() );
    Reference< XInvocation > xInvok( new XLB_Invocation( xMgr, xOriginal ) );
    Reference< XLanguageBindingTest > xLBT( xAdapFac->createAdapter(
        xInvok, cppu::UnoType<XLanguageBindingTest>::get()), UNO_QUERY );
    Reference< XLanguageBindingTest > xLBT2(
        xAdapFac->createAdapter(
            xInvok, cppu::UnoType<XLanguageBindingTest>::get()), UNO_QUERY );
    if (xLBT != xLBT2)
        return sal_False;
    Reference< XInterface > xLBT3(
        xAdapFac->createAdapter(
            xInvok, cppu::UnoType<XInterface>::get()), UNO_QUERY );
    if (xLBT != xLBT3)
        return sal_False;
    Type ar[ 2 ] = {
        cppu::UnoType<XLBTestBase>::get(),
        cppu::UnoType<XInterface>::get()};
    Reference< XInterface > xLBT4(
        xAdapFac2->createAdapter( xInvok, Sequence< Type >( ar, 2 ) ), UNO_QUERY );
    if (xLBT != xLBT4)
        return sal_False;
    Reference< XSimpleRegistry > xInvalidAdapter(
        xAdapFac->createAdapter(
            xInvok, cppu::UnoType<XSimpleRegistry>::get()), UNO_QUERY );
    if (xLBT == xInvalidAdapter)
        return sal_False;

    try
    {
        xInvalidAdapter->isValid();
        return sal_False;
    }
    catch (RuntimeException &)
    {
    }

    return (performTest( xLBT ) && raiseException( xLBT ));
}

static sal_Bool test_invocation( const Reference< XMultiServiceFactory > & xMgr )
{
    Reference< XInvocationAdapterFactory > xAdapFac(
        xMgr->createInstance("com.sun.star.script.InvocationAdapterFactory"), UNO_QUERY );
    Reference< XSingleServiceFactory > xInvocFac(
        xMgr->createInstance("com.sun.star.script.Invocation"), UNO_QUERY );

    Reference< XLanguageBindingTest > xOriginal( (XLanguageBindingTest *)new Test_Impl() );
    Any aOriginal( &xOriginal, cppu::UnoType<decltype(xOriginal)>::get() );
    Reference< XInvocation > xInvok(
        xInvocFac->createInstanceWithArguments( Sequence< Any >( &aOriginal, 1 ) ), UNO_QUERY );

    Reference< XLanguageBindingTest > xLBT( xAdapFac->createAdapter(
        xInvok, cppu::UnoType<XLanguageBindingTest>::get()), UNO_QUERY );

    return (performTest( xLBT ) && raiseException( xLBT ));
}

SAL_IMPLEMENT_MAIN()
{
    Reference< XMultiServiceFactory > xMgr( createRegistryServiceFactory(
        OUString( "stoctest.rdb" ) ) );

    try
    {
        Reference< XImplementationRegistration > xImplReg(
            xMgr->createInstance( "com.sun.star.registry.ImplementationRegistration" ),
            UNO_QUERY );
        OSL_ENSURE( xImplReg.is(), "### no impl reg!" );

        xImplReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary"),
            OUString("invocadapt.uno" SAL_DLLEXTENSION),
            Reference< XSimpleRegistry >() );
        xImplReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary"),
            OUString("stocservices.uno" SAL_DLLEXTENSION),
            Reference< XSimpleRegistry >() );
        xImplReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary"),
            OUString("invocation.uno" SAL_DLLEXTENSION),
            Reference< XSimpleRegistry >() );
        xImplReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary"),
            OUString("reflection.uno" SAL_DLLEXTENSION),
            Reference< XSimpleRegistry >() );
        xImplReg->registerImplementation(
            OUString("com.sun.star.loader.SharedLibrary"),
            OUString("introspection.uno" SAL_DLLEXTENSION),
            Reference< XSimpleRegistry >() );

        if (test_adapter( xMgr ))
        {
            fprintf( stderr, "> test_iadapter() succeeded.\n" );
            if (test_invocation( xMgr ))
            {
                fprintf( stderr, "> test_invocation() succeeded.\n" );
            }
        }
    }
    catch (const Exception & rExc)
    {
        fprintf( stderr, "> exception occurred: " );
        OString aMsg( OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
        fprintf( stderr, "%s\n", aMsg.getStr() );
    }

    Reference< XComponent >( xMgr, UNO_QUERY )->dispose();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
