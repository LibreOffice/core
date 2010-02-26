/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.system.XSystemShellExecute
*/
public class _XSystemShellExecute extends MultiMethodTest {

    public XSystemShellExecute oObj = null;

    /**
    * Excecutes 'java SystemShellExecute SystemShellExecute.txt' command line.
    * <p>Has <b> OK </b> status if the method successfully returns
    * and file 'SystemShellExecute.txt' was created. <p>
    */
    public void _execute() {
        String cClassPath = System.getProperty("DOCPTH");
        String cResFile = utils.getOfficeTempDirSys((XMultiServiceFactory)tParam.getMSF())+"SystemShellExecute.txt";
        String cResURL = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF())+"SystemShellExecute.txt";
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
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
            Object fa = xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess");
            xFileAccess = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class, fa);
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
            shortWait();
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

    /**
    * Sleeps to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}  // finish class _XSystemShellExecute


