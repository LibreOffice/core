/*************************************************************************
 *
 *  $RCSfile: MarkableOutputStream.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:31:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._stm;

import java.io.PrintWriter;
import java.util.Vector;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

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
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

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

