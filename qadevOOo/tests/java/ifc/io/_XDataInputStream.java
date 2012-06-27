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
import lib.Status;
import lib.StatusException;

import com.sun.star.io.XDataInputStream;
import com.sun.star.io.XDataOutputStream;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.io.XDataInputStream</code>
* interface methods:
* <ul>
*   <li><code>readBoolean()</code></li>
*   <li><code>readByte()</code></li>
*   <li><code>readChar()</code></li>
*   <li><code>readShort()</code></li>
*   <li><code>readLong()</code></li>
*   <li><code>readHyper()</code></li>
*   <li><code>readFloat()</code></li>
*   <li><code>readDouble()</code></li>
*   <li><code>readUTF()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'StreamData'</code> (of type <code>Vector</code>):
*   vector of data for comparing with data that obtained from stream </li>
*  <li> <code>'StreamWriter'</code> (of type <code>XDataOutputStream</code>):
*   a possiblitiy to write values to the stream. </li>
* <ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.io.XDataInputStream
* @see java.util.Vector
*/
public class _XDataInputStream extends MultiMethodTest {

    public XDataInputStream oObj = null;
    public XDataOutputStream oStream = null;

    // values that are written
    private boolean writeBoolean;
    private byte writeByte;
    private char writeChar;
    private double writeDouble;
    private float writeFloat;
    private long writeHyper;
    private int writeLong;
    private short writeShort;
    private String writeUTF;


    /**
     * Retrieves relations. From relation 'StreamData' extracts
     * data of different types and fills the appropriate variables.
     * @throws StatusException If one of relations not found.
     */
    public void before(){

        XInterface x = (XInterface)tEnv.getObjRelation("StreamWriter") ;
        oStream = UnoRuntime.queryInterface(
                                                    XDataOutputStream.class, x);
        List<Object> data = (List<Object>) tEnv.getObjRelation("StreamData") ;
        if (data == null || oStream == null) {
            throw new StatusException(Status.failed("Object relation not found."));
        }

        // extract data from vector
        Object dataElem = null ;
        for (int i = 0; i < data.size(); i++) {
            dataElem = data.get(i) ;

            if (dataElem instanceof Boolean) {
                writeBoolean = ((Boolean)dataElem).booleanValue();
            } else
            if (dataElem instanceof Byte) {
                writeByte = ((Byte)dataElem).byteValue();
            } else
            if (dataElem instanceof Character) {
                writeChar = ((Character)dataElem).charValue();
            } else
            if (dataElem instanceof Short) {
                writeShort = ((Short)dataElem).shortValue();
            } else
            if (dataElem instanceof Integer) {
                writeLong = ((Integer)dataElem).intValue();
            } else
            if (dataElem instanceof Long) {
                writeHyper = ((Long)dataElem).longValue();
            } else
            if (dataElem instanceof Float) {
                writeFloat = ((Float)dataElem).floatValue();
            } else
            if (dataElem instanceof Double) {
                writeDouble = ((Double)dataElem).doubleValue();
            } else
            if (dataElem instanceof String) {
                writeUTF = (String)dataElem;
            }
        }
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readBoolean() {
        boolean res = true ;
        try {
            oStream.writeBoolean(writeBoolean);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        byte readElem;
        try {
            readElem = oObj.readBoolean();
            res = ((readElem != 0) == writeBoolean);

            if (!res)
                log.println("Must be read " +
                    writeBoolean +
                    " but was read " + (readElem != 0)) ;
        } catch (com.sun.star.io.IOException e) {
            log.println("Couldn't  read Boolean from stream");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("readBoolean()", res) ;
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readByte() {
        boolean res = true ;
        try {
            oStream.writeByte(writeByte);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        byte readElem;
        try {
            readElem = oObj.readByte() ;
            res = (readElem == writeByte);

            if (!res)
                log.println("Must be read " +
                    writeByte +
                    " but was read " + readElem);
        } catch(com.sun.star.io.IOException e) {
            log.println("Couldn't read Byte from stream");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("readByte()", res) ;
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readChar() {
        boolean res = true ;
        try {
            oStream.writeChar(writeChar);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        char readElem;
        try {
            readElem = oObj.readChar() ;
            res = (readElem == writeChar);

            if (!res)
                log.println("Must be read " +
                    writeChar +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Char from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readChar()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readShort() {
        boolean res = true ;
        try {
            oStream.writeShort(writeShort);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        short readElem;
        try {
            readElem = oObj.readShort() ;
            res = (readElem == writeShort);

            if (!res)
                log.println("Must be read " +
                    writeShort +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Short from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readShort()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readLong() {
        try {
            oStream.writeLong(writeLong);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        boolean res = true ;
        int readElem;
        try {
            readElem = oObj.readLong() ;
            res = (readElem == writeLong);

            if (!res)
                log.println("Must be read " +
                    writeLong +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Long from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readLong()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readHyper() {
        boolean res = true ;
        try {
            oStream.writeHyper(writeHyper);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        long readElem;
        try {
            readElem = oObj.readHyper() ;
            res = (readElem == writeHyper);

            if (!res)
                log.println("Must be read " +
                    writeHyper +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Hyper from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readHyper()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readFloat() {
        boolean res = true ;
        try {
            oStream.writeFloat(writeFloat);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        float readElem;
        try {
            readElem = oObj.readFloat() ;
            res = (readElem == writeFloat);

            if (!res)
                log.println("Must be read " +
                    writeFloat +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Float from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readFloat()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readDouble() {
        boolean res = true ;
        try {
            oStream.writeDouble(writeDouble);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        double readElem;
        try {
            readElem = oObj.readDouble() ;
            res = (readElem == writeDouble);

            if (!res)
                log.println("Must be read " +
                    writeDouble +
                    " but was read " + readElem) ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read Double from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readDouble()", res);
    }

    /**
     * First writes a value to outStream then reads it from input. <p>
     *
     * Has <b> OK </b> status if read and written values are equal. <p>
     */
    public void _readUTF() {
        boolean res = true ;
        try {
            oStream.writeUTF(writeUTF);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write data to the stream", e);
        }
        String readElem;
        try {
            readElem = oObj.readUTF() ;
            res = writeUTF.equals(readElem) ;

            if (!res)
                log.println("Must be read '" +
                    writeUTF +
                    "' but was read '" + readElem + "'") ;
        } catch( com.sun.star.io.IOException e ) {
            log.println("Couldn't read String from stream");
            e.printStackTrace(log);
            res = false;
        }
        tRes.tested("readUTF()", res);
    }

    /**
     * Forces object environment recreation.
     */
    public void after() {
        try {
            oStream.flush();
        } catch (com.sun.star.io.NotConnectedException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.io.BufferSizeExceededException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
        }
        this.disposeEnvironment() ;
    }
}

