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

package mod._svx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import com.sun.star.view.XSelectionSupplier;


/**
 *
 * @author  sw93809
 * @version
 */
public class SvxGraphCtrlAccessibleContext extends TestCase{

    static XComponent xDrawDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Switches the document to Print Preview mode.
    * Obtains accessible object for the page view.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment
    */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;


        XModel aModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);

        XController xController = aModel.getCurrentController();

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        final XShape oShape = SOF.createShape(xDrawDoc,
            5000,5000,1500,1000,"GraphicObject");


        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        XPropertySet shapeProps = UnoRuntime.queryInterface(XPropertySet.class, oShape);

        String url = util.utils.getFullTestURL("space-metal.jpg");

        log.println("Inserting Graphic: "+url);

        try {
            shapeProps.setPropertyValue("GraphicURL", url);
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Property GraphicURL is unknown");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println("Property GraphicURL is read only");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Property GraphicURL tried to set to illegal argument");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("Wrapped Target Exception was thrown while setting Property GraphicURL");
        }


        //Opening ImageMapDialog
        try {
            String aSlotID = "slot:10371";
            XDispatchProvider xDispProv = UnoRuntime.queryInterface( XDispatchProvider.class, xController );
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
         ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = aSlotID;
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
            if( xDispatcher != null )
                    xDispatcher.dispatch( aURL, null );
            utils.shortWait(Param.getInt(util.PropertyName.SHORT_WAIT));
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
        }


        try {
            oObj = (XInterface) ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class,oObj);


        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //selecting the inserted shape
        final XSelectionSupplier SelSupp = UnoRuntime.queryInterface(XSelectionSupplier.class,xController);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        SelSupp.select(oShape);
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                        System.out.println("Couldn't select shape");
                    }
                }
            });

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes text document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);

        XController xController = aModel.getCurrentController();

        //Closing ImageMapDialog
        try {
            String aSlotID = "slot:10371";
            XDispatchProvider xDispProv = UnoRuntime.queryInterface( XDispatchProvider.class, xController );
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
         ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = aSlotID;
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
            if( xDispatcher != null )
                    xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
        }
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println("creating a draw document");
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }

}
