package com.sun.star.uno;
import com.sun.star.lang.IllegalArgumentException;

/** This class provides static methods which aim at exploring the contents of an
 * Any and extracting its value. All public methods take an Object argument that
 * either is the immediate object, such as Boolean, Type, interface implementation,
 * or an Any that contains an object. <br>The methods which extract the value do a
 * widening conversion. See the method comments for the respective conversions.
 */
public class AnyConverter
{
    /** checks if the any contains the idl type <code>void</code>.
        @param object the object to check
        @return true when the any is void, false otherwise
     */
    static public boolean isVoid( Object object){
        return containsType( TypeClass.VOID, object);
    }

    /** checks if the any contains a value of the idl type <code>char</code>.
        @param object the object to check
        @return true when the any contains a char, false otherwise.
     */
    static public boolean isChar(Object object){
        return containsType( TypeClass.CHAR, object);
    }

    /** checks if the any contains a value of the idl type <code>boolean</code>.
        @param object the object to check
        @return true when the any contains a boolean, false otherwise.
     */
    static public boolean isBoolean(Object object){
        return containsType( TypeClass.BOOLEAN, object);
    }

    /** checks if the any contains a value of the idl type <code>byte</code>.
        @param object the object to check
        @return true when the any contains a byte, false otherwise.
     */
    static public boolean isByte(Object object){
        return containsType( TypeClass.BYTE, object);
    }

    /** checks if the any contains a value of the idl type <code>short</code>.
        @param object the object to check
        @return true when the any contains a short, false otherwise.
     */
    static public boolean isShort(Object object){
        return containsType( TypeClass.SHORT, object);
    }

    /** checks if the any contains a value of the idl type <code>long</code> (which maps to a java-int).
        @param object the object to check
        @return true when the any contains a int, false otherwise.
     */
    static public boolean isInt(Object object){
        return containsType( TypeClass.LONG, object);
    }

    /** checks if the any contains a value of the idl type <code>hyper</code> (which maps to a java-long).
        @param object the object to check
        @return true when the any contains a long, false otherwise.
     */
    static public boolean isLong(Object object){
        return containsType( TypeClass.HYPER, object);
    }

    /** checks if the any contains a value of the idl type <code>float</code>.
        @param object the object to check
        @return true when the any contains a float, false otherwise.
     */
    static public boolean isFloat(Object object){
        return containsType( TypeClass.FLOAT, object);
    }

    /** checks if the any contains a value of the idl type <code>double</code>.
        @param object the object to check
        @return true when the any contains a double, false otherwise.
     */
    static public boolean isDouble(Object object){
        return containsType( TypeClass.DOUBLE, object);
    }

    /** checks if the any contains a value of the idl type <code>string</code>.
        @param object the object to check
        @return true when the any contains a string, false otherwise.
     */
    static public boolean isString(Object object){
        return containsType( TypeClass.STRING, object);
    }


    /** checks if the any contains a value of the idl type <code>type</code>.
        @param object the object to check
        @return true when the any contains a type, false otherwise.
     */
    static public boolean isType(Object object){
        return containsType( TypeClass.TYPE, object);
    }

    /** checks if the any contains a value which implements interfaces.
        If <em>object</em> is an any with an interface type, then true is also returned if the any contains
        a null reference. This is because interfacec are allowed to have a null value contrary
        to other UNO types.
        @param object the object to check
        @return true when the any contains an object which implements interfaces, false otherwise.
     */
    static public boolean isObject(Object object){
        boolean retVal= false;
        if (object instanceof Any)
        {
            if( ((Any) object).getObject() == null )
            {
                Type _t= ((Any) object).getType();
                if (_t.getTypeClass() == TypeClass.INTERFACE)
                    return true;
            }
            object= ((Any) object).getObject();

        }
        if (object != null && object.getClass().getInterfaces().length > 0)
            retVal= true;
        return retVal;
    }

    /** checks if the any contains UNO idl sequence value ( meaning a java array
        containing elements which are values of UNO idl types).
        @param object the object to check
        @return true when the any contains an object which implements interfaces, false otherwise.
     */
    static public boolean isArray( Object object){
        return containsType( TypeClass.SEQUENCE, object);
    }

    /** converts an Char object or an Any object containing a Char object into a simple char.
        @param object the object to convert
        @return the char contained within the object
        @throws com.sun.star.lang.IllegalArgumentException in case no char is contained within object
        @see #isChar
    */
    static public char  toChar(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Character ret=(Character) convertSimple( TypeClass.CHAR, null, object);
        return ret.charValue();
    }

    /** converts an Boolean object or an Any object containing a Boolean object into a simple boolean.
        @param object the object to convert
        @return the boolean contained within the object
        @throws com.sun.star.lang.IllegalArgumentException in case no boolean is contained within object
        @see #isBoolean
    */
    static public boolean toBoolean(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Boolean ret= (Boolean)  convertSimple( TypeClass.BOOLEAN, null, object);
        return ret.booleanValue();
    }

    /** converts an Byte object or an Any object containing a Byte object into a simple byte.
        @param object the object to convert
        @return the boolean contained within the object
        @throws com.sun.star.lang.IllegalArgumentException in case no byte is contained within object
        @see #isBoolean
    */
    static public byte toByte(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Byte ret= (Byte) convertSimple( TypeClass.BYTE, null, object);
        return ret.byteValue();
    }

    /** converts a number object into a simple short and allows widening conversions.
        Allowed argument types are Byte, Short or Any containing these types.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no short or byte is contained within object
        @return the short contained within the object
     */
    static public short toShort(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Short ret= (Short) convertSimple( TypeClass.SHORT, null, object);
        return ret.shortValue();
    }

    /** converts a number object into a simple int and allows widening conversions.
        Allowed argument types are Byte, Short, Integer or Any containing these types.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no short, byte or int is contained within object.
        @return the int contained within the object
     */
    static public int toInt(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Integer ret= (Integer) convertSimple( TypeClass.LONG, null, object);
        return ret.intValue();
    }

    /** converts a number object into a simple long and allows widening conversions.
        Allowed argument types are Byte, Short, Integer, Long or Any containing these types.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no short, byte, int or long
                is contained within object.
        @return the long contained within the object
     */
    static public long toLong(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Long ret= (Long) convertSimple( TypeClass.HYPER, null, object);
        return ret.longValue();
    }

    /** converts a number object into a simple float and allows widening conversions.
        Allowed argument types are Byte, Short, Float or Any containing these types.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no byte, short or float
                is contained within object.
        @return the float contained within the object
     */
    static public float toFloat(Object object) throws com.sun.star.lang.IllegalArgumentException{
        Float ret= (Float) convertSimple( TypeClass.FLOAT,null, object);
        return ret.floatValue();
    }

    /** converts a number object into a simple double and allows widening conversions.
        Allowed argument types are Byte, Short, Int, Float, Double or Any containing these types.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no byte, short, int, float
                or double is contained within object.
        @return the double contained within the object
     */
    static public double toDouble(Object object) throws com.sun.star.lang.IllegalArgumentException {
        Double ret= (Double) convertSimple( TypeClass.DOUBLE, null, object);
        return ret.doubleValue();
    }

    /** converts a string or an any containing a string into a string.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no string is contained within object.
        @return the string contained within the object
     */
    static public String toString(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (String) convertSimple( TypeClass.STRING, null, object);
    }

    /** converts a Type or an any containing a Type into a Type.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no type is contained within object.
        @return the type contained within the object
     */
    static public Type toType(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (Type) convertSimple( TypeClass.TYPE, null, object);
    }

    /** converts a UNO object or an Any containing a UNO object into an UNO object of a specified type.
     *  The argument <em>object</em> is examined for implemented interfaces. If it has implemented interfaces
     *  then the method attempts to query for the interface specified by the <em>type</em> argument. That query
     *  (UnoRuntime.queryInterface) might return null, if the interface is not implemented.
     *  If <em>object</em> is an Any which contains an interface type and a null reference,
     *  then null is returned.
     *
     *  @param type the Type of the returned value
     *  @param object the object that is to be converted
     *  @return the object contained within the object
     *  @throws com.sun.star.lang.IllegalArgumentException in case no UNO object is contained within object.
     */
    static public Object toObject(Type type, Object object)
        throws com.sun.star.lang.IllegalArgumentException{
        return convertSimple( TypeClass.INTERFACE,  type, object);
    }

    /** converts an array or an any containing an array into an array.
        @param object the object to convert
        @throws com.sun.star.lang.IllegalArgumentException in case no array is contained within object.
        @return the array contained within the object
     */
    static public Object toArray( Object object) throws com.sun.star.lang.IllegalArgumentException {
        return convertSimple( TypeClass.SEQUENCE, null, object);
    }

    /**
       Examines the argument <em>object</em> if is correspond to the type in argument <em>what</em>.
       <em>object</em> is either matched directly against the type or if it is an any then the
       contained object is matched against the type.
    */
    static private boolean containsType( TypeClass what, Object object){
        boolean retVal= false;
        if (object instanceof Any)
            object= ((Any) object).getObject();

        Type _t= object == null ? new Type(void.class) : new Type( object.getClass());
        if (_t.getTypeClass().getValue() == what.getValue())
            retVal= true;

        return retVal;
    }

    static private Object convertSimple( TypeClass destTClass, Type destType, Object src)
        throws com.sun.star.lang.IllegalArgumentException {

        Object _src= src;
        int srcTypeValue=0;

        if (src instanceof Any)
        {
            Any _a= (Any) src;
            // If the any contains an Interface with a null reference then it is valid and
            // null is returned
            if ( _a.getObject() == null && _a.getType().getTypeClass() == TypeClass.INTERFACE)
                return null;
            _src= _a.getObject();
        }

        if (_src != null)
        {
            Type srcType= new Type(_src.getClass());

            if( destTClass == TypeClass.INTERFACE)
            {
                if( _src.getClass().getInterfaces().length == 0)
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "The argument does not implement interfaces");
                else
                    srcTypeValue= TypeClass.INTERFACE.getValue();
            }
            else
                srcTypeValue= new Type(_src.getClass()).getTypeClass().getValue();

            switch( destTClass.getValue())
            {
            case TypeClass.CHAR_value:
                if( srcTypeValue == TypeClass.CHAR_value)
                    return _src;
                break;
            case TypeClass.BOOLEAN_value:
                if( srcTypeValue == TypeClass.BOOLEAN_value)
                    return _src;
                break;
            case TypeClass.BYTE_value:
                if( srcTypeValue == TypeClass.BYTE_value)
                    return _src;
                break;
            case TypeClass.SHORT_value:
                switch( srcTypeValue)
                {
                case TypeClass.BYTE_value:
                    return new Short( ((Byte)_src).byteValue());
                case TypeClass.SHORT_value:
                    return _src;
                }
                break;
            case TypeClass.LONG_value:
                switch( srcTypeValue)
                {
                case TypeClass.BYTE_value:
                    return new Integer( ((Byte)_src).byteValue());
                case TypeClass.SHORT_value:
                    return new Integer( ((Short)_src).shortValue());
                case TypeClass.LONG_value:
                    return _src;
                }
                break;
            case TypeClass.HYPER_value:
                switch( srcTypeValue)
                {
                case TypeClass.BYTE_value:
                    return new Long( ((Byte)_src).byteValue());
                case TypeClass.SHORT_value:
                    return new Long( ((Short)_src).shortValue());
                case TypeClass.LONG_value:
                    return new Long( ((Integer)_src).intValue());
                case TypeClass.HYPER_value:
                    return _src;
                }
                break;
            case TypeClass.FLOAT_value:
                switch( srcTypeValue)
                {
                case TypeClass.BYTE_value:
                    return new Float( ((Byte)_src).byteValue());
                case TypeClass.SHORT_value:
                    return new Float( ((Short)_src).shortValue());
                case TypeClass.FLOAT_value:
                    return _src;
                }
                break;
            case TypeClass.DOUBLE_value:
                switch( srcTypeValue)
                {
                case TypeClass.BYTE_value:
                    return new Double( ((Byte)_src).byteValue());
                case TypeClass.SHORT_value:
                    return new Double( ((Short)_src).shortValue());
                case TypeClass.LONG_value:
                    return new Double( ((Integer)_src).intValue());
                case TypeClass.FLOAT_value:
                    return new Double( ((Float)_src).floatValue());
                case TypeClass.DOUBLE_value:
                    return _src;
                }
                break;
            case TypeClass.STRING_value:
                if( srcTypeValue == TypeClass.STRING_value)
                    return _src;
                break;

            case TypeClass.TYPE_value:
                if( srcTypeValue == TypeClass.TYPE_value)
                    return _src;
                break;
            case TypeClass.INTERFACE_value:
                if( srcTypeValue == TypeClass.INTERFACE_value)
                    return UnoRuntime.queryInterface( destType, _src);
                break;
            case TypeClass.SEQUENCE_value:
                if( srcTypeValue == TypeClass.SEQUENCE_value)
                    return _src;
                break;

            }
        }
        throw new com.sun.star.lang.IllegalArgumentException(
            "The Argument did not hold the proper type");
    }
}
