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

package com.sun.star.lib.uno.helper;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.DisposedException;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.uno.XWeak;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.beans.XPropertiesChangeListener;

import java.util.logging.Level;
import java.util.logging.Logger;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import org.junit.Test;

public class PropertySet_Test
{

    private static final Logger logger = Logger.getLogger(PropertySet_Test.class.getName());

    @Test public void convertPropertyValue() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.convertPropertyValue");
        TestClass cl= new TestClass();
        cl.test_convertPropertyValue();
    }

    @Test public void setPropertyValueNoBroadcast() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.setValueNoBroadcast");
        TestClass cl= new TestClass();
        cl.test_setPropertyValueNoBroadcast();
    }

    @Test public void setPropertyValue() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.setPropertyValue");
        TestClass cl= new TestClass();
        cl.resetPropertyMembers();
        Object value;
        Object ret;

        value= Boolean.TRUE;
        cl.setPropertyValue("PropBoolA", value);
        assertEquals(cl.getPropertyValue("PropBoolA"), value);

        value= new Character('A');
        cl.setPropertyValue("PropCharA",value);
        assertEquals(cl.getPropertyValue("PropCharA"), value);

        value= Byte.valueOf((byte) 111);
        cl.setPropertyValue("PropByteA",value);
        assertEquals(cl.getPropertyValue("PropByteA"), value);

        value= Short.valueOf((short)112);
        cl.setPropertyValue("PropShortA", value);
        assertEquals(cl.getPropertyValue("PropShortA"), value);

        value= Integer.valueOf(113);
        cl.setPropertyValue("PropIntA", value);
        assertEquals(cl.getPropertyValue("PropIntA"), value);

        value= Long.valueOf(115);
        cl.setPropertyValue("PropLongA", value);
        assertEquals(cl.getPropertyValue("PropLongA"), value);

        value= new Float(3.14);
        cl.setPropertyValue("PropFloatA", value);
        assertEquals(cl.getPropertyValue("PropFloatA"), value);

        value= new Double(3.145);
        cl.setPropertyValue("PropDoubleA",value);
        assertEquals(cl.getPropertyValue("PropDoubleA"), value);

        value= "string";
        cl.setPropertyValue("PropStringA",value);
        assertEquals(cl.getPropertyValue("PropStringA"), value);

        value= new ComponentBase();
        cl.setPropertyValue("PropXInterfaceA",value);
        assertEquals(cl.getPropertyValue("PropXInterfaceA"), value);

        value= new ComponentBase();
        cl.setPropertyValue("PropXWeakA",value);
        assertEquals(cl.getPropertyValue("PropXWeakA"), value);

        value = com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE;
        cl.setPropertyValue("PropEnum",value);
        assertSame(cl.getPropertyValue("PropEnum"), value);

        value= new byte[]{1,2,3};
        cl.setPropertyValue("PropArrayByteA", value);
        assertEquals(cl.getPropertyValue("PropArrayByteA"), value);

        value= new Type(String.class);
        cl.setPropertyValue("PropTypeA", value);
        assertEquals(cl.getPropertyValue("PropTypeA"), value);

        // test protected,package,private members
        value= Boolean.TRUE;
        cl.setPropertyValue("PropBoolB", value);
        assertEquals(cl.getPropertyValue("PropBoolB"), value);

        cl.setPropertyValue("PropBoolC", value);
        assertEquals(cl.getPropertyValue("PropBoolC"), value);

        try {
            cl.setPropertyValue("PropBoolD", value);
            fail("com.sun.star.lang.WrappedTargetException expected");
        } catch(com.sun.star.lang.WrappedTargetException e) {
            logger.log(Level.FINE, "com.sun.star.lang.WrappedTargetException caught");
        }

        cl.resetPropertyMembers();

        value= Boolean.TRUE;
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new Character('A');
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= Byte.valueOf((byte) 111);
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= Short.valueOf((short)112);
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= Integer.valueOf(113);
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= Long.valueOf(115);
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new Float(3.14);
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new Double(3.145);
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= "string";
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new ComponentBase();
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new ComponentBase();
        cl.setPropertyValue("PropObjectA",value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new byte[]{1,2,3};
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        value= new Type(String.class);
        cl.setPropertyValue("PropObjectA", value);
        assertEquals(cl.getPropertyValue("PropObjectA"), value);

        cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), Byte.valueOf((byte)1)));
        assertEquals(((Byte) cl.getPropertyValue("PropObjectA")).byteValue(), 1);

        cl.resetPropertyMembers();

        value= Boolean.TRUE;
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Character('A');
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= Byte.valueOf((byte) 111);
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= Short.valueOf((short)112);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= Integer.valueOf(113);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= Long.valueOf(115);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Float(3.14);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Double(3.145);
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= "string";
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new ComponentBase();
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new ComponentBase();
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new byte[]{1,2,3};
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Type(String.class);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        cl.resetPropertyMembers();

        value= new Any(new Type(boolean.class), Boolean.TRUE);
        cl.setPropertyValue("PropBoolA", value);
        ret= cl.getPropertyValue("PropBoolA");
        assertTrue(ret instanceof Boolean);
        assertTrue(util.anyEquals(value, ret));

        value= new Any (new Type(char.class), new Character('A'));
        cl.setPropertyValue("PropCharA",value);
        ret= cl.getPropertyValue("PropCharA");
        assertTrue(ret instanceof Character);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
        cl.setPropertyValue("PropByteA",value);
        ret= cl.getPropertyValue("PropByteA");
        assertTrue(ret instanceof Byte);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(short.class), Short.valueOf((short)112));
        cl.setPropertyValue("PropShortA", value);
        ret= cl.getPropertyValue("PropShortA");
        assertTrue(ret instanceof Short);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(int.class), Integer.valueOf(113));
        cl.setPropertyValue("PropIntA", value);
        ret= cl.getPropertyValue("PropIntA");
        assertTrue(ret instanceof Integer);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new  Type(long.class), Long.valueOf(115));
        cl.setPropertyValue("PropLongA", value);
        ret= cl.getPropertyValue("PropLongA");
        assertTrue(ret instanceof Long);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(float.class), new Float(3.14));
        cl.setPropertyValue("PropFloatA", value);
        ret= cl.getPropertyValue("PropFloatA");
        assertTrue(ret instanceof Float);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(double.class),new Double(3.145));
        cl.setPropertyValue("PropDoubleA",value);
        ret= cl.getPropertyValue("PropDoubleA");
        assertTrue(ret instanceof Double);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(String.class), "string");
        cl.setPropertyValue("PropStringA",value);
        ret= cl.getPropertyValue("PropStringA");
        assertTrue(ret instanceof String);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(ComponentBase.class), new ComponentBase());
        cl.setPropertyValue("PropXInterfaceA",value);
        ret= cl.getPropertyValue("PropXInterfaceA");
        assertTrue(ret instanceof ComponentBase);
        assertTrue(util.anyEquals(value, ret));

        value= new Any( new Type(ComponentBase.class), new ComponentBase());
        cl.setPropertyValue("PropXWeakA",value);
        ret= cl.getPropertyValue("PropXWeakA");
        assertTrue(ret instanceof ComponentBase);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(byte[].class), new byte[]{1,2,3});
        cl.setPropertyValue("PropArrayByteA", value);
        ret= cl.getPropertyValue("PropArrayByteA");
        assertTrue(ret instanceof byte[]);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(Type.class), new Type(String.class));
        cl.setPropertyValue("PropTypeA", value);
        ret= cl.getPropertyValue("PropTypeA");
        assertTrue(ret instanceof Type);
        assertTrue(util.anyEquals(value, ret));

        cl.resetPropertyMembers();

        value= new Any(new Type(boolean.class), Boolean.TRUE);
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any (new Type(char.class), new Character('A'));
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(short.class), Short.valueOf((short)112));
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(int.class), Integer.valueOf(113));
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new  Type(long.class), Long.valueOf(115));
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(float.class), new Float(3.14));
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(double.class),new Double(3.145));
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(String.class), "string");
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(ComponentBase.class), new ComponentBase());
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any( new Type(ComponentBase.class), new ComponentBase());
        cl.setPropertyValue("PropAnyA",value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(byte[].class), new byte[]{1,2,3});
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(Type.class), new Type(String.class));
        cl.setPropertyValue("PropAnyA", value);
        ret= cl.getPropertyValue("PropAnyA");
        assertTrue(ret instanceof Any);
        assertTrue(util.anyEquals(value, ret));

        cl.resetPropertyMembers();

        value= Boolean.TRUE;
        cl.setPropertyValue("PropBoolClass", value);
        assertEquals(cl.getPropertyValue("PropBoolClass"), value);

        value= new Character('A');
        cl.setPropertyValue("PropCharClass",value);
        assertEquals(cl.getPropertyValue("PropCharClass"), value);

        value= Byte.valueOf((byte) 111);
        cl.setPropertyValue("PropByteClass",value);
        assertEquals(cl.getPropertyValue("PropByteClass"), value);

        value= Short.valueOf((short)112);
        cl.setPropertyValue("PropShortClass", value);
        assertEquals(cl.getPropertyValue("PropShortClass"), value);

        value= Integer.valueOf(113);
        cl.setPropertyValue("PropIntClass", value);
        assertEquals(cl.getPropertyValue("PropIntClass"), value);

        value= Long.valueOf(115);
        cl.setPropertyValue("PropLongClass", value);
        assertEquals(cl.getPropertyValue("PropLongClass"), value);

        value= new Float(3.14);
        cl.setPropertyValue("PropFloatClass", value);
        assertEquals(cl.getPropertyValue("PropFloatClass"), value);

        value= new Double(3.145);
        cl.setPropertyValue("PropDoubleClass",value);
        assertEquals(cl.getPropertyValue("PropDoubleClass"), value);

        cl.resetPropertyMembers();

        cl.resetPropertyMembers();

        value= new Any(new Type(boolean.class), Boolean.TRUE);
        cl.setPropertyValue("PropBoolClass", value);
        ret= cl.getPropertyValue("PropBoolClass");
        assertTrue(ret instanceof Boolean);
        assertTrue(util.anyEquals(value, ret));

        value= new Any (new Type(char.class), new Character('A'));
        cl.setPropertyValue("PropCharClass",value);
        ret= cl.getPropertyValue("PropCharClass");
        assertTrue(ret instanceof Character);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
        cl.setPropertyValue("PropByteClass",value);
        ret= cl.getPropertyValue("PropByteClass");
        assertTrue(ret instanceof Byte);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(short.class), Short.valueOf((short)112));
        cl.setPropertyValue("PropShortClass", value);
        ret= cl.getPropertyValue("PropShortClass");
        assertTrue(ret instanceof Short);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(int.class), Integer.valueOf(113));
        cl.setPropertyValue("PropIntClass", value);
        ret= cl.getPropertyValue("PropIntClass");
        assertTrue(ret instanceof Integer);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new  Type(long.class), Long.valueOf(115));
        cl.setPropertyValue("PropLongClass", value);
        ret= cl.getPropertyValue("PropLongClass");
        assertTrue(ret instanceof Long);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(float.class), new Float(3.14));
        cl.setPropertyValue("PropFloatClass", value);
        ret= cl.getPropertyValue("PropFloatClass");
        assertTrue(ret instanceof Float);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(double.class),new Double(3.145));
        cl.setPropertyValue("PropDoubleClass",value);
        ret= cl.getPropertyValue("PropDoubleClass");
        assertTrue(ret instanceof Double);
        assertTrue(util.anyEquals(value, ret));

        value= new Any(new Type(String.class), "string");

        // PropertyAttribute.READONLY
        cl.propBoolA.Attributes= PropertyAttribute.READONLY;
        try {
            cl.setPropertyValue("PropBoolA", Boolean.TRUE);
            fail("com.sun.star.beans.PropertyVetoException expected");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            logger.log(Level.FINE, "com.sun.star.beans.PropertyVetoException caught");
        }
        cl.propBoolA.Attributes= 0;

        // MAYBEVOID
        cl.resetPropertyMembers();
        // first MAYBEVOID not set

        //primitive members: must not work

        cl.boolPropA= false;
        try {
            cl.setPropertyValue("PropBoolA", null);
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        try {
            cl.setPropertyValue("PropBoolA", new Any(new Type(boolean.class), null));
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        cl.propBoolA.Attributes= PropertyAttribute.MAYBEVOID;
        try {
            cl.setPropertyValue("PropBoolA", null);
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        cl.propBoolA.Attributes= 0;

        cl.propBoolClass.Attributes= PropertyAttribute.MAYBEVOID;
        cl.boolClassProp= null;
        cl.setPropertyValue("PropBoolClass", null);
        assertNull(cl.boolClassProp);

        // the returned value must be a void any
        Object objAny= cl.getPropertyValue("PropBoolClass");
        assertTrue(util.isVoidAny( objAny));

        cl.boolClassProp= Boolean.TRUE;
        cl.setPropertyValue("PropBoolClass", null);
        assertNull(cl.boolClassProp);

        cl.boolClassProp= Boolean.FALSE;
        cl.setPropertyValue("PropBoolClass", new Any(new Type(boolean.class),null));
        assertNull(cl.boolClassProp);

        cl.propXWeakA.Attributes= PropertyAttribute.MAYBEVOID;
        cl.setPropertyValue("PropXWeakA", null);
        assertTrue(util.isVoidAny(cl.getPropertyValue("PropXWeakA")));

        cl.propXWeakA.Attributes= 0;

        cl.anyPropA= null;
        try {
            cl.setPropertyValue("PropAnyA", null);
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }
        cl.anyPropA= null;
        cl.propAnyA.Attributes= PropertyAttribute.MAYBEVOID;

        new Type(Object.class);
        cl.setPropertyValue("PropAnyA", null);
        assertEquals(cl.anyPropA.getType(), new Type(void.class));
        assertNull(cl.anyPropA.getObject());

        cl.anyPropA= new Any(new Type(byte.class),Byte.valueOf((byte) 111));
        cl.setPropertyValue("PropAnyA", null);
        assertEquals(cl.anyPropA.getType(), new Type(byte.class));
        assertNull(cl.anyPropA.getObject());

        cl.anyPropA= null;
        try {
            cl.setPropertyValue("PropAnyA", new Object());
            fail("com.sun.star.lang.IllegalArgumentException expected");
        }catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        cl.propObjectA.Attributes= 0;
        try {
            cl.setPropertyValue("PropObjectA", null);
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        try {
            cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), null));
            fail("com.sun.star.lang.IllegalArgumentException expected");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "com.sun.star.lang.IllegalArgumentException caught");
        }

        cl.propObjectA.Attributes= PropertyAttribute.MAYBEVOID;
        cl.propObjectA= null;
        cl.setPropertyValue("PropObjectA", null);
        assertNull(cl.propObjectA);

        cl.propObjectA= null;
        cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), null));
        assertNull(cl.propObjectA);
    }

    @Test public void addPropertyChangeListener() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.addPropertyChangeListener,\n" +
                               "PropertySet.removePropertChangeListener," +
                               "PropertySet.addVetoableChangeListener, \n" +
                               "PropertySet.removeVetoableChangeListener" +
                               "Notification of listeners");
        TestClass cl= new TestClass();
        Listener li= new Listener();

        cl.addPropertyChangeListener("PropByteA", li);
        Byte val1= Byte.valueOf((byte)115);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);

        cl.propByteA.Attributes = PropertyAttribute.BOUND;
        cl.addPropertyChangeListener("PropByteA", li);
        Byte val2= Byte.valueOf((byte)116);
        cl.setPropertyValue("PropByteA", val2);
        assertEquals(li.nChangeCalled, 1);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li.evt.OldValue, val1);
        assertEquals(li.evt.NewValue, val2);
        assertSame(li.evt.Source, cl);

        li.reset();
        Listener li2= new Listener();
        cl.addPropertyChangeListener("PropByteA", li2);
        Byte val3= Byte.valueOf((byte) 117);
        cl.setPropertyValue("PropByteA", val3);
        assertEquals(li.nChangeCalled, 1);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 1);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li.evt.OldValue, val2);
        assertEquals(li.evt.NewValue,val3);
        assertSame(li.evt.Source, cl);
        assertEquals(li2.evt.OldValue, val2);
        assertEquals(li2.evt.NewValue, val3);
        assertSame(li2.evt.Source, cl);

        li.reset();
        li2.reset();
        Listener li3= new Listener();
        val1= Byte.valueOf((byte)118);
        cl.addPropertyChangeListener("", li3);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 1);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 1);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 1);
        assertEquals(li3.nVetoCalled, 0);
        assertEquals(li.evt.OldValue, val3);
        assertEquals(li.evt.NewValue, val1);
        assertSame(li.evt.Source, cl);
        assertEquals(li2.evt.OldValue, val3);
        assertEquals(li2.evt.NewValue, val1);
        assertSame(li2.evt.Source, cl);
        assertEquals(li3.evt.OldValue, val3);
        assertEquals(li3.evt.NewValue, val1);
        assertSame(li3.evt.Source, cl);

        li.reset();
        li2.reset();
        li3.reset();
        cl.removePropertyChangeListener("PropByteA",li);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 1);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 1);
        assertEquals(li3.nVetoCalled, 0);

        cl.removePropertyChangeListener("PropByteA", li2);
        li.reset();
        li2.reset();
        li3.reset();
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 1);
        assertEquals(li3.nVetoCalled, 0);

        cl.removePropertyChangeListener("", li3);
        li.reset();
        li2.reset();
        li3.reset();
        cl.setPropertyValue("PropByteA", val2);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 0);
        assertEquals(li3.nVetoCalled, 0);

        cl.addPropertyChangeListener("PropByteA", li);
        cl.addPropertyChangeListener("PropByteA", li2);
        cl.addPropertyChangeListener("", li3);
        cl.dispose();
        li.reset();
        li2.reset();
        li3.reset();
        try {
            cl.setPropertyValue("PropByteA", val2);
            fail("DisposedException expected");
        } catch (DisposedException e) {
            logger.log(Level.FINE, "DisposedException caught");
        }

        //Vetoable tests
        cl= new TestClass();
        li.reset();
        li2.reset();
        li3.reset();
        cl.addVetoableChangeListener("PropByteA", li);
        val1= Byte.valueOf((byte)115);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled,  0);
        assertEquals(li.nVetoCalled, 0);

        cl.propByteA.Attributes = PropertyAttribute.CONSTRAINED;
        cl.addVetoableChangeListener("PropByteA", li);
        val2= Byte.valueOf((byte)116);
        li.reset();
        cl.setPropertyValue("PropByteA", val2);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 1);
        assertEquals(li.evt.OldValue, val1);
        assertEquals(li.evt.NewValue, val2);
        assertSame(li.evt.Source, cl);

        li.reset();
        li2.reset();
        li3.reset();
        cl.addVetoableChangeListener("PropByteA", li2);
        val3= Byte.valueOf((byte) 117);
        cl.setPropertyValue("PropByteA", val3);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 1);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 1);
        assertEquals(li.evt.OldValue, val2);
        assertEquals(li.evt.NewValue, val3);
        assertSame(li.evt.Source, cl);
        assertEquals(li2.evt.OldValue, val2);
        assertEquals(li2.evt.NewValue, val3);
        assertSame(li2.evt.Source, cl);

        li.reset();
        li2.reset();
        li3.reset();
        val1= Byte.valueOf((byte)118);
        cl.addVetoableChangeListener("", li3);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 1);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 1);
        assertEquals(li3.nChangeCalled, 0);
        assertEquals(li3.nVetoCalled, 1);
        assertEquals(li.evt.OldValue, val3);
        assertEquals(li.evt.NewValue, val1);
        assertSame(li.evt.Source, cl);
        assertEquals(li2.evt.OldValue, val3);
        assertEquals(li2.evt.NewValue, val1);
        assertSame(li2.evt.Source, cl);
        assertEquals(li3.evt.OldValue, val3);
        assertEquals(li3.evt.NewValue, val1);
        assertSame(li3.evt.Source, cl);

        li.reset();
        li2.reset();
        li3.reset();
        // Test Veto Exception
        cl.setPropertyValue("PropByteA", val1);
        li.bVeto= true;
        try {
            cl.setPropertyValue("PropByteA", val2);
            fail("PropertyVetoException expected");
        } catch (PropertyVetoException e) {
            logger.log(Level.FINE, "PropertyVetoException caught");
        }
        assertSame(cl.bytePropA, val1.byteValue());
        li.bVeto= false;

        li.reset();
        li2.reset();
        li3.reset();
        cl.removeVetoableChangeListener("PropByteA",li);
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 1);
        assertEquals(li3.nChangeCalled, 0);
        assertEquals(li3.nVetoCalled, 1);

        cl.removeVetoableChangeListener("PropByteA", li2);
        li.reset();
        li2.reset();
        li3.reset();
        cl.setPropertyValue("PropByteA", val1);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 0);
        assertEquals(li3.nVetoCalled, 1);

        cl.removeVetoableChangeListener("", li3);
        li.reset();
        li2.reset();
        li3.reset();
        cl.setPropertyValue("PropByteA", val2);
        assertEquals(li.nChangeCalled, 0);
        assertEquals(li.nVetoCalled, 0);
        assertEquals(li2.nChangeCalled, 0);
        assertEquals(li2.nVetoCalled, 0);
        assertEquals(li3.nChangeCalled, 0);
        assertEquals(li3.nVetoCalled, 0);

        cl.addVetoableChangeListener("PropByteA", li);
        cl.addVetoableChangeListener("PropByteA", li2);
        cl.addVetoableChangeListener("", li3);
        cl.dispose();
        li.reset();
        li2.reset();
        li3.reset();
        try {
            cl.setPropertyValue("PropByteA", val2);
            fail("DisposedException expected");
        } catch (DisposedException e) {
            logger.log(Level.FINE, "DisposedException caught");
        }
    }

    @Test public void getPropertySetInfo() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.getPropertySetInfo");
        TestClass cl= new TestClass();
        XPropertySetInfo info= cl.getPropertySetInfo();
        Property[] arProps= info.getProperties();
        Property[] arRegProps= cl.getRegisteredProperties();
        assertEquals(arProps.length, arRegProps.length);

        for (int j= 0; j < arProps.length; j++)
        {
            boolean bFound= false;
            for (int k= 0; k < arRegProps.length; k++)
            {
                if (arProps[j] == arRegProps[k])
                {
                    bFound= true;
                    break;
                }
            }
            assertTrue(bFound);
        }

        for (int j= 0; j < arRegProps.length; j++)
        {
            Property prop= info.getPropertyByName(arRegProps[j].Name);
            assertSame(prop, arRegProps[j]);
            assertTrue(info.hasPropertyByName(arRegProps[j].Name));
        }
    }

    @Test public void setFastPropertyValue() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.setFastPropertyValue");
        TestClass cl= new TestClass();
        cl.setFastPropertyValue(5, Integer.valueOf(111));
        assertEquals(cl.intPropA,111);
        try {
            cl.setFastPropertyValue(-1, Integer.valueOf(1));
            fail("UnknownPropertyException expected");
        } catch(UnknownPropertyException e) {
            logger.log(Level.FINE, "UnknownPropertyException caught");
        }
    }

    @Test public void getFastPropertyValue() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.setFastPropertyValue");
        TestClass cl= new TestClass();
        cl.setFastPropertyValue(5, Integer.valueOf(111));
        Integer aInt= (Integer) cl.getFastPropertyValue(5);
        assertEquals(aInt.intValue(), 111);
    }

    @Test public void setPropertyValues() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.setPropertyValues");
        TestClass cl= new TestClass();
        cl.setPropertyValues(new String[0], new Object[0]);
        String[] arNames= new String[] {"PropCharA","PropIntClass","PropObjectA"};
        Character aChar= new Character('A');
        Integer aInt= Integer.valueOf(111);
        Byte aByte= Byte.valueOf((byte)11);
        Object[] values= new Object[]{aChar, aInt, aByte};
        cl.setPropertyValues(arNames, values);
        assertEquals(cl.charPropA, 'A');
        assertEquals(cl.intClassProp.intValue(), 111);
        assertEquals(((Byte)cl.objectPropA).byteValue(), 11);

        arNames= new String[] {"blabla","PropIntClass","PropObjectA"};
        cl.resetPropertyMembers();
        cl.setPropertyValues(arNames, values);
        assertEquals(cl.intClassProp.intValue(), 111);
        assertEquals(((Byte)cl.objectPropA).byteValue(), 11);
    }

    @Test public void getPropertyValues() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.getPropertyValues");
        TestClass cl= new TestClass();
        cl.charPropA= 'A';
        cl.intClassProp= Integer.valueOf(111);
        cl.objectPropA= Byte.valueOf((byte)11);
        Object[] values= cl.getPropertyValues(new String[] {"PropCharA","PropIntClass","PropObjectA"});
        assertEquals(((Character) values[0]).charValue(), 'A');
        assertEquals(((Integer) values[1]).intValue(), 111);
        assertEquals(((Byte) values[2]).byteValue(), 11);
    }

    // Currently the listeners are always notified if one of properties has changed.
    // The property names in the first argument are ignored.
    @Test public void addPropertiesChangeListener() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.addPropertiesChangeListener\n" +
                               "PropertySet.removePropertiesChangeListener\n" +
                               "notification of such listeners");
        TestClass cl= new TestClass();
        Listener li1= new Listener();

        cl.addPropertiesChangeListener(new String[]{"PropCharA"}, li1);
        cl.setPropertyValue("PropCharA", new Character('B'));
        assertEquals(li1.nPropertiesChange, 0);

        cl.propCharA.Attributes= PropertyAttribute.BOUND;
        cl.setPropertyValue("PropCharA", new Character('C'));
        assertEquals(li1.nPropertiesChange, 1);

        PropertyChangeEvent evt= li1.arEvt[0];
        assertEquals(evt.PropertyName, "PropCharA");
        assertEquals(((Character)evt.OldValue).charValue(), 'B');
        assertEquals(((Character) evt.NewValue).charValue(), 'C');

        li1.reset();
        cl.removePropertiesChangeListener(li1);
        cl.setPropertyValue("PropCharA", new Character('F'));
        assertEquals(li1.nPropertiesChange, 0);
    }

    @Test public void firePropertiesChangeEvent() throws Exception
    {
        logger.log(Level.INFO, "PropertySet.firePropertiesChangeEvent");
        TestClass cl= new TestClass();
        Listener li1= new Listener();

        cl.intClassProp= Integer.valueOf(111);
        cl.charPropA= 'A';
        cl.firePropertiesChangeEvent(new String[]{"PropCharA","PropIntClass"},  li1);
        assertEquals(li1.nPropertiesChange, 1);

        PropertyChangeEvent[] arEvt= li1.arEvt;
        assertEquals(arEvt[0].PropertyName, "PropCharA");
        assertEquals(((Character) arEvt[0].OldValue).charValue(), 'A');
        assertEquals(((Character) arEvt[0].NewValue).charValue(), 'A');
        assertEquals(arEvt[1].PropertyName, "PropIntClass");
        assertEquals(((Integer) arEvt[1].OldValue).intValue(), 111);
        assertEquals(((Integer) arEvt[1].NewValue).intValue(), 111);
    }

    @Test public void registerProperty1() throws Exception
    {
        TestClass2 cl= new TestClass2();
        cl.test_registerProperty1();
    }

    @Test public void registerProperty2() throws Exception
    {
        TestClass2 cl= new TestClass2();
        cl.test_registerProperty2();
    }
}

class TestClass extends PropertySet
{
    private static final Logger logger = Logger.getLogger(TestClass.class.getName());

    public Property propBoolA= new Property("PropBoolA", 1, new Type(Boolean.TYPE), (short)0);
    public boolean boolPropA;
    public Property propCharA= new Property("PropCharA", 2, new Type(Character.TYPE), (short) 0);
    public char charPropA;
    public Property propByteA= new Property("PropByteA", 3, new Type(Byte.TYPE), (short) 0);
    public byte bytePropA;
    public Property propShortA= new Property("PropShortA", 4, new Type(Short.TYPE), (short) 0);
    public short shortPropA;
    public Property propIntA= new Property("PropIntA", 5, new Type(Integer.TYPE), (short) 0);
    public int intPropA;
    public Property propLongA= new Property("PropLongA", 6, new Type(Long.TYPE), (short) 0);
    public long longPropA;
    public Property propFloatA= new Property("PropFloatA", 7, new Type(Float.TYPE), (short) 0);
    public float floatPropA;
    public Property propDoubleA= new Property("PropDoubleA", 8, new Type(Double.TYPE), (short) 0);
    public double doublePropA;
    public Property propStringA= new Property("PropStringA", 9, new Type(String.class), (short) 0);
    public String stringPropA;
    public Property propArrayByteA= new Property("PropArrayByteA", 10, new Type(byte[].class), (short) 0);
    public byte[] arBytePropA;
    public Property propTypeA= new Property("PropTypeA", 11, new Type(Type.class), (short) 0);
    public Type typePropA;
    public Property propObjectA= new Property("PropObjectA",12, new Type(Object.class), (short) 0);
    public Object objectPropA;
    public Property propAnyA= new Property("PropAnyA", 13, new Type(Any.class), (short) 0);
    public Any anyPropA;
    public Property propXInterfaceA= new Property("PropXInterfaceA", 13, new Type(Any.class), (short) 0);
    public XInterface xInterfacePropA;
    public Property propXWeakA= new Property("PropXWeakA", 13, new Type(Any.class), (short) 0);
    public XWeak xWeakPropA;
    public Property propEnum =
        new Property("PropEnum", 14, new Type("com.sun.star.beans.PropertyState", TypeClass.ENUM), (short)0);
    public com.sun.star.beans.PropertyState enumPropertyState = com.sun.star.beans.PropertyState.DEFAULT_VALUE;
    // Test private, protected, package access, Anys as arguments and members, members with a value

    public Property propBoolB= new Property("PropBoolB", 101, new Type(Boolean.TYPE), (short) 0);
    protected boolean boolPropB;

    public Property propBoolC= new Property("PropBoolC", 201, new Type(Boolean.TYPE), (short) 0);
    boolean boolPropC;

    public Property propBoolD= new Property("PropBoolD", 301, new Type(Boolean.TYPE), (short) 0);

    public Property propBoolClass= new Property("PropBoolClass", 1001, new Type(Boolean.class), (short) 0);
    public Boolean boolClassProp;
    public Property propCharClass= new Property("PropCharClass", 1002, new Type(Character.class), (short) 0);
    public Character charClassProp;
    public Property propByteClass= new Property("PropByteClass", 1003, new Type(Byte.class), (short) 0);
    public Byte byteClassProp;
    public Property propShortClass= new Property("PropShortClass", 1004, new Type(Short.class), (short) 0);
    public Short shortClassProp;
    public Property propIntClass= new Property("PropIntClass", 1005, new Type(Integer.class), (short) 0);
    public Integer intClassProp;
    public Property propLongClass= new Property("PropLongClass", 1006, new Type(Long.class), (short) 0);
    public Long longClassProp;
    public Property propFloatClass= new Property("PropFloatClass", 1007, new Type(Float.class), (short) 0);
    public Float floatClassProp;
    public Property propDoubleClass= new Property("PropDoubleClass", 1008, new Type(Double.class), (short) 0);
    public Double doubleClassProp;


    public TestClass()
    {

        super();
        // When adding properties then modify the getRegisteredProperties method
        // registerProperty(String name, int handle, Type type, short attributes, String memberName)
        registerProperty(propBoolA, "boolPropA");
        registerProperty(propCharA, "charPropA");
        registerProperty(propByteA, "bytePropA");
        registerProperty(propShortA, "shortPropA");
        registerProperty(propIntA, "intPropA");
        registerProperty(propLongA, "longPropA");
        registerProperty(propFloatA, "floatPropA");
        registerProperty(propDoubleA, "doublePropA");
        registerProperty(propStringA, "stringPropA");
        registerProperty(propArrayByteA, "arBytePropA");
        registerProperty(propTypeA, "typePropA");
        registerProperty(propObjectA, "objectPropA");
        registerProperty(propAnyA, "anyPropA");
        registerProperty(propXInterfaceA, "xInterfacePropA");
        registerProperty(propXWeakA, "xWeakPropA");
        registerProperty(propEnum,"enumPropertyState");
        registerProperty(propBoolB, "boolPropB");
        registerProperty(propBoolC, "boolPropC");
        registerProperty(propBoolD, "boolPropD");
        registerProperty(propBoolClass, "boolClassProp");
        registerProperty(propCharClass, "charClassProp");
        registerProperty(propByteClass, "byteClassProp");
        registerProperty(propShortClass, "shortClassProp");
        registerProperty(propIntClass, "intClassProp");
        registerProperty(propLongClass, "longClassProp");
        registerProperty(propFloatClass, "floatClassProp");
        registerProperty(propDoubleClass, "doubleClassProp");
    }

    /** When adding properties then modify the getRegisteredProperties method
     */
    public Property[] getRegisteredProperties()
    {
        return new Property[] {
                    propBoolA, propCharA, propByteA, propShortA,
                    propIntA, propLongA, propFloatA, propDoubleA,
                    propStringA, propArrayByteA, propTypeA, propObjectA,
                    propAnyA, propXInterfaceA, propXWeakA, propEnum, propBoolB,
                    propBoolC, propBoolD, propBoolClass, propCharClass,
                    propByteClass, propShortClass, propIntClass, propLongClass,
                    propFloatClass, propDoubleClass
        };

    }

    public void test_convertPropertyValue() throws Exception
    {
        resetPropertyMembers();
        Object[] outOldVal= new Object[1];
        Object[] outNewVal= new Object[1];

        Object value= Boolean.TRUE;
        assertTrue(convertPropertyValue(propBoolA,  outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Boolean);
        assertEquals(outNewVal[0], value);
        assertEquals(outOldVal[0], Boolean.FALSE);

        value= new Character('A');
        assertTrue(convertPropertyValue(propCharA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Character);
        assertEquals(outNewVal[0], value);
        assertEquals(outOldVal[0], new Character((char)0));

        charPropA= 'B';
        assertTrue(convertPropertyValue(propCharA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Character);
        assertEquals(outNewVal[0], value);
        assertEquals(outOldVal[0], new Character('B'));

        value= Byte.valueOf((byte) 111);
        assertTrue(convertPropertyValue(propByteA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Byte);
        assertEquals(outNewVal[0], value);

        value= Short.valueOf((short) 112);
        assertTrue(convertPropertyValue(propShortA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Short);
        assertEquals(outNewVal[0], value);

        value= Integer.valueOf( 113);
        assertTrue(convertPropertyValue(propIntA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Integer);
        assertEquals(outNewVal[0], value);

        value= Long.valueOf(114);
        assertTrue(convertPropertyValue(propLongA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Long);
        assertEquals(outNewVal[0], value);

        value= new Float(3.14);
        assertTrue(convertPropertyValue(propFloatA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Float);
        assertEquals(outNewVal[0], value);

        value= new Double(3.145);
        assertTrue(convertPropertyValue(propDoubleA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Double);
        assertEquals(outNewVal[0], value);

        value= "string";
        assertTrue(convertPropertyValue(propStringA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof String);
        assertEquals(outNewVal[0], value);

        value= new byte[]{1,2,3};
        arBytePropA= null;
        assertTrue(convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof byte[]);
        assertEquals(outNewVal[0], value);
        assertNull(outOldVal[0]);

        assertTrue(convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value));
        assertNull(outOldVal[0]);

        value= new Type(XInterface.class);
        assertTrue(convertPropertyValue(propTypeA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Type);
        assertEquals(outNewVal[0], value);

        value= new Object(); // TypeClass.VOID
        assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
        assertEquals(outNewVal[0], value);

        value= Integer.valueOf(111);
        assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Integer);
        assertEquals(outNewVal[0], value);

        value= new ComponentBase();
        assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
        assertEquals(outNewVal[0], value);

        value= Integer.valueOf(111);
        assertTrue(convertPropertyValue(propAnyA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Any);
        assertEquals(((Any)outNewVal[0]).getType(), new Type(Integer.class));
        assertEquals(((Any)outNewVal[0]).getObject(), value);

        XWeak oWeak= new ComponentBase();
        value= oWeak;
        // The returned Any must contain an XInterface
        assertTrue(convertPropertyValue(propAnyA, outNewVal, outOldVal, value));
        assertEquals(((Any) outNewVal[0]).getType(), new Type(XInterface.class));
        assertSame(((Any) outNewVal[0]).getObject(), oWeak);

        value= new ComponentBase();
        assertTrue(convertPropertyValue(propXInterfaceA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof XInterface);
        assertEquals(outNewVal[0], value);
        assertTrue(convertPropertyValue(propXWeakA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof XWeak);
        assertEquals(outNewVal[0], value);

        value = com.sun.star.beans.PropertyState.DIRECT_VALUE;
        assertTrue(convertPropertyValue(propEnum, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof com.sun.star.uno.Enum);
        assertEquals(outNewVal[0], value);

        // Any arguments ------------------------------------------------------------------
        value= new Any( new Type(Integer.class),Integer.valueOf(111));
        assertTrue(convertPropertyValue(propIntA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Integer);
        assertEquals(outNewVal[0], ((Any) value).getObject());

        value= new Any(new Type(Boolean.class), Boolean.TRUE);
        assertTrue(convertPropertyValue(propBoolA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Boolean);
        assertEquals(outNewVal[0], ((Any) value).getObject());

        //Character, Byte, Short, Long
        // must fail
        try {
            value= new Any(new Type(Object.class), new Object());
            fail("java.lang.IllegalArgumentException expected");
            assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
            assertTrue(convertPropertyValue(propAnyA, outNewVal, outOldVal, value));
        } catch (java.lang.IllegalArgumentException e) {
            logger.log(Level.FINE, "java.lang.IllegalArgumentException caught");
        }

        value= new Any(new Type(Integer.class), Integer.valueOf(111));
        assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Integer);
        assertEquals(outNewVal[0], ((Any)value).getObject());
        assertTrue(convertPropertyValue(propAnyA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Any);
        assertEquals(((Any) outNewVal[0]).getType(), ((Any) value).getType());
        assertEquals(((Any) outNewVal[0]).getObject(), ((Any) value).getObject());

        value= new Any(new Type(XInterface.class), new ComponentBase());
        assertTrue(convertPropertyValue(propObjectA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof XInterface);
        assertSame(outNewVal[0], ((Any) value).getObject());
        assertTrue(convertPropertyValue(propXInterfaceA, outNewVal, outOldVal, value));
        assertSame(outNewVal[0], ((Any) value).getObject());

        value= new Any(new Type(byte[].class), new byte[]{1,2,3});
        assertTrue(convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof byte[]);

        // test private, protected, package fields
        value= Boolean.TRUE;
        assertTrue(convertPropertyValue(propBoolB, outNewVal, outOldVal, value));
        assertEquals(((Boolean)value).booleanValue(), ((Boolean) outNewVal[0]).booleanValue());
        assertTrue(convertPropertyValue(propBoolC, outNewVal,  outOldVal, value));
        assertEquals(((Boolean)value).booleanValue(), ((Boolean) outNewVal[0]).booleanValue());
        // must fail because the member boolPropD is private
        try{
            convertPropertyValue(propBoolD, outNewVal,  outOldVal, value);
            fail("com.sun.star.lang.WrappedTargetException expected");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            logger.log(Level.FINE, "com.sun.star.lang.WrappedTargetException caught");
        }

        // Properties member of type Byte,Short etc.
        value= Boolean.TRUE;
        assertTrue(convertPropertyValue(propBoolClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Boolean);
        assertEquals(outNewVal[0], value);

        value= new Character('A');
        assertTrue(convertPropertyValue(propCharClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Character);
        assertEquals(outNewVal[0], value);

        value= Byte.valueOf((byte) 111);
        assertTrue(convertPropertyValue(propByteClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Byte);
        assertEquals(outNewVal[0], value);

        value= Short.valueOf((short) 112);
        assertTrue(convertPropertyValue(propShortClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Short);
        assertEquals(outNewVal[0], value);

        value= Integer.valueOf(113);
        assertTrue(convertPropertyValue(propIntClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Integer);
        assertEquals(outNewVal[0], value);

        value= Long.valueOf(114);
        assertTrue(convertPropertyValue(propLongClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Long);
        assertEquals(outNewVal[0], value);

        value= new Float(3.14);
        assertTrue(convertPropertyValue(propFloatClass, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Float);
        assertEquals(outNewVal[0], value);

        value= new Double(3.145);
        assertTrue(convertPropertyValue(propDoubleA, outNewVal, outOldVal, value));
        assertTrue(outNewVal[0] instanceof Double);
        assertEquals(outNewVal[0], value);
    }

    public void test_setPropertyValueNoBroadcast() throws Exception
    {
        resetPropertyMembers();

        Object value= Boolean.TRUE;
        setPropertyValueNoBroadcast(propBoolA, value);
        assertEquals(boolPropA, ((Boolean) value).booleanValue());

        value= new Character('A');
        setPropertyValueNoBroadcast(propCharA, value);
        assertEquals(charPropA, ((Character) value).charValue());

        value= Byte.valueOf((byte) 111);
        setPropertyValueNoBroadcast(propByteA, value);
        assertEquals(bytePropA, ((Byte)value).byteValue());

        value= Short.valueOf((short) 112);
        setPropertyValueNoBroadcast(propShortA, value);
        assertEquals(shortPropA, ((Short) value).shortValue());

        value= Integer.valueOf( 113);
        setPropertyValueNoBroadcast(propIntA, value);
        assertEquals(intPropA, ((Integer) value).intValue());

        value= Long.valueOf(114);
        setPropertyValueNoBroadcast(propLongA, value);
        assertEquals(longPropA, ((Long) value).longValue());

        value= new Float(3.14);
        setPropertyValueNoBroadcast(propFloatA,  value);
        assertEquals(floatPropA, ((Float) value).floatValue(), 0.0f);

        value= new Double(3.145);
        setPropertyValueNoBroadcast(propDoubleA, value);
        assertEquals(doublePropA, ((Double) value).doubleValue(), 0.0f);

        value= "string";
        setPropertyValueNoBroadcast(propStringA, value);
        assertEquals(stringPropA, value);

        value= new byte[]{1,2,3};
        setPropertyValueNoBroadcast(propArrayByteA, value);
        assertEquals(arBytePropA, value);

        value= new Type(XInterface.class);
        setPropertyValueNoBroadcast(propTypeA, value);
        assertEquals(typePropA, value);

        value= Integer.valueOf(111);
        setPropertyValueNoBroadcast(propObjectA, value);
        assertEquals(objectPropA, value);

        value= new ComponentBase();
        setPropertyValueNoBroadcast(propObjectA, value);
        assertEquals(objectPropA, value);

        value= new Any( new Type(Integer.TYPE), Integer.valueOf(111));
        setPropertyValueNoBroadcast(propAnyA, value);
        assertTrue(util.anyEquals(anyPropA, value));

        value= new ComponentBase();
        setPropertyValueNoBroadcast(propXInterfaceA, value);
        assertTrue(xInterfacePropA instanceof XInterface);
        assertEquals(xInterfacePropA, value);

        setPropertyValueNoBroadcast(propXWeakA, value);
        assertTrue(xInterfacePropA instanceof XWeak);
        assertEquals(xInterfacePropA, value);

        value = com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE;
        setPropertyValueNoBroadcast(propEnum, value);
        assertSame(enumPropertyState, value);

        value= Boolean.TRUE;
        setPropertyValueNoBroadcast(propBoolB, value);
        assertEquals(boolPropB, ((Boolean) value).booleanValue());

        setPropertyValueNoBroadcast(propBoolC, value);
        assertEquals(boolPropC, ((Boolean) value).booleanValue());

        // must fail because the member boolPropD is private
        try {
            setPropertyValueNoBroadcast(propBoolD, value);
            fail("com.sun.star.lang.WrappedTargetException expected");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            logger.log(Level.FINE, "com.sun.star.lang.WrappedTargetException caught");
        }
    }

    void resetPropertyMembers()
    {
        boolPropA= false;
        charPropA= (char) 0;
        bytePropA= 0;
        shortPropA= 0;
        intPropA= 0;
        longPropA= 0;
        floatPropA= 0;
        doublePropA= 0.;
        stringPropA= null;
        arBytePropA= null;
        typePropA= null;
        objectPropA= null;
        anyPropA= null;
        xInterfacePropA= null;
        xWeakPropA= null;
        enumPropertyState = com.sun.star.beans.PropertyState.DEFAULT_VALUE;
        boolPropB= false;
        boolPropC= false;
        boolClassProp= null;
        charClassProp= null;
        byteClassProp= null;
        shortClassProp= null;
        intClassProp= null;
        longClassProp= null;
        floatClassProp= null;
        doubleClassProp= null;
    }
}

class TestClass2 extends PropertySet
{

    public char charA;
    protected char charB;
    char charC;

    int intMemberA;

    public Character charClassA;
    protected Character charClassB;
    Character charClassC;

    void test_registerProperty1() throws Exception
    {
        registerProperty("PropChar", new Type(char.class), (short) 0, "PropChar");
        registerProperty("PropInt", new Type(int.class), (short) 0, "PropInt");
        registerProperty("PropString", new Type(String.class), (short) 0, "PropString");

        XPropertySetInfo info= getPropertySetInfo();
        Property[] props= info.getProperties();
        for (int j= 0; j < props.length; j++)
        {
           boolean result= false;
           Property aProp= props[j];
           if (aProp.Name.equals("PropChar") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               result= true;
           else if (aProp.Name.equals("PropInt") && aProp.Type.equals(new Type(int.class)) &&
                aProp.Attributes == 0)
               result= true;
           else if (aProp.Name.equals("PropString") && aProp.Type.equals(new Type(String.class)) &&
                aProp.Attributes == 0)
               result= true;
           assertTrue(result);
        }
    }

    void test_registerProperty2() throws Exception
    {
        System.out.println("registerProperty Test 2");

        registerProperty("charA", "charA", (short) 0);
        registerProperty("charB", "charB", (short) 0);
        registerProperty("charC", "charC", (short) 0);
        registerProperty("charClassB", "charClassB", PropertyAttribute.MAYBEVOID);
        registerProperty("IntProp", "intMemberA", (short) 0);

        XPropertySetInfo info= getPropertySetInfo();
        Property[] props= info.getProperties();
        for (int j= 0; j < props.length; j++)
        {
           boolean result= false;
           Property aProp= props[j];
           if (aProp.Name.equals("charA") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               result= true;
           else if (aProp.Name.equals("charB") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               result= true;
           else if (aProp.Name.equals("charC") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               result= true;
           else if (aProp.Name.equals("charClassB") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == PropertyAttribute.MAYBEVOID)
               result= true;
           else if (aProp.Name.equals("IntProp") && aProp.Type.equals(new Type(int.class)) &&
                aProp.Attributes == 0)
               result= true;
           assertTrue(result);
        }
        Object val= new Character('A');
        setPropertyValue("charA", val);
        assertEquals(val, getPropertyValue("charA"));
        setPropertyValue("charClassB",val);
        assertEquals(val, getPropertyValue("charClassB"));
        val= Integer.valueOf(111);
        setPropertyValue("IntProp",val);
        assertEquals(val, getPropertyValue("IntProp"));
    }
}

class util
{
    // An Object is considered an Any with TypeClass.VOID and no value.
    static boolean anyEquals(Object val1, Object val2)
    {
        Object obj1= null;
        Object obj2= null;
        Type t1= null;
        Type t2= null;
        if (val1 instanceof Any)
        {
            obj1= ((Any) val1).getObject();
            t1= ((Any) val1).getType();
        }
        else
            obj1= val1;

        if (val2 instanceof Any)
        {
            obj2= ((Any) val2).getObject();
            t2= ((Any) val2).getType();
        }
        else
            obj2= val2;

        if (obj1 != null && obj1.equals(obj2))
            return true;
        else if ((obj1 == null && obj2 == null) && t1 != null && t1.equals(t2))
            return true;
        return false;
    }

    // returns true if obj is an any that contains a void or interface type and the
    // object is null
    static boolean isVoidAny(Object obj)
    {
        boolean ret= false;
        if(obj instanceof Any)
        {
            Any a= (Any) obj;
            if( a.getType().getTypeClass().equals( TypeClass.INTERFACE)
                && a.getObject() == null) {
                ret= true;
            }
            else if( a.getType().equals( new Type(void.class)) && a.getObject() == null) {
                ret= true;
            }
        }
        return ret;
    }
}

class Listener implements XPropertyChangeListener, XVetoableChangeListener,
XPropertiesChangeListener
{
    int nChangeCalled;
    int nPropertiesChange;
    int nVetoCalled;
    boolean bVeto= false;
    PropertyChangeEvent evt;
    PropertyChangeEvent[] arEvt;
    // XPropertyChangeListener
    public void propertyChange(PropertyChangeEvent evt )
    {
        nChangeCalled++;
        this.evt= evt;
    }

    //VetoableChangeListener
    public void vetoableChange(PropertyChangeEvent evt ) throws PropertyVetoException
    {
        nVetoCalled++;
         this.evt= evt;
         if (bVeto)
             throw new PropertyVetoException();
    }

    public void disposing( /*IN*/EventObject Source )
    {
    }

    public void reset()
    {
        nChangeCalled= 0;
        nPropertiesChange= 0;
        nVetoCalled= 0;
        evt= null;
        arEvt= null;
        bVeto= false;
    }
    // XPropertiesChangeListener
    public void propertiesChange(PropertyChangeEvent[] propertyChangeEvent)
    {
        nPropertiesChange++;
        arEvt= propertyChangeEvent;
    }

}
