/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PopupMenuControllerFactory.java,v $
 * $Revision: 1.5 $
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
     * @param The log writer.
     * @return The test environment.
     */
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
     * Create test environment:
     * @param tParam The test parameters.
     * @param The log writer.
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
        XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        try {
            Object o = xProp.getPropertyValue("DefaultContext");
            XComponentContext xContext = (XComponentContext)UnoRuntime.queryInterface(XComponentContext.class, o);
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
        XUIControllerRegistration xReg = (XUIControllerRegistration)
                UnoRuntime.queryInterface(XUIControllerRegistration.class, xInst);

        xReg.registerController(".uno:MyCommandUrl", "", "com.sun.star.comp.framework.FooterMenuController");
        tEnv.addObjRelation("XUIControllerRegistration.RegisteredController", ".uno:MyCommandUrl");
        tEnv.addObjRelation("XMultiComponentFactory.ServiceNames", new String[]{".uno:MyCommandUrl"});

        return tEnv;
    }
}


