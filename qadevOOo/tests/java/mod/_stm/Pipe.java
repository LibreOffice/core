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

package mod._stm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.io.Pipe</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XInputStream</code></li>
*  <li> <code>com::sun::star::io::XOutputStream</code></li>
* </ul>
* @see com.sun.star.io.Pipe
* @see com.sun.star.io.XInputStream
* @see com.sun.star.io.XOutputStream
* @see ifc.io._XInputStream
* @see ifc.io._XOutputStream
*/
public class Pipe extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service <code>com.sun.star.io.Pipe</code>.
    * Writes some information to the created pipe.
    * Object relations created :
    * <ul>
    *  <li> <code>'XOutputStream.StreamChecker'</code> for
    *      {@link ifc.io._XOutputStream}( implementation of the interface
    *      ifc.io._XOutputStream.StreamChecker ) </li>
    * </ul>
    * @see com.sun.star.io.Pipe
    */
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance( "com.sun.star.io.Pipe" );
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        oObj = (XInterface) oInterface;

        // writing some information to the pipe
        byte[] byteData = new byte[] {
            1, 2, 3, 4, 5, 6, 7, 8 } ;

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //add relation for io.XOutputStream
        final XInputStream iStream = (XInputStream)
                UnoRuntime.queryInterface(XInputStream.class, oObj);

        tEnv.addObjRelation("ByteData", byteData);
        tEnv.addObjRelation("StreamWriter", oObj);

        tEnv.addObjRelation("XOutputStream.StreamChecker",
            new ifc.io._XOutputStream.StreamChecker() {
                XInputStream inStream = null;
                public void resetStreams() {
                }

                public XInputStream getInStream() {
                    return iStream;
                }
            });

        return tEnv;
    } // finish method getTestEnvironment

}

