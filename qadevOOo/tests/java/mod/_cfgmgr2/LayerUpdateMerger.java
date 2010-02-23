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
package mod._cfgmgr2;

import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XLayerHandler;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.XLayerHandlerImpl;
import util.XLayerImpl;

public class LayerUpdateMerger extends TestCase {

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
        XLayerHandler xLayerHandler = null;
        XLayer xLayer = null;

        try {
            oObj = (XInterface) ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                           "com.sun.star.comp.configuration.backend.LayerUpdateMerger");

            XInitialization xInit = (XInitialization) UnoRuntime.queryInterface(
                                            XInitialization.class, oObj);
            xLayerHandler = new XLayerHandlerImpl();
            xLayer = new XLayerImpl();

            xInit.initialize(new Object[] { xLayerHandler,xLayer });
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        log.println("Implementation name: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("LayerHandler", xLayerHandler);
                tEnv.addObjRelation("Layer", xLayer);

        return tEnv;
    }
}