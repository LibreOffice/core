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
    static public boolean isVoid( Object object){
        return containsType( TypeClass.VOID, object);
    }

    static public boolean isChar(Object object){
        return containsType( TypeClass.CHAR, object);
    }

    static public boolean isBoolean(Object object){
        return containsType( TypeClass.BOOLEAN, object);
    }

    static public boolean isByte(Object object){
        return containsType( TypeClass.BYTE, object);
    }

    static public boolean isShort(Object object){
        return containsType( TypeClass.SHORT, object);
    }

    static public boolean isInt(Object object){
        return containsType( TypeClass.LONG, object);
    }

    static public boolean isLong(Object object){
        return containsType( TypeClass.HYPER, object);
    }

    static public boolean isFloat(Object object){
        return containsType( TypeClass.FLOAT, object);
    }

    static public boolean isDouble(Object object){
        return containsType( TypeClass.DOUBLE, object);
    }

    static public boolean isString(Object object){
        return containsType( TypeClass.STRING, object);
    }

    static public boolean isType(Object object){
        return containsType( TypeClass.TYPE, object);
    }

    static public boolean isObject(Object object){
        boolean retVal= false;
        if( object.getClass().getInterfaces().length > 0)
            retVal= true;
        else
        {
            Type _t= new Type( object.getClass());
            if( _t.getTypeClass() == TypeClass.ANY)
            {
                Any _any= (Any)object;
                if( _any.getType().getTypeClass() == TypeClass.INTERFACE)
                    retVal= true;
            }
        }
        return retVal;
    }

    static public boolean isArray( Object object){
        return containsType( TypeClass.SEQUENCE, object);
    }

    static public char  toChar(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Character ret=(Character) convertSimple( TypeClass.CHAR, null, object);
        return ret.charValue();
    }

    static public boolean toBoolean(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Boolean ret= (Boolean)  convertSimple( TypeClass.BOOLEAN, null, object);
        return ret.booleanValue();
    }

    static public byte toByte(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Byte ret= (Byte) convertSimple( TypeClass.BYTE, null, object);
        return ret.byteValue();
    }

        /** Allowed argument types are Byte, Short or Any containing these types.
         */
    static public short toShort(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Short ret= (Short) convertSimple( TypeClass.SHORT, null, object);
        return ret.shortValue();
    }

        /** Allowed argument types are Byte, Short, Integer or Any containing these types.
         */
    static public int toInt(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Integer ret= (Integer) convertSimple( TypeClass.LONG, null, object);
        return ret.intValue();
    }

        /** Allowed argument types are Byte, Short, Integer, Long or Any containing these types.
         */
    static public long toLong(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Long ret= (Long) convertSimple( TypeClass.HYPER, null, object);
        return ret.longValue();
    }

        /** Allowed argument types are Byte, Short, Float or Any containing these types.
         */
    static public float toFloat(Object object) throws com.sun.star.lang.IllegalArgumentException{
        Float ret= (Float) convertSimple( TypeClass.FLOAT,null, object);
        return ret.floatValue();
    }

        /** Allowed argument types are Byte, Short, Integer, Float, Double or Any containing these types.
         */
    static public double toDouble(Object object) throws com.sun.star.lang.IllegalArgumentException {
        Double ret= (Double) convertSimple( TypeClass.DOUBLE, null, object);
        return ret.doubleValue();
    }

    static public String toString(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (String) convertSimple( TypeClass.STRING, null, object);
    }

    static public Type toType(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (Type) convertSimple( TypeClass.TYPE, null, object);
    }

    static public Object toObject(Type type, Object object)
        throws com.sun.star.lang.IllegalArgumentException{
        return convertSimple( TypeClass.ANY,  type, object);
    }

    static public Object toArray( Object object) throws com.sun.star.lang.IllegalArgumentException {
        return convertSimple( TypeClass.SEQUENCE, null, object);
    }

    /**
       Examines the argument |object| if is correspond to the type in argument |what|.
       |object| is either matched directly against the type or if it is an any then the
       contained object is matched against the type.
    */
    static private boolean containsType( TypeClass what, Object object){
        boolean retVal= false;
        Type _t= new Type( object.getClass());

        if( _t.getTypeClass() == TypeClass.ANY)
        {
            Any _any= (Any)object;
            if( _any.getType().getTypeClass().getValue() == what.getValue())
                retVal= true;
        }
        else if( _t.getTypeClass().getValue() == what.getValue())
            retVal= true;
        return retVal;
    }

    static private Object convertSimple( TypeClass destTClass, Type destType, Object src)
        throws com.sun.star.lang.IllegalArgumentException {

        Type srcType= new Type( src.getClass());
        Object _src= src;
        int srcTypeValue=0;
        // If |src| is an Any then we check if the  Any's Type matches the type
        // of the contained object and obtain the object which is processed further on.
        if( Any.class.isAssignableFrom( src.getClass()))
        {
            Any _any = (Any)src;
            _src= _any.getObject();
            srcType= new Type( _src.getClass());
            // If the Any's type in an interface then check if the Any's object member
            // implements interfaces
            if( _any.getType().getTypeClass() == TypeClass.INTERFACE)
            {
                if( _src.getClass().getInterfaces().length ==  0)
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "The argument does not implement interfaces");
                else
                    srcTypeValue= TypeClass.ANY.getValue();
            }
            else if( srcType.getTypeClass().getValue() != _any.getType().getTypeClass().getValue())
                // The Type of the object in the any must match the type in the any
                throw new com.sun.star.lang.IllegalArgumentException(
                    "The Type in the Any does not fit the Any's data");
            else
                srcTypeValue= srcType.getTypeClass().getValue();
        }
        else
        {
            // if we have an object
            if( destTClass == TypeClass.ANY)
            {
                if( _src.getClass().getInterfaces().length == 0)
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "The argument does not implement interfaces");
                else
                    srcTypeValue= TypeClass.ANY.getValue();
            }
            else
                srcTypeValue= srcType.getTypeClass().getValue();
        }

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
        case TypeClass.ANY_value:
            if( srcTypeValue == TypeClass.ANY_value)
                return UnoRuntime.queryInterface( destType, _src);
            break;
        case TypeClass.SEQUENCE_value:
            if( srcTypeValue == TypeClass.SEQUENCE_value)
                return _src;
            break;

        }

        throw new com.sun.star.lang.IllegalArgumentException(
            "The Argument did not hold the proper type");
    }
}
