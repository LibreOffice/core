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

package ifc.style;

import java.util.HashMap;
import java.util.Iterator;

import lib.MultiPropertyTest;
import util.ValueChanger;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.xml.AttributeData;


/**
* Testing <code>com.sun.star.style.CharacterProperties</code>
* service properties :
* <ul>
*  <li><code> CharFontName</code></li>
*  <li><code> CharFontStyleName</code></li>
*  <li><code> CharFontFamily</code></li>
*  <li><code> CharFontCharSet</code></li>
*  <li><code> CharFontPitch</code></li>
*  <li><code> CharColor</code></li>
*  <li><code> CharEscapement</code></li>
*  <li><code> CharHeight</code></li>
*  <li><code> CharUnderline</code></li>
*  <li><code> CharWeight</code></li>
*  <li><code> CharPosture</code></li>
*  <li><code> CharAutoKerning</code></li>
*  <li><code> CharBackColor</code></li>
*  <li><code> CharBackTransparent</code></li>
*  <li><code> CharCaseMap</code></li>
*  <li><code> CharCrossedOut</code></li>
*  <li><code> CharFlash</code></li>
*  <li><code> CharStrikeout</code></li>
*  <li><code> CharWordMode</code></li>
*  <li><code> CharKerning</code></li>
*  <li><code> CharLocale</code></li>
*  <li><code> CharKeepTogether</code></li>
*  <li><code> CharNoLineBreak</code></li>
*  <li><code> CharShadowed</code></li>
*  <li><code> CharFontType</code></li>
*  <li><code> CharStyleName</code></li>
*  <li><code> CharContoured</code></li>
*  <li><code> CharCombineIsOn</code></li>
*  <li><code> CharCombinePrefix</code></li>
*  <li><code> CharCombineSuffix</code></li>
*  <li><code> CharEmphasize</code></li>
*  <li><code> CharRelief</code></li>
*  <li><code> RubyText</code></li>
*  <li><code> RubyAdjust</code></li>
*  <li><code> RubyCharStyleName</code></li>
*  <li><code> RubyIsAbove</code></li>
*  <li><code> CharRotation</code></li>
*  <li><code> CharRotationIsFitToLine</code></li>
*  <li><code> CharScaleWidth</code></li>
*  <li><code> HyperLinkURL</code></li>
*  <li><code> HyperLinkTarget</code></li>
*  <li><code> HyperLinkName</code></li>
*  <li><code> TextUserDefinedAttributes</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'PARA'</code>: <b>optional</b>
*   (must implement <code>XPropertySet</code>):
*   if this relation is specified then some properties
*   testing is performed in a special manner. (e.g. this used in
*   <code>sw.SwXParagraph</code> component) For details
*   see {@link #changeProp} method description. </li>
*  <li> <code>'PORTION'</code>: <b>optional</b>
*   (must implement <code>XPropertySet</code>):
*   if this relation is specified then some properties
*   testing is performed in a special manner. (e.g. this used in
*   <code>sw.SwXParagraph</code> component) For details
*   see {@link #changeProp} method description. </li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.CharacterProperties
*/
public class _CharacterProperties extends MultiPropertyTest {

    public void _CharFontName() {
        testProperty("CharFontName", "Times New Roman", "Arial") ;
    }

    public void _CharHeight() {
        testProperty("CharHeight", new PropertyTester() {
            protected Object getNewValue(String propName, Object old) {
                if (utils.isVoid(old)) {
                    return new Float(10) ;
                } else {
                    return new Float(((Float) old).floatValue() + 10) ;
                }
            }
        }) ;
    }

    /**
     * Custom tester for properties which contains image URLs.
     * Switches between two JPG images' URLs.
     */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals("http://www.sun.com"))
                return "http://www.openoffice.org"; else
                return "http://www.sun.com";
        }
    } ;

    public void _HyperLinkURL() {
        testProperty("HyperLinkURL", URLTester) ;
    }

    public void _HyperLinkName() {
        testProperty("HyperLinkName", URLTester) ;
    }

    public void _HyperLinkTarget() {
        testProperty("HyperLinkTarget", URLTester) ;
    }

    public void _CharWeight() {
        testProperty("CharWeight", new Float(com.sun.star.awt.FontWeight.BOLD),
            new Float(com.sun.star.awt.FontWeight.THIN)) ;
    }

    public void _CharPosture() {
        testProperty("CharPosture", com.sun.star.awt.FontSlant.ITALIC,
            com.sun.star.awt.FontSlant.NONE) ;
    }

    /**
     * Custom tester for style name properties. If object relations "STYLENAME1"
     * and "STYLENAME2" exists, then testing with these strings, else switches
     * between 'Citation' and 'Emphasis' names.
     */
    protected PropertyTester StyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            String oStyleName1 = (String) tEnv.getObjRelation("STYLENAME1");
            String oStyleName2 = (String) tEnv.getObjRelation("STYLENAME2");
            if ((oStyleName1 != null) && (oStyleName2 != null)){
                log.println("use strings given by object relation: '"
                            + oStyleName1 + "' '" + oStyleName2 +"'");
                if (oldValue.equals( oStyleName1))
                    return oStyleName2;
                else
                    return oStyleName1;
            }
            if (utils.isVoid(oldValue) || (oldValue.equals("Standard")))
                return "Example"; else
                return "Emphasis";
        }
    } ;

    /**
     * Custom tester for style names properties. Switches between
     * 'Citation' and 'Emphasis' names.
     */
    protected PropertyTester StylesTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (utils.isVoid(oldValue) || (oldValue.equals("Standard")))
                return new String[] {"Citation"}; else
                return new String[] {"Emphasis"};
        }
    } ;

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _CharStyleName() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("CharStyleName", StyleTester) ;
        } else {
            changeProp((XPropertySet) oPara,
                (XPropertySet) oPort,"CharStyleName","Standard");
        }
    }

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _CharStyleNames() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("CharStyleNames", StylesTester) ;
        } else {
            changeProp((XPropertySet) oPara,
               (XPropertySet) oPort,"CharStyleNames",new String[] {"Standard"});
        }
    }

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _RubyCharStyleName() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("RubyCharStyleName", StyleTester) ;
        } else {
            changeProp((XPropertySet) oPara, (XPropertySet)
                oPort,"RubyCharStyleName","Standard");
        }
    }

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _RubyAdjust() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("RubyAdjust",new Short((short)0),new Short((short)1));
        } else {
            Short aShort = new Short((short) 1);
            changeProp((XPropertySet) oPara,
                (XPropertySet) oPort,"RubyAdjust", aShort);
        }
        new Short((short) 1);
    }

    /**
     * Custom tester for the ruby text property.
     */
    protected PropertyTester RubyTextTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (utils.isVoid(oldValue)) {
                return new String("RubyText");
            } else {
                return ValueChanger.changePValue(oldValue);
            }
        }
    } ;

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _RubyText() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("RubyText", RubyTextTester) ;
        } else {
            changeProp((XPropertySet) oPara, (XPropertySet) oPort,
                "RubyText","");
        }
    }

    /**
     * If relations for paragraph and portion exist, then testing
     * of this property performed using these objects, else
     * testing is performed in common way.
     */
    public void _RubyIsAbove() {
        log.println("Testing with custom Property tester") ;
        Object oPara = tEnv.getObjRelation("PARA");
        Object oPort = tEnv.getObjRelation("PORTION");
        if (oPara == null) {
            testProperty("RubyIsAbove") ;
        } else {
            changeProp((XPropertySet) oPara, (XPropertySet) oPort,
                "RubyIsAbove",new Boolean(true));
        }
    }

    /**
    * This property only takes values between 0..100
    * so ist must be treated special
    */
    public void _CharEscapementHeight() {
        Byte aByte = new Byte((byte)75);
        Byte max = new Byte((byte)100);
        testProperty("CharEscapementHeight", aByte, max) ;
    }


    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _CharRotation() {
        Short aShort = new Short((short) 10);
        changeProp(oObj,oObj, "CharRotation", aShort);
    }

    /**
     * Tests the property specified by <code>name</code> using
     * property set <code>oProps</code>, but value after setting
     * this property to a new value is checked using another
     * PropertySet <code>get</code>. Special cases used for
     * <code>CharRotation</code> property (it can have only certain values
     * 0, 900, ...), and for  <code>*StyleName</code> properties
     * (only existing style names are accepted)
     * @param oProps PropertySet from which property value is get
     * changed and set.
     * @param get PropertySet where property value is checked after
     * setting.
     * @param name Propety name to test.
     * @param newVal Value used to set as new property value if
     * the value get is null.
     */
    public void changeProp(XPropertySet oProps,
            XPropertySet get,String name, Object newVal) {

        Object gValue = null;
        Object sValue = null;
        Object ValueToSet = null;

        try {
            //waitForAllThreads();
            gValue = oProps.getPropertyValue(name);

            if ( (gValue == null) || (utils.isVoid(gValue)) ) {
                log.println("Value for "+name+" is NULL");
                gValue = newVal;
            }

            //waitForAllThreads();
            if (name.equals("CharRotation")) {
                Short s1 = new Short((short) 0);
                Short s2 = new Short((short) 900);
                if (gValue.equals(s1)) {
                    ValueToSet = s2;
                } else {
                    ValueToSet = s1;
                }
            } else {
                ValueToSet = ValueChanger.changePValue(gValue);
            }
            if (name.endsWith("StyleName")) {
                if ( ((String) gValue).equals("Standard") ) {
                    ValueToSet="Main index entry";
                }
                else {
                    ValueToSet="Emphasis";
                }
            }

            //waitForAllThreads();
            oProps.setPropertyValue(name,ValueToSet);
            sValue = get.getPropertyValue(name);
            if (sValue == null) {
                log.println("Value for "+name+" is NULL after setting");
                sValue = gValue;
            }

            //check get-set methods
            if (gValue.equals(sValue)) {
                log.println("Value for '"+name+"' hasn't changed");
                tRes.tested(name, false);
            } else {
                log.println("Property '"+name+"' OK");
                log.println("old: "+gValue.toString());
                log.println("new: "+ValueToSet.toString());
                log.println("result: "+sValue.toString());
                tRes.tested(name, true);
            }
        }
        catch (com.sun.star.beans.UnknownPropertyException ex) {
            if (isOptional(name)) {
                log.println("Property '"+name+
                    "' is optional and not supported");
                tRes.tested(name,true);
            }
            else {
                log.println("Exception occurred while testing property '" +
                     name + "'");
                ex.printStackTrace(log);
                tRes.tested(name, false);
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred while testing property '" +
                name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occurred while testing property '" +
                 name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occurred while testing property '" +
                 name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        }
    }// end of changeProp

    public void _TextUserDefinedAttributes() {
        XNameContainer uda = null;
        boolean res = false;

        try {
            try{
                uda = (XNameContainer) AnyConverter.toObject(
                          new Type(XNameContainer.class),
                          oObj.getPropertyValue("TextUserDefinedAttributes"));
            } catch (com.sun.star.lang.IllegalArgumentException e){
                log.println("TextUserDefinedAttributes is empty.");
                uda = new _CharacterProperties.OwnUserDefinedAttributes();
            }
            AttributeData attr = new AttributeData();
            attr.Namespace = "http://www.sun.com/staroffice/apitest/Cellprop";
            attr.Type = "CDATA";
            attr.Value = "true";
            uda.insertByName("Cellprop:has-first-alien-attribute", attr);

            uda.getElementNames();
            oObj.setPropertyValue("TextUserDefinedAttributes", uda);
            uda = (XNameContainer) AnyConverter.toObject(
                          new Type(XNameContainer.class),
                          oObj.getPropertyValue("TextUserDefinedAttributes"));
            uda.getElementNames();

            uda.getByName("Cellprop:has-first-alien-attribute");
            res = true;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            if (isOptional("TextUserDefinedAttributes")) {
                log.println("Property is optional and not supported");
                res = true;
            } else {
                log.println("Don't know the Property 'TextUserDefinedAttributes'");
            }
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println(
                    "WrappedTargetException while getting Property 'TextUserDefinedAttributes'");
        } catch (com.sun.star.container.NoSuchElementException nee) {
            log.println("added Element isn't part of the NameContainer");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println(
                    "IllegalArgumentException while getting Property 'TextUserDefinedAttributes'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println(
                    "PropertyVetoException while getting Property 'TextUserDefinedAttributes'");
        } catch (com.sun.star.container.ElementExistException eee) {
            log.println(
                    "ElementExistException while getting Property 'TextUserDefinedAttributes'");
        }

        tRes.tested("TextUserDefinedAttributes", res);
    }

    private class OwnUserDefinedAttributes implements XNameContainer{
        HashMap<String, Object> members = null;


        public OwnUserDefinedAttributes() {
            members = new HashMap<String, Object>();
        }

        public Object getByName(String str) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException {
            return members.get(str);
        }

        public String[] getElementNames() {
            Iterator<String> oEnum = members.keySet().iterator();
            int count = members.size();
            String[] res = new String[count];
            int i=0;
            while(oEnum.hasNext())
                res[i++] = oEnum.next();
            return res;
        }

        public com.sun.star.uno.Type getElementType() {
            Iterator<String> oEnum = members.keySet().iterator();
            String key = oEnum.next();
            Object o = members.get(key);
            return new Type(o.getClass());
        }

        public boolean hasByName(String str) {
            return members.get(str) != null;
        }

        public boolean hasElements() {
            return members.size() > 0;
        }

        public void insertByName(String str, Object obj) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.ElementExistException, com.sun.star.lang.WrappedTargetException {
            members.put(str, obj);
        }

        public void removeByName(String str) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException {
            members.remove(str);
        }

        public void replaceByName(String str, Object obj) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException {
            members.put(str, obj);
        }

    }

} //finish class _CharacterProperties

