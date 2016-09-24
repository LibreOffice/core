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

import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.frame.XDesktop;
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
 * threads concurrently.
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.inspection.XObjectInspector
 * @see com.sun.star.frame.XController
 */
public class ObjectInspector extends TestCase {

    /**
     * module variable which holds the Desktop
     * @see com.sun.star.frame.Desktop
     */
    protected static XDesktop xDesktop = null;

    /**
     * assign to the module variable <CODE>xDesktop</CODE> the desktop
     * @param Param the test parameters
     * @param log the log writer
     * @see lib.TestParameters
     * @see share.LogWriter
     * @see com.sun.star.frame.Desktop
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println("create a desktop...");
        xDesktop = DesktopTools.createDesktop(Param.getMSF());
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.inspection.ObjectInspector</code> and adds it to
     * a floating frame.<br>
     * To test <CODE>com.sun.star.inspection.XObjectInspector.inspect()</CODE>
     * an own implementation of
     * <CODE>com.sun.star.inspection.XPropertyHandler</CODE> was used.
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

        XInterface oInspector = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspector");

        XObjectInspector xInspector = UnoRuntime.queryInterface(XObjectInspector.class, oInspector);

        log.println("ImplementationName '" + utils.getImplName(xInspector) + "'");

        XInterface oInspectorModel = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspectorModel");

        XObjectInspectorModel xInspectorModel = UnoRuntime.queryInterface(XObjectInspectorModel.class, oInspectorModel);

        XInterface oInspectorModelToSet = (XInterface) xMSF.createInstance("com.sun.star.inspection.ObjectInspectorModel");

        XObjectInspectorModel xInspectorModelToSet = UnoRuntime.queryInterface(XObjectInspectorModel.class, oInspectorModelToSet);


        log.println("create a floating frame...");

        XWindowPeer xWindowPeer = DesktopTools.createFloatingWindow(xMSF);

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, xWindowPeer);

        XInterface oFrame = (XInterface) xMSF.createInstance("com.sun.star.frame.Frame");

        XFrame xFrame = UnoRuntime.queryInterface(XFrame.class, oFrame);

        xFrame.setName("ObjectInspector");
        xFrame.initialize(xWindow);

        XFramesSupplier xFramesSup = UnoRuntime.queryInterface(XFramesSupplier.class, xDesktop);

        XFrames xFrames = xFramesSup.getFrames();
        xFrames.append(xFrame);


        log.println("attach ObjectInspector to floating frame...");

        XInitialization xOII = UnoRuntime.queryInterface(XInitialization.class, xInspectorModel);

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

        XFrame xFrame = UnoRuntime.queryInterface(XFrame.class, xDesktop);

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
