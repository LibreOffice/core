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
* Test is <b> NOT </b> multithread compliant. <p>
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

        oShapes = UnoRuntime.queryInterface( XShapes.class, dp );
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



