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
import com.sun.star.io.XObjectInputStream;
import com.sun.star.io.XObjectOutputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XPersistObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.XImplementationRegistration;
import com.sun.star.registry.XSimpleRegistry;
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
* <code>com.sun.star.io.ObjectOutputStream</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XActiveDataSource</code></li>
*  <li> <code>com::sun::star::io::XOutputStream</code></li>
*  <li> <code>com::sun::star::io::XConnectable</code></li>
*  <li> <code>com::sun::star::io::XDataOutputStream</code></li>
*  <li> <code>com::sun::star::io::XObjectOutputStream</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> MyPersistObjectImpl.jar </b> : the implementation of the persist
*       object</li>
* </ul> <p>
* @see com.sun.star.io.ObjectOutputStream
* @see com.sun.star.io.XActiveDataSource
* @see com.sun.star.io.XOutputStream
* @see com.sun.star.io.XConnectable
* @see com.sun.star.io.XDataOutputStream
* @see com.sun.star.io.XObjectOutputStream
* @see ifc.io._XActiveDataSource
* @see ifc.io._XOutputStream
* @see ifc.io._XConnectable
* @see ifc.io._XDataOutputStream
* @see ifc.io._XObjectOutputStream
*/
public class ObjectOutputStream extends TestCase {

    /**
    * Register the implementation of service
    * <code>com.sun.star.cmp.PersistObject</code> if not yet registered.
    * @see com.sun.star.cmp.PersistObject
    */
    public void initialize(TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        Object oPersObj = null;
        // test first if object is already registered
        try {
            oPersObj = xMSF.createInstance("com.sun.star.cmp.PersistObject");
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Could not create instance of PersistObject");
            e.printStackTrace(log);
            log.println("Going on with test...");
        }
        if ( oPersObj == null ) {
            // object is  not available: it has to be registered
            String url = util.utils.getFullTestURL
                ("qadevlibs/MyPersistObjectImpl.jar");
            XImplementationRegistration xir;
            try {
                Object o = xMSF.createInstance(
                        "com.sun.star.registry.ImplementationRegistration");
                xir = (XImplementationRegistration)
                                    UnoRuntime.queryInterface(
                                    XImplementationRegistration.class, o);
            }
            catch (com.sun.star.uno.Exception e) {
                System.err.println(
                            "Couldn't create implementation registration");
                e.printStackTrace();
                throw new StatusException("Couldn't create ImplReg", e);
            }

            XSimpleRegistry xReg = null;
            try {
                System.out.println("Register library: " + url);
                xir.registerImplementation(
                                    "com.sun.star.loader.Java2", url, xReg);
                System.out.println("...done");
            } catch (CannotRegisterImplementationException e) {
                System.err.println("Name: " + url + "  msg: " +
                                    e.getMessage());
                e.printStackTrace();
                throw new StatusException(
                                    "Couldn't register MyPersistObject", e);
            }
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instances of services
    * <code>com.sun.star.io.ObjectOutputStream</code>,
    * <code>com.sun.star.io.Pipe</code> and
    * <code>com.sun.star.io.MarkableOutputStream</code>. Plugs the created
    * markable output stream as output stream for the created
    * <code>ObjectOutputStream</code>. Plugs the created pipe as output stream
    * for the created <code>MarkableOutputStream</code>. Creates an instance
    * of the service <code>com.sun.star.cmp.PersistObject</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'StreamData'</code> for
    *      {@link ifc.io._XDataOutputStream}(the data that should
    *      be written into the stream) </li>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XOutputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'Connectable'</code> for
    *      {@link ifc.io._XConnectable}
    *       (another object that can be connected) </li>
    *  <li> <code>'OutputStream'</code> for
    *      {@link ifc.io._XActiveDataSource}
    *       (an input stream to set and get) </li>
    *  <li> <code>'PersistObject'</code> for
    *      {@link ifc.io._XObjectOutputStream}(the created instance of the
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XObjectInputStream}(the created instance of the
    *      persist object ) </li>
    *  <li> <code>'XOutputStream.StreamChecker'</code> for
    *      {@link ifc.io._XOutputStream}( implementation of the interface
    *      ifc.io._XOutputStream.StreamChecker ) </li>
    * </ul>
    * @see com.sun.star.io.ObjectInputStream
    * @see com.sun.star.io.ObjectOutputStream
    * @see com.sun.star.io.Pipe
    * @see com.sun.star.io.MarkableInputStream
    * @see com.sun.star.io.MarkableOutputStream
    * @see com.sun.star.cmp.PersistObject
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XObjectOutputStream oStream = null;

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        Object ostream = null, istream = null;
        Object aPipe = null;
        Object mostream = null;
        XInterface aConnect = null;
        Object minstream = null;

        try {
            ostream = xMSF.createInstance
                ( "com.sun.star.io.ObjectOutputStream" );
            istream = xMSF.createInstance
                ("com.sun.star.io.ObjectInputStream");
            aPipe = xMSF.createInstance("com.sun.star.io.Pipe");
            mostream = xMSF.createInstance
                ("com.sun.star.io.MarkableOutputStream");
            aConnect = (XInterface)xMSF.createInstance
                ("com.sun.star.io.DataInputStream");
            minstream = xMSF.createInstance
                ("com.sun.star.io.MarkableInputStream");
        } catch( com.sun.star.uno.Exception e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        // creating the pipe where object has to be written to
        XActiveDataSource xdSo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, ostream);

        XActiveDataSource xdSmo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, mostream);

        XOutputStream moStream = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class, mostream);

        XActiveDataSink markIn = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, minstream);
        XActiveDataSink inStream = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, istream);
        XInputStream markInStream = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class, minstream);

        final XOutputStream PipeOut = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class,aPipe);
        final XInputStream PipeIn = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class,aPipe);

        markIn.setInputStream(PipeIn);
        inStream.setInputStream(markInStream);
        XObjectInputStream objInputStream = (XObjectInputStream)
            UnoRuntime.queryInterface(XObjectInputStream.class, istream);
        xdSo.setOutputStream(moStream);
        xdSmo.setOutputStream(PipeOut);

        oStream = (XObjectOutputStream)
            UnoRuntime.queryInterface(XObjectOutputStream.class, ostream);

        // creating Persistent object which has to be written
        XPersistObject xPersObj = null ;
        try {
            Object oPersObj = xMSF.createInstance
                ("com.sun.star.cmp.PersistObject");
            xPersObj = (XPersistObject)
                UnoRuntime.queryInterface(XPersistObject.class, oPersObj);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write persist object.", e) ;
        }

        oObj = oStream;

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

        tEnv.addObjRelation("PersistObject", xPersObj);
        tEnv.addObjRelation("StreamData", data);
        tEnv.addObjRelation("ByteData", byteData);
        tEnv.addObjRelation("OutputStream", aPipe);
        tEnv.addObjRelation("Connectable", aConnect);

        tEnv.addObjRelation("InputStream", objInputStream);

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
                            "com.sun.star.io.ObjectInputStream");
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

