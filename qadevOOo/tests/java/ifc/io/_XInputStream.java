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

package ifc.io;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XInputStream</code>
* interface methods:
* <ul>
*   <li><code>readBytes()</code></li>
*   <li><code>readSomeBytes()</code></li>
*   <li><code>skipBytes()</code></li>
*   <li><code>available()</code></li>
*   <li><code>closeInput()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'StreamWriter'</code>:
*   object that supports interface <code>XOutputStream</code>;
*   a stream to write data to</li>
*  <li> <code>'ByteData'</code> (of type <code>byte []</code>):
*   data to write to the stream</li>
* <ul> <p>

* @see com.sun.star.io.XInputStream
*/
public class _XInputStream extends MultiMethodTest {

    public XInputStream oObj = null;
    public XOutputStream oStream = null;

    byte[] bytes = null;

    int bytesReady = 0 ;

    /**
     * Before the test, the stream writer and the data are ecxtracted from
     * the object relations and the data is written to the stream.
     */
    public void before() {
        XInterface x = (XInterface)tEnv.getObjRelation("StreamWriter");
        oStream = (XOutputStream)UnoRuntime.queryInterface(
                                                    XOutputStream.class, x) ;
        bytes = (byte[])tEnv.getObjRelation("ByteData");
        try {
            oStream.writeBytes(bytes);
        }
        catch(com.sun.star.io.NotConnectedException e) {}
        catch(com.sun.star.io.BufferSizeExceededException e) {}
        catch(com.sun.star.io.IOException e) {}
    }

    /**
     * After the test, the stream writer is closed and the
     * environment is disposed.
     */
    public void after() {
        try {
            oStream.flush();
            oStream.closeOutput();
        }
        catch(com.sun.star.io.NotConnectedException e) {}
        catch(com.sun.star.io.BufferSizeExceededException e) {}
        catch(com.sun.star.io.IOException e) {}
        this.disposeEnvironment();
    }
    /**
    * Test calls the method and stores number of available bytes. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _available() {
        boolean result = true ;
        try {
            bytesReady = oObj.available() ;
            log.println("Bytes available :" + bytesReady) ;
        } catch (com.sun.star.io.IOException e){
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("available()", result) ;
    }

    /**
    * Test reads one byte from stream. If no bytes available
    * then test of method is skipped. <p>
    * Has <b> OK </b> status if returned value equal to number of read bytes,
    * no exceptions were thrown and read data is not null. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> available() </code> : to have available number
    *  of bytes in stream </li>
    * </ul>
    */
    public void _readBytes() {
        requiredMethod("available()") ;
        boolean result ;

        if (bytesReady-- > 0) {
            try {
                byte[][] data = new byte[1][1] ;
                int read = oObj.readBytes(data, 1) ;

                result = read == 1 &&
                         data != null &&
                         data.length == 1 ;
            } catch (com.sun.star.io.IOException e){
                e.printStackTrace(log) ;
                result = false ;
            }

            tRes.tested("readBytes()", result) ;
        } else {
            log.println("No more bytes available in the stream");
            tRes.tested("readBytes()", Status.skipped(false));
        }
    }

    /**
    * Test reads one byte from stream. If no bytes available
    * then test of method is skipped. <p>
    * Has <b> OK </b> status if returned value equal to number of read bytes,
    * no exceptions were thrown and read data is not null. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> available() </code> : to have available number
    *  of bytes in stream </li>
    * </ul>
    */
    public void _readSomeBytes() {
        requiredMethod("available()") ;
        boolean result ;

        if (bytesReady-- > 0) {
            try {
                byte[][] data = new byte [1][1] ;
                int read = oObj.readSomeBytes(data, 1) ;

                result = read == 1 &&
                         data != null &&
                         data.length == 1 ;
            } catch (com.sun.star.io.IOException e){
                e.printStackTrace(log) ;
                result = false ;
            }
            tRes.tested("readSomeBytes()", result) ;
        } else {
            log.println("No more bytes available in the stream") ;
            tRes.tested("readBytes()", Status.skipped(false));
        }
    }

    /**
    * Test skips one byte from stream. If no bytes available
    * then test of method is skipped. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> available() </code> : to have available number
    *  of bytes in stream </li>
    * </ul>
    */
    public void _skipBytes() {
        requiredMethod("available()") ;
        boolean result ;

        if (bytesReady-- > 0) {
            try {
                oObj.skipBytes(1) ;

                result = true ;
            } catch (com.sun.star.io.IOException e){
                e.printStackTrace(log) ;
                result = false ;
            }
            tRes.tested("skipBytes()", result) ;
        } else {
            log.println("No more bytes available in the stream") ;
            tRes.tested("readBytes()", Status.skipped(false));
        }
    }

    /**
    * Test calls the method and forces object environment recreation. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> available() </code> </li>
    *  <li> <code> readBytes() </code> </li>
    *  <li> <code> readSomeBytes() </code> </li>
    *  <li> <code> skipBytes() </code> </li>
    * </ul>
    */
    public void _closeInput() {
        executeMethod("available()") ;
        executeMethod("readBytes()") ;
        executeMethod("readSomeBytes()") ;
        executeMethod("skipBytes()") ;

        boolean result = true ;
        try {
            oObj.closeInput() ;
        } catch (com.sun.star.io.IOException e){
            e.printStackTrace(log) ;
            result = false ;
        }

        tRes.tested("closeInput()", result) ;
        this.disposeEnvironment() ;
    }
}

