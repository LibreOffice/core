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

import lib.MultiMethodTest;

import com.sun.star.sheet.XSheetAnnotations;
import com.sun.star.table.CellAddress;


/**
* Testing <code>com.sun.star.sheet.XSheetAnnotations</code>
* interface methods :
* <ul>
*  <li><code> insertNew()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.sheet.XSheetAnnotations
*/
public class _XSheetAnnotations extends MultiMethodTest {

    public XSheetAnnotations oObj = null;

    /**
    * Adds two new annotations into collection. <p>
    * Has <b>OK</b> status if the number of elements in collection
    * increased by 2 after method call.
    */
    public void _insertNew(){
        boolean bResult = false;

        int initialAmount = oObj.getCount();
        String sAnno = oObj.toString();

        oObj.insertNew(new CellAddress((short)1, 2, 5), sAnno + "1");
        oObj.insertNew(new CellAddress((short)1, 1, 1), sAnno + "2");

        bResult = (oObj.getCount() == 2 + initialAmount);
        tRes.tested("insertNew()", bResult);
    }

    /**
    * Removes one annotation from collection. <p>
    * Has <b>OK</b> status if the number of elements in collection
    * decreased after method call. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertNew </code> : to be sure at least two elements
    *   exist in the collection.</li>
    * </ul>
    */
    public void _removeByIndex(){
        requiredMethod("insertNew()");
        int tmpCnt = oObj.getCount();

        oObj.removeByIndex(1);
        int newCnt = oObj.getCount();

        tRes.tested("removeByIndex()", newCnt < tmpCnt);
    }

} // EOC _XSheetAnnotations


