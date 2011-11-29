/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



using System;
using System.Diagnostics;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.test.performance;

namespace testobj
{

[ServiceImplementation( "com.sun.star.test.performance.cli_uno.TestObject" )]
public class PerformanceTestObject : ServiceBase, XPerformanceTest
{
    private XComponentContext m_xContext;
    
    public PerformanceTestObject( XComponentContext xContext )
    {
        m_xContext = xContext;
    }
    public PerformanceTestObject()
    {
    }
    
    private int      _long;   
    private long     _hyper;    
    private float    _float;
    private double   _double;
    private String   _string = "";
    private Object   _xInterface;
    private Any   _any;
    private Object[]   _interface_sequence = new Object[0];
    private ComplexTypes _complexTypes = new ComplexTypes();
    
    // Attributes
    public int getLong_attr() { return _long; }
    public void setLong_attr( int _long_attr ) { _long = _long_attr; }
    public long getHyper_attr() { return _hyper; }
    public void setHyper_attr( long _hyper_attr ) { _hyper = _hyper_attr; }
    public float getFloat_attr() { return _float; }
    public void setFloat_attr( float _float_attr ) { _float = _float; }
    public double getDouble_attr() { return _double; }
    public void setDouble_attr( double _double_attr ) { _double = _double_attr; }
    public String getString_attr() { return _string; }
    public void setString_attr( String _string_attr ) { _string = _string_attr; }
    public Object getInterface_attr() { return _xInterface; }
    public void setInterface_attr( Object _interface_attr ) { _xInterface = _interface_attr; }
    public Any getAny_attr() { return _any; }
    public void setAny_attr( ref Any _any_attr ) { _any = _any_attr; }
    public Object[] getSequence_attr() { return _interface_sequence; }
    public void setSequence_attr(Object[] _sequence_attr ) { _interface_sequence = _sequence_attr; }
    public ComplexTypes getStruct_attr() { return _complexTypes; }
    public void setStruct_attr( ComplexTypes _struct_attr ) { _complexTypes = _struct_attr; }
    
    // Methods
    public void async() {}
    public void sync(  ) {}
    public ComplexTypes complex_in( /*IN*/ComplexTypes aVal ) { return aVal; }
    public ComplexTypes complex_inout( /*INOUT*/ref ComplexTypes aVal ) { return aVal; }
    public void complex_oneway( /*IN*/ComplexTypes aVal ) {}
    public void complex_noreturn( /*IN*/ComplexTypes aVal ) {}
    public XPerformanceTest createObject(  ) { return new PerformanceTestObject(); }
    public int getLong() { return _long; }
    public void setLong(/*IN*/int n) { _long = n; }
    public long getHyper() { return _hyper; }
    public void setHyper(/*IN*/long n) { _hyper = n; }
    public float getFloat() { return _float; }
    public void setFloat( /*IN*/float f ) { _float = f; }
    public double getDouble(  ) { return _double; }
    public void setDouble( /*IN*/double f ) { _double = f; }    
    public String getString(  ) { return _string; }
    public void setString( /*IN*/String s ) { _string = s; }
    public Object getInterface(  ) { return _xInterface; }
    public void setInterface( /*IN*/Object x ) { _xInterface = x; }
    public Any getAny(  ) { return _any; }
    public void setAny( /*IN*/ref Any a ) { _any = a; }
    public Object[] getSequence(  ) { return _interface_sequence; }
    public void setSequence( /*IN*/Object[] seq )
    {
#if DEBUG
//         Debug.WriteLine( "#### " + GetType().FullName + ".setSequence:" + seq );
        Console.WriteLine( "#### " + GetType().FullName + ".setSequence:" + seq );
#endif
        _interface_sequence = seq;
    }
    public ComplexTypes getStruct(  ) { return _complexTypes; }
    public void setStruct( /*IN*/ComplexTypes c ) { _complexTypes = c; }
    public void raiseRuntimeException(  ) { throw new RuntimeException(); }    
}

}
