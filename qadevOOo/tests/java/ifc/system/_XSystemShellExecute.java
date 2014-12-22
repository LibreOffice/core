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

package ifc.system;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.system.XSystemShellExecute;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;


/**
* Testing <code>com.sun.star.system.XSystemShellExecute</code>
* interface methods :
* <ul>
*  <li><code> execute()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.system.XSystemShellExecute
*/
public class _XSystemShellExecute extends MultiMethodTest {

    public XSystemShellExecute oObj = null;

    /**
    * Executes 'java SystemShellExecute SystemShellExecute.txt' command line.
    * <p>Has <b> OK </b> status if the method successfully returns
    * and file 'SystemShellExecute.txt' was created. <p>
    */
    public void _execute() {
        String cClassPath = System.getProperty("DOCPTH");
        String cResFile = utils.getOfficeTempDirSys(tParam.getMSF())+"SystemShellExecute.txt";
        String cResURL = utils.getOfficeTemp(tParam.getMSF())+"SystemShellExecute.txt";
        String cArgs = "-classpath " + cClassPath +
                       " SystemShellExecute " + cResFile;

        String jh = System.getProperty("java.home");
        String fs = System.getProperty("file.separator");
        String cmd = jh+fs+"bin"+fs+"java";

        log.println("Executing : '"+cmd+" " + cArgs + "'");
        try {
            oObj.execute(cmd, cArgs, 1);
        } catch (com.sun.star.system.SystemShellExecuteException e) {
            log.println("Exception during execute: " + e);
            log.println("This has been implemented due to security reasons");
            tRes.tested("execute()", true);
            return;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception during execute: " + e);
            tRes.tested("execute()", false);
            return;
        }

        XSimpleFileAccess xFileAccess = null;
        try {
            XMultiServiceFactory xMSF = tParam.getMSF();
            Object fa = xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            xFileAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class, fa);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create SimpleFileAccess:" + e);
            tRes.tested("execute()", false);
        }

        log.println("Waiting while the file will be created or timeout "+
            "reached ...");
        boolean bExist = false;
        int i = 0;
        while (i < 20 && !bExist) {
            try {
                bExist = xFileAccess.exists(cResURL);
            } catch(com.sun.star.uno.Exception e) {
                log.println("Exception:" + e);
            }
            util.utils.pause(1000);
            i++;
        }

        if (bExist) {
            log.println("The command was executed and file created in " +
                 i + " sec.");
        } else {
            log.println("File was not created");
        }

        tRes.tested("execute()", bExist);
    }
}  // finish class _XSystemShellExecute


