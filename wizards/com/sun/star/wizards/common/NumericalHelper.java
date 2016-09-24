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
package com.sun.star.wizards.common;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.TypeClass;

/**
 * A class for helping with all kinds of numerical conversions.
 * Optional or named parameters in SO are of the Object type in Java.
 * These objects must be converted to the right simple value type.
 * Unfortunately, StarBasic does not know the original desired type,
 * and a value that should be a "Float" is delivered as "Byte". This class
 * handles the conversions of these types.<br>
 * This class does not log warnings (or throws Exceptions) when the precision
 * of a value is lost.
 */
public class NumericalHelper
{

    private static final int BYTE_TYPE = 0;
    private static final int SHORT_TYPE = 1;
    private static final int INT_TYPE = 2;
    private static final int LONG_TYPE = 3;
    private static final int FLOAT_TYPE = 4;
    private static final int DOUBLE_TYPE = 5;
    private static final int CHAR_TYPE = 6;
    private static final int STRING_TYPE = -1;
    private static final int BOOLEAN_TYPE = -2;
    private static final int SEQUENCE_TYPE = -3;

    /**
     * private c'tor to prevent instantiation
     */
    private NumericalHelper()
    {
        // private c'tor, so no one can instantiate
    }



    /**
     * get an int value from the object
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static int toInt(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        int retValue = 0;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = getByte(aTypeObject);
                break;
            case CHAR_TYPE:
                retValue = getChar(aTypeObject);
                break;
            case SHORT_TYPE:
                retValue = getShort(aTypeObject);
                break;
            case INT_TYPE:
                retValue = getInt(aTypeObject);
                break;
            case LONG_TYPE:
                retValue = (int) getLong(aTypeObject);
                break;
            case FLOAT_TYPE:
                retValue = (int) getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = (int) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    retValue = Integer.parseInt((String) aTypeObject.aValue);
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(e,
                            "Cannot convert to int: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? -1 : 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get a double value from the object
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static double toDouble(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        double retValue = 0.0;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = getByte(aTypeObject);
                break;
            case CHAR_TYPE:
                retValue = getChar(aTypeObject);
                break;
            case SHORT_TYPE:
                retValue = getShort(aTypeObject);
                break;
            case INT_TYPE:
                retValue = getInt(aTypeObject);
                break;
            case LONG_TYPE:
                retValue = getLong(aTypeObject);
                break;
            case FLOAT_TYPE:
                retValue = getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    retValue = Float.parseFloat((String) aTypeObject.aValue);
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(e,
                            "Cannot convert to short: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? (double) -1 : (double) 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get the type object from the given object
     * @param aValue an object representing a (numerical) value; can also be an 'any'
     * @return a type object: the object together with the its type information
     * @throws com.sun.star.lang.IllegalArgumentException if the object is unknown
     */
    private static TypeObject getTypeObject(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        TypeObject aTypeObject = new TypeObject();
        if (aValue == null || AnyConverter.isVoid(aValue))
        {
            throw new com.sun.star.lang.IllegalArgumentException("Cannot convert a null object.");
        }
        int type = AnyConverter.getType(aValue).getTypeClass().getValue();
        switch (type)
        {
            case TypeClass.CHAR_value:
                aTypeObject.iType = CHAR_TYPE;
                aTypeObject.aValue = new Character(AnyConverter.toChar(aValue));
                break;
            case TypeClass.BYTE_value:
                aTypeObject.iType = BYTE_TYPE;
                aTypeObject.aValue = Byte.valueOf(AnyConverter.toByte(aValue));
                break;
            case TypeClass.SHORT_value:
                aTypeObject.iType = SHORT_TYPE;
                aTypeObject.aValue = Short.valueOf(AnyConverter.toShort(aValue));
                break;
            case TypeClass.LONG_value:
                aTypeObject.iType = INT_TYPE;
                aTypeObject.aValue = Integer.valueOf(AnyConverter.toInt(aValue));
                break;
            case TypeClass.HYPER_value:
                aTypeObject.iType = LONG_TYPE;
                aTypeObject.aValue = Long.valueOf(AnyConverter.toLong(aValue));
                break;
            case TypeClass.FLOAT_value:
                aTypeObject.iType = FLOAT_TYPE;
                aTypeObject.aValue = new Float(AnyConverter.toFloat(aValue));
                break;
            case TypeClass.DOUBLE_value:
                aTypeObject.iType = DOUBLE_TYPE;
                aTypeObject.aValue = new Double(AnyConverter.toDouble(aValue));
                break;
            case TypeClass.STRING_value:
                aTypeObject.iType = STRING_TYPE;
                aTypeObject.aValue = AnyConverter.toString(aValue);
                break;
            case TypeClass.BOOLEAN_value:
                aTypeObject.iType = BOOLEAN_TYPE;
                aTypeObject.aValue = Boolean.valueOf(AnyConverter.toBoolean(aValue));
                break;
            case TypeClass.SEQUENCE_value:
                aTypeObject.iType = SEQUENCE_TYPE;
                aTypeObject.aValue = aValue;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return aTypeObject;
    }

    /**
     * get the simple byte type
     */
    private static byte getByte(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != BYTE_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a byte type.");
        }
        return ((Byte) typeObject.aValue).byteValue();
    }

    /**
     * get the simple char type
     */
    private static char getChar(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != CHAR_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a char type.");
        }
        return ((Character) typeObject.aValue).charValue();
    }

    /**
     * get the simple short type
     */
    private static short getShort(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != SHORT_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a short type.");
        }
        return ((Short) typeObject.aValue).shortValue();
    }

    /**
     * get the simple int type
     * @param typeObject
     * @return
     * @throws com.sun.star.lang.IllegalArgumentException
     */
    private static int getInt(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != INT_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not an int type.");
        }
        return ((Integer) typeObject.aValue).intValue();
    }

    /**
     * get the simple float type
     * @throws com.sun.star.lang.IllegalArgumentException
     */
    private static float getFloat(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != FLOAT_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a float type.");
        }
        return ((Float) typeObject.aValue).floatValue();
    }

    /**
     * get the simple double type
     */
    private static double getDouble(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != DOUBLE_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a double type.");
        }
        return ((Double) typeObject.aValue).doubleValue();
    }

    /**
     * get the simple long type
     */
    private static long getLong(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != LONG_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a long type.");
        }
        return ((Long) typeObject.aValue).longValue();
    }

    /**
     * get the simple boolean type
     */
    private static boolean getBool(TypeObject typeObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        if (typeObject.iType != BOOLEAN_TYPE)
        {
            throw new com.sun.star.lang.IllegalArgumentException(
                    "Given argument is not a boolean type.");
        }
        return ((Boolean) typeObject.aValue).booleanValue();
    }

    /**
     * a class to contain a type and a value for easier conversions
     */
    private static class TypeObject
    {

        public int iType;
        public Object aValue;
    }

}
