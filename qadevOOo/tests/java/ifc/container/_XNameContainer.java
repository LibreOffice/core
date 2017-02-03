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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import lib.MultiMethodTest;
import lib.StatusException;

/**
* Testing <code>com.sun.star.container.XNameContainer</code>
* interface methods :
* <ul>
*  <li><code> insertByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul>
* This test needs the following object relations :
* <ul>
*  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> : N relations
*   which represents objects to be inserted. See below
*   for more information.</li>
*  <li> <code>'XNameContainerINDEX'</code> : For internal test
*   usage. Contains current thread number. </li>
*  <li> <code>'XNameContainer.AllowDuplicateNames'</code> <b>optional</b>:
*   if this relation exists then container elements can have duplicate
*   names. </li>
*  <li> Test environment variable <code>'THRCNT'</code> : number
*   of interface threads running concurrently. </li>
* <ul> <p>
* XNameContainer needs n ObjectRelations "INSTANCEn" , where n=1, ..., THRCNT.
*
* When this interface tested by different threads, it must use different
* instances to insert/remove - one for each thread.
*
* That's why we use objRelation "XNameContainerINDEX" to store the number of
* last taken instance. If there is no such relation, it initialize with 1.
*
* If you insert the same Object by insertByName() several times you
* don't insert the Object several times. The first insertByName() inserts
* the Object to the Container but all other insertByName() changes
* the Name in the Container because it's the same Object.
* @see com.sun.star.container.XNameContainer
*/

public class _XNameContainer extends MultiMethodTest {
    public XNameContainer oObj = null;
    String Name = "XNameContainer";

    /**
    * First inserts object by name (different objects for different threads)
    * and checks if it exists. Second, if duplicate names are not allowed
    * test tries to insert element with the same name and checks for
    * proper exception. Third, tries to add <code>null</code> element and
    * checks for proper exception. <p>
    * Has <b>OK</b> status if in the first case element added exists in
    * the container, in the second case <code>ElementExistException</code>
    * is thrown, and in the third case <code>IllegalArgumentException</code>
    * is thrown.
    */
    public void _insertByName() {
        boolean result = true;
        int Index = 0;

        //get for every thread its own Object to insert it
        log.println("get ObjRelation(\"XNameContainerINDEX\")");
        String sIndex = null ;
        synchronized (tEnv) {
            sIndex = (String)tEnv.getObjRelation("XNameContainerINDEX");
            if (sIndex == null) {
                log.println("No XNameContainerINDEX - so set it to 1.");
                tEnv.addObjRelation("XNameContainerINDEX",Integer.toString(1));
                Index = 1;
            } else {
                Index = Integer.parseInt(sIndex);
                Index++;
                tEnv.addObjRelation("XNameContainerINDEX",
                    Integer.toString(Index));
            }
        }
        Name += Index ;

        log.println("get ObjRelation(\"INSTANCE" + Index +"\")");
        Object oInstance = tEnv.getObjRelation("INSTANCE"+ Index);
        log.println("ObjRelation(\"INSTANCE" + Index +"\") Object n.a.");

        log.println("testing insertByName(\""+Name+"\")...");
        try {
            String[] names = oObj.getElementNames() ;
            log.println("Element names :") ;
            for (int i = 0; i<names.length; i++) {
                 log.println("  '" + names[i] + "'") ;
            }

            oObj.insertByName(Name, oInstance);

            names = oObj.getElementNames() ;
            log.println("Element names :") ;
            for (int i = 0; i<names.length; i++) {
                 log.println("  " + names[i]) ;
            }

            result &= oObj.hasByName(Name) ;
            log.println("insertByName(\""+Name+"\")...OK");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("insertByName(\""+Name+"\"): " + e + " FALSE");
            result = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("insertByName(\""+Name+"\"): " + e + " FALSE");
            result = false;
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("insertByName(\""+Name+"\"): " + e + " FALSE");
            result = false;
        }

        // if duplicate names is not allowed test for valid exception
        if (tEnv.getObjRelation("XNameContainer.AllowDuplicateNames")==null) {
            Object secondInstance = tEnv.getObjRelation("SecondInstance");
            if (secondInstance != null) {
                oInstance = secondInstance;
            }
            log.println("Trying to add element with the same name ...") ;
            try {
                oObj.insertByName(Name, oInstance);
                result = false ;
                log.println("!!! No exception were thrown !!!");
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("!!! Wrong exception : " + e + " FALSE");
                result = false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("!!! Wrong exception : " + e + " FALSE");
                result = false;
            } catch (com.sun.star.container.ElementExistException e) {
                log.println("Right exception : " + e + " OK");
            }
        }

        log.println("inserting a wrong Object occurs Exceptions ...");
        try {
            Object dummy = null;
            oObj.insertByName("Dummy", dummy);
            log.println("No Exception: -> FALSE");
            result = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Dummy-Exception: " + e + " -> OK");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("!!! This exception not expected: " +e+ " -> FAILED");
            result = false;
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("!!! This exception not expected: " +e+ " -> FAILED");
            result = false;
        }

        tRes.tested("insertByName()", result);

    } // end insertByName()

    /**
    * Test removes element inserted before and checks if element
    * still exists in the container. Second test tries to remove
    * element with non-existing name and checks for proper exception. <p>
    * Has <b> OK </b> status if in the first case element doesn't
    * exist anymore (or duplicate names are allowed), and in the
    * second case <code>NoSuchElementException</code> is thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertByName() </code> : to remove the element inserted
    *    in this test. </li>
    * </ul>
    */
    public void _removeByName() {
        try {
            requiredMethod("insertByName()");
        } catch (StatusException e) {
            // removing the name anywhere
            try {
                oObj.removeByName(Name);
            } catch (com.sun.star.container.NoSuchElementException e1) {
            } catch (com.sun.star.lang.WrappedTargetException e1) {
            }
        }

        boolean result = true;

        log.println("testing removeByName() ...");

        try {
            log.println("remove " + Name);
            String[] names = oObj.getElementNames() ;
            log.println("Element names :") ;
            for (int i = 0; i<names.length; i++) {
                log.println("  " + names[i]) ;
            }
            oObj.removeByName(Name);
            boolean loc_res = !oObj.hasByName(Name) || tEnv.getObjRelation
                ("XNameContainer.AllowDuplicateNames") != null ;
            result &= loc_res ;
            if (loc_res)
                log.println("1. removeByName(\""+Name+"\") ...OK");
            else
                log.println("1. !!! Container still has element with name "
                    + Name) ;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("1. removeByName:(\""+Name+"\") " + e + " - FAILED");
        } catch (com.sun.star.container.NoSuchElementException e) {
            result = false;
            log.println("1. removeByName:(\""+Name+"\") " + e + " - FAILED");
        }

        log.println("removing a non existent object to get an exception");
        try {
            oObj.removeByName(Name+ " dummy");
            result = false;
            log.println("2. removeByName(): Exception expected! - FAILED");
        } catch (NoSuchElementException e) {
            log.println("2. removeByName(): Expected exception - OK");
            result &= true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            result = false;
            log.println("2. removeByName(): Unexpected exception! - " +
                e + " - FAILED");
        }

        tRes.tested("removeByName()", result);
    } // end removeByName()
} //XNameContainer



