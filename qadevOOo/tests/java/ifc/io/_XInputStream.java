/*************************************************************************
 *
 *  $RCSfile: _XInputStream.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:43:11 $
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
        tEnv.dispose();
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
        tEnv.dispose() ;
    }
}

