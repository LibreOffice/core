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

public class PropertySet_Test
{

    private static final Logger logger = Logger.getLogger(PropertySet_Test.class.getName());

    public boolean convertPropertyValue()
    {
        logger.log(Level.INFO, "PropertySet.convertPropertyValue");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
        r[i++]= cl.test_convertPropertyValue();
        }catch(java.lang.Exception e){
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean setPropertyValueNoBroadcast()
    {
        logger.log(Level.INFO, "PropertySet.setValueNoBroadcast");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            r[i++]= cl.test_setPropertyValueNoBroadcast();
        }catch(java.lang.Exception e){
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean setPropertyValue()
    {
        logger.log(Level.INFO, "PropertySet.setPropertyValue");
        boolean[] r= new boolean[150];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            cl.resetPropertyMembers();
            Object value;
            Object ret;
            value= Boolean.TRUE;
            cl.setPropertyValue("PropBoolA", value);
            ret=  cl.getPropertyValue("PropBoolA");
            r[i++]= ((Boolean) ret).equals( value);
            value= new Character('A');
            cl.setPropertyValue("PropCharA",value);
            ret= cl.getPropertyValue("PropCharA");
            r[i++]= ((Character) ret).equals(value);
            value= Byte.valueOf((byte) 111);
            cl.setPropertyValue("PropByteA",value);
            ret= cl.getPropertyValue("PropByteA");
            r[i++]= ((Byte) ret).equals(value);
            value= Short.valueOf((short)112);
            cl.setPropertyValue("PropShortA", value);
            ret= cl.getPropertyValue("PropShortA");
            r[i++]= ((Short) ret).equals(value);
            value= Integer.valueOf(113);
            cl.setPropertyValue("PropIntA", value);
            ret= cl.getPropertyValue("PropIntA");
            r[i++]= ((Integer) ret).equals(value);
            value= Long.valueOf(115);
            cl.setPropertyValue("PropLongA", value);
            ret= cl.getPropertyValue("PropLongA");
            r[i++]= ((Long) ret).equals(value);
            value= new Float(3.14);
            cl.setPropertyValue("PropFloatA", value);
            ret= cl.getPropertyValue("PropFloatA");
            r[i++]= ((Float) ret).equals(value);
            value= new Double(3.145);
            cl.setPropertyValue("PropDoubleA",value);
            ret= cl.getPropertyValue("PropDoubleA");
            r[i++]= ((Double) ret).equals(value);
            value= "string";
            cl.setPropertyValue("PropStringA",value);
            ret= cl.getPropertyValue("PropStringA");
            r[i++]= ((String) ret).equals(value);
            value= new ComponentBase();
            cl.setPropertyValue("PropXInterfaceA",value);
            ret= cl.getPropertyValue("PropXInterfaceA");
            r[i++]= ((XInterface) ret).equals(value);
            value= new ComponentBase();
            cl.setPropertyValue("PropXWeakA",value);
            ret= cl.getPropertyValue("PropXWeakA");
            r[i++]= ((XWeak) ret).equals(value);
            value = com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE;
            cl.setPropertyValue("PropEnum",value);
            ret= cl.getPropertyValue("PropEnum");
            r[i++]= ret == value;
            value= new byte[]{1,2,3};
            cl.setPropertyValue("PropArrayByteA", value);
            ret= cl.getPropertyValue("PropArrayByteA");
            r[i++]= ((byte[]) ret).equals(value);
            value= new Type(String.class);
            cl.setPropertyValue("PropTypeA", value);
            ret= cl.getPropertyValue("PropTypeA");
            r[i++]= ((Type) ret).equals(value);

            // test protected,package,private members
            value= Boolean.TRUE;
            cl.setPropertyValue("PropBoolB", value);
            ret= cl.getPropertyValue("PropBoolB");
            r[i++]= ((Boolean) ret).equals(value);
            cl.setPropertyValue("PropBoolC", value);
            ret= cl.getPropertyValue("PropBoolC");
            r[i++]= ((Boolean) ret).equals(value);

            try{
                cl.setPropertyValue("PropBoolD", value);
                i++;
            }catch (Exception e)
            {
                r[i++]= true;
            }
            cl.resetPropertyMembers();

            value= Boolean.TRUE;
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Boolean) ret).equals(value);
            value= new Character('A');
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Character) ret).equals(value);
            value= Byte.valueOf((byte) 111);
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Byte) ret).equals(value);
            value= Short.valueOf((short)112);
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Short) ret).equals(value);
            value= Integer.valueOf(113);
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Integer) ret).equals(value);
            value= Long.valueOf(115);
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Long) ret).equals(value);
            value= new Float(3.14);
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Float) ret).equals(value);
            value= new Double(3.145);
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Double) ret).equals(value);
            value= "string";
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((String) ret).equals(value);
            value= new ComponentBase();
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((XInterface) ret).equals(value);
            value= new ComponentBase();
            cl.setPropertyValue("PropObjectA",value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((XWeak) ret).equals(value);
            value= new byte[]{1,2,3};
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((byte[]) ret).equals(value);
            value= new Type(String.class);
            cl.setPropertyValue("PropObjectA", value);
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Type) ret).equals(value);
            cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), Byte.valueOf((byte)1)));
            ret= cl.getPropertyValue("PropObjectA");
            r[i++]= ((Byte) ret).byteValue() == 1;

           cl.resetPropertyMembers();
            value= Boolean.TRUE;
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Character('A');
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= Byte.valueOf((byte) 111);
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= Short.valueOf((short)112);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= Integer.valueOf(113);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= Long.valueOf(115);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Float(3.14);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Double(3.145);
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= "string";
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new ComponentBase();
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new ComponentBase();
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new byte[]{1,2,3};
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Type(String.class);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);

            cl.resetPropertyMembers();
            value= new Any(new Type(boolean.class), Boolean.TRUE);
            cl.setPropertyValue("PropBoolA", value);
            ret= cl.getPropertyValue("PropBoolA");
            r[i++]= ret instanceof Boolean && util.anyEquals(value, ret);
            value= new Any (new Type(char.class), new Character('A'));
            cl.setPropertyValue("PropCharA",value);
            ret= cl.getPropertyValue("PropCharA");
            r[i++]= ret instanceof Character && util.anyEquals(value, ret);
            value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
            cl.setPropertyValue("PropByteA",value);
            ret= cl.getPropertyValue("PropByteA");
            r[i++]= ret instanceof Byte && util.anyEquals(value, ret);
            value= new Any(new Type(short.class), Short.valueOf((short)112));
            cl.setPropertyValue("PropShortA", value);
            ret= cl.getPropertyValue("PropShortA");
            r[i++]= ret instanceof Short && util.anyEquals(value, ret);
            value= new Any(new Type(int.class), Integer.valueOf(113));
            cl.setPropertyValue("PropIntA", value);
            ret= cl.getPropertyValue("PropIntA");
            r[i++]= ret instanceof Integer && util.anyEquals(value, ret);
            value= new Any(new  Type(long.class), Long.valueOf(115));
            cl.setPropertyValue("PropLongA", value);
            ret= cl.getPropertyValue("PropLongA");
            r[i++]= ret instanceof Long && util.anyEquals(value, ret);
            value= new Any(new Type(float.class), new Float(3.14));
            cl.setPropertyValue("PropFloatA", value);
            ret= cl.getPropertyValue("PropFloatA");
            r[i++]= ret instanceof Float && util.anyEquals(value, ret);
            value= new Any(new Type(double.class),new Double(3.145));
            cl.setPropertyValue("PropDoubleA",value);
            ret= cl.getPropertyValue("PropDoubleA");
            r[i++]= ret instanceof Double && util.anyEquals(value, ret);
            value= new Any(new Type(String.class), "string");
            cl.setPropertyValue("PropStringA",value);
            ret= cl.getPropertyValue("PropStringA");
            r[i++]= ret instanceof String && util.anyEquals(value, ret);
            value= new Any(new Type(ComponentBase.class), new ComponentBase());
            cl.setPropertyValue("PropXInterfaceA",value);
            ret= cl.getPropertyValue("PropXInterfaceA");
            r[i++]= ret instanceof ComponentBase && util.anyEquals(value, ret);
            value= new Any( new Type(ComponentBase.class), new ComponentBase());
            cl.setPropertyValue("PropXWeakA",value);
            ret= cl.getPropertyValue("PropXWeakA");
            r[i++]= ret instanceof ComponentBase && util.anyEquals(value, ret);
            value= new Any(new Type(byte[].class), new byte[]{1,2,3});
            cl.setPropertyValue("PropArrayByteA", value);
            ret= cl.getPropertyValue("PropArrayByteA");
            r[i++]= ret instanceof byte[] && util.anyEquals(value, ret);
            value= new Any(new Type(Type.class), new Type(String.class));
            cl.setPropertyValue("PropTypeA", value);
            ret= cl.getPropertyValue("PropTypeA");
            r[i++]= ret instanceof Type && util.anyEquals(value, ret);


            cl.resetPropertyMembers();
            value= new Any(new Type(boolean.class), Boolean.TRUE);
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any (new Type(char.class), new Character('A'));
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(short.class), Short.valueOf((short)112));
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(int.class), Integer.valueOf(113));
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new  Type(long.class), Long.valueOf(115));
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(float.class), new Float(3.14));
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(double.class),new Double(3.145));
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(String.class), "string");
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(ComponentBase.class), new ComponentBase());
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any( new Type(ComponentBase.class), new ComponentBase());
            cl.setPropertyValue("PropAnyA",value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(byte[].class), new byte[]{1,2,3});
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);
            value= new Any(new Type(Type.class), new Type(String.class));
            cl.setPropertyValue("PropAnyA", value);
            ret= cl.getPropertyValue("PropAnyA");
            r[i++]= ret instanceof Any && util.anyEquals(value, ret);



            cl.resetPropertyMembers();
            value= Boolean.TRUE;
            cl.setPropertyValue("PropBoolClass", value);
            ret=  cl.getPropertyValue("PropBoolClass");
            r[i++]= ((Boolean) ret).equals( value);
            value= new Character('A');
            cl.setPropertyValue("PropCharClass",value);
            ret= cl.getPropertyValue("PropCharClass");
            r[i++]= ((Character) ret).equals(value);
            value= Byte.valueOf((byte) 111);
            cl.setPropertyValue("PropByteClass",value);
            ret= cl.getPropertyValue("PropByteClass");
            r[i++]= ((Byte) ret).equals(value);
            value= Short.valueOf((short)112);
            cl.setPropertyValue("PropShortClass", value);
            ret= cl.getPropertyValue("PropShortClass");
            r[i++]= ((Short) ret).equals(value);
            value= Integer.valueOf(113);
            cl.setPropertyValue("PropIntClass", value);
            ret= cl.getPropertyValue("PropIntClass");
            r[i++]= ((Integer) ret).equals(value);
            value= Long.valueOf(115);
            cl.setPropertyValue("PropLongClass", value);
            ret= cl.getPropertyValue("PropLongClass");
            r[i++]= ((Long) ret).equals(value);
            value= new Float(3.14);
            cl.setPropertyValue("PropFloatClass", value);
            ret= cl.getPropertyValue("PropFloatClass");
            r[i++]= ((Float) ret).equals(value);
            value= new Double(3.145);
            cl.setPropertyValue("PropDoubleClass",value);
            ret= cl.getPropertyValue("PropDoubleClass");
            r[i++]= ((Double) ret).equals(value);

            cl.resetPropertyMembers();

            cl.resetPropertyMembers();
            value= new Any(new Type(boolean.class), Boolean.TRUE);
            cl.setPropertyValue("PropBoolClass", value);
            ret= cl.getPropertyValue("PropBoolClass");
            r[i++]= ret instanceof Boolean && util.anyEquals(value, ret);
            value= new Any (new Type(char.class), new Character('A'));
            cl.setPropertyValue("PropCharClass",value);
            ret= cl.getPropertyValue("PropCharClass");
            r[i++]= ret instanceof Character && util.anyEquals(value, ret);
            value= new Any(new Type(byte.class), Byte.valueOf((byte) 111));
            cl.setPropertyValue("PropByteClass",value);
            ret= cl.getPropertyValue("PropByteClass");
            r[i++]= ret instanceof Byte && util.anyEquals(value, ret);
            value= new Any(new Type(short.class), Short.valueOf((short)112));
            cl.setPropertyValue("PropShortClass", value);
            ret= cl.getPropertyValue("PropShortClass");
            r[i++]= ret instanceof Short && util.anyEquals(value, ret);
            value= new Any(new Type(int.class), Integer.valueOf(113));
            cl.setPropertyValue("PropIntClass", value);
            ret= cl.getPropertyValue("PropIntClass");
            r[i++]= ret instanceof Integer && util.anyEquals(value, ret);
            value= new Any(new  Type(long.class), Long.valueOf(115));
            cl.setPropertyValue("PropLongClass", value);
            ret= cl.getPropertyValue("PropLongClass");
            r[i++]= ret instanceof Long && util.anyEquals(value, ret);
            value= new Any(new Type(float.class), new Float(3.14));
            cl.setPropertyValue("PropFloatClass", value);
            ret= cl.getPropertyValue("PropFloatClass");
            r[i++]= ret instanceof Float && util.anyEquals(value, ret);
            value= new Any(new Type(double.class),new Double(3.145));
            cl.setPropertyValue("PropDoubleClass",value);
            ret= cl.getPropertyValue("PropDoubleClass");
            r[i++]= ret instanceof Double && util.anyEquals(value, ret);
            value= new Any(new Type(String.class), "string");



            // PropertyAttribute.READONLY
            cl.propBoolA.Attributes= PropertyAttribute.READONLY;
            try{
                cl.setPropertyValue("PropBoolA", Boolean.TRUE);
                i++;
            }catch (com.sun.star.beans.PropertyVetoException e)
            {
                r[i++]= true;
            }
            cl.propBoolA.Attributes= 0;

            // MAYBEVOID
            cl.resetPropertyMembers();
            // first MAYBEVOID not set

            //primitive members: must not work

            cl.boolPropA= false;
            try {
                cl.setPropertyValue("PropBoolA", null); i++;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                r[i++]= true;
            }
            try {
                cl.setPropertyValue("PropBoolA", new Any(new Type(boolean.class), null)); i++;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                r[i++]= true;
            }
            cl.propBoolA.Attributes= PropertyAttribute.MAYBEVOID;
            try{
                cl.setPropertyValue("PropBoolA", null); i++;
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                r[i++]= true;
            }
            cl.propBoolA.Attributes= 0;

            cl.propBoolClass.Attributes= PropertyAttribute.MAYBEVOID;
            cl.boolClassProp= null;
            cl.setPropertyValue("PropBoolClass", null);
            r[i++]= cl.boolClassProp == null;
            // the returned value must be a void any
            Object objAny= cl.getPropertyValue("PropBoolClass");
            r[i++]= util.isVoidAny( objAny);

            cl.boolClassProp= Boolean.TRUE;
            cl.setPropertyValue("PropBoolClass", null);
            r[i++]= cl.boolClassProp == null;
            cl.boolClassProp= Boolean.FALSE;
            cl.setPropertyValue("PropBoolClass", new Any(new Type(boolean.class),null));
            r[i++]= cl.boolClassProp == null;

            cl.propXWeakA.Attributes= PropertyAttribute.MAYBEVOID;
            cl.setPropertyValue("PropXWeakA", null);
            r[i++]= util.isVoidAny(cl.getPropertyValue("PropXWeakA"));
            cl.propXWeakA.Attributes= 0;

            cl.anyPropA= null;
            try{
                cl.setPropertyValue("PropAnyA", null); i++;
            }catch (com.sun.star.lang.IllegalArgumentException e) {
                r[i++]= true;
            }
            cl.anyPropA= null;
            cl.propAnyA.Attributes= PropertyAttribute.MAYBEVOID;

            Type _t= new Type(Object.class);
            cl.setPropertyValue("PropAnyA", null);
            r[i++]= cl.anyPropA.getType().equals(new Type(void.class)) &&
                    cl.anyPropA.getObject() == null;

            cl.anyPropA= new Any(new Type(byte.class),Byte.valueOf((byte) 111));
            cl.setPropertyValue("PropAnyA", null);
            r[i++]= cl.anyPropA.getType().equals(new Type(byte.class)) &&
                    cl.anyPropA.getObject() == null;

            cl.anyPropA= null;
            try{
                cl.setPropertyValue("PropAnyA", new Object()); i++;
            }catch (com.sun.star.lang.IllegalArgumentException e)
            {
                r[i++]= true;
            }

            cl.propObjectA.Attributes= 0;
            try{
                cl.setPropertyValue("PropObjectA", null); i++;
            }catch (com.sun.star.lang.IllegalArgumentException e)
            {
                r[i++]= true;
            }
            try{
                cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), null)); i++;
            } catch (com.sun.star.lang.IllegalArgumentException e)
            {
                r[i++]= true;
            }

            cl.propObjectA.Attributes= PropertyAttribute.MAYBEVOID;
            cl.propObjectA= null;
            cl.setPropertyValue("PropObjectA", null);
            r[i++]= cl.propObjectA == null;

            cl.propObjectA= null;
            cl.setPropertyValue("PropObjectA", new Any( new Type(byte.class), null));
            r[i++]= cl.propObjectA == null;




        }catch(java.lang.Exception e){
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean addPropertyChangeListener()
    {
        logger.log(Level.INFO, "PropertySet.addPropertyChangeListener,\n" +
                               "PropertySet.removePropertChangeListener," +
                               "PropertySet.addVetoableChangeListener, \n" +
                               "PropertySet.removeVetoableChangeListener" +
                               "Notification of listeners");
        boolean[] r= new boolean[50];
        int i= 0;
        try {
            TestClass cl= new TestClass();
            Listener li= new Listener();
            cl.addPropertyChangeListener("PropByteA", li);
            Byte val1= Byte.valueOf((byte)115);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0;
            cl.propByteA.Attributes = PropertyAttribute.BOUND;
            cl.addPropertyChangeListener("PropByteA", li);
            Byte val2= Byte.valueOf((byte)116);
            cl.setPropertyValue("PropByteA", val2);
            r[i++]= li.nChangeCalled == 1 && li.nVetoCalled == 0;
            r[i++]= li.evt.OldValue.equals(val1) && li.evt.NewValue.equals(val2) && li.evt.Source == cl;

            li.reset();
            Listener li2= new Listener();
            cl.addPropertyChangeListener("PropByteA", li2);
            Byte val3= Byte.valueOf((byte) 117);
            cl.setPropertyValue("PropByteA", val3);
            r[i++]= li.nChangeCalled == 1 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 1 && li2.nVetoCalled == 0;
            r[i++]= li.evt.OldValue.equals(val2) && li.evt.NewValue.equals(val3) && li.evt.Source == cl
                    && li2.evt.OldValue.equals(val2) && li2.evt.NewValue.equals(val3) && li2.evt.Source == cl ;

            li.reset();
            li2.reset();
            Listener li3= new Listener();
            val1= Byte.valueOf((byte)118);
            cl.addPropertyChangeListener("", li3);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 1 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 1 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 1 && li3.nVetoCalled == 0;
            r[i++]= li.evt.OldValue.equals(val3) && li.evt.NewValue.equals(val1) && li.evt.Source == cl;
            r[i++]= li2.evt.OldValue.equals(val3) && li2.evt.NewValue.equals(val1) && li2.evt.Source == cl;
            r[i++]= li3.evt.OldValue.equals(val3) && li3.evt.NewValue.equals(val1) && li3.evt.Source == cl ;

            li.reset();
            li2.reset();
            li3.reset();
            cl.removePropertyChangeListener("PropByteA",li);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 1 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 1 && li3.nVetoCalled == 0;
            cl.removePropertyChangeListener("PropByteA", li2);
            li.reset();
            li2.reset();
            li3.reset();
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 1 && li3.nVetoCalled == 0;

            cl.removePropertyChangeListener("", li3);
            li.reset();
            li2.reset();
            li3.reset();
            cl.setPropertyValue("PropByteA", val2);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 0 && li3.nVetoCalled == 0;

            cl.addPropertyChangeListener("PropByteA", li);
            cl.addPropertyChangeListener("PropByteA", li2);
            cl.addPropertyChangeListener("", li3);
            cl.dispose();
            li.reset();
            li2.reset();
            li3.reset();
            try {
                cl.setPropertyValue("PropByteA", val2); i++;
            }catch (DisposedException e)
            {
                r[i++]= true;
            }

            //Vetoable tests
            cl= new TestClass();
            li.reset();
            li2.reset();
            li3.reset();
            cl.addVetoableChangeListener("PropByteA", li);
            val1= Byte.valueOf((byte)115);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0;
            cl.propByteA.Attributes = PropertyAttribute.CONSTRAINED;
            cl.addVetoableChangeListener("PropByteA", li);
            val2= Byte.valueOf((byte)116);
            li.reset();
            cl.setPropertyValue("PropByteA", val2);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 1;
            r[i++]= li.evt.OldValue.equals(val1) && li.evt.NewValue.equals(val2) && li.evt.Source == cl;

            li.reset();
            li2.reset();
            li3.reset();
            cl.addVetoableChangeListener("PropByteA", li2);
            val3= Byte.valueOf((byte) 117);
            cl.setPropertyValue("PropByteA", val3);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 1
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 1;
            r[i++]= li.evt.OldValue.equals(val2) && li.evt.NewValue.equals(val3) && li.evt.Source == cl
                    && li2.evt.OldValue.equals(val2) && li2.evt.NewValue.equals(val3) && li2.evt.Source == cl ;

            li.reset();
            li2.reset();
            li3.reset();
            val1= Byte.valueOf((byte)118);
            cl.addVetoableChangeListener("", li3);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 1
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 1
                    &&  li3.nChangeCalled == 0 && li3.nVetoCalled == 1;
            r[i++]= li.evt.OldValue.equals(val3) && li.evt.NewValue.equals(val1) && li.evt.Source == cl;
            r[i++]= li2.evt.OldValue.equals(val3) && li2.evt.NewValue.equals(val1) && li2.evt.Source == cl;
            r[i++]= li3.evt.OldValue.equals(val3) && li3.evt.NewValue.equals(val1) && li3.evt.Source == cl ;

            li.reset();
            li2.reset();
            li3.reset();
            // Test Veto Exception
            cl.setPropertyValue("PropByteA", val1);
            li.bVeto= true;
            try {
                cl.setPropertyValue("PropByteA", val2);i++;
            } catch (PropertyVetoException e)
            {
                r[i++]= true;
            }
            r[i++]= cl.bytePropA == val1.byteValue();
            li.bVeto= false;

            li.reset();
            li2.reset();
            li3.reset();
            cl.removeVetoableChangeListener("PropByteA",li);
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 1
                    &&  li3.nChangeCalled == 0 && li3.nVetoCalled == 1;
            cl.removeVetoableChangeListener("PropByteA", li2);
            li.reset();
            li2.reset();
            li3.reset();
            cl.setPropertyValue("PropByteA", val1);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 0 && li3.nVetoCalled == 1;

            cl.removeVetoableChangeListener("", li3);
            li.reset();
            li2.reset();
            li3.reset();
            cl.setPropertyValue("PropByteA", val2);
            r[i++]= li.nChangeCalled == 0 && li.nVetoCalled == 0
                    && li2.nChangeCalled == 0 && li2.nVetoCalled == 0
                    &&  li3.nChangeCalled == 0 && li3.nVetoCalled == 0;

            cl.addVetoableChangeListener("PropByteA", li);
            cl.addVetoableChangeListener("PropByteA", li2);
            cl.addVetoableChangeListener("", li3);
            cl.dispose();
            li.reset();
            li2.reset();
            li3.reset();
            try {
                cl.setPropertyValue("PropByteA", val2);
            }catch (DisposedException e)
            {
                r[i++]= true;
            }
        }catch (Exception e)
        {
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean getPropertySetInfo()
    {
        logger.log(Level.INFO, "PropertySet.getPropertySetInfo");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            XPropertySetInfo info= cl.getPropertySetInfo();
            Property[] arProps= info.getProperties();
            Property[] arRegProps= cl.getRegisteredProperties();
            r[i++]= arProps.length == arRegProps.length;
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
                if ( !bFound)
                    r[i++]= false;
            }

            for (int j= 0; j < arRegProps.length; j++)
            {
                Property prop= info.getPropertyByName(arRegProps[j].Name);
                if (prop != arRegProps[j])
                    r[i++]= false;
                if (! info.hasPropertyByName(arRegProps[j].Name))
                    r[i++]= false;
            }


        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean setFastPropertyValue()
    {
        logger.log(Level.INFO, "PropertySet.setFastPropertyValue");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            cl.setFastPropertyValue(5, Integer.valueOf(111));
            r[i++]= cl.intPropA == 111;
            try {
            cl.setFastPropertyValue(-1, Integer.valueOf(1)); i++;
            } catch(UnknownPropertyException e)
            {
                r[i++]= true;
            }
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean getFastPropertyValue()
    {
        logger.log(Level.INFO, "PropertySet.setFastPropertyValue");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            cl.setFastPropertyValue(5, Integer.valueOf(111));
            Integer aInt= (Integer) cl.getFastPropertyValue(5);
            r[i++]= aInt.intValue() == 111;
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean setPropertyValues()
    {
        logger.log(Level.INFO, "PropertySet.setPropertyValues");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            cl.setPropertyValues(new String[0], new Object[0]);
            String[] arNames= new String[] {"PropCharA","PropIntClass","PropObjectA"};
            Character aChar= new Character('A');
            Integer aInt= Integer.valueOf(111);
            Byte aByte= Byte.valueOf((byte)11);
            Object[] values= new Object[]{aChar, aInt, aByte};
            cl.setPropertyValues(arNames, values);
            r[i++]= cl.charPropA == 'A' && cl.intClassProp.intValue() == 111 && ((Byte)cl.objectPropA).byteValue() == 11;

            arNames= new String[] {"blabla","PropIntClass","PropObjectA"};
            cl.resetPropertyMembers();
            cl.setPropertyValues(arNames, values);
            r[i++]= cl.intClassProp.intValue() == 111 && ((Byte)cl.objectPropA).byteValue() == 11;
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean getPropertyValues()
    {
        logger.log(Level.INFO, "PropertySet.getPropertyValues");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            cl.charPropA= 'A';
            cl.intClassProp= Integer.valueOf(111);
            cl.objectPropA= Byte.valueOf((byte)11);
            Object[] values= cl.getPropertyValues(new String[] {"PropCharA","PropIntClass","PropObjectA"});

            r[i++]= ((Character) values[0]).charValue() == 'A' && ((Integer) values[1]).intValue() == 111
                        && ((Byte) values[2]).byteValue() == 11;
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    // Currently the listeners are always notified if one of properties has changed.
    // The property names in the first argument are ignored.
    public boolean addPropertiesChangeListener()
    {
        logger.log(Level.INFO, "PropertySet.addPropertiesChangeListener\n" +
                               "PropertySet.removePropertiesChangeListener\n" +
                               "notification of such listeners");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            Listener li1= new Listener();
            Listener li2= new Listener();
            cl.addPropertiesChangeListener(new String[]{"PropCharA"}, li1);
            cl.setPropertyValue("PropCharA", new Character('B'));
            r[i++]= li1.nPropertiesChange == 0;
            cl.propCharA.Attributes= PropertyAttribute.BOUND;
            cl.setPropertyValue("PropCharA", new Character('C'));
            r[i++]= li1.nPropertiesChange == 1;

            PropertyChangeEvent evt= li1.arEvt[0];
            r[i++]= evt.PropertyName.equals("PropCharA") && ((Character)evt.OldValue).charValue() == 'B'
                    && ((Character) evt.NewValue).charValue() == 'C';
            li1.reset();
            cl.removePropertiesChangeListener(li1);
            cl.setPropertyValue("PropCharA", new Character('F'));
            r[i++]= li1.nPropertiesChange == 0;
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean firePropertiesChangeEvent()
    {
        logger.log(Level.INFO, "PropertySet.firePropertiesChangeEvent");
        boolean[] r= new boolean[50];
        int i= 0;

        TestClass cl= new TestClass();
        try {
            Listener li1= new Listener();
            cl.intClassProp= Integer.valueOf(111);
            cl.charPropA= 'A';
            cl.firePropertiesChangeEvent(new String[]{"PropCharA","PropIntClass"},  li1);
            r[i++]= li1.nPropertiesChange == 1;
            PropertyChangeEvent[] arEvt= li1.arEvt;
            r[i++]= arEvt[0].PropertyName.equals("PropCharA")
                    && ((Character) arEvt[0].OldValue).charValue() == 'A'
                    && ((Character) arEvt[0].NewValue).charValue() == 'A';
            r[i++]= arEvt[1].PropertyName.equals("PropIntClass")
                    && ((Integer) arEvt[1].OldValue).intValue() == 111
                    && ((Integer) arEvt[1].NewValue).intValue() == 111;
        }catch(java.lang.Exception e){
            System.out.println(e.getMessage());
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public boolean registerProperty1()
    {
        TestClass2 cl= new TestClass2();
        return cl.test_registerProperty1();
    }

    public boolean registerProperty2()
    {
        TestClass2 cl= new TestClass2();
        return cl.test_registerProperty2();
    }

    public static boolean test()
    {
        PropertySet_Test test= new PropertySet_Test();
        boolean r[]= new boolean[50];
        int i= 0;
        r[i++]= test.convertPropertyValue();
        r[i++]= test.setPropertyValueNoBroadcast();
        r[i++]= test.setPropertyValue();
        r[i++]= test.addPropertyChangeListener();
        r[i++]= test.getPropertySetInfo();
        r[i++]= test.setFastPropertyValue();
        r[i++]= test.getFastPropertyValue();
        r[i++]= test.setPropertyValues();
        r[i++]= test.getPropertyValues();
        r[i++]= test.addPropertiesChangeListener();
        r[i++]= test.firePropertiesChangeEvent();
        r[i++]= test.registerProperty1();
        r[i++]= test.registerProperty2();
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        logger.log(Level.INFO, bOk ? "Ok" : "Failed");
        return bOk;
    }

    public static void main(String[] args)
    {
        System.exit( test() ? 0: -1 );
    }
}

class TestClass extends PropertySet
{
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
//        When adding properties then modify the getRegisteredProperties method
        //registerProperty(String name, int handle, Type type, short attributes, String memberName)
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
    public boolean test_convertPropertyValue()
    {
        boolean[] r= new boolean[150];
        int i= 0;

        resetPropertyMembers();
        Object[] outOldVal= new Object[1];
        Object[] outNewVal= new Object[1];

        Object value= Boolean.TRUE;
        try
        {
            r[i++]= convertPropertyValue(propBoolA,  outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Boolean && outNewVal[0].equals(value)  && outOldVal[0].equals(Boolean.FALSE);
            value= new Character('A');
            r[i++]= convertPropertyValue(propCharA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Character && outNewVal[0].equals(value) && outOldVal[0].equals(new Character((char)0));
            charPropA= 'B';
            r[i++]= convertPropertyValue(propCharA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Character && outNewVal[0].equals(value) && outOldVal[0].equals(new Character('B'));
            value= Byte.valueOf((byte) 111);
            r[i++]= convertPropertyValue(propByteA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Byte && outNewVal[0].equals(value);
            value= Short.valueOf((short) 112);
            r[i++]= convertPropertyValue(propShortA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Short && outNewVal[0].equals(value);
            value= Integer.valueOf( 113);
            r[i++]= convertPropertyValue(propIntA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Integer && outNewVal[0].equals(value);
            value= Long.valueOf(114);
            r[i++]= convertPropertyValue(propLongA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Long && outNewVal[0].equals(value);
            value= new Float(3.14);
            r[i++]= convertPropertyValue(propFloatA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Float && outNewVal[0].equals(value);
            value= new Double(3.145);
            r[i++]= convertPropertyValue(propDoubleA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Double && outNewVal[0].equals(value);
            value= "string";
            r[i++]= convertPropertyValue(propStringA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof String && outNewVal[0].equals(value);
            value= new byte[]{1,2,3};
            arBytePropA= null;
            r[i++]= convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof byte[] && outNewVal[0].equals(value) && outOldVal[0] == null;
            r[i++]= convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value);
            r[i++]= outOldVal[0] == null;
            value= new Type(XInterface.class);
            r[i++]= convertPropertyValue(propTypeA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Type && outNewVal[0].equals(value);

            value= new Object(); // TypeClass.VOID
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0].equals(value);
            value= Integer.valueOf(111);
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Integer && outNewVal[0].equals(value);
            value= new ComponentBase();
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0].equals(value);
            value= Integer.valueOf(111);
            r[i++]= convertPropertyValue(propAnyA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Any && ((Any)outNewVal[0]).getType().equals(new Type(Integer.class))
                    && ((Any)outNewVal[0]).getObject().equals(value);
            XWeak oWeak= new ComponentBase();
            value= oWeak;
            // The returned Any must contain an XInterface
            r[i++]= convertPropertyValue(propAnyA, outNewVal, outOldVal, value);
            r[i++]= ((Any) outNewVal[0]).getType().equals(new Type(XInterface.class))
                    && ((Any) outNewVal[0]).getObject() == oWeak;
            value= new ComponentBase();
            r[i++]= convertPropertyValue(propXInterfaceA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof XInterface && outNewVal[0].equals(value);
            r[i++]= convertPropertyValue(propXWeakA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof XWeak && outNewVal[0].equals(value);

            value = com.sun.star.beans.PropertyState.DIRECT_VALUE;
            r[i++]= convertPropertyValue(propEnum, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof com.sun.star.uno.Enum && outNewVal[0].equals(value);

            // Any arguments ------------------------------------------------------------------
            value= new Any( new Type(Integer.class),Integer.valueOf(111));
            r[i++]= convertPropertyValue(propIntA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Integer && ((Integer)outNewVal[0]).equals( ((Any)value).getObject());
            value= new Any(new Type(Boolean.class), Boolean.TRUE);
            r[i++]= convertPropertyValue(propBoolA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Boolean && ((Boolean)outNewVal[0]).equals(((Any) value).getObject());
            //Character, Byte, Short, Long
            // must fail
            value= new Any(new Type(Object.class), new Object());
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= convertPropertyValue(propAnyA, outNewVal, outOldVal, value);
            value= new Any(new Type(Integer.class), Integer.valueOf(111));
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Integer && outNewVal[0].equals( ((Any)value).getObject());
            r[i++]= convertPropertyValue(propAnyA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Any && ((Any) outNewVal[0]).getType().equals( ((Any) value).getType())
                    && ((Any) outNewVal[0]).getObject().equals( ((Any) value).getObject());
            value= new Any(new Type(XInterface.class), new ComponentBase());
            r[i++]= convertPropertyValue(propObjectA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof XInterface && outNewVal[0] == ((Any) value).getObject();
            r[i++]= convertPropertyValue(propXInterfaceA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] == ((Any) value).getObject();
            value= new Any(new Type(byte[].class), new byte[]{1,2,3});
            r[i++]= convertPropertyValue(propArrayByteA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof byte[];



            // test private, protected, package fields
            value= Boolean.TRUE;
            r[i++]= convertPropertyValue(propBoolB, outNewVal, outOldVal, value);
            r[i++]= ((Boolean)value).booleanValue() == ((Boolean) outNewVal[0]).booleanValue();
            r[i++]= convertPropertyValue(propBoolC, outNewVal,  outOldVal, value);
            r[i++]= ((Boolean)value).booleanValue() == ((Boolean) outNewVal[0]).booleanValue();
            // must fail because the member boolPropD is private
            try{
                convertPropertyValue(propBoolD, outNewVal,  outOldVal, value);
                i++;
            }catch (Exception e)
            {
                r[i++]= true;
            }

            // Properties member of type Byte,Short etc.
            value= Boolean.TRUE;
            r[i++]= convertPropertyValue(propBoolClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Boolean && outNewVal[0].equals(value);
            value= new Character('A');
            r[i++]= convertPropertyValue(propCharClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Character && outNewVal[0].equals(value);
            value= Byte.valueOf((byte) 111);
            r[i++]= convertPropertyValue(propByteClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Byte && outNewVal[0].equals(value);
            value= Short.valueOf((short) 112);
            r[i++]= convertPropertyValue(propShortClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Short && outNewVal[0].equals(value);
            value= Integer.valueOf( 113);
            r[i++]= convertPropertyValue(propIntClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Integer && outNewVal[0].equals(value);
            value= Long.valueOf(114);
            r[i++]= convertPropertyValue(propLongClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Long && outNewVal[0].equals(value);
            value= new Float(3.14);
            r[i++]= convertPropertyValue(propFloatClass, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Float && outNewVal[0].equals(value);
            value= new Double(3.145);
            r[i++]= convertPropertyValue(propDoubleA, outNewVal, outOldVal, value);
            r[i++]= outNewVal[0] instanceof Double && outNewVal[0].equals(value);
        }catch (com.sun.star.uno.Exception e)
        {
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];

        return bOk;
    }

    public boolean test_setPropertyValueNoBroadcast()
    {
        boolean[] r= new boolean[150];
        int i= 0;
        resetPropertyMembers();
        try {
            Object value= Boolean.TRUE;
            setPropertyValueNoBroadcast(propBoolA, value);
            r[i++]= boolPropA == ((Boolean) value).booleanValue();
            value= new Character('A');
            setPropertyValueNoBroadcast(propCharA, value);
            r[i++]= charPropA == ((Character) value).charValue();
            value= Byte.valueOf((byte) 111);
            setPropertyValueNoBroadcast(propByteA, value);
            r[i++]= bytePropA == ((Byte)value).byteValue();
            value= Short.valueOf((short) 112);
            setPropertyValueNoBroadcast(propShortA, value);
            r[i++]= shortPropA == ((Short) value).shortValue();
            value= Integer.valueOf( 113);
            setPropertyValueNoBroadcast(propIntA, value);
            r[i++]= intPropA == ((Integer) value).intValue();
            value= Long.valueOf(114);
            setPropertyValueNoBroadcast(propLongA, value);
            r[i++]= longPropA == ((Long) value).longValue();
            value= new Float(3.14);
            setPropertyValueNoBroadcast(propFloatA,  value);
            r[i++]= floatPropA == ((Float) value).floatValue();
            value= new Double(3.145);
            setPropertyValueNoBroadcast(propDoubleA, value);
            r[i++]= doublePropA == ((Double) value).doubleValue();
            value= "string";
            setPropertyValueNoBroadcast(propStringA, value);
            r[i++]= stringPropA.equals(value);
            value= new byte[]{1,2,3};
            setPropertyValueNoBroadcast(propArrayByteA, value);
            r[i++]= arBytePropA.equals(value);
            value= new Type(XInterface.class);
            setPropertyValueNoBroadcast(propTypeA, value);
            r[i++]= typePropA.equals(value);
            value= Integer.valueOf(111);
            setPropertyValueNoBroadcast(propObjectA, value);
            r[i++]= objectPropA.equals(value);
            value= new ComponentBase();
            setPropertyValueNoBroadcast(propObjectA, value);
            r[i++]= objectPropA.equals(value);
            value= new Any( new Type(Integer.TYPE), Integer.valueOf(111));
            setPropertyValueNoBroadcast(propAnyA, value);
            r[i++]= util.anyEquals(anyPropA, value);
            value= new ComponentBase();
            setPropertyValueNoBroadcast(propXInterfaceA, value);
            r[i++]= xInterfacePropA instanceof XInterface && xInterfacePropA.equals(value);
            setPropertyValueNoBroadcast(propXWeakA, value);
            r[i++]= xInterfacePropA instanceof XWeak && xInterfacePropA.equals(value);
            value = com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE;
            setPropertyValueNoBroadcast(propEnum, value);
            r[i++]= enumPropertyState == value;
            value= Boolean.TRUE;
            setPropertyValueNoBroadcast(propBoolB, value);
            r[i++]= boolPropB == ((Boolean) value).booleanValue();
            setPropertyValueNoBroadcast(propBoolC, value);
            r[i++]= boolPropC == ((Boolean) value).booleanValue();
            // must fail because the member boolPropD is private
            try{
            setPropertyValueNoBroadcast(propBoolD, value);
            }catch(com.sun.star.lang.WrappedTargetException e)
            {
                r[i++]= true;
            }
        }catch (java.lang.Exception e)
        {
            i++;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        return bOk;
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

    boolean test_registerProperty1()
    {
        boolean r[]= new boolean[50];
        int i= 0;

        registerProperty("PropChar", new Type(char.class), (short) 0, "PropChar");
        registerProperty("PropInt", new Type(int.class), (short) 0, "PropInt");
        registerProperty("PropString", new Type(String.class), (short) 0, "PropString");

        XPropertySetInfo info= getPropertySetInfo();
        Property[] props= info.getProperties();
        for (int j= 0; j < props.length; j++)
        {
           Property aProp= props[j];
           if (aProp.Name.equals("PropChar") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else if (aProp.Name.equals("PropInt") && aProp.Type.equals(new Type(int.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else if (aProp.Name.equals("PropString") && aProp.Type.equals(new Type(String.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else
               r[i++]= false;
        }

        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (!bOk)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
    }

    boolean test_registerProperty2()
    {
        System.out.println("registerProperty Test 2");
        boolean r[]= new boolean[50];
        int i= 0;

        registerProperty("charA", "charA", (short) 0);
        registerProperty("charB", "charB", (short) 0);
        registerProperty("charC", "charC", (short) 0);
        registerProperty("charClassB", "charClassB", PropertyAttribute.MAYBEVOID);
        registerProperty("IntProp", "intMemberA", (short) 0);

        XPropertySetInfo info= getPropertySetInfo();
        Property[] props= info.getProperties();
        for (int j= 0; j < props.length; j++)
        {
           Property aProp= props[j];
           if (aProp.Name.equals("charA") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else if (aProp.Name.equals("charB") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else if (aProp.Name.equals("charC") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else if (aProp.Name.equals("charClassB") && aProp.Type.equals(new Type(char.class)) &&
                aProp.Attributes == PropertyAttribute.MAYBEVOID)
               r[i++]= true;
           else if (aProp.Name.equals("IntProp") && aProp.Type.equals(new Type(int.class)) &&
                aProp.Attributes == 0)
               r[i++]= true;
           else
               r[i++]= false;
        }
        Object ret;
        Object val= new Character('A');
        try{
            setPropertyValue("charA", val);
            ret= getPropertyValue("charA");
            r[i++]= val.equals(ret);
            setPropertyValue("charClassB",val);
            ret= getPropertyValue("charClassB");
            r[i++]= val.equals(ret);
            val= Integer.valueOf(111);
            setPropertyValue("IntProp",val);
            ret= getPropertyValue("IntProp");
            r[i++]= val.equals(ret);
        }
        catch(Exception e)
        {
            r[i++]=false;
        }
        boolean bOk= true;
        for (int c= 0; c < i; c++)
            bOk= bOk && r[c];
        if (!bOk)
            System.out.println("Failed");
        else
            System.out.println("Ok");
        return bOk;
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
        if( obj != null && obj instanceof Any)
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
    int nDisposingCalled;
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
        nDisposingCalled++;
    }

    public void reset()
    {
        nChangeCalled= 0;
        nPropertiesChange= 0;
        nVetoCalled= 0;
        nDisposingCalled= 0;
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
