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

package ifc.awt;


import java.io.PrintWriter;

import lib.MultiMethodTest;

import com.sun.star.awt.XImageConsumer;
import com.sun.star.awt.XImageProducer;

/**
* Testing <code>com.sun.star.awt.XImageProducer</code>
* interface methods :
* <ul>
*  <li><code> addConsumer()</code></li>
*  <li><code> removeConsumer()</code></li>
*  <li><code> startProduction()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XImageProducer
*/
public class _XImageProducer extends MultiMethodTest {

    public XImageProducer oObj = null;

    /**
    * Consumer implementation which sets flags on appropriate
    * method calls.
    */
    protected static class TestImageConsumer implements XImageConsumer {
        PrintWriter log = null ;
        public boolean initCalled = false ;

        TestImageConsumer(PrintWriter log) {
            log.println("### Consumer initialized" ) ;
            this.log = log ;
        }

        public void init(int width, int height) {
            log.println("### init() called") ;
            initCalled = true ;
        }

        public void setColorModel(short bitCount, int[] RGBAPal,
            int redMask, int greenMask, int blueMask, int alphaMask) {
            log.println("### setColorModel() called") ;
        }

        public void setPixelsByBytes(int x, int y, int width, int height,
            byte[] data, int offset, int scanSize) {
            log.println("### setPixelByBytes() called") ;
        }

        public void setPixelsByLongs(int x, int y, int width, int height,
            int[] data, int offset, int scanSize) {
            log.println("### setPixelByLongs() called") ;
        }

        public void complete(int status, XImageProducer prod) {
            log.println("### complete() called") ;
        }
    }

    TestImageConsumer consumer = null ;

    /**
    * Creates a new XImageConsumer implementation.
    */
    @Override
    public void before() {
        consumer = new TestImageConsumer(log) ;
    }

    /**
    * Adds a new consumer to producer. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _addConsumer() {

        boolean result = true ;
        oObj.addConsumer(consumer) ;

        tRes.tested("addConsumer()", result) ;
    }

    /**
    * Removes the consumer added before. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> startProduction </code>  </li>
    * </ul>
    */
    public void _removeConsumer() {
        executeMethod("startProduction()") ;

        boolean result = true ;
        oObj.removeConsumer(consumer) ;

        tRes.tested("removeConsumer()", result) ;
    }

    /**
    * Starts the production and after short waiting  checks what
    * consumer's methods were called. <p>
    * Has <b> OK </b> status if at least <code>init</code> consumer
    * methods was called.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addConsumer </code> </li>
    * </ul>
    */
    public void _startProduction() {
        requiredMethod("addConsumer()") ;

        oObj.startProduction() ;

        waitForEventIdle();

        tRes.tested("startProduction()", consumer.initCalled) ;
    }

}


