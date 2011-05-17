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

package mod._pcr;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XFrame;
import com.sun.star.inspection.XObjectInspectorModel;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import helper.PropertyHandlerFactroy;
import java.io.PrintWriter;

import lib.StatusException;
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
 * threads concurently.
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
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println("create a desktop...");
        StarDesktop = DesktopTools.createDesktop((XMultiServiceFactory) Param.getMSF());
        if (StarDesktop == null){
            throw new StatusException("Could not get a Desktop: null", null);
        }
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.inspection.ObjectInspectorModel</code> with a <code>DefaultContext</code> and
     * <code>PropertyHandlerFactroy[]</code> as parameter
     *
     * @param tParam the tests parameter
     * @param log the logger
     * @return the test environement
     * @see util.DesktopTools
     * @see helper.PropertyHandlerImpl
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        this.cleanup(tParam, log);

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        XPropertySet xMSFProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        XComponentContext xDefaultContext = null;
        try{
            // Get the default context from the office server.
            Object oDefaultContext = xMSFProp.getPropertyValue("DefaultContext");

            // Query for the interface XComponentContext.
            xDefaultContext = (XComponentContext) UnoRuntime.queryInterface(
                    XComponentContext.class, oDefaultContext);

        } catch (UnknownPropertyException e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        } catch (WrappedTargetException e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        } catch (Exception e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        }

        try {

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
            tEnv.addObjRelation("minHelpTextLines", new Integer(minHelpTextLines));
            tEnv.addObjRelation("maxHelpTextLines", new Integer(maxHelpTextLines));

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }

    /**
     * Closes the ObjectOnspector using <CODE>XCloseable</CODE>
     * @see com.sun.star.util.XCloseable
     * @param Param the test parameter
     * @param log the logger
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog if one exists ... ");

        XFrame existentInspector = null;

        XFrame xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, StarDesktop);

        existentInspector = xFrame.findFrame( "ObjectInspector", 255 );

        if ( existentInspector != null ){
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                    XCloseable.class, existentInspector);
            try{
                closer.close(true);
            } catch (CloseVetoException e){
                log.println("Could not close inspector: " + e.toString());
            }
        }
    }
}
