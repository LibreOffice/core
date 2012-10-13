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

    public static final int UNKNOWN_TYPE = -32768;
    public static final int BYTE_TYPE = 0;
    public static final int SHORT_TYPE = 1;
    public static final int INT_TYPE = 2;
    public static final int LONG_TYPE = 3;
    public static final int FLOAT_TYPE = 4;
    public static final int DOUBLE_TYPE = 5;
    public static final int CHAR_TYPE = 6;
    public static final int STRING_TYPE = -1;
    public static final int BOOLEAN_TYPE = -2;
    public static final int ARRAY_TYPE = -3;
    public static final int SEQUENCE_TYPE = -4;
    public static final int ASCII_VALUE_0 = 48;
    public static final int ASCII_VALUE_A = 65;
    public static final int COUNT_CHARS_IN_ALPHABET = 26;
    private static final int HEX_BASE = 16;
    private static final int DEC_BASE = 10;
    private static final int ASCII_LETTER_A_OFFSET = 55;

    /**
     * private c'tor to prevent instantiation
     */
    private NumericalHelper()
    {
        // private c'tor, so noone can instantiate
    }

    /**
     * get the type of an object: returns all types that can possibly converted
     * with this class.
     * @param obj an object that is checked for conversion
     * @return the type of the object
     */
    public static int getType(Object obj)
    {
        try
        {
            TypeObject aTypeObject = getTypeObject(obj);
            return aTypeObject.iType;
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore this one; just return unknown type
        }
        return UNKNOWN_TYPE;
    }

    /**
     * get a byte value from the object
     * @param aValue 
     * @return a byte
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static byte toByte(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {

        byte retValue = 0;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = getByte(aTypeObject);
                break;
            case CHAR_TYPE:
                retValue = (byte) getChar(aTypeObject);
                break;
            case SHORT_TYPE:
                retValue = (byte) getShort(aTypeObject);
                break;
            case INT_TYPE:
                retValue = (byte) getInt(aTypeObject);
                break;
            case LONG_TYPE:
                retValue = (byte) getLong(aTypeObject);
                break;
            case FLOAT_TYPE:
                retValue = (byte) getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = (byte) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    Byte b = new Byte((String) aTypeObject.aValue);
                    retValue = b.byteValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to byte: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? (byte) -1 : (byte) 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get a char value from the object
     * @param aValue 
     * @return a char
     * @throws com.sun.star.lang.IllegalArgumentException  if the object cannot be converted
     */
    public static char toChar(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {

        char retValue = 0;
        boolean hasConversionWarning = false;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case CHAR_TYPE:
                retValue = getChar(aTypeObject);
                break;
            case BYTE_TYPE:
                retValue = (char) getByte(aTypeObject);
                break;
            case SHORT_TYPE:
                retValue = (char) getShort(aTypeObject);
                break;
            case INT_TYPE:
                retValue = (char) getInt(aTypeObject);
                break;
            case LONG_TYPE:
                retValue = (char) getLong(aTypeObject);
                break;
            case FLOAT_TYPE:
                retValue = (char) getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = (char) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    String s = (String) aTypeObject.aValue;
                    if (s.length() > 0)
                    {
                        retValue = s.charAt(0);
                    }
                    else
                    {
                        retValue = (char) 0;
                    }
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to char: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? (char) -1 : (char) 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get a short value from the object
     * @param aValue 
     * @return a short
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static short toShort(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        short retValue = 0;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = getByte(aTypeObject);
                break;
            case CHAR_TYPE:
                retValue = (byte) getChar(aTypeObject);
                break;
            case SHORT_TYPE:
                retValue = getShort(aTypeObject);
                break;
            case INT_TYPE:
                retValue = (short) getInt(aTypeObject);
                break;
            case LONG_TYPE:
                retValue = (short) getLong(aTypeObject);
                break;
            case FLOAT_TYPE:
                retValue = (short) getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = (short) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    Short s = new Short((String) aTypeObject.aValue);
                    retValue = s.shortValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to short: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? (short) -1 : (short) 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    public static boolean isValidAndNumerical(Object aValue) throws com.sun.star.lang.IllegalArgumentException
    {
        if (aValue != null)
        {
            if (!AnyConverter.isVoid(aValue))
            {
                return (NumericalHelper.isNumerical(aValue));
            }
        }
        return false;
    }

    public static boolean isValidAndBoolean(Object aValue) throws com.sun.star.lang.IllegalArgumentException
    {
        if (aValue != null)
        {
            if (!AnyConverter.isVoid(aValue))
            {
                int nType = AnyConverter.getType(aValue).getTypeClass().getValue();
                return (nType == TypeClass.BOOLEAN_value);
            }
        }
        return false;
    }

    public static boolean isValid(Object aValue)
    {
        if (aValue != null)
        {
            if (!AnyConverter.isVoid(aValue))
            {
                return true;
            }
        }
        return false;
    }

    /**
    @param aValue a object this can contain anything
    @return true, if the parameter aValue is type of real numbers
    @deprecate, use isRealNumber() instead.
     */
    public static boolean isNumerical(Object aValue)
    {
        try
        {
            TypeObject aTypeObject = getTypeObject(aValue);
            switch (aTypeObject.iType)
            {
                case BYTE_TYPE:
                case CHAR_TYPE:
                case SHORT_TYPE:
                case INT_TYPE:
                case LONG_TYPE:
                case DOUBLE_TYPE:
                case FLOAT_TYPE:
                    return true;
                default:
                    return false;
            }
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            return false;
        }
    }

    /**
    @param _aValue a object this can contain anything
    @return true, if the parameter aValue is type of real numbers
    
    see also http://en.wikipedia.org/wiki/Mathematics
     */
    public static boolean isRealNumber(Object _aValue)
    {
        return isNumerical(_aValue);
    }

    /**
    @param aValue a object this can contain anything
     * @return true, if the value is type of any integer values. double / float are not(!) integer values
     * @throws com.sun.star.lang.IllegalArgumentException 
     */
    public static boolean isInteger(Object aValue) throws com.sun.star.lang.IllegalArgumentException
    {
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
            case CHAR_TYPE:
            case SHORT_TYPE:
            case INT_TYPE:
            case LONG_TYPE:
                return true;
            default:
                return false;
        }
    }

    /**
     * Can a given object be converted to a String array?
     * @param aValue the object to test
     * @return true, if the object can be converted to a String array.
     */
    public static boolean isStringArray(Object aValue)
    {
        try
        {
            toStringArray(aValue);
            return true;
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore
        }
        return false;
    }

    /**
     * Can a given object be converted to an int array?
     * @param aValue the object to test
     * @return true, if the object can be converted to an Integer array.
     */
    public static boolean isIntegerArray(Object aValue)
    {
        try
        {
            toIntArray(aValue);
            return true;
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore
        }
        return false;
    }

    /**
     * get an int value from the object
     * @param aValue 
     * @return an int
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
                    Integer i = new Integer((String) aTypeObject.aValue);
                    retValue = i.intValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
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
     * get a long value from the object
     * @param aValue 
     * @return a long
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static long toLong(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        long retValue = 0;
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
                retValue = (long) getFloat(aTypeObject);
                break;
            case DOUBLE_TYPE:
                retValue = (long) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    Long l = new Long((String) aTypeObject.aValue);
                    retValue = l.longValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to short: " + aTypeObject.aValue);
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
     * get a float value from the object
     * @param aValue 
     * @return a float
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static float toFloat(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        float retValue = (float) 0.0;
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
                retValue = (float) getDouble(aTypeObject);
                break;
            case STRING_TYPE:
                try
                {
                    Float f = new Float((String) aTypeObject.aValue);
                    retValue = f.floatValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to short: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = getBool(aTypeObject) ? (float) -1 : (float) 0;
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get a double value from the object
     * @param aValue 
     * @return a double
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
                    Float f = new Float((String) aTypeObject.aValue);
                    retValue = f.floatValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
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
     * get a String value from the object
     * @param aValue 
     * @return a String
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static String toString(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        String retValue = null;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case CHAR_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case SHORT_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case INT_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case LONG_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case FLOAT_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case DOUBLE_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case STRING_TYPE:
                retValue = (String) aTypeObject.aValue;
                break;
            case BOOLEAN_TYPE:
                retValue = aTypeObject.aValue.toString();
                break;
            case ARRAY_TYPE:
                retValue = new String(toByteArray((aValue)));
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get a boolean value from the object
     * @param aValue 
     * @return a boolean
     * @throws com.sun.star.lang.IllegalArgumentException if the object cannot be converted
     */
    public static boolean toBoolean(Object aValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        boolean retValue = true;
        TypeObject aTypeObject = getTypeObject(aValue);
        switch (aTypeObject.iType)
        {
            case BYTE_TYPE:
                retValue = (((Byte) aTypeObject.aValue).byteValue() != 0);
                break;
            case CHAR_TYPE:
                retValue = (((Character) aTypeObject.aValue).charValue() != 0);
                break;
            case SHORT_TYPE:
                retValue = (((Short) aTypeObject.aValue).shortValue() != 0);
                break;
            case INT_TYPE:
                retValue = (((Integer) aTypeObject.aValue).intValue() != 0);
                break;
            case LONG_TYPE:
                retValue = (((Long) aTypeObject.aValue).longValue() != 0);
                break;
            case FLOAT_TYPE:
                retValue = (((Float) aTypeObject.aValue).floatValue() != 0);
                break;
            case DOUBLE_TYPE:
                retValue = (((Double) aTypeObject.aValue).doubleValue() != 0);
                break;
            case STRING_TYPE:
                try
                {
                    Boolean b = Boolean.valueOf((String) aTypeObject.aValue);
                    retValue = b.booleanValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    throw new com.sun.star.lang.IllegalArgumentException(
                            "Cannot convert to short: " + aTypeObject.aValue);
                }
                break;
            case BOOLEAN_TYPE:
                retValue = ((Boolean) aTypeObject.aValue).booleanValue();
                break;
            default:
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert this type: " + aValue.getClass().getName());
        }
        return retValue;
    }

    /**
     * get an int array from an object
     * @param anArrayValue a value that is constructed into an array
     * @return an integer array
     * @throws com.sun.star.lang.IllegalArgumentException 
     */
    public static int[] toIntArray(Object anArrayValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        int[] retValue = null;
        TypeObject aTypeObject = getTypeObject(anArrayValue);
        if (aTypeObject.iType == SEQUENCE_TYPE)
        {
            aTypeObject = convertSequenceToObjectArray(aTypeObject);
        }
        if (aTypeObject.iType == ARRAY_TYPE)
        {
            Object[] obj = (Object[]) aTypeObject.aValue;
            retValue = new int[obj.length];
            for (int i = 0; i < obj.length; i++)
            {
                retValue[i] = toInt(obj[i]);
            }
        }
        else
        { // object is not really an array
            retValue = new int[]
                    {
                        toInt(anArrayValue)
                    };
        }
        return retValue;
    }

    /**
     * get an byte array from an object
     * @param anArrayValue a value that is constructed into an array
     * @return a byte array
     * @throws com.sun.star.lang.IllegalArgumentException 
     */
    public static byte[] toByteArray(Object anArrayValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        byte[] retValue = null;
        TypeObject aTypeObject = getTypeObject(anArrayValue);
        if (aTypeObject.iType == SEQUENCE_TYPE)
        {
            aTypeObject = convertSequenceToObjectArray(aTypeObject);
        }
        if (aTypeObject.iType == ARRAY_TYPE)
        {
            Object[] obj = (Object[]) aTypeObject.aValue;
            retValue = new byte[obj.length];
            for (int i = 0; i < obj.length; i++)
            {
                retValue[i] = toByte(obj[i]);
            }
        }
        else
        { // object is not really an array
            retValue = new byte[]
                    {
                        toByte(anArrayValue)
                    };
        }
        return retValue;
    }

    /**
     * get a short array from an object
     * @param anArrayValue a value that is constructed into an array
     * @return a short array
     * @throws com.sun.star.lang.IllegalArgumentException 
     */
    public static short[] toShortArray(Object anArrayValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        short[] retValue = null;
        TypeObject aTypeObject = getTypeObject(anArrayValue);
        if (aTypeObject.iType == SEQUENCE_TYPE)
        {
            aTypeObject = convertSequenceToObjectArray(aTypeObject);
        }
        if (aTypeObject.iType == ARRAY_TYPE)
        {
            Object[] obj = (Object[]) aTypeObject.aValue;
            retValue = new short[obj.length];
            for (int i = 0; i < obj.length; i++)
            {
                retValue[i] = toShort(obj[i]);
            }
        }
        else
        { // object is not really an array
            retValue = new short[]
                    {
                        toShort(anArrayValue)
                    };
        }
        return retValue;
    }

    /**
     * get a string array from an object
     * @param anArrayValue a value that is constructed into an array
     * @return a short array
     * @throws com.sun.star.lang.IllegalArgumentException 
     */
    public static String[] toStringArray(Object anArrayValue)
            throws com.sun.star.lang.IllegalArgumentException
    {
        String[] retValue = null;
        TypeObject aTypeObject = getTypeObject(anArrayValue);
        if (aTypeObject.iType == SEQUENCE_TYPE)
        {
            aTypeObject = convertSequenceToObjectArray(aTypeObject);
        }
        if (aTypeObject.iType == ARRAY_TYPE)
        {
            Object[] obj = (Object[]) aTypeObject.aValue;
            retValue = new String[obj.length];
            for (int i = 0; i < obj.length; i++)
            {
                retValue[i] = toString(obj[i]);
            }
        }
        else
        { // object is not really an array
            retValue = new String[]
                    {
                        toString(anArrayValue)
                    };
        }
        return retValue;
    }

    /**
     * get an int from an object
     * @param _aValue a value that is constructed into an int
     * @param _ndefaultValue the value that is returned, if conversion fails, or if 'aValue' is null
     * @return an int value
     * @throws java.lang.Exception 
     */
    public static int toInt(Object _aValue, int _ndefaultValue) throws Exception
    {
        int nreturn = _ndefaultValue;
        try
        {
            if ((_aValue != null) && (!(AnyConverter.isVoid(_aValue))))
            {
                if (isInteger(_aValue))
                {
                    nreturn = toInt(_aValue);
                }
                else
                {
                    DebugHelper.exception(1/* BasicErrorCode.SbERR_CONVERSION*/, PropertyNames.EMPTY_STRING);
                }
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            DebugHelper.exception(1 /*BasicErrorCode.SbERR_METHOD_FAILED*/, PropertyNames.EMPTY_STRING);
        }
        return nreturn;
    }

    /**
     * get a long from an object
     * @param aValue a value that is constructed into a long
     * @param defaultValue the value that is returned, if conversion fails
     * @return a long value
     */
    public static long toLong(Object aValue, long defaultValue)
    {
        try
        {
            return toLong(aValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a float from an object
     * @param aValue a value that is constructed into a float
     * @param defaultValue the value that is returned, if conversion fails
     * @return a long value
     */
    public static float toFloat(Object aValue, float defaultValue)
    {
        try
        {
            return toFloat(aValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a double from an object
     * @param aValue a value that is constructed into a double
     * @param defaultValue the value that is returned, if conversion fails
     * @return a double value
     */
    public static double toDouble(Object aValue, double defaultValue)
    {
        try
        {
            return toDouble(aValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a string from an object
     * @param aValue a value that is constructed into a string
     * @param defaultValue the value that is returned, if conversion fails
     * @return a string value
     */
    public static String toString(Object aValue, String defaultValue)
    {
        try
        {
            return toString(aValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a boolean from an object
     * @param aValue a value that is constructed into a boolean
     * @param defaultValue the value that is returned, if conversion fails
     * @return a boolean value
     */
    public static boolean toBoolean(Object aValue, boolean defaultValue)
    {
        try
        {
            return toBoolean(aValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a int array from an object
     * @param anArrayValue  a value that is constructed into an int array
     * @param defaultValue the value that is returned, if conversion fails
     * @return an int array
     */
    public static int[] toIntArray(Object anArrayValue, int[] defaultValue)
    {
        try
        {
            return toIntArray(anArrayValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a short array from an object
     * @param anArrayValue a value that is constructed into a short array
     * @param defaultValue the value that is returned, if conversion fails
     * @return a short array
     */
    public static short[] toShortArray(Object anArrayValue, short[] defaultValue)
    {
        try
        {
            return toShortArray(anArrayValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a string array from an object
     * @param anArrayValue a value that is constructed into a string array
     * @param defaultValue the value that is returned, if conversion fails
     * @return a string array
     */
    public static String[] toStringArray(Object anArrayValue, String[] defaultValue)
    {
        try
        {
            return toStringArray(anArrayValue);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            // ignore exception
        }
        return defaultValue;
    }

    /**
     * get a hexadecimal representation from a number
     * @param number the number to transform
     * @return a String with the hex code of the number
     */
    public static String getHexStringFromNumber(long number)
    {
        TransformNumToHex num = new TransformNumToHex(number);
        return num.getResult();
    }

    /**
     * Get the roman equivalent to an arabic number, e.g. 17 -> XVII.
     * The allowed range for numbers goes from 1 to 3999. These can be
     * converted using ASCII letters (3999 -> MMMCMXCIX).
     * @param n the arabic number
     * @return the roman equivalent as string
     * @throws BasicErrorException if the number cannot be converted.
     */
//    public static String getRomanEquivalent(int n)
//            throws com.sun.star.script.BasicErrorException {
//        return RomanNumbering.getRomanEquivalent(n);
//    }
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
                aTypeObject.aValue = new Byte(AnyConverter.toByte(aValue));
                break;
            case TypeClass.SHORT_value:
                aTypeObject.iType = SHORT_TYPE;
                aTypeObject.aValue = new Short(AnyConverter.toShort(aValue));
                break;
            case TypeClass.LONG_value:
                aTypeObject.iType = INT_TYPE;
                aTypeObject.aValue = new Integer(AnyConverter.toInt(aValue));
                break;
            case TypeClass.HYPER_value:
                aTypeObject.iType = LONG_TYPE;
                aTypeObject.aValue = new Long(AnyConverter.toLong(aValue));
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
            case TypeClass.ARRAY_value:
                aTypeObject.iType = ARRAY_TYPE;
                aTypeObject.aValue = new Object[]
                        {
                            AnyConverter.toArray(aValue)
                        };
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
    static int getInt(TypeObject typeObject)
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
    static float getFloat(TypeObject typeObject)
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

    /**
     * simple class to construct a hexadecimal value from a long number
     */
    private static class TransformNumToHex
    {

        private StringBuffer val;

        public TransformNumToHex(long number)
        {
            val = new StringBuffer();
            transform(number);
        }

        private void transform(long number)
        {
            int index = (int) (number % HEX_BASE);
            number = number / HEX_BASE;
            if (index < DEC_BASE)
            {
                val.insert(0, index);
            }
            else
            {
                val.insert(0, (char) (ASCII_LETTER_A_OFFSET + index));
            }
            if (number > 0)
            {
                transform(number);
            }
        }

        public String getResult()
        {
            return val.toString();
        }
    }

    private static TypeObject convertSequenceToObjectArray(
            TypeObject sourceObject)
            throws com.sun.star.lang.IllegalArgumentException
    {
        TypeObject destObject = new TypeObject();
        Object array = sourceObject.aValue;
        destObject.iType = ARRAY_TYPE;
        Class<?> c = array.getClass();
        Object[] aShortVal = null;
        if (c.equals(byte[].class))
        {
            byte[] vals = (byte[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Byte(vals[i]);
            }
        }
        else if (c.equals(short[].class))
        {
            short[] vals = (short[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Short(vals[i]);
            }
        }
        else if (c.equals(int[].class))
        {
            int[] vals = (int[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Integer(vals[i]);
            }
        }
        else if (c.equals(long[].class))
        {
            long[] vals = (long[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Long(vals[i]);
            }
        }
        else if (c.equals(float[].class))
        {
            float[] vals = (float[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Float(vals[i]);
            }
        }
        else if (c.equals(double[].class))
        {
            double[] vals = (double[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = new Double(vals[i]);
            }
        }
        else if (c.equals(boolean[].class))
        {
            boolean[] vals = (boolean[]) array;
            aShortVal = new Object[vals.length];
            for (int i = 0; i < vals.length; i++)
            {
                aShortVal[i] = Boolean.valueOf(vals[i]);
            }
        }
        // if nothing did match, try this
        if (aShortVal == null)
        {
            try
            {
                aShortVal = (Object[]) array;
            }
            catch (java.lang.ClassCastException e)
            {
                // unknown type cannot be converted
                throw new com.sun.star.lang.IllegalArgumentException(
                        "Cannot convert unknown type: '" + e.getMessage() + "'");
            }
        }
        destObject.aValue = aShortVal;
        return destObject;
    }

    /**
     * Helper class for roman numbering
     */
    private static class RomanNumbering
    {

        /** the used roman lettesrs **/
        private static final String[] ROMAN_EQUIV = new String[]
        {
            "I", "V", "X", "L", "C", "D", "M"
        };
        /** max number that can be converted **/
        private static final int MAX_NUMBER = 3999;
        /** min number that can be converted **/
        private static final int MIN_NUMBER = 1;
        /** ASCII code for the number 0 **/
        private static final int ASCII_CODE_0 = 48;
        /** special number for the conversion algorithm **/
        private static final int FOUR = 4;
        /** special number for the conversion algorithm **/
        private static final int FIVE = 5;
        /** special number for the conversion algorithm **/
        private static final int NINE = 9;

        /**
         * Get the roman equivalent to an arabic number, e.g. 17 -> XVII.
         * The allowed range for numbers goes from 1 to 3999. These can be
         * converted using ASCII letters (3999 -> MMMCMXCIX).
         * @param n the arabic number
         * @return the roman equivalent as string
         * @throws com.sun.star.script.BasicErrorException if the number cannot be converted.
         */
        public static String getRomanEquivalent(int n)
                throws Exception
        {
            StringBuffer romanNumber = new StringBuffer();
            try
            {
                if (n > MAX_NUMBER || n < MIN_NUMBER)
                {
                    DebugHelper.exception(1 /*BasicErrorCode.SbERR_OUT_OF_RANGE*/, PropertyNames.EMPTY_STRING);
                }
                String number = NumericalHelper.toString(new Integer(n));
                /* converison idea: every digit is written with a maximum of two
                 * different roman symbols, using three in total, e.g. CC, CD, 
                 * DCC, CM for the hundreds (meaning 200, 400, 700 and 900).
                 * So every digit is converted seperately with regard to the
                 * special cases 4 and 9.
                 */
                int symbolIndex = 0;
                for (int i = number.length() - 1; i >= 0; i--)
                {
                    StringBuffer romanDigit = new StringBuffer();
                    int b = number.charAt(i) - ASCII_CODE_0;
                    if (b == FOUR)
                    { // special case IV
                        romanDigit.append(ROMAN_EQUIV[symbolIndex]);
                        romanDigit.append(ROMAN_EQUIV[symbolIndex + 1]);
                    }
                    else if (b == NINE)
                    { // special case IX
                        romanDigit.append(ROMAN_EQUIV[symbolIndex]);
                        romanDigit.append(ROMAN_EQUIV[symbolIndex + 2]);
                    }
                    else
                    {
                        if (b >= FIVE)
                        { // special case V
                            romanDigit.append(ROMAN_EQUIV[symbolIndex + 1]);
                            b = b - FIVE;
                        }
                        for (int j = 0; j < b; j++)
                        { // append I's
                            romanDigit.append(ROMAN_EQUIV[symbolIndex]);
                        }
                    }
                    // next group of three symbols
                    symbolIndex += 2;
                    // append in reverse: we are starting at the right
                    romanNumber.append(romanDigit.reverse());
                }
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.exception(e);
            }
            // reverse again to get the number
            return romanNumber.reverse().toString();
        }
    }

    public static boolean representsIntegerNumber(double _dblvalue)
    {
        double dblsecvalue = ((int) _dblvalue);
        return Double.compare(_dblvalue, dblsecvalue) == 0;
    }

    public static double roundDouble(Double _Dblvalue, int _ndecimals)
    {
        return roundDouble(_Dblvalue.doubleValue(), _ndecimals);
    }

    public static double roundDouble(double _dblvalue, int _ndecimals)
    {
        double dblfactor = java.lang.Math.pow(10.0, _ndecimals);
        return ((int) (_dblvalue * dblfactor)) / dblfactor;
    }
}
