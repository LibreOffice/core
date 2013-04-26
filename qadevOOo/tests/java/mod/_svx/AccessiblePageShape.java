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
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessiblePageShape extends TestCase {

    static XComponent xDrawDoc;
    static XModel aModel;

    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = SOF.createDrawDoc(null);
            aModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);

        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes the Draw document loaded before.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        //XShape oShape = null;
        XDrawPage oPage = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = AccessibilityTools.getCurrentWindow ((XMultiServiceFactory)tParam.getMSF(),aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

//        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.SHAPE,
//            "PageShape");
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.UNKNOWN, "PageShape");

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //at.getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLLBAR);
        //final XAccessibleValue xAccVal = (XAccessibleValue)
        //  UnoRuntime.queryInterface
        //    (XAccessibleValue.class, at.SearchedContext) ;
        oPage = DrawTools.getDrawPage(xDrawDoc,0);
        final XPropertySet PageProps = UnoRuntime.queryInterface(XPropertySet.class, oPage);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        PageProps.setPropertyValue("Height",new Integer(5000));
                    } catch (com.sun.star.beans.UnknownPropertyException upe) {
                        System.out.println("Don't no the Property Height");
                    } catch (com.sun.star.beans.PropertyVetoException pve) {
                        System.out.println(
                        "PropertyVetoException Exception while changing Height");
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                        System.out.println(
                        "IllegalArgumentException Exception while changing Height");
                    } catch (com.sun.star.lang.WrappedTargetException wte) {
                        System.out.println(
                        "WrappedTargetException Exception while changing Height");
                    }
                }
            });

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

}

