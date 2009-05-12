/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LoadDocument.java,v $
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
package complex.broken_document;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;

/**
 * Check, if message boxes appear when the Office is in "headless" mode. Tests
 * bug i15809. This test uses the broken document dbf.dbf.emf.
 */
public class LoadDocument extends ComplexTestCase {

    /** defect file to load **/
    private final String mFileName = "dbf.dbf.emf";

    /**
     * Get all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"checkHeadlessState"};
    }

    /**
     * Start Office with "-headless" parameter, then
     * load the broken document "dbf.dbf.emf", that brings a message box up in
     * the ui, see if the headless mode of SOffice changes.
     */
    public void checkHeadlessState() {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
        XFrame xDesktop = null;

        try {
            xDesktop = (XFrame)UnoRuntime.queryInterface(XFrame.class,
                        xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch(com.sun.star.uno.Exception e) {
            failed("Could not create a desktop instance.");
        }

        XComponentLoader xDesktopLoader = (XComponentLoader)
                UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
        System.out.println("xDektopLoader is null: " + (xDesktopLoader == null));
        PropertyValue[] val = new PropertyValue[0];

        String workingDir = (String)param.get("WorkingDir") + System.getProperty("file.separator") + mFileName;
        System.out.println("Working dir: " + workingDir);
        String fileUrl = util.utils.getFullURL(workingDir);
        System.out.println("File Url: " + fileUrl);

        try {
            xDesktopLoader.loadComponentFromURL(fileUrl, "_blank", 0, val);
        }
        catch(com.sun.star.io.IOException e) {
            failed("Could not load document");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            failed("Could not load document");
        }

        // try again: headless mode defect now?
        try {
            xDesktopLoader.loadComponentFromURL(fileUrl, "_blank", 0, val);
        }
        catch(com.sun.star.io.IOException e) {
            failed("Could not load document");
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            failed("Could not load document");
        }

    }
}
