/*************************************************************************
 *
 *  $RCSfile: _XImageProducer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:11:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _addConsumer() {

        boolean result = true ;
        oObj.addConsumer(consumer) ;

        tRes.tested("addConsumer()", result) ;
    }

    /**
    * Removes the consumer added before. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
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


