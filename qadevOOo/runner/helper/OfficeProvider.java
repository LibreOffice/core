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

import com.sun.star.beans.XFastPropertySet;
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
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;

import lib.TestParameters;

import share.DescEntry;
import share.LogWriter;

import util.DynamicClassLoader;
import util.PropertyName;
import util.utils;

/**
 * This class will connect the office and start it if possible
 *
 */
public class OfficeProvider implements AppProvider
{

    private boolean debug = false;

    /**
     * copy the user layer to a safe place, usually to $TMP/user_backup$USER
     */
    private void backupUserLayer(TestParameters param, XMultiServiceFactory msf)
    {
        try
        {
            final XStringSubstitution sts = createStringSubstitution(msf);
            debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

            String userLayer = sts.getSubstituteVariableValue("$(user)");
            userLayer = getDirSys(userLayer);
            param.put("userLayer", userLayer);

            final String copyLayer = util.utils.getUsersTempDir() + System.getProperty("file.separator") +
                    "user_backup" +
                    System.getProperty("user.name");
            param.put("copyLayer", copyLayer);


            dbg(" copy '" + userLayer + "' ->" + copyLayer + "'");
            // Slow machines the copy job could spend some time. To avoid activating of OfficeWatcher it must be pinged
            OfficeWatcherPing owp = new OfficeWatcherPing((OfficeWatcher) param.get(PropertyName.OFFICE_WATCHER));
            owp.start();

            deleteFilesAndDirector (new File(copyLayer));
            FileTools.copyDirectory(new File(userLayer), new File(copyLayer), new String[]
                    {
                        "temp"
                    });

            owp.finish();

        }
        catch (com.sun.star.container.NoSuchElementException e)
        {
            System.out.println("User Variable '$(user)' not defined.");
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println("Couldn't backup user layer");
            e.printStackTrace();
        }
        catch (java.io.IOException e)
        {
            System.out.println("Couldn't backup user layer");
            e.printStackTrace();
        }
    }

    /**
     * Dispose the office.
     * This method can only be used, if the office was connected in the first
     * place: getManager() was called first.
     * @return return true if desktop is terminates, else false
     */
    public boolean disposeManager(lib.TestParameters param)
    {

        XMultiServiceFactory msf = param.getMSF();

        if (msf == null)
        {
            return true;
        }
        else
        {
            XDesktop desk = null;

            try
            {
                desk = UnoRuntime.queryInterface(XDesktop.class, msf.createInstance("com.sun.star.frame.Desktop"));
            }
            catch (com.sun.star.uno.Exception ue)
            {
                return false;
            }

            msf = null;

            if (desk != null)
            {
                desk.terminate();

                return true;
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * Method to get the ServiceManager of an Office
     */
    public Object getManager(lib.TestParameters param) throws UnsupportedEncodingException
    {
        String errorMessage = null;
        boolean bAppExecutionHasWarning = false;
        debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        String additionalArgs = (String) param.get(
                "AdditionalConnectionArguments");

        if (additionalArgs == null)
        {
            additionalArgs = ";";
        }
        else
        {
            additionalArgs = "," + additionalArgs + ";";
        }

        final String cncstr = "uno:" + param.get("ConnectionString") + ";urp" +
                additionalArgs + "StarOffice.ServiceManager";

        System.out.println("Connecting the Office with " + cncstr);

        XMultiServiceFactory msf = connectOffice(cncstr);

        // if the office is running and the office crashes while testing it could
        // be useful to restart the office if possible and continue the tests.
        // Example: the UNO-API-Tests in the projects will be executed by calling
        // 'dmake'.
        if ((param.getBool(util.PropertyName.AUTO_RESTART)) && (msf != null))
        {
            makeAppExecCommand(msf, param);
        }

        if (msf == null)
        {
            String exc = "";
            Exception exConnectFailed = null;
            boolean isExecutable = false;
            boolean isAppKnown = ((cncstr.indexOf("host=localhost") > 0) || (cncstr.indexOf("pipe,name=") > 0));
            isAppKnown &= ((String) param.get("AppExecutionCommand")).length() != 0;

            if (isAppKnown)
            {
                dbg("Local Connection trying to start the Office");

                //ensure that a pending officewatcher gets finished before a new
                //office is started
                final OfficeWatcher ow_old = (OfficeWatcher) param.get("Watcher");

                if (ow_old != null)
                {
                    ow_old.finish = true;
                }

                final String cmd = (String) param.get("AppExecutionCommand");
                dbg("AppExecutionCommand: " + cmd);
                // validate the AppExecutionCommand, but try it out anyway.
                // keep the error message for later.
                errorMessage =
                        util.utils.validateAppExecutionCommand(cmd, (String) param.get("OperatingSystem"));
                if (errorMessage.startsWith("Error"))
                {
                    System.out.println(errorMessage);
                    return null;
                }
                bAppExecutionHasWarning = !errorMessage.equals("OK");

                final DynamicClassLoader dcl = new DynamicClassLoader();
                final LogWriter log = (LogWriter) dcl.getInstance(
                        (String) param.get("LogWriter"));

                //create empty entry
                final DescEntry Entry = new DescEntry();
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

                final ProcessHandler ph = new ProcessHandler(cmd, (PrintWriter) log);
                isExecutable = ph.executeAsynchronously();

                if (isExecutable)
                {
                    param.put("AppProvider", ph);
                    final OfficeWatcher ow = new OfficeWatcher(param);
                    param.put("Watcher", ow);
                    ow.start();
                    ow.ping();
                }

                int k = 0;

                // wait up to 21 seconds to get an office connection
                while ((k < 42) && (msf == null))
                {
                    try
                    {
                        msf = connect(cncstr);
                    }
                    catch (com.sun.star.uno.Exception ue)
                    {
                        exConnectFailed = ue;
                        exc = ue.getMessage();
                    }
                    catch (java.lang.Exception je)
                    {
                        exConnectFailed = je;
                        exc = je.getMessage();
                    }
                    if (msf == null)
                    {
                        util.utils.pause(500 * k);
                    }
                    k++;
                }

                if (msf == null)
                {
                    System.out.println("Exception while connecting.\n" + exConnectFailed);
                    if (exc != null)
                    {
                        System.out.println(exc);
                    }
                    if (bAppExecutionHasWarning)
                    {
                        System.out.println(errorMessage);
                    }
                }
                else if (isExecutable)
                {
                    backupUserLayer(param, msf);
                }
            }
            else
            {
                System.out.println("Could not connect an Office and cannot start one.\n".concat("please start an office with following parameter:\n").
                        concat("\nsoffice --accept=").concat((String) param.get("ConnectionString")).concat(";urp;\n"));
            }
        }

        return msf;
    }

    /**
     * Connect an Office
     * @param connectStr
     * @return
     * @throws com.sun.star.uno.Exception
     * @throws com.sun.star.uno.RuntimeException
     * @throws com.sun.star.connection.NoConnectException
     * @throws Exception
     */
    private XMultiServiceFactory connect(String connectStr)
            throws com.sun.star.uno.Exception,
            com.sun.star.uno.RuntimeException,
            com.sun.star.connection.NoConnectException,
            Exception
    {

        // Get component context
        final XComponentContext xcomponentcontext = com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

        // initial serviceManager
        final XMultiComponentFactory xLocalServiceManager = xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
        final Object xUrlResolver = xLocalServiceManager.createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", xcomponentcontext);
        final XUnoUrlResolver urlResolver = UnoRuntime.queryInterface(XUnoUrlResolver.class, xUrlResolver);

        final Object rInitialObject = urlResolver.resolve(connectStr);

        XMultiServiceFactory xMSF = null;

        if (rInitialObject != null)
        {
            // debug = true;
            dbg("resolved url");

            xMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, rInitialObject);
        }

        return xMSF;
    }

    /**
     * Close an office.
     * @param param The test parameters.
     * @param closeIfPossible If true, close even if
     * it was running before the test
     */
    public boolean closeExistingOffice(lib.TestParameters param, boolean closeIfPossible)
    {

        XMultiServiceFactory msf = param.getMSF();
        final boolean alreadyConnected = (msf != null);
        debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        if (alreadyConnected)
        {
            dbg("try to get ProcessHandler");

            final ProcessHandler ph = (ProcessHandler) param.get("AppProvider");

            if (ph != null)
            {
                dbg("ProcessHandler != null");

                disposeOffice(msf, param);

                // dispose watcher in case it's still running.
                dbg("try to get OfficeWatcher");

                final OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

                if ((ow != null) && ow.isAlive())
                {
                    dbg("OfficeWatcher will be finished");
                    ow.finish = true;
                }
                else
                {
                    dbg("OfficeWatcher seems to be finished");
                }

                return true;
            }
            else
            {
                if (closeIfPossible)
                {
                    return disposeOffice(msf, param);
                }
            }
        }
        else
        {
            final String cncstr = "uno:" + param.get("ConnectionString") +
                    ";urp;StarOffice.ServiceManager";
            dbg("try to connect office");
            msf = connectOffice(cncstr);

            if (closeIfPossible)
            {
                return disposeOffice(msf, param);
            }
        }
        dbg("closeExistingOffice finished");
        return true;
    }

    private XMultiServiceFactory connectOffice(String cncstr)
    {
        XMultiServiceFactory msf = null;
        String exc = "";
        // debug = true;

        dbg("trying to connect to " + cncstr);

        try
        {
            msf = connect(cncstr);
        }
        catch (com.sun.star.uno.Exception ue)
        {
            exc = ue.getMessage();
        }
        catch (java.lang.Exception je)
        {
            exc = je.getMessage();
        }

        if (debug && exc != null && exc.length() != 0)
        {
            dbg("Could not connect an Office. " + exc);
        }

        return msf;
    }

    private boolean disposeOffice(XMultiServiceFactory msf,
            TestParameters param)
    {
        XDesktop desk = null;

        debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        boolean result = true;

        if (msf != null)
        {

            // disable QuickStarter
            try
            {
                Object quickStarter = msf.createInstance("com.sun.star.office.Quickstart");
                XFastPropertySet fps = UnoRuntime.queryInterface(XFastPropertySet.class, quickStarter);
                fps.setFastPropertyValue(0, false);
            }
            catch (com.sun.star.uno.Exception ex)
            {
                dbg("ERROR: Could not disable QuickStarter: " + ex.toString());
            }

            try
            {
                desk = UnoRuntime.queryInterface(XDesktop.class, msf.createInstance("com.sun.star.frame.Desktop"));
                msf = null;

                if (desk != null)
                {
                    final boolean allClosed = closeAllWindows(desk);

                    if (!allClosed)
                    {
                        dbg("Couldn't close all office windows!");
                    }

                    dbg("Trying to terminate the desktop");

                    desk.terminate();
                    dbg("Desktop terminated");
                }
            }
            catch (com.sun.star.uno.Exception ue)
            {
                result = false;
            }
            catch (com.sun.star.lang.DisposedException ue)
            {
                result = false;
            }
        }

        final ProcessHandler ph = (ProcessHandler) param.get("AppProvider");

        if (ph != null)
        {
            // dispose watcher in case it's still running.
            final OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

            if ((ow != null) && ow.isAlive())
            {
                ow.finish = true;
            }

            ph.kill();
        }

        param.remove("AppProvider");
        param.remove("ServiceFactory");

        //copy user_backup into user layer
        try
        {
            final String userLayer = (String) param.get("userLayer");
            final String copyLayer = (String) param.get("copyLayer");
            if (userLayer != null && copyLayer != null)
            {
                deleteFilesAndDirector(new File(userLayer));
                final File copyFile = new File(copyLayer);
                dbg("copy '" + copyFile + "' -> '" + userLayer + "'");
                FileTools.copyDirectory(copyFile, new File(userLayer), new String[]
                        {
                            "temp"
                        });
                dbg("copy '" + copyFile + "' -> '" + userLayer + "' finished");

            // remove all user_backup folder in temp dir
            // this is for the case the runner was killed and some old backup folder still stay in temp dir


            }
            else
            {
                System.out.println("Cannot copy layer: '" + copyLayer + "' back to user layer: '" + userLayer + "'");
            }
        }
        catch (java.io.IOException e)
        {
            dbg("Couldn't recover from backup\n" + e.getMessage());
        }
        return result;
    }

    private boolean closeAllWindows(XDesktop desk)
    {
        final XEnumerationAccess compEnumAccess = desk.getComponents();
        final XEnumeration compEnum = compEnumAccess.createEnumeration();
        boolean res = true;

        try
        {
            while (compEnum.hasMoreElements())
            {
                final XCloseable closer = UnoRuntime.queryInterface(XCloseable.class, compEnum.nextElement());

                if (closer != null)
                {
                    closer.close(true);
                }
            }
        }
        catch (com.sun.star.util.CloseVetoException cve)
        {
            res = false;
        }
        catch (com.sun.star.container.NoSuchElementException nsee)
        {
            res = false;
        }
        catch (com.sun.star.lang.WrappedTargetException wte)
        {
            res = false;
        }

        return res;
    }

    private static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) throws com.sun.star.uno.Exception
    {
        Object xPathSubst = xMSF.createInstance(
                    "com.sun.star.util.PathSubstitution");
        return UnoRuntime.queryInterface(XStringSubstitution.class, xPathSubst);
    }

    /**
     * converts directory without 'file:///' prefix.
     * and System dependent file separator
     */
    private static String getDirSys(String dir)
    {
        String sysDir = "";

        final int idx = dir.indexOf("file://");

        final int idx2 = dir.indexOf("file:///");

        // remove leading 'file://'
        if (idx < 0)
        {
            sysDir = dir;
        }
        else
        {
            sysDir = dir.substring("file://".length());
        }

        sysDir = sysDir.replace("%20", " ");

        // append '/' if not there (e.g. linux)
        if (sysDir.charAt(sysDir.length() - 1) != '/')
        {
            sysDir += "/";
        }

        // remove leading '/' and replace others with '\' on windows machines
        final String sep = System.getProperty("file.separator");

        if (sep.equalsIgnoreCase("\\"))
        {
            if (idx2 >= 0)
            {
                sysDir = sysDir.substring(1);
            }
            else
            {
                //network path
                sysDir = "//" + sysDir;
            }
            sysDir = sysDir.replace('/', '\\');
        }

        return sysDir;
    }

    /**
     * If the office is connected but the <CODE>AppExecutionCommand</CODE> is not set,
     * this function asks the office for its location and fill the
     * <CODE>AppExecutionCommand</CODE> with valid content.
     * This function was only called if parameter <CODE>AutoRestart</CODE> is set.
     * @param msf the <CODE>MultiServiceFactory</CODE>
     * @param param the <CODE>TestParameters</CODE>
     */
    private void makeAppExecCommand(XMultiServiceFactory msf, TestParameters param)
    {
        debug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        // get existing AppExecutionCommand if available, else empty string
        String command = (String) param.get(util.PropertyName.APP_EXECUTION_COMMAND);

        String connectionString;
        if (param.getBool(util.PropertyName.USE_PIPE_CONNECTION))
        {
            // This is the default behaviour
            connectionString = (String) param.get(util.PropertyName.PIPE_CONNECTION_STRING);
        }
        else
        {
            // is used if UsePipeConnection=false
            connectionString = (String) param.get(util.PropertyName.CONNECTION_STRING);
        }

        String sysBinDir = "";

        try
        {
            sysBinDir = utils.getSystemURL(utils.expandMacro(msf, "$SYSBINDIR"));
        }
        catch (java.lang.Exception e)
        {
            dbg("could not get system binary directory");
            return;
        }

        // does the existing command show to the connected office?
        if (command.indexOf(sysBinDir) == -1)
        {
            command = sysBinDir + System.getProperty("file.separator") + "soffice" +
                    " --norestore --accept=" + connectionString + ";urp;";
        }

        dbg("update AppExecutionCommand: " + command);

        param.put(util.PropertyName.APP_EXECUTION_COMMAND, command);
    }

    private void dbg(String message)
    {
        if (debug)
        {
            System.out.println(utils.getDateTime() + "OfficeProvider: " + message);
        }

    }

    private static class OfficeWatcherPing extends Thread
    {

        private final OfficeWatcher ow;
        private boolean bStop = false;

        public OfficeWatcherPing(OfficeWatcher ow)
        {
            this.ow = ow;
        }

        @Override
        public void run()
        {
            System.out.println(utils.getDateTime() + "OfficeProvider:Owp: start ");

            while (!bStop)
            {
                System.out.println(utils.getDateTime() + "OfficeProvider:Owp: ping ");
                ow.ping();
                System.out.println(utils.getDateTime() + "OfficeProvider:Owp: sleep ");
                util.utils.pause(1000);
            }

        }

        public void finish()
        {
            synchronized(this)
            {
                bStop = true;
                System.out.println(utils.getDateTime() + "OfficeProvider:Owp: stop ");

                notify();
            }
        }
    }

    private void deleteFilesAndDirector(File file)
    {
        File f = file;
        if(f.isDirectory())
        {
            File files[] = f.listFiles();
            int filesLength = files != null ? files.length : 0;
            for(int i = 0; i < filesLength; ++i)
            {
                deleteFilesAndDirector(files[i]);
            }
            boolean bDeleteOk = f.delete();
            if (!bDeleteOk) {
                System.out.println("delete failed");
            }
        }
        else if (f.isFile())
        {
            boolean bDeleteOk = f.delete();
            if (!bDeleteOk) {
                System.out.println("delete failed");
            }
        }
    }
}
