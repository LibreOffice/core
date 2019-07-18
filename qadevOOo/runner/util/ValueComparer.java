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

package util;

import com.sun.star.beans.PropertyValue;
import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import com.sun.star.uno.Type;
import com.sun.star.uno.Enum;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import java.util.HashMap;


public class ValueComparer {

    // Method to change a Value, thought for properties
    public static boolean equalValue( Object first, Object second ) {

        if (first instanceof com.sun.star.uno.Any) {
            try {
                first = AnyConverter.toObject(((Any) first).getType(),first);
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
            }
        }
        if (second instanceof com.sun.star.uno.Any) {
            try {
                second = AnyConverter.toObject(((Any) second).getType(),second);
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
            }
        }
        boolean eq = false;
        try {
            if ( (first==null) || (second == null) ) {
                eq = (first == second);
            }
            else {
                if ( util.utils.isVoid(first) && util.utils.isVoid(second) ) {
                    eq=true;
                } else if ( util.utils.isVoid(first) || util.utils.isVoid(second) ) {
                    eq = (first == second);
                } else {
                    eq = compareObjects(first, second);
                }
            }
        }
        catch (Exception e) {
            System.out.println("Exception occurred while comparing Objects");
            e.printStackTrace();
        }
        return eq;
    } // end of equalValue

    private static boolean compareArrayOfPropertyValue(PropertyValue[] pv1, PropertyValue[] pv2){
        if ( pv1.length != pv2.length) {
            return  false;
        }
        HashMap<String, Object> hm1 = new HashMap<String, Object>();
        boolean result = true;
        int i = 0;

        for (i = 0; i < pv1.length; i++){
            hm1.put(pv1[i].Name, pv1[i].Value);
        }

        i = 0;
        while (i < pv2.length && result) {
            result &= equalValue(hm1.get(pv2[i].Name),pv2[i].Value);
            i++;
        }
        return result;
    }

    private static boolean compareArrays(Object op1, Object op2) throws Exception {

        if (op1 instanceof PropertyValue[] && op2 instanceof PropertyValue[]) {
           return compareArrayOfPropertyValue((PropertyValue[])op1,(PropertyValue[])op2);
       }
        boolean result = true;
        if((op1.getClass().getComponentType() == op2.getClass().getComponentType())
           && (Array.getLength(op1) == Array.getLength(op2)))
        {
            for(int i = 0; i < Array.getLength(op1); ++ i)
                result = result & compareObjects(Array.get(op1, i), Array.get(op2, i));
        } else {
            result = false ;
        }

        return result;
    }

    private static boolean compareInterfaces(XInterface op1, XInterface op2) {
        return op1 == op2;
    }

    private static boolean compareUntil(Class<?> zClass, Class<?> untilClass, Object op1, Object op2) throws Exception {
        boolean result = true;

        // write inherited members first
        Class<?> superClass = zClass.getSuperclass();
        if(superClass != null && superClass != untilClass)
            result = result & compareUntil(superClass, untilClass, op1, op2);

        Field fields[] = zClass.getDeclaredFields();

        for(int i = 0; i < fields.length && result; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                Object obj1 = fields[i].get(op1);
                Object obj2 = fields[i].get(op2);
                if (obj1 instanceof com.sun.star.uno.Any) {
                    try {
                        if (utils.isVoid(obj1)) {
                            obj1 = null;
                        } else {
                            obj1 = AnyConverter.toObject(((Any) obj1).getType(),obj1);
                        }
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    }
                }
                if (obj2 instanceof com.sun.star.uno.Any) {
                    try {
                        if (utils.isVoid(obj2)) {
                            obj2 = null;
                        } else {
                            obj2 = AnyConverter.toObject(((Any) obj2).getType(),obj2);
                        }
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    }
                }

                result = result & compareObjects(obj1, obj2);

            }
        }

        return result;
    }

    private static boolean compareStructs(Object op1, Object op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareUntil(op1.getClass(), Object.class, op1, op2);
        }

        return result;
    }

    private static boolean compareThrowable(Throwable op1, Throwable op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareUntil(op1.getClass(), Throwable.class, op1, op2);

            result = result & op1.getMessage().equals(op2.getMessage());
        }

        return result;
    }

    private static boolean compareEnums(Enum en1, Enum en2) {
        return en1.getValue() == en2.getValue();
    }

    private static boolean compareObjects(Object op1, Object op2) throws Exception {
        if(op1 == op2)
            return true;
        else if(op1==null || op2 == null)
            return op1 == op2;
        else if(op1.getClass().isPrimitive() && op2.getClass().isPrimitive())
            return op1.equals(op2);
        else if(op1.getClass() == Byte.class && op2.getClass() == Byte.class)
            return op1.equals(op2);
        else if(op1.getClass() == Type.class && op2.getClass() == Type.class)
            return op1.equals(op2);
        else if(op1.getClass() == Boolean.class && op2.getClass() == Boolean.class)
            return op1.equals(op2);
        else if(op1.getClass() == Short.class && op2.getClass() == Short.class)
            return op1.equals(op2);
        else if(Throwable.class.isAssignableFrom(op1.getClass()) && Throwable.class.isAssignableFrom(op2.getClass()))
            return compareThrowable((Throwable)op1, (Throwable)op2);
        else if(op1.getClass() == Integer.class && op2.getClass() == Integer.class)
            return op1.equals(op2);
        else if(op1.getClass() == Character.class && op2.getClass() == Character.class)
            return op1.equals(op2);
        else if(op1.getClass() == Long.class && op2.getClass() == Long.class)
            return op1.equals(op2);
        else if(op1.getClass() == Void.class && op2.getClass() == Void.class)
            return op1.equals(op2);
        else if(op1.getClass() == Float.class && op2.getClass() == Float.class)
            return op1.equals(op2);
        else if(op1.getClass() == Double.class && op2.getClass() == Double.class)
            return op1.equals(op2);
        else if(op1.getClass().isArray() && op2.getClass().isArray())
            return compareArrays(op1, op2);
        else if(op1.getClass() == Void.class || op2.getClass() == void.class) // write nothing ?
            return true;
        else if(XInterface.class.isAssignableFrom(op1.getClass()) && XInterface.class.isAssignableFrom(op2.getClass()))
            return compareInterfaces((XInterface)op1, (XInterface)op2);
        else if(Enum.class.isAssignableFrom(op1.getClass()) && Enum.class.isAssignableFrom(op2.getClass()))
            return compareEnums((Enum)op1, (Enum)op2);
        else if(op1.getClass() == String.class && op2.getClass() == String.class) // is it a String ?
            return ((String)op1).equals(op2);
        else // otherwise it must be a struct
            return compareStructs(op1, op2);
    }


}
