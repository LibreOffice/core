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
    public String stringA;
    public Object objectA;
    public Any anyA;
    public Type typeA;
    public XInterface xinterfaceA;
    public XTypeProvider xtypeproviderA;
    public boolean[] arBoolA;
    public char[] arCharA;
    public byte[] arByteA;
    public short[] arShortA;
    public int[] arIntA;
    public long[] arLongA;
    public float[] arFloatA;
    public double[] arDoubleA;
    public String[] arStringA;
    public Object[] arObjectA;
    public Any[] arAnyA;
    public XInterface[] arXinterfaceA;
    public boolean[][] ar2BoolA;
    public Boolean boolClassA;
    public Character charClassA;
    public Byte byteClassA;
    public Short shortClassA;
    public Integer intClassA;
    public Long longClassA;
    public Float floatClassA;
    public Double doubleClassA;

    // readonly
    public int roIntA= 100;
    public Integer roIntClassA= new Integer(100);
    public Object roObjectA= new Integer(101);
    public Any roAnyA= new Any( new Type(int.class), new Integer(102));

    // BOUND & CONSTRAINED
    public boolean bcBoolA;
    public Boolean bcBoolClassA;
    public Object bcObjectA;
    public Any bcAnyA;

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
        registerProperty("stringA", (short) 0);
        registerProperty("objectA", (short) 0);
        registerProperty("anyA", (short) 0);
        registerProperty("typeA", (short) 0);
        registerProperty("xinterfaceA", (short) 0);
        registerProperty("xtypeproviderA", (short) 0);
        registerProperty("arBoolA", (short) 0);
        registerProperty("arCharA", (short) 0);
        registerProperty("arByteA", (short) 0);
        registerProperty("arShortA", (short) 0);
        registerProperty("arIntA", (short) 0);
        registerProperty("arLongA", (short) 0);
        registerProperty("arFloatA", (short) 0);
        registerProperty("arDoubleA", (short) 0);
        registerProperty("arStringA", (short) 0);
        registerProperty("arObjectA", (short) 0);
        registerProperty("arAnyA", (short) 0);
        registerProperty("arXinterfaceA", (short) 0);
        registerProperty("ar2BoolA", (short) 0);
        registerProperty("boolClassA", (short) 0);
        registerProperty("charClassA", (short) 0);
        registerProperty("byteClassA", (short) 0);
        registerProperty("shortClassA", (short) 0);
        registerProperty("intClassA", (short) 0);
        registerProperty("longClassA", (short) 0);
        registerProperty("floatClassA", (short) 0);
        registerProperty("doubleClassA", (short) 0);
        registerProperty("roIntA", PropertyAttribute.READONLY);
        registerProperty("roIntClassA", PropertyAttribute.READONLY);
        registerProperty("roObjectA", PropertyAttribute.READONLY);
        registerProperty("roAnyA", PropertyAttribute.READONLY);
        registerProperty("bcBoolA",(short) ( PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED));
        registerProperty("bcBoolClassA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED));
        registerProperty("bcObjectA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED));
        registerProperty("bcAnyA", (short) (PropertyAttribute.BOUND | PropertyAttribute.CONSTRAINED));
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

    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)
    {
        return FactoryHelper.writeRegistryServiceInfo( PropTest.class.getName(),
        PropTest.__serviceName, regKey);
    }
}
