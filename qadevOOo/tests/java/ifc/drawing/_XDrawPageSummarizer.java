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

import com.sun.star.drawing.XDrawPageSummarizer;
import com.sun.star.drawing.XDrawPages;

/**
* Testing <code>com.sun.star.drawing.XDrawPageSummarizer</code>
* interface methods :
* <ul>
*  <li><code> summarize()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPages'</code> (of type <code>XDrawPages</code>):
*   a collection of pages to be summarized. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawPageSummarizer
*/
public class _XDrawPageSummarizer extends MultiMethodTest {
    public XDrawPageSummarizer oObj = null;

    /**
    * First the relation is retrieved. Then the method is called.<p>
    * Has <b> OK </b> status if the method returns not null value. <p>
    */
    public void summarize(){
        boolean result = true;
        XDrawPages oDrawPages = (XDrawPages) tEnv.getObjRelation("DrawPages");
        if ( oDrawPages == null ) {
            // SKIPPED.FAILED - the ObjectRelation is not available
            Status.failed( "ObjectRelation('DrawPages') XDrawPages n.a." );
            return;
        }
        log.println("testing summarize() ... ");

        oObj.summarize(oDrawPages);
        result=true;

        tRes.tested("summarize()", result);
    } // end of summarize()

} // end of XDrawPageSummarizer

