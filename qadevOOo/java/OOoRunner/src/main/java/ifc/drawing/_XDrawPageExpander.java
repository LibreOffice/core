/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.drawing;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageExpander;
import com.sun.star.drawing.XDrawPages;

/**
* Testing <code>com.sun.star.drawing.XDrawPageExpander</code>
* interface methods :
* <ul>
*  <li><code> expand()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (of type <code>XDrawPage</code>):
*   draw page to be expanded. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawPageExpander
*/
public class _XDrawPageExpander extends MultiMethodTest {
    public XDrawPageExpander oObj = null;

    /**
    * First the relation is retrieved. Then the method is called.<p>
    * Has <b> OK </b> status if the method returns not null value. <p>
    */
    public void _expand() {
        boolean result = true;
        XDrawPage oDrawPage = (XDrawPage) tEnv.getObjRelation("DrawPage");
        if ( oDrawPage == null ) {
            // SKIPPED.FAILED - the ObjectRelation is not available
            Status.failed( "ObjectRelation('DrawPages') XDrawPages n.a.");
            return;
        }
        log.println("testing expand() ... ");

        XDrawPages pages = oObj.expand(oDrawPage);
        result = pages != null;

        tRes.tested("expand()", result);
    } // end expand
} // end DrawPageExpander

