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

import com.sun.star.form.XGridFieldDataSupplier;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.form.XGridFieldDataSupplier</code>
* interface methods :
* <ul>
*  <li><code>queryFieldDataType()</code></li>
*  <li><code>queryFieldData()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGridFieldDataSupplier
*/
public class _XGridFieldDataSupplier extends MultiMethodTest {
     public XGridFieldDataSupplier oObj = null;

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
     public void _queryFieldDataType() {
        boolean[] dataType = oObj.queryFieldDataType(new Type(String.class));
        tRes.tested("queryFieldDataType()", dataType != null);
     }

    /**
    * Calls the method and checks returned value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
     public void _queryFieldData() {
        Object[] data = oObj.queryFieldData(0, new Type(String.class));
        tRes.tested("queryFieldData()", data != null);
     }
}// finish class _XGridFieldDataSupplier

