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
import lib.Status;
import lib.StatusException;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeGroup;
import com.sun.star.drawing.XShapeGrouper;
import com.sun.star.drawing.XShapes;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XShapeGrouper</code>
* interface methods :
* <ul>
*  <li><code> group()</code></li>
*  <li><code> ungroup()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (must implement <code>XShapes</code>):
*   the collection of shapes in a document which used to create a group.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XShapeGrouper
*/
public class _XShapeGrouper extends MultiMethodTest {

    public XShapeGrouper oObj = null;                // oObj filled by MultiMethodTest
    XShape oGroup = null;
    int countAfterGr = 0;
    XShapes oShapes = null;

    /**
    * Retrieves draw page collection from relation and groups them. <p>
    * Has <b> OK </b> status if the shape group returned is not null. <p>
    */
    public void _group() {
        Object dp = tEnv.getObjRelation("DrawPage");
        if (dp == null)
            throw new StatusException(Status.failed("Relation not found")) ;

        oShapes = (XShapes)UnoRuntime.queryInterface( XShapes.class, dp );
        boolean result = false;
        log.println("Grouping " + oShapes.getCount() + " shapes ... ");

        int countBeforeGr = oShapes.getCount();
        oGroup = oObj.group(oShapes);
        countAfterGr = oShapes.getCount();
        log.println("Number of shapes after grouping: " + countAfterGr);
        result = oGroup != null ;
        result &= countAfterGr < countBeforeGr;

        tRes.tested("group()", result);
    }

    /**
    * Ungroups the group created before. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> group() </code> : to create a shape group </li>
    * </ul>
    */
    public void _ungroup() {
        requiredMethod("group()");
        boolean result = false;
        log.println("ungrouping the shape...");

        oObj.ungroup((XShapeGroup)oGroup);
        int countAfterUnGr = oShapes.getCount();
        log.println("Number of shapes after ungrouping: " + countAfterUnGr);

        result = countAfterUnGr != countAfterGr;

        tRes.tested("ungroup()", result);
    }
}



