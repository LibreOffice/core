/*************************************************************************
 *
 *  $RCSfile: test_di.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 13:28:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <osl/time.h>
//
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <uno/data.h>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppu/macros.hxx>
#include <test/XLanguageBindingTest.hpp>

#include <cppuhelper/implbase1.hxx>


using namespace test;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace osl;
using namespace rtl;


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
class Test_Impl : public cppu::WeakImplHelper1< XLanguageBindingTest >
{
    test::TestData _aData, _aStructData;

public:
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
                0x123456789abcdef0, 0xfedcba9876543210,
                (float)17.0815, 3.1415926359, TestEnum_LOLA,
                OUString::createFromAscii("dumdidum"), xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

        OSL_ENSURE( aData.Any == xI, "### unexpected any!" );
        OSL_ENSURE( !(aData.Any != xI), "### unexpected any!" );

        aData.Sequence = Sequence<test::TestElement >( (const test::TestElement *)&aData, 1 );
        // aData complete
        //================================================================================

        // this is a manually copy of aData for first setting...
        test::TestData aSetData;

        assign( (test::TestElement &)aSetData,
                aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort,
                aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, aData.Double,
                aData.Enum, aData.String, xI,
                Any( &xI, ::getCppuType( (const Reference<XInterface > *)0 ) ) );

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

//__________________________________________________________________________________________________
test::TestData Test_Impl::raiseException( sal_Bool& bBool, sal_Unicode& cChar, sal_Int8& nByte, sal_Int16& nShort, sal_uInt16& nUShort, sal_Int32& nLong, sal_uInt32& nULong, sal_Int64& nHyper, sal_uInt64& nUHyper, float& fFloat, double& fDouble, test::TestEnum& eEnum, ::rtl::OUString& aString, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInterface, ::com::sun::star::uno::Any& aAny, ::com::sun::star::uno::Sequence< test::TestElement >& aSequence, test::TestData& aStruct )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    IllegalArgumentException aExc;
    aExc.ArgumentPosition = 5;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
    return test::TestData();
}
//__________________________________________________________________________________________________
sal_Int32 Test_Impl::getRuntimeException() throw(::com::sun::star::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
    return 0;
}
//__________________________________________________________________________________________________
void Test_Impl::setRuntimeException( sal_Int32 _runtimeexception ) throw(::com::sun::star::uno::RuntimeException)
{
    RuntimeException aExc;
    aExc.Message          = OUString::createFromAscii( "dum dum dum ich tanz im kreis herum..." );
    aExc.Context          = getInterface();
    throw aExc;
}
//==================================================================================================
sal_Bool raiseException( const Reference< XLanguageBindingTest > & xLBT )
{
    sal_Int32 nCount = 0;
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
            }
            catch (IllegalArgumentException aExc)
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
void test_di(void)
{
    Test_Impl * p = new Test_Impl();
    Reference< XInterface > x( *p );
    {
    Reference<XLanguageBindingTest > xOriginal( x, UNO_QUERY );
    Reference<XLanguageBindingTest > xLBT;

    uno_Environment * pCppEnv1 = 0;
    uno_Environment * pCppEnv2 = 0;

    OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    uno_getEnvironment( &pCppEnv1, aCppEnvTypeName.pData, 0 );
    uno_createEnvironment( &pCppEnv2, aCppEnvTypeName.pData, 0 ); // anonymous

    Mapping aMapping( pCppEnv1, pCppEnv2, OUString::createFromAscii("prot") );
    aMapping.mapInterface( (void **)&xLBT, xOriginal.get(), ::getCppuType( &xOriginal ) );
    OSL_ENSURE( aMapping.is(), "### cannot get mapping!" );

    (*pCppEnv2->release)( pCppEnv2 );
    (*pCppEnv1->release)( pCppEnv1 );

    OSL_ASSERT( xLBT.is() );

    if (xLBT.is() && performTest( xLBT ))
    {
        if (raiseException( xLBT ))
        {
            printf( "> dynamic invocation test succeeded!\n" );
        }
        else
        {
            printf( "> exception test failed!\n" );
        }
    }
    else
    {
        printf( "> dynamic invocation test failed!\n" );
    }
    }
}
