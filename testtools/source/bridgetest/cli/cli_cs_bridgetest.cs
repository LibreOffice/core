/*************************************************************************
 *
 *  $RCSfile: cli_cs_bridgetest.cs,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 09:17:06 $
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
 *  Software provided under this License is provided on an "ASIS" basis,
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

using System;
using System.Diagnostics;
using System.Reflection;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.test.bridge;

namespace foo
{
    public interface MyInterface
    {
    }
}

namespace cs_testobj
{
    class ORecursiveCall: WeakBase, XRecursiveCall
    {
        public void  callRecursivly(XRecursiveCall xCall, int nToCall)
		{
			lock (this)
            {
                if (nToCall > 0)
                {
                    nToCall --;
                    xCall.callRecursivly(this, nToCall);
                }    
            }
        }
    };
        
class Constants
{
   public const string STRING_TEST_CONSTANT = "\" paco\' chorizo\\\' \"\'";
}
    
public class BridgeTest : WeakBase, XMain
{
	static bool compareData(Object val1, Object val2)
	{
		if (val1 == null && val2 == null || val1 == val2)
			return true;
		if ((val1 == null && val2 != null) ||
			(val1 != null && val2 == null) || val1.GetType() != val2.GetType())
			return false;
					
		bool ret = false;
		Type t1  = val1.GetType();
			//Sequence
		if (t1.IsArray)
		{
			ret = compareSequence((Array) val1, (Array) val2);
		}
			//String
		else if (t1 == typeof(string))
		{
			ret = (string) val1 == (string) val2;
		}
			// Interface implementation
		else if (t1.GetInterfaces().Length > 0 && ! t1.IsValueType)
		{
			ret = val1 == val2;						
		}		
			// Struct	
		else if ( ! t1.IsValueType)
		{
			ret = compareStruct(val1, val2);
		}
		else if (t1 == typeof(Any))
		{
			Any a1 = (Any) val1;
			Any a2 = (Any) val2;
			ret = a1.Type == a2.Type && compareData(a1.Value, a2.Value);
		}
		else if (t1.IsValueType)
		{
			//Any, enum, int, bool char, float, double etc.
			ret = val1.Equals(val2);
		}
		else
		{
			Debug.Assert(false);
		}		
		return ret;
	}

	// Arrays have only one dimension
	static bool compareSequence(Array ar1, Array ar2)
	{
		Debug.Assert(ar1 != null && ar2 != null);
		Type t1 = ar1.GetType();
		Type t2 = ar2.GetType();
		
		if (!(ar1.Rank == 1 && ar2.Rank == 1 
			&& ar1.Length == ar2.Length && t1.GetElementType() == t2.GetElementType()))
			return false;

		//arrays have same rank and size and element type.
		int len  = ar1.Length;
		Type elemType = t1.GetElementType();
		bool ret = true;
		for (int i = 0; i < len; i++)
		{
			if (compareData(ar1.GetValue(i), ar2.GetValue(i)) == false)
			{
				ret = false;
				break;
			}
		}
		return ret;
	}

	static bool compareStruct(Object val1, Object val2)
	{
		Debug.Assert(val1 != null && val2 != null);
		Type t1 = val1.GetType();
		Type t2 = val2.GetType();
		if (t1 != t2)
			return false;
		FieldInfo[] fields = t1.GetFields();
		int cFields = fields.Length;
		bool ret = true;
		for (int i = 0; i < cFields; i++)
		{
			Object fieldVal1 = fields[i].GetValue(val1);
			Object fieldVal2 = fields[i].GetValue(val2);
			if ( ! compareData(fieldVal1, fieldVal2))
			{
				ret = false;
				break;
			}
		}
		return ret;
	}

    static bool check( bool b , string message )
    {
        if ( ! b)
        Console.WriteLine("{0} failed\n" , message);
        return b;
    }

    static bool equals(TestElement rData1, TestElement  rData2)
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
        check( compareData(rData1.Any, rData2.Any), "### any does not match!" );
        
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
                compareData(rData1.Any, rData2.Any));
    }

static void assign( TestElement rData,
					bool bBool, char cChar, byte nByte,
					short nShort, ushort nUShort,
					int nLong, uint nULong,
					long nHyper, ulong nUHyper,
					float fFloat, double fDouble,
					TestEnum eEnum, string rStr,
					Object xTest,
					Any rAny )
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

static void assign( TestDataElements rData,
					bool bBool, char cChar, byte nByte,
					short nShort, ushort nUShort,
					int nLong, uint nULong,
					long nHyper, ulong nUHyper,
					float fFloat, double fDouble,
					TestEnum eEnum, string rStr,
					Object xTest,
					Any rAny,
					TestElement[] rSequence)
{
	assign( (TestElement) rData,
			bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
			eEnum, rStr, xTest, rAny );
	rData.Sequence = rSequence;
}

// template < class type >
static bool testAny(Type typ, Object  value, XBridgeTest xLBT )
{
	Any any;
	if (typ == null)
		any = new Any(value.GetType(), value);
	else
		any = new Any(typ, value);

	Any any2 = xLBT.transportAny(any);
	bool ret;
	if( ! (ret= compareData(any, any2)))
    {
        Console.WriteLine("any is different after roundtrip: in {0}, out {1}\n",
                          any.Type.FullName, any2.Type.FullName);
    }
	return ret;
}



static bool performAnyTest(XBridgeTest xLBT,  TestDataElements data)
{
	bool bReturn = true;
	bReturn = testAny( null, data.Byte ,xLBT ) && bReturn;
	bReturn = testAny( null, data.Short,xLBT ) && bReturn;
	bReturn = testAny(  null, data.UShort,xLBT ) && bReturn;
	bReturn = testAny(  null, data.Long,xLBT ) && bReturn;
	bReturn = testAny(  null, data.ULong,xLBT ) && bReturn;
	bReturn = testAny(  null, data.Hyper,xLBT ) && bReturn;
	bReturn = testAny(  null,data.UHyper,xLBT ) && bReturn;
	bReturn = testAny( null, data.Float,xLBT ) && bReturn;
	bReturn = testAny( null, data.Double,xLBT ) && bReturn;
	bReturn = testAny( null, data.Enum,xLBT ) && bReturn;
	bReturn = testAny( null, data.String,xLBT ) && bReturn;
	bReturn = testAny(typeof(XWeak), data.Interface,xLBT ) && bReturn;
	bReturn = testAny(null, data, xLBT ) && bReturn;

	{
        Any a1= new Any(true);
		Any a2 = xLBT.transportAny( a1 );
		bReturn = compareData(a2, a1) && bReturn;
	}

	{
        Any a1= new Any('A');
		Any a2 = xLBT.transportAny(a1);
		bReturn = compareData(a2, a1) && bReturn;
	}
	return bReturn; 
}

static bool performSequenceOfCallTest(XBridgeTest xLBT)
{
	int i,nRounds;
	int nGlobalIndex = 0;
	const int nWaitTimeSpanMUSec = 10000;
	for( nRounds = 0 ; nRounds < 10 ; nRounds ++ )
	{
		for( i = 0 ; i < nRounds ; i ++ )
		{
			// fire oneways
			xLBT.callOneway(nGlobalIndex, nWaitTimeSpanMUSec);
			nGlobalIndex++;
		}

		// call synchron
		xLBT.call(nGlobalIndex, nWaitTimeSpanMUSec);
		nGlobalIndex++;
	}
 	return xLBT.sequenceOfCallTestPassed();
}




static bool performRecursiveCallTest(XBridgeTest  xLBT)
{
	xLBT.startRecursiveCall(new ORecursiveCall(), 50);
	// on failure, the test would lock up or crash
	return true;
}

static bool performQueryForUnknownType(XBridgeTest xLBT)
{
    bool bRet = false;
    // test queryInterface for an unknown type 
    try
    {
        foo.MyInterface a = (foo.MyInterface) xLBT;
    }
    catch( System.InvalidCastException)
    {
        bRet = true;
    }

    return bRet;
}

// //==================================================================================================
static bool performTest(XBridgeTest xLBT)
{
	check( xLBT != null, "### no test interface!" );
    bool bRet = true;
	if (xLBT != null)
	{
		// this data is never ever granted access to by calls other than equals(), assign()!
		TestDataElements aData = new TestDataElements(); // test against this data
		
		Object xI= new WeakBase();

        Any aAny = new Any( typeof(Object), xI);
		assign( (TestElement)aData,
				true, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
				0x123456789abcdef0, 0xfedcba9876543210,
				17.0815f, 3.1415926359, TestEnum.LOLA,
				Constants.STRING_TEST_CONSTANT, xI,
				aAny);

		bRet = check( aData.Any.Value == xI, "### unexpected any!" ) && bRet;
		bRet = check( !(aData.Any.Value != xI), "### unexpected any!" ) && bRet;
		
		aData.Sequence = new TestElement[2];
        aData.Sequence[0] = new TestElement(
			aData.Bool, aData.Char, aData.Byte, aData.Short,
			aData.UShort, aData.Long, aData.ULong, 
			aData.Hyper, aData.UHyper, aData.Float, 
			aData.Double, aData.Enum, aData.String, 
			aData.Interface, aData.Any); //(TestElement) aData;
        aData.Sequence[1] = new TestElement(); //is empty
        
		// aData complete
		//
		// this is a manually copy of aData for first setting...
		TestDataElements aSetData = new TestDataElements();
		Any aAnySet= new Any(typeof(Object), xI);
		assign( (TestElement)aSetData,
				aData.Bool, aData.Char, aData.Byte, aData.Short, aData.UShort,
				aData.Long, aData.ULong, aData.Hyper, aData.UHyper, aData.Float, aData.Double,
				aData.Enum, aData.String, xI,
				aAnySet);
		
		aSetData.Sequence = new TestElement[2];
        aSetData.Sequence[0] = new TestElement(
			aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short,
			aSetData.UShort, aSetData.Long, aSetData.ULong, 
			aSetData.Hyper, aSetData.UHyper, aSetData.Float, 
			aSetData.Double, aSetData.Enum, aSetData.String, 
			aSetData.Interface, aSetData.Any); //TestElement) aSetData;
        aSetData.Sequence[1] = new TestElement(); // empty struct

		xLBT.setValues(
				aSetData.Bool, aSetData.Char, aSetData.Byte, aSetData.Short, aSetData.UShort,
				aSetData.Long, aSetData.ULong, aSetData.Hyper, aSetData.UHyper, aSetData.Float, aSetData.Double,
				aSetData.Enum, aSetData.String, aSetData.Interface, aSetData.Any, aSetData.Sequence, aSetData );
		
		{
		TestDataElements aRet = new TestDataElements();
        TestDataElements aRet2 = new TestDataElements();
		xLBT.getValues(
			out aRet.Bool, out aRet.Char, out aRet.Byte, out aRet.Short, out aRet.UShort,
			out aRet.Long, out aRet.ULong, out aRet.Hyper, out aRet.UHyper,
            out aRet.Float, out aRet.Double, out aRet.Enum, out aRet.String,
            out aRet.Interface, out aRet.Any, out aRet.Sequence, out aRet2 );
		
		bRet = check( compareData( aData, aRet ) && compareData( aData, aRet2 ) , "getValues test") && bRet;
        
		// set last retrieved values
		TestDataElements aSV2ret = xLBT.setValues2(
			ref aRet.Bool, ref aRet.Char, ref aRet.Byte, ref aRet.Short, ref aRet.UShort,
			ref aRet.Long, ref aRet.ULong, ref aRet.Hyper, ref aRet.UHyper, ref aRet.Float,
            ref aRet.Double, ref aRet.Enum, ref aRet.String, ref aRet.Interface, ref aRet.Any,
            ref aRet.Sequence, ref aRet2 );
		
        // check inout sequence order
        // => inout sequence parameter was switched by test objects
		TestElement temp = aRet.Sequence[ 0 ];
        aRet.Sequence[ 0 ] = aRet.Sequence[ 1 ];
        aRet.Sequence[ 1 ] = temp;
        
		bRet = check(
            compareData( aData, aSV2ret ) && compareData( aData, aRet2 ),
            "getValues2 test") && bRet;
		}
		{
		TestDataElements aRet = new TestDataElements();
        TestDataElements aRet2 = new TestDataElements();
		TestDataElements aGVret = xLBT.getValues(
			out aRet.Bool, out aRet.Char, out aRet.Byte, out aRet.Short,
            out aRet.UShort, out aRet.Long, out aRet.ULong, out aRet.Hyper,
            out aRet.UHyper, out aRet.Float, out aRet.Double, out aRet.Enum,
            out aRet.String, out aRet.Interface, out aRet.Any, out aRet.Sequence,
            out aRet2 );
		
		bRet = check( compareData( aData, aRet ) && compareData( aData, aRet2 ) && compareData( aData, aGVret ), "getValues test" ) && bRet;
		
		// set last retrieved values
		xLBT.Bool = aRet.Bool;
		xLBT.Char = aRet.Char;
		xLBT.Byte = aRet.Byte;
		xLBT.Short = aRet.Short;
		xLBT.UShort = aRet.UShort;
        xLBT.Long = aRet.Long;
		xLBT.ULong = aRet.ULong;
		xLBT.Hyper = aRet.Hyper;
		xLBT.UHyper = aRet.UHyper;
		xLBT.Float = aRet.Float;
		xLBT.Double = aRet.Double;
		xLBT.Enum = aRet.Enum;
		xLBT.String = aRet.String;
		xLBT.Interface = aRet.Interface;
		xLBT.Any = aRet.Any;
		xLBT.Sequence = aRet.Sequence;
		xLBT.Struct = aRet2;
		}
		{
		TestDataElements aRet = new TestDataElements();
        TestDataElements aRet2 = new TestDataElements();
		aRet.Hyper = xLBT.Hyper;
		aRet.UHyper = xLBT.UHyper;
		aRet.Float = xLBT.Float;
		aRet.Double = xLBT.Double;
		aRet.Byte = xLBT.Byte;
		aRet.Char = xLBT.Char;
		aRet.Bool = xLBT.Bool;
		aRet.Short = xLBT.Short;
		aRet.UShort = xLBT.UShort;
		aRet.Long = xLBT.Long;
		aRet.ULong = xLBT.ULong;
		aRet.Enum = xLBT.Enum;
		aRet.String = xLBT.String;
		aRet.Interface = xLBT.Interface;
		aRet.Any = xLBT.Any;
		aRet.Sequence = xLBT.Sequence;
		aRet2 = xLBT.Struct;
		
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
        xLBT.Interface = null;
        aRet.Interface = xLBT.Interface;
        bRet = (aRet.Interface == null) && bRet;

        }
        

	}
        return bRet;
 }
static bool performSequenceTest(XBridgeTest xBT)
{
    bool bRet = true;
    XBridgeTest2  xBT2 = xBT as XBridgeTest2;
    if ( xBT2 == null)
        return false;

    // perform sequence tests (XBridgeTest2)
    // create the sequence which are compared with the results
    bool[] arBool = {true, false, true};
    char[] arChar = {'A','B','C'};
    byte[] arByte = { 1,  2,  0xff};
    short[] arShort = {Int16.MinValue, 1,  Int16.MaxValue};
    UInt16[] arUShort = {UInt16.MinValue , 1, UInt16.MaxValue};
    int[] arLong = {Int32.MinValue, 1, Int32.MaxValue};
    UInt32[] arULong = {UInt32.MinValue, 1, UInt32.MaxValue};
    long[] arHyper = {Int64.MinValue, 1, Int64.MaxValue};
    UInt64[] arUHyper = {UInt64.MinValue, 1, UInt64.MaxValue};
    float[] arFloat = {1.1f, 2.2f, 3.3f};
    double[] arDouble = {1.11, 2.22, 3.33};
    string[] arString = {"String 1", "String 2", "String 3"};

    Any[] arAny = {new Any(true), new Any(11111), new Any(3.14)};
    Object[] arObject = {new WeakBase(), new WeakBase(), new WeakBase()};
    TestEnum[] arEnum = {TestEnum.ONE, TestEnum.TWO, TestEnum.CHECK};

    TestElement[] arStruct = {new TestElement(), new TestElement(),
                               new TestElement()};
    assign( arStruct[0], true, '@', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
 			0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum.LOLA, Constants.STRING_TEST_CONSTANT, arObject[0],
            new Any( typeof(Object),  arObject[0]) );
    assign( arStruct[1], true, 'A', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
			0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum.TWO, Constants.STRING_TEST_CONSTANT, arObject[1],
            new Any( typeof(Object), arObject[1]) );
    assign( arStruct[2], true, 'B', 17, 0x1234, 0xfedc, 0x12345678, 0xfedcba98,
			0x123456789abcdef0, 0xfedcba9876543210, 17.0815f, 3.1415926359,
            TestEnum.CHECK, Constants.STRING_TEST_CONSTANT, arObject[2],
            new Any( typeof(Object), arObject[2] ) );

    
    int[][][] arLong3 = new int[][][]{
        new int[][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9} },
        new int [][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9}},
        new int[][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9}}};

    {
    int[][] seqSeqRet = xBT2.setDim2(arLong3[0]);
    bRet = check( compareData(seqSeqRet, arLong3[0]), "sequence test") && bRet;
    int[][][] seqSeqRet2 = xBT2.setDim3(arLong3);
    bRet = check( compareData(seqSeqRet2, arLong3), "sequence test") && bRet;
    Any[] seqAnyRet = xBT2.setSequenceAny(arAny);
    bRet = check( compareData(seqAnyRet, arAny), "sequence test") && bRet;
    bool[] seqBoolRet = xBT2.setSequenceBool(arBool);
    bRet = check( compareData(seqBoolRet, arBool), "sequence test") && bRet;
    byte[] seqByteRet = xBT2.setSequenceByte(arByte);
    bRet = check( compareData(seqByteRet, arByte), "sequence test") && bRet;
    char[] seqCharRet = xBT2.setSequenceChar(arChar);
    bRet = check( compareData(seqCharRet, arChar), "sequence test") && bRet;
    short[] seqShortRet = xBT2.setSequenceShort(arShort);
    bRet = check( compareData(seqShortRet, arShort), "sequence test") && bRet;
    int[] seqLongRet = xBT2.setSequenceLong(arLong);
    bRet = check( compareData(seqLongRet, arLong), "sequence test") && bRet;
    long[] seqHyperRet = xBT2.setSequenceHyper(arHyper);
    bRet = check( compareData(seqHyperRet,arHyper), "sequence test") && bRet;
    float[] seqFloatRet = xBT2.setSequenceFloat(arFloat);
    bRet = check( compareData(seqFloatRet, arFloat), "sequence test") && bRet;
    double[] seqDoubleRet = xBT2.setSequenceDouble(arDouble);
    bRet = check( compareData(seqDoubleRet, arDouble), "sequence test") && bRet;
    TestEnum[] seqEnumRet = xBT2.setSequenceEnum(arEnum);
    bRet = check( compareData(seqEnumRet, arEnum), "sequence test") && bRet;
    UInt16[] seqUShortRet = xBT2.setSequenceUShort(arUShort);
    bRet = check( compareData(seqUShortRet, arUShort), "sequence test") && bRet;
    UInt32[] seqULongRet = xBT2.setSequenceULong(arULong);
    bRet = check( compareData(seqULongRet, arULong), "sequence test") && bRet;
    UInt64[] seqUHyperRet = xBT2.setSequenceUHyper(arUHyper);
    bRet = check( compareData(seqUHyperRet, arUHyper), "sequence test") && bRet;
    Object[] seqObjectRet = xBT2.setSequenceXInterface(arObject);
    bRet = check( compareData(seqObjectRet, arObject), "sequence test") && bRet;
    string[] seqStringRet = xBT2.setSequenceString(arString);
    bRet = check( compareData(seqStringRet, arString), "sequence test") && bRet;
    TestElement[] seqStructRet = xBT2.setSequenceStruct(arStruct);
    bRet = check( compareData(seqStructRet, arStruct), "sequence test") && bRet;
    }
    {
    bool[] arBoolTemp = (bool[]) arBool.Clone();
    char[] arCharTemp = (char[]) arChar.Clone();
    byte[] arByteTemp = (byte[]) arByte.Clone();
    short[] arShortTemp = (short[]) arShort.Clone();
    UInt16[] arUShortTemp = (UInt16[]) arUShort.Clone();
    int[] arLongTemp = (int[]) arLong.Clone();
    UInt32[] arULongTemp = (UInt32[]) arULong.Clone();
    long[] arHyperTemp = (long[]) arHyper.Clone();
    UInt64[] arUHyperTemp = (UInt64[]) arUHyper.Clone();
    float[] arFloatTemp = (float[]) arFloat.Clone();
    double[] arDoubleTemp = (double[]) arDouble.Clone();
    TestEnum[] arEnumTemp = (TestEnum[]) arEnum.Clone();
    string[] arStringTemp = (string[]) arString.Clone();
    Object[] arObjectTemp = (Object[]) arObject.Clone();
    Any[] arAnyTemp = (Any[]) arAny.Clone();
    // make sure this are has the same contents as arLong3[0]
    int[][] arLong2Temp = new int[][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9} };
    // make sure this are has the same contents as arLong3
    int[][][] arLong3Temp = new int[][][]{
        new int[][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9} },
        new int [][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9}},
        new int[][]{new int[]{1,2,3},new int[]{4,5,6}, new int[]{7,8,9}}};
    
    xBT2.setSequencesInOut(ref arBoolTemp, ref arCharTemp, ref arByteTemp,
                           ref arShortTemp, ref arUShortTemp, ref arLongTemp,
                           ref arULongTemp,ref arHyperTemp, ref arUHyperTemp,
                           ref arFloatTemp,ref arDoubleTemp, ref arEnumTemp,
                           ref arStringTemp, ref  arObjectTemp,
                           ref arAnyTemp, ref arLong2Temp, ref arLong3Temp);
    bRet = check(
        compareData(arBoolTemp, arBool) &&
        compareData(arCharTemp , arChar) &&
        compareData(arByteTemp , arByte) &&
        compareData(arShortTemp , arShort) &&
        compareData(arUShortTemp , arUShort) &&
        compareData(arLongTemp , arLong) &&
        compareData(arULongTemp , arULong) &&
        compareData(arHyperTemp , arHyper) &&
        compareData(arUHyperTemp , arUHyper) &&
        compareData(arFloatTemp , arFloat) &&
        compareData(arDoubleTemp , arDouble) &&
        compareData(arEnumTemp , arEnum) &&
        compareData(arStringTemp , arString) &&
        compareData(arObjectTemp , arObject) &&
        compareData(arAnyTemp , arAny) &&
        compareData(arLong2Temp , arLong3[0]) &&
        compareData(arLong3Temp , arLong3), "sequence test") && bRet;
    
    bool[] arBoolOut;
    char[] arCharOut;
    byte[] arByteOut;
    short[] arShortOut;
    UInt16[] arUShortOut;
    int[] arLongOut;
    UInt32[] arULongOut;
    long[] arHyperOut;
    UInt64[] arUHyperOut;
    float[] arFloatOut;
    double[] arDoubleOut;
    TestEnum[] arEnumOut;
    string[] arStringOut;
    Object[] arObjectOut;
    Any[] arAnyOut;
    int[][] arLong2Out;
    int[][][] arLong3Out;

    xBT2.setSequencesOut(out arBoolOut, out arCharOut, out arByteOut,
                         out arShortOut, out arUShortOut, out arLongOut,
                         out arULongOut, out arHyperOut, out arUHyperOut,
                         out arFloatOut, out arDoubleOut, out arEnumOut,
                         out arStringOut, out arObjectOut, out arAnyOut,
                         out arLong2Out, out arLong3Out);
    bRet = check(
        compareData(arBoolOut, arBool) &&
        compareData(arCharOut, arChar) &&
        compareData(arByteOut, arByte) &&
        compareData(arShortOut, arShort) &&
        compareData(arUShortOut, arUShort) &&
        compareData(arLongOut, arLong) &&
        compareData(arULongOut, arULong) &&
        compareData(arHyperOut, arHyper) &&
        compareData(arUHyperOut, arUHyper) &&
        compareData(arFloatOut, arFloat) &&
        compareData(arDoubleOut, arDouble) &&
        compareData(arEnumOut, arEnum) &&
        compareData(arStringOut, arString) &&
        compareData(arObjectOut, arObject) &&
        compareData(arAnyOut, arAny) &&
        compareData(arLong2Out, arLong3[0]) &&
        compareData(arLong3Out, arLong3), "sequence test") && bRet;
    }
    {
    //test with empty sequences
    int[][] _arLong2 = new int[0][];
    int[][] seqSeqRet = xBT2.setDim2(_arLong2);
    bRet = check( compareData(seqSeqRet, _arLong2), "sequence test") && bRet;
    int[][][] _arLong3 = new int[0][][];
    int[][][] seqSeqRet2 = xBT2.setDim3(_arLong3);
    bRet = check( compareData(seqSeqRet2, _arLong3), "sequence test") && bRet;
    Any[] _arAny = new Any[0];
    Any[] seqAnyRet = xBT2.setSequenceAny(_arAny);
    bRet = check( compareData(seqAnyRet, _arAny), "sequence test") && bRet;
    bool[] _arBool = new bool[0];
    bool[] seqBoolRet = xBT2.setSequenceBool(_arBool);
    bRet = check( compareData(seqBoolRet, _arBool), "sequence test") && bRet;
    byte[] _arByte = new byte[0];
    byte[] seqByteRet = xBT2.setSequenceByte(_arByte);
    bRet = check( compareData(seqByteRet, _arByte), "sequence test") && bRet;
    char[] _arChar = new char[0];
    char[] seqCharRet = xBT2.setSequenceChar(_arChar);
    bRet = check( compareData(seqCharRet, _arChar), "sequence test") && bRet;
    short[] _arShort = new short[0];
    short[] seqShortRet = xBT2.setSequenceShort(_arShort);
    bRet = check( compareData(seqShortRet, _arShort), "sequence test") && bRet;
    int[] _arLong = new int[0];
    int[] seqLongRet = xBT2.setSequenceLong(_arLong);
    bRet = check( compareData(seqLongRet, _arLong), "sequence test") && bRet;
    long[] _arHyper = new long[0];
    long[] seqHyperRet = xBT2.setSequenceHyper(_arHyper);
    bRet = check( compareData(seqHyperRet, _arHyper), "sequence test") && bRet;
    float[] _arFloat = new float[0];
    float[] seqFloatRet = xBT2.setSequenceFloat(_arFloat);
    bRet = check( compareData(seqFloatRet, _arFloat), "sequence test") && bRet;
    double[] _arDouble = new double[0];
    double[] seqDoubleRet = xBT2.setSequenceDouble(_arDouble);
    bRet = check( compareData(seqDoubleRet, _arDouble), "sequence test") && bRet;
    TestEnum[] _arEnum = new TestEnum[0];
    TestEnum[] seqEnumRet = xBT2.setSequenceEnum(_arEnum);
    bRet = check( compareData(seqEnumRet, _arEnum), "sequence test") && bRet;
    UInt16[] _arUShort = new UInt16[0];
    UInt16[] seqUShortRet = xBT2.setSequenceUShort(_arUShort);
    bRet = check( compareData(seqUShortRet, _arUShort), "sequence test") && bRet;
    UInt32[] _arULong = new UInt32[0];
    UInt32[] seqULongRet = xBT2.setSequenceULong(_arULong);
    bRet = check( compareData(seqULongRet, _arULong), "sequence test") && bRet;
    UInt64[] _arUHyper = new UInt64[0];
    UInt64[] seqUHyperRet = xBT2.setSequenceUHyper(_arUHyper);
    bRet = check( compareData(seqUHyperRet, _arUHyper), "sequence test") && bRet;
    Object[] _arObject = new Object[0];
    Object[] seqObjectRet = xBT2.setSequenceXInterface(_arObject);
    bRet = check( compareData(seqObjectRet, _arObject), "sequence test") && bRet;
    string[] _arString = new string[0];
    string[] seqStringRet = xBT2.setSequenceString(_arString);
    bRet = check( compareData(seqStringRet, _arString), "sequence test") && bRet;
    TestElement[] _arStruct = new TestElement[0];
    TestElement[] seqStructRet = xBT2.setSequenceStruct(_arStruct);
    bRet = check( compareData(seqStructRet, _arStruct), "sequence test") && bRet;
    
    }


    return bRet;
}
/** Test the System::Object method on the proxy object
 */
static bool testObjectMethodsImplemention(XBridgeTest xLBT)
{
    bool ret = false;
    Object obj = new Object();
	Object xInt = (Object) xLBT;
	XBridgeTestBase xBase = xLBT as XBridgeTestBase;
	if (xBase == null)
		return false;
	// Object.Equals
	ret = xLBT.Equals(obj) == false;
	ret = xLBT.Equals(xLBT) && ret;
	ret = Object.Equals(obj, obj) && ret;
	ret = Object.Equals(xLBT, xBase) && ret;
	//Object.GetHashCode
	// Don't know how to verify this. Currently it is not possible to get the object id from a proxy
	int nHash = xLBT.GetHashCode();
	ret = nHash == xBase.GetHashCode() && ret;

	//Object.ToString
    // Don't know how to verify this automatically.
	string s = xLBT.ToString();
    ret = (s.Length > 0) && ret;
    return ret;
}


static bool raiseOnewayException(XBridgeTest xLBT)
{
    bool bReturn = true;
	string sCompare = Constants.STRING_TEST_CONSTANT;
	try
	{
		// Note : the exception may fly or not (e.g. remote scenario).
		//        When it flies, it must contain the correct elements.
		xLBT.raiseRuntimeExceptionOneway(sCompare, xLBT.Interface );
	}
	catch (RuntimeException  e )
	{
		bReturn = ( xLBT.Interface == e.Context );
	}
    return bReturn;
}

// //==================================================================================================
static bool raiseException(XBridgeTest xLBT )
{
	int nCount = 0;
	try
	{
		try
		{
			try
			{
				TestDataElements aRet = new TestDataElements();
                TestDataElements aRet2 = new TestDataElements();
				xLBT.raiseException(
					5, Constants.STRING_TEST_CONSTANT, xLBT.Interface );
			}
			catch (unoidl.com.sun.star.lang.IllegalArgumentException aExc)
			{
				if (aExc.ArgumentPosition == 5 &&
                    aExc.Context == xLBT.Interface)
				{
					++nCount;
				}
				else
				{
					check( false, "### unexpected exception content!" );
				}
				
				/** it is certain, that the RuntimeException testing will fail,
                    if no */
				xLBT.RuntimeException = 0;
			}
		}
		catch (unoidl.com.sun.star.uno.RuntimeException rExc)
		{
			if (rExc.Context == xLBT.Interface )
			{
				++nCount;
			}
			else
			{
				check( false, "### unexpected exception content!" );
			}
			
			/** it is certain, that the RuntimeException testing will fail, if no */
            unchecked
             {
                 xLBT.RuntimeException = (int) 0xcafebabe;
             }
		}
	}
	catch (unoidl.com.sun.star.uno.Exception  rExc)
	{
		if (rExc.Context == xLBT.Interface)
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

    static private void perform_test( XBridgeTest xLBT )
    {
        bool bRet= true;;
       bRet = check( performTest( xLBT ), "standard test" ) && bRet;
       bRet = check( raiseException( xLBT ) , "exception test" )&& bRet;
       bRet = check( raiseOnewayException( xLBT ), "oneway exception test" ) && bRet;
       bRet = check( testObjectMethodsImplemention(xLBT), "object methods test") && bRet;
       bRet = performQueryForUnknownType( xLBT ) && bRet;
        if (! bRet)
        {
            throw new unoidl.com.sun.star.uno.RuntimeException( "error: test failed!", null);
        }
    }

    public BridgeTest( XComponentContext xContext )
    {
        m_xContext = xContext;
    }
    
    private XComponentContext m_xContext;
    
    public int run( String [] args )
    {
        try
        {
            if (args.Length < 1)
            {
                throw new RuntimeException(
                    "missing argument for bridgetest!", this );
            }
            Object test_obj =
                m_xContext.getServiceManager().createInstanceWithContext(
                    args[ 0 ], m_xContext );
            
            Console.WriteLine(
                "cli target bridgetest obj: {0}", test_obj.ToString() );
            XBridgeTest xTest = (XBridgeTest) test_obj ;
            perform_test( xTest );
            Console.WriteLine( "\n### cli_uno C# bridgetest succeeded." );
            return 0;
        }
        catch (System.Exception exc)
        {
			Console.WriteLine( "\n### unexpected exception occured: {0}", exc );
            return 1;
        }
    }
}

}
