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

package mod._svtools;

import com.sun.star.view.XSelectionSupplier;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;

/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li>
 *      <code>::com::sun::star::accessibility::XAccessibleContext
 *  </code></li>
 *  <li>
 *      <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleBrowseBoxHeaderCell extends TestCase {

    static XDesktop the_Desk;
    static XTextDocument xTextDoc;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory)Param.getMSF()));
    }

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

    /**
     * Creates a text document. Opens the DataSource browser and loads
     * the table. Creates an instance of the service
     * <code>com.sun.star.awt.Toolkit</code> and gets active top window.
     * Then obtains an accessible object with the role
     * <code>AccessibleRole.UNKNOWN</code> and with the name
     * <code>"Identifier"</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      method <code>fireEvent()</code> is empty because object is transient
     *   </li>
     * </ul>
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see com.sun.star.awt.Toolkit
     * @see com.sun.star.accessibility.AccessibleRole
     * @see ifc.accessibility._XAccessibleEventBroadcaster
     * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) {

        log.println( "creating a test environment" );

        if (xTextDoc != null) xTextDoc.dispose();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        shortWait();

        XModel aModel1 = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = UnoRuntime.
            queryInterface(XDispatchProvider.class, secondController);

        XDispatch getting = null;

        log.println( "opening DatasourceBrowser" );
        URL the_url = new URL();
        the_url.Complete = ".component:DB/DataSourceBrowser";
        getting = aProv.queryDispatch(the_url, "_beamer", 12);
        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(the_url, noArgs);

        shortWait();

        XFrame the_frame1 = the_Desk.getCurrentFrame();

        if (the_frame1 == null) {
            log.println("Current frame was not found !!!");
        }

        XFrame the_frame2 = the_frame1.findFrame("_beamer", 4);

        the_frame2.setName("DatasourceBrowser");

        XInterface oObj = null;

        UnoRuntime.queryInterface(
                XInitialization.class, the_frame2.getController());

        PropertyValue[] params = new PropertyValue[3];
        PropertyValue param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        params[0] = param1;
        PropertyValue param2 = new PropertyValue();
        param2.Name = "CommandType";
        param2.Value = new Integer(com.sun.star.sdb.CommandType.TABLE);
        params[1] = param2;
        PropertyValue param3 = new PropertyValue();
        param3.Name = "Command";
        param3.Value = "biblio";
        params[2] = param3;

        XController xCont = the_frame2.getController();

        XSelectionSupplier xSelect = UnoRuntime.queryInterface(
            XSelectionSupplier.class, xCont);

        try {
            xSelect.select(params);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            throw new StatusException("Could not select Biblio-Database", ex);
        }

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = secondController.getFrame().getContainerWindow();

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.ROW_HEADER);

        log.println("ImplementationName: "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                }
            });

        return tEnv;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

}
