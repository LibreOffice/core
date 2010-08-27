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

package ifc.document;


import lib.MultiMethodTest;

import com.sun.star.document.XMimeTypeInfo;
import com.sun.star.lang.XComponent;

/**
 * Testing <code>com.sun.star.document.XMimeTypeInfo</code>
 * interface methods :
 * <ul>
 *  <li><code> supportsMimeType()</code></li>
 *  <li><code> getSupportedMimeTypeNames()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.document.XMimeTypeInfo
 */
public class _XMimeTypeInfo extends MultiMethodTest {

    public XMimeTypeInfo oObj = null;
    public XComponent source = null ;
    public String[] smi = null;

    /**
    * Gets supported types and stores them. <p>
    * Has <b> OK </b> status if at least one type exists.
    */
    public void _getSupportedMimeTypeNames() {
        smi = oObj.getSupportedMimeTypeNames();
        tRes.tested("getSupportedMimeTypeNames()", smi.length>0) ;
    }

    /**
     * Calls the method for one supported type retrieved by
     * <code>getSupportedMimeTypeNames</code> method and for
     * bad type. <p>
     *
     * Has <b> OK </b> status if <code>true</code> returned for
     * supported type and <code>false</code> for bad type.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedMimeTypeNames </code> : to have a list of
     *    supported types. </li>
     * </ul>
     */
    public void _supportsMimeType() {
        requiredMethod("getSupportedMimeTypeNames()");
        boolean pos = false;
        pos = oObj.supportsMimeType(smi[0]);
        if (!pos) {
            log.println("Method returns false for existing MimeType");
        }
        boolean neg = true;
        neg = oObj.supportsMimeType("NoRealMimeType");
        if (neg) {
            log.println("Method returns true for non existing MimeType");
        }
        tRes.tested("supportsMimeType()", (pos && !neg)) ;
    }
}


