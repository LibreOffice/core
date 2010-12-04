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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.form.XGridColumnFactory;


/**
* Testing <code>com.sun.star.form.XGridColumnFactory</code>
* interface methods :
* <ul>
*  <li><code> createColumn()</code></li>
*  <li><code> getColumnTypes()</code></li>
* </ul>
* Test is multithread compilant. <p>
* @see com.sun.star.form.XGridColumnFactory
*/
public class _XGridColumnFactory extends MultiMethodTest {

    public XGridColumnFactory oObj = null;


    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns
    * non null value.
    */
    public void _getColumnTypes() {

        String[] types = oObj.getColumnTypes();
        tRes.tested("getColumnTypes()",types!=null);
    }

    /**
    * Creates column of type 'TextField'. <p>
    * Has <b>OK</b> status if non null value returned.
    */
    public void _createColumn() {

        try {
            XPropertySet column = oObj.createColumn("TextField");
            tRes.tested("createColumn()",column!=null);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while creating Column");
            tRes.tested("createColumn()",false);
        }
    }
}  // finish class _XGridColumnFactory



