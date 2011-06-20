/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package ifc.style;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.container.XIndexReplace;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.xml.AttributeData;
import ifc.text._NumberingLevel;
import java.util.Enumeration;
import java.util.Hashtable;

import lib.MultiPropertyTest;
import lib.Status;
import share.LogWriter;


import util.utils;


/**
* Testing <code>com.sun.star.style.ParagraphProperties</code>
* service properties :
* <ul>
*  <li><code> ParaAdjust</code></li>
*  <li><code> ParaLineSpacing</code></li>
*  <li><code> ParaBackColor</code></li>
*  <li><code> ParaBackTransparent</code></li>
*  <li><code> ParaBackGraphicURL</code></li>
*  <li><code> ParaBackGraphicFilter</code></li>
*  <li><code> ParaBackGraphicLocation</code></li>
*  <li><code> ParaLastLineAdjust</code></li>
*  <li><code> ParaExpandSingleWord</code></li>
*  <li><code> ParaLeftMargin</code></li>
*  <li><code> ParaRightMargin</code></li>
*  <li><code> ParaTopMargin</code></li>
*  <li><code> ParaBottomMargin</code></li>
*  <li><code> ParaLineNumberCount</code></li>
*  <li><code> ParaLineNumberStartValue</code></li>
*  <li><code> ParaIsHyphenation</code></li>
*  <li><code> PageDescName</code></li>
*  <li><code> PageNumberOffset</code></li>
*  <li><code> ParaRegisterModeActive</code></li>
*  <li><code> ParaTabStops</code></li>
*  <li><code> ParaStyleName</code></li>
*  <li><code> DropCapFormat</code></li>
*  <li><code> DropCapWholeWord</code></li>
*  <li><code> ParaKeepTogether</code></li>
*  <li><code> ParaSplit</code></li>
*  <li><code> NumberingLevel</code></li>
*  <li><code> NumberingRules</code></li>
*  <li><code> NumberingStartValue</code></li>
*  <li><code> ParaIsNumberingRestart</code></li>
*  <li><code> NumberingStyleName</code></li>
*  <li><code> ParaOrphans</code></li>
*  <li><code> ParaWidows</code></li>
*  <li><code> ParaShadowFormat</code></li>
*  <li><code> IsHangingPunctuation</code></li>
*  <li><code> IsCharacterDistance</code></li>
*  <li><code> IsForbiddenRules</code></li>
*  <li><code> LeftBorder</code></li>
*  <li><code> RightBorder</code></li>
*  <li><code> TopBorder</code></li>
*  <li><code> BottomBorder</code></li>
*  <li><code> BorderDistance</code></li>
*  <li><code> LeftBorderDistance</code></li>
*  <li><code> RightBorderDistance</code></li>
*  <li><code> TopBorderDistance</code></li>
*  <li><code> BottomBorderDistance</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'NRULES'</code> : <b>optional</b>
*   (service <code>com.sun.star.text.NumberingRules</code>):
*   instance of the service which can be set as 'NumberingRules'
*   property new value. If the relation doesn't then two
*   different <code>NumberingRules</code> objects are tried
*   to be obtained by setting different 'NumberingStyleName'
*   property styles and getting 'NumberingRules' property values.</li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.ParagraphProperties
*/
public class _ParagraphProperties extends MultiPropertyTest {
    /**
     * Custom tester for numbering style properties. Switches between
     * 'Numbering 1' and 'Numbering 2' styles.
     */
    protected PropertyTester NumberingStyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                              throws java.lang.IllegalArgumentException {
            if ((oldValue != null) && (oldValue.equals("Numbering 1"))) {
                return "Numbering 2";
            } else {
                return "Numbering 1";
            }
        }
    };

    /**
     * Custom tester for paragraph style properties. Switches between
     * 'Salutation' and 'Heading' styles.
     */
    protected PropertyTester charStyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (!utils.isVoid(oldValue) && (oldValue.equals("Example"))) {
                return "Emphasis";
            } else {
                return "Example";
            }
        }
    };

    /**
     * Custom tester for paragraph style properties. Switches between
     * 'Salutation' and 'Heading' styles.
     */
    protected PropertyTester ParaStyleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (!utils.isVoid(oldValue) && (oldValue.equals("Heading"))) {
                return "Salutation";
            } else {
                return "Heading";
            }
        }
    };

    /**
     * Custom tester for PageDescName properties. Switches between
     * 'HTML' and 'Standard' descriptor names.
     */
    protected PropertyTester PageDescTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (!util.utils.isVoid(oldValue) &&
                    (oldValue.equals("Default"))) {
                return "HTML";
            } else {
                return "Default";
            }
        }
    };

    /**
     * Custom tester for properties which have <code>short</code> type
     * and can be void, so if they have void value, the new value must
     * be specified. Switches between two different values.
     */
    protected PropertyTester ShortTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if ((oldValue != null) &&
                    (oldValue.equals(new Short((short) 0)))) {
                return new Short((short) 2);
            } else {
                return new Short((short) 0);
            }
        }
    };

    /**
     * Custom tester for properties which have <code>boolean</code> type
     * and can be void, so if they have void value, the new value must
     * be specified. Switches between true and false.
     */
    protected PropertyTester BooleanTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if ((oldValue != null) &&
                    (oldValue.equals(new Boolean((boolean) false)))) {
                return new Boolean((boolean) true);
            } else {
                return new Boolean((boolean) false);
            }
        }
    };

    /**
     * Custom tester for properties which contains image URLs.
     * Switches between two JPG images' URLs.
     */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg"))) {
                return util.utils.getFullTestURL("crazy-blue.jpg");
            } else {
                return util.utils.getFullTestURL("space-metal.jpg");
            }
        }
    };

    protected PropertyTester rules = null;

    /**
     * Creates tester for 'NumberingRules' depending on relation.
     */
    public void before() {
        final Object nRules = tEnv.getObjRelation("NRULES");

        if (nRules != null) {
            rules = new PropertyTester() {
                protected Object getNewValue(String propName, Object oldValue) {
                    return nRules;
                }
            };

        } else {
            Object rules1 = null;
            Object rules2 = null;

            try {
                oObj.setPropertyValue("NumberingStyleName", "Numbering 1");
                rules1 = oObj.getPropertyValue("NumberingRules");
                oObj.setPropertyValue("NumberingStyleName", "Numbering 2");
                rules2 = oObj.getPropertyValue("NumberingRules");
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("WARNING !!! Exception getting numbering rules :");
                e.printStackTrace(log);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("WARNING !!! Exception getting numbering rules :");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.PropertyVetoException e) {
                log.println("WARNING !!! Exception getting numbering rules :");
                e.printStackTrace(log);
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Property 'NumberingStyleName' is not supported.");
            }

            rules = new PropertyValueSwitcher(rules1, rules2);
        }
    }

    /**
     * Tested with custom property tester.
     */
    public void _NumberingStyleName() {
        log.println("Testing with custom Property tester");
        testProperty("NumberingStyleName", NumberingStyleTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _DropCapCharStyleName() {
        log.println("Testing with custom Property tester");
        testProperty("DropCapCharStyleName", charStyleTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _ParaStyleName() {
        log.println("Testing with custom Property tester");
        testProperty("ParaStyleName", ParaStyleTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _PageDescName() {
        log.println("Testing with custom Property tester");
        testProperty("PageDescName", PageDescTester);
    }

    /**
     * Tested with custom property tester. Before testing property
     * <code>ParaAdjust</code> is setting to value <code>BLOCK</code>
     * because setting the property <code>ParaLastLineAdjust</code>
     * makes sense only in this case.
     */
    public void _ParaLastLineAdjust() {
        log.println("Testing with custom Property tester");

        try {
            oObj.setPropertyValue("ParaAdjust",
                                  com.sun.star.style.ParagraphAdjust.BLOCK);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred setting property 'ParagraphAdjust'" + e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occurred setting property 'ParagraphAdjust'" + e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occurred setting property 'ParagraphAdjust'" + e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occurred setting property 'ParagraphAdjust'" + e);
        }

        testProperty("ParaLastLineAdjust", ShortTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _ParaBackGraphicURL() {
        log.println("Testing with custom Property tester");
        testProperty("ParaBackGraphicURL", URLTester);
    }

    /**
     * Tested with custom property tester. <p>
     * The following property tests are to be completed successfully before :
     * <ul>
     *  <li> <code> NumberingStyleName </code> : a numbering style must
     *  be set before testing this property </li>
     * </ul>
     */
    public void _NumberingLevel() {
        requiredMethod("NumberingStyleName");
        log.println("Testing with custom Property tester");
        testProperty("NumberingLevel", ShortTester);
    }

    /**
     * Tested with custom property tester. <p>
     */
    public void _ParaIsConnectBorder() {

        log.println("Testing with custom Property tester");
        testProperty("ParaIsConnectBorder", BooleanTester);
    }

    /**
     * Tested with custom property tester.
     */
    public void _ParaVertAlignment() {
        log.println("Testing with custom Property tester");
        testProperty("ParaVertAlignment", ShortTester);
    }

    /**
     * Tested with com.sun.star.text.NumberingLevel <p>
     * The value of this property is a com.sun.star.container.XIndexReplace which is represneted by
     * com.sun.star.text.NumberingLevel.
     * The following property tests are to be completed successfully before :
     * <ul>
     *  <li> <code> NumberingStyleName </code> : a numbering style must
     *  be set before testing this property </li>
     * </ul>
     * @see com.sun.star.text.NumberlingLevel
     * @see com.sun.star.container.XIndexReplace
     * @see ifc.text._NumberingLevel
     */
    public void _NumberingRules() {
        requiredMethod("NumberingStyleName");

        XIndexReplace NumberingRules = null;
        PropertyValue[] propertyValues = null;
        try {
            NumberingRules = (XIndexReplace) AnyConverter.toObject(
                           new Type(XIndexReplace.class), oObj.getPropertyValue("NumberingRules"));
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            Status.failed( "could not get NumberingRuels: "+ ex.toString() );
            return;
        } catch (UnknownPropertyException ex) {
            Status.failed( "could not get NumberingRuels: "+ ex.toString() );
            return;
        } catch (WrappedTargetException ex) {
            Status.failed( "could not get NumberingRuels: "+ ex.toString() );
            return;
        }
        try {
            propertyValues = (PropertyValue[]) NumberingRules.getByIndex(0);

        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            Status.failed( "could not get NumberlingLevel-Array from NumberingRuels: "+ ex.toString() );
            return;
        } catch (WrappedTargetException ex) {
            Status.failed( "could not get NumberlingLevel-Array from NumberingRuels: "+ ex.toString() );
            return;
        }

        _NumberingLevel numb = new _NumberingLevel((LogWriter)log, tParam, propertyValues);

        boolean result = numb.testPropertieArray();

        tRes.tested("NumberingRules", result);
    }

    public void _ParaUserDefinedAttributes() {
        XNameContainer uda = null;
        boolean res = false;

        try {
            try{
                uda = (XNameContainer) AnyConverter.toObject(
                          new Type(XNameContainer.class),
                          oObj.getPropertyValue("ParaUserDefinedAttributes"));
            } catch (com.sun.star.lang.IllegalArgumentException e){
                log.println("ParaUserDefinedAttributes is empty.");
                uda = new _ParagraphProperties.OwnUserDefinedAttributes();
            }
            AttributeData attr = new AttributeData();
            attr.Namespace = "http://www.sun.com/staroffice/apitest/Cellprop";
            attr.Type = "CDATA";
            attr.Value = "true";
            uda.insertByName("Cellprop:has-first-alien-attribute", attr);

            String[] els = uda.getElementNames();
            oObj.setPropertyValue("ParaUserDefinedAttributes", uda);
            uda = (XNameContainer) AnyConverter.toObject(
                          new Type(XNameContainer.class),
                          oObj.getPropertyValue("ParaUserDefinedAttributes"));
            els = uda.getElementNames();

            Object obj = uda.getByName("Cellprop:has-first-alien-attribute");
            res = true;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            if (isOptional("ParaUserDefinedAttributes")) {
                log.println("Property is optional and not supported");
                res = true;
            } else {
                log.println("Don't know the Property 'ParaUserDefinedAttributes'");
            }
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println(
                    "WrappedTargetException while getting Property 'ParaUserDefinedAttributes'");
        } catch (com.sun.star.container.NoSuchElementException nee) {
            log.println("added Element isn't part of the NameContainer");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println(
                    "IllegalArgumentException while getting Property 'ParaUserDefinedAttributes'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println(
                    "PropertyVetoException while getting Property 'ParaUserDefinedAttributes'");
        } catch (com.sun.star.container.ElementExistException eee) {
            log.println(
                    "ElementExistException while getting Property 'ParaUserDefinedAttributes'");
        }

        tRes.tested("ParaUserDefinedAttributes", res);
    }

    private class OwnUserDefinedAttributes implements XNameContainer{
        Hashtable members = null;


        public OwnUserDefinedAttributes() {
            members = new Hashtable();
        }

        public Object getByName(String str) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException {
            return members.get(str);
        }

        public String[] getElementNames() {
            Enumeration oEnum = members.keys();
            int count = members.size();
            String[] res = new String[count];
            int i=0;
            while(oEnum.hasMoreElements())
                res[i] = (String)oEnum.nextElement();
            return res;
        }

        public com.sun.star.uno.Type getElementType() {
            Enumeration oEnum = members.keys();
            String key = (String)oEnum.nextElement();
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
} // finish class _ParagraphProperties
