/*************************************************************************
 *
 *  $RCSfile: _XShapeCombiner.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:32:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

