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

package com.sun.star.comp.bridge;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;

import com.sun.star.test.performance.ComplexTypes;
import com.sun.star.test.performance.XPerformanceTest;

import test.testtools.bridgetest.BadConstructorArguments;
import test.testtools.bridgetest.Constructors;
import test.testtools.bridgetest.Constructors2;
import test.testtools.bridgetest.TestDataElements;
import test.testtools.bridgetest.TestElement;
import test.testtools.bridgetest.TestEnum;
import test.testtools.bridgetest.TestPolyStruct;
import test.testtools.bridgetest.TestPolyStruct2;
import test.testtools.bridgetest.TestStruct;
import test.testtools.bridgetest.SmallStruct;
import test.testtools.bridgetest.MediumStruct;
import test.testtools.bridgetest.BigStruct;
import test.testtools.bridgetest.TwoFloats;
import test.testtools.bridgetest.FourFloats;
import test.testtools.bridgetest.MixedFloatAndInteger;
import test.testtools.bridgetest.ThreeByteStruct;
import test.testtools.bridgetest.XBridgeTest;
import test.testtools.bridgetest.XBridgeTest2;
import test.testtools.bridgetest.XCurrentContextChecker;
import test.testtools.bridgetest.XMulti;
import test.testtools.bridgetest.XRecursiveCall;


import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

@SuppressWarnings("unchecked")
public class TestComponent {
    public static final boolean DEBUG = false;

    public static class _PerformancTestObject implements XPerformanceTest, XServiceInfo, XTypeProvider {
        private static final String __serviceName = "com.sun.star.comp.benchmark.JavaTestObject";

        private int      _long;
        private long     _hyper;
        private float    _float;
        private double   _double;
        private String   _string = "";
        private Object   _xInterface;
        private Object   _any;
        private Object   _interface_sequence[] = new Object[0];
        private ComplexTypes _complexTypes = new ComplexTypes();

        // Attributes
        public int getLong_attr() throws com.sun.star.uno.RuntimeException {
            return _long;
        }

        public void setLong_attr( int _long_attr ) throws com.sun.star.uno.RuntimeException {
            _long = _long_attr;
        }

        public long getHyper_attr() throws com.sun.star.uno.RuntimeException {
            return _hyper;
        }

        public void setHyper_attr( long _hyper_attr ) throws com.sun.star.uno.RuntimeException {
            _hyper = _hyper_attr;
        }

        public float getFloat_attr() throws com.sun.star.uno.RuntimeException {
            return _float;
        }

        public void setFloat_attr( float _float_attr ) throws com.sun.star.uno.RuntimeException {
            _float = _float_attr;
        }

        public double getDouble_attr() throws com.sun.star.uno.RuntimeException {
            return _double;
        }

        public void setDouble_attr( double _double_attr ) throws com.sun.star.uno.RuntimeException {
            _double = _double_attr;
        }

        public String getString_attr() throws com.sun.star.uno.RuntimeException {
            return _string;
        }

        public void setString_attr( String _string_attr ) throws com.sun.star.uno.RuntimeException {
            _string = _string_attr;
        }

        public Object getInterface_attr() throws com.sun.star.uno.RuntimeException {
            return _xInterface;
        }

        public void setInterface_attr( java.lang.Object _interface_attr ) throws com.sun.star.uno.RuntimeException {
            _xInterface = _interface_attr;
        }

        public Object getAny_attr() throws com.sun.star.uno.RuntimeException {
            return _any;
        }

        public void setAny_attr(Object _any_attr ) throws com.sun.star.uno.RuntimeException {
            _any = _any_attr;
        }

        public Object[] getSequence_attr() throws com.sun.star.uno.RuntimeException {
            return _interface_sequence;
        }

        public void setSequence_attr(Object[] _sequence_attr ) throws com.sun.star.uno.RuntimeException {
            _interface_sequence = _sequence_attr;
        }

        public ComplexTypes getStruct_attr() throws com.sun.star.uno.RuntimeException {
            return _complexTypes;
        }

        public void setStruct_attr( ComplexTypes _struct_attr ) throws com.sun.star.uno.RuntimeException {
            _complexTypes = _struct_attr;
        }


        // Methods
        public void async() throws com.sun.star.uno.RuntimeException {
        }

        public void sync(  ) throws com.sun.star.uno.RuntimeException {
        }

        public ComplexTypes complex_in( /*IN*/ComplexTypes aVal ) throws com.sun.star.uno.RuntimeException {
            return aVal;
        }

        public ComplexTypes complex_inout( /*INOUT*/ComplexTypes[] aVal ) throws com.sun.star.uno.RuntimeException {
            return aVal[0];
        }

        public void complex_oneway( /*IN*/ComplexTypes aVal ) throws com.sun.star.uno.RuntimeException {
        }

        public void complex_noreturn( /*IN*/ComplexTypes aVal ) throws com.sun.star.uno.RuntimeException {
        }

        public XPerformanceTest createObject(  ) throws com.sun.star.uno.RuntimeException {
            return new _PerformancTestObject();
        }

        public int getLong() throws com.sun.star.uno.RuntimeException {
            return _long;
        }

        public void setLong(/*IN*/int n) throws com.sun.star.uno.RuntimeException {
            _long = n;
        }

        public long getHyper() throws com.sun.star.uno.RuntimeException {
            return _hyper;
        }

        public void setHyper(/*IN*/long n) throws com.sun.star.uno.RuntimeException {
            _hyper = n;
        }

        public float getFloat() throws com.sun.star.uno.RuntimeException {
            return _float;
        }

        public void setFloat( /*IN*/float f ) throws com.sun.star.uno.RuntimeException {
            _float = f;
        }

        public double getDouble(  ) throws com.sun.star.uno.RuntimeException {
            return _double;
        }

        public void setDouble( /*IN*/double f ) throws com.sun.star.uno.RuntimeException {
            _double = f;
        }

        public String getString(  ) throws com.sun.star.uno.RuntimeException {
            return _string;
        }

        public void setString( /*IN*/String s ) throws com.sun.star.uno.RuntimeException {
            _string = s;
        }

        public Object getInterface(  ) throws com.sun.star.uno.RuntimeException {
            return _xInterface;
        }

        public void setInterface( /*IN*/Object x ) throws com.sun.star.uno.RuntimeException {
            _xInterface = x;
        }

        public Object getAny(  ) throws com.sun.star.uno.RuntimeException {
            return _any;
        }

        public void setAny( /*IN*/java.lang.Object a ) throws com.sun.star.uno.RuntimeException {
            _any = a;
        }

        public Object[] getSequence(  ) throws com.sun.star.uno.RuntimeException {
            return _interface_sequence;
        }

        public void setSequence( /*IN*/Object[] seq ) throws com.sun.star.uno.RuntimeException {
            if(DEBUG) System.err.println("#### " + getClass().getName() + ".setSequence:" + seq);

            _interface_sequence = seq;
        }

        public ComplexTypes getStruct(  ) throws com.sun.star.uno.RuntimeException {
            return _complexTypes;
        }

        public void setStruct( /*IN*/ComplexTypes c ) throws com.sun.star.uno.RuntimeException {
            _complexTypes = c;
        }

        public void raiseRuntimeException(  ) throws com.sun.star.uno.RuntimeException {
            throw new com.sun.star.uno.RuntimeException();
        }

        // XServiceInfo
        public String getImplementationName() throws com.sun.star.uno.RuntimeException {
            return __serviceName;
        }

        public boolean supportsService(String rServiceName) throws com.sun.star.uno.RuntimeException {
            String rSNL[] = getSupportedServiceNames();

            for(int nPos = rSNL.length; (nPos--) != 0;) {
                if (rSNL[nPos].equals(rServiceName))
                    return true;
            }

            return false;
        }

        public String [] getSupportedServiceNames() throws com.sun.star.uno.RuntimeException {
            return new String[]{__serviceName};
        }

        // XTypeProvider
        public Type[] getTypes() throws com.sun.star.uno.RuntimeException {
            try {
                return new Type[]{new Type(XPerformanceTest.class), new Type(XServiceInfo.class), new Type(XTypeProvider.class)};
            }
            catch(Exception exception) {
                throw new com.sun.star.uno.RuntimeException(exception);
            }
        }

        public byte[] getImplementationId() throws com.sun.star.uno.RuntimeException {
            return new byte[0];
        }
    }

    public static class _TestObject implements XBridgeTest2, XRecursiveCall, XServiceInfo, XTypeProvider {
        private static final String __serviceName = "com.sun.star.test.bridge.JavaTestObject";

        private boolean  _bool;
        private char     _char;
        private byte     _byte;
        private short    _short;
        private short    _ushort;
        private int      _long;
        private int      _ulong;
        private long     _hyper;
        private long     _uhyper;
        private float    _float;
        private double   _double;
        private String   _string;
        private Object   _xInterface;
        private Object   _any;
        private TestEnum _testEnum = TestEnum.TEST;
        private TestElement _testElements[] = new TestElement[0];
        private TestDataElements _testDataElements = new TestDataElements();

        private int     _nLastCallId;
        private boolean _bFirstCall;
        private boolean _bSequenceOfCallTestPassed;

        private boolean[] arBool;
        private char[] arChar;
        private byte[] arByte;
        private short[] arShort;
        private short[] arUShort;
        private int[] arLong;
        private int[] arULong;
        private long[] arHyper;
        private long[] arUHyper;
        private float[] arFloat;
        private double[] arDouble;
        private String[] arString;
        private Object[] arObject;
        private Object[] arAny;
        private TestEnum[] arEnum;
        private int[][] arLong2;
        private int[][][] arLong3;
        public _TestObject(XMultiServiceFactory xMultiServiceFactory) {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init> " + xMultiServiceFactory);

            _nLastCallId = 0;
            _bFirstCall = true;
            _bSequenceOfCallTestPassed = true;
        }


        public void setValues(boolean          bBool,
                              char             cChar,
                              byte             nByte,
                              short            nShort,
                              short            nUShort,
                              int              nLong,
                              int              nULong,
                              long             nHyper,
                              long             nUHyper,
                              float            fFloat,
                              double           fDouble,
                              TestEnum         testEnum,
                              String           string,
                              Object           xInterface,
                              Object           any,
                              TestElement      testElements[],
                              TestDataElements testDataElements) throws com.sun.star.uno.RuntimeException
        {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".setValues:" + any);

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
            _string           = string;
            _xInterface       = xInterface;
            _any              = any;
            _testElements     = testElements;
            _testDataElements = testDataElements;
        }



        public TestDataElements setValues2(/*INOUT*/boolean[]          io_bool,
                                           /*INOUT*/char[]             io_char,
                                           /*INOUT*/byte[]             io_byte,
                                           /*INOUT*/short[]            io_short,
                                           /*INOUT*/short[]            io_ushort,
                                           /*INOUT*/int[]              io_long,
                                           /*INOUT*/int[]              io_ulong,
                                           /*INOUT*/long[]             io_hyper,
                                           /*INOUT*/long[]             io_uhyper,
                                           /*INOUT*/float[]            io_float,
                                           /*INOUT*/double[]           io_double,
                                           /*INOUT*/TestEnum[]         io_testEnum,
                                           /*INOUT*/String[]           io_string,
                                           /*INOUT*/Object[]           io_xInterface,
                                           /*INOUT*/Object[]           io_any,
                                           /*INOUT*/TestElement[][]    io_testElements,
                                           /*INOUT*/TestDataElements[] io_testDataElements) throws com.sun.star.uno.RuntimeException
        {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".setValues2:" + io_any[0]);

            _bool             = io_bool[0];
            _char             = io_char[0];
            _byte             = io_byte[0];
            _short            = io_short[0];
            _ushort           = io_ushort[0];
            _long             = io_long[0];
            _ulong            = io_ulong[0];
            _hyper            = io_hyper[0];
            _uhyper           = io_uhyper[0];
            _float            = io_float[0];
            _double           = io_double[0];
            _testEnum         = io_testEnum[0];
            _string           = io_string[0];
            _xInterface       = io_xInterface[0];
            _any              = io_any[0];
            _testElements     = io_testElements[0];
            _testDataElements = io_testDataElements[0];

            io_testElements[ 0 ] =
                new TestElement [] { io_testElements[ 0 ][ 1 ], io_testElements[ 0 ][ 0 ] };

            return _testDataElements;
        }

        public TestDataElements getValues(/*OUT*/boolean[]          o_bool,
                                          /*OUT*/char[]             o_char,
                                          /*OUT*/byte[]             o_byte,
                                          /*OUT*/short[]            o_short,
                                          /*OUT*/short[]            o_ushort,
                                          /*OUT*/int[]              o_long,
                                          /*OUT*/int[]              o_ulong,
                                          /*OUT*/long[]             o_hyper,
                                          /*OUT*/long[]             o_uhyper,
                                          /*OUT*/float[]            o_float,
                                          /*OUT*/double[]           o_double,
                                          /*OUT*/TestEnum[]         o_testEnum,
                                          /*OUT*/String[]           o_string,
                                          /*OUT*/Object[]           o_xInterface,
                                          /*OUT*/Object[]           o_any,
                                          /*OUT*/TestElement[][]    o_testElements,
                                          /*OUT*/TestDataElements[] o_testDataElements) throws com.sun.star.uno.RuntimeException
        {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".getValues:" + _any);

            o_bool[0]             = _bool;
            o_char[0]             = _char;
            o_byte[0]             = _byte;
            o_short[0]            = _short;
            o_ushort[0]           = _ushort;
            o_long[0]             = _long;
            o_ulong[0]            = _ulong;
            o_hyper[0]            = _hyper;
            o_uhyper[0]           = _uhyper;
            o_float[0]            = _float;
            o_double[0]           = _double;
            o_testEnum[0]         = _testEnum;
            o_string[0]           = _string;
              o_xInterface[0]       = _xInterface;
            o_any[0]              = _any;
            o_testElements[0]     = _testElements;
            o_testDataElements[0] = _testDataElements;

            return _testDataElements;
        }

        public SmallStruct echoSmallStruct( SmallStruct i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public MediumStruct echoMediumStruct( MediumStruct i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public BigStruct echoBigStruct( BigStruct i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public TwoFloats echoTwoFloats( TwoFloats i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public FourFloats echoFourFloats( FourFloats i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public MixedFloatAndInteger echoMixedFloatAndInteger( MixedFloatAndInteger i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public ThreeByteStruct echoThreeByteStruct( ThreeByteStruct i_Struct) throws com.sun.star.uno.RuntimeException {
            return i_Struct;
        }

        public int testPPCAlignment( long l1, long l2, int i1, long l3, int i2 ) throws com.sun.star.uno.RuntimeException {
            return i2;
        }

        public int testPPC64Alignment( double d1, double d2, double d3, int i1 ) throws com.sun.star.uno.RuntimeException {
            return i1;
        }

        public double testTenDoubles( double d1, double d2, double d3, double d4, double d5, double d6, double d7, double d8, double d9, double d10 ) {
            return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
        }

        // Attributes
        public boolean getBool() throws com.sun.star.uno.RuntimeException {
            return _bool;
        }

        public void setBool(boolean bool) throws com.sun.star.uno.RuntimeException {
            _bool = bool;
        }

        public byte getByte() throws com.sun.star.uno.RuntimeException {
            return _byte;
        }

        public void setByte(byte zbyte) throws com.sun.star.uno.RuntimeException {
            _byte = zbyte;
        }

        public char getChar() throws com.sun.star.uno.RuntimeException {
            return _char;
        }

        public void setChar(char zchar) throws com.sun.star.uno.RuntimeException {
            _char = zchar;
        }

        public short getShort() throws com.sun.star.uno.RuntimeException {
            return _short;
        }

        public void setShort(short zshort) throws com.sun.star.uno.RuntimeException {
            _short = zshort;
        }

        public short getUShort() throws com.sun.star.uno.RuntimeException {
            return _ushort;
        }

        public void setUShort(short ushort) throws com.sun.star.uno.RuntimeException {
            _ushort = ushort;
        }

        public int getLong() throws com.sun.star.uno.RuntimeException {
            return _long;
        }

        public void setLong(int zint) throws com.sun.star.uno.RuntimeException {
            _long = zint;
        }

        public int getULong() throws com.sun.star.uno.RuntimeException {
            return _ulong;
        }

        public void setULong(int uint) throws com.sun.star.uno.RuntimeException {
            _ulong = uint;
        }

        public long getHyper() throws com.sun.star.uno.RuntimeException {
            return _hyper;
        }

        public void setHyper(long hyper) throws com.sun.star.uno.RuntimeException {
            _hyper = hyper;
        }

        public long getUHyper() throws com.sun.star.uno.RuntimeException {
            return _uhyper;
        }

        public void setUHyper(long uhyper) throws com.sun.star.uno.RuntimeException {
            _uhyper = uhyper;
        }

        public float getFloat() throws com.sun.star.uno.RuntimeException {
            return _float;
        }

        public void setFloat(float zfloat) throws com.sun.star.uno.RuntimeException {
            _float = zfloat;
        }

        public double getDouble() throws com.sun.star.uno.RuntimeException {
            return _double;
        }

        public void setDouble(double zdouble) throws com.sun.star.uno.RuntimeException {
            _double = zdouble;
        }

        public TestEnum getEnum() throws com.sun.star.uno.RuntimeException {
            return _testEnum;
        }

        public void setEnum(TestEnum testEnum) throws com.sun.star.uno.RuntimeException {
            _testEnum = testEnum;
        }

        public String getString() throws com.sun.star.uno.RuntimeException {
            return _string;
        }

        public void setString(String string) throws com.sun.star.uno.RuntimeException {
            _string = string;
        }

        public Object getInterface() throws com.sun.star.uno.RuntimeException {
            return _xInterface;
        }

        public void setInterface(Object zinterface) throws com.sun.star.uno.RuntimeException {
            _xInterface = zinterface;
        }

        public Object getAny() throws com.sun.star.uno.RuntimeException {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".setAny:" + _any);

            return _any;
        }

        public void setAny(Object any) throws com.sun.star.uno.RuntimeException {
            if(DEBUG) System.err.println("##### " + getClass().getName() + ".setAny:" + any);

            _any = any;
        }

        public TestElement[] getSequence() throws com.sun.star.uno.RuntimeException {
            return _testElements;
        }

        public void setSequence(TestElement testElements[]) throws com.sun.star.uno.RuntimeException {
            _testElements = testElements;
        }

        public TestDataElements getStruct() throws com.sun.star.uno.RuntimeException {
            return _testDataElements;
        }

        public void setStruct(TestDataElements testDataElements) throws com.sun.star.uno.RuntimeException {
            _testDataElements = testDataElements;
        }

        public int getRaiseAttr1() {
            throw new com.sun.star.uno.RuntimeException();
        }

        public void setRaiseAttr1(int n) throws IllegalArgumentException {
            throw new IllegalArgumentException();
        }

        public int getRaiseAttr2() throws IllegalArgumentException {
            throw new IllegalArgumentException();
        }

        public TestPolyStruct transportPolyBoolean(TestPolyStruct arg) {
            Boolean dummy = (Boolean) arg.member;
            return arg;
        }

        public void transportPolyHyper(TestPolyStruct[] arg) {
            Long dummy = (Long) arg[0].member;
        }

        public void transportPolySequence(
            TestPolyStruct arg1, TestPolyStruct[] arg2)
        {
            Object[] dummy = (Object[]) arg1.member;
            arg2[0] = arg1;
        }

        public TestPolyStruct getNullPolyLong() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyString() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyType() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyAny() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolySequence() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyEnum() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyBadEnum() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyStruct() {
            return new TestPolyStruct();
        }

        public TestPolyStruct getNullPolyInterface() {
            return new TestPolyStruct();
        }

        public Object transportAny(Object value) throws com.sun.star.uno.RuntimeException {
            return value;
        }

        public void call(int nCallId , int nWaitMUSEC) throws com.sun.star.uno.RuntimeException {
            try {
                Thread.sleep(nWaitMUSEC / 10000);
            }
            catch(InterruptedException interruptedException) {
                throw new com.sun.star.uno.RuntimeException(interruptedException);
            }

            if(_bFirstCall)
                _bFirstCall = false;

            else
                _bSequenceOfCallTestPassed = _bSequenceOfCallTestPassed && (nCallId > _nLastCallId);

            _nLastCallId = nCallId;
        }

        public void callOneway( int nCallId , int nWaitMUSEC )  throws com.sun.star.uno.RuntimeException {
            try {
                Thread.sleep(nWaitMUSEC / 10000);
            }
            catch(InterruptedException interruptedException) {
                throw new com.sun.star.uno.RuntimeException(interruptedException);
            }

            _bSequenceOfCallTestPassed = _bSequenceOfCallTestPassed && (nCallId > _nLastCallId);
            _nLastCallId = nCallId;
        }

        public boolean sequenceOfCallTestPassed()  throws com.sun.star.uno.RuntimeException {
            return _bSequenceOfCallTestPassed;
        }

        public synchronized void callRecursivly(XRecursiveCall xCall,   int nToCall) throws com.sun.star.uno.RuntimeException {
            if(nToCall != 0)
            {
                nToCall --;
                xCall.callRecursivly(this , nToCall);
            }
        }

        public synchronized void  startRecursiveCall(XRecursiveCall xCall, int nToCall) throws com.sun.star.uno.RuntimeException {
            if(nToCall != 0)
            {
                nToCall --;
                xCall.callRecursivly( this , nToCall );
            }
        }

        public XMulti getMulti() {
            return new XMulti() {
                    public double getatt1() {
                        return attribute1;
                    }

                    public void setatt1(double value) {
                        attribute1 = value;
                    }

                    public int fn11(int arg) {
                        return 11 * arg;
                    }

                    public String fn12(String arg) {
                        return "12" + arg;
                    }

                    public int fn21(int arg) {
                        return 21 * arg;
                    }

                    public String fn22(String arg) {
                        return "22" + arg;
                    }

                    public double getatt3() {
                        return attribute3;
                    }

                    public void setatt3(double value) {
                        attribute3 = value;
                    }

                    public int fn31(int arg) {
                        return 31 * arg;
                    }

                    public String fn32(String arg) {
                        return "32" + arg;
                    }

                    public int fn33() {
                        return 33;
                    }

                    public int fn41(int arg) {
                        return 41 * arg;
                    }

                    public int fn61(int arg) {
                        return 61 * arg;
                    }

                    public String fn62(String arg) {
                        return "62" + arg;
                    }

                    public int fn71(int arg) {
                        return 71 * arg;
                    }

                    public String fn72(String arg) {
                        return "72" + arg;
                    }

                    public int fn73() {
                        return 73;
                    }

                    private double attribute1 = 0.0;
                    private double attribute3 = 0.0;
                };
        }

        private static final class CheckFailed extends Exception {
            CheckFailed(String message) {
                super(message);
            }
        }

        private static void checkEqual(int value, int argument)
            throws CheckFailed
        {
            if (argument != value) {
                throw new CheckFailed(value + " != " + argument);
            }
        }

        private static void checkEqual(double value, double argument)
            throws CheckFailed
        {
            if (argument != value) {
                throw new CheckFailed(value + " != " + argument);
            }
        }

        private static void checkEqual(String value, String argument)
            throws CheckFailed
        {
            if (!argument.equals(value)) {
                throw new CheckFailed(value + " != " + argument);
            }
        }

        public String testMulti(XMulti multi) {
            try {
                checkEqual(0.0, multi.getatt1());
                multi.setatt1(0.1);
                checkEqual(0.1, multi.getatt1());
                checkEqual(11 * 1, multi.fn11(1));
                checkEqual("12" + "abc", multi.fn12("abc"));
                checkEqual(21 * 2, multi.fn21(2));
                checkEqual("22" + "de", multi.fn22("de"));
                checkEqual(0.0, multi.getatt3());
                multi.setatt3(0.3);
                checkEqual(0.3, multi.getatt3());
                checkEqual(31 * 3, multi.fn31(3));
                checkEqual("32" + "f", multi.fn32("f"));
                checkEqual(33, multi.fn33());
                checkEqual(41 * 4, multi.fn41(4));
                checkEqual(61 * 6, multi.fn61(6));
                checkEqual("62", multi.fn62(""));
                checkEqual(71 * 7, multi.fn71(7));
                checkEqual("72" + "g", multi.fn72("g"));
                checkEqual(73, multi.fn73());
            } catch (CheckFailed f) {
                return f.getMessage();
            }
            return "";
        }

        // XBridgeTest
        public TestDataElements raiseException(short nArgumentPos, String rMsg, Object xContext)
            throws com.sun.star.lang.IllegalArgumentException,
                   com.sun.star.uno.RuntimeException
        {
            throw new com.sun.star.lang.IllegalArgumentException(rMsg, xContext, nArgumentPos);
        }

        public void raiseRuntimeExceptionOneway(String rMsg, Object xContext) throws com.sun.star.uno.RuntimeException {
            throw new com.sun.star.uno.RuntimeException(rMsg, xContext);
        }

        private void dothrow( com.sun.star.uno.RuntimeException t )
            throws com.sun.star.uno.RuntimeException
        {
            throw t;
        }
        public int getRuntimeException()
            throws com.sun.star.uno.RuntimeException
        {
            dothrow( new com.sun.star.uno.RuntimeException(
                         _string, _xInterface ) );
            return 0; // dummy
        }

        public void setRuntimeException(int _runtimeexception) throws com.sun.star.uno.RuntimeException {
            throw new com.sun.star.uno.RuntimeException(_string, _xInterface);
        }



        // XServiceInfo
        public String getImplementationName() throws com.sun.star.uno.RuntimeException {
            return __serviceName;
        }

        public boolean supportsService(String rServiceName) throws com.sun.star.uno.RuntimeException {
            String rSNL[] = getSupportedServiceNames();

            for(int nPos = rSNL.length; (nPos--) != 0;) {
                if (rSNL[nPos].equals(rServiceName))
                    return true;
            }

            return false;
        }

        public String [] getSupportedServiceNames() throws com.sun.star.uno.RuntimeException {
            return new String[]{__serviceName};
        }

        // XTypeProvider
        public Type[] getTypes() throws com.sun.star.uno.RuntimeException {
            try {
                return new Type[]{new Type(XBridgeTest.class), new Type(XRecursiveCall.class), new Type(XServiceInfo.class), new Type(XTypeProvider.class)};
            }
            catch(Exception exception) {
                throw new com.sun.star.uno.RuntimeException(exception);
            }
        }

        public byte[] getImplementationId() throws com.sun.star.uno.RuntimeException {
            return new byte[0];
        }

        //XBridgeTest2
        public boolean[] setSequenceBool( /*IN*/boolean[] aSeq )
        {
            arBool = aSeq;
            return aSeq;
        }
        public char[] setSequenceChar( /*IN*/char[] aSeq )
        {
            arChar = aSeq;
            return aSeq;
        }
        public byte[] setSequenceByte( /*IN*/byte[] aSeq )
        {
            arByte = aSeq;
            return aSeq;
        }
        public short[] setSequenceShort( /*IN*/short[] aSeq )
        {
            arShort = aSeq;
            return aSeq;
        }
        public short[] setSequenceUShort( /*IN*/short[] aSeq )
        {
            arUShort = aSeq;
            return aSeq;
        }
        public int[] setSequenceLong( /*IN*/int[] aSeq )
        {
            arLong = aSeq;
            return aSeq;
        }
        public int[] setSequenceULong( /*IN*/int[] aSeq )
        {
            arULong = aSeq;
            return aSeq;
        }
        public long[] setSequenceHyper( /*IN*/long[] aSeq )
        {
            arHyper = aSeq;
            return aSeq;
        }
        public long[] setSequenceUHyper( /*IN*/long[] aSeq )
        {
            arUHyper = aSeq;
            return aSeq;
        }
        public float[] setSequenceFloat( /*IN*/float[] aSeq )
        {
            arFloat = aSeq;
            return aSeq;
        }
        public double[] setSequenceDouble( /*IN*/double[] aSeq )
        {
            arDouble = aSeq;
            return aSeq;
        }
        public TestEnum[] setSequenceEnum( /*IN*/TestEnum[] aSeq )
        {
            arEnum = aSeq;
            return aSeq;
        }
        public String[] setSequenceString( /*IN*/String[] aSeq )
        {
            arString = aSeq;
            return aSeq;
        }
        public java.lang.Object[] setSequenceXInterface( /*IN*/java.lang.Object[] aSeq )
        {
            arObject = aSeq;
            return aSeq;
        }
        public java.lang.Object[] setSequenceAny( /*IN*/java.lang.Object[] aSeq )
        {
            arAny = aSeq;
            return aSeq;
        }
        public TestElement[] setSequenceStruct( /*IN*/TestElement[] aSeq )
        {
            _testElements = aSeq;
            return aSeq;
        }
        public int[][] setDim2( /*IN*/int[][] aSeq )
        {
            arLong2 = aSeq;
            return aSeq;
        }
        public int[][][] setDim3( /*IN*/int[][][] aSeq )
        {
            arLong3 = aSeq;
            return aSeq;
        }
        public void setSequencesInOut( /*INOUT*/boolean[][] aSeqBoolean,
                                       /*INOUT*/char[][] aSeqChar, /*INOUT*/byte[][] aSeqByte,
                                       /*INOUT*/short[][] aSeqShort, /*INOUT*/short[][] aSeqUShort,
                                       /*INOUT*/int[][] aSeqLong, /*INOUT*/int[][] aSeqULong,
                                       /*INOUT*/long[][] aSeqHyper, /*INOUT*/long[][] aSeqUHyper,
                                       /*INOUT*/float[][] aSeqFloat, /*INOUT*/double[][] aSeqDouble,
                                       /*INOUT*/TestEnum[][] aSeqEnum, /*INOUT*/String[][] aSeqString,
                                       /*INOUT*/java.lang.Object[][] aSeqXInterface,
                                       /*INOUT*/java.lang.Object[][] aSeqAny,
                                       /*INOUT*/int[][][] aSeqDim2, /*INOUT*/int[][][][] aSeqDim3 )
        {
            arBool = aSeqBoolean[0];
            arChar = aSeqChar[0];
            arByte = aSeqByte[0];
            arShort = aSeqShort[0];
            arUShort = aSeqUShort[0];
            arLong = aSeqLong[0];
            arULong = aSeqULong[0];
            arFloat = aSeqFloat[0];
            arDouble = aSeqDouble[0];
            arEnum = aSeqEnum[0];
            arString = aSeqString[0];
            arObject = aSeqXInterface[0];
            arAny = aSeqAny[0];
            arLong2 = aSeqDim2[0];
            arLong3 = aSeqDim3[0];
        }
        public void setSequencesOut( /*OUT*/boolean[][] aSeqBoolean, /*OUT*/char[][] aSeqChar,
                                     /*OUT*/byte[][] aSeqByte, /*OUT*/short[][] aSeqShort,
                                     /*OUT*/short[][] aSeqUShort, /*OUT*/int[][] aSeqLong,
                                     /*OUT*/int[][] aSeqULong, /*OUT*/long[][] aSeqHyper,
                                     /*OUT*/long[][] aSeqUHyper, /*OUT*/float[][] aSeqFloat,
                                     /*OUT*/double[][] aSeqDouble, /*OUT*/TestEnum[][] aSeqEnum,
                                     /*OUT*/String[][] aSeqString,
                                     /*OUT*/java.lang.Object[][] aSeqXInterface,
                                     /*OUT*/java.lang.Object[][] aSeqAny, /*OUT*/int[][][] aSeqDim2,
                                     /*OUT*/int[][][][] aSeqDim3 )
        {
             aSeqBoolean[0] = arBool;
             aSeqChar[0] = arChar;
             aSeqByte[0] = arByte;
             aSeqShort[0] = arShort;
             aSeqUShort[0] = arUShort;
             aSeqLong[0] = arLong;
             aSeqULong[0] = arULong;
             aSeqHyper[0] = arHyper;
             aSeqUHyper[0] = arUHyper;
             aSeqFloat[0] = arFloat;
             aSeqDouble[0] = arDouble;
             aSeqEnum[0] = arEnum;
             aSeqString[0] = arString;
             aSeqXInterface[0] = arObject;
             aSeqAny[0] = arAny;
             aSeqDim2[0] = arLong2;
             aSeqDim3[0] = arLong3;
        }

        public void testConstructorsService(XComponentContext context)
            throws BadConstructorArguments
        {
            Constructors.create1(context,
                true,
                Byte.MIN_VALUE,
                Short.MIN_VALUE,
                (short) -1,
                Integer.MIN_VALUE,
                -1,
                Long.MIN_VALUE,
                -1L,
                0.123f,
                0.456,
                'X',
                "test",
                Type.ANY,
                new Any(Type.BOOLEAN, Boolean.TRUE),
                new boolean[] { true },
                new byte[] { Byte.MIN_VALUE },
                new short[] { Short.MIN_VALUE },
                new short[] { (short) -1 },
                new int[] { Integer.MIN_VALUE },
                new int[] { -1 },
                new long[] { Long.MIN_VALUE },
                new long[] { -1L },
                new float[] { 0.123f },
                new double[] { 0.456 },
                new char[] { 'X' },
                new String[] { "test" },
                new Type[] { Type.ANY },
                new Boolean[] { Boolean.TRUE },
                new boolean[][] { new boolean[] { true } },
                new Object[][] {
                    new Object[] { new Any(Type.BOOLEAN, Boolean.TRUE) } },
                new TestEnum[] { TestEnum.TWO },
                new TestStruct[] { new TestStruct(10) },
                new TestPolyStruct[] { new TestPolyStruct(Boolean.TRUE) },
                new TestPolyStruct[] {
                    new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE)) },
                new Object[] { null },
                TestEnum.TWO,
                new TestStruct(10),
                new TestPolyStruct(Boolean.TRUE),
                new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE)),
                null);
            Constructors.create2(context, new Object[] {
                Boolean.TRUE,
                Byte.valueOf(Byte.MIN_VALUE),
                Short.valueOf(Short.MIN_VALUE),
                new Any(Type.UNSIGNED_SHORT, Short.valueOf((short) -1)),
                Integer.valueOf(Integer.MIN_VALUE),
                new Any(Type.UNSIGNED_LONG, Integer.valueOf(-1)),
                Long.valueOf(Long.MIN_VALUE),
                new Any(Type.UNSIGNED_HYPER, Long.valueOf(-1L)),
                new Float(0.123f),
                new Double(0.456),
                new Character('X'),
                "test",
                Type.ANY,
                new Any(Type.BOOLEAN, Boolean.TRUE),
                new boolean[] { true },
                new byte[] { Byte.MIN_VALUE },
                new short[] { Short.MIN_VALUE },
                new Any(
                    new Type("[]unsigned short"), new short[] { (short) -1 }),
                new int[] { Integer.MIN_VALUE },
                new Any(new Type("[]unsigned long"), new int[] { -1 }),
                new long[] { Long.MIN_VALUE },
                new Any(new Type("[]unsigned hyper"), new long[] { -1L }),
                new float[] { 0.123f },
                new double[] { 0.456 },
                new char[] { 'X' },
                new String[] { "test" },
                new Type[] { Type.ANY },
                new Any(new Type("[]any"), new Boolean[] { Boolean.TRUE }),
                new boolean[][] { new boolean[] { true } },
                new Object[][] {
                    new Object[] { new Any(Type.BOOLEAN, Boolean.TRUE) } },
                new TestEnum[] { TestEnum.TWO },
                new TestStruct[] { new TestStruct(10) },
                new Any(
                    new Type(
                        "[]test.testtools.bridgetest.TestPolyStruct<boolean>"),
                    new TestPolyStruct[] { new TestPolyStruct(Boolean.TRUE) }),
                new Any(
                    new Type("[]test.testtools.bridgetest.TestPolyStruct<any>"),
                    new TestPolyStruct[] {
                        new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE))
                    }),
                new XInterface[] { null },
                TestEnum.TWO,
                new TestStruct(10),
                new Any(
                    new Type(
                        "test.testtools.bridgetest.TestPolyStruct<boolean>"),
                    new TestPolyStruct(Boolean.TRUE)),
                new Any(
                    new Type("test.testtools.bridgetest.TestPolyStruct<any>"),
                    new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE))),
                null });
            Constructors2.create1(
                context,
                new TestPolyStruct(Type.LONG),
                new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE)),
                new TestPolyStruct(Boolean.TRUE),
                new TestPolyStruct(Byte.valueOf(Byte.MIN_VALUE)),
                new TestPolyStruct(Short.valueOf(Short.MIN_VALUE)),
                new TestPolyStruct(Integer.valueOf(Integer.MIN_VALUE)),
                new TestPolyStruct(Long.valueOf(Long.MIN_VALUE)),
                new TestPolyStruct(new Character('X')),
                new TestPolyStruct("test"),
                new TestPolyStruct(new Float(0.123f)),
                new TestPolyStruct(new Double(0.456)),
                new TestPolyStruct(new com.sun.star.lib.uno.helper.ComponentBase()),
                new TestPolyStruct(new com.sun.star.lib.uno.helper.ComponentBase()),
                new TestPolyStruct(TestEnum.TWO),
                new TestPolyStruct(new TestPolyStruct2(new Character('X'),
                    new Any(Type.BOOLEAN, Boolean.TRUE))),
                new TestPolyStruct(new TestPolyStruct2(new TestPolyStruct2(
                    new Character('X'), new Any(Type.BOOLEAN, Boolean.TRUE)), "test")),
                new TestPolyStruct2("test", new TestPolyStruct2(new Character('X'),
                    new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE)))),
                new TestPolyStruct2( new TestPolyStruct2(new Character('X'),
                    new Any(Type.BOOLEAN, Boolean.TRUE)), new TestPolyStruct(new Character('X'))),
                new TestPolyStruct(new Type[] { Type.LONG}),
                new TestPolyStruct(new Any[] { new Any(Type.BOOLEAN, Boolean.TRUE) }),
                new TestPolyStruct(new boolean[] {true}),
                new TestPolyStruct(new byte[] {Byte.MIN_VALUE}),
                new TestPolyStruct(new short[] {Short.MIN_VALUE}),
                new TestPolyStruct(new int[] {Integer.MIN_VALUE}),
                new TestPolyStruct(new long[] {Long.MIN_VALUE}),
                new TestPolyStruct(new char[] {'X'}),
                new TestPolyStruct(new String[] {"test"}),
                new TestPolyStruct(new float[] {0.123f}),
                new TestPolyStruct(new double[] {0.456d}),
                new TestPolyStruct(new Object[] {new com.sun.star.lib.uno.helper.ComponentBase()}),
                new TestPolyStruct(new com.sun.star.lang.XComponent[] {new com.sun.star.lib.uno.helper.ComponentBase()}),
                new TestPolyStruct(new TestEnum[] {TestEnum.TWO}),
                new TestPolyStruct(new TestPolyStruct2[] {new TestPolyStruct2(
                    new Character('X'), new Any[] {new Any(Type.BOOLEAN, Boolean.TRUE)})}),
                new TestPolyStruct(new TestPolyStruct2[] {new TestPolyStruct2(
                    new TestPolyStruct(new Character('X')), new Any[] {new Any(Type.BOOLEAN, Boolean.TRUE)})}),
                new TestPolyStruct(new int[][] { new int[] {Integer.MIN_VALUE} }),
                new TestPolyStruct[]{ new TestPolyStruct(Integer.valueOf(Integer.MIN_VALUE))},
                new TestPolyStruct[]{new TestPolyStruct(new TestPolyStruct2(
                    new Character('X'), new Any(Type.BOOLEAN, Boolean.TRUE)))},
                new TestPolyStruct[]{new TestPolyStruct(new TestPolyStruct2(
                    new TestPolyStruct2(new Character('X'), new Any(Type.BOOLEAN, Boolean.TRUE)), "test"))},
                new TestPolyStruct2[]{new TestPolyStruct2("test", new TestPolyStruct2(
                    new Character('X'), new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE))))},
                new TestPolyStruct2[]{new TestPolyStruct2(new TestPolyStruct2(new Character('X'), new Any(
                    Type.BOOLEAN, Boolean.TRUE)),new TestPolyStruct(new Character('X')))},
                new TestPolyStruct[][]{new TestPolyStruct[]{new TestPolyStruct(new Character('X'))}},
                new TestPolyStruct[][]{new TestPolyStruct[]{
                    new TestPolyStruct(new TestPolyStruct2(new Character('X'), new Any(Type.BOOLEAN, Boolean.TRUE)))}},
                new TestPolyStruct[][]{new TestPolyStruct[] {new TestPolyStruct(new TestPolyStruct2(
                    new TestPolyStruct2(new Character('X'),new Any(Type.BOOLEAN, Boolean.TRUE)), "test"))}},
                new TestPolyStruct2[][]{new TestPolyStruct2[]{new TestPolyStruct2(
                    "test", new TestPolyStruct2(new Character('X'),new TestPolyStruct(new Any(Type.BOOLEAN, Boolean.TRUE))))}},
                new TestPolyStruct2[][]{new TestPolyStruct2[]{new TestPolyStruct2(
                    new TestPolyStruct2(new Character('X'),new Any(Type.BOOLEAN, Boolean.TRUE)),
                        new TestPolyStruct(new Character('X')))}});
        }

        public XCurrentContextChecker getCurrentContextChecker() {
            return new CurrentContextChecker();
        }
    }

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be uses if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                            XMultiServiceFactory multiFactory,
                                                            XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

          if(implName.equals(_TestObject.class.getName()))
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_TestObject.class,
                                                                    _TestObject.__serviceName,
                                                                    multiFactory,
                                                                    regKey);

        else if(implName.equals(_PerformancTestObject.class.getName()))
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_PerformancTestObject.class,
                                                                    _PerformancTestObject.__serviceName,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }
}
