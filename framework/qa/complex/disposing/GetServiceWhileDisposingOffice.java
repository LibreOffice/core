/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetServiceWhileDisposingOffice.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-27 08:40:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
