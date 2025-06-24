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
import util.FormTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.Size;
import com.sun.star.awt.XWindow;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessibleControlShape extends TestCase {

    static XComponent xDrawDoc;
    static XModel aModel;

    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a drawdoc" );
        xDrawDoc = SOF.createDrawDoc(null);
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
            (TestParameters tParam, PrintWriter log)
        throws com.sun.star.uno.Exception
    {

        XInterface oObj = null;

        log.println( "creating a test environment" );

        final XShape oShape = FormTools.insertControlShape
            (xDrawDoc,3000,4500,15000,1000,"CommandButton");

        util.utils.waitForEventIdle(tParam.getMSF());
        XWindow xWindow = AccessibilityTools.getCurrentWindow (aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.UNKNOWN, "Button");

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        Size size = oShape.getSize();
                        size.Width += 100;
                        oShape.setSize(size);
                    } catch(com.sun.star.beans.PropertyVetoException e) {
                        System.out.println("caught exception: " + e);
                    }
                }
            });

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

}

