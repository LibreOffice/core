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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.chart.XChartDocument;
import com.sun.star.frame.XModel;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccDataSeries extends TestCase {

    XChartDocument xChartDoc = null;

    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        if (xChartDoc != null) cleanup(Param, log);
        log.println( "creating a chart document" );
        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());
        log.println( "creating a chartdocument" );
        xChartDoc = SOF.createChartDoc();

        XInterface oObj = null;

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xChartDoc);

        XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        XAccessibleContext cont = AccessibilityTools.getAccessibleObjectForRole(
                xRoot, AccessibleRole.SHAPE, "", "AccDataSeries");

        oObj = cont;

        log.println("ImplementationName " + utils.getImplName(oObj));
        log.println("AccessibleName " + cont.getAccessibleName());

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleComponent acc = UnoRuntime.queryInterface(
            XAccessibleComponent.class,oObj);
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                        acc.grabFocus();
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
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        if( xChartDoc!=null ) {
            log.println( "    closing xChartDoc" );
            util.DesktopTools.closeDoc(xChartDoc);
            xChartDoc = null;
        }
    }

}
