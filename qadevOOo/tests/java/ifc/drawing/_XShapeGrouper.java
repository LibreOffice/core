/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XShapeGrouper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:43:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



