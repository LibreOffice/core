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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexContainer;
import com.sun.star.lang.IndexOutOfBoundsException;

/**
* Testing <code>com.sun.star.container.XIndexContainer</code>
* interface methods :
* <ul>
*  <li><code> insertByIndex()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> : N relations
*   which represents objects to be inserted. See below
*   for more information.</li>
*  <li> <code>'XIndexContainerINDEX'</code> : For internal test
*   usage. Contains current thread number. </li>
*  <li> Test environment variable <code>'THRCNT'</code> : number
*   of interface threads running concurrently. </li>
* <ul> <p>
* XIndexContainer needs n ObjectRelations "INSTANCEn" , where n=1, ...,
* THRCNT.<p>
* When this interface tested by different threads, it must use different
* instances to insert/remove - one for each thread.
* <p>
* That's why we use objRelation "XIndexContainerINDEX" to store the number of
* last taken instance. If there is no such relation, it initialize with 1.
* <p>
* This ObjectRelations should be necessary to create an Object,
* which is insertable by insterByIndex()
* INSTANCEn are n Objectrelations so that every thread can isert it's own
* object. n depends on the variable THRCNT which and comes from API.INI
* <p>
* Why that:
* If you insert the same Object by insertByIndex() several times you
* don't insert the Object several times. The first insertByIndex() inserts
* the Object to the Container but all other insertByIndex() changes
* the Index in the Container because it's the same Object. <p>
* Test is multithread compliant. <p>
* @see com.sun.star.container.XIndexContainer
*/

public class _XIndexContainer extends MultiMethodTest {
    public XIndexContainer oObj = null;

     int Index = 0;

    /**
    * First tries to insert proper object. Second tries to insert
    * null value. For each test thread different objects are inserted
    * on different indexes. For example for the first started test index
    * is 0 and object is get from relation 'INCTANCE1', and so on. <p>
    * Has <b>OK</b> status if in the first case <code>getByIndex</code>
    * method returns non null value and in the second <code>
    * IndexOutOfBoundsException</code> was thrown.
    */
    public void _insertByIndex() {
        boolean result = true;

        log.println("get ObjRelation(\"XIndexContainerINDEX\")");
        String sIndex = (String)tEnv.getObjRelation("XIndexContainerINDEX");
        if (sIndex == null) {
            log.println("No XIndexContainerINDEX - so set it to 1.");
            tEnv.addObjRelation("XIndexContainerINDEX", Integer.toString(1));
            Index = 1;
        } else {
            Index = Integer.parseInt(sIndex);
            Index++;
            tEnv.addObjRelation("XIndexContainerINDEX",
                Integer.toString(Index));
        }


        log.println("get ObjRelation(\"INSTANCE" + Index +"\")");
        Object oInstance = tEnv.getObjRelation("INSTANCE"+ Index);
        if (oInstance == null) {
            log.println("ObjRelation(\"INSTANCE" + Index +"\") Object n.a.");
        }

        log.println("testing insertByIndex(\"" + Index + "\")...");
        try {
            oObj.insertByIndex(Index, oInstance);
            result &= oObj.getByIndex(Index) != null ;
            log.println("insertByIndex(\""+Index+"\")...OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("insertByIndex(\""+Index+"\"): " + e + " FLASE");
            result = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("insertByIndex(\""+Index+"\"): " + e + " FLASE");
            result = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("insertByIndex(\""+Index+"\"): " + e + " FLASE");
            result = false;
        }

        log.println("inserting a wrong Object occurs Exceptions ...");
        try {
            Object dummy = null;
            oObj.insertByIndex(0, dummy);
            log.println("No Exception: -> FALSE");
            result = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Dummy-Exception: " + e + " -> OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("!!! Wrong Exception: " + e + " -> FAILED");
            result = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("!!! Wrong Exception: " + e + " -> FAILED");
            result = false;
        }

        tRes.tested("insertByIndex()", result);
    }

    /**
    * Removes the element inserted by <code>insertByIndex</code> method test.
    * The number of elements is checked before and after removing.
    * Then tries to remove an element with invalid index and checks exceptions.
    * <p>
    * Has <b>OK</b> status if after removing number of elements decreases by
    * one and <code>IndexOutOfBoundsException</code> is thrown on invalid index
    * removing.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertByIndex </code> : to have an object which can be
    * removed.</li>
    * </ul>
    */
    public void _removeByIndex() {
        requiredMethod("insertByIndex()");
        boolean result = true;

        log.println("testing removeByIndex() ...");

        try {
            log.println("remove " +Index);
            int cnt1 = -1 , cnt2 = -1 ;
            synchronized (oObj) {
                cnt1 = oObj.getCount() ;
                oObj.removeByIndex(Index);
                cnt2 = oObj.getCount() ;
            }
            log.println("Count before removing : " + cnt1 +
                ", and after : " + cnt2) ;

            result &= cnt1 == cnt2 + 1 ;

            log.println("1. removeByIndex(\""+Index+"\") ...OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            log.println("1. removeByIndex:(\""+Index+"\") " +
                e + " - FAILED");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("1. removeByIndex:(\""+Index+"\") " +
                e + " - FAILED");
        }

        log.println("removing a non existent object to get an exception");
        try {
            oObj.removeByIndex(100);
            result = false;
            log.println("2. removeByIndex(): Exception expected! - FAILED");
        } catch (IndexOutOfBoundsException e) {
            log.println("2. removeByIndex(): Expected exception - OK");
            result &= true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("2. removeByIndex(): Unexpected exception! - " +
                e + " - FAILED");
        }

        tRes.tested("removeByIndex()", result);
    }
}



