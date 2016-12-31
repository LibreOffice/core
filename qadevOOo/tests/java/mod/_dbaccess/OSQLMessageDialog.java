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

package mod._dbaccess;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sdb.ErrorMessageDialog</code>.
 * <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 *  <li> <code>com::sun::star::sdb::ErrorMessageDialog</code></li>
 *  <li> <code>com::sun::star::ui::dialogs::XExecutableDialog</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.sdb.ErrorMessageDialog
 * @see com.sun.star.ui.dialogs.XExecutableDialog
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XInitialization
 * @see ifc.sdb._ErrorMessageDialog
 * @see ifc.ui.dialogs._XExecutableDialog
 * @see ifc.beans._XPropertySet
 */
public class OSQLMessageDialog extends TestCase {

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     *
     * Creates an instance of the service
     * <code>com.sun.star.sdb.ErrorMessageDialog</code>. Object relations
     * created :
     * <ul>
     * <li> <code>'ERR1', 'ERR2'</code> for {@link ifc.sdb._ErrorMessageDialog}</li>
     * </ul>
     */
    @Override
    public TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) throws Exception {
        XInterface oObj = null;
        Object oInterface = null;

        Object err1 = null;
        Object err2 = null;

        oInterface = Param.getMSF().createInstance(
                "com.sun.star.sdb.ErrorMessageDialog");

        err1 = new com.sun.star.sdbc.SQLException("err1");
        err2 = new com.sun.star.sdbc.SQLException("err2");

        oObj = (XInterface) oInterface;

        // create XWindow for Object relations...
        com.sun.star.awt.XToolkit xToolkit = UnoRuntime.queryInterface(
                com.sun.star.awt.XToolkit.class,
                Param.getMSF().createInstance("com.sun.star.awt.Toolkit"));

        // Describe the properties of the container window.
        com.sun.star.awt.WindowDescriptor aDescriptor = new com.sun.star.awt.WindowDescriptor();

        aDescriptor.Type = com.sun.star.awt.WindowClass.TOP;
        aDescriptor.WindowServiceName = "window";
        aDescriptor.ParentIndex = -1;
        aDescriptor.Parent = null;
        aDescriptor.Bounds = new com.sun.star.awt.Rectangle(0, 0, 0, 0);

        aDescriptor.WindowAttributes = com.sun.star.awt.WindowAttribute.BORDER
                | com.sun.star.awt.WindowAttribute.MOVEABLE
                | com.sun.star.awt.WindowAttribute.SIZEABLE
                | com.sun.star.awt.WindowAttribute.CLOSEABLE;

        com.sun.star.awt.XWindowPeer xPeer = xToolkit.createWindow(aDescriptor);

        com.sun.star.awt.XWindow xWindow = UnoRuntime.queryInterface(
                com.sun.star.awt.XWindow.class, xPeer);

        log.println("    creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println("add ObjectRelations err1 and err2 for 'ErrorMessageDialog'");
        tEnv.addObjRelation("ERR1", err1);
        tEnv.addObjRelation("ERR2", err2);
        tEnv.addObjRelation("ERR_XWindow", xWindow);

        return tEnv;
    } // finish method getTestEnvironment

    @Override public void disposeTestEnvironment(
        TestEnvironment tEnv, TestParameters tParam)
    {
        UnoRuntime.queryInterface(
            com.sun.star.lang.XComponent.class,
            (com.sun.star.awt.XWindow) tEnv.getObjRelation("ERR_XWindow"))
            .dispose();
        super.disposeTestEnvironment(tEnv, tParam);
    }
} // finish class OSQLMessageDialog

