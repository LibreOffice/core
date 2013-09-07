/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

/*
This example shows a UNO component that inherits com.sun.star.lib.uno.helper.PropertySet in
order to provide implementations of  these interfaces:

      com.sun.star.beans.XPropertySet
      com.sun.star.beans.XMultiPropertySet
      com.sun.star.beans.XFastPropertySet
      com.sun.star.lang.XComponent
      com.sun.star.uno.XWeak
      com.sun.star.lang.XTypeProvider
*/
import com.sun.star.lib.uno.helper.PropertySet;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.beans.PropertyAttribute;

/**
   This component contains the followin properties:
 *  <table border>
 *  <tr>
 *  <th> Property Name </th>
 *  <th> Type Name </th>
 *  <th> PropertyAttribute </th>
 *  <tr><td>boolA</td><td>BOOLEAN</td><td> - </td</tr>
 *  <tr><td>charA</td><td>CHAR</td><td> - </td></tr>
 *  <tr><td>byteA</td><td>BYTE</td><td> - </td></tr>
 *  <tr><td>shortA</td><td>SHORT</td><td> - </td></tr>
 *  <tr><td>intA</td><td>LONG</td><td> - </td></tr>
 *  <tr><td>longA</td><td>HYPER</td><td> - </td></tr>
 *  <tr><td>floatA</td><td>FLOAT</td><td> - </td></tr>
 *  <tr><td>doubleA</td><td>DOUBLE</td><td> - </td></tr>
 *  <tr><td>stringA</td><td>STRING</td><td> - </td></tr>
 *  <tr><td>objectA</td><td>ANY</td><td> - </td></tr>
 *  <tr><td>anyA</td><td>ANY</td><td> - </td></tr>
 *  <tr><td>typeA</td><td>TYPE</td><td> - </td></tr>
 *  <tr><td>xinterfaceA</td><td>INTERFACE</td><td> - </td></tr>
 *  <tr><td>xtypeproviderA</td><td>INTERFACE</td><td> - </td></tr>
 *  <tr><td>arBoolA</td><td>[]boolean</td><td> - </td></tr>
 *  <tr><td>arCharA</td><td>[]char</td><td> - </td></tr>
 *  <tr><td>arByteA</td><td>[]byte</td><td> - </td></tr>
 *  <tr><td>arShortA</td><td>[]short</td><td> - </td></tr>
 *  <tr><td>arIntA</td><td>[]long</td><td> - </td></tr>
 *  <tr><td>arLongA</td><td>[]hyper</td><td> - </td></tr>
 *  <tr><td>arFloatA</td><td>[]float</td><td> - </td></tr>
 *  <tr><td>arDoubleA</td><td>[]double</td><td> - </td></tr>
 *  <tr><td>arStringA</td><td>[]string</td><td> - </td></tr>
 *  <tr><td>arObjectA</td><td>[]any</td><td> - </td></tr>
 *  <tr><td>arXinterfaceA</td><td>[]com.sun.star.uno.XInterface</td><td> - </td></tr>
 *  <tr><td>ar2BoolA</td><td>[][]boolean</td><td> - </td></tr>
 *  <tr><td>boolClassA</td><td>boolean</td><td> - </td></tr>
 *  <tr><td>charClassA</td><td>char</td><td> - </td></tr>
 *  <tr><td>byteClassA</td><td>byte</td><td> - </td></tr>
 *  <tr><td>shortClassA</td><td>short</td><td> - </td></tr>
 *  <tr><td>intClassA</td><td>long</td><td> - </td></tr>
 *  <tr><td>longClassA</td><td>hyper</td><td> - </td></tr>
 *  <tr><td>floatClassA</td><td>float</td><td> - </td></tr>
 *  <tr><td>doubleClassA</td><td>double</td><td> - </td></tr>
 *  <tr><td>roIntA</td><td>long</td><td>READONLY</td></tr>
 *  <tr><td>roIntClassA</td><td>long</td><td>READONLY</td></tr>
 *  <tr><td>roObjectA</td><td>any</td><td>READONLY</td></tr>
 *  <tr><td>roAnyA</td><td>any</td><td>READONLY</td></tr>
 *  <tr><td>bcBoolA</td><td>boolean</td><td>BOUND,CONSTRAINED</td></tr>
 *  <tr><td>bcBoolClassA</td><td>boolean</td><td>BOUND,CONSTRAINED</td></tr>
 *  <tr><td>bcObjectA</td><td>any</td><td>BOUND,CONSTRAINED</td></tr>
 *  <tr><td>bcAnyA</td><td>any</td><td>BOUND,CONSTRAINED</td></tr>
 *  <tr><td>mvIntA</td><td>long</td><td>MAYBEVOID</td></tr>
 *  <tr><td>mvObjectA</td><td>any</td><td>MAYBEVOID</td></tr>
 *  <tr><td>mvAnyA</td><td>any</td><td>MAYBEVOID</td></tr>
 *  <tr><td>mvXinterfaceA</td><td>any</td><td>MAYBEVOID</td></tr>
 </table>
*/

public class PropTest extends PropertySet implements XServiceInfo
{

    public boolean boolA;
    public char charA;
    public byte byteA;
    public short shortA;
    public int intA;
    public long longA;
    public float floatA;
    public double doubleA;
    public String stringA; // MAYBEVOID
    public Object objectA; // MAYBEVOID
    public Any anyA; // MAYBEVOID
    public Type typeA;// MAYBEVOID
    public XInterface xinterfaceA;// MAYBEVOID
    public XTypeProvider xtypeproviderA;// MAYBEVOID
    public boolean[] arBoolA; // MAYBEVOID
    public char[] arCharA; // MAYBEVOID
    public byte[] arByteA; // MAYBEVOID
    public short[] arShortA; // MAYBEVOID
    public int[] arIntA; // MAYBEVOID
    public long[] arLongA; // MAYBEVOID
    public float[] arFloatA; // MAYBEVOID
    public double[] arDoubleA; // MAYBEVOID
    public String[] arStringA; // MAYBEVOID
    public Object[] arObjectA; // MAYBEVOID
    public Any[] arAnyA; // MAYBEVOID
    public XInterface[] arXinterfaceA; // MAYBEVOID
    public boolean[][] ar2BoolA; // MAYBEVOID
    public Boolean boolClassA; // MAYBEVOID
    public Character charClassA; // MAYBEVOID
    public Byte byteClassA; // MAYBEVOID
    public Short shortClassA; // MAYBEVOID
    public Integer intClassA; // MAYBEVOID
    public Long longClassA; // MAYBEVOID
    public Float floatClassA; // MAYBEVOID
    public Double doubleClassA; // MAYBEVOID

    // readonly
    public int roIntA= 100;
    public Integer roIntClassA= new Integer(100);
    public Object roObjectA= new Integer(101);
    public Any roAnyA= new Any( new Type(int.class), new Integer(102));

    // BOUND & CONSTRAINED
    public boolean bcBoolA;
    public Boolean bcBoolClassA; // MAYBEVOID
    public Object bcObjectA; // MAYBEVOID
    public Any bcAnyA;       // MAYBEVOID

    // MAYBEVOID
    public Integer mvIntA;
    public Object mvObjectA;
    public Any mvAnyA;
    public XInterface mvXinterfaceA;

    public static final String __serviceName="PropTest";

    public PropTest() {
        registerProperty("boolA", (short) 0);
        registerProperty("charA", (short) 0);
        registerProperty("byteA", (short) 0);
        registerProperty("shortA", (short) 0);
        registerProperty("intA", (short) 0);
        registerProperty("longA", (short) 0);
        registerProperty("floatA", (short) 0);
        registerProperty("doubleA", (short) 0);
        registerProperty("stringA", PropertyAttribute.MAYBEVOID);
        registerProperty("objectA", PropertyAttribute.MAYBEVOID);
        registerProperty("anyA", PropertyAttribute.MAYBEVOID);
        registerProperty("typeA", PropertyAttribute.MAYBEVOID);
        registerProperty("xinterfaceA", PropertyAttribute.MAYBEVOID);
        registerProperty("xtypeproviderA", PropertyAttribute.MAYBEVOID);
        registerProperty("arBoolA", PropertyAttribute.MAYBEVOID);
        registerProperty("arCharA", PropertyAttribute.MAYBEVOID);
        registerProperty("arByteA", PropertyAttribute.MAYBEVOID);
        registerProperty("arShortA", PropertyAttribute.MAYBEVOID);
        registerProperty("arIntA", PropertyAttribute.MAYBEVOID);
        registerProperty("arLongA", PropertyAttribute.MAYBEVOID);
        registerProperty("arFloatA", PropertyAttribute.MAYBEVOID);
        registerProperty("arDoubleA", PropertyAttribute.MAYBEVOID);
        registerProperty("arStringA", PropertyAttribute.MAYBEVOID);
        registerProperty("arObjectA", PropertyAttribute.MAYBEVOID);
        registerProperty("arAnyA", PropertyAttribute.MAYBEVOID);
        registerProperty("arXinterfaceA", PropertyAttribute.MAYBEVOID);
        registerProperty("ar2BoolA", PropertyAttribute.MAYBEVOID);
        registerProperty("boolClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("charClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("byteClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("shortClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("intClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("longClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("floatClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("doubleClassA", PropertyAttribute.MAYBEVOID);
        registerProperty("roIntA", PropertyAttribute.READONLY);
        registerProperty("roIntClassA", PropertyAttribute.READONLY);
        registerProperty("roObjectA", PropertyAttribute.READONLY);
        registerProperty("roAnyA", PropertyAttribute.READONLY);
        registerProperty("bcBoolA",(short) ( PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED));
        registerProperty("bcBoolClassA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED | PropertyAttribute.MAYBEVOID));
        registerProperty("bcObjectA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED | PropertyAttribute.MAYBEVOID));
        registerProperty("bcAnyA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED |PropertyAttribute.MAYBEVOID));
        registerProperty("mvIntA", PropertyAttribute.MAYBEVOID);
        registerProperty("mvObjectA", PropertyAttribute.MAYBEVOID);
        registerProperty("mvAnyA", PropertyAttribute.MAYBEVOID);
        registerProperty("mvXinterfaceA", PropertyAttribute.MAYBEVOID);
    }


    // XServiceName
    public String getImplementationName(  )
    {
        return getClass().getName();
    }

    // XServiceName
    public boolean supportsService( /*IN*/String name )
    {
        if (name.equals(__serviceName))
            return true;
        return false;
    }

    //XServiceName
    public String[] getSupportedServiceNames(  )
    {
        String[] retValue= new String[]{__serviceName};
        return retValue;
    }

    public static XSingleServiceFactory __getServiceFactory(String implName,
    XMultiServiceFactory multiFactory,
    XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals( PropTest.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory( PropTest.class,
            PropTest.__serviceName,
            multiFactory,
            regKey);

        return xSingleServiceFactory;
    }

    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)
//     {
//         return FactoryHelper.writeRegistryServiceInfo( PropTest.class.getName(),
//         PropTest.__serviceName, regKey);
//     }
}
