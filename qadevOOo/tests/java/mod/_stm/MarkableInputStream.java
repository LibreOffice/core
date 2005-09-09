/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MarkableInputStream.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:25:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._stm;

import java.io.PrintWriter;

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
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();;

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

