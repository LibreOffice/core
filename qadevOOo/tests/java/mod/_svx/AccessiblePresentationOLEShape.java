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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.Size;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessiblePresentationOLEShape extends TestCase {

    static XComponent xDrawDoc;
    static XModel aModel;

    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a drawdoc" );
        xDrawDoc = SOF.createImpressDoc(null);
        aModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);
    }

    /**
     * Disposes the Draw document loaded before.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory docMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xDrawDoc);
        oShape = UnoRuntime.queryInterface(XShape.class,
            docMSF.createInstance("com.sun.star.presentation.OLE2Shape"));


        XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            oShapeProps.setPropertyValue("IsEmptyPresentationObject", Boolean.FALSE);
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }

        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        try {
            oShapeProps.setPropertyValue(
                "CLSID","12DCAE26-281F-416F-a234-c3086127382e");
            oShapeProps.setPropertyValue("IsEmptyPresentationObject", Boolean.FALSE);
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }

        XWindow xWindow = AccessibilityTools.getCurrentWindow (aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.UNKNOWN, "ImpressOLE");

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        final XShape fShape = oShape ;
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        Size size = fShape.getSize();
                        size.Width += 100;
                        fShape.setSize(size);
                    } catch(com.sun.star.beans.PropertyVetoException e) {
                        System.out.println("caught exception: " + e);
                    }
                }
            });

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

}

