/*************************************************************************
 *
 *  $RCSfile: cli_cs_testobj.cs,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 09:17:14 $
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

using System;
using System.Diagnostics;
using System.Threading;
using System.Runtime.CompilerServices;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.test.bridge;

namespace cs_testobj
{

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
#if DEBUG
//         Debug.WriteLine( "##### " + GetType().FullName + ".setValues:" + any );
        Console.WriteLine( "##### " + GetType().FullName + ".setValues:" + any );
#endif
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
#if DEBUG
//         Debug.WriteLine( "##### " + GetType().FullName + ".setValues2:" + io_any );
        Console.WriteLine( "##### " + GetType().FullName + ".setValues2:" + io_any );
#endif
        
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
#if DEBUG
//         Debug.WriteLine( "##### " + GetType().FullName + ".getValues" );
        Console.WriteLine( "##### " + GetType().FullName + ".getValues" );
#endif
        
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
#if DEBUG
//         Debug.WriteLine( "##### " + GetType().FullName + ".Any" );
            Console.WriteLine( "##### " + GetType().FullName + ".Any" );
#endif
            return _any;
        }
        set {
#if DEBUG
//         Debug.WriteLine( "##### " + GetType().FullName + ".Any:" + value );
            Console.WriteLine( "##### " + GetType().FullName + ".Any:" + value );
#endif
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
    public int RuntimeException
    {
        get { throw new RuntimeException(_string, _xInterface); }
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

    

    

}

}
