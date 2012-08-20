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
package com.sun.star.lib.uno.protocols.urp;

import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;
import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import org.junit.Test;
import static org.junit.Assert.*;

public final class Marshaling_Test {
    @Test public void test() throws Exception {
        short cacheSize = (short)256;
        TestBridge testBridge = new TestBridge();
        Marshal marshal = new Marshal(testBridge, cacheSize);

        TestObject testObject = new TestObject();

        TestPrimitiveSeqStruct x = new TestPrimitiveSeqStruct();
        x.zAny = new Object[]{new Integer(1), new Double(2) };


        Object data[] = new Object[] {
            new com.sun.star.uno.RuntimeException("testRuntimeException"),
            new com.sun.star.uno.Exception("testException"),
            new Boolean(true),
            new Byte((byte)47),
            new Character('k'),
            new Double(0.12345),
            TestEnum.B,
            new Float(0.5678),
            new Integer(0),
            new Integer(128),
            new Integer(0x0f00),
            new Integer(0x0f0000),
            new Integer(0x0f000000),
            new Integer(-128),
            new Integer(0xff00),
            new Integer(0xff0000),
            new Integer(0xff000000),
            new Long(666L),
            new Short((short)444),
            new String("blabla"),
            new Integer(10),  // Any as object
            new Integer(10),  // Any as object
            new Any(new Type(Integer.class), new Integer(10)), // Any as Any
            new Any(new Type(Integer.class), new Integer(10)), // Any as Any
            null,
            new TestPrimitiveStruct(),
            x, //new TestPrimitiveSeqStruct(),
            new byte[]{1,2,3,4,5,6,7}, // primitive sequence
            new int[]{7,6,5,4,3,2,1}, // primitive sequence
            new Object[]{new Integer(123), new String("hallo")}, // any sequence
            new TestPrimitiveStruct[]{new TestPrimitiveStruct(), new TestPrimitiveStruct()}, // sequence of primitive structs
            new TestPrimitiveSeqStruct[]{new TestPrimitiveSeqStruct(), new TestPrimitiveSeqStruct()}, // sequence of primitive structs
            new TestNestedStruct(),
            new TestNestedSeqStruct(),
            new Type(Void.class),
            new Type(String.class),
            new Type(Object.class),
            new Type(Byte.class),
            new Type(Integer.class),
            new Type(Double.class),
            new Type(Float.class),
            new Type(Character.class),
            new Type(Short.class),
            new Type(Boolean.class),
            new Type(void.class),
            new Type(byte.class),
            new Type(int.class),
            new Type(double.class),
            new Type(float.class),
            new Type(char.class),
            new Type(short.class),
            new Type(boolean.class),
            new Type(Class.forName("[Ljava.lang.String;")),
            new Type(Class.forName("[Ljava.lang.Object;")),
            new Type(Class.forName("[B")),
            new Type(Class.forName("[Z")),
            new Type("boolean"),
            new Type("byte"),
            new Type("char"),
            new Type("short"),
            new Type("long"),
            new Type("hyper"),
            new Type("float"),
            new Type("double"),
            new Type("string"),
            new Type("void"),
            new Type("any"),
            new Type(
                "com.sun.star.lib.uno.protocols.urp.TestEnum", TypeClass.ENUM),
            new Type("[]boolean", TypeClass.SEQUENCE),
            new Type("[][]byte", TypeClass.SEQUENCE),
            new Type("[][][]char", TypeClass.SEQUENCE),
            new Type("[][][][]short", TypeClass.SEQUENCE),
            new Type("[][][][][]any", TypeClass.SEQUENCE),
            new Type("com.sun.star.uno.XInterface", TypeClass.INTERFACE),
            new Type("[]com.sun.star.uno.XInterface", TypeClass.SEQUENCE),
            testObject,
            testObject,
            new TestInterfaceStruct(testObject, null)
        };

        TypeDescription dataTypes[] = new TypeDescription[] {
            TypeDescription.getTypeDescription(com.sun.star.uno.RuntimeException.class),
            TypeDescription.getTypeDescription(com.sun.star.uno.Exception.class),
            TypeDescription.getTypeDescription(Boolean.class),
            TypeDescription.getTypeDescription(Byte.class),
            TypeDescription.getTypeDescription(Character.class),
            TypeDescription.getTypeDescription(Double.class),
            TypeDescription.getTypeDescription(TestEnum.class),
            TypeDescription.getTypeDescription(Float.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Integer.class),
            TypeDescription.getTypeDescription(Long.class),
            TypeDescription.getTypeDescription(Short.class),
            TypeDescription.getTypeDescription(String.class),
            TypeDescription.getTypeDescription(Object.class),
            TypeDescription.getTypeDescription(Any.class),
            TypeDescription.getTypeDescription(Any.class),
            TypeDescription.getTypeDescription(Object.class),
            TypeDescription.getTypeDescription(XInterface.class),
            TypeDescription.getTypeDescription(TestPrimitiveStruct.class),
            TypeDescription.getTypeDescription(TestPrimitiveSeqStruct.class),
            TypeDescription.getTypeDescription(Class.forName("[B")),
            TypeDescription.getTypeDescription(Class.forName("[I")),
            TypeDescription.getTypeDescription(Class.forName("[Lcom.sun.star.uno.Any;")),
            TypeDescription.getTypeDescription(
                Class.forName(
                    "[Lcom.sun.star.lib.uno.protocols.urp."
                    + "TestPrimitiveStruct;")),
            TypeDescription.getTypeDescription(
                Class.forName(
                    "[Lcom.sun.star.lib.uno.protocols.urp."
                    + "TestPrimitiveSeqStruct;")),
            TypeDescription.getTypeDescription(TestNestedStruct.class),
            TypeDescription.getTypeDescription(TestNestedSeqStruct.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(Type.class),
            TypeDescription.getTypeDescription(XInterface.class),
            TypeDescription.getTypeDescription(XInterface.class),
            TypeDescription.getTypeDescription(TestInterfaceStruct.class),
        };


        Unmarshal unmarshal = new Unmarshal(testBridge, cacheSize);
        for(int i = 0; i < dataTypes.length; ++ i) {
            Object op1 = data[i];
            marshal.writeValue(dataTypes[i], data[i]);

            unmarshal.reset(marshal.reset());

            Object op2 = unmarshal.readValue(dataTypes[i]);

            if(op1 instanceof Any)
                op1 = ((Any)op1).getObject();

            assertTrue(compareObjects(op1, op2));
        }
    }

    private static boolean compareArrays(Object op1, Object op2) throws Exception {
        boolean result = true;
        if((op1.getClass().getComponentType() == op2.getClass().getComponentType())
           && (Array.getLength(op1) == Array.getLength(op2)))
        {
            Class zClass = op1.getClass().getComponentType();

            for(int i = 0; i < Array.getLength(op1); ++ i)
                result = result & compareObjects(Array.get(op1, i), Array.get(op2, i));
        }

        return result;
    }

    private static boolean compareInterfaces(XInterface op1, XInterface op2) {
        return op1 == op2;
    }

    private static boolean compareStructs(Class zClass, Object op1, Object op2) throws Exception {
        boolean result = true;

        Field fields[] = zClass.getFields();

        for(int i = 0; i < fields.length && result; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                result = result & compareObjects(fields[i].get(op1), fields[i].get(op2));

                if(!result)
                    System.err.println("blabal :" + fields[i]);
            }
        }

        return result;
    }

    private static boolean compareStructs(Object op1, Object op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareStructs(op1.getClass(), op1, op2);
        }

        return result;
    }

    private static boolean compareThrowable(Throwable op1, Throwable op2) throws Exception {
        boolean result = true;

        if(op1.getClass() != op2.getClass())
            result = false;
        else {
            result = compareStructs(op1.getClass(), op1, op2);

            result = result & op1.getMessage().equals(op2.getMessage());
        }

        return result;
    }

    private static boolean compareObjects(Object op1, Object op2) throws Exception {
        boolean result = false;

        if(op1 == op2)
            result = true;

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
            result = compareInterfaces((XInterface)op1, (XInterface)op2);

        else if(op1.getClass() == String.class && op2.getClass() == String.class) // is it a String ?
            result = ((String)op1).equals(op2);

        else if(op1.getClass() == Type.class && op2.getClass() == Type.class) // types?
            result = op1.equals(op2);

        else // otherwise it must be a struct
            result = compareStructs(op1, op2);

        return result;
    }
}
