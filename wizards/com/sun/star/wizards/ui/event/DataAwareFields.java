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
package com.sun.star.wizards.ui.event;

import java.lang.reflect.Field;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.uno.Any;

/**
 * This class is a factory for Value objects for different types of
 * memebers. 
 * Other than some Value implementations classes this class contains static
 * type conversion methods and factory methods.
 * 
 * @see com.sun.star.wizards.ui.event.DataAware.Value
 */
public class DataAwareFields
{

    private static final String TRUE = "true";
    private static final String FALSE = "false";

    /**
     * returns a Value Object which sets and gets values
     * and converting them to other types, according to the "value" argument.
     * 
     * @param owner
     * @param fieldname
     * @param value
     * @return
     */
    public static DataAware.Value getFieldValueFor(Object owner, String fieldname, Object value)
    {
        try
        {
            Field f = owner.getClass().getField(fieldname);

            Class<?> c = f.getType();
            Class<?> c2 = value.getClass();
            if (c.equals(Boolean.TYPE))
            {
                return new BooleanFieldValue(f, c2);
            }
            else if (c.equals(Integer.TYPE))
            {
                return new IntFieldValue(f, c2);
            }
            else if (c.equals(Double.TYPE))
            {
                return new DoubleFieldValue(f, c2);
            }
            else if (c.equals(String.class) && c2.equals(Integer.class))
            {
                return new ConvertedStringValue(f, c2);
            }
            else
            {
                return new SimpleFieldValue(f);
            }
        }
        catch (NoSuchFieldException ex)
        {
            ex.printStackTrace();
            return null;
        }
    }

    /**
     * an abstract implementation of DataAware.Value to access
     * object memebers (fields) usign reflection.
     */
    private static abstract class FieldValue implements DataAware.Value
    {

        Field field;

        public FieldValue(Field field_)
        {
            field = field_;
        }

        public boolean isAssignable(Class<?> type)
        {
            return field.getDeclaringClass().isAssignableFrom(type);
        }
    }

    private static class BooleanFieldValue extends FieldValue
    {

        private Class<?> convertTo;

        public BooleanFieldValue(Field f, Class<?> convertTo_)
        {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target)
        {
            try
            {
                boolean b = field.getBoolean(target);
                if (convertTo.equals(Boolean.class))
                {
                    return b ? Boolean.TRUE : Boolean.FALSE;
                }
                else if (Number.class.isAssignableFrom(convertTo))
                {
                    return toNumber(b ? 1 : 0, convertTo);
                }
                else if (convertTo.equals(String.class))
                {
                    return String.valueOf(b);
                }
                else if (convertTo.isArray())
                {
                    return toShortArray(toInt(b));
                }
                else
                {
                    throw new IllegalArgumentException("Cannot convert boolean value to given type (" + convertTo.getName() + ").");
                }
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target)
        {
            try
            {
                field.setBoolean(target, toBoolean(value));
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static class IntFieldValue extends FieldValue
    {

        private Class<?> convertTo;

        public IntFieldValue(Field f, Class<?> convertTo_)
        {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target)
        {
            try
            {
                int i = field.getInt(target);
                if (convertTo.equals(Boolean.class))
                {
                    return i != 0 ? Boolean.TRUE : Boolean.FALSE;
                }
                else if (Number.class.isAssignableFrom(convertTo))
                {
                    return toNumber(i, convertTo);
                }
                else if (convertTo.equals(String.class))
                {
                    return String.valueOf(i);
                }
                else if (convertTo.isArray())
                {
                    return toShortArray(i);
                }
                else
                {
                    throw new IllegalArgumentException("Cannot convert int value to given type (" + convertTo.getName() + ").");
                }
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target)
        {
            try
            {
                field.setInt(target, (int) toDouble(value));
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static class DoubleFieldValue extends FieldValue
    {

        private Class<?> convertTo;

        public DoubleFieldValue(Field f, Class<?> convertTo_)
        {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target)
        {
            try
            {
                double d = field.getDouble(target);
                if (convertTo.equals(Boolean.class))
                {
                    return d != 0 ? Boolean.TRUE : Boolean.FALSE;
                }
                else if (Number.class.isAssignableFrom(convertTo))
                {
                    return toNumber(d, convertTo);
                }
                else if (convertTo.equals(String.class))
                {
                    return String.valueOf(d);
                }
                else if (convertTo.isArray())
                {
                    return toShortArray(d);
                }
                else
                {
                    throw new IllegalArgumentException("Cannot convert int value to given type (" + convertTo.getName() + ").");
                }
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target)
        {
            try
            {
                field.setDouble(target, toDouble(value));
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static class ConvertedStringValue extends FieldValue
    {

        private Class<?> convertTo;

        public ConvertedStringValue(Field f, Class<?> convertTo_)
        {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target)
        {
            try
            {
                String s = (String) field.get(target);

                if (convertTo.equals(Boolean.class))
                {
                    return (s != null && !s.equals(PropertyNames.EMPTY_STRING) && s.equals("true")) ? Boolean.TRUE : Boolean.FALSE;
                }
                else if (convertTo.equals(Integer.class))
                {
                    if (s == null || s.equals(PropertyNames.EMPTY_STRING))
                    {
                        return Any.VOID;
                    }
                    else
                    {
                        return new Integer(s);
                    }
                }
                else if (convertTo.equals(Double.class))
                {
                    if (s == null || s.equals(PropertyNames.EMPTY_STRING))
                    {
                        return Any.VOID;
                    }
                    else
                    {
                        return new Double(s);
                    }
                }
                else
                {
                    throw new IllegalArgumentException("Cannot convert int value to given type (" + convertTo.getName() + ").");
                }
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target)
        {
            try
            {
                field.set(target, value == null || (value.equals(Any.VOID)) ? PropertyNames.EMPTY_STRING : value.toString());
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static class SimpleFieldValue extends FieldValue
    {

        public SimpleFieldValue(Field f)
        {
            super(f);
        }
        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */

        public Object get(Object target)
        {
            try
            {
                if (target == null)
                {
                    if (field.getType().equals(String.class))
                    {
                        return PropertyNames.EMPTY_STRING;
                    }
                    if (field.getType().equals(Short.class))
                    {
                        return new Short((short) 0);
                    }
                    if (field.getType().equals(Integer.class))
                    {
                        return 0;
                    }
                    if (field.getType().equals(short[].class))
                    {
                        return new short[0];
                    }
                    else
                    {
                        return null;
                    }
                }
                else
                {
                    return field.get(target);
                }
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target)
        {
            try
            {
                field.set(target, value);
            }
            catch (IllegalAccessException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static double toDouble(Object value)
    {
        if (value instanceof Boolean)
        {
            return ((Boolean) value).booleanValue() ? 1 : 0;
        }
        else if (value instanceof Number)
        {
            return ((Number) value).doubleValue();
        }
        else if (value instanceof String)
        {
            return Double.valueOf((String) value).doubleValue();
        }
        else if (value instanceof short[])
        {
            return ((short[]) value).length == 0 ? 0 : ((short[]) value)[0];
        }
        else
        {
            throw new IllegalArgumentException("Can't convert value to double." + value.getClass().getName());
        }
    }

    private static boolean toBoolean(Object value)
    {
        if (value instanceof Boolean)
        {
            return ((Boolean) value).booleanValue();
        }
        else if (value instanceof Number)
        {
            return ((Number) value).intValue() != 0;
        }
        else if (value instanceof String)
        {
            return value.equals(TRUE);
        }
        else if (value instanceof short[])
        {
            return ((short[]) value).length != 0 && ((short[]) value)[0] != 0;
        }
        else
        {
            throw new IllegalArgumentException("Can't convert value to boolean." + value.getClass().getName());
        }
    }

    private static int toInt(boolean b)
    {
        return b ? 1 : 0;
    }

    private static short[] toShortArray(double i)
    {
        return new short[]
                {
                    (short) i
                };
    }

    private static Number toNumber(double i, Class<?> c)
    {
        if (c.equals(Integer.class))
        {
            return new Integer((int) i);
        }
        else if (c.equals(Short.class))
        {
            return new Short((short) i);
        }
        else if (c.equals(Double.class))
        {
            return new Double(i);
        }
        else
        {
            throw new IllegalArgumentException("Cannot convert to the given Number type.");
        }
    }
}
