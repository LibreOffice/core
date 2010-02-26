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
* Test is <b> NOT </b> multithread compilant. <p>
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


