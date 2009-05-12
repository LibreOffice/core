/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: test_di.cxx,v $
 * $Revision: 1.21 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include <stdio.h>
#include <string.h>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <osl/time.h>
#include <sal/types.h>
//
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <uno/data.h>

//  #include <uno/cuno.h>
//  #include <test/XLanguageBindingTest.h>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppu/macros.hxx>
#include <test/XLanguageBindingTest.hpp>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>


using namespace test;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;


//==================================================================================================
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
//==================================================================================================
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
                OSL_ENSURE( sal_False, "### sequence element did not match!" );
                return sal_False;
            }
        }
        return sal_True;
    }
    return sal_False;
}
//==================================================================================================
void assign( test::TestElement & rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test::TestEnum eEnum, const ::rtl::OUString& rStr,
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
//==================================================================================================
void assign( test::TestData & rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test::TestEnum eEnum, const ::rtl::OUString& rStr,
             const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
             const ::com::sun::star::uno::Any& rAny,
             const com::sun::star::uno::Sequence< test::TestElement >& rSequence )
{
    assign( (test::TestElement &)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny );
    rData.Sequence = rSequence;
}

//==================================================================================================
class TestDummy : public OWeakObject
{
public:
    sal_Int32 getRefCount() const
        { return m_refCount; }

    virtual ~TestDummy()
        { OSL_TRACE( "> scalar TestDummy dtor <\n" ); }
};
//==================================================================================================
class Test_Impl : public cppu::WeakImplHelper1< XLanguageBindingTest >
{
    test::TestData _aData, _aStructData;

public:
    sal_Int32 getRefCount() const
        { return m_refCount; }

    virtual ~Test_Impl()
        { OSL_TRACE( "> scalar Test_Impl dtor <\n" ); }

    // XLBTestBase
    virtual void SAL_CALL setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                                     sal_Int16 nShort, sal_uInt16 nUShort,
                                     sal_Int32 nLong, sal_uInt32 nULong,
                                     sal_Int64 nHyper, sal_uInt64 nUHyper,
                                     float fFloat, double fDouble,
                                     test::TestEnum eEnum, const ::rtl::OUString& rStr,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                     const ::com::sun::star::uno::Any& rAny,
                                     const ::com::sun::star::uno::Sequence<test::TestElement >& rSequence,
                                     const test::TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual test::TestData SAL_CALL setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                                sal_Int16& nShort, sal_uInt16& nUShort,
                                                sal_Int32& nLong, sal_uInt32& nULong,
                                                sal_Int64& nHyper, sal_uInt64& nUHyper,
                                                float& fFloat, double& fDouble,
                                                test::TestEnum& eEnum, rtl::OUString& rStr,
                                                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                                ::com::sun::star::uno::Any& rAny,
                                                ::com::sun::star::uno::Sequence<test::TestElement >& rSequence,
                                                test::TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual test::TestData SAL_CALL getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                               sal_Int16& nShort, sal_uInt16& nUShort,
                                               sal_Int32& nLong, sal_uInt32& nULong,
                                               sal_Int64& nHyper, sal_uInt64& nUHyper,
                                               float& fFloat, double& fDouble,
                                               test::TestEnum& eEnum, rtl::OUString& rStr,
                                               ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                               ::com::sun::star::uno::Any& rAny,
                                               ::com::sun::star::uno::Sequence< test::TestElement >& rSequence,
                                               test::TestData& rStruct )
        throw(com::sun::star::uno::RuntimeException);

    virtual test::SmallStruct echoSmallStruct(const test::SmallStruct& rStruct) throw(com::sun::star::uno::RuntimeException)
        { return rStruct; }
    virtual test::MediumStruct echoMediumStruct(const test::MediumStruct& rStruct) throw(com::sun::star::uno::RuntimeException)
        { return rStruct; }
    virtual test::BigStruct echoBigStruct(const test::BigStruct& rStruct) throw(com::sun::star::uno::RuntimeException)
        { return rStruct; }
    virtual test::AllFloats echoAllFloats(const test::AllFloats& rStruct) throw(com::sun::star::uno::RuntimeException)
        { return rStruct; }

    virtual sal_Bool SAL_CALL getBool() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Bool; }
    virtual sal_Int8 SAL_CALL getByte() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Byte; }
    virtual sal_Unicode SAL_CALL getChar() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Char; }
    virtual sal_Int16 SAL_CALL getShort() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Short; }
    virtual sal_uInt16 SAL_CALL getUShort() throw(com::sun::star::uno::RuntimeException)
        { return _aData.UShort; }
    virtual sal_Int32 SAL_CALL getLong() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Long; }
    virtual sal_uInt32 SAL_CALL getULong() throw(com::sun::star::uno::RuntimeException)
        { return _aData.ULong; }
    virtual sal_Int64 SAL_CALL getHyper() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Hyper; }
    virtual sal_uInt64 SAL_CALL getUHyper() throw(com::sun::star::uno::RuntimeException)
        { return _aData.UHyper; }
    virtual float SAL_CALL getFloat() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Float; }
    virtual double SAL_CALL getDouble() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Double; }
    virtual test::TestEnum SAL_CALL getEnum() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Enum; }
    virtual rtl::OUString SAL_CALL getString() throw(com::sun::star::uno::RuntimeException)
        { return _aData.String; }
    virtual com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getInterface(  ) throw(com::sun::star::uno::RuntimeException)
        { return _aData.Interface; }
    virtual com::sun::star::uno::Any SAL_CALL getAny() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Any; }
    virtual com::sun::star::uno::Sequence< test::TestElement > SAL_CALL getSequence() throw(com::sun::star::uno::RuntimeException)
        { return _aData.Sequence; }
    virtual test::TestData SAL_CALL getStruct() throw(com::sun::star::uno::RuntimeException)
        { return _aStructData; }

    virtual void SAL_CALL setBool( sal_Bool _bool ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Bool = _bool; }
    virtual void SAL_CALL setByte( sal_Int8 _byte ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Byte = _byte; }
    virtual void SAL_CALL setChar( sal_Unicode _char ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Char = _char; }
    virtual void SAL_CALL setShort( sal_Int16 _short ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Short = _short; }
    virtual void SAL_CALL setUShort( sal_uInt16 _ushort ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.UShort = _ushort; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Long = _long; }
    virtual void SAL_CALL setULong( sal_uInt32 _ulong ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.ULong = _ulong; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Hyper = _hyper; }
    virtual void SAL_CALL setUHyper( sal_uInt64 _uhyper ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.UHyper = _uhyper; }
    virtual void SAL_CALL setFloat( float _float ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Float = _float; }
    virtual void SAL_CALL setDouble( double _double ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Double = _double; }
    virtual void SAL_CALL setEnum( test::TestEnum _enum ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Enum = _enum; }
    virtual void SAL_CALL setString( const ::rtl::OUString& _string ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.String = _string; }
    virtual void SAL_CALL setInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _interface ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Interface = _interface; }
    virtual void SAL_CALL setAny( const ::com::sun::star::uno::Any& _any ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Any = _any; }
    virtual void SAL_CALL setSequence( const ::com::sun::star::uno::Sequence<test::TestElement >& _sequence ) throw(::com::sun::star::uno::RuntimeException)
        { _aData.Sequence = _sequence; }
    virtual void SAL_CALL setStruct( const test::TestData& _struct ) throw(::com::sun::star::uno::RuntimeException)
        { _aStructData = _struct; }

    // XLanguageBindingTest
    virtual test::TestData SAL_CALL raiseException( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte, sal_Int16& nShort, sal_uInt16& nUShort, sal_Int32& nLong, sal_uInt32& nULong, sal_Int64& nHyper, sal_uInt64& nUHyper, float& fFloat, double& fDouble, test::TestEnum& eEnum, ::rtl::OUString& aString, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInterface, ::com::sun::star::uno::Any& aAny, ::com::sun::star::uno::Sequence<test::TestElement >& aSequence,test::TestData& aStruct )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getRuntimeException() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRuntimeException( sal_Int32 _runtimeexception ) throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
void Test_Impl::setValues( sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
                           sal_Int16 nShort, sal_uInt16 nUShort,
                           sal_Int32 nLong, sal_uInt32 nULong,
                           sal_Int64 nHyper, sal_uInt64 nUHyper,
                           float fFloat, double fDouble,
                           test::TestEnum eEnum, const ::rtl::OUString& rStr,
                           const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                           const ::com::sun::star::uno::Any& rAny,
                           const ::com::sun::star::uno::Sequence<test::TestElement >& rSequence,
                           const test::TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
}
//__________________________________________________________________________________________________
test::TestData Test_Impl::setValues2( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                      sal_Int16& nShort, sal_uInt16& nUShort,
                                      sal_Int32& nLong, sal_uInt32& nULong,
                                      sal_Int64& nHyper, sal_uInt64& nUHyper,
                                      float& fFloat, double& fDouble,
                                      test::TestEnum& eEnum, rtl::OUString& rStr,
                                      ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                      ::com::sun::star::uno::Any& rAny,
                                      ::com::sun::star::uno::Sequence<test::TestElement >& rSequence,
                                      test::TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
{
    assign( _aData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny, rSequence );
    _aStructData = rStruct;
    return _aStructData;
}
//__________________________________________________________________________________________________
test::TestData Test_Impl::getValues( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte,
                                     sal_Int16& nShort, sal_uInt16& nUShort,
                                     sal_Int32& nLong, sal_uInt32& nULong,
                                     sal_Int64& nHyper, sal_uInt64& nUHyper,
                                     float& fFloat, double& fDouble,
                                     test::TestEnum& eEnum, rtl::OUString& rStr,
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xTest,
                                     ::com::sun::star::uno::Any& rAny,
                                     ::com::sun::star::uno::Sequence<test::TestElement >& rSequence,
                                     test::TestData& rStruct )
    throw(com::sun::star::uno::RuntimeException)
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

//==================================================================================================
static sal_Bool performTest(
    const Reference< XLanguageBindingTest > & xLBT,
    const Reference< XInterface > & xDummyInterface )
{
    OSL_ENSURE( xLBT.is() && xDummyInterface.is(), "### no test interfaces!" );
    if (xLBT.is() && xDummyInterface.is())
    {
        // this data is never ever granted access to by calls other than equals(), assign()!
        test::TestData aData; // test against this data

        assign( (test::TestElement &)aData,
                sal_True, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
                SAL_CONST_INT64(0x123456789abcdef0),
                SAL_CONST_UINT64(0xfedcba9876543210),
                (float)17.0815, 3.1415926359, TestEnum_LOLA,
                OUString::createFromAscii("dumdidum"), xDummyInterface,
                makeAny( xDummyInterface ) );
        aData.Sequence = Sequence<test::TestElement >( (const test::TestElement *)&aData, 1 );
        OSL_ENSURE( aData.Any == xDummyInterface, "### unexpected any!" );
        OSL_ENSURE( !(aData.Any != xDummyInterface), "### unexpected any!" );

        // aData complete ==> never touched again
        //================================================================================

        // this is a manually copy of aData for first setting...
        test::TestData aSetData0( aData ); // copy ctor
        // assignment
        test::TestData aSetData1 = aSetData0;

        test::TestData aSetData;
        assign( (test::TestElement &)aSetData,
                aSetData1.Bool, aSetData1.Char, aSetData1.Byte, aSetData1.Short, aSetData1.UShort,
                aSetData1.Long, aSetData1.ULong, aSetData1.Hyper, aSetData1.UHyper,
                aSetData1.Float, aSetData1.Double,
                aSetData1.Enum, aSetData1.String, aSetData1.Interface, aSetData1.Any );
        // switch over to new sequence allocation
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

            OSL_ASSERT( equals( aData, aRet ) && equals( aData, aRet2 ) );
        }
        {
            test::SmallStruct aIn(1, 2);
            test::SmallStruct aOut = xLBT->echoSmallStruct(aIn);
            OSL_ASSERT( memcmp(&aIn, &aOut, sizeof(test::SmallStruct)) == 0 );
        }
        {
            test::MediumStruct aIn(1, 2, 3, 4);
            test::MediumStruct aOut = xLBT->echoMediumStruct(aIn);
            OSL_ASSERT( memcmp(&aIn, &aOut, sizeof(test::MediumStruct)) == 0 );
        }
        {
            test::BigStruct aIn(1, 2, 3, 4, 5, 6, 7, 8);
            test::BigStruct aOut = xLBT->echoBigStruct(aIn);
            OSL_ASSERT( memcmp(&aIn, &aOut, sizeof(test::BigStruct)) == 0 );
        }
        {
            test::AllFloats aIn(1.1, 2.2, 3.3, 4.4);
            test::AllFloats aOut = xLBT->echoAllFloats(aIn);
            return( memcmp(&aIn, &aOut, sizeof(test::AllFloats)) == 0 );
        }
    }
    return sal_False;
}

//__________________________________________________________________________________________________
test::TestData Test_Impl::raiseException( sal_Bool& /*bBool*/, sal_Unicode& /*cChar*/, sal_Int8& /*nByte*/, sal_Int16& /*nShort*/, sal_uInt16& /*nUShort*/, sal_Int32& /*nLong*/, sal_uInt32& /*nULong*/, sal_Int64& /*nHyper*/, sal_uInt64& /*nUHyper*/, float& /*fFloat*/, double& /*fDouble*/, test::TestEnum& /*eEnum*/, ::rtl::OUString& /*aString*/, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& /*xInterface*/, ::com::sun::star::uno::Any& /*aAny*/, ::com::sun::star::uno::Sequence< test::TestElement >& /*aSequence*/, test::TestData& /*aStruct*/ )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    lang::IllegalArgumentException aExc;
    aExc.ArgumentPosition = 5;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
}
//__________________________________________________________________________________________________
sal_Int32 Test_Impl::getRuntimeException() throw(::com::sun::star::uno::RuntimeException)
{
    lang::DisposedException aExc;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
}
//__________________________________________________________________________________________________
void Test_Impl::setRuntimeException( sal_Int32 /*_runtimeexception*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    lang::DisposedException aExc;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
}

static void raising1( const Reference< XLanguageBindingTest > & xLBT )
{
    test::TestData aRet, aRet2;
    xLBT->raiseException(
        aRet.Bool, aRet.Char, aRet.Byte, aRet.Short, aRet.UShort,
        aRet.Long, aRet.ULong, aRet.Hyper, aRet.UHyper, aRet.Float, aRet.Double,
        aRet.Enum, aRet.String, aRet.Interface, aRet.Any, aRet.Sequence, aRet2 );
}
static void raising2( const Reference< XLanguageBindingTest > & xLBT )
{
    try
    {
        raising1( xLBT );
    }
    catch (RuntimeException &)
    {
    }
    catch (...)
    {
        throw;
    }
}
//==================================================================================================
sal_Bool raiseException( const Reference< XLanguageBindingTest > & xLBT )
{
    try
    {
        xLBT->getRuntimeException();
    }
    catch (lang::DisposedException & exc)
    {
        OSL_ENSURE( exc.Context == xLBT->getInterface() &&
                    exc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dum dum dum ich tanz im kreis herum...") ),
                    "### unexpected exception content!" );
    }
    catch (RuntimeException &)
    {
    }
    catch (Exception &)
    {
        return sal_False;
    }
    catch (...)
    {
        return sal_False;
    }

    sal_Int32 nCount = 0;
    try
    {
        try
        {
            try
            {
                raising2( xLBT );
            }
            catch (RuntimeException &)
            {
            }
            catch (lang::IllegalArgumentException aExc)
            {
                ++nCount;
                OSL_ENSURE( aExc.ArgumentPosition == 5 &&
                             aExc.Context == xLBT->getInterface() &&
                             aExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dum dum dum ich tanz im kreis herum...") ),
                             "### unexpected exception content!" );

                /** it is certain, that the RuntimeException testing will fail, if no */
                xLBT->getRuntimeException();
            }
        }
        catch (const RuntimeException & rExc)
        {
            ++nCount;
            OSL_ENSURE( rExc.Context == xLBT->getInterface() &&
                         rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dum dum dum ich tanz im kreis herum...") ),
                         "### unexpected exception content!" );

            /** it is certain, that the RuntimeException testing will fail, if no */
            xLBT->setRuntimeException( 0xcafebabe );
        }
        catch (lang::IllegalArgumentException &)
        {
        }
    }
    catch (Exception & rExc)
    {
        ++nCount;
        OSL_ENSURE( rExc.Context == xLBT->getInterface() &&
                     rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dum dum dum ich tanz im kreis herum...") ),
                     "### unexpected exception content!" );
        return (nCount == 3 &&
                rExc.Context == xLBT->getInterface() &&
                rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dum dum dum ich tanz im kreis herum...") ));
    }
    return sal_False;
}

//==================================================================================================
static void checkInvalidInterfaceQuery(
    Reference< XInterface > const & xObj )
{
    try
    {
        Any aRet( xObj->queryInterface( ::getCppuType( (const lang::IllegalArgumentException *)0 ) ) );
        OSL_ASSERT( ! aRet.hasValue() );
    }
    catch (RuntimeException &)
    {
    }
    try
    {
        Reference< lang::XComponent > xComp( xObj, UNO_QUERY_THROW );
        OSL_ASSERT( 0 );
    }
    catch (RuntimeException & /*exc*/)
    {
//         OString str( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
//         OSL_TRACE( str.getStr() );
    }
}

//==================================================================================================
static bool perform_test(
    Reference< XLanguageBindingTest > const & xObj,
    Reference< XInterface > const & xDummy )
{
    checkInvalidInterfaceQuery( xObj );

    if (performTest( xObj, xDummy ))
    {
        ::fprintf( stderr, "> invocation test succeeded!\n" );
        if (raiseException( xObj ))
        {
            ::fprintf( stderr, "> exception test succeeded!\n" );
            return true;
        }
        else
        {
            ::fprintf( stderr, "> exception test failed!\n" );
        }
    }

    ::fprintf( stderr, "> dynamic invocation test failed!\n" );
    return false;
}

//==================================================================================================
void test_CppBridge(void)
{
    // C++-UNO test
    {
    TestDummy * p = new TestDummy();
    Reference< XInterface > xDummy( *p );
    {
        Test_Impl * p2 = new Test_Impl();
        Reference< XLanguageBindingTest > xOriginal( p2 );
        checkInvalidInterfaceQuery( xOriginal );
        {
            const char * pExtraMapping = "";

            Reference< XLanguageBindingTest > xMapped;
            {
            uno_Interface * pUnoI = 0;

            OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
            OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );

            uno_Environment * pCppEnv = 0;
            uno_Environment * pUnoEnv = 0;
            ::uno_getEnvironment( &pCppEnv, aCppEnvTypeName.pData, 0 );
            ::uno_getEnvironment( &pUnoEnv, aUnoEnvTypeName.pData, 0 );

            // C++ -> UNO
            Mapping mapping( pCppEnv, pUnoEnv );
            mapping.mapInterface( (void **)&pUnoI, xOriginal.get(), ::getCppuType( &xOriginal ) );

#ifdef EXTRA_MAPPING
            // UNO -> ano C++a
            ::uno_createEnvironment( &pCppEnv, aCppEnvTypeName.pData, 0 );
            mapping = Mapping( pUnoEnv, pCppEnv );
            mapping.mapInterface( (void **)&xMapped, pUnoI, ::getCppuType( &xMapped ) );
            // ano C++a -> ano UNOa
            ::uno_createEnvironment( &pUnoEnv, aUnoEnvTypeName.pData, 0 );
            mapping = Mapping( pCppEnv, pUnoEnv );
            mapping.mapInterface( (void **)&pUnoI, xMapped.get(), ::getCppuType( &xMapped ) );
            pExtraMapping = " <-> c++ <-> uno";
#endif

            // ano UNOa -> ano C++b
            ::uno_createEnvironment( &pCppEnv, aCppEnvTypeName.pData, 0 );
            mapping = Mapping( pUnoEnv, pCppEnv );
            mapping.mapInterface( (void **)&xMapped, pUnoI, ::getCppuType( &xMapped ) );
            (*pUnoI->release)( pUnoI );
            (*pCppEnv->release)( pCppEnv );
            (*pUnoEnv->release)( pUnoEnv );
            }

            if (perform_test( xMapped, xDummy ))
            {
                ::fprintf( stderr, "> C++-UNO test (c++ <-> uno%s <-> c++ [component impl]) succeeded!\n", pExtraMapping );
            }
            else
            {
                ::fprintf( stderr, "> C++-UNO test (c++ <-> uno%s <-> c++ [component impl]) failed!\n", pExtraMapping );
                exit( 1 );
            }
        }
        OSL_ENSURE( p2->getRefCount() == 1, "### test object ref count > 1 !" );
    }
    OSL_ENSURE( p->getRefCount() == 1, "### dummy object ref count > 1 !" );
    }
}

//==================================================================================================
void test_CBridge(void)
{
    // C-UNO test
    {
    TestDummy * p = new TestDummy();
    Reference< XInterface > xDummy( *p );
    {
        Test_Impl * p2 = new Test_Impl();
        Reference< XLanguageBindingTest > xOriginal( p2 );
        checkInvalidInterfaceQuery( xOriginal );
        {
            Reference< XLanguageBindingTest > xMapped;
            {
            uno_Interface * pUnoI2 = 0;
            void * pC = 0;
            uno_Interface * pUnoI1 = 0;

            OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
            OUString aCEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_C) );
            OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );

            // C++ -> UNO
            uno_Environment * pCppEnv = 0;
            uno_Environment * pUnoEnv = 0;
            ::uno_getEnvironment( &pCppEnv, aCppEnvTypeName.pData, 0 );
            ::uno_getEnvironment( &pUnoEnv, aUnoEnvTypeName.pData, 0 );
            Mapping aCpp2Uno( pCppEnv, pUnoEnv );
            aCpp2Uno.mapInterface( (void **)&pUnoI1, xOriginal.get(), ::getCppuType( &xOriginal ) );
            (*pCppEnv->release)( pCppEnv );

            // UNO -> C
            uno_Environment * pCEnv = 0;
            ::uno_getEnvironment( &pCEnv, aCEnvTypeName.pData, 0 );
            Mapping aUno2C( pUnoEnv, pCEnv );
            aUno2C.mapInterface( &pC, pUnoI1, ::getCppuType( &xOriginal ) );
            (*pUnoI1->release)( pUnoI1 );
            (*pUnoEnv->release)( pUnoEnv );

            // C -> ano UNO
            uno_Environment * pAnoUnoEnv = 0;
            ::uno_createEnvironment( &pAnoUnoEnv, aUnoEnvTypeName.pData, 0 ); // anonymous
            Mapping aC2Uno( pCEnv, pAnoUnoEnv );
            aC2Uno.mapInterface( (void **)&pUnoI2, pC, ::getCppuType( &xOriginal ) );
            (*pCEnv->pExtEnv->releaseInterface)( pCEnv->pExtEnv, pC );
            (*pCEnv->release)( pCEnv );

            // ano UNO -> ano C++
            uno_Environment * pAnoCppEnv = 0;
            ::uno_createEnvironment( &pAnoCppEnv, aCppEnvTypeName.pData, 0 );
            Mapping aUno2Cpp( pAnoUnoEnv, pAnoCppEnv );
            (*pAnoCppEnv->release)( pAnoCppEnv );
            (*pAnoUnoEnv->release)( pAnoUnoEnv );
            aUno2Cpp.mapInterface( (void **)&xMapped, pUnoI2, ::getCppuType( &xOriginal ) );
            (*pUnoI2->release)( pUnoI2 );
            }

            if (perform_test( xMapped, xDummy ))
            {
                ::fprintf( stderr, "> C-UNO test (c++ <-> uno <-> c <-> uno <-> c++ [component impl]) succeeded!\n" );
            }
            else
            {
                ::fprintf( stderr, "> C-UNO test (c++ <-> uno <-> c <-> uno <-> c++ [component impl]) failed!\n" );
                exit( 1 );
            }
        }
        OSL_ENSURE( p->getRefCount() == 1, "### test object ref count > 1 !" );
    }
    OSL_ENSURE( p->getRefCount() == 1, "### dummy object ref count > 1 !" );
    }
}
#if 0
//==================================================================================================
extern "C" com_sun_star_uno_XInterface* SAL_CALL createTestObject();

void test_CBridge2(void)
{
    // C-UNO test
    {
        TestDummy * p = new TestDummy();
        Reference< XInterface > xDummy( *p );
        {
            com_sun_star_uno_XInterface* pXIface = createTestObject();
            test_XLanguageBindingTest* pXLBTest = 0;
            uno_Any aExc;
            Reference< XLanguageBindingTest > xMapped;

            OSL_ENSURE( pXIface != 0, "create test object failed\n");

            /* Get interface XFoo2 */
            if (CUNO_EXCEPTION_OCCURED( CUNO_CALL(pXIface)->queryInterface( pXIface, &aExc, (com_sun_star_uno_XInterface**)&pXLBTest, ::getCppuType( &xMapped ).getTypeLibType()) ))
            {
                uno_any_destruct( &aExc, 0 );
            }
            OSL_ENSURE( pXLBTest != 0, "query_Interface XLanguageBindingTest failed\n");

            Mapping aC2Cpp(
                OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_C) ),
                OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) );
            aC2Cpp.mapInterface( (void **)&xMapped, pXLBTest, ::getCppuType( &xMapped ) );

            OSL_ENSURE( xMapped.is(), "mapping interface failed\n");

            if (perform_test( xMapped, xDummy ))
            {
                ::fprintf( stderr, "> second C-UNO test (c++ <-> uno <-> c [component impl]) succeeded!\n" );
            }
            else
            {
                ::fprintf( stderr, "> second C-UNO test (c++ <-> uno <-> c [component impl]) failed!\n" );
                exit( 1 );
            }


            CUNO_CALL(pXIface)->release( pXIface );
            CUNO_CALL(pXLBTest)->release( (com_sun_star_uno_XInterface *)pXLBTest );
        }
        OSL_ENSURE( p->getRefCount() == 1, "### dummy object ref count > 1 !" );
    }

}
#endif

