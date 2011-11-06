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



package ifc.view;

import lib.MultiMethodTest;

import com.sun.star.view.XScreenCursor;


/**
 * Testing <code>com.sun.star.view.XScreenCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> screenDown()</code></li>
 *  <li><code> screenUp()</code></li>
 * </ul> <p>
 *
 * Uses test parameter <b><code>soapi.test.hidewindows</code></b>.
 * Methods of this interface works with view representation,
 * thereby their calls have no effect when document is opened
 * in hidden mode. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XScreenCursor
 */
public class _XScreenCursor extends MultiMethodTest {

    // oObj filled by MultiMethodTest

    public XScreenCursor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value or if document is
     * opened in hidden mode.
     */
    public void _screenDown(){
        log.println("test for screenDown() ");
        tRes.tested("screenDown()", oObj.screenDown() );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value or if document is
     * opened in hidden mode.
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> screenDown() </code> : by default view is on the
     *  top of pag, thereby it's needed first to moved down. </li>
     * </ul>
     */
    public void _screenUp(){
        requiredMethod("screenDown()") ;

        log.println("test for screenUp() ");
        tRes.tested("screenUp()", oObj.screenUp() );
    }

}  // finish class _XScreenCursor

