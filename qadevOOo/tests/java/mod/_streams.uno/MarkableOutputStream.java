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

package mod._streams.uno;

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import java.util.Vector;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.MarkableOutputStream</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XMarkableStream</code></li>
*  <li> <code>com::sun::star::io::XActiveDataSource</code></li>
*  <li> <code>com::sun::star::io::XOutputStream</code></li>
*  <li> <code>com::sun::star::io::XConnectable</code></li>
* </ul>
* @see com.sun.star.io.MarkableOutputStream
* @see com.sun.star.io.XMarkableStream
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XOutputStream
* @see com.sun.star.io.XConnectable
* @see ifc.io._XMarkableStream
* @see ifc.io._XActiveDataSource
* @see ifc.io._XOutputStream
* @see ifc.io._XConnectable
*/
public class MarkableOutputStream extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instances of services <code>com.sun.star.io.Pipe</code>,
    * <code>com.sun.star.io.MarkableInputStream</code> and
    * <code>com.sun.star.io.MarkableOutputStream</code>.
    * Plugs the created pipe as output stream for the created
    * <code>MarkableOutputStream</code>. Plugs the created pipe as input stream
    * for the created <code>MarkableInputStream</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XOutputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'StreamData'</code> for
    *      {@link ifc.io._XDataOutputStream}(the data that should be
    *      written into the stream) </li>
    *  <li> <code>'Connectable'</code> for
    *      {@link ifc.io._XConnectable}
    *      (another object that can be connected) </li>
    *  <li> <code>'OutputStream'</code> for
    *      {@link ifc.io._XActiveDataSource}
    *      (an input stream to set and get) </li>
    *  <li> <code>'XOutputStream.StreamChecker'</code> for
    *      {@link ifc.io._XOutputStream}( implementation of the interface
    *      ifc.io._XOutputStream.StreamChecker ) </li>
    * </ul>
    * @see com.sun.star.io.Pipe
    * @see com.sun.star.io.MarkableInputStream
    * @see com.sun.star.io.MarkableOutputStream
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        Object aPipe = null;
        Object mostream = null;
        Object mistream = null;
        XInterface aConnect;

        try {
            aPipe = xMSF.createInstance("com.sun.star.io.Pipe");
            mistream = xMSF.createInstance
                ("com.sun.star.io.MarkableInputStream");
            mostream = xMSF.createInstance
                ("com.sun.star.io.MarkableOutputStream");
            aConnect = (XInterface)xMSF.createInstance
                ("com.sun.star.io.DataOutputStream");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e) ;
        }

        // Creating construction :
        // MarkableOutputStream -> Pipe -> MarkableInputStream
        XActiveDataSource xdSmo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, mostream);

        final XOutputStream PipeOut = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class,aPipe);
        final XInputStream PipeIn = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class,aPipe);

        xdSmo.setOutputStream(PipeOut);

        XActiveDataSink xmSi = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, mistream);

        xmSi.setInputStream(PipeIn) ;

        oObj = (XInterface) mostream;

        // all data types for writing to an XDataInputStream
        Vector data = new Vector() ;
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

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("StreamData", data);
        tEnv.addObjRelation("ByteData", byteData);
        tEnv.addObjRelation("OutputStream", aPipe);
        tEnv.addObjRelation("Connectable", aConnect);

        //add relation for io.XOutputStream
        final XMultiServiceFactory msf = xMSF;
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
                            PipeOut.closeOutput();
                        } catch(com.sun.star.io.IOException e) {
                        }
                    }
                }

                public XInputStream getInStream() {
                    resetStreams();
                    try {
                        Object oInStream = msf.createInstance(
                            "com.sun.star.io.MarkableInputStream");
                        xInStream = (XInputStream) UnoRuntime.queryInterface
                            (XInputStream.class, oInStream);
                    } catch(com.sun.star.uno.Exception e) {
                        return null;
                    }

                    XActiveDataSink xDataSink = (XActiveDataSink)
                        UnoRuntime.queryInterface(
                            XActiveDataSink.class, xInStream);
                    xDataSink.setInputStream(PipeIn);

                    return xInStream;
                }
            });

        return tEnv;
    } // finish method getTestEnvironment

}

