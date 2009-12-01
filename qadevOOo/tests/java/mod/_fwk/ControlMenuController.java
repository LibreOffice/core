/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ControlMenuController.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
     * @param The log writer.
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
