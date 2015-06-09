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

package mod._stm;

import java.io.PrintWriter;
import java.util.ArrayList;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XDataOutputStream;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.DataInputStream</code>.
* <ul>
*  <li> <code>com::sun::star::io::XInputStream</code></li>
*  <li> <code>com::sun::star::io::XDataInputStream</code></li>
*  <li> <code>com::sun::star::io::XConnectable</code></li>
*  <li> <code>com::sun::star::io::XActiveDataSink</code></li>
* </ul>
* @see com.sun.star.io.DataInputStream
* @see com.sun.star.io.XInputStream
* @see com.sun.star.io.XDataInputStream
* @see com.sun.star.io.XConnectable
* @see com.sun.star.io.XActiveDataSink
* @see ifc.io._XInputStream
* @see ifc.io._XDataInputStream
* @see ifc.io._XConnectable
* @see ifc.io._XActiveDataSink
*/
public class DataInputStream extends TestCase {

    /**
    * Creates a TestEnvironment for the interfaces to be tested.
    * Creates <code>com.sun.star.io.DataInputStream</code> object,
    * connects it to <code>com.sun.star.io.DataOutputStream</code>
    * through <code>com.sun.star.io.Pipe</code>. All of possible data
    * types are written into <code>DataOutputStream</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'StreamData'</code> for
    *      {@link ifc.io._XDataInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'StreamWriter'</code> for
    *      {@link ifc.io._XDataInputStream}
    *      {@link ifc.io._XInputStream}(a stream to write data to) </li>
    *  <li> <code>'Connectable'</code> for
    *      {@link ifc.io._XConnectable}(another object that can be connected) </li>
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XActiveDataSink}(an input stream to set and get) </li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        Object oInterface = null;

        XMultiServiceFactory xMSF = Param.getMSF();
        oInterface = xMSF.createInstance("com.sun.star.io.DataInputStream");

        XInterface oObj = (XInterface) oInterface;

        // creating and connecting DataOutputStream to the
        // DataInputStream created through the Pipe
        XActiveDataSink xDataSink = UnoRuntime.queryInterface(XActiveDataSink.class, oObj);

        XInterface oPipe = (XInterface)
                xMSF.createInstance("com.sun.star.io.Pipe");

        XInputStream xPipeInput = UnoRuntime.queryInterface(XInputStream.class, oPipe);
        XOutputStream xPipeOutput = UnoRuntime.queryInterface(XOutputStream.class, oPipe);

        XInterface oDataOutput = (XInterface)
                xMSF.createInstance("com.sun.star.io.DataOutputStream");

        XDataOutputStream xDataOutput = UnoRuntime.queryInterface(XDataOutputStream.class, oDataOutput) ;
        XActiveDataSource xDataSource = UnoRuntime.queryInterface(XActiveDataSource.class, oDataOutput) ;

        xDataSource.setOutputStream(xPipeOutput) ;
        xDataSink.setInputStream(xPipeInput) ;

        // all data types for writing to an XDataInputStream
        ArrayList<Object> data = new ArrayList<Object>();
        data.add(Boolean.TRUE) ;
        data.add(Byte.valueOf((byte)123)) ;
        data.add(new Character((char)1234)) ;
        data.add(Short.valueOf((short)1234)) ;
        data.add(Integer.valueOf(123456)) ;
        data.add(new Float(1.234)) ;
        data.add(new Double(1.23456)) ;
        data.add("DataInputStream") ;
        // information for writing to the pipe
        byte[] byteData = new byte[] {
            1, 2, 3, 4, 5, 6, 7, 8 } ;

        // creating a connectable object for XConnectable interface
        XInterface xConnect = (XInterface)xMSF.createInstance(
                                    "com.sun.star.io.DataInputStream") ;

        // creating an input stream to set in XActiveDataSink
        XInterface oDataInput = (XInterface) xMSF.createInstance(
                                        "com.sun.star.io.Pipe" );


        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding sequence of data that must be read
        // by XDataInputStream interface methods
        tEnv.addObjRelation("StreamData", data) ;
        // add a writer
        tEnv.addObjRelation("StreamWriter", xDataOutput);
        // add a connectable
        tEnv.addObjRelation("Connectable", xConnect);
        // add an inputStream
        tEnv.addObjRelation("InputStream", oDataInput);
        tEnv.addObjRelation("ByteData", byteData);

        return tEnv;
    } // finish method getTestEnvironment

}

