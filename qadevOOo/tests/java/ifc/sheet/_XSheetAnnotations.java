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
* Test is <b> NOT </b> multithread compliant. <p>
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


