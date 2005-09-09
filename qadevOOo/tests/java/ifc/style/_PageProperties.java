/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _PageProperties.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:01:46 $
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
 *  <li>UserDefinedAttribures</li>
 * </ul>
 */
public class _PageProperties  extends MultiPropertyTest {

    /**
     * Switch on Header and Footer properties
     * so all props can be tested.
     */
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
     * it is set to a allowed value.
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
            String[] els = uda.getElementNames();
            oObj.setPropertyValue("UserDefinedAttributes",uda);
            uda = (XNameContainer) AnyConverter.toObject(
                new Type(XNameContainer.class),
                    oObj.getPropertyValue("UserDefinedAttributes"));
            els = uda.getElementNames();
            Object obj = uda.getByName("Cellprop:has-first-alien-attribute");
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
