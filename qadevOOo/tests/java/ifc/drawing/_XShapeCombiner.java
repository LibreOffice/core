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
* Test is <b> NOT </b> multithread compliant. <p>
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
        oShapes = UnoRuntime.queryInterface( XShapes.class, dp );

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

