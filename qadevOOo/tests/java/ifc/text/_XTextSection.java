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

import lib.MultiMethodTest;

import com.sun.star.text.XTextSection;

/**
 * Testing <code>com.sun.star.text.XTextSection</code>
 * interface methods :
 * <ul>
 *  <li><code> getParentSection()</code></li>
 *  <li><code> getChildSections()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextSection
 */
public class _XTextSection extends MultiMethodTest {

    public static XTextSection oObj = null;
    public XTextSection child = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns non zero legth array.
     */
    public void _getChildSections(){
        boolean bResult = false;
        XTextSection oSect[] = oObj.getChildSections();
        if (oSect.length > 0){
            bResult = true;
            child = oSect[0];
        }
        tRes.tested("getChildSections()",  bResult);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getParentSection(){
        requiredMethod("getChildSections()");
        boolean bResult = false;
        bResult = (child.getParentSection() != null);
        tRes.tested("getParentSection()",  bResult);
    }


}  // finish class _XTextSection

