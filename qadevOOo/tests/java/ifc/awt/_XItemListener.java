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



package ifc.awt;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.awt.ItemEvent;
import com.sun.star.awt.XItemListener;

/**
* Testing <code>com.sun.star.awt.XItemListener</code>
* interface methods :
* <ul>
*  <li><code> itemStateChanged()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'TestItemListener'</code>
*   (of type <code>ifc.awt._XItemListener.TestItemListener</code>):
*   this <code>XItemListener</code> implementation must be
*   added to the object tested for checking
*   <code> itemStateChanged()</code> method call. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XItemListener
*/
public class _XItemListener extends MultiMethodTest {

    public XItemListener oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    * and stores event passed.
    */
    public static class TestItemListener implements com.sun.star.awt.XItemListener {
        public boolean itemStateChangedCalled = false ;
        public ItemEvent event = null ;

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true ;
            event = e ;
        }

        public void disposing(com.sun.star.lang.EventObject e) {}

    }

    TestItemListener itemListener = null ;

    /**
    * Retrieves object relation.
    * @throws StatusException If the relation not found.
    */
    public void before() {
        itemListener = (TestItemListener) tEnv.getObjRelation("TestItemListener") ;
        if (itemListener == null)
            throw new StatusException(Status.failed("Relation not found")) ;
    }

    /**
    * First a <code>ItemEvent</code> object created and
    * it is passed to <code>itemStateChanged</code> method
    * call. Then a short wait follows for listener already
    * registered at the object to be caled. <p>
    * Has <b> OK </b> status if the listener was called with
    * the same <code>ItemEvent</code> object as was created
    * before.
    */
    public void _itemStateChanged() {

        boolean result = true ;

        ItemEvent event = new ItemEvent() ;
        event.Selected = 1 ;
        event.Highlighted = 2 ;
        oObj.itemStateChanged(event) ;

        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {}

        result = itemListener.itemStateChangedCalled &&
            itemListener.event.Selected == 1 &&
            itemListener.event.Highlighted == 2 ;

        tRes.tested("itemStateChanged()", result) ;
    }

}


