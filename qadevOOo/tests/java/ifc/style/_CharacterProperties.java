/*************************************************************************
 *
 *  $RCSfile: _CharacterProperties.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:07:54 $
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

package ifc.style;

import lib.MultiPropertyTest;
import util.ValueChanger;
import util.utils;

import com.sun.star.beans.XPropertySet;


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
     * Custom tester for style name properties. Switches between
     * 'Citation' and 'Emphasis' names.
     */
    protected PropertyTester StyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (utils.isVoid(oldValue) || (oldValue.equals("Standard")))
                return "Citation"; else
                return "Emphasis";
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
                log.println("Exception occured while testing property '" +
                     name + "'");
                ex.printStackTrace(log);
                tRes.tested(name, false);
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while testing property '" +
                name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while testing property '" +
                 name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occured while testing property '" +
                 name + "'");
            e.printStackTrace(log);
            tRes.tested(name, false);
        }
    }// end of changeProp

} //finish class _CharacterProperties

