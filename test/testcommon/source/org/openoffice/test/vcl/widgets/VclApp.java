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


import java.util.Properties;

import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.VclHook;

public class VclApp {
    String home = null;

    public static final String CMD_KILL_WINDOWS = "taskkill /F /IM soffice.bin /IM soffice.exe";

    public static final String CMD_KILL_LINUX = "killall -9 soffice soffice.bin";

    String cmdKill = null;

    String cmdStart = null;

    String versionFile = null;

    Properties version = null;

    String port = System.getProperty("openoffice.automation.port", "12479");

    public VclApp(String appHome) {
        setHome(appHome);
    }

    public VclApp() {
        this(null);
    }

    public void setHome(String home) {
        this.home = home;
        if (home == null)
            home = System.getProperty("openoffice.home");
        if (home == null)
            home = System.getenv("OPENOFFICE_HOME");
        if (home == null)
            home = "unkown";

        versionFile = "versionrc";

        cmdKill = CMD_KILL_LINUX;
        cmdStart = "cd \"" + home + "\" ; ./soffice";
        if (SystemUtil.isWindows()) {
            cmdKill = CMD_KILL_WINDOWS;
            cmdStart = "\"" + home + "\\soffice.exe\"";
            versionFile = "version.ini";

        } else if (SystemUtil.isMac()) {

        } else {

        }

    }
    public String getHome() {
        return home;
    }

    public void kill() {
        SystemUtil.execScript(cmdKill, false);
    }

    public int start(String args) {
        if (args == null)
            args = "";

        return SystemUtil.execScript(cmdStart + " -norestore -quickstart=no -nofirststartwizard -enableautomation -automationport=" + port + " " + args, true);
    }

    public void start() {
        start(null);
    }

    /**
     * Activate the document window at the given index
     *
     * @param i
     * @return
     */
    public void activateDoc(int i) {
        VclHook.invokeCommand(Constant.RC_ActivateDocument,
                new Object[] { i + 1 });
    }

    public void reset() {
        VclHook.invokeCommand(Constant.RC_ResetApplication);
    }

    public boolean existsSysDialog() {
        return (Boolean) VclHook.invokeCommand(Constant.RC_ExistsSysDialog);
    }

    public void closeSysDialog() {
        VclHook.invokeCommand(Constant.RC_CloseSysDialog);
    }

    public String getClipboard() {
        return (String) VclHook.invokeCommand(Constant.RC_GetClipboard);
    }

    public void setClipboard(String content) {
        VclHook.invokeCommand(Constant.RC_SetClipboard, content);
    }

    public boolean exists() {
        return VclHook.available();
    }

    /**
     * Check if the control exists in a period of time
     */
    public boolean exists(double iTimeout) {
        return exists(iTimeout, 1);
    }

    /**
     * Check if the control exists in a period of time
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

    public void waitForExistence(double iTimeout, double interval) {
        if (!exists(iTimeout, interval))
            throw new RuntimeException("OpenOffice is not found!");
    }

    /**
     * Get document window count
     *
     * @return
     */
    public int getDocCount() {
        return (Integer) VclHook.invokeCommand(Constant.RC_GetDocumentCount);
    }

    public Properties getVersion() {
        if (version == null)
            version = FileUtil.loadProperties(home + "/" + versionFile);
        return version;
    }


    public void dispatch(String url) {
        VclHook.invokeUNOSlot(url);
    }

    private static final int CONST_WSTimeout = 701;
//  private static final int CONST_WSAborted = 702; // Not used now!
//  private static final int CONST_WSFinished = 703; //

    public void dispatch(String url, double time) {
        VclHook.invokeUNOSlot(url);
        int result = (Integer) VclHook.invokeCommand(Constant.RC_WaitSlot, (int) time * 1000);
        if (result == CONST_WSTimeout)
            throw new RuntimeException("Timeout to execute the dispatch!");
    }
}
