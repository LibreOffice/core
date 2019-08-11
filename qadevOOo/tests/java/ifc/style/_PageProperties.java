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

import com.sun.star.container.XNameContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.xml.AttributeData;
import lib.MultiPropertyTest;

/**
 * Test page properties.
 * Testing is done by lib.MultiPropertyTest, except for properties
 * <ul>
 *  <li>PrinterPaperTray</li>
 *  <li>UserDefinedAttributes</li>
 * </ul>
 */
public class _PageProperties  extends MultiPropertyTest {

    /**
     * Switch on Header and Footer properties
     * so all props can be tested.
     */
    @Override
    protected void before() {
        try {
            oObj.setPropertyValue("HeaderIsOn", Boolean.TRUE);
            oObj.setPropertyValue("FooterIsOn", Boolean.TRUE);
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Don't know the Property 'HeaderIsOn' or 'FooterIsOn'");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("WrappedTargetException while setting Property 'HeaderIsOn' or 'FooterIsOn'");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("IllegalArgumentException while setting Property 'HeaderIsOn' or 'FooterIsOn'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println("PropertyVetoException while setting Property 'HeaderIsOn' or 'FooterIsOn'");
        }
    }

    /**
     * This property is system dependent and printer dependent.
     * So only reading it does make sense, since it cannot be determined, if
     * it is set to an allowed value.
     */
    public void _PrinterPaperTray() {
        boolean res = false;
        String setting = null;
        try {
            setting = (String)oObj.getPropertyValue("PrinterPaperTray");
            log.println("Property 'PrinterPaperTray' is set to '" + setting + "'.");
            res = setting != null;
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Don't know the Property 'PrinterPaperTray'");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("WrappedTargetException while getting Property 'PrinterPaperTray'");
        }
        tRes.tested("PrinterPaperTray", res);
    }

    /**
     * Create some valid user defined attributes
     */
    public void _UserDefinedAttributes() {
        XNameContainer uda = null;
        boolean res = false;
        try {
            uda = (XNameContainer) AnyConverter.toObject(
                new Type(XNameContainer.class),
                    oObj.getPropertyValue("UserDefinedAttributes"));
            AttributeData attr = new AttributeData();
            attr.Namespace = "http://www.sun.com/staroffice/apitest/Cellprop";
            attr.Type="CDATA";
            attr.Value="true";
            uda.insertByName("Cellprop:has-first-alien-attribute",attr);
            uda.getElementNames();
            oObj.setPropertyValue("UserDefinedAttributes",uda);
            uda = (XNameContainer) AnyConverter.toObject(
                new Type(XNameContainer.class),
                    oObj.getPropertyValue("UserDefinedAttributes"));
            uda.getElementNames();
            uda.getByName("Cellprop:has-first-alien-attribute");
            res = true;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Don't know the Property 'UserDefinedAttributes'");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("WrappedTargetException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.container.NoSuchElementException nee) {
            log.println("added Element isn't part of the NameContainer");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("IllegalArgumentException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println("PropertyVetoException while getting Property 'UserDefinedAttributes'");
        } catch (com.sun.star.container.ElementExistException eee) {
            log.println("ElementExistException while getting Property 'UserDefinedAttributes'");
        }
        tRes.tested("UserDefinedAttributes",res);
    }

}
