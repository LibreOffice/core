/*************************************************************************
 *
 *  $RCSfile: UnoProvider.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-10-06 12:39:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package helper;

import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.Hashtable;
import lib.TestParameters;
import util.utils;

/**
 * Bootstrap UNO from a Java environment.
 * Needed parameters:
 * <ol>
 *  <li>
 *      <ul>
 *          <li>UNORC - complete path to the unorc file</li>
 *      </ul>
 *  </li>
 *  <li>
 *      <ul>
 *          <li>AppExecutionCommand - path to the soffice executable</li>
 *          <li>OS - the operating system in case it's Windows, because the
 *              unorc is called uno.ini</li>
 *      </ul>
 *  </li>
 * </ol>
 */
public class UnoProvider implements AppProvider {

    /**
     * Close existing office: calls disposeManager()
     * @param param The test parameters.
     * @param closeIfPossible Not needed, since UNO is bootstrapped by this
     * class in every case.
     * @return True, if bootstrapping worked.
     */
    public boolean closeExistingOffice(TestParameters param,
                                                    boolean closeIfPossible) {
        return disposeManager(param);
    }

    /**
     * Dispose the UNO environment: just clears the bootstrapped
     * MultiServiceFactory
     * @param param The test parameters.
     * @return True, if bootstrapping worked.
     */
    public boolean disposeManager(TestParameters param) {
        XMultiServiceFactory xMSF =
                    (XMultiServiceFactory)param.remove("ServiceManager");
        xMSF = null;
        System.gc();
        try {
            Thread.sleep(1000);
        }
        catch(java.lang.InterruptedException e) {}
        return true;
    }

    /**
     * Bootstrap UNO and return the created MultiServiceFactory.
     * @param param The test parameters.
     * @return A created MultiServiceFactory.
     */
    public Object getManager(TestParameters param) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
        if (xMSF == null) {
            // bootstrap UNO.
            String unorcName = getUnorcName(param);
            Hashtable env = new Hashtable();
            env.put("SYSBINDIR", getSysBinDir(param));

            XComponentContext xContext = null;
            try {
                xContext = Bootstrap.defaultBootstrap_InitialComponentContext(
                                                                 unorcName, env);
            }
            catch(java.lang.Exception e) {
                e.printStackTrace();
                return null;
            }
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            xMSF = (XMultiServiceFactory)UnoRuntime.queryInterface(
                                XMultiServiceFactory.class, xMCF);
        }
        return xMSF;
    }

    private String getUnorcName(TestParameters param) {
        String unorcName = (String)param.get("UNORC");
        if (unorcName == null) {
            String office = (String)param.get("AppExecutionCommand");
            // determine unorc name: unorc or uno.ini on windows
            String opSystem = (String)param.get("OS");
            if ( opSystem != null && opSystem.equalsIgnoreCase("wntmsci")) {
                unorcName = "uno.ini";
            }
            else {
                unorcName = "unorc";
            }
            if (office == null)
                return null;
            // use '/', because this will be a URL in any case.
            unorcName = office.substring(0, office.indexOf("program")+7) +
                        "/" + unorcName;
        }
        unorcName = utils.getFullURL(unorcName);
        if (param.DebugIsActive) {
            System.out.println("UnoUcr: " + unorcName);
        }
        return unorcName;
    }

    private String getSysBinDir(TestParameters param) {
        String base = (String)param.get("AppExecutionCommand");
        if (base == null)
            base = (String)param.get("UNORC");

        if (base == null)
            return null;

        String sysbindir = base.substring(0,
                                base.indexOf("program")+7);

        sysbindir = utils.getFullURL(sysbindir);
        if (param.DebugIsActive) {
            System.out.println("SysBinDir: " + sysbindir);
        }
        return sysbindir;
    }
}
