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
package mod._sch;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XWindow;
import com.sun.star.chart.XChartDocument;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessibleDocumentView extends TestCase {

    XChartDocument xChartDoc = null;

    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xChartDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        oObj = AccessibilityTools.SearchedContext;

        if (oObj == null) {
            log.println("DocumentView hasn't the role 'Document'");
            log.println("trying the role 'Shape'");
            at.getAccessibleObjectForRole(xRoot, AccessibleRole.SHAPE);
            oObj = AccessibilityTools.SearchedContext;
        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XWindow xDocWin = xWindow;
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    Rectangle rect = xDocWin.getPosSize();
                    xDocWin.setPosSize(100,100,100,100,PosSize.POSSIZE);
                    xDocWin.setPosSize(rect.X,rect.Y,rect.Width,rect.Height,
                                                            PosSize.POSSIZE);
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
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
    }

    /**
     * Called while the <code>TestCase</code> initialization.
     * Creates a chart document.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println( "creating a text document" );
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        try {
            log.println( "creating a chartdocument" );
            xChartDoc = SOF.createChartDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }
}
