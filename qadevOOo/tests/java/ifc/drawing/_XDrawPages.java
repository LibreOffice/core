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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;

/**
* Testing <code>com.sun.star.drawing.XDrawPages</code>
* interface methods :
* <ul>
*  <li><code> insertNewByIndex()</code></li>
*  <li><code> remove()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XDrawPages
*/
public class _XDrawPages extends MultiMethodTest {
    public XDrawPages oObj = null;
    public XDrawPage DrawPage;

    /**
    * Inserts a draw page at index 0. Number of pages is obtained
    * before and after insertion.<p>
    * Has <b> OK </b> status if the value returned is not null and
    * number of pages increases by one after insertion. <p>
    */
    public void _insertNewByIndex(){
        boolean result = true;
        log.println("testing insertNewByIndex() ... ");

        int cntBefore = oObj.getCount();
        DrawPage = oObj.insertNewByIndex(0);
        int cntAfter = oObj.getCount();
        result = DrawPage != null &&
            cntAfter == cntBefore + 1 ;

        tRes.tested("insertNewByIndex()", result);
    } // end InsertNewByIndex

    /**
    * Removes the draw page inserted before. Number of pages is obtained
    * before and after removing.<p>
    * Has <b> OK </b> status if the number of pages decreases by one
    * after removing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertByIndex() </code> : the page to be removed must
    *   be inserted first.</li>
    * </ul>
    */
    public void _remove(){
        requiredMethod("insertNewByIndex()") ;

        boolean result = true;
        log.println("removing DrawPage...");

        int cntBefore = oObj.getCount();
        oObj.remove(DrawPage);
        int cntAfter = oObj.getCount();
        result = cntAfter + 1 == cntBefore ;

        tRes.tested("remove()", result);
    } // end remove()

} // end XDrawPages

