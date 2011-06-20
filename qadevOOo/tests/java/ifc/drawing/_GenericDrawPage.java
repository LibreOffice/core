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

package ifc.drawing;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.drawing.GenericDrawPage</code>
* service properties :
* <ul>
*  <li><code> BorderBottom</code></li>
*  <li><code> BorderLeft</code></li>
*  <li><code> BorderRight</code></li>
*  <li><code> BorderTop</code></li>
*  <li><code> Height</code></li>
*  <li><code> Width</code></li>
*  <li><code> Number</code></li>
*  <li><code> Orientation</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.GenericDrawPage
*/
public class _GenericDrawPage extends MultiPropertyTest {

    public void _Number() {
        log.println("Number started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Short wat = (Short) oObj.getPropertyValue("Number");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occurred");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occurred");
        }

        tRes.tested("Number",res);
    }

    public void _UserDefinedAttributes() {
        log.println("Userdefined started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Object wat = oObj.getPropertyValue("UserDefinedAttributes");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occurred");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occurred");
        }

        tRes.tested("UserDefinedAttributes",res);
    }

} // end of GenericDrawPage

