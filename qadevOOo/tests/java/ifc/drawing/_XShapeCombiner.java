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
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeCombiner;
import com.sun.star.drawing.XShapes;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XShapeCombiner</code>
* interface methods :
* <ul>
*  <li><code> combine()</code></li>
*  <li><code> split()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (must implement <code>XShapes</code>):
*   the collection of shapes in a document which used to create a group.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XShapeCombiner
*/
public class _XShapeCombiner extends MultiMethodTest {

    public XShapeCombiner oObj = null;           //filled by MultiMethodTest
    protected XShape oGroup = null;
    int countBeforeComb = 0;
    XShapes oShapes = null;

    /**
    * Retrieves draw page collection from relation and combines them. <p>
    * Has <b> OK </b> status if the shape group returned is not null nd
    * number of shapes in collection is 1 (shapes are combined into a single
    * shape). <p>
    */
    public void _combine () {
        XDrawPage dp = (XDrawPage) tEnv.getObjRelation("DrawPage");
        oShapes = (XShapes)UnoRuntime.queryInterface( XShapes.class, dp );

        boolean result = false;

        log.println("testing combine() ... ");
        countBeforeComb = oShapes.getCount();
        log.println("Count before combining:" + countBeforeComb);
        oGroup = oObj.combine(oShapes);
        int countAfterComb = oShapes.getCount();
        log.println("Count after combining:" + countAfterComb);
        result = oGroup != null && countAfterComb == 1;

        tRes.tested("combine()", result);
    }

    /**
    * Splits the group created before. <p>
    *
    * Has <b> OK </b> status if number of shapes in collection after
    * <code>split</code> is the same as before <code>combine</code>. <p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> combine() </code> : to create a shape group </li>
    * </ul>
    */
    public void _split() {
        requiredMethod("combine()");

        boolean result = false;

        log.println("spiltting the shape...");

        oObj.split(oGroup);
        int countAfterSplit = oShapes.getCount();
        log.println("Count after split:" + countAfterSplit);
        result = countAfterSplit == countBeforeComb;

        tRes.tested("split()", result);
    } // end of split

} // end of XShapeCombiner

