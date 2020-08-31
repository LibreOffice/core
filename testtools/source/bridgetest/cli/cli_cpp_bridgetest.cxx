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


#using <mscorlib.dll>
#using <System.dll>
#using <cli_basetypes.dll>
#using <cli_uretypes.dll>
#using <cli_ure.dll>
#using <cli_types_bridgetest.dll>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Reflection;
using namespace System::Threading;
using namespace uno;
using namespace uno::util;
using namespace unoidl::com::sun::star::uno;
using namespace unoidl::com::sun::star::lang;
using namespace unoidl::test::testtools::bridgetest;
namespace foo
{
    public __gc  __interface MyInterface
    {
    };
}

namespace cpp_bridgetest
{
    __gc class ORecursiveCall: public WeakBase, public XRecursiveCall
    {
        public:
        void  callRecursivly(XRecursiveCall * xCall, int nToCall)
        {
            Monitor::Enter(this);
            try
            {
                {
                    if (nToCall > 0)
                    {
                        nToCall --;
                        xCall->callRecursivly(this, nToCall);
                    }
                }
            }
            __finally
            {
                Monitor::Exit(this);
            }

        }
    };

public __gc class Constants
{
public:
    static String* STRING_TEST_CONSTANT  = new String(S"\" paco\' chorizo\\\' \"\'");
};

public __gc class BridgeTest : public WeakBase, public XMain
{
    static bool compareData(Object* val1, Object* val2)
    {
        if (val1 == 0 && val2 == 0 || val1 == val2)
            return true;
        if ((val1 == 0 && val2 != 0) ||
            (val1 != 0 && val2 == 0) || val1->GetType() != val2->GetType())
            return false;

        bool ret = false;
        Type* t1  = val1->GetType();
            //Sequence
        if (t1->IsArray)
        {
            ret = compareSequence(static_cast<Array*>(val1),
                                  static_cast<Array*>(val2));
        }
            //String
        else if (t1 == __typeof(String))
        {
            ret = val1->Equals(val2);
        }
            // Interface implementation
        else if (t1->GetInterfaces()->Length > 0 && ! t1->IsValueType)
        {
            ret = val1 == val2;
        }
            // Struct
        else if ( ! t1->IsValueType)
        {
            ret = compareStruct(val1, val2);
        }
        else if (t1 == __typeof(Any))
        {
            Any a1 = (Any) val1;
            Any a2 = (Any) val2;
            ret = a1.Type == a2.Type && compareData(a1.Value, a2.Value);
        }
        else
        {
            //Any, enum, int, bool char, float, double etc.
            ret = val1->Equals(val2);
        }
        return ret;
    }

    // Arrays have only one dimension
    static bool compareSequence(Array* ar1, Array* ar2)
    {
        Debug::Assert(ar1 != 0 && ar2 != 0);
        Type* t1 = ar1->GetType();
        Type* t2 = ar2->GetType();

        if (!(ar1->Rank == 1 && ar2->Rank == 1
            && ar1->Length == ar2->Length && t1->GetElementType() == t2->GetElementType()))
            return false;

        //arrays have same rank and size and element type.
        int len  = ar1->Length;
        bool ret = true;
        for (int i = 0; i < len; i++)
        {
            if (compareData(ar1->GetValue(i), ar2->GetValue(i)) == false)
            {
                ret = false;
                break;
            }
        }
        return ret;
    }

    static bool compareStruct(Object* val1, Object* val2)
    {
        Debug::Assert(val1 != 0 && val2 != 0);
        Type* t1 = val1->GetType();
        Type* t2 = val2->GetType();
        if (t1 != t2)
            return false;
        FieldInfo* fields[] = t1->GetFields();
        int cFields = fields->Length;
        bool ret = true;
        for (int i = 0; i < cFields; i++)
        {
            Object* fieldVal1 = fields[i]->GetValue(val1);
            Object* fieldVal2 = fields[i]->GetValue(val2);
            if ( ! compareData(fieldVal1, fieldVal2))
            {
                ret = false;
                break;
            }
        }
        return ret;
    }

    static bool check( bool b , String* message )
    {
        if ( ! b)
        Console::WriteLine("{0} failed\n" , message);
        return b;
    }

    static bool equals(TestElement* rData1, TestElement*  rData2)
    {
        check( rData1->Bool == rData2->Bool, "### bool does not match!" );
        check( rData1->Char == rData2->Char, "### char does not match!" );
        check( rData1->Byte == rData2->Byte, "### byte does not match!" );
        check( rData1->Short == rData2->Short, "### short does not match!" );
        check( rData1->UShort == rData2->UShort, "### unsigned short does not match!" );
        check( rData1->Long == rData2->Long, "### long does not match!" );
        check( rData1->ULong == rData2->ULong, "### unsigned long does not match!" );
        check( rData1->Hyper == rData2->Hyper, "### hyper does not match!" );
        check( rData1->UHyper == rData2->UHyper, "### unsigned hyper does not match!" );
        check( rData1->Float == rData2->Float, "### float does not match!" );
        check( rData1->Double == rData2->Double, "### double does not match!" );
        check( rData1->Enum == rData2->Enum, "### enum does not match!" );
        check( rData1->String == rData2->String, "### string does not match!" );
        check( rData1->Interface == rData2->Interface, "### interface does not match!" );
        check( compareData(__box(rData1->Any), __box(rData2->Any)), "### any does not match!" );

        return (rData1->Bool == rData2->Bool &&
                rData1->Char == rData2->Char &&
                rData1->Byte == rData2->Byte &&
                rData1->Short == rData2->Short &&
                rData1->UShort == rData2->UShort &&
                rData1->Long == rData2->Long &&
                rData1->ULong == rData2->ULong &&
                rData1->Hyper == rData2->Hyper &&
                rData1->UHyper == rData2->UHyper &&
                rData1->Float == rData2->Float &&
                rData1->Double == rData2->Double &&
                rData1->Enum == rData2->Enum &&
                rData1->String == rData2->String &&
                rData1->Interface == rData2->Interface &&
                compareData(__box(rData1->Any), __box(rData2->Any)));
    }

static void assign( TestElement* rData,
                    bool bBool, Char cChar, Byte nByte,
                    Int16 nShort, UInt16 nUShort,
                    Int32 nLong, UInt32 nULong,
                    Int64 nHyper, UInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, String* rStr,
                    Object* xTest,
                    uno::Any rAny )
{
    rData->Bool = bBool;
    rData->Char = cChar;
    rData->Byte = nByte;
    rData->Short = nShort;
    rData->UShort = nUShort;
    rData->Long = nLong;
    rData->ULong = nULong;
    rData->Hyper = nHyper;
    rData->UHyper = nUHyper;
    rData->Float = fFloat;
    rData->Double = fDouble;
    rData->Enum = eEnum;
    rData->String = rStr;
    rData->Interface = xTest;
    rData->Any = rAny;
}

static void assign( TestDataElements* rData,
                    bool bBool, Char cChar, Byte nByte,
                    Int16 nShort, UInt16 nUShort,
                    Int32 nLong, UInt32 nULong,
                    Int64 nHyper, UInt64 nUHyper,
                    float fFloat, double fDouble,
                    TestEnum eEnum, String* rStr,
                    Object* xTest,
                    Any rAny,
                    TestElement* rSequence[])
{
    assign( static_cast<TestElement*>(rData),
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum, rStr, xTest, rAny );
    rData->Sequence = rSequence;
}

static bool testAny(Type* typ, Object*  value, XBridgeTest* xLBT )
{
    Any any;
    if (typ == 0)
        any = Any(value->GetType(), value);
    else
        any = Any(typ, value);

    Any any2 = xLBT->transportAny(any);
    bool ret = compareData(__box(any), __box(any2));
    if (!ret)
    {
        Console::WriteLine("any is different after roundtrip: in {0}, out {1}\n",
                          any.Type->FullName, any2.Type->FullName);
    }
    return ret;
}


static bool performAnyTest(XBridgeTest* xLBT,  TestDataElements* data)
{
    bool bReturn = true;
    bReturn = testAny( 0, __box(data->Byte), xLBT ) && bReturn;
    bReturn = testAny( 0, __box(data->Short), xLBT ) && bReturn;
    bReturn = testAny(  0, __box(data->UShort), xLBT ) && bReturn;
    bReturn = testAny(  0, __box(data->Long), xLBT ) && bReturn;
    bReturn = testAny(  0, __box(data->ULong), xLBT ) && bReturn;
    bReturn = testAny(  0, __box(data->Hyper), xLBT ) && bReturn;
    bReturn = testAny(  0, __box(data->UHyper), xLBT ) && bReturn;
    bReturn = testAny( 0, __box(data->Float), xLBT ) && bReturn;
    bReturn = testAny( 0, __box(data->Double),xLBT ) && bReturn;
    bReturn = testAny( 0, __box(data->Enum), xLBT ) && bReturn;
    bReturn = testAny( 0, data->String,xLBT ) && bReturn;
    bReturn = testAny(__typeof(XWeak), data->Interface,xLBT ) && bReturn;
    bReturn = testAny(0, data, xLBT ) && bReturn;

    {
        Any a1(true);
        Any a2 = xLBT->transportAny( a1 );
        bReturn = compareData(__box(a2), __box(a1)) && bReturn;
    }

    {
        Any a1('A');
        Any a2 = xLBT->transportAny(a1);
        bReturn = compareData( __box(a2), __box(a1)) && bReturn;
    }
    return bReturn;
}

static bool performSequenceOfCallTest(XBridgeTest* xLBT)
{
    int i,nRounds;
    int nGlobalIndex = 0;
    const int nWaitTimeSpanMUSec = 10000;
    for( nRounds = 0 ; nRounds < 10 ; nRounds ++ )
    {
        for( i = 0 ; i < nRounds ; i ++ )
        {
            // fire oneways
            xLBT->callOneway(nGlobalIndex, nWaitTimeSpanMUSec);
            nGlobalIndex++;
        }

        // call synchron
        xLBT->call(nGlobalIndex, nWaitTimeSpanMUSec);
        nGlobalIndex++;
    }
     return xLBT->sequenceOfCallTestPassed();
}


static bool performRecursiveCallTest(XBridgeTest*  xLBT)
{
    xLBT->startRecursiveCall(new ORecursiveCall(), 50);
    // on failure, the test would lock up or crash
    return true;
}

static bool performQueryForUnknownType(XBridgeTest* xLBT)
{
    bool bRet = false;
    // test queryInterface for an unknown type
    try
    {
        __try_cast<foo::MyInterface*>(xLBT);
    }
    catch( System::InvalidCastException*)
    {
        bRet = true;
    }

    return bRet;
}


static bool performTest(XBridgeTest* xLBT)
{
    check( xLBT != 0, "### no test interface!" );
    bool bRet = true;
    if (xLBT != 0)
    {
        // this data is never ever granted access to by calls other than equals(), assign()!
        TestDataElements* aData = new TestDataElements(); // test against this data

        Object* xI= new WeakBase();

        Any aAny( __typeof(Object), xI);
        assign( static_cast<TestElement*>(aData),
                true, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
                0x123456789abcdef0, 0xfedcba9876543210,
                17.0815f, 3.1415926359, TestEnum::LOLA,
                Constants::STRING_TEST_CONSTANT, xI,
                aAny);

        bRet = check( aData->Any.Value == xI, "### unexpected any!" ) && bRet;
        bRet = check( !(aData->Any.Value != xI), "### unexpected any!" ) && bRet;

        aData->Sequence = new TestElement*[2];
        aData->Sequence[0] = new TestElement(
            aData->Bool, aData->Char, aData->Byte, aData->Short,
            aData->UShort, aData->Long, aData->ULong,
            aData->Hyper, aData->UHyper, aData->Float,
            aData->Double, aData->Enum, aData->String,
            aData->Interface, aData->Any); //(TestElement) aData;
        aData->Sequence[1] = new TestElement(); //is empty

        // aData complete

        // this is a manually copy of aData for first setting...
        TestDataElements* aSetData = new TestDataElements;
        Any aAnySet(__typeof(Object), xI);
        assign( static_cast<TestElement*>(aSetData),
                aData->Bool,
                aData->Char,
                aData->Byte,
                aData->Short,
                aData->UShort,
                aData->Long, aData->ULong, aData->Hyper, aData->UHyper, aData->Float, aData->Double,
                aData->Enum,
                aData->String,
                xI,
                aAnySet);

        aSetData->Sequence = new TestElement*[2];
        aSetData->Sequence[0] = new TestElement(
            aSetData->Bool, aSetData->Char, aSetData->Byte, aSetData->Short,
            aSetData->UShort, aSetData->Long, aSetData->ULong,
            aSetData->Hyper, aSetData->UHyper, aSetData->Float,
            aSetData->Double, aSetData->Enum, aSetData->String,
            aSetData->Interface, aSetData->Any); //TestElement) aSetData;
        aSetData->Sequence[1] = new TestElement(); // empty struct

        xLBT->setValues(
                aSetData->Bool, aSetData->Char, aSetData->Byte, aSetData->Short, aSetData->UShort,
                aSetData->Long, aSetData->ULong, aSetData->Hyper, aSetData->UHyper, aSetData->Float, aSetData->Double,
                aSetData->Enum, aSetData->String, aSetData->Interface, aSetData->Any, aSetData->Sequence, aSetData );

        {
        TestDataElements* aRet = new TestDataElements();
        TestDataElements* aRet2 = new TestDataElements();
        xLBT->getValues(
            & aRet->Bool, & aRet->Char, & aRet->Byte, & aRet->Short, & aRet->UShort,
            & aRet->Long, & aRet->ULong, & aRet->Hyper, & aRet->UHyper,
            & aRet->Float, & aRet->Double, & aRet->Enum, & aRet->String,
            & aRet->Interface, & aRet->Any, & aRet->Sequence, & aRet2 );

        bRet = check( compareData( aData, aRet ) && compareData( aData, aRet2 ) , "getValues test") && bRet;

        // set last retrieved values
        TestDataElements* aSV2ret = xLBT->setValues2(
            & aRet->Bool, & aRet->Char, & aRet->Byte, & aRet->Short, & aRet->UShort,
            & aRet->Long, & aRet->ULong, & aRet->Hyper, & aRet->UHyper, & aRet->Float,
            & aRet->Double, & aRet->Enum, & aRet->String, & aRet->Interface, & aRet->Any,
            & aRet->Sequence, & aRet2 );

        // check inout sequence order
        // => inout sequence parameter was switched by test objects
        TestElement* temp = aRet->Sequence[ 0 ];
        aRet->Sequence[ 0 ] = aRet->Sequence[ 1 ];
        aRet->Sequence[ 1 ] = temp;

        bRet = check(
            compareData( aData, aSV2ret ) && compareData( aData, aRet2 ),
            "getValues2 test") && bRet;
        }
        {
        TestDataElements* aRet = new TestDataElements();
        TestDataElements* aRet2 = new TestDataElements();
        TestDataElements* aGVret = xLBT->getValues(
            & aRet->Bool, & aRet->Char, & aRet->Byte, & aRet->Short,
            & aRet->UShort, & aRet->Long, & aRet->ULong, & aRet->Hyper,
            & aRet->UHyper, & aRet->Float, & aRet->Double, & aRet->Enum,
            & aRet->String, & aRet->Interface, & aRet->Any, & aRet->Sequence,
            & aRet2 );

        bRet = check( compareData( aData, aRet ) && compareData( aData, aRet2 ) && compareData( aData, aGVret ), "getValues test" ) && bRet;

        // set last retrieved values
        xLBT->Bool = aRet->Bool;
        xLBT->Char = aRet->Char;
        xLBT->Byte = aRet->Byte;
        xLBT->Short = aRet->Short;
        xLBT->UShort = aRet->UShort;
        xLBT->Long = aRet->Long;
        xLBT->ULong = aRet->ULong;
        xLBT->Hyper = aRet->Hyper;
        xLBT->UHyper = aRet->UHyper;
        xLBT->Float = aRet->Float;
        xLBT->Double = aRet->Double;
        xLBT->Enum = aRet->Enum;
        xLBT->String = aRet->String;
        xLBT->Interface = aRet->Interface;
        xLBT->Any = aRet->Any;
        xLBT->Sequence = aRet->Sequence;
        xLBT->Struct = aRet2;
        }
        {
        TestDataElements* aRet = new TestDataElements();
        TestDataElements* aRet2 = new TestDataElements();
        aRet->Hyper = xLBT->Hyper;
        aRet->UHyper = xLBT->UHyper;
        aRet->Float = xLBT->Float;
        aRet->Double = xLBT->Double;
        aRet->Byte = xLBT->Byte;
        aRet->Char = xLBT->Char;
        aRet->Bool = xLBT->Bool;
        aRet->Short = xLBT->Short;
        aRet->UShort = xLBT->UShort;
        aRet->Long = xLBT->Long;
        aRet->ULong = xLBT->ULong;
        aRet->Enum = xLBT->Enum;
        aRet->String = xLBT->String;
        aRet->Interface = xLBT->Interface;
        aRet->Any = xLBT->Any;
        aRet->Sequence = xLBT->Sequence;
        aRet2 = xLBT->Struct;

        bRet = check( compareData( aData, aRet ) && compareData( aData, aRet2 ) , "struct comparison test") && bRet;

        bRet = check(performSequenceTest(xLBT), "sequence test") && bRet;

        // any test
        bRet = check( performAnyTest( xLBT , aData ) , "any test" ) && bRet;

        // sequence of call test
        bRet = check( performSequenceOfCallTest( xLBT ) , "sequence of call test" ) && bRet;

        // recursive call test
        bRet = check( performRecursiveCallTest( xLBT ) , "recursive test" ) && bRet;

        bRet = (compareData( aData, aRet ) && compareData( aData, aRet2 )) && bRet ;

        // check setting of null reference
        xLBT->Interface = 0;
        aRet->Interface = xLBT->Interface;
        bRet = (aRet->Interface == 0) && bRet;

        }


    }
        return bRet;
 }
static bool performSequenceTest(XBridgeTest* xBT)
{
    bool bRet = true;
    XBridgeTest2*  xBT2 = dynamic_cast<XBridgeTest2*>(xBT);
    if ( xBT2 == 0)
        return false;

    // perform sequence tests (XBridgeTest2)
    // create the sequence which are compared with the results
    bool arBool __gc[] = new bool __gc [3];
    arBool[0] = true; arBool[1] = false; arBool[2] = true;
    Char  arChar[] = new  Char[3];
    arChar[0] = 'A'; arChar[1] = 'B'; arChar[2] = 'C';
    Byte arByte[] = new Byte[3];
    arByte[0] =  1; arByte[1] = 2; arByte[2] = 0xff;
    Int16 arShort[] = new Int16[3];
    arShort[0] = Int16::MinValue; arShort[1] = 1; arShort[2] = Int16::MaxValue;
    UInt16 arUShort[] = new UInt16[3];
    arUShort[0] = UInt16::MinValue; arUShort[1] = 1; arUShort[2] = UInt16::MaxValue;
    Int32 arLong[] = new Int32[3];
    arLong[0] = Int32::MinValue; arLong[1] = 1; arLong[2] = Int32::MaxValue;
    UInt32 arULong[] = new UInt32[3];
    arULong[0] = UInt32::MinValue; arULong[1] = 1; arULong[2] = UInt32::MaxValue;
    Int64 arHyper[] = new Int64[3];
    arHyper[0] = Int64::MinValue; arHyper[1] = 1; arHyper[2] = Int64::MaxValue;
    UInt64 arUHyper[] = new UInt64[3];
    arUHyper[0] = UInt64::MinValue; arUHyper[1] = 1;
    arUHyper[2] = UInt64::MaxValue;
    Single arFloat[] = new Single[3];
    arFloat[0] = 1.1f; arFloat[1] = 2.2f; arFloat[2] = 3.3f;
    Double arDouble[] = new Double[3];
    arDouble[0] = 1.11; arDouble[1] = 2.22; arDouble[2] = 3.33;
    String* arString[] = new String*[3];
    arString[0] = new String("String 1");
    arString[1] = new String("String 2");
    arString[2] = new String("String 3");

    Any arAny[] = new Any[3];
    arAny[0] = Any(true); arAny[1] = Any(11111); arAny[2] = Any(3.14);
    Object* arObject[] = new Object*[3];
    arObject[0] = new WeakBase(); arObject[1] =  new WeakBase();
    arObject[1] = new WeakBase();

    Console::WriteLine(new String("cli_cpp_bridgetest: Workaround for C++ compiler bug:"
        " using Array of Int32 instead of Array of enums w"));
    Int32 arEnum[] = new Int32[3];
    arEnum[0] = static_cast<Int32>(TestEnum::ONE);
    arEnum[1] = static_cast<Int32>(TestEnum::TWO);
    arEnum[2] = static_cast<Int32>(TestEnum::CHECK);

    TestElement* arStruct[] = new TestElement*[3];
    arStruct[0] = new TestElement(); arStruct[1] = new TestElement();
    arStruct[2] = new TestElement();
    assign( arStruct[0], true, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
             0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum::LOLA, Constants::STRING_TEST_CONSTANT, arObject[0],
            Any( __typeof(Object),  arObject[0]) );
    assign( arStruct[1], true, 'A', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
            0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum::TWO, Constants::STRING_TEST_CONSTANT, arObject[1],
            Any( __typeof(Object), arObject[1]) );
    assign( arStruct[2], true, 'B', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
            0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum::CHECK, Constants::STRING_TEST_CONSTANT, arObject[2],
            Any( __typeof(Object), arObject[2] ) );
    {
    Any seqAnyRet[] = xBT2->setSequenceAny(arAny);
    bRet = check( compareData(seqAnyRet, arAny), "sequence test") && bRet;
    Boolean seqBoolRet[] = xBT2->setSequenceBool(arBool);
    bRet = check( compareData(seqBoolRet, arBool), "sequence test") && bRet;
    Byte seqByteRet[] = xBT2->setSequenceByte(arByte);
    bRet = check( compareData(seqByteRet, arByte), "sequence test") && bRet;
    Char seqCharRet[] = xBT2->setSequenceChar(arChar);
    bRet = check( compareData(seqCharRet, arChar), "sequence test") && bRet;
    Int16 seqShortRet[] = xBT2->setSequenceShort(arShort);
    bRet = check( compareData(seqShortRet, arShort), "sequence test") && bRet;
    Int32 seqLongRet[] = xBT2->setSequenceLong(arLong);
    bRet = check( compareData(seqLongRet, arLong), "sequence test") && bRet;
    Int64 seqHyperRet[] = xBT2->setSequenceHyper(arHyper);
    bRet = check( compareData(seqHyperRet,arHyper), "sequence test") && bRet;
    Single seqFloatRet[] = xBT2->setSequenceFloat(arFloat);
    bRet = check( compareData(seqFloatRet, arFloat), "sequence test") && bRet;
    Double seqDoubleRet[] = xBT2->setSequenceDouble(arDouble);
    bRet = check( compareData(seqDoubleRet, arDouble), "sequence test") && bRet;
    xBT2->setSequenceEnum(arEnum);
    //comparing seqEnumRet with arEnum will fail since they are of different
    //types because of workaround. arEnum is Int32[].
    Console::WriteLine(new String("cli_cpp_bridgetest: Test omitted because "
        "of C++ compiler bug. XBridgeTest2::setSequenceEnum(sequence<TestEnum>)"));
    UInt16 seqUShortRet[] = xBT2->setSequenceUShort(arUShort);
    bRet = check( compareData(seqUShortRet, arUShort), "sequence test") && bRet;
    UInt32 seqULongRet[] = xBT2->setSequenceULong(arULong);
    bRet = check( compareData(seqULongRet, arULong), "sequence test") && bRet;
    UInt64 seqUHyperRet[] = xBT2->setSequenceUHyper(arUHyper);
    bRet = check( compareData(seqUHyperRet, arUHyper), "sequence test") && bRet;
    Object* seqObjectRet[] = xBT2->setSequenceXInterface(arObject);
    bRet = check( compareData(seqObjectRet, arObject), "sequence test") && bRet;
    String* seqStringRet[] = xBT2->setSequenceString(arString);
    bRet = check( compareData(seqStringRet, arString), "sequence test") && bRet;
    TestElement* seqStructRet[] = xBT2->setSequenceStruct(arStruct);
    bRet = check( compareData(seqStructRet, arStruct), "sequence test") && bRet;
    }
    {
    Console::WriteLine(new String("cli_cpp_bridgetest: no test of "
        "XBridgeTest2::setSequencesInOut and XBridgeTest2.setSequencesOut "
        "because jagged arrays are not supported by C++ compiler"));
    }
    {
    Any _arAny[] = new Any[0];
    Any seqAnyRet[] = xBT2->setSequenceAny(_arAny);
    bRet = check( compareData(seqAnyRet, _arAny), "sequence test") && bRet;
    Boolean _arBool[] = new Boolean[0];
    Boolean seqBoolRet[] = xBT2->setSequenceBool(_arBool);
    bRet = check( compareData(seqBoolRet, _arBool), "sequence test") && bRet;
    Byte _arByte[] = new Byte[0];
    Byte seqByteRet[] = xBT2->setSequenceByte(_arByte);
    bRet = check( compareData(seqByteRet, _arByte), "sequence test") && bRet;
    Char _arChar[] = new Char[0];
    Char seqCharRet[] = xBT2->setSequenceChar(_arChar);
    bRet = check( compareData(seqCharRet, _arChar), "sequence test") && bRet;
    Int16 _arShort[] = new Int16[0];
    Int16 seqShortRet[] = xBT2->setSequenceShort(_arShort);
    bRet = check( compareData(seqShortRet, _arShort), "sequence test") && bRet;
    Int32 _arLong[] = new Int32[0];
    Int32 seqLongRet[] = xBT2->setSequenceLong(_arLong);
    bRet = check( compareData(seqLongRet, _arLong), "sequence test") && bRet;
    Int64 _arHyper[] = new Int64[0];
    Int64 seqHyperRet[] = xBT2->setSequenceHyper(_arHyper);
    bRet = check( compareData(seqHyperRet, _arHyper), "sequence test") && bRet;
    Single _arFloat[] = new Single[0];
    Single  seqFloatRet[] = xBT2->setSequenceFloat(_arFloat);
    bRet = check( compareData(seqFloatRet, _arFloat), "sequence test") && bRet;
    Double _arDouble[] = new Double[0];
    Double seqDoubleRet[] = xBT2->setSequenceDouble(_arDouble);
    bRet = check( compareData(seqDoubleRet, _arDouble), "sequence test") && bRet;
    TestEnum _arEnum[] = new TestEnum[0];
    xBT2->setSequenceEnum(_arEnum);
    UInt16 _arUShort[] = new UInt16[0];
    UInt16 seqUShortRet[] = xBT2->setSequenceUShort(_arUShort);
    bRet = check( compareData(seqUShortRet, _arUShort), "sequence test") && bRet;
    UInt32 _arULong[] = new UInt32[0];
    UInt32 seqULongRet[] = xBT2->setSequenceULong(_arULong);
    bRet = check( compareData(seqULongRet, _arULong), "sequence test") && bRet;
    UInt64 _arUHyper[] = new UInt64[0];
    UInt64 seqUHyperRet[] = xBT2->setSequenceUHyper(_arUHyper);
    bRet = check( compareData(seqUHyperRet, _arUHyper), "sequence test") && bRet;
    Object* _arObject[] = new Object*[0];
    Object* seqObjectRet[] = xBT2->setSequenceXInterface(_arObject);
    bRet = check( compareData(seqObjectRet, _arObject), "sequence test") && bRet;
    String* _arString[] = new String*[0];
    String* seqStringRet[] = xBT2->setSequenceString(_arString);
    bRet = check( compareData(seqStringRet, _arString), "sequence test") && bRet;
    TestElement* _arStruct[] = new TestElement*[0];
    TestElement* seqStructRet[] = xBT2->setSequenceStruct(_arStruct);
    bRet = check( compareData(seqStructRet, _arStruct), "sequence test") && bRet;

    }
    return bRet;
}
/** Test the System::Object method on the proxy object
 */
static bool testObjectMethodsImplementation(XBridgeTest* xLBT)
{
    bool ret = false;
    Object* obj = new Object();
    XBridgeTestBase* xBase = dynamic_cast<XBridgeTestBase*>(xLBT);
    if (xBase == 0)
        return false;
    // Object.Equals
    ret = xLBT->Equals(obj) == false;
    ret = xLBT->Equals(xLBT) && ret;
    ret = Object::Equals(obj, obj) && ret;
    ret = Object::Equals(xLBT, xBase) && ret;
    //Object.GetHashCode
    // Don't know how to verify this. Currently it is not possible to get the object id from a proxy
    int nHash = xLBT->GetHashCode();
    ret = nHash == xBase->GetHashCode() && ret;

    //Object.ToString
    // Don't know how to verify this automatically.
    String* s = xLBT->ToString();
    ret = (s->Length > 0) && ret;
    return ret;
}


static bool raiseOnewayException(XBridgeTest* xLBT)
{
    bool bReturn = true;
    String* sCompare = Constants::STRING_TEST_CONSTANT;
    try
    {
        // Note : the exception may fly or not (e.g. remote scenario).
        //        When it flies, it must contain the correct elements.
        xLBT->raiseRuntimeExceptionOneway(sCompare, xLBT->Interface );
    }
    catch (RuntimeException*  e )
    {
        bReturn = ( xLBT->Interface == e->Context );
    }
    return bReturn;
}


static bool raiseException(XBridgeTest* xLBT )
{
    int nCount = 0;
    try
    {
        try
        {
            try
            {
                xLBT->raiseException(
                    5, Constants::STRING_TEST_CONSTANT, xLBT->Interface );
            }
            catch (unoidl::com::sun::star::lang::IllegalArgumentException* aExc)
            {
                if (aExc->ArgumentPosition == 5 &&
                    aExc->Context == xLBT->Interface)
                {
                    ++nCount;
                }
                else
                {
                    check( false, "### unexpected exception content!" );
                }

                /** it is certain, that the RuntimeException testing will fail,
                    if no */
                xLBT->RuntimeException = 0;
            }
        }
        catch (unoidl::com::sun::star::uno::RuntimeException* rExc)
        {
            if (rExc->Context == xLBT->Interface )
            {
                ++nCount;
            }
            else
            {
                check( false, "### unexpected exception content!" );
            }

            /** it is certain, that the RuntimeException testing will fail, if no */
            xLBT->RuntimeException = (int) 0xcafebabe;
        }
    }
    catch (unoidl::com::sun::star::uno::Exception*  rExc)
    {
        if (rExc->Context == xLBT->Interface)
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

    static private void perform_test( XBridgeTest* xLBT )
    {
        bool bRet= true;
       bRet = check( performTest( xLBT ), "standard test" ) && bRet;
       bRet = check( raiseException( xLBT ) , "exception test" )&& bRet;
       bRet = check( raiseOnewayException( xLBT ), "oneway exception test" ) && bRet;
       bRet = check( testObjectMethodsImplementation(xLBT), "object methods test") && bRet;
       bRet = performQueryForUnknownType( xLBT ) && bRet;
        if (! bRet)
        {
            throw new unoidl::com::sun::star::uno::RuntimeException(
                new String("error: test failed!"), 0);
        }
    }
    XComponentContext* m_xContext;

    public:
    explicit BridgeTest( XComponentContext* xContext )
    {
        m_xContext = xContext;
    }


    int run( String* args[] )
    {
        try
        {
            if (args->Length < 1)
            {
                throw new RuntimeException(
                    "missing argument for bridgetest!", this );
            }
            Object* test_obj =
                m_xContext->getServiceManager()->createInstanceWithContext(
                    args[ 0 ], m_xContext );
            if (test_obj == 0)
                test_obj = m_xContext->getValueByName( args[ 0 ] ).Value;

            Console::WriteLine(
                "cli target bridgetest obj: {0}", test_obj->ToString() );
            XBridgeTest* xTest = __try_cast<XBridgeTest*>(test_obj) ;
            perform_test( xTest );
            Console::WriteLine( "\n### cli_uno C++  bridgetest succeeded." );
            return 0;
        }
        catch (unoidl::com::sun::star::uno::RuntimeException* )
        {
            throw;
        }
        catch (System::Exception* exc)
        {
            System::Text::StringBuilder* s = new System::Text::StringBuilder();
            s->Append(S"cli_cpp_bridgetest: unexpected exception occurred in XMain::run. Original exception: ");
            s->Append(exc->GetType()->Name);
            s->Append(S"\n Message: ");
            s->Append(exc->Message);
            throw new unoidl::com::sun::star::uno::RuntimeException(
                s->ToString(), 0);
        }
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
