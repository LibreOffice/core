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
package helper;

import java.util.HashMap;

import lib.TestParameters;
import util.PropertyName;
import util.utils;

import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

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

    public UnoProvider(){

    }

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
        param.remove("ServiceManager");
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
            HashMap<String,String> env = new HashMap<String,String>();
            env.put("SYSBINDIR", getSysBinDir(param));

            XComponentContext xContext = null;
            try {
                xContext = Bootstrap.defaultBootstrap_InitialComponentContext(
                                                                 unorcName, env);
            }
            catch(Exception e) {
                e.printStackTrace();
                System.out.println("Could not get XComponentContext. Maybe you must add program folder to LD_LIBRARY_PATH");
                return null;
            }
            XMultiComponentFactory xMCF = xContext.getServiceManager();
            xMSF = UnoRuntime.queryInterface(
                                XMultiServiceFactory.class, xMCF);
        }
        return xMSF;
    }

    private String getUnorcName(TestParameters param) {
        String unorcName = (String)param.get("UNORC");
        if (unorcName == null) {
            String office = (String)param.get("AppExecutionCommand");
            // determine unorc name: unorc or uno.ini on windows
            String opSystem = (String)param.get(PropertyName.OPERATING_SYSTEM);
            if ( opSystem != null && opSystem.equalsIgnoreCase(PropertyName.WNTMSCI)) {
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
