/*************************************************************************
 *
 *  $RCSfile: Pump.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:32:27 $
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

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.io.NotConnectedException;
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

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
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

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

