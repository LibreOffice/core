/*************************************************************************
 *
 *  $RCSfile: _XOutputStream.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:43:54 $
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
import lib.StatusException;

import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;

/**
* Testing <code>com.sun.star.io.XOutputStream</code>
* interface methods:
* <ul>
*   <li><code>writeBytes()</code></li>
*   <li><code>flush()</code></li>
*   <li><code>closeOutput()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ByteData'</code> : Data that is written on the stream.
*  </li>
*  <li> <code>'XOutputStream.StreamChecker'</code> : <code>
*    _XOutputStream.StreamChecker</code> interface implementation
*    which can reset streams and return input stream for check if the
*    data was successfully written.</li>
* <ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XOutputStream
*/
public class _XOutputStream extends MultiMethodTest {

    public XOutputStream oObj = null;
    StreamChecker checker = null;
    byte[] data = null;

    public static interface StreamChecker {
        public XInputStream getInStream();
        public void resetStreams();
    }

    protected void before() {
        checker = (StreamChecker)
            tEnv.getObjRelation("XOutputStream.StreamChecker");
        if (checker == null) throw
            new StatusException(Status.failed(
                "Couldn't get relation 'XOutputStream.StreamChecker'"));

        data = (byte[])tEnv.getObjRelation("ByteData");
        if (data == null) throw
            new StatusException(Status.failed(
                "Couldn't get relation 'ByteData'"));
    }
    /**
    * Test writes data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeBytes() {
        boolean res = true;
        try {
            oObj.writeBytes(data);
         } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log) ;
            res = false;
        }

        XInputStream xInStream = checker.getInStream();
        byte[][] readData = new byte[1][data.length];
        int iReadBytes = 0;
        try {
            iReadBytes = xInStream.readBytes(readData, data.length);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't read data:" + e);
            res = false;
        }

        for(int i = 0; i < readData[0].length; i++) {
            log.println("Expected: "+data[i]+", actual is "+readData[0][i]);
            res &= readData[0][i] == data[i];
        }

        tRes.tested("writeBytes()", res);
    }

    /**
    * Test flushes out data from stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> writeBytes() </code></li>
    * </ul>
    */
    public void _flush() {
        requiredMethod("writeBytes()");

        boolean res;
        try {
            oObj.flush();
            res = true;
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log) ;
            res = false;
        }

        tRes.tested("flush()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> writeBytes() </code></li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li><code> flush() </code></li>
    * </ul>
    */
    public void _closeOutput() {
        requiredMethod("writeBytes()");
        executeMethod("flush()");

        boolean res;
        try {
            oObj.closeOutput();
            res = true;
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            res = false;
        }

        log.println("This method is called in main module");

        tRes.tested("closeOutput()", res);
    }

    /**
    * Forces object environment recreation.
    */
    public void after() {
        tEnv.dispose() ;
    }
}

