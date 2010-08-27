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
package ifc.sheet;

import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSubTotalDescriptor;

import lib.MultiMethodTest;


/**
* Testing <code>com.sun.star.sheet.XSubTotalDescriptor</code>
* interface methods :
* <ul>
*  <li><code> addNew()</code></li>
*  <li><code> clear()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSubTotalDescriptor
*/
public class _XSubTotalDescriptor extends MultiMethodTest {
    public XSubTotalDescriptor oObj = null;

    /**
    * Test creates a subtotal field definition and adds it to the descriptor.<p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _addNew() {
        SubTotalColumn[] columns = new SubTotalColumn[1];
        SubTotalColumn column = new SubTotalColumn();
        column.Column = 5;
        column.Function = GeneralFunction.SUM;
        columns[0] = column;
        oObj.addNew(columns, 1);
        tRes.tested("addNew()", true);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _clear() {
        oObj.clear();
        tRes.tested("clear()", true);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
} // finish class _XSubTotalDescriptor
