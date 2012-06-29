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

package mod._streams.uno;

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import java.util.ArrayList;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.DataOutputStream</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XActiveDataSource</code></li>
*  <li> <code>com::sun::star::io::XOutputStream</code></li>
*  <li> <code>com::sun::star::io::XDataOutputStream</code></li>
* </ul>
* @see com.sun.star.io.DataOutputStream
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XOutputStream
* @see com.sun.star.io.XDataOutputStream
* @see ifc.io._XActiveDataSource
* @see ifc.io._XOutputStream
* @see ifc.io._XDataOutputStream
*/
public class DataOutputStream extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.io.DataOutputStream</code>
    * and an instance of the service <code>com.sun.star.io.Pipe</code>.
    * Plugs the created pipe as output stream for the created DataOutputStream.
    * @see com.sun.star.io.DataOutputStream
    * Object relations created :
    * <ul>
    *  <li> <code>'StreamData'</code> for
    *      {@link ifc.io._XDataInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'OutputStream'</code> for
    *      {@link ifc.io._XActiveDataSource}
    *      (an input stream to set and get) </li>
    *  <li> <code>'XOutputStream.StreamChecker'</code> for
    *      {@link ifc.io._XOutputStream}( implementation of the interface
    *      ifc.io._XOutputStream.StreamChecker ) </li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        Object oInterface = null;
        XInterface oPipe = null;
        XMultiServiceFactory xMSF = null ;

        try {
            xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ("com.sun.star.io.DataOutputStream");
            oPipe = (XInterface)xMSF.createInstance("com.sun.star.io.Pipe");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        oObj = (XInterface) oInterface;

        final XOutputStream xPipeOutput = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class, oPipe);

        XActiveDataSource xDataSource = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, oObj);

        xDataSource.setOutputStream(xPipeOutput);

        // all data types for writing to an XDataInputStream
        ArrayList<Object> data = new ArrayList<Object>() ;
        data.add(new Boolean(true)) ;
        data.add(new Byte((byte)123)) ;
        data.add(new Character((char)1234)) ;
        data.add(new Short((short)1234)) ;
        data.add(new Integer(123456)) ;
        data.add(new Float(1.234)) ;
        data.add(new Double(1.23456)) ;
        data.add("DataInputStream") ;
        // information for writing to the pipe
        byte[] byteData = new byte[] {
            1, 2, 3, 4, 5, 6, 7, 8 } ;

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("StreamData", data);
        tEnv.addObjRelation("ByteData", byteData);
        tEnv.addObjRelation("OutputStream", oPipe);

        //add relation for io.XOutputStream
        final XMultiServiceFactory msf = xMSF;
        final XInputStream xPipeInput = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class, oPipe);
        tEnv.addObjRelation("XOutputStream.StreamChecker",
            new ifc.io._XOutputStream.StreamChecker() {
                XInputStream xInStream = null;
                public void resetStreams() {
                    if (xInStream != null) {
                        try {
                            xInStream.closeInput();
                            xInStream = null;
                        } catch(com.sun.star.io.IOException e) {
                        }
                    } else {
                        try {
                            xPipeOutput.closeOutput();
                        } catch(com.sun.star.io.IOException e) {
                        }
                    }
                }

                public XInputStream getInStream() {
                    resetStreams();
                    try {
                        Object oInStream = msf.createInstance(
                            "com.sun.star.io.DataInputStream");
                        xInStream = (XInputStream) UnoRuntime.queryInterface
                            (XInputStream.class, oInStream);
                    } catch(com.sun.star.uno.Exception e) {
                        return null;
                    }

                    XActiveDataSink xDataSink = (XActiveDataSink)
                        UnoRuntime.queryInterface(
                            XActiveDataSink.class, xInStream);
                    xDataSink.setInputStream(xPipeInput);

                    return xInStream;
                }
            });

        return tEnv;
    } // finish method getTestEnvironment
}

