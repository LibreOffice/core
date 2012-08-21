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

import com.sun.star.lang.XTypeProvider;
import org.junit.Test;
import static org.junit.Assert.*;

public final class AnyConverter_Test {
    Any anyBool; //
    Any anyChar; //
    Any anyByte; //
    Any anyShort; //
    Any anyInt; //
    Any anyLong; //
    Any anyFloat; //
    Any anyDouble; //
    Any anyStr; //
    Any anyType; //
    Any anyArByte; //
    Any anyVoid;   //
    Any anyXTypeProvider;

    Boolean aBool= new Boolean(true);
    Character aChar= new Character('A');
    Byte aByte= new Byte((byte) 111);
    Short aShort= new Short((short) 11111);
    Integer aInt= new Integer( 1111111);
    Long aLong= new Long( 0xffffffff);
    Float aFloat= new Float( 3.14);
    Double aDouble= new Double( 3.145);
    Object aObj= new ATypeProvider();
    String aStr= new String("I am a string");
    Type aType= new Type(String.class);
    byte[] arByte= new byte[] {1,2,3};

    public AnyConverter_Test() {
        anyVoid= new Any(new Type(void.class), null);
        anyBool= new Any(new Type(Boolean.TYPE), aBool);
        anyChar= new Any(new Type(Character.TYPE), aChar);
        anyByte= new Any(new Type(Byte.TYPE), aByte);
        anyShort= new Any(new Type(Short.TYPE), aShort);
        anyInt= new Any(new Type(Integer.TYPE), aInt);
        anyLong= new Any(new Type(Long.TYPE), aLong);
        anyFloat= new Any(new Type(Float.TYPE), aFloat);
        anyDouble= new Any(new Type(Double.TYPE), aDouble);
        anyStr= new Any(new Type(String.class), aStr);
        anyType= new Any(new Type(Type.class), aType);
        anyArByte= new Any(new Type(byte[].class), arByte);
        anyXTypeProvider= new Any(new Type(XTypeProvider.class), aObj);
    }

    @Test public void test_toBoolean()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        boolean b= AnyConverter.toBoolean(aBool);
        assertEquals(b, aBool.booleanValue());
        b= AnyConverter.toBoolean(anyBool);
        assertEquals(b, ((Boolean)anyBool.getObject()).booleanValue());

        // must fail
        try { AnyConverter.toBoolean(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toChar()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        char b= AnyConverter.toChar(aChar);
        assertEquals(b, aChar.charValue());
        b= AnyConverter.toChar(anyChar);
        assertEquals(b, ((Character)anyChar.getObject()).charValue());

        // must fail
        try { AnyConverter.toChar(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toByte()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        byte val= AnyConverter.toByte(aByte);
        assertEquals(val, aByte.byteValue());
        val= AnyConverter.toByte(anyByte);
        assertEquals(val, ((Byte)anyByte.getObject()).byteValue());

        // must fail
        try { AnyConverter.toByte(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toShort()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        short sh= AnyConverter.toShort(aByte);
        assertEquals(sh, aByte.byteValue());
        sh= AnyConverter.toShort(aShort);
        assertEquals(sh, aShort.shortValue());
        sh= AnyConverter.toShort(anyByte);
        assertEquals(sh, ((Byte)anyByte.getObject()).byteValue());
        sh= AnyConverter.toShort(anyShort);
        assertEquals(sh, ((Short) anyShort.getObject()).shortValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assertEquals(5, AnyConverter.toUnsignedShort( a ));

        // must fail
        try { AnyConverter.toShort(a); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedShort(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toInt()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        int val= AnyConverter.toInt(aByte);
        assertEquals(val, aByte.byteValue());
        val= AnyConverter.toInt(aShort);
        assertEquals(val, aShort.shortValue());
        val= AnyConverter.toInt(aInt);
        assertEquals(val, aInt.intValue());
        val= AnyConverter.toInt(anyByte);
        assertEquals(val, ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toInt(anyShort);
        assertEquals(val, ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toInt(anyInt);
        assertEquals(val, ((Integer) anyInt.getObject()).intValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assertEquals(5, AnyConverter.toInt(a));
        assertEquals(5, AnyConverter.toUnsignedInt(a));
        a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
        assertEquals(5, AnyConverter.toUnsignedInt(a));

        // must fail
        try { AnyConverter.toUnsignedInt(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(a); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedInt(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toLong()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        long val= AnyConverter.toLong(aByte);
        assertEquals(val, aByte.byteValue());
        val= AnyConverter.toLong(aShort);
        assertEquals(val, aShort.shortValue());
        val= AnyConverter.toLong(aInt);
        assertEquals(val, aInt.intValue());
        val= AnyConverter.toLong(aLong);
        assertEquals(val, aLong.longValue());
        val= AnyConverter.toLong(anyByte);
        assertEquals(val, ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toLong(anyShort);
        assertEquals(val, ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toLong(anyInt);
        assertEquals(val, ((Integer) anyInt.getObject()).intValue());
        val= AnyConverter.toLong(anyLong);
        assertEquals(val, ((Long) anyLong.getObject()).longValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assertEquals(5, AnyConverter.toLong(a));
        assertEquals(5, AnyConverter.toUnsignedLong(a));
        a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
        assertEquals(5, AnyConverter.toUnsignedLong(a));
        assertEquals(5, AnyConverter.toLong(a));
        a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
        assertEquals(5, AnyConverter.toUnsignedLong(a));

        // must fail
        try { AnyConverter.toUnsignedLong(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedLong(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(a); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedLong(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toFloat()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        float val= AnyConverter.toFloat(aByte);
        assertEquals(val, aByte.byteValue(), 0); // 111 = 111.0
        val= AnyConverter.toFloat(anyByte);
        assertEquals(val, ((Byte)anyByte.getObject()).byteValue(), 0);
        val= AnyConverter.toFloat(aShort);
        assertEquals(val, aShort.shortValue(), 0); //11111 = 11111.0
        val= AnyConverter.toFloat(anyShort);
        assertEquals(val, ((Short) anyShort.getObject()).shortValue(), 0);
        val= AnyConverter.toFloat(aFloat);
        assertEquals(val, aFloat.floatValue(), 0);
        val= AnyConverter.toFloat(anyFloat);
        assertEquals(val, ((Float) anyFloat.getObject()).floatValue(), 0);

        // must fail
        try { AnyConverter.toFloat(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toDouble()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        double val= AnyConverter.toDouble(aByte);
        assertEquals(val, aByte.byteValue(), 0); // 111 = 111.0
        val= AnyConverter.toDouble(anyByte);
        assertEquals(val, ((Byte)anyByte.getObject()).byteValue(), 0);
        val= AnyConverter.toDouble(aShort);
        assertEquals(val, aShort.shortValue(), 0); //11111 = 11111.0
        val= AnyConverter.toDouble(anyShort);
        assertEquals(val, ((Short) anyShort.getObject()).shortValue(), 0);
        val= AnyConverter.toDouble(aInt);
        assertEquals(val, aInt.intValue(), 0);
        val= AnyConverter.toDouble(anyInt);
        assertEquals(val, ((Integer) anyInt.getObject()).intValue(), 0);
        val= AnyConverter.toDouble(aFloat);
        assertEquals(val, aFloat.floatValue(), 0);
        val= AnyConverter.toDouble(anyFloat);
        float float1= ((Float) anyFloat.getObject()).floatValue();
        assertTrue(val <= (float1 + 0.1) || val >= (float1 - 0.1));
        val= AnyConverter.toDouble(aDouble);
        assertEquals(val, aDouble.doubleValue(), 0);
        val= AnyConverter.toDouble(anyDouble);
        assertEquals(val, ((Double) anyDouble.getObject()).doubleValue(), 0);

        // must fail
        try { AnyConverter.toDouble(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toObject()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Type _type= new Type(XTypeProvider.class);
        Object val= AnyConverter.toObject(_type, aObj);
        assertTrue(UnoRuntime.areSame(val, aObj));
        val= AnyConverter.toObject(
            _type, new Any( new Type(XTypeProvider.class), null));
        assertNull(val);

        // structs, exceptions
        com.sun.star.lang.IllegalArgumentException exc =
            new com.sun.star.lang.IllegalArgumentException();
        Any any_exc = new Any(
            new Type("com.sun.star.lang.IllegalArgumentException",
                     TypeClass.EXCEPTION), exc);
        assertEquals(
            exc,
            AnyConverter.toObject(
                new Type(com.sun.star.lang.IllegalArgumentException.class),
                any_exc));
        assertEquals(
            exc,
            AnyConverter.toObject(
                new Type(com.sun.star.uno.Exception.class), any_exc));
        try {
            AnyConverter.toObject(
                new Type(com.sun.star.uno.RuntimeException.class), any_exc);
            fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        any_exc = new Any(com.sun.star.lang.IllegalArgumentException.class,
                          exc);
        assertEquals(
            exc,
            AnyConverter.toObject(
                new Type(com.sun.star.lang.IllegalArgumentException.class),
                any_exc));
        assertEquals(
            exc,
            AnyConverter.toObject(
                new Type(com.sun.star.uno.Exception.class), any_exc));
        try {
            AnyConverter.toObject(
                new Type(com.sun.star.uno.RuntimeException.class), any_exc);
            fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}

        // must fail
        try { AnyConverter.toObject(_type, aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, new Object()); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toString()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        String val= AnyConverter.toString(aStr);
        assertEquals(val, aStr);
        val= AnyConverter.toString(anyStr);
        assertEquals(val, anyStr.getObject());

        // must fail
        try { AnyConverter.toString(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyType); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toType()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Type val= AnyConverter.toType(aType);
        assertSame(val, aType);
        val= AnyConverter.toType(anyType);
        assertSame(val, anyType.getObject());

        // must fail
        try { AnyConverter.toType(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_toArray()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Object val= AnyConverter.toArray(arByte);
        assertSame(val, arByte);
        val= AnyConverter.toArray(anyArByte);
        assertSame(val, anyArByte.getObject());

        // must fail
        try { AnyConverter.toType(aBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyBool); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyChar); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyShort); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyInt); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyLong); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyFloat); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyDouble); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aObj); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyStr); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyVoid); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(arByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyArByte); fail();
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    @Test public void test_isBoolean() {
        assertTrue(AnyConverter.isBoolean(aBool));
        assertTrue(AnyConverter.isBoolean(anyBool));
        assertFalse(AnyConverter.isBoolean(aChar));
    }

    @Test public void test_isChar() {
        assertTrue(AnyConverter.isChar(aChar));
        assertTrue(AnyConverter.isChar(anyChar));
        assertFalse(AnyConverter.isChar(aBool));
    }

    @Test public void test_isByte() {
        assertTrue(AnyConverter.isByte(aByte));
        assertTrue(AnyConverter.isByte(anyByte));
        assertFalse(AnyConverter.isByte(aBool));
    }

    @Test public void test_isShort() {
        assertTrue(AnyConverter.isShort(aShort));
        assertTrue(AnyConverter.isShort(anyShort));
        assertEquals(Type.SHORT, AnyConverter.getType(anyShort));
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assertEquals(Type.UNSIGNED_SHORT, AnyConverter.getType(a));
        assertFalse(AnyConverter.isShort(a));
        assertFalse(Type.SHORT.equals(AnyConverter.getType(a)));
        assertFalse(AnyConverter.isShort(aBool));
    }

    @Test public void test_isInt() {
        assertTrue(AnyConverter.isInt(aInt));
        assertTrue(AnyConverter.isInt(anyInt));
        assertEquals(Type.LONG, AnyConverter.getType(anyInt));
        Any a = new Any(Type.UNSIGNED_LONG, new Integer(5));
        assertEquals(Type.UNSIGNED_LONG, AnyConverter.getType(a));
        assertFalse(AnyConverter.isInt(a));
        assertFalse(Type.LONG.equals(AnyConverter.getType(a)));
        assertFalse(AnyConverter.isInt(aBool));
    }

    @Test public void test_isLong() {
        assertTrue(AnyConverter.isLong(aLong));
        assertTrue(AnyConverter.isLong(anyLong));
        assertEquals(Type.HYPER, AnyConverter.getType(anyLong));
        Any a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
        assertEquals(Type.UNSIGNED_HYPER, AnyConverter.getType(a));
        assertFalse(AnyConverter.isLong(a));
        assertFalse(Type.HYPER.equals( AnyConverter.getType(a) ));
        assertFalse(AnyConverter.isLong(aBool));
    }

    @Test public void test_isFloat() {
        assertTrue(AnyConverter.isFloat(aFloat));
        assertTrue(AnyConverter.isFloat(anyFloat));
        assertFalse(AnyConverter.isFloat(aDouble));
    }

    @Test public void test_isDouble() {
        assertTrue(AnyConverter.isDouble(aDouble));
        assertTrue(AnyConverter.isDouble(anyDouble));
        assertFalse(AnyConverter.isDouble(aFloat));
    }

    @Test public void test_isObject() {
        assertTrue(AnyConverter.isObject(aObj));
        assertTrue(AnyConverter.isObject( new Any( XInterface.class, null)));
        assertFalse(AnyConverter.isObject(new Object()));
    }

    @Test public void test_isString() {
        assertTrue(AnyConverter.isString(aStr));
        assertTrue(AnyConverter.isString(anyStr));
        assertFalse(AnyConverter.isString(new Object()));
    }

    @Test public void test_isType() {
        assertTrue(AnyConverter.isType(aType));
        assertTrue(AnyConverter.isType(anyType));
        assertFalse(AnyConverter.isType(new Object()));
    }

    @Test public void test_isArray() {
        assertTrue(AnyConverter.isArray(arByte));
        assertTrue(AnyConverter.isArray(anyArByte));
        assertFalse(AnyConverter.isArray(new Object()));
    }

    @Test public void test_isVoid() {
        assertTrue(AnyConverter.isVoid(anyVoid));
        assertFalse(AnyConverter.isVoid(new Object()));
    }
}


class ATypeProvider implements com.sun.star.lang.XTypeProvider
{

    public byte[] getImplementationId()
    {
        return new byte[]{1,2,3};
    }

    public com.sun.star.uno.Type[] getTypes()
    {
        return new Type[]{new Type(XTypeProvider.class)};
    }

}
