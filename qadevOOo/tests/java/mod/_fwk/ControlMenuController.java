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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;


/**
 */
public class ControlMenuController extends TestCase {
    XInterface oObj = null;
    XTextDocument xTextDoc;

    /**
     * Cleanup: close the created document
     * @param tParam The test parameters.
     * @param log The log writer.
     * @return The test environment.
     *
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
            XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    /**
     * Create test environment.
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        try {
            oObj = (XInterface)xMSF.createInstance("com.sun.star.comp.framework.ControlMenuController");
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Could not create object 'ControlMenuController'", e);
        }


/*        log.println("Creating instance...");

        xTextDoc = WriterTools.createTextDoc(xMSF);

/*        XText xText = xTextDoc.getText();
        XTextCursor xTextCursor = xText.createTextCursor();

        for (int i = 0; i < 11; i++) {
            xText.insertString(xTextCursor, "A sample text and why not? ", false);
        }

        XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, xTextDoc);

        // the supplier is available from the model
        XUIConfigurationManagerSupplier xSupplier =
                (XUIConfigurationManagerSupplier)UnoRuntime.queryInterface(
                XUIConfigurationManagerSupplier.class, xModel);

        xManager = xSupplier.getUIConfigurationManager();

        // just to make sure, it's the right one.
        log.println("TestObject: " + util.utils.getImplName(xManager));
        tEnv = new TestEnvironment(xManager); */
        log.println("TestObject: " + utils.getImplName(oObj));
        tEnv = new TestEnvironment(oObj);
        return tEnv;
    }

}
