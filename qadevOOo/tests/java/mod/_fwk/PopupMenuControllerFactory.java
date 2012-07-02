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

package mod._fwk;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;
import com.sun.star.frame.XUIControllerRegistration;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.WriterTools;

/**
 */
public class PopupMenuControllerFactory extends TestCase {
    XTextDocument xTextDoc;

    /**
     * Cleanup: close the created document
     * @param tParam The test parameters.
     * @param log The log writer.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
            XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }


    /**
     * Create test environment:
     * @param tParam The test parameters.
     * @param log The log writer.
     * @return The test environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface xInst = null;

        log.println("Creating instance...");

        xTextDoc = WriterTools.createTextDoc(xMSF);
        util.dbg.printInterfaces(xTextDoc);

        try {
            xInst = (XInterface)xMSF.createInstance(
                            "com.sun.star.comp.framework.PopupMenuControllerFactory");
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Couldn't create test object", e);
        }

        log.println("TestObject: " + util.utils.getImplName(xInst));
        tEnv = new TestEnvironment(xInst);
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        try {
            Object o = xProp.getPropertyValue("DefaultContext");
            XComponentContext xContext = UnoRuntime.queryInterface(XComponentContext.class, o);
            tEnv.addObjRelation("DC", xContext);
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Cannot get the 'DefaultContext' for XMultiComponentFactory test.");
            e.printStackTrace(log);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Cannot get the 'DefaultContext' for XMultiComponentFactory test.");
            e.printStackTrace(log);
        }

        // register one controller, so it can be instantiated
        XUIControllerRegistration xReg = UnoRuntime.queryInterface(XUIControllerRegistration.class, xInst);

        xReg.registerController(".uno:MyCommandUrl", "", "com.sun.star.comp.framework.FooterMenuController");
        tEnv.addObjRelation("XUIControllerRegistration.RegisteredController", ".uno:MyCommandUrl");
        tEnv.addObjRelation("XMultiComponentFactory.ServiceNames", new String[]{".uno:MyCommandUrl"});

        return tEnv;
    }
}


