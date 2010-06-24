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

package ifc.table;

import lib.MultiPropertyTest;

import com.sun.star.container.XNameContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.xml.AttributeData;

/**
* Testing <code>com.sun.star.table.CellProperties</code>
* service properties :
* <ul>
*  <li><code> CellStyle</code></li>
*  <li><code> CellBackColor</code></li>
*  <li><code> IsCellBackgroundTransparent</code></li>
*  <li><code> NumberFormat</code></li>
*  <li><code> ShadowFormat</code></li>
*  <li><code> HoriJustify</code></li>
*  <li><code> VertJustify</code></li>
*  <li><code> Orientation</code></li>
*  <li><code> CellProtection</code></li>
*  <li><code> TableBorder</code></li>
*  <li><code> IsTextWrapped</code></li>
*  <li><code> RotateAngle</code></li>
*  <li><code> RotateReference</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.table.CellProperties
*/
public class _CellProperties extends MultiPropertyTest {


    /**
     * This property is tested with custom property tester which
     * switches between 'Default' and 'Result' style names.
     */
    public void _CellStyle() {
        testProperty("CellStyle", new PropertyTester() {
            protected Object getNewValue(String name, Object old) {
                return "Default".equals(old) ? "Result" : "Default" ;
            }
        }) ;
    }

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

} // finish class _CellProperties

