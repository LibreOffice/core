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
import util.XInstCreator;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.uno.XInterface;


/**
* Testing <code>com.sun.star.drawing.XShapes</code>
* interface methods :
* <ul>
*  <li><code> add()</code></li>
*  <li><code> remove()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Shape'</code> (of type <code>XShape</code>):
*   a shape which can be inserted into shape collection. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XShapes
*/
public class _XShapes extends MultiMethodTest {

    public XShapes oObj = null;                // oObj filled by MultiMethodTest
    XInstCreator shape = null;
    XInterface oShape = null;


    /**
    * Retrieves a shape from relation and adds it to the collection.
    * Number of shapes is checked before and after adding.<p>
    * Has <b> OK </b> status if after adding number of shapes increases by
    * 1. <p>
    */
    public void _add () {

        boolean result = false;
        shape = (XInstCreator)tEnv.getObjRelation("Shape");
        oShape = shape.createInstance();
        XShape oSh = (XShape) oShape;

        log.println("testing add() ... ");

        int cntBefore = oObj.getCount();
        oObj.add(oSh);
        int cntAfter = oObj.getCount();
        result = cntBefore + 1 == cntAfter ;

        tRes.tested("add()", result);
    }

    /**
    * Removes the shape added before from the collection.
    * Number of shapes is checked before and after removing.<p>
    * Has <b> OK </b> status if after removing number of shapes decreases by
    * 1. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> add() </code> : a shape added. </li>
    * </ul>
    */
    public void _remove () {
        requiredMethod("add()");
        boolean result = false;

        log.println("removing the shape...");

        int cntBefore = oObj.getCount();
        oObj.remove((XShape) oShape);
        int cntAfter = oObj.getCount();
        result = cntBefore == cntAfter + 1;

        tRes.tested("remove()", result);
    }

}


