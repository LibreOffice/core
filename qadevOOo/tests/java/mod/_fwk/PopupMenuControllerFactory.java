/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PopupMenuControllerFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:22:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._fwk;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.embed.XTransactedObject;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import ifc.ui._XUIConfiguration;
import java.io.PrintWriter;
import com.sun.star.lang.EventObject;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;
import com.sun.star.frame.XUIControllerRegistration;
import com.sun.star.ui.ConfigurationEvent;
import com.sun.star.ui.XUIConfigurationManager;
import com.sun.star.ui.XUIConfigurationManagerSupplier;
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


