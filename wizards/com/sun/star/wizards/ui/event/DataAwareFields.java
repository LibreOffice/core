/*************************************************************************
 *
 *  $RCSfile: DataAwareFields.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:08:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

package com.sun.star.wizards.ui.event;

import java.lang.reflect.Field;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class DataAwareFields {
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
     * @throws NoSuchFieldException
     */
    public static DataAware.Value getFieldValueFor(Object owner, String fieldname, Object value)

    {
        try {
            Field f = owner.getClass().getField(fieldname);

            Class c = f.getType();
            Class c2 = value.getClass();
            if (c.equals(Boolean.TYPE))
                return new BooleanFieldValue(f,c2);
            else if (c.equals(Integer.TYPE))
                return new IntFieldValue(f,c2);
            else if (c.equals(Double.TYPE))
                return new DoubleFieldValue(f,c2);
            else
                return new SimpleFieldValue(f);
        }
        catch (NoSuchFieldException ex) {
            ex.printStackTrace();
            return null;
        }
    }


    private static abstract class FieldValue implements DataAware.Value {
        Field field;

        public FieldValue(Field field_) {
            field = field_;
        }

        public boolean isAssignable(Class type) {
            return field.getDeclaringClass().isAssignableFrom(type);
        }

    }

    private static class BooleanFieldValue extends FieldValue {
        private Class convertTo;

        public BooleanFieldValue(Field f, Class convertTo_) {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                boolean b = field.getBoolean(target);
                if (convertTo.equals(Boolean.class))
                    return b ? Boolean.TRUE : Boolean.FALSE;
                else if (Number.class.isAssignableFrom(convertTo))
                    return toNumber(b ? 1 : 0, convertTo);
                else if (convertTo.equals(String.class))
                    return String.valueOf(b);
                else if (convertTo.isArray())
                    return toShortArray(toInt(b));
                else
                    throw new IllegalArgumentException("Cannot convert boolean value to given type (" + convertTo.getName() + ").");
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target) {
            try {
                field.setBoolean(target, toBoolean(value));
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
            }
        }

    }

    private static class IntFieldValue extends FieldValue {
        private Class convertTo;

        public IntFieldValue(Field f, Class convertTo_) {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                int i = field.getInt(target);
                if (convertTo.equals(Boolean.class))
                    return i != 0 ? Boolean.TRUE : Boolean.FALSE;
                else if (Number.class.isAssignableFrom(convertTo))
                    return toNumber(i, convertTo);
                else if (convertTo.equals(String.class))
                    return String.valueOf(i);
                else if ( convertTo.isArray() )
                    return toShortArray(i);
                else
                    throw new IllegalArgumentException("Cannot convert int value to given type (" + convertTo.getName() + ").");
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
                return null;
            }
        }

        public void set(Object value, Object target) {
            try {
                field.setInt(target, (int) toDouble(value));
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
            }
        }

    }
    private static class DoubleFieldValue extends FieldValue {
        private Class convertTo;

        public DoubleFieldValue(Field f, Class convertTo_) {
            super(f);
            convertTo = convertTo_;
        }

        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                double d = field.getDouble(target);
                if (convertTo.equals(Boolean.class))
                    return d != 0 ? Boolean.TRUE : Boolean.FALSE;
                else if (Number.class.isAssignableFrom(convertTo))
                    return toNumber(d, convertTo);
                else if (convertTo.equals(String.class))
                    return String.valueOf(d);
                else if (convertTo.isArray())
                    return toShortArray(d);
                else
                    throw new IllegalArgumentException("Cannot convert int value to given type (" + convertTo.getName() + ").");
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
                return null;
            }
        }
        public void set(Object value, Object target) {
            try {
                field.setDouble(target, toDouble(value));
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
            }
        }
    }

    private static class SimpleFieldValue extends FieldValue {

        public SimpleFieldValue(Field f) {
            super(f);
        }
        /* (non-Javadoc)
         * @see com.sun.star.wizards.ui.event.DataAware.Value#get(java.lang.Object)
         */
        public Object get(Object target) {
            try {
                return field.get(target);
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
                return null;
            }
        }
        public void set(Object value, Object target) {
            try {
                field.set(target, value);
            } catch (IllegalAccessException ex) {
                ex.printStackTrace();
            }
        }
    }

    private static double toDouble(Object value) {
        if (value instanceof Boolean)
            return ((Boolean) value).booleanValue() ? 1 : 0;
        else if (value instanceof Number)
            return ((Number) value).doubleValue();
        else if (value instanceof String)
            return Double.valueOf((String) value).doubleValue();
        else if (value instanceof short[])
            return ((short[]) value).length == 0 ? 0 : ((short[]) value)[0];
        else
            throw new IllegalArgumentException("Can't convert value to double." + value.getClass().getName());
    }

    private static boolean toBoolean(Object value) {
        if (value instanceof Boolean)
            return ((Boolean) value).booleanValue();
        else if (value instanceof Number)
            return ((Number) value).intValue() != 0;
        else if (value instanceof String)
            return ((String) value).equals(TRUE);
        else if (value instanceof short[])
            return ((short[]) value).length != 0 && ((short[]) value)[0] != 0;
        else
            throw new IllegalArgumentException("Can't convert value to boolean." + value.getClass().getName());
    }

    private static int toInt(boolean b) {
        return b ? 1 : 0;
    }

    private static short[] toShortArray(double i) {
        return new short[] {(short) i };
    }

    private static Number toNumber(double i, Class c) {
        if (c.equals(Integer.class))
            return new Integer((int) i);
        else if (c.equals(Short.class))
            return new Short((short) i);
        else if (c.equals(Double.class))
            return new Double(i);
        else
            throw new IllegalArgumentException("Cannot convert to the given Number type.");
    }

}
