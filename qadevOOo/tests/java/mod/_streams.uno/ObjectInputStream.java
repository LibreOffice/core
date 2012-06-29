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
* <code>com.sun.star.io.ObjectInputStream</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XInputStream</code></li>
*  <li> <code>com::sun::star::io::XMarkableStream</code></li>
*  <li> <code>com::sun::star::io::XDataInputStream</code></li>
*  <li> <code>com::sun::star::io::XConnectable</code></li>
*  <li> <code>com::sun::star::io::XActiveDataSink</code></li>
*  <li> <code>com::sun::star::io::XObjectInputStream</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> MyPersistObjectImpl.jar </b> : the implementation of the persist
*       object</li>
* </ul> <p>
* @see com.sun.star.io.ObjectInputStream
* @see com.sun.star.io.XInputStream
* @see com.sun.star.io.XMarkableStream
* @see com.sun.star.io.XDataInputStream
* @see com.sun.star.io.XConnectable
* @see com.sun.star.io.XActiveDataSink
* @see com.sun.star.io.XObjectInputStream
* @see ifc.io._XInputStream
* @see ifc.io._XMarkableStream
* @see ifc.io._XDataInputStream
* @see ifc.io._XConnectable
* @see ifc.io._XActiveDataSink
* @see ifc.io._XObjectInputStream
*/
public class ObjectInputStream extends TestCase {

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
    * <code>com.sun.star.io.ObjectInputStream</code>,
    * <code>com.sun.star.io.ObjectOutputStream</code>,
    * <code>com.sun.star.io.Pipe</code>,
    * <code>com.sun.star.io.MarkableInputStream</code> and
    * <code>com.sun.star.io.MarkableOutputStream</code>. Plugs the created
    * markable output stream as output stream for the created
    * <code>ObjectOutputStream</code>. Plugs the created pipe as output stream
    * for the created <code>MarkableOutputStream</code>. Plugs the created
    * markable input stream as input stream for the created
    * <code>ObjectInputStream</code>. Plugs the created pipe as input stream
    * for the created <code>MarkableInputStream</code>. Creates an instance
    * of the service <code>com.sun.star.cmp.PersistObject</code> and writes
    * the created object to the object output stream.
    * Object relations created :
    * <ul>
    *  <li> <code>'PersistObject'</code> for
    *      {@link ifc.io._XObjectInputStream}(the created instance of the
    *      persist object ) </li>
    *  <li> <code>'StreamData'</code> for
    *      {@link ifc.io._XDataInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'StreamWriter'</code> for
    *      {@link ifc.io._XDataInputStream}
    *      {@link ifc.io._XObjectInputStream}
    *      {@link ifc.io._XInputStream}(a stream to write data to) </li>
    *  <li> <code>'Connectable'</code> for
    *      {@link ifc.io._XConnectable}
    *      (another object that can be connected) </li>
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XActiveDataSink}(an input stream to set and get) </li>
    * </ul>
    * @see com.sun.star.io.ObjectInputStream
    * @see com.sun.star.io.ObjectOutputStream
    * @see com.sun.star.io.Pipe
    * @see com.sun.star.io.MarkableInputStream
    * @see com.sun.star.io.MarkableOutputStream
    * @see com.sun.star.cmp.PersistObject
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        System.out.println("create TestEnvironment started.");
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
        Object ostream = null;
        Object aPipe = null;
        Object mostream = null;
        Object mistream = null;
        Object istream = null;
        Object xConnect = null;
        try {
            istream = xMSF.createInstance
                ("com.sun.star.io.ObjectInputStream");
            ostream = xMSF.createInstance
                ("com.sun.star.io.ObjectOutputStream");
            aPipe = xMSF.createInstance
                ("com.sun.star.io.Pipe");
            mistream = xMSF.createInstance
                ("com.sun.star.io.MarkableInputStream");
            mostream = xMSF.createInstance
                ("com.sun.star.io.MarkableOutputStream");
            xConnect = (XInterface)xMSF.createInstance
                ("com.sun.star.io.DataInputStream") ;

        } catch( com.sun.star.uno.Exception e ) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }
        // Creating construction :
        // ObjectOutputStream -> MarkableOutputStream -> Pipe ->
        // -> MarkableInputStream -> ObjectInputStream
        XActiveDataSource xdSo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, ostream);

        XActiveDataSource xdSmo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, mostream);

        XOutputStream moStream = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class, mostream);

        XOutputStream PipeOut = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class, aPipe);
        XInputStream PipeIn = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class, aPipe);

        xdSo.setOutputStream(moStream);
        xdSmo.setOutputStream(PipeOut);

        XObjectInputStream iStream = (XObjectInputStream)
            UnoRuntime.queryInterface(XObjectInputStream.class, istream);
        XObjectOutputStream oStream = null;
        oStream = (XObjectOutputStream)
            UnoRuntime.queryInterface(XObjectOutputStream.class, ostream);

        XActiveDataSink xmSi = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, mistream);
        XInputStream xmIstream = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class, mistream);

        XActiveDataSink xdSi = (XActiveDataSink) UnoRuntime.queryInterface
            (XActiveDataSink.class, istream);
        xdSi.setInputStream(xmIstream);
        xmSi.setInputStream(PipeIn);

        // creating Persist object which has to be written
        XPersistObject xPersObj = null;
        try {
            Object oPersObj = xMSF.createInstance
                ("com.sun.star.cmp.PersistObject");
            xPersObj = (XPersistObject)
                UnoRuntime.queryInterface(XPersistObject.class, oPersObj);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't write persist object.", e);
        }



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


        System.out.println("create environment");
        XInterface oObj = iStream;
        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding persistent object
        tEnv.addObjRelation("PersistObject", xPersObj);
        // add a connectable
        tEnv.addObjRelation("Connectable", xConnect);
        tEnv.addObjRelation("StreamWriter", oStream);
        // for XActiveDataSink
        tEnv.addObjRelation("InputStream", aPipe);
        // adding sequence of data that must be read
        // by XDataInputStream interface methods
        tEnv.addObjRelation("StreamData", data) ;
        // and by XInputStream interface methods
        tEnv.addObjRelation("ByteData", byteData) ;

        System.out.println("create TestEnvironment finished.");
        return tEnv;
    } // finish method getTestEnvironment

}

