/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

using System;
using System.Diagnostics;
using System.Threading;
using System.Runtime.CompilerServices;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.test.testtools.bridgetest;

namespace cs_testobj
{

class CheckFailed: System.Exception {
    public CheckFailed(string message): base(message) {}
}

public class BridgeTestObject : WeakBase, XRecursiveCall, XBridgeTest2
{
    private XComponentContext m_xContext;

    public BridgeTestObject( XComponentContext xContext )
    {
        m_xContext = xContext;
    }
    public BridgeTestObject()
    {
    }

    private bool  _bool;
    private char     _char;
    private byte     _byte;
    private short    _short;
    private ushort    _ushort;
    private int      _long;
    private uint      _ulong;
    private long     _hyper;
    private ulong     _uhyper;
    private float    _float;
    private double   _double;
    private String   _string;
    private Object   _xInterface;
    private Any   _any;
    private TestEnum _testEnum = TestEnum.TEST;
    private TestElement[] _testElements = new TestElement[0];
    private TestDataElements _testDataElements = new TestDataElements();
    private int     _nLastCallId = 0;
    private bool _bFirstCall = true;
    private bool _bSequenceOfCallTestPassed = true;

    private bool[] _arBool;
    private char[] _arChar;
    private byte[] _arByte;
    private short[]_arShort;
    private int[] _arLong;
    private long[] _arHyper;
    private UInt16[] _arUShort;
    private UInt32[] _arULong;
    private UInt64[] _arUHyper;
    private string[] _arString;
    private float[] _arFloat;
    private double[] _arDouble;
    private TestEnum[] _arEnum;
    private Object[] _arObject;
    private int[][] _arLong2;
    private int[][][] _arLong3;
    private Any[] _arAny;

//    private int _raiseAttr1;


    public void setValues(
        bool          bBool,
        char             cChar,
        byte             nByte,
        short            nShort,
        ushort            nUShort,
        int              nLong,
        uint              nULong,
        long             nHyper,
        ulong             nUHyper,
        float            fFloat,
        double           fDouble,
        TestEnum         testEnum,
        String           str,
        Object           xInterface,
        Any              any,
        TestElement []      testElements,
        TestDataElements testDataElements )
    {
         Debug.WriteLine( "##### " + GetType().FullName + ".setValues:" + any );

        _bool             = bBool;
        _char             = cChar;
        _byte             = nByte;
        _short            = nShort;
        _ushort           = nUShort;
        _long             = nLong;
        _ulong            = nULong;
        _hyper            = nHyper;
        _uhyper           = nUHyper;
        _float            = fFloat;
        _double           = fDouble;
        _testEnum         = testEnum;
        _string           = str;
        _xInterface       = xInterface;
        _any              = any;
        _testElements     = testElements;
        _testDataElements = testDataElements;
    }

    public TestDataElements setValues2(
        /*INOUT*/ref bool          io_bool,
        /*INOUT*/ref char             io_char,
        /*INOUT*/ref byte             io_byte,
        /*INOUT*/ref short            io_short,
        /*INOUT*/ref ushort            io_ushort,
        /*INOUT*/ref int              io_long,
        /*INOUT*/ref uint              io_ulong,
        /*INOUT*/ref long             io_hyper,
        /*INOUT*/ref ulong             io_uhyper,
        /*INOUT*/ref float            io_float,
        /*INOUT*/ref double           io_double,
        /*INOUT*/ref TestEnum         io_testEnum,
        /*INOUT*/ref String           io_string,
        /*INOUT*/ref Object           io_xInterface,
        /*INOUT*/ref Any           io_any,
        /*INOUT*/ref TestElement[]    io_testElements,
        /*INOUT*/ref TestDataElements io_testDataElements )
    {
        Debug.WriteLine( "##### " + GetType().FullName + ".setValues2:" + io_any );
        
        _bool             = io_bool;
        _char             = io_char;
        _byte             = io_byte;
        _short            = io_short;
        _ushort           = io_ushort;
        _long             = io_long;
        _ulong            = io_ulong;
        _hyper            = io_hyper;
        _uhyper           = io_uhyper;
        _float            = io_float;
        _double           = io_double;
        _testEnum         = io_testEnum;
        _string           = io_string;
        _xInterface       = io_xInterface;
        _any              = io_any;
        _testElements     = (TestElement[]) io_testElements.Clone();
        _testDataElements = io_testDataElements;

        TestElement temp = io_testElements[ 0 ];
        io_testElements[ 0 ] = io_testElements[ 1 ];
        io_testElements[ 1 ] = temp;

        return _testDataElements;
    }

    public TestDataElements getValues(
        /*OUT*/out bool          o_bool,
        /*OUT*/out char             o_char,
        /*OUT*/out byte             o_byte,
        /*OUT*/out short            o_short,
        /*OUT*/out ushort            o_ushort,
        /*OUT*/out int              o_long,
        /*OUT*/out uint              o_ulong,
        /*OUT*/out long             o_hyper,
        /*OUT*/out ulong             o_uhyper,
        /*OUT*/out float            o_float,
        /*OUT*/out double           o_double,
        /*OUT*/out TestEnum         o_testEnum,
        /*OUT*/out String           o_string,
        /*OUT*/out Object           o_xInterface,
        /*OUT*/out Any           o_any,
        /*OUT*/out TestElement[]    o_testElements,
        /*OUT*/out TestDataElements o_testDataElements )
    {
        Debug.WriteLine( "##### " + GetType().FullName + ".getValues" );
        
        o_bool             = _bool;
        o_char             = _char;
        o_byte             = _byte;
        o_short            = _short;
        o_ushort           = _ushort;
        o_long             = _long;
        o_ulong            = _ulong;
        o_hyper            = _hyper;
        o_uhyper           = _uhyper;
        o_float            = _float;
        o_double           = _double;
        o_testEnum         = _testEnum;
        o_string           = _string;
        o_xInterface       = _xInterface;
        o_any              = _any;
        o_testElements     = _testElements;
        o_testDataElements = _testDataElements;

        return _testDataElements;
    }

    public SmallStruct echoSmallStruct(/*[in]*/SmallStruct arg)
    {
        return arg;
    }

    public MediumStruct echoMediumStruct(/*[in]*/MediumStruct arg)
    {
        return arg;
    }

    public BigStruct echoBigStruct(/*[in]*/BigStruct arg)
    {
        return arg;
    }

    public TwoFloats echoTwoFloats(/*[in]*/TwoFloats arg)
    {
        return arg;
    }

    public FourFloats echoFourFloats(/*[in]*/FourFloats arg)
    {
        return arg;
    }

    public int testPPCAlignment( long l1, long l2, int i1, long l3, int i2 )
    {
        return i2;
    }

    // Attributes
    public bool Bool
    {
        get { return _bool; }
        set { _bool = value; }
    }
    public byte Byte
    {
        get { return _byte; }
        set { _byte = value; }
    }
    public char Char
    {
        get { return _char; }
        set { _char = value; }
    }
    public short Short
    {
        get { return _short; }
        set { _short = value; }
    }
    public ushort UShort
    {
        get { return _ushort; }
        set { _ushort = value; }
    }
    public int Long
    {
        get { return _long; }
        set { _long = value; }
    }
    public uint ULong
    {
        get { return _ulong; }
        set { _ulong = value; }
    }
    public long Hyper
    {
        get { return _hyper; }
        set { _hyper = value; }
    }
    public ulong UHyper
    {
        get { return _uhyper; }
        set { _uhyper = value; }
    }
    public float Float
    {
        get { return _float; }
        set { _float = value; }
    }
    public double Double
    {
        get { return _double; }
        set { _double = value; }
    }
    public TestEnum Enum
    {
        get { return _testEnum; }
        set { _testEnum = value; }
    }
    public String String
    {
        get { return _string; }
        set { _string = value; }
    }
    public Object Interface
    {
        get { return _xInterface; }
        set { _xInterface = value; }
    }
    public uno.Any Any
    {
        get {

            Debug.WriteLine( "##### " + GetType().FullName + ".Any" );
            return _any;
        }
        set {
            Debug.WriteLine( "##### " + GetType().FullName + ".Any:" + value );
            _any = value;
        }
    }
    public TestElement [] Sequence
    {
        get { return _testElements; }
        set { _testElements = value; }
    }
    public TestDataElements Struct
    {
        get { return _testDataElements; }
        set { _testDataElements = value; }
    }
    public Any transportAny(Any value)
    {
        return value;
    }
    public void call(int nCallId , int nWaitMUSEC)
    {
        Thread.Sleep(nWaitMUSEC / 10000);

        if(_bFirstCall)
            _bFirstCall = false;

        else
            _bSequenceOfCallTestPassed = _bSequenceOfCallTestPassed && (nCallId > _nLastCallId);

        _nLastCallId = nCallId;
    }
    public void callOneway( int nCallId , int nWaitMUSEC )
    {
        Thread.Sleep(nWaitMUSEC / 10000);

        _bSequenceOfCallTestPassed = _bSequenceOfCallTestPassed && (nCallId > _nLastCallId);
        _nLastCallId = nCallId;
    }
    public bool sequenceOfCallTestPassed()
    {
        return _bSequenceOfCallTestPassed;
    }
    [MethodImpl( MethodImplOptions.Synchronized )]
    public void callRecursivly( XRecursiveCall xCall,   int nToCall )
    {
        lock (this)
        {
            if(nToCall != 0)
            {
                nToCall --;
                xCall.callRecursivly(this , nToCall);
            }
        }
    }
    [MethodImpl( MethodImplOptions.Synchronized )]
    public void startRecursiveCall( XRecursiveCall xCall, int nToCall )
    {
        lock (this)
        {
            if(nToCall != 0)
            {
                nToCall --;
                xCall.callRecursivly( this , nToCall );
            }
        }
    }

    // XBridgeTest
    public TestDataElements raiseException(
        short nArgumentPos, String rMsg, Object xContext )
    {
        throw new IllegalArgumentException(rMsg, xContext, nArgumentPos);
    }
    public void raiseRuntimeExceptionOneway( String rMsg, Object xContext )
    {
        throw new RuntimeException(rMsg, xContext);
    }

    private void dothrow( System.Exception e )
    {
        throw e;
    }
    public int RuntimeException
    {
        get {
            try
            {
                dothrow( new RuntimeException(_string, _xInterface) );
                return 0; // dummy
            }
            catch (System.Exception exc)
            {
                throw exc;
            }
        }
        set { throw new RuntimeException(_string, _xInterface); }
    }

    // XBridgeTest2
    public int[][] setDim2(int[][] val)
    {
        _arLong2 = val;
        return val;
    }

    public int[][][] setDim3(int[][][] val)
    {
        _arLong3 = val;
        return val;
    }

    public Any[] setSequenceAny(Any[] val)
    {
        _arAny = val;
        return val;
    }

    public bool[] setSequenceBool(bool[] val)
    {
        _arBool = val;
        return val;
    }

    public byte[] setSequenceByte(byte[] val)
    {
        _arByte = val;
        return val;
    }

    public char[] setSequenceChar(char[] val)
    {
        _arChar = val;
        return val;
    }

    public short[] setSequenceShort(short[] val)
    {
        _arShort = val;
        return val;
    }

    public int[] setSequenceLong(int[] val)
    {
        _arLong = val;
        return val;
    }

    public long[] setSequenceHyper(long[] val)
    {
        _arHyper = val;
        return val;
    }

    public float[] setSequenceFloat(float[] val)
    {
        _arFloat = val;
        return val;
    }

    public double[] setSequenceDouble(double[] val)
    {
        _arDouble = val;
        return val;
    }

    public TestEnum[] setSequenceEnum(TestEnum[] val)
    {
        _arEnum = val;
        return val;
    }

    public UInt16[] setSequenceUShort(UInt16[] val)
    {
        _arUShort = val;
        return val;
    }

    public UInt32[] setSequenceULong(UInt32[] val)
    {
        _arULong = val;
        return  val;
    }

    public UInt64[] setSequenceUHyper(UInt64[] val)
    {
        _arUHyper = val;
        return val;
    }

    public Object[] setSequenceXInterface(Object[] val)
    {
        _arObject = val;
        return val;
    }

    public string[] setSequenceString(string[] val)
    {
        _arString = val;
        return val;
    }

    public TestElement[] setSequenceStruct(TestElement[] val)
    {
        _testElements = val;
        return val;
    }

    public void setSequencesInOut(ref bool[] aSeqBoolean,
                               ref char[] aSeqChar,
                               ref byte[] aSeqByte,
                               ref short[] aSeqShort,
                               ref UInt16[] aSeqUShort,
                               ref int[] aSeqLong,
                               ref UInt32[] aSeqULong,
                               ref long[] aSeqHyper,
                               ref UInt64[] aSeqUHyper,
                               ref float[] aSeqFloat,
                               ref double[] aSeqDouble,
                               ref TestEnum[] aSeqTestEnum,
                               ref string[] aSeqString,
                               ref object[] aSeqXInterface,
                               ref Any[] aSeqAny,
                               ref int[][] aSeqDim2,
                               ref int[][][] aSeqDim3)
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

    public void setSequencesOut(out bool[] aSeqBoolean,
                               out char[] aSeqChar,
                               out byte[] aSeqByte,
                               out short[] aSeqShort,
                               out UInt16[] aSeqUShort,
                               out int[] aSeqLong,
                               out UInt32[] aSeqULong,
                               out long[] aSeqHyper,
                               out UInt64[] aSeqUHyper,
                               out float[] aSeqFloat,
                               out double[] aSeqDouble,
                               out TestEnum[] aSeqTestEnum,
                               out string[] aSeqString,
                               out object[] aSeqXInterface,
                               out Any[] aSeqAny,
                               out int[][] aSeqDim2,
                               out int[][][] aSeqDim3)
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
        aSeqTestEnum = _arEnum;
        aSeqString = _arString;
        aSeqXInterface = _arObject;
        aSeqAny = _arAny;
        aSeqDim2 = _arLong2;
        aSeqDim3 = _arLong3;

    }

    /* Attention: Debugging this code (probably in mixed mode) may lead to exceptions
     * which do not occur when running normally (Visual Studio 2003)
     */
    public void testConstructorsService(XComponentContext context) 
	{ 
		Constructors.create1(context,
			true,
			0x80, // -128 in C++,
			Int16.MinValue,
			UInt16.MaxValue,
			Int32.MinValue,
			UInt32.MaxValue,
			Int64.MinValue,
			UInt64.MaxValue,
			0.123f,
			0.456,
			'X',
			"test",
			typeof(Any),
			new Any(true),
			new bool[] { true },
			new byte[] { 0x80}, // in C++ the value is compared with SAL_MIN_INT8 which is -128
			new short[] { Int16.MinValue },
			new UInt16[] { UInt16.MaxValue },
			new Int32[] {Int32.MinValue},  
			new UInt32[] { UInt32.MaxValue },
			new long[] { Int64.MinValue }, 
			new UInt64[] { UInt64.MaxValue },
			new float[] { 0.123f },
			new double[] { 0.456 },
			new char[] { 'X' },
			new string[] { "test" },
			new Type[] { typeof(Any) },
			new Any[] { new Any(true) },
			new bool[][] { new bool[] { true } },
			new Any[][] { new Any[] { new Any(true) } },
			new TestEnum[] { TestEnum.TWO },
			new TestStruct[] { new TestStruct(10) },
			new TestPolyStruct[] { new TestPolyStruct(true) },
			new TestPolyStruct[] { new TestPolyStruct(new Any(true)) },
			new object[] { null },
			TestEnum.TWO,
			new TestStruct(10),
			new TestPolyStruct(true),
			new TestPolyStruct(new Any(true)),
			null
			);

	Constructors.create2(context,
             new Any(true),
             new Any((System.Byte) 0x80),
             new Any(Int16.MinValue),
             new Any(UInt16.MaxValue),
             new Any(Int32.MinValue),
             new Any(UInt32.MaxValue),
             new Any(Int64.MinValue),
             new Any(UInt64.MaxValue),
             new Any(0.123f),
             new Any(0.456),
             new Any('X'),
             new Any("test"),
             new Any(typeof(Any)),
             new Any(true),
             new Any(typeof(bool[]), new bool[] { true }),
             new Any(typeof(byte[]), new byte[] { (System.Byte) 0x80}),
             new Any(typeof(short[]), new short[] { Int16.MinValue }),
             new Any(typeof(UInt16[]), new UInt16[] { UInt16.MaxValue }),
             new Any(typeof(int[]), new int[] { Int32.MinValue }),
             new Any(typeof(UInt32[]), new UInt32[] { UInt32.MaxValue }),
             new Any(typeof(long[]), new long[] { Int64.MinValue }),
             new Any(typeof(UInt64[]), new UInt64[] { UInt64.MaxValue }),
             new Any(typeof(float[]), new float[] { 0.123f }),
             new Any(typeof(double[]), new double[] { 0.456 }),
             new Any(typeof(char[]), new char[] { 'X' }),
             new Any(typeof(string[]), new string[] { "test" }),
             new Any(typeof(Type[]), new Type[] { typeof(Any) }),
             new Any(typeof(Any[]), new Any[] { new Any(true) }),
             new Any(typeof(bool[][]), new bool[][] { new bool[] { true } }),
             new Any(
                 typeof(Any[][]), new Any[][] { new Any[] { new Any(true) } }),
             new Any(typeof(TestEnum[]), new TestEnum[] { TestEnum.TWO }),
             new Any(
                 typeof(TestStruct[]), new TestStruct[] { new TestStruct(10) }),
             new Any(
                 PolymorphicType.GetType(
                     typeof(TestPolyStruct[]),
                     "unoidl.test.testtools.bridgetest.TestPolyStruct<System.Boolean>[]"),
                 new TestPolyStruct[] { new TestPolyStruct(true) }) ,
             new Any(
                 PolymorphicType.GetType(
                     typeof(TestPolyStruct[]),
                     "unoidl.test.testtools.bridgetest.TestPolyStruct<uno.Any>[]"),
                 new TestPolyStruct[] { new TestPolyStruct(new Any(true)) }),
             new Any(typeof(object[])/*TODO*/, new object[] { null }),
             new Any(typeof(TestEnum), TestEnum.TWO),
             new Any(typeof(TestStruct), new TestStruct(10)),
             new Any(
                 PolymorphicType.GetType(
                     typeof(TestPolyStruct),
                     "unoidl.test.testtools.bridgetest.TestPolyStruct<System.Boolean>"),
                 new TestPolyStruct(true)),
             new Any(
                 PolymorphicType.GetType(
                     typeof(TestPolyStruct),
                     "unoidl.test.testtools.bridgetest.TestPolyStruct<uno.Any>"),
                 new TestPolyStruct(new Any(true))),
             new Any(typeof(object), null) 
		);


    XMultiBase1 xMulti = Constructors2.create1(
            context,
            new TestPolyStruct(typeof(int)),
            new TestPolyStruct(new Any(true)),
            new TestPolyStruct(true),
            new TestPolyStruct((Byte) 0x80),
            new TestPolyStruct(Int16.MinValue),
            new TestPolyStruct(Int32.MinValue),
            new TestPolyStruct(Int64.MinValue),
            new TestPolyStruct('X'),
            new TestPolyStruct("test"),
            new TestPolyStruct(0.123f),
            new TestPolyStruct(0.456d),
            new TestPolyStruct(new object()),
            new TestPolyStruct(new uno.util.WeakComponentBase()),
            new TestPolyStruct(TestEnum.TWO),
            new TestPolyStruct(new TestPolyStruct2('X', new Any(true))),
            new TestPolyStruct(new TestPolyStruct2(new TestPolyStruct2('X', new Any(true)), "test")),
            new TestPolyStruct2("test", new TestPolyStruct2('X', new TestPolyStruct(new Any(true)))),
            new TestPolyStruct2( new TestPolyStruct2('X', new Any(true)), new TestPolyStruct('X')),
            new TestPolyStruct(new Type[] { typeof(int)}),
            new TestPolyStruct(new Any[] { new Any(true) }),
            new TestPolyStruct(new bool[] {true}),
            new TestPolyStruct(new byte[] {0x80}),
            new TestPolyStruct(new short[] {Int16.MinValue}),
            new TestPolyStruct(new int[] {Int32.MinValue}),
            new TestPolyStruct(new long[] {Int64.MinValue}),
            new TestPolyStruct(new char[] {'X'}),
            new TestPolyStruct(new string[] {"test"}),
            new TestPolyStruct(new float[] {0.123f}),
            new TestPolyStruct(new double[] {0.456d}),
            new TestPolyStruct(new object[] {new object()}),
            new TestPolyStruct(new unoidl.com.sun.star.lang.XComponent[] {new uno.util.WeakComponentBase()}),
            new TestPolyStruct(new TestEnum[] {TestEnum.TWO}),
            new TestPolyStruct(new TestPolyStruct2[] {new TestPolyStruct2('X', new Any[] {new Any(true)})}),
            new TestPolyStruct(new TestPolyStruct2[] {
                new TestPolyStruct2(new TestPolyStruct('X'), new Any[] {new Any(true)})}),
            new TestPolyStruct(new int[][] { new int[] {Int32.MinValue} }),
            new TestPolyStruct[]{ new TestPolyStruct(Int32.MinValue)},
            new TestPolyStruct[]{new TestPolyStruct(new TestPolyStruct2('X', new Any(true)))},
            new TestPolyStruct[]{new TestPolyStruct(new TestPolyStruct2(new TestPolyStruct2('X', new Any(true)), "test"))},
            new TestPolyStruct2[]{new TestPolyStruct2("test", new TestPolyStruct2('X', new TestPolyStruct(new Any(true))))},
            new TestPolyStruct2[]{new TestPolyStruct2(new TestPolyStruct2('X', new Any(true)),new TestPolyStruct('X'))},
            new TestPolyStruct[][]{new TestPolyStruct[]{new TestPolyStruct('X')}},
            new TestPolyStruct[][]{new TestPolyStruct[]{new TestPolyStruct(new TestPolyStruct2('X', new Any(true)))}},
            new TestPolyStruct[][]{new TestPolyStruct[] {new TestPolyStruct(new TestPolyStruct2(new TestPolyStruct2('X',new Any(true)), "test"))}},
            new TestPolyStruct2[][]{new TestPolyStruct2[]{new TestPolyStruct2("test", new TestPolyStruct2('X',new TestPolyStruct(new Any(true))))}},
            new TestPolyStruct2[][]{new TestPolyStruct2[]{new TestPolyStruct2(new TestPolyStruct2('X',new Any(true)),new TestPolyStruct('X'))}}
            );

    //test the returned interface
    xMulti.fn11(1);
    
    
    }

    public XCurrentContextChecker getCurrentContextChecker()
    {
        return null; //TODO: not yet tested in CLI UNO
    }

    public TestPolyStruct transportPolyBoolean(/*[in]*/TestPolyStruct arg)
    {
        return arg;
    }
  
    public  void  transportPolyHyper(/*[in][out]*/ ref TestPolyStruct arg)
    {
    }

    public void  transportPolySequence(TestPolyStruct arg1,
                                       out TestPolyStruct arg2)
    {
        arg2 = arg1;
    }

    public TestPolyStruct getNullPolyBadEnum()
    {
        return new TestPolyStruct(unoidl.test.testtools.bridgetest.TestBadEnum.M);
    }
    
    public TestPolyStruct getNullPolyLong()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyString()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyType()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyAny()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolySequence()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyEnum()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyStruct()
    {
        return new TestPolyStruct();
    }

    public TestPolyStruct getNullPolyInterface()
    {
        return new TestPolyStruct();
    }

    public XMulti getMulti()
    {
        return new testtools.bridgetest.cli_cs.Multi();
    }

    private static void checkEqual(int value, int argument) {
        if (argument != value) {
            throw new CheckFailed(value + " != " + argument);
        }
    }

    private static void checkEqual(double value, double argument) {
        if (argument != value) {
            throw new CheckFailed(value + " != " + argument);
        }
    }

    private static void checkEqual(string value, string argument) {
        if (argument != value) {
            throw new CheckFailed(value + " != " + argument);
        }
    }

    public string testMulti(XMulti multi)
    {
        try {
            checkEqual(0.0, multi.att1);
            multi.att1 = 0.1;
            checkEqual(0.1, multi.att1);
            checkEqual(11 * 1, multi.fn11(1));
            checkEqual("12" + "abc", multi.fn12("abc"));
            checkEqual(21 * 2, multi.fn21(2));
            checkEqual("22" + "de", multi.fn22("de"));
            checkEqual(0.0, multi.att3);
            multi.att3 = 0.3;
            checkEqual(0.3, multi.att3);
            checkEqual(31 * 3, multi.fn31(3));
            checkEqual("32" + "f", multi.fn32("f"));
            checkEqual(33, multi.fn33());
            checkEqual(41 * 4, multi.fn41(4));
            checkEqual(61 * 6, multi.fn61(6));
            checkEqual("62" + "", multi.fn62(""));
            checkEqual(71 * 7, multi.fn71(7));
            checkEqual("72" + "g", multi.fn72("g"));
            checkEqual(73, multi.fn73());
        } catch (CheckFailed f) {
            return f.Message;
        }
        return "";
    }
    
    public int RaiseAttr1
    {
        get { throw new RuntimeException(); }
        set { throw new IllegalArgumentException(); }
    }

    public int RaiseAttr2
    {
        get { throw new IllegalArgumentException(); }
        set { throw new IllegalArgumentException(); }
    }


}

}
