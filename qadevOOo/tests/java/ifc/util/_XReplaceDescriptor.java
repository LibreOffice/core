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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XReplaceDescriptor;

/**
 * Testing <code>com.sun.star.util.XReplaceDescriptor</code>
 * interface methods :
 * <ul>
 *  <li><code> getReplaceString()</code></li>
 *  <li><code> setReplaceString()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XReplaceDescriptor
 */
public class _XReplaceDescriptor extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XReplaceDescriptor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getReplaceString(){
        log.println("test for getReplaceString() ");
        tRes.tested("getReplaceString()", oObj.getReplaceString()!=null);
    }

    /**
     * Set a new string and checks the result. <p>
     * Has <b> OK </b> status if the string before setting differs
     * from string after setting. <p>
     */
    public void _setReplaceString(){
        log.println("test for setReplaceString() ");
        String oldReplaceStr = oObj.getReplaceString();
        oObj.setReplaceString("XReplaceDescriptor");
        String cmpReplaceStr = oObj.getReplaceString();
        tRes.tested("setReplaceString()", !(cmpReplaceStr.equals(oldReplaceStr)));
        return;
    }

}  // finish class _XReplaceDescriptor


