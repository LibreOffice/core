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
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.MarkableInputStream</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XInputStream</code></li>
*  <li> <code>com::sun::star::io::XMarkableStream</code></li>
*  <li> <code>com::sun::star::io::XConnectable</code></li>
*  <li> <code>com::sun::star::io::XActiveDataSink</code></li>
* </ul>
* @see com.sun.star.io.MarkableInputStream
* @see com.sun.star.io.XInputStream
* @see com.sun.star.io.XMarkableStream
* @see com.sun.star.io.XConnectable
* @see com.sun.star.io.XActiveDataSink
* @see ifc.io._XInputStream
* @see ifc.io._XMarkableStream
* @see ifc.io._XConnectable
* @see ifc.io._XActiveDataSink
*/
public class MarkableInputStream extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instances of services <code>com.sun.star.io.Pipe</code>,
    * <code>com.sun.star.io.MarkableInputStream</code> and
    * <code>com.sun.star.io.MarkableOutputStream</code>.
    * Plugs the created pipe as output stream for the created
    * <code>MarkableOutputStream</code>. Plugs the created pipe as input stream
    * for the created <code>MarkableInputStream</code>. Writes some data to the
    * <code>MarkableOutputStream</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'StreamWriter'</code> for
    *      {@link ifc.io._XInputStream}(a stream to write data to) </li>
    *  <li> <code>'ByteData'</code> for
    *      {@link ifc.io._XInputStream}(the data that should be written into
    *      the stream) </li>
    *  <li> <code>'Connectable'</code> for
    *      {@link ifc.io._XConnectable}(another object that can be connected) </li>
    *  <li> <code>'InputStream'</code> for
    *      {@link ifc.io._XActiveDataSink}(an input stream to set and get) </li>
    * </ul>
    * @see com.sun.star.io.Pipe
    * @see com.sun.star.io.MarkableInputStream
    * @see com.sun.star.io.MarkableOutputStream
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();

        Object aPipe = null;
        Object mostream = null;
        Object mistream = null;
        Object xConnect = null;
        try {
            aPipe = xMSF.createInstance("com.sun.star.io.Pipe");
            mistream = xMSF.createInstance("com.sun.star.io.MarkableInputStream");
            mostream = xMSF.createInstance("com.sun.star.io.MarkableOutputStream");
            xConnect = xMSF.createInstance("com.sun.star.io.DataInputStream");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e) ;
        }

        // Creating construction :
        // MarkableOutputStream -> Pipe -> MarkableInputStream
        XActiveDataSource xdSmo = (XActiveDataSource)
            UnoRuntime.queryInterface(XActiveDataSource.class, mostream);

        XOutputStream PipeOut = (XOutputStream)
            UnoRuntime.queryInterface(XOutputStream.class, aPipe);
        XInputStream PipeIn = (XInputStream)
            UnoRuntime.queryInterface(XInputStream.class, aPipe);

        xdSmo.setOutputStream(PipeOut);

        XActiveDataSink xmSi = (XActiveDataSink)
            UnoRuntime.queryInterface(XActiveDataSink.class, mistream);

        xmSi.setInputStream(PipeIn) ;

        XInterface oObj = (XInterface) mistream;

        byte[] byteData = new byte[] {1,2,3,4,5,6};

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // add a writer
        tEnv.addObjRelation("StreamWriter", mostream);
        // add a connectable
        tEnv.addObjRelation("Connectable", xConnect);
        // add an inputStream
        tEnv.addObjRelation("InputStream", mistream);
        tEnv.addObjRelation("ByteData", byteData);

        return tEnv;
    } // finish method getTestEnvironment

}

