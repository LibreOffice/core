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

package ifc.io;

import lib.MultiMethodTest;

import com.sun.star.io.XMarkableStream;

/**
* Testing <code>com.sun.star.io.XMarkableStream</code>
* interface methods:
* <ul>
*   <li><code>createMark()</code></li>
*   <li><code>deleteMark()</code></li>
*   <li><code>jumpToFurthest()</code></li>
*   <li><code>jumpToMark()</code></li>
*   <li><code>offsetToMark()</code></li>
* </ul> <p>
* @see com.sun.star.io.XMarkableStream
*/
public class _XMarkableStream extends MultiMethodTest {

    public XMarkableStream oObj = null;
    private int mark = -1 ;

    /**
    * Test creates mark and stores it. <p>
    * Has <b> OK </b> status if no exceptions were thrown
    * and returned isn't less than zero. <p>
    */
    public void _createMark() {
        boolean res;
        try {
            mark = oObj.createMark() ;
            res = mark >= 0;
        } catch (com.sun.star.io.IOException e) {
            log.println("Couldn't create mark");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("createMark()", res);
    }

    /**
    * Test deletes the mark that was created by method <code>createMark()
    * </code>.<p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createMark() </code> : to have mark </li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> jumpToFurthest() </code></li>
    *  <li> <code> jumpToMark() </code></li>
    *  <li> <code> offsetToMark() </code></li>
    * </ul>
    */
    public void _deleteMark() {
        requiredMethod("createMark()") ;

        executeMethod("jumpToFurthest()") ;
        executeMethod("jumpToMark()") ;
        executeMethod("offsetToMark()") ;

        boolean res;
        try {
            oObj.deleteMark(mark);
            res = true;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't delete mark");
            e.printStackTrace(log);
            res = false;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't delete mark");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("deleteMark()", res) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createMark() </code></li>
    * </ul>
    */
    public void _jumpToFurthest() {
        requiredMethod("createMark()") ;

        boolean res;
        try {
            oObj.jumpToFurthest() ;
            res = true;
        } catch (com.sun.star.io.IOException e) {
            log.println("Couldn't jump to furthest");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("jumpToFurthest()", res) ;
    }

    /**
    * Test jumps to mark that was created by method <code>createMark()</code>.
    * <p>Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> jumpToFurthest() </code> : for the right order of tests
    *  execution </li>
    * </ul>
    */
    public void _jumpToMark() {
        requiredMethod("jumpToFurthest()") ;
        boolean res;

        try {
            oObj.jumpToMark(mark) ;
            res = true;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't jump to mark");
            e.printStackTrace(log);
            res = false;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't jump to mark");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("jumpToMark()", res) ;
    }

    /**
    * Test obtains offset to mark that was created by
    * method <code>createMark()</code> and checks returned value.<p>
    * Has <b> OK </b> status if returned value is equal to zero
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> jumpToMark() </code> : to have current position at
    *  the mark position </li>
    * </ul>
    */
    public void _offsetToMark() {

        requiredMethod("jumpToMark()") ;

        boolean res;
        try {
            int offset = oObj.offsetToMark(mark);
            res = offset == 0;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't get offser to mark");
            e.printStackTrace(log);
            res = false;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't get offser to mark");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("offsetToMark()", res);
    }
}

