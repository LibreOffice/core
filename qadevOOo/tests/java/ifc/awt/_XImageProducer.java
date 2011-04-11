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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XImageProducer
*/
public class _XImageProducer extends MultiMethodTest {

    public XImageProducer oObj = null;

    /**
    * Consumer implementation which sets flags on appropriate
    * method calls.
    */
    protected class TestImageConsumer implements XImageConsumer {
        PrintWriter log = null ;
        public boolean initCalled = false ;
        public boolean setColorModelCalled = false ;
        public boolean setPixelsByBytesCalled = false ;
        public boolean setPixelsByLongsCalled = false ;
        public boolean completeCalled = false ;

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
            setColorModelCalled = true ;
        }

        public void setPixelsByBytes(int x, int y, int width, int height,
            byte[] data, int offset, int scanSize) {

            log.println("### setPixelByBytes() called") ;
            setPixelsByBytesCalled = true ;
        }

        public void setPixelsByLongs(int x, int y, int width, int height,
            int[] data, int offset, int scanSize) {

            log.println("### setPixelByLongs() called") ;
            setPixelsByLongsCalled = true ;
        }

        public void complete(int status, XImageProducer prod) {
            log.println("### complete() called") ;
            completeCalled = true ;
        }
    }

    TestImageConsumer consumer = null ;

    /**
    * Creates a new XImageConsumer implementation.
    */
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

        boolean result = true ;
        oObj.startProduction() ;

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {}

        tRes.tested("startProduction()", consumer.initCalled) ;
    }

}


