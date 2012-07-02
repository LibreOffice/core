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
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.util.XCloseable;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.WriterTools;

/**
 */
public class LayoutManager extends TestCase {
    XInterface xManager = null;
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
     * <ul>
     * <li>Create test doc</li>
     * <li>Get the frame</li>
     * <li>Get the LayoutManager from the frame</li>
     * </ul>
     * @param tParam The test parameters.
     * @param log The log writer.
     * @return The test environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        log.println("Creating instance...");

        xTextDoc = WriterTools.createTextDoc(xMSF);

        XText xText = xTextDoc.getText();
        XTextCursor xTextCursor = xText.createTextCursor();

        for (int i = 0; i < 11; i++) {
            xText.insertString(xTextCursor, "A sample text and why not? ", false);
        }

        XFrame xFrame = xTextDoc.getCurrentController().getFrame();
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, xFrame);
        try {
            Object any = xProp.getPropertyValue("LayoutManager");
            xManager = UnoRuntime.queryInterface(XInterface.class, any);
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Could not get property 'LayoutManager' from the current frame.", e);
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Could not get property 'LayoutManager' from the current frame.", e);
        }

        // just to make sure, it's the right one.
        log.println("TestObject: " + util.utils.getImplName(xManager));
        tEnv = new TestEnvironment(xManager);

        tEnv.addObjRelation("XLayoutManager.TextDoc", xTextDoc);
        tEnv.addObjRelation("XLayoutManager.Frame",xFrame);

        return tEnv;
    }
}


