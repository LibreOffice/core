/*************************************************************************
 *
 *  $RCSfile: ValueComparer.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-11-18 16:17:54 $
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
 *
 ************************************************************************/

package util;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import com.sun.star.uno.Type;
import com.sun.star.uno.Enum;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;


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
            System.out.println("Exception occured while comparing Objects");
            e.printStackTrace();
        }
        return eq;
    } // end of equalValue

    static boolean compareArrays(Object op1, Object op2) throws Exception {
        boolean result = true;
        if((op1.getClass().getComponentType() == op2.getClass().getComponentType())
           && (Array.getLength(op1) == Array.getLength(op2)))
        {
            Class zClass = op1.getClass().getComponentType();

            for(int i = 0; i < Array.getLength(op1); ++ i)
                result = result & compareObjects(Array.get(op1, i), Array.get(op2, i));
        } else {
            result = false ;
        }

        return result;
    }

    static boolean compareInterfaces(XInterface op1, XInterface op2) {
        return op1 == op2;
    }

    static boolean compareUntil(Class zClass, Class untilClass, Object op1, Object op2) throws Exception {
        boolean result = true;

        // write inherited members first
        Class superClass = zClass.getSuperclass();
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

    static boolean compareStructs(Object op1, Object op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareUntil(op1.getClass(), Object.class, op1, op2);
        }

        return result;
    }

    static boolean compareThrowable(Throwable op1, Throwable op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareUntil(op1.getClass(), Throwable.class, op1, op2);

            result = result & op1.getMessage().equals(op2.getMessage());
        }

        return result;
    }

    static boolean compareEnums(Enum en1, Enum en2) {
        return en1.getValue() == en2.getValue();
    }

    static boolean compareObjects(Object op1, Object op2) throws Exception {
        boolean result = false;

        if(op1 == op2)
            result = true;

        if ( (op1==null) || (op2 == null) ) {
            result = (op1 == op2);
            }

        else if(op1.getClass().isPrimitive() && op2.getClass().isPrimitive())
            result = op1.equals(op2);

        else if(op1.getClass() == Byte.class && op2.getClass() == Byte.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Type.class && op2.getClass() == Type.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Boolean.class && op2.getClass() == Boolean.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Short.class && op2.getClass() == Short.class)
            result = op1.equals(op2);

        else if(Throwable.class.isAssignableFrom(op1.getClass()) && Throwable.class.isAssignableFrom(op2.getClass()))
            result = compareThrowable((Throwable)op1, (Throwable)op2);

        else if(op1.getClass() == Integer.class && op2.getClass() == Integer.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Character.class && op2.getClass() == Character.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Long.class && op2.getClass() == Long.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Void.class && op2.getClass() == Void.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Float.class && op2.getClass() == Float.class)
            result = op1.equals(op2);

        else if(op1.getClass() == Double.class && op2.getClass() == Double.class)
            result = op1.equals(op2);

        else if(op1.getClass().isArray() && op2.getClass().isArray())
            result = compareArrays(op1, op2);

        else if(op1.getClass() == Void.class || op2.getClass() == void.class) // write nothing ?
            result = true;

        else if(XInterface.class.isAssignableFrom(op1.getClass()) && XInterface.class.isAssignableFrom(op2.getClass()))
            compareInterfaces((XInterface)op1, (XInterface)op2);

        else if(Enum.class.isAssignableFrom(op1.getClass()) && Enum.class.isAssignableFrom(op2.getClass()))
            compareEnums((Enum)op1, (Enum)op2);

        else if(op1.getClass() == String.class && op2.getClass() == String.class) // is it a String ?
            result = ((String)op1).equals((String)op2);

        else // otherwise it must be a struct
            result = compareStructs(op1, op2);

        return result;
    }


}