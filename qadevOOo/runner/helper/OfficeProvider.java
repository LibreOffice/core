/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeProvider.java,v $
 * $Revision: 1.21 $
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
package helper;

//import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XStringSubstitution;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Iterator;
import java.util.StringTokenizer;

import lib.TestParameters;

import share.DescEntry;
import share.LogWriter;

import util.DynamicClassLoader;
import util.utils;


/**
 * This class will connect the office and start it if possible
 *
 */
public class OfficeProvider implements AppProvider {
    protected static boolean debug = false;

    /**
     * Dispose the office.
     * This method can only be used, if the office was connected in the first
     * place: getManager() was called first.
     */
    public boolean disposeManager(lib.TestParameters param) {



        XMultiServiceFactory msf = (XMultiServiceFactory) param.getMSF();

        if (msf == null) {
            return true;
        } else {
            XDesktop desk = null;

            try {
                desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                msf.createInstance(
                "com.sun.star.frame.Desktop"));
            } catch (com.sun.star.uno.Exception ue) {
                return false;
            }

            msf = null;

            if (desk != null) {
                desk.terminate();

                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Method to get the ServiceManager of an Office
     */
    public Object getManager(lib.TestParameters param) {
        String errorMessage = null;
        boolean bAppExecutionHasWarning = false;
        debug = param.getBool("DebugIsActive");

        String additionalArgs = (String) param.get(
            "AdditionalConnectionArguments");

        if (additionalArgs == null) {
            additionalArgs = ";";
        } else {
            additionalArgs = "," + additionalArgs + ";";
        }

        String cncstr = "uno:" + param.get("ConnectionString") + ";urp" +
        additionalArgs + "StarOffice.ServiceManager";

        if (debug) {
            System.out.print("Connecting the Office with " + cncstr);
        }


        XMultiServiceFactory msf = connectOffice(cncstr);

        // if the office is running and the office crashes while testing it could
        // be usesfull to restart the office if possible and continuing the tests.
        // Example: The UNO-API-Tests in the projects will be executed by calling
        // 'damke'. This connects to an existing office. If the office crashes
        // it is usefull to restart the office and continuing the tests.
        if ((param.getBool(util.PropertyName.AUTO_RESTART)) && (msf != null)) makeAppExecCommand(msf, param);

        if (msf == null) {
            String exc = "";
            Exception exConnectFailed = null;
            boolean isExecutable = false;
            boolean isAppKnown = ((cncstr.indexOf("host=localhost") > 0) || (cncstr.indexOf("pipe,name=")>0));
            isAppKnown &= !((String) param.get("AppExecutionCommand")).equals("");

            if (isAppKnown) {
                if (debug) {
                    System.out.println(
                    "Local Connection trying to start the Office");
                }

                //ensure that a pending officewatcher gets finished before a new
                //office is started
                OfficeWatcher ow_old = (OfficeWatcher) param.get("Watcher");

                if (ow_old != null) {
                    ow_old.finish = true;
                }

                String cmd = (String) param.get("AppExecutionCommand");
                // validate the AppExecutionCommand, but try it out anyway.
                // keep the error message for later.
                errorMessage =
                    util.utils.validateAppExecutionCommand(cmd, (String)param.get("OperatingSystem"));
                if (errorMessage.startsWith("Error")) {
                    System.out.println(errorMessage);
                    return null;
                }
                bAppExecutionHasWarning = !errorMessage.equals("OK");

                DynamicClassLoader dcl = new DynamicClassLoader();
                LogWriter log = (LogWriter) dcl.getInstance(
                (String) param.get("LogWriter"));

                //create empty entry
                DescEntry Entry = new DescEntry();
                Entry.entryName = "office";
                Entry.longName = "office";
                Entry.EntryType = "placebo";
                Entry.isOptional = false;
                Entry.isToTest = false;
                Entry.SubEntryCount = 0;
                Entry.hasErrorMsg = false;
                Entry.State = "non possible";
                Entry.UserDefinedParams = param;

                log.initialize(Entry, debug);

                ProcessHandler ph = new ProcessHandler(cmd, (PrintWriter) log);
                isExecutable = ph.executeAsynchronously();

                if (isExecutable) {
                    param.put("AppProvider", ph);

                    OfficeWatcher ow = new OfficeWatcher(param);
                    param.put("Watcher", ow);
                    ow.start();
                    ow.ping();
                }

                int k = 0;

                while ((k < 21) && (msf == null)) {
                    try {
                        Thread.sleep(k * 1000);
                        msf = connect(cncstr);
                    } catch (com.sun.star.uno.Exception ue) {
                        exConnectFailed = ue;
                        exc = ue.getMessage();
                    } catch (java.lang.Exception je) {
                        exConnectFailed = je;
                        exc = je.getMessage();
                    }

                    k++;
                }

                if (msf == null) {
                    System.out.println("Exception while connecting.\n" + exConnectFailed);
                    if (exc != null)
                        System.out.println(exc);
                    if (bAppExecutionHasWarning)
                        System.out.println(errorMessage);
                } else if (isExecutable) {
                    //copy the user layer to a safe place
                    try {
                        XStringSubstitution sts = createStringSubstitution(msf);
                        String userLayer = sts.getSubstituteVariableValue(
                        "$(user)");
                        userLayer = getDirSys(userLayer);
                        param.put("userLayer", userLayer);

                        String copyLayer = util.utils.getUsersTempDir() + System.getProperty("file.separator") +
                        "user_backup" + System.getProperty("user.name");
                        param.put("copyLayer", copyLayer);


                        if (debug) System.out.println("copy '" + userLayer +"' ->" + copyLayer +"'");
                        FileTools.copyDirectory(new File(userLayer), new File(copyLayer), new String[]{"temp"});
                    } catch (com.sun.star.container.NoSuchElementException e) {
                        System.out.println("User Variable '$(user)' not defined.");
                    } catch (java.io.IOException e) {
                        System.out.println("Couldn't backup user layer");
                        e.printStackTrace();
                    }
                }
            } else {
                System.out.println("Could not connect an Office" +
                    " and cannot start one.");
                if (bAppExecutionHasWarning)
                    System.out.println(errorMessage);
            }
        }

        return msf;
    }

    /**
     * Connect an Office
     */
    protected static XMultiServiceFactory connect(String connectStr)
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException,
    com.sun.star.connection.NoConnectException,
    Exception {
        // Get component context
        XComponentContext xcomponentcontext = com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
        null);

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
//        XUnoUrlResolver urlResolver = UnoUrlResolver.create(xcomponentcontext);
        Object xUrlResolver = xLocalServiceManager.createInstanceWithContext(
                    "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext);
        XUnoUrlResolver urlResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
                    XUnoUrlResolver.class, xUrlResolver);

        Object rInitialObject = urlResolver.resolve(connectStr);

        XMultiServiceFactory xMSF = null;

        if (rInitialObject != null) {
            if (debug) {
                System.out.println("resolved url");
            }

            xMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, rInitialObject);
        }

        return xMSF;
    }

    /**
     * Close an office.
     * @param param The test parameters.
     * @param closeIfPossible If true, close even if
     * it was running before the test
     */
    public boolean closeExistingOffice(lib.TestParameters param, boolean closeIfPossible) {
        debug = param.getBool("DebugIsActive");

        XMultiServiceFactory msf = (XMultiServiceFactory) param.getMSF();
        boolean alreadyConnected = (msf != null);

        if (alreadyConnected) {
            if (debug) System.out.println("DEBUG: try to get ProcessHandler");
            ProcessHandler ph = (ProcessHandler) param.get("AppProvider");

            if (ph != null) {
                if (debug) System.out.println("DEBUG: ProcessHandler != null");
                disposeOffice(msf, param);

                // dispose watcher in case it's still running.
                if (debug) System.out.println("DEBUG: try to get OfficeWatcher");
                OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

                if ((ow != null) && ow.isAlive()) {
                    if (debug) System.out.println("DEBUG: OfficeWatcher finished");
                    ow.finish = true;
                }

                return true;
            } else {
                if (closeIfPossible) {
                    return disposeOffice(msf, param);
                }
            }
        } else {
            String cncstr = "uno:" + param.get("ConnectionString") +
            ";urp;StarOffice.ServiceManager";
            if (debug) System.out.println("DEBUG: try to connect office");
            msf = connectOffice(cncstr);

            if (closeIfPossible) {
                return disposeOffice(msf, param);
            }
        }

        return true;
    }

    private XMultiServiceFactory connectOffice(String cncstr) {
        XMultiServiceFactory msf = null;
        String exc = "";

        if (debug) System.out.println("trying to connect to " + cncstr);

        try {
            msf = connect(cncstr);
        } catch (com.sun.star.uno.Exception ue) {
            exc = ue.getMessage();
        } catch (java.lang.Exception je) {
            exc = je.getMessage();
        }

        if (debug && exc != null && exc.length() != 0) {
            if (exc == null) exc="";
            System.out.println("Could not connect an Office. " + exc);
        }

        return msf;
    }

    private synchronized boolean disposeOffice(XMultiServiceFactory msf,
    TestParameters param) {
        XDesktop desk = null;

        boolean result = true;

        if (msf != null) {
            try {
                desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                msf.createInstance(
                "com.sun.star.frame.Desktop"));
                msf = null;

                if (desk != null) {
                    boolean allClosed = closeAllWindows(desk);

                    if (!allClosed) {
                        if (debug) {
                            System.out.println(
                            "Couldn't close all office windows!");
                        }
                    }

                    if (debug) {
                        System.out.println("Trying to terminate the desktop");
                    }

                    desk.terminate();

                    if (debug) {
                        System.out.println("Desktop terminated");
                    }

                    try {
                        int closeTime = param.getInt(util.PropertyName.OFFICE_CLOSE_TIME_OUT);
                        if (debug){
                            System.out.println("the Office has " + closeTime/1000 +
                                               " seconds for closing...");
                        }
                        Thread.sleep(closeTime);
                    } catch (java.lang.InterruptedException e) {
                    }
                }
            } catch (com.sun.star.uno.Exception ue) {
                result = false;
            } catch (com.sun.star.lang.DisposedException ue) {
                result = false;
            }
        }

        String AppKillCommand = (String) param.get ("AppKillCommand");
        if (AppKillCommand != null)
        {
            StringTokenizer aKillCommandToken = new StringTokenizer( AppKillCommand,";" );
            while (aKillCommandToken.hasMoreTokens())
            {
                String sKillCommand = aKillCommandToken.nextToken();
                if (debug){
                    System.out.println("User defined an application to destroy the started process.");
                    System.out.println("Trying to execute: "+sKillCommand);
                }

                try
                {

                    Process myprc = Runtime.getRuntime().exec(sKillCommand);
                    myprc.waitFor();

                } catch (InterruptedException ex) {
                    ex.printStackTrace();
                } catch (java.io.IOException e) {
                    e.printStackTrace ();
                }
            }
        }

        ProcessHandler ph = (ProcessHandler) param.get("AppProvider");

        if (ph != null) {
            // dispose watcher in case it's still running.
            OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

            if ((ow != null) && ow.isAlive()) {
                ow.finish = true;
            }

            ph.kill();
        }

        param.remove("AppProvider");
        param.remove("ServiceFactory");

        //copy user_backup into user layer
        try {
            String userLayer = (String) param.get("userLayer");
            String copyLayer = (String) param.get("copyLayer");
            if (userLayer != null && copyLayer != null) {
                File copyFile = new File(copyLayer);
                if (debug) System.out.println("copy '" + copyFile + "' ->" + userLayer + "'");
                FileTools.copyDirectory(copyFile, new File(userLayer), new String[]{"temp"});

                // remove all user_backup folder in temp dir
                // this is for the case the runner was killed and some old backup folder still stay in temp dir


            }
            else
                System.out.println("Cannot copy layer: " + copyLayer + " back to user layer: " + userLayer);
        } catch (java.io.IOException e) {
            if (debug) {
                System.out.println("Couldn't recover from backup");
                e.printStackTrace();
            }
        }

        return result;
    }

    protected boolean closeAllWindows(XDesktop desk) {
        XEnumerationAccess compEnumAccess = desk.getComponents();
        XEnumeration compEnum = compEnumAccess.createEnumeration();
        boolean res = true;

        try {
            while (compEnum.hasMoreElements()) {
                XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                XCloseable.class,
                compEnum.nextElement());

                if (closer != null) {
                    closer.close(true);
                }
            }
        } catch (com.sun.star.util.CloseVetoException cve) {
            res = false;
        } catch (com.sun.star.container.NoSuchElementException nsee) {
            res = false;
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res = false;
        }

        return res;
    }


    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;

        try {
            xPathSubst = xMSF.createInstance(
            "com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        if (xPathSubst != null) {
            return (XStringSubstitution) UnoRuntime.queryInterface(
            XStringSubstitution.class, xPathSubst);
        } else {
            return null;
        }
    }

    /**
     * converts directory without 'file:///' prefix.
     * and System dependend file separator
     */
    public static String getDirSys(String dir) {
        String sysDir = "";

        int idx = dir.indexOf("file://");

        int idx2 = dir.indexOf("file:///");

        // remove leading 'file://'
        if (idx < 0) {
            sysDir = dir;
        } else {
            sysDir = dir.substring("file://".length());
        }

        sysDir = utils.replaceAll13(sysDir, "%20", " ");

        // append '/' if not there (e.g. linux)
        if (sysDir.charAt(sysDir.length() - 1) != '/') {
            sysDir += "/";
        }

        // remove leading '/' and replace others with '\' on windows machines
        String sep = System.getProperty("file.separator");

        if (sep.equalsIgnoreCase("\\")) {
            if (!(idx2 < 0)) {
                sysDir = sysDir.substring(1);
            } else {
                //network path
                sysDir = "//"+sysDir;
            }
            sysDir = sysDir.replace('/', '\\');
        }

        return sysDir;
    }

    /**
     * If the office is connected but the <CODE>AppExecutionCommand</CODE> is not set,
     * this function asks the office for its location and fill the
     * <CODE>AppExecutionCommand</CODE> with valid contet.
     * This function was only called if parameter <CODE>AutoRestart</CODE> is set.
     * @param msf the <CODE>MultiServiceFactory</CODE>
     * @param param the <CODE>TestParameters</CODE>
     */
    private static void makeAppExecCommand(XMultiServiceFactory msf, TestParameters param){

        boolean debug = param.getBool(util.PropertyName.DEBUG_IS_ACTIVE);

        // get existing AppExecutionCommand if available, else empty string
        String command = (String) param.get(util.PropertyName.APP_EXECUTION_COMMAND);

        String connectionString = (String) param.get(util.PropertyName.CONNECTION_STRING);

        String sysBinDir = "";

        try{
           sysBinDir  = utils.getSystemURL(utils.expandMacro(msf, "$SYSBINDIR"));
        } catch (java.lang.Exception e){
            if (debug) System.out.println("could not get system binary directory");
            return;
        }

        // does the existing command show to the connected office?
        if (command.indexOf(sysBinDir) == -1){
            command = sysBinDir + System.getProperty("file.separator") + "soffice"+
                      " -norestore -accept=" + connectionString + ";urp;";
        }

        if (debug){
            System.out.println("update AppExecutionCommand: " + command);
        }

        Object dummy = param.put(util.PropertyName.APP_EXECUTION_COMMAND, command);
    }


}