/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayerWriter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:51:49 $
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
package mod._cfgmgr2;

import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.File;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;


public class LayerWriter extends TestCase {
    public XOutputStream xStream = null;

    /**
    * Just clears flag which indicates that port is free now.
    */
    public synchronized void disposeTestEnvironment(TestEnvironment tEnv,
                                                    TestParameters tParam) {


        try {
            xStream.closeOutput();
        } catch (com.sun.star.io.NotConnectedException e) {
        } catch (com.sun.star.io.BufferSizeExceededException e) {
        } catch (com.sun.star.io.IOException e) {
        }
    }

    /** Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     *
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XSimpleFileAccess simpleAccess = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                           "com.sun.star.comp.configuration.backend.xml.LayerWriter");

            Object fileacc = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                                     "com.sun.star.comp.ucb.SimpleFileAccess");
            simpleAccess = (XSimpleFileAccess) UnoRuntime.queryInterface(
                                   XSimpleFileAccess.class, fileacc);

            String filename = util.utils.getOfficeTemp(
                                      (XMultiServiceFactory) tParam.getMSF()) +
                              "LayerWriter.xcu";
            log.println("Going to parse: " + filename);

            xStream = simpleAccess.openFileWrite(filename);
        } catch (com.sun.star.uno.Exception e) {
        }

        log.println("Implementation name: " + util.utils.getImplName(oObj));

        XActiveDataSource xSource = (XActiveDataSource) UnoRuntime.queryInterface(
                                            XActiveDataSource.class, oObj);
        xSource.setOutputStream(xStream);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OutputStream", xStream);

        return tEnv;
    }
}