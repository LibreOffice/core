/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LayerWriter.java,v $
 * $Revision: 1.4 $
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
package mod._cfgmgr2;

import com.sun.star.io.XActiveDataSource;
import com.sun.star.io.XOutputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

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
