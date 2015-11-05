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

import java.util.List;

import lib.MultiMethodTest;

import com.sun.star.io.XDataOutputStream;

/**
* Testing <code>com.sun.star.io.XDataOutputStream</code>
* interface methods:
* <ul>
*   <li><code>writeBoolean()</code></li>
*   <li><code>writeByte()</code></li>
*   <li><code>writeChar()</code></li>
*   <li><code>writeShort()</code></li>
*   <li><code>writeLong()</code></li>
*   <li><code>writeHyper()</code></li>
*   <li><code>writeFloat()</code></li>
*   <li><code>writeDouble()</code></li>
*   <li><code>writeUTF()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'StreamData'</code> (of type <code>Vector</code>):
*   vector of data for writing to the stream </li>
* <ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XDataOutputStream
*/
public class _XDataOutputStream extends MultiMethodTest {

    public XDataOutputStream oObj = null;

    /**
    * Retrieves object relation <code>'StreamData'</code>
    * and executes methods of interface depending of data in stream.
    * If relation or data of some type in stream not found then
    * tests of corresponding methods are skipped.
    */
    @Override
    public void before() throws RuntimeException {

        List<Object> data = (List<Object>) tEnv.getObjRelation("StreamData") ;
        if (data == null) {
            throw new RuntimeException("Object relation 'StreamData' not found.");
        }

        // extract data from vector
        Object dataElem = null ;
        for (int i = 0; i < data.size(); i++) {
            dataElem = data.get(i) ;

            if (dataElem instanceof Boolean) {
                ((Boolean)dataElem).booleanValue();
            }
            else if (dataElem instanceof Byte) {
                ((Byte)dataElem).byteValue();
            }
            else if (dataElem instanceof Character) {
                ((Character)dataElem).charValue();
            }
            else if (dataElem instanceof Short) {
                ((Short)dataElem).shortValue();
            }
            else if (dataElem instanceof Integer) {
                ((Integer)dataElem).intValue();
            }
            else if (dataElem instanceof Long) {
                ((Long)dataElem).longValue();
            }
            else if (dataElem instanceof Float) {
                ((Float)dataElem).floatValue();
            }
            else if (dataElem instanceof Double) {
                ((Double)dataElem).doubleValue();
            }
        }
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeBoolean() {
        boolean res = true;
        try {
            oObj.writeBoolean(true) ;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Boolean to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeBoolean()", res) ;
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeByte() {
        boolean res = true;
        try {
            oObj.writeByte((byte) 123);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Byte to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeByte()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeChar() {
        boolean res = true;
        try {
            oObj.writeChar((char)12345);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Char to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeChar()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeShort() {
        boolean res = true;
        try {
            oObj.writeShort((short)12345) ;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Short to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeShort()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeLong() {
        boolean res = true;
        try {
            oObj.writeLong(123456);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Long to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeLong()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeHyper() {
        boolean res = true;
        try {
            oObj.writeHyper(123456789);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Hyper to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeHyper()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeFloat() {
        boolean res = true;
        try {
            oObj.writeFloat((float)1.2345);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Float to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeFloat()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeDouble() {
        boolean res = true;
        try {
            oObj.writeDouble(1.2345);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write Double to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeDouble()", res);
    }

    /**
    * Test writes some data to stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _writeUTF() {
        boolean res = true;
        try {
            oObj.writeUTF("XDataOutputStream") ;
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't write String to stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("writeUTF()", res);
    }

    /**
    * Forces object environment recreation.
    */
    @Override
    public void after() {
        this.disposeEnvironment() ;
    }
}

