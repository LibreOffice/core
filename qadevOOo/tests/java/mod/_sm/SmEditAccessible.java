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

package mod._sm;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessible</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessible
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessible
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleText
 */
public class SmEditAccessible extends TestCase {
    XComponent xMathDoc;

    /**
     * Creates a <code>StarMath</code> document and obtains an accessibility
     * component with the role <code>AccessibleRole.TEXT_FRAME</code>.
     */
    @Override
    protected TestEnvironment createTestEnvironment(final TestParameters Param, PrintWriter log) throws Exception {

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF() );
        xMathDoc = SOF.openDoc("smath","_blank");

        // setting a formula in document
        final String expFormula = "sum hat a";
        final XPropertySet xPS = UnoRuntime.queryInterface
            (XPropertySet.class, xMathDoc);
        xPS.setPropertyValue("Formula", expFormula);

        XInterface oObj = null;

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xMathDoc);


        XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole
            (xRoot, AccessibleRole.TEXT_FRAME, "", "WeldEditAccessible");

        log.println("ImplementationName " + utils.getImplName(oObj));
        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("Destroy", Boolean.TRUE);

        final XAccessibleContext con = UnoRuntime.queryInterface(XAccessibleContext.class, oObj);
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        System.out.println("Children "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", "sum hat x \n int a \n sum b");
                        util.utils.waitForEventIdle(Param.getMSF());
                        System.out.println("Children "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", expFormula);
                        util.utils.waitForEventIdle(Param.getMSF());
                    } catch(com.sun.star.lang.WrappedTargetException e) {
                    } catch(com.sun.star.lang.IllegalArgumentException e) {
                    } catch(com.sun.star.beans.PropertyVetoException e) {
                    } catch(com.sun.star.beans.UnknownPropertyException e) {
                    }
                }
            });

        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }
}    // finish class SmModel
