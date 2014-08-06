/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
    @Override
    protected void after() {
        disposeEnvironment();
    }
} // finish class _XSubTotalDescriptor
