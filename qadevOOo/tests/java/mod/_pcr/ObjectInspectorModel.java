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

package mod._pcr;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XFrame;
import com.sun.star.inspection.XObjectInspectorModel;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import helper.PropertyHandlerFactroy;
import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import util.DesktopTools;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.reflection.ObjectInspectorModel</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::inspection::XObjectInspectorModel</code></li>
 * </ul>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.inspection.XObjectInspectorModel
 */
public class ObjectInspectorModel extends TestCase {

    /**
     * module variable which holds the Desktop
     * @see com.sun.star.frame.Desktop
     */
    protected static Object StarDesktop = null;

    /**
     * assign to the module variable <CODE>StarDesktop</CODE> the desktop
     * @param Param the test parameters
     * @param log the log writer
     * @see lib.TestParameters
     * @see share.LogWriter
     * @see com.sun.star.frame.Desktop
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println("create a desktop...");
        StarDesktop = DesktopTools.createDesktop(Param.getMSF());
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.inspection.ObjectInspectorModel</code> with a <code>DefaultContext</code> and
     * <code>PropertyHandlerFactroy[]</code> as parameter
     *
     * @param tParam the tests parameter
     * @param log the logger
     * @return the test environment
     * @see util.DesktopTools
     * @see helper.PropertyHandlerImpl
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        this.cleanup(tParam, log);

        XMultiServiceFactory xMSF = tParam.getMSF();

        XPropertySet xMSFProp = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        XComponentContext xDefaultContext = null;
        // Get the default context from the office server.
        Object oDefaultContext = xMSFProp.getPropertyValue("DefaultContext");

        // Query for the interface XComponentContext.
        xDefaultContext = UnoRuntime.queryInterface(
                XComponentContext.class, oDefaultContext);

        Object[] oHandlerFactories = new Object[1];
        oHandlerFactories[0] = new PropertyHandlerFactroy();

        int minHelpTextLines = 200;
        int maxHelpTextLines = 400;

        XObjectInspectorModel oInspectorModel = com.sun.star.inspection.ObjectInspectorModel.
                createWithHandlerFactoriesAndHelpSection(xDefaultContext, oHandlerFactories,
                                                         minHelpTextLines, maxHelpTextLines);

        log.println("ImplementationName '" + utils.getImplName(oInspectorModel) + "'");

        TestEnvironment tEnv = new TestEnvironment(oInspectorModel);

        // com.sun.star.inspection.XObjectInspectorModel
        tEnv.addObjRelation("minHelpTextLines", Integer.valueOf(minHelpTextLines));
        tEnv.addObjRelation("maxHelpTextLines", Integer.valueOf(maxHelpTextLines));

        return tEnv;
    }

    /**
     * Closes the ObjectOnspector using <CODE>XCloseable</CODE>
     * @see com.sun.star.util.XCloseable
     * @param Param the test parameter
     * @param log the logger
     */
    @Override
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog if one exists ... ");

        XFrame existentInspector = null;

        XFrame xFrame = UnoRuntime.queryInterface(XFrame.class, StarDesktop);

        existentInspector = xFrame.findFrame( "ObjectInspector", 255 );

        if ( existentInspector != null ){
            XCloseable closer = UnoRuntime.queryInterface(
                    XCloseable.class, existentInspector);
            try{
                closer.close(true);
            } catch (CloseVetoException e){
                log.println("Could not close inspector: " + e.toString());
            }
        }
    }
}
