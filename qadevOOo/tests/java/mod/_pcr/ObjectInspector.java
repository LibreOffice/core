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

import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFrames;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.inspection.XObjectInspector;
import com.sun.star.inspection.XObjectInspectorModel;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import helper.PropertyHandlerImpl;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import util.DesktopTools;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.reflection.ObjectInspector</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::inspection::XObjectInspector</code></li>
 *  <li> <code>com::sun::star::frame::XController</code></li>
 * </ul>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.inspection.XObjectInspector
 * @see com.sun.star.frame.XController
 */
public class ObjectInspector extends TestCase {

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
     * <code>com.sun.star.inspection.ObjectInspector</code> and adds it to
     * a floating frame.<br>
     * To test <CODE>com.sun.star.inspection.XObjectInspector.inspect()</CODE>
     * an own implementation of
     * <CODE>com.sun.star.inspection.XPropertyHandler</CODE> was used.
     * @param tParam the tests parameter
     * @param log the logger
     * @return the test environement
     * @see util.DesktopTools
     * @see helper.PropertyHandlerImpl
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        this.cleanup(tParam, log);

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface oInspector = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspector");

            XObjectInspector xInspector = (XObjectInspector) UnoRuntime.queryInterface(XObjectInspector.class, oInspector);

            log.println("ImplementationName '" + utils.getImplName(xInspector) + "'");

            XInterface oInspectorModel = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspectorModel");

            XObjectInspectorModel xInspectorModel = (XObjectInspectorModel)
            UnoRuntime.queryInterface(XObjectInspectorModel.class, oInspectorModel);

            XInterface oInspectorModelToSet = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspectorModel");

            XObjectInspectorModel xInspectorModelToSet = (XObjectInspectorModel)
            UnoRuntime.queryInterface(XObjectInspectorModel.class, oInspectorModelToSet);


            log.println("create a floating frame...");

            XWindow xWindow = null;
            try{

                XWindowPeer xWindowPeer = DesktopTools.createFloatingWindow(xMSF);

                xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, xWindowPeer);

            } catch (StatusException e){
                throw new StatusException("Coud not create test object", e);
            }

            XInterface oFrame = (XInterface) xMSF.createInstance("com.sun.star.frame.Frame");

            XFrame xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, oFrame);

            xFrame.setName("ObjectInspector");
            xFrame.initialize(xWindow);

            XFramesSupplier xFramesSup = (XFramesSupplier) UnoRuntime.queryInterface(XFramesSupplier.class, StarDesktop);

            XFrames xFrames = xFramesSup.getFrames();
            xFrames.append(xFrame);


            log.println("attach ObjectInspector to floating frame...");

            XInitialization xOII = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, xInspectorModel);

            xOII.initialize(new Object[0]);

            xInspector.setInspectorModel(xInspectorModel);

            // for debug purposes the following lines could commented out. But in
            // this case the com.sun.star.frame.XController would be failed!
            //xInspector.attachFrame(xFrame);
            //xWindow.setVisible(true);

            Object[] oInspect = new Object[1];
            oInspect[0] = new PropertyHandlerImpl();

            TestEnvironment tEnv = new TestEnvironment(xInspector);

            // com.sun.star.frame.XController
            tEnv.addObjRelation("Frame",xFrame);

            tEnv.addObjRelation("XObjectInspector.toInspect", oInspect);

            tEnv.addObjRelation("XObjectInspector.InspectorModelToSet", xInspectorModelToSet);

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
