/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnyConverter_Test.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:17:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.uno;

import com.sun.star.lang.XTypeProvider;
import complexlib.ComplexTestCase;

public final class AnyConverter_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] {
            "test_toBoolean", "test_toChar", "test_toByte", "test_toShort",
            "test_toInt", "test_toLong", "test_toFloat", "test_toDouble",
            "test_toObject", "test_toString", "test_toType", "test_toArray",
            "test_isBoolean", "test_isChar", "test_isByte", "test_isShort",
            "test_isInt", "test_isLong", "test_isFloat", "test_isDouble",
            "test_isObject", "test_isString", "test_isType", "test_isArray",
            "test_isVoid" };
    }

    Any anyBool; //
    Any anyChar; //
    Any anyByte; //
    Any anyShort; //
    Any anyInt; //
    Any anyLong; //
    Any anyFloat; //
    Any anyDouble; //
    Any anyObj; //
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
        anyObj= new Any(new Type(Object.class) /* any */, aObj);
        anyStr= new Any(new Type(String.class), aStr);
        anyType= new Any(new Type(Type.class), aType);
        anyArByte= new Any(new Type(byte[].class), arByte);
        anyXTypeProvider= new Any(new Type(XTypeProvider.class), aObj);
    }

    public void test_toBoolean()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        boolean b= AnyConverter.toBoolean(aBool);
        assure("", b == aBool.booleanValue());
        b= AnyConverter.toBoolean(anyBool);
        assure("", b == ((Boolean)anyBool.getObject()).booleanValue());

        // must fail
        try { AnyConverter.toBoolean(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toBoolean(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toChar()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        char b= AnyConverter.toChar(aChar);
        assure("", b == aChar.charValue());
        b= AnyConverter.toChar(anyChar);
        assure("", b == ((Character)anyChar.getObject()).charValue());

        // must fail
        try { AnyConverter.toChar(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toByte()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        byte val= AnyConverter.toByte(aByte);
        assure("", val == aByte.byteValue());
        val= AnyConverter.toByte(anyByte);
        assure("", val == ((Byte)anyByte.getObject()).byteValue());

        // must fail
        try { AnyConverter.toByte(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toByte(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toShort()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        short sh= AnyConverter.toShort(aByte);
        assure("", sh == aByte.byteValue());
        sh= AnyConverter.toShort(aShort);
        assure("", sh == aShort.shortValue());
        sh= AnyConverter.toShort(anyByte);
        assure("", sh == ((Byte)anyByte.getObject()).byteValue());
        sh= AnyConverter.toShort(anyShort);
        assure("", sh == ((Short) anyShort.getObject()).shortValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assure("", 5 == AnyConverter.toUnsignedShort( a ));

        // must fail
        try { AnyConverter.toShort(a); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedShort(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toChar(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toShort(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toInt()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        int val= AnyConverter.toInt(aByte);
        assure("", val == aByte.byteValue());
        val= AnyConverter.toInt(aShort);
        assure("", val == aShort.shortValue());
        val= AnyConverter.toInt(aInt);
        assure("", val == aInt.intValue());
        val= AnyConverter.toInt(anyByte);
        assure("", val == ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toInt(anyShort);
        assure("", val == ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toInt(anyInt);
        assure("", val == ((Integer) anyInt.getObject()).intValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assure("", 5 == AnyConverter.toInt(a));
        assure("", 5 == AnyConverter.toUnsignedInt(a));
        a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
        assure("", 5 == AnyConverter.toUnsignedInt(a));

        // must fail
        try { AnyConverter.toUnsignedInt(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(a); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedInt(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toInt(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toLong()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        long val= AnyConverter.toLong(aByte);
        assure("", val == aByte.byteValue());
        val= AnyConverter.toLong(aShort);
        assure("", val == aShort.shortValue());
        val= AnyConverter.toLong(aInt);
        assure("", val == aInt.intValue());
        val= AnyConverter.toLong(aLong);
        assure("", val == aLong.longValue());
        val= AnyConverter.toLong(anyByte);
        assure("", val == ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toLong(anyShort);
        assure("", val == ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toLong(anyInt);
        assure("", val == ((Integer) anyInt.getObject()).intValue());
        val= AnyConverter.toLong(anyLong);
        assure("", val == ((Long) anyLong.getObject()).longValue());
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assure("", 5 == AnyConverter.toLong(a));
        assure("", 5 == AnyConverter.toUnsignedLong(a));
        a = new Any( Type.UNSIGNED_LONG, new Integer(5) );
        assure("", 5 == AnyConverter.toUnsignedLong(a));
        assure("", 5 == AnyConverter.toLong(a));
        a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
        assure("", 5 == AnyConverter.toUnsignedLong(a));

        // must fail
        try { AnyConverter.toUnsignedLong(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedLong(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(a); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toUnsignedLong(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toLong(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toFloat()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        float val= AnyConverter.toFloat(aByte);
        assure("", val == aByte.byteValue()); // 111 = 111.0
        val= AnyConverter.toFloat(anyByte);
        assure("", val == ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toFloat(aShort);
        assure("", val == aShort.shortValue()); //11111 = 11111.0
        val= AnyConverter.toFloat(anyShort);
        assure("", val == ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toFloat(aFloat);
        assure("", val == aFloat.floatValue());
        val= AnyConverter.toFloat(anyFloat);
        assure("", val == ((Float) anyFloat.getObject()).floatValue());

        // must fail
        try { AnyConverter.toFloat(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toFloat(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toDouble()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        double val= AnyConverter.toDouble(aByte);
        assure("", val == aByte.byteValue()); // 111 = 111.0
        val= AnyConverter.toDouble(anyByte);
        assure("", val == ((Byte)anyByte.getObject()).byteValue());
        val= AnyConverter.toDouble(aShort);
        assure("", val == aShort.shortValue()); //11111 = 11111.0
        val= AnyConverter.toDouble(anyShort);
        assure("", val == ((Short) anyShort.getObject()).shortValue());
        val= AnyConverter.toDouble(aInt);
        assure("", val == aInt.intValue());
        val= AnyConverter.toDouble(anyInt);
        assure("", val == ((Integer) anyInt.getObject()).intValue());
        val= AnyConverter.toDouble(aFloat);
        assure("", val == aFloat.floatValue());
        val= AnyConverter.toDouble(anyFloat);
        float float1= ((Float) anyFloat.getObject()).floatValue();
        assure("", val <= (float1 + 0.1) || val >= (float1 - 0.1));
        val= AnyConverter.toDouble(aDouble);
        assure("", val == aDouble.doubleValue());
        val= AnyConverter.toDouble(anyDouble);
        assure("", val == ((Double) anyDouble.getObject()).doubleValue());

        // must fail
        try { AnyConverter.toDouble(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toDouble(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toObject()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Type _type= new Type(XTypeProvider.class);
        Object val= AnyConverter.toObject(_type, aObj);
        assure("", UnoRuntime.areSame(val, aObj));
        val= AnyConverter.toObject(_type, anyObj);
        assure("", UnoRuntime.areSame(val, anyObj.getObject()));
        val= AnyConverter.toObject(
            _type, new Any( new Type(XTypeProvider.class), null));
        assure("", val == null);

        // structs, exceptions
        com.sun.star.lang.IllegalArgumentException exc =
            new com.sun.star.lang.IllegalArgumentException();
        Any any_exc = new Any(
            new Type("com.sun.star.lang.IllegalArgumentException",
                     TypeClass.EXCEPTION), exc);
        assure("",
               AnyConverter.toObject(
                   new Type(com.sun.star.lang.IllegalArgumentException.class),
                   any_exc).equals(exc));
        assure("",
               AnyConverter.toObject(
                   new Type(com.sun.star.uno.Exception.class), any_exc).equals(
                       exc));
        try {
            AnyConverter.toObject(
                new Type(com.sun.star.uno.RuntimeException.class), any_exc);
            failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {};
        any_exc = new Any(com.sun.star.lang.IllegalArgumentException.class,
                          exc);
        assure("",
               AnyConverter.toObject(
                   new Type(com.sun.star.lang.IllegalArgumentException.class),
                   any_exc).equals(exc));
        assure("",
               AnyConverter.toObject(new Type(com.sun.star.uno.Exception.class),
                                     any_exc).equals(exc));
        try {
            AnyConverter.toObject(
                new Type(com.sun.star.uno.RuntimeException.class), any_exc);
            failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {};

        // must fail
        try { AnyConverter.toObject(_type, aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toObject(_type, new Object()); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toString()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        String val= AnyConverter.toString(aStr);
        assure("", aStr.equals(val));
        val= AnyConverter.toString(anyStr);
        assure("", ((String)anyStr.getObject()).equals(val));

        // must fail
        try { AnyConverter.toString(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(aType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyType); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toString(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toType()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Type val= AnyConverter.toType(aType);
        assure("", val == aType);
        val= AnyConverter.toType(anyType);
        assure("", val == anyType.getObject());

        // must fail
        try { AnyConverter.toType(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_toArray()
        throws com.sun.star.lang.IllegalArgumentException
    {
        // must work
        Object val= AnyConverter.toArray(arByte);
        assure("", val == arByte);
        val= AnyConverter.toArray(anyArByte);
        assure("", val == anyArByte.getObject());

        // must fail
        try { AnyConverter.toType(aBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyBool); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyChar); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyShort); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyInt); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyLong); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyFloat); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyDouble); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyObj); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(aStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyStr); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyVoid); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(arByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
        try { AnyConverter.toType(anyArByte); failed("");
        } catch (com.sun.star.lang.IllegalArgumentException ie) {}
    }

    public void test_isBoolean() {
        assure("", AnyConverter.isBoolean(aBool));
        assure("", AnyConverter.isBoolean(anyBool));
        assure("", !AnyConverter.isBoolean(aChar));
    }

    public void test_isChar() {
        assure("", AnyConverter.isChar(aChar));
        assure("", AnyConverter.isChar(anyChar));
        assure("", !AnyConverter.isChar(aBool));
    }

    public void test_isByte() {
        assure("", AnyConverter.isByte(aByte));
        assure("", AnyConverter.isByte(anyByte));
        assure("", !AnyConverter.isByte(aBool));
    }

    public void test_isShort() {
        assure("", AnyConverter.isShort(aShort));
        assure("", AnyConverter.isShort(anyShort));
        assure("", Type.SHORT.equals(AnyConverter.getType(anyShort)));
        Any a = new Any( Type.UNSIGNED_SHORT, new Short((short)5) );
        assure("", Type.UNSIGNED_SHORT.equals(AnyConverter.getType(a)));
        assure("", !AnyConverter.isShort(a));
        assure("", !Type.SHORT.equals(AnyConverter.getType(a)));
        assure("", !AnyConverter.isShort(aBool));
    }

    public void test_isInt() {
        assure("", AnyConverter.isInt(aInt));
        assure("", AnyConverter.isInt(anyInt));
        assure("", Type.LONG.equals(AnyConverter.getType(anyInt)));
        Any a = new Any(Type.UNSIGNED_LONG, new Integer(5));
        assure("", Type.UNSIGNED_LONG.equals(AnyConverter.getType(a)));
        assure("", !AnyConverter.isInt(a));
        assure("", !Type.LONG.equals(AnyConverter.getType(a)));
        assure("", !AnyConverter.isInt(aBool));
    }

    public void test_isLong() {
        assure("", AnyConverter.isLong(aLong));
        assure("", AnyConverter.isLong(anyLong));
        assure("", Type.HYPER.equals(AnyConverter.getType(anyLong)));
        Any a = new Any( Type.UNSIGNED_HYPER, new Long(5) );
        assure("", Type.UNSIGNED_HYPER.equals( AnyConverter.getType(a) ));
        assure("", !AnyConverter.isLong(a));
        assure("", !Type.HYPER.equals( AnyConverter.getType(a) ));
        assure("", !AnyConverter.isLong(aBool));
    }

    public void test_isFloat() {
        assure("", AnyConverter.isFloat(aFloat));
        assure("", AnyConverter.isFloat(anyFloat));
        assure("", !AnyConverter.isFloat(aDouble));
    }

    public void test_isDouble() {
        assure("", AnyConverter.isDouble(aDouble));
        assure("", AnyConverter.isDouble(anyDouble));
        assure("", !AnyConverter.isDouble(aFloat));
    }

    public void test_isObject() {
        assure("", AnyConverter.isObject(aObj));
        assure("", AnyConverter.isObject(anyObj));
        assure("", AnyConverter.isObject( new Any( XInterface.class, null)));
        assure("", !AnyConverter.isObject(new Object()));
    }

    public void test_isString() {
        assure("", AnyConverter.isString(aStr));
        assure("", AnyConverter.isString(anyStr));
        assure("", !AnyConverter.isString(new Object()));
    }

    public void test_isType() {
        assure("", AnyConverter.isType(aType));
        assure("", AnyConverter.isType(anyType));
        assure("", !AnyConverter.isType(new Object()));
    }

    public void test_isArray() {
        assure("", AnyConverter.isArray(arByte));
        assure("", AnyConverter.isArray(anyArByte));
        assure("", !AnyConverter.isArray(new Object()));
    }

    public void test_isVoid() {
        assure("", AnyConverter.isVoid(anyVoid));
        assure("", !AnyConverter.isVoid(new Object()));
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
