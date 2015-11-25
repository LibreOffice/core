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

package com.sun.star.uno;

/**
 * This class provides static methods which aim at exploring the contents of an
 * Any and extracting its value.
 *
 * <p>All public methods take an Object argument that either is the immediate object,
 * such as Boolean, Type, interface implementation, or an Any that contains an
 * object.</p>
 *
 * <p>The methods which extract the value do a widening conversion. See the
 * method comments for the respective conversions.</p>
 */
public class AnyConverter
{
    /**
     * Determines the type of an any object.
     *
     * @param object any object.
     * @return type object.
     */
    public static Type getType( Object object )
    {
        Type t;
        if (null == object)
        {
            t = m_XInterface_type;
        }
        else if (object instanceof Any)
        {
            t = ((Any)object).getType();
            // nested any
            if (TypeClass.ANY_value == t.getTypeClass().getValue())
                return getType( ((Any)object).getObject() );
        }
        else
        {
            t = new Type( object.getClass() );
        }
        return t;
    }

    /**
     * Checks if the any contains the idl type <code>void</code>.
     *
     * @param object the object to check.
     * @return true when the any is void, false otherwise.
     */
    public static boolean isVoid(Object object){
        return containsType(TypeClass.VOID, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>char</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a char, false otherwise.
     */
    public static boolean isChar(Object object){
        return containsType(TypeClass.CHAR, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>boolean</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a boolean, false otherwise.
     */
    public static boolean isBoolean(Object object){
        return containsType(TypeClass.BOOLEAN, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>byte</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a byte, false otherwise.
     */
    public static boolean isByte(Object object){
        return containsType(TypeClass.BYTE, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>short</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a short, false otherwise.
     */
    public static boolean isShort(Object object){
        return containsType(TypeClass.SHORT, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>long</code>
     * (which maps to a java-int).
     *
     * @param object the object to check.
     * @return true when the any contains a int, false otherwise.
     */
    public static boolean isInt(Object object){
        return containsType(TypeClass.LONG, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>hyper</code>
     * (which maps to a java-long).
     *
     * @param object the object to check.
     * @return true when the any contains a long, false otherwise.
     */
    public static boolean isLong(Object object){
        return containsType(TypeClass.HYPER, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>float</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a float, false otherwise.
     */
    public static boolean isFloat(Object object){
        return containsType(TypeClass.FLOAT, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>double</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a double, false otherwise.
     */
    public static boolean isDouble(Object object){
        return containsType(TypeClass.DOUBLE, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>string</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a string, false otherwise.
     */
    public static boolean isString(Object object){
        return containsType(TypeClass.STRING, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>enum</code>.
     *
     * @param object the object to check.
     * @return true if the any contains an enum, false otherwise.
     */
    public static boolean isEnum(Object object)
    {
        return containsType(TypeClass.ENUM, object);
    }

    /**
     * Checks if the any contains a value of the idl type <code>type</code>.
     *
     * @param object the object to check.
     * @return true when the any contains a type, false otherwise.
     */
    public static boolean isType(Object object){
        return containsType(TypeClass.TYPE, object);
    }

    /**
     * Checks if the any contains an interface, struct, exception, sequence or enum.
     *
     * <p>If <em>object</em> is an any with an interface type, then true is also
     * returned if the any contains a null reference. This is because interfaces
     * are allowed to have a null value contrary to other UNO types.</p>
     *
     * @param object the object to check.
     * @return true if the any contains an object.
     */
    public static boolean isObject(Object object)
    {
        int tc = getType(object).getTypeClass().getValue();
        return (TypeClass.INTERFACE_value == tc ||
                TypeClass.STRUCT_value == tc ||
                TypeClass.EXCEPTION_value == tc ||
                TypeClass.SEQUENCE_value == tc ||
                TypeClass.ENUM_value == tc);
    }

    /**
     * Checks if the any contains UNO idl sequence value (meaning a java array
     * containing elements which are values of UNO idl types).
     *
     * @param object the object to check.
     * @return true when the any contains an object which implements interfaces,
     * false otherwise.
     */
    public static boolean isArray(Object object){
        return containsType(TypeClass.SEQUENCE, object);
    }

    /**
     * Converts an Char object or an Any object containing a Char object into a
     * simple char.
     *
     * @param object the object to convert.
     * @return the char contained within the object.
     * @throws com.sun.star.lang.IllegalArgumentException in case no char is
     * contained within object.
     *
     * @see #isChar
     */
    public static char toChar(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Character ret= (Character)convertSimple(TypeClass.CHAR, null, object);
        return ret.charValue();
    }

    /**
     * Converts an Boolean object or an Any object containing a Boolean object
     * into a simple boolean.
     *
     * @param object the object to convert.
     * @return the boolean contained within the object
     * @throws com.sun.star.lang.IllegalArgumentException in case no boolean is
     * contained within object
     *
     * @see #isBoolean
    */
    public static boolean toBoolean(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Boolean ret= (Boolean)convertSimple(TypeClass.BOOLEAN, null, object);
        return ret.booleanValue();
    }

    /**
     * Converts an Byte object or an Any object containing a Byte object into a
     * simple byte.
     *
     * @param object the object to convert.
     * @return the boolean contained within the object.
     * @throws com.sun.star.lang.IllegalArgumentException in case no byte is
     * contained within object.
     *
     * @see #isBoolean
     */
    public static byte toByte(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Byte ret= (Byte)convertSimple(TypeClass.BYTE, null, object);
        return ret.byteValue();
    }

    /**
     * Converts a number object into a simple short and allows widening conversions.
     *
     * <p>Allowed argument types are Byte, Short or Any containing these types.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no short or
     * byte is contained within object.
     *
     * @return the short contained within the object.
     */
    public static short toShort(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Short ret= (Short)convertSimple(TypeClass.SHORT, null, object);
        return ret.shortValue();
    }
    /**
     * Converts a number object into an idl unsigned short and allows widening
     * conversions.
     *
     * <p>Allowed argument types are Anies containing idl unsigned short values.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no idl unsigned
     * short is contained within Any.
     *
     * @return an (unsigned) short.
     */
    public static short toUnsignedShort(Object object)
        throws com.sun.star.lang.IllegalArgumentException
    {
        Short ret= (Short)convertSimple(TypeClass.UNSIGNED_SHORT, null, object);
        return ret.shortValue();
    }

    /**
     * Converts a number object into a simple int and allows widening conversions.
     *
     * <p>Allowed argument types are Byte, Short, Integer or Any containing these
     * types.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no short, byte
     * or int is contained within object.
     *
     * @return the int contained within the object.
     */
    public static int toInt(Object object) throws  com.sun.star.lang.IllegalArgumentException{
        Integer ret= (Integer) convertSimple( TypeClass.LONG, null, object);
        return ret.intValue();
    }
    /**
     * Converts a number object into an idl unsigned long and allows widening
     * conversions.
     *
     * <p>Allowed argument types are Anies containing idl unsigned short or
     * unsigned long values.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no idl unsigned
     * short nor unsigned long is contained within Any.
     *
     * @return an (unsigned) int.
     */
    public static int toUnsignedInt(Object object)
        throws  com.sun.star.lang.IllegalArgumentException
    {
        Integer ret = (Integer)convertSimple(TypeClass.UNSIGNED_LONG, null, object);
        return ret.intValue();
    }

    /**
     * Converts a number object into a simple long and allows widening conversions.
     *
     * <p>Allowed argument types are Byte, Short, Integer, Long or Any containing
     * these types.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no short, byte,
     * int or long is contained within object.
     *
     * @return the long contained within the object.
     */
    public static long toLong(Object object) throws   com.sun.star.lang.IllegalArgumentException{
        Long ret= (Long) convertSimple( TypeClass.HYPER, null, object);
        return ret.longValue();
    }
    /**
     * Converts a number object into an idl unsigned hyper and allows widening
     * conversions.
     *
     * <p>Allowed argument types are Anies containing idl unsigned short, unsigned
     * long or unsigned hyper values.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no idl unsigned
     * short, nor unsigned long nor unsigned hyper is contained within object.
     *
     * @return an (unsigned) long.
     */
    public static long toUnsignedLong(Object object)
        throws com.sun.star.lang.IllegalArgumentException
    {
        Long ret = (Long)convertSimple(TypeClass.UNSIGNED_HYPER, null, object);
        return ret.longValue();
    }

    /**
     * Converts a number object into a simple float and allows widening conversions.
     *
     * <p>Allowed argument types are Byte, Short, Float or Any containing these
     * types.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no byte, short
     * or float is contained within object.
     *
     * @return the float contained within the object.
     */
    public static float toFloat(Object object) throws com.sun.star.lang.IllegalArgumentException{
        Float ret= (Float) convertSimple( TypeClass.FLOAT,null, object);
        return ret.floatValue();
    }

    /**
     * Converts a number object into a simple double and allows widening conversions.
     *
     * <p>Allowed argument types are Byte, Short, Int, Float, Double or Any
     * containing these types.</p>
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no byte, short,
     * int, float or double is contained within object.
     *
     * @return the double contained within the object.
     */
    public static double toDouble(Object object) throws com.sun.star.lang.IllegalArgumentException {
        Double ret= (Double) convertSimple( TypeClass.DOUBLE, null, object);
        return ret.doubleValue();
    }

    /**
     * Converts a string or an any containing a string into a string.
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no string is
     * contained within object.
     *
     * @return the string contained within the object.
     */
    public static String toString(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (String) convertSimple( TypeClass.STRING, null, object);
    }

    /**
     * Converts a Type or an any containing a Type into a Type.
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no type is
     * contained within object.
     *
     * @return the type contained within the object.
     */
    public static Type toType(Object object) throws com.sun.star.lang.IllegalArgumentException {
        return (Type) convertSimple( TypeClass.TYPE, null, object);
    }

    /**
     * Converts a UNO object (struct, exception, sequence, enum or interface) or
     * an Any containing these types into an UNO object of a specified destination
     * type.
     *
     * <p> For interfaces, the argument <em>object</em> is queried for the interface
     * specified by the <em>type</em> argument.</p>
     *
     * <p>That query (UnoRuntime.queryInterface) might return null, if the interface
     * is not implemented or a null-ref or a VOID any is given.</p>
     *
     * @param type type of the returned value.
     * @param object the object that is to be converted.
     * @throws com.sun.star.lang.IllegalArgumentException in case conversion is
     * not possible.
     *
     * @return destination object.
     */
    public static Object toObject(Type type, Object object)
        throws com.sun.star.lang.IllegalArgumentException
    {
        return convertSimple( type.getTypeClass(), type, object );
    }
    /**
     * Converts a UNO object (struct, exception, sequence, enum or interface) or
     * an Any containing these types into an UNO object of a specified destination
     * type.
     *
     * <p>For interfaces, the argument <em>object</em> is queried for the interface
     * specified by the <em>type</em> argument. That query (UnoRuntime.queryInterface)
     * might return null, if the interface is not implemented or a null-ref or a
     * VOID any is given.</p>
     *
     * @param clazz class of the returned value.
     * @param object the object that is to be converted.
     * @throws com.sun.star.lang.IllegalArgumentException in case conversion is
     * not possible.
     *
     * @return destination object.
     */
    public static Object toObject(Class<?> clazz, Object object)
        throws com.sun.star.lang.IllegalArgumentException
    {
        return toObject( new Type( clazz ), object );
    }

    /**
     * Converts an array or an any containing an array into an array.
     *
     * @param object the object to convert.
     * @throws com.sun.star.lang.IllegalArgumentException in case no array is
     * contained within object.
     *
     * @return the array contained within the object.
     */
    public static Object toArray( Object object) throws com.sun.star.lang.IllegalArgumentException {
        return convertSimple( TypeClass.SEQUENCE, null, object);
    }

    /**
     * Examines the argument <em>object</em> if is correspond to the type in
     * argument <em>what</em>.
     *
     * <p><em>object</em> is either matched directly against the type or if it is
     * an any then the contained object is matched against the type.</p>
     */
    private static boolean containsType( TypeClass what, Object object){
        return (getType(object).getTypeClass().getValue() == what.getValue());
    }

    private static final Type m_XInterface_type = new Type( XInterface.class );

    private static Object convertSimple( TypeClass destTClass, Type destType, Object object_ )
        throws com.sun.star.lang.IllegalArgumentException
    {
        Object object;
        Type type;
        if (object_ instanceof Any) {
            // unbox
            Any a = (Any)object_;
            object = a.getObject();
            type = a.getType();
            // nested any
            if (TypeClass.ANY_value == type.getTypeClass().getValue())
                return convertSimple( destTClass, destType, object );
        } else {
            object = object_;
            type = (null == object ? m_XInterface_type : new Type( object.getClass() ));
        }

        int tc = type.getTypeClass().getValue();
        int dest_tc = destTClass.getValue();

        if (null == object) {
            // special for interfaces
            if (TypeClass.INTERFACE_value == tc && dest_tc == tc)
                return null;
        } else {
            switch (dest_tc) {
            case TypeClass.CHAR_value:
                if (tc == TypeClass.CHAR_value)
                    return object;
                break;
            case TypeClass.BOOLEAN_value:
                if (tc == TypeClass.BOOLEAN_value)
                    return object;
                break;
            case TypeClass.BYTE_value:
                if (tc == TypeClass.BYTE_value)
                    return object;
                break;
            case TypeClass.SHORT_value:
                switch (tc) {
                case TypeClass.BYTE_value:
                    return Short.valueOf( ((Byte)object).byteValue() );
                case TypeClass.SHORT_value:
                    return object;
                }
                break;
            case TypeClass.UNSIGNED_SHORT_value:
                switch (tc) {
                case TypeClass.UNSIGNED_SHORT_value:
                    return object;
                }
                break;
            case TypeClass.LONG_value:
                switch (tc) {
                case TypeClass.BYTE_value:
                    return Integer.valueOf( ((Byte)object).byteValue() );
                case TypeClass.SHORT_value:
                case TypeClass.UNSIGNED_SHORT_value:
                    return Integer.valueOf( ((Short)object).shortValue() );
                case TypeClass.LONG_value:
                    return object;
                }
                break;
            case TypeClass.UNSIGNED_LONG_value:
                switch (tc) {
                case TypeClass.UNSIGNED_SHORT_value:
                    return Integer.valueOf( ((Short)object).shortValue() );
                case TypeClass.UNSIGNED_LONG_value:
                    return object;
                }
                break;
            case TypeClass.HYPER_value:
                switch (tc) {
                case TypeClass.BYTE_value:
                    return Long.valueOf( ((Byte)object).byteValue() );
                case TypeClass.SHORT_value:
                case TypeClass.UNSIGNED_SHORT_value:
                    return Long.valueOf( ((Short)object).shortValue() );
                case TypeClass.LONG_value:
                case TypeClass.UNSIGNED_LONG_value:
                    return Long.valueOf( ((Integer)object).intValue() );
                case TypeClass.HYPER_value:
                    return object;
                }
                break;
            case TypeClass.UNSIGNED_HYPER_value:
                switch (tc) {
                case TypeClass.UNSIGNED_SHORT_value:
                    return Long.valueOf( ((Short)object).shortValue() );
                case TypeClass.UNSIGNED_LONG_value:
                    return Long.valueOf( ((Integer)object).intValue() );
                case TypeClass.UNSIGNED_HYPER_value:
                    return object;
                }
                break;
            case TypeClass.FLOAT_value:
                switch (tc) {
                case TypeClass.BYTE_value:
                    return new Float( ((Byte)object).byteValue() );
                case TypeClass.SHORT_value:
                    return new Float( ((Short)object).shortValue() );
                case TypeClass.FLOAT_value:
                    return object;
                }
                break;
            case TypeClass.DOUBLE_value:
                switch (tc) {
                case TypeClass.BYTE_value:
                    return new Double( ((Byte)object).byteValue() );
                case TypeClass.SHORT_value:
                    return new Double( ((Short)object).shortValue() );
                case TypeClass.LONG_value:
                    return new Double( ((Integer)object).intValue() );
                case TypeClass.FLOAT_value:
                    return new Double( ((Float)object).floatValue() );
                case TypeClass.DOUBLE_value:
                    return object;
                }
                break;
            case TypeClass.ENUM_value:
                if (tc == TypeClass.ENUM_value &&
                    (null == destType || destType.equals( type ) /* optional destType */))
                {
                    return object;
                }
                break;
            case TypeClass.STRING_value:
                if (tc == TypeClass.STRING_value)
                    return object;
                break;
            case TypeClass.TYPE_value:
                if (tc == TypeClass.TYPE_value)
                    return object;
                break;
            case TypeClass.INTERFACE_value:
                // Because object is a class, not an interface, it is
                // controversial what kind of Type "new Type(object.class)"
                // above should return (UNKNOWN or INTERFACE), so that we should
                // not check here for "tc == TypeClass.INTERFACE_value".
                // Instead, we check whether object (indirectly) derives from
                // XInterface:
                if (object instanceof XInterface)
                    return UnoRuntime.queryInterface( destType, object );
                break;
            case TypeClass.STRUCT_value:
            case TypeClass.EXCEPTION_value:
                if (destType.isSupertypeOf(type)) {
                    return object;
                }
                break;
            case TypeClass.SEQUENCE_value:
                if (tc == TypeClass.SEQUENCE_value &&
                    (null == destType || destType.equals( type ) /* optional destType */))
                {
                    return object;
                }
                break;
            }
        }
        throw new com.sun.star.lang.IllegalArgumentException(
            "The Argument did not hold the proper type");
    }
}
