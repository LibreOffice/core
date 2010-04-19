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
* Test is <b> NOT </b> multithread compilant. <p>
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

