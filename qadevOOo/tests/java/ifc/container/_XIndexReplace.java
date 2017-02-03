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
import util.ValueComparer;

import com.sun.star.container.XIndexReplace;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.container.XIndexReplace</code>
* interface methods :
* <ul>
*  <li><code> replaceByIndex()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> : N relations
*   which represents objects to be replaced with. See below
*   for more information.</li>
*  <li> <code>'XIndexReplaceINDEX'</code> : For internal test
*   usage. Contains current thread number. </li>
*  <li> Test environment variable <code>'THRCNT'</code> : number
*   of interface threads running concurrently. </li>
* <ul> <p>
* XIndexReplace needs n ObjectRelations "INSTANCEn" , where n = 1, ..., THRCNT.
* <p>
* When this interface tested by different threads, it must use different
* instances to replace - one for each thread.<p>
* That's why we use objRelation "XIndexReplaceINDEX" to store the number of
* last taken instance. If there is no such relation, it initialize with 1.
* <p>
* This ObjectRelations should be necessary to create an Object,
* which is can be replaced by index
* INSTANCEn are n Objectrelations so that every thread can isert it's own
* object. n depends on the variable THRCNT which and comes from API.INI
* <p>
* Why that:
* If you insert the same Object by replaceByIndex() several times you
* don't insert the Object several times. The first replaceByIndex() inserts
* the Object to the Container but all other replaceByIndex() changes
* the Index in the Container because it's the same Object. <p>
* Test is multithread compliant. <p>
* @see com.sun.star.container.XIndexReplace
*/

public class _XIndexReplace extends MultiMethodTest {
    public XIndexReplace oObj = null;

    /**
    * Primarily tries to replace elements in a proper way :
    * replaces the first, middle and the last element then
    * checks if elements were properly replaced.
    * Then wrong parameters are passed : invalid index and
    * null value for replacing, and test checks for proper
    * exceptions to be thrown. <p>
    * In different threads it replaces elements with different
    * objects.
    * Has <b>OK</b> status if in the first (correct) case
    * elements were successfully replaced (i.e. values got
    * after replacing must be equal to those replaced with),
    * and in the second case proper exceptions were thrown.
    */
    public void _replaceByIndex() {
        boolean result = true;
        Object old = null;
        Object oInstance = null;
        int Index = 0;

        //get for every thread its own Object to insert it
        log.println("get ObjRelation(\"XIndexReplaceINDEX\")");
        String sIndex = (String)tEnv.getObjRelation("XIndexReplaceINDEX");
        if (sIndex == null) {
            log.println("No XIndexReplaceINDEX - so set it to 1.");
            tEnv.addObjRelation("XIndexReplaceINDEX", Integer.toString(1));
            Index = 1;
        } else {
            Index = Integer.parseInt(sIndex);
            Index++;
            tEnv.addObjRelation("XIndexReplaceINDEX", Integer.toString(Index));
        }


        log.println("get ObjRelation(\"INSTANCE" + Index +"\")");
        oInstance = tEnv.getObjRelation("INSTANCE"+ Index);
        if (oInstance == null) {
            log.println("ObjRelation(\"INSTANCE" + Index +"\") Object n.a.");
        }

        log.println("testing replaceByIndex(0)...");

        try {

               log.println("Getting old object");
               old = oObj.getByIndex(0);
               oObj.replaceByIndex(0, oInstance);
            result = !(oObj.getByIndex(0)).equals(old);
            result = ! ValueComparer.equalValue(oObj,old);

        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
            result = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            result = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            result = false;
        }


        log.println("replace with a wrong Object occurs Exceptions ...");
        try {
               oObj.replaceByIndex(999, oInstance);
               result = false;
            log.println("1. replaceByIndex(): Exception expected! - FAILED");


            XNameContainer xNC = UnoRuntime.queryInterface(XNameContainer.class, oObj) ;
            String[] names = xNC.getElementNames() ;
            log.println("Element names :") ;
            for (int i = 0; i<names.length; i++) {
                 log.println("  '" + names[i] + "'") ;
            }
           } catch (IndexOutOfBoundsException e) {
            log.println("1. replaceByIndex(): Expected exception - OK");
            result &= true;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            result = false;
            log.println("1. replaceByIndex(): Unexpected exception! - " +
                 e + " - FAILED");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("1. replaceByIndex(): Unexpected exception! - " +
                e + " - FAILED");
        }

        log.println("replace with a wrong Object occurs Exceptions ...");
        try {
               oObj.replaceByIndex(0, null);
               result = false;
            log.println("2. replaceByIndex(): Exception expected! - FAILED");


            XNameContainer xNC = UnoRuntime.queryInterface(XNameContainer.class, oObj) ;
            String[] names = xNC.getElementNames() ;
            log.println("Element names :") ;
            for (int i = 0; i<names.length; i++) {
                 log.println("  '" + names[i] + "'") ;
            }
           } catch (IllegalArgumentException e) {
            log.println("2. replaceByIndex(): Expected exception - OK");
            result &= true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("2. replaceByIndex(): Unexpected exception! - " +
                e + " - FAILED");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            result = false;
            log.println("2. replaceByIndex(): Unexpected exception! - " +
                e + " - FAILED");
        }

        log.println("replace with the old object");
        try {
            oObj.replaceByIndex(0, old);
        } catch (IllegalArgumentException e) {
            e.printStackTrace(log) ;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log) ;
        }

        tRes.tested("replaceByIndex()", result);
    }
}


