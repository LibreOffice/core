/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GetServiceWhileDisposingOffice.java,v $
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
package complex.disposing;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import com.sun.star.frame.XDesktop;

/**
 * This test is for bug110698. The Office is closed and is continually connected
 * while it closes. This did let the Office freeze. Now when the Office is
 * closed, the connection is refused.
 */
public class GetServiceWhileDisposingOffice extends ComplexTestCase {

    public String[] getTestMethodNames() {
        return new String[]{"checkServiceWhileDisposing"};
    }

    public void checkServiceWhileDisposing() {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
        XDesktop xDesktop = null;

        try {
            xDesktop = (XDesktop)UnoRuntime.queryInterface(XDesktop.class,
                        xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch(com.sun.star.uno.Exception e) {
            failed("Could not create a desktop instance.");
        }
        int step = 0;
        try {
            log.println("Start the termination of the Office.");
            xDesktop.terminate();
            for ( ; step<10000; step++ ) {
                Object o = xMSF.createInstance("com.sun.star.frame.Desktop");
            }
        }
        catch(com.sun.star.lang.DisposedException e) {
            log.println("DisposedException in step: " + step);
            e.printStackTrace();
        }
        catch(Exception e) {
            e.printStackTrace();
            failed(e.getMessage());
        }

    }
}
