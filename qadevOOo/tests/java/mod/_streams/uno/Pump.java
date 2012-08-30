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

import com.sun.star.io.NotConnectedException;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.Pump</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XActiveDataSource</code></li>
*  <li> <code>com::sun::star::io::XActiveDataControl</code></li>
*  <li> <code>com::sun::star::io::XActiveDataSink</code></li>
* </ul>
* @see com.sun.star.io.Pump
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XActiveDataControl
* @see com.sun.star.io.XActiveDataSink
* @see ifc.io._XActiveDataSource
* @see ifc.io._XActiveDataControl
* @see ifc.io._XActiveDataSink
*/
public class Pump extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service <code>com.sun.star.io.Pump</code>.
    * Settings up input and output streams for the created pump.
    * Object relations created :
    * <ul>
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XActiveDataSource}(an input stream to set) </li>
    *  <li> <code>'OutputStream'</code> for
    *      {@link ifc.io._XActiveDataSource}(an output stream to set) </li>
    * </ul>
    * @see com.sun.star.io.Pump
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        XInterface oPipe;

        // creating an instance of stm.Pump
        try {
            oInterface = xMSF.createInstance( "com.sun.star.io.Pump" );
            oPipe = (XInterface) xMSF.createInstance( "com.sun.star.io.Pipe" );
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create the needed objects.", e) ;
        }


        XInterface oObj = (XInterface) oInterface;

        // setting up input and output streams for pump
        XActiveDataSink xSink = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, oObj);
        XActiveDataSource xSource = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, oObj);

        XInputStream xInput = new MyInput();
        XOutputStream xOutput = new MyOutput();

        xSink.setInputStream(xInput);
        xSource.setOutputStream(xOutput);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // add object relations for ActiveDataSource and XActiveDataSink
        tEnv.addObjRelation("InputStream", oPipe);
        tEnv.addObjRelation("OutputStream", oPipe);

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * XInputStream implementation to use with the test. It returns bytes of
    * which a simple string consists.
    */
    private static class MyInput implements XInputStream  {
        String str = "Pump tesing string" ;

        public int readBytes(byte[][] bytes, int len)
            throws NotConnectedException{

            if (str == null)
                throw new NotConnectedException("Input stream was closed");

            int actual = 0 ;
            if (len <= str.length()) {
                String resStr = str.substring(0, len-1) ;
                bytes[0] = resStr.getBytes() ;
                actual = len ;
                str = str.substring(len) ;
            } else {
                bytes[0] = str.getBytes() ;
                actual = str.length() ;
            }

            return actual;
        }

        public int readSomeBytes(byte[][] bytes, int len)
            throws NotConnectedException{
            return readBytes(bytes, len);
        }

        public void skipBytes(int len) throws NotConnectedException {
            if (str == null)
                throw new NotConnectedException("Stream was closed.") ;

            if (len >= str.length())
                str = "" ;
            else
                str = str.substring(len) ;
        }

        public void closeInput() throws NotConnectedException {
            if (str == null)
                throw new NotConnectedException("Stream was closed.") ;

            str = null ;
        }

        public int available() throws NotConnectedException {
            if (str == null)
                throw new NotConnectedException("Stream was closed.") ;

            return str.length();
        }
    }

    /**
    * Dummy XOutputStream implementation to use with the test. Does nothing.
    */
    private static class MyOutput implements XOutputStream {
        public void writeBytes(byte[] bytes) {
        }

        public void flush() {
        }

        public void closeOutput() {
        }
    }
}

