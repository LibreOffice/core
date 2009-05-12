/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DesktopTerminate.java,v $
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
package complex.desktop;


import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFramesSupplier;
import com.sun.star.frame.XFrames;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.container.XIndexAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.awt.Rectangle;
import com.sun.star.util.XCloseable;
import helper.ConfigurationRead;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
//import complex.persistent_window_states.helper.DocumentHandle;

/**
 * Parameters:
 * <ul>
 *   <li>NoOffice=yes - StarOffice is not started initially.</li>
 * </ul>
 */
public class DesktopTerminate extends ComplexTestCase {

    private XMultiServiceFactory xMSF;
    private OfficeProvider oProvider;
    private int iOfficeCloseTime = 0;

    /**
     * A frunction to tell the framework, which test functions are available.
     * Right now, it's only 'checkPersistentWindowState'.
     * @return All test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"checkPersistentWindowState"};
    }

    /**
     * Test if all available document types change the
     * persistent Window Attributes
     *
     * The test follows basically these steps:
     * - Create a configuration reader and a componentloader
     * - Look for all document types in the configuration
     * - Do for every doc type
     *   - start office
     *   - read configuration attibute settings
     *   - create a new document
     *   - resize the document and close it
     *   - close office
     *   - start office
     *   - read configuration attribute settings
     *   - create another new document
     *   - compare old settings with new ones: should be different
     *   - compare the document size with the resized document: should be equal
     *   - close office
     * - Test finished
     */
    public void checkPersistentWindowState()
    {
        try {

            log.println("Connect the first time.");
            log.println("AppExecCommand: " + (String)param.get("AppExecutionCommand"));
            log.println("ConnString: " + (String)param.get("ConnectionString"));
            oProvider = new OfficeProvider();
            iOfficeCloseTime = param.getInt("OfficeCloseTime");
            if ( iOfficeCloseTime == 0 ) {
                iOfficeCloseTime = 1000;
            }

            if (!connect()) return;

            if (!disconnect()) return;
        }
        catch(Exception e) {
            e.printStackTrace();
        }
    }

    private boolean connect() {
        try {
            xMSF = (XMultiServiceFactory)oProvider.getManager(param);
            try {
                Thread.sleep(10000);
            }
            catch(java.lang.InterruptedException e) {}
        }
        catch (java.lang.Exception e) {
            log.println(e.getClass().getName());
            log.println("Message: " + e.getMessage());
            failed("Cannot connect the Office.");
            return false;
        }
        return true;
    }

    private boolean disconnect() {
        try {
            XDesktop desk = null;
            desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, xMSF.createInstance(
                    "com.sun.star.frame.Desktop"));
            desk.terminate();
            log.println("Waiting " + iOfficeCloseTime + " milliseconds for the Office to close down");
            try {
                Thread.sleep(iOfficeCloseTime);
            }
            catch(java.lang.InterruptedException e) {}
            xMSF = null;
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
            failed("Cannot dispose the Office.");
            return false;
        }
        return true;
    }

}
