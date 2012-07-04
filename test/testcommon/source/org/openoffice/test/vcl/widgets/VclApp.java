/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package org.openoffice.test.vcl.widgets;

import java.io.File;
import java.net.MalformedURLException;
import java.util.Properties;

import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.VclHook;

/**
 * This class provides a proxy to interact with OpenOffice application.
 *
 */
public class VclApp {

    private String port = System.getProperty("openoffice.automation.port", "12479");

    private File userInstallation = null;

    private File defaultUserInstallation = null;

    private File home = null;

    private String args = null;

    private boolean automationEnabled = true;

    /**
     * Construct VclApp with the home path of OpenOffice. The home is the
     * directory which contains soffice.bin.
     *
     * @param appHome
     */
    public VclApp(String appHome) {
        if (appHome == null)
            appHome = System.getProperty("openoffice.home");
        if (appHome == null)
            appHome = System.getenv("OPENOFFICE_HOME");
        if (appHome == null) {
            if (SystemUtil.isWindows()) {
                appHome = "C:/Program Files/OpenOffice.org 3/program";
                if (!new File(appHome).exists())
                    appHome = "C:/Program Files (x86)/OpenOffice.org 3/program";
            } else if (SystemUtil.isMac()) {
                appHome = "/Applications/OpenOffice.org.app/Contents/MacOS";
            } else {
                appHome = "/opt/openoffice.org3/program";
            }
        }

        home = new File(appHome);

        File bootstrapFile = new File(home, "bootstraprc");
        if (!bootstrapFile.exists())
            bootstrapFile = new File(home, "bootstrap.ini");
        if (!bootstrapFile.exists())
            throw new RuntimeException("OpenOffice can not be found or it's broken.");

        Properties props = FileUtil.loadProperties(bootstrapFile);
        String defaultUserInstallationPath = props.getProperty("UserInstallation");
        String sysUserConfig = null;
        if (SystemUtil.isWindows()) {
            sysUserConfig = System.getenv("APPDATA");
        } else if (SystemUtil.isMac()) {
            sysUserConfig = System.getProperty("user.home") + "/Library/Application Support";
        } else {
            sysUserConfig = System.getProperty("user.home");
        }

        defaultUserInstallationPath = defaultUserInstallationPath.replace("$ORIGIN", home.getAbsolutePath()).replace("$SYSUSERCONFIG", sysUserConfig);
        defaultUserInstallation = new File(defaultUserInstallationPath);
    }

    /**
     * Set UserInstallation directory. When openoffice is launched, the argument
     * "-env:UserInstallation" will be enabled.
     *
     * @param dir
     *            user installation directory. If null is given, the default
     *            will be used.
     */
    public void setUserInstallation(File dir) {
        userInstallation = dir;
    }

    /**
     * Get UserInstallation directory
     *
     * @return
     */
    public File getUserInstallation() {
        return userInstallation == null ? defaultUserInstallation : userInstallation;
    }

    /**
     * Get default UserInstallation directory
     *
     * @return
     */
    public File getDefaultUserInstallation() {
        return defaultUserInstallation;
    }

    /**
     * Get installation directory of OpenOffice.
     *
     * @return
     */
    public File getHome() {
        return home;
    }

    /**
     * Set other command line arguments
     *
     * @param args
     */
    public void setArgs(String args) {
        this.args = args;
    }

    public boolean isAutomationEnabled() {
        return automationEnabled;
    }

    public void setAutomationEnabled(boolean automationEnabled) {
        this.automationEnabled = automationEnabled;
    }

    /**
     * Kill OpenOffice
     */
    public void kill() {
        if (SystemUtil.isWindows()) {
            SystemUtil.execScript("taskkill /F /IM soffice.bin /IM soffice.exe", false);
        } else {
            SystemUtil.execScript("killall -9 soffice soffice.bin", false);
        }

    }

    /**
     * Start OpenOffice
     *
     * @return
     */
    public int start() {
        String cmd = null;

        if (SystemUtil.isWindows()) {
            cmd = "\"" + home + "\\soffice.exe\"";
        } else {
            cmd = "cd \"" + home + "\" ; ./soffice";
        }

        if (automationEnabled) {
            cmd += " -norestore -quickstart=no -nofirststartwizard -enableautomation -automationport=" + port;
        }

        if (userInstallation != null) {
            try {
                String url = userInstallation.toURL().toString();
                url = url.replace("file:/", "file:///");
                cmd += " -env:UserInstallation=" + url;
            } catch (MalformedURLException e) {
                // ignore never to occur
            }
        }

        if (args != null)
            cmd += " " + args;

        return SystemUtil.execScript(cmd, true);
    }

    /**
     * Activate the document window at the given index
     * Note: this method requires automation enabled.
     * @param i
     * @return
     */
    public void activateDoc(int i) {
        VclHook.invokeCommand(Constant.RC_ActivateDocument, new Object[] { i + 1 });
    }

    /**
     * Try to close all OpenOffice windows, until startcenter window appears.
     * The method does not always succeed.
     * Note: this method requires automation enabled.
     */
    public void reset() {
        VclHook.invokeCommand(Constant.RC_ResetApplication);
    }

    /**
     * Note: this method requires automation enabled.
     * @return
     */
    public boolean existsSysDialog() {
        return (Boolean) VclHook.invokeCommand(Constant.RC_ExistsSysDialog);
    }

    /**
     * Note: this method requires automation enabled.
     */
    public void closeSysDialog() {
        VclHook.invokeCommand(Constant.RC_CloseSysDialog);
    }

    /**
     * Note: this method requires automation enabled.
     * @return
     */
    public String getClipboard() {
        return (String) VclHook.invokeCommand(Constant.RC_GetClipboard);
    }

    /**
     * Note: this method requires automation enabled.
     * @param content
     */
    public void setClipboard(String content) {
        VclHook.invokeCommand(Constant.RC_SetClipboard, content);
    }

    /**
     * Check if OpenOffice exists.
     * Note: this method requires automation enabled.
     * @return
     */
    public boolean exists() {
        return VclHook.available();
    }

    /**
     * Check if the control exists in a period of time
     * Note: this method requires automation enabled.
     */
    public boolean exists(double iTimeout) {
        return exists(iTimeout, 1);
    }

    /**
     * Check if the control exists in a period of time
     * Note: this method requires automation enabled.
     */
    public boolean exists(double iTimeout, double interval) {
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < iTimeout * 1000) {
            if (exists())
                return true;
            Tester.sleep(interval);
        }

        return exists();
    }

    /**
     * Wait OpenOffice for existence in the given period.
     * When time is out, an runtime exception will be thrown.
     * Note: this method requires automation enabled.
     * @param iTimeout
     * @param interval
     */
    public void waitForExistence(double iTimeout, double interval) {
        if (!exists(iTimeout, interval))
            throw new RuntimeException("OpenOffice is not found!");
    }

    /**
     * Get document window count
     * Note: this method requires automation enabled.
     * @return
     */
    public int getDocCount() {
        return (Integer) VclHook.invokeCommand(Constant.RC_GetDocumentCount);
    }

    /**
     * Run a dispatch
     * Note: this method requires automation enabled.
     * @param url
     */
    public void dispatch(String url) {
        VclHook.invokeUNOSlot(url);
    }

    private static final int CONST_WSTimeout = 701;

    // private static final int CONST_WSAborted = 702; // Not used now!
    // private static final int CONST_WSFinished = 703; //

    /**
     * Run a dispatch and then wait some time.
     * If time is out, an runtime exception will be thrown
     * Note: this method requires automation enabled.
     * @param url
     * @param time timeout
     */
    public void dispatch(String url, double time) {
        VclHook.invokeUNOSlot(url);
        int result = (Integer) VclHook.invokeCommand(Constant.RC_WaitSlot, (int) time * 1000);
        if (result == CONST_WSTimeout)
            throw new RuntimeException("Timeout to execute the dispatch!");
    }
}
