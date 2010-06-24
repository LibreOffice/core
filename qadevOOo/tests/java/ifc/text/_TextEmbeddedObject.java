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
package ifc.text;

import lib.MultiPropertyTest;


/**
 * Testing <code>com.sun.star.text.TextEmbeddedObject</code>
 * service properties :
 * <ul>
 *  <li><code> CLSID</code></li>
 *  <li><code> Model</code></li>
 *  <li><code> Component</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextEmbeddedObject
 */
public class _TextEmbeddedObject extends MultiPropertyTest {

    /**
     * This property can't be set, as soon as the object is inserted
     * so the set method will be skipped.
     */
    public void _CLSID() {
        boolean result = false;

        try {
            String clsid = (String) oObj.getPropertyValue("CLSID");
            log.println("Getting " + clsid);
            log.println(
                    "According to the idl-description this property can't be set as soon as the Object is inserted in the document, so setting will be skipped");
            result=true;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while getting Property 'CLSID' " +
                        e.getMessage());
        }

        tRes.tested("CLSID", result);
    }
} // finish class _TextEmbeddedObject
