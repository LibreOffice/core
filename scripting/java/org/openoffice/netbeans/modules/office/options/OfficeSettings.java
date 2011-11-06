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



package org.openoffice.netbeans.modules.office.options;

import java.util.Hashtable;
import java.util.Enumeration;
import java.io.File;
import java.io.IOException;

import org.openide.options.SystemOption;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;

import org.openoffice.idesupport.SVersionRCFile;
import org.openoffice.idesupport.OfficeInstallation;

/** Options for something or other.
 *
 * @author tomaso
 */
public class OfficeSettings extends SystemOption {

    // private static final long serialVersionUID = ...;

    public static final String OFFICE_DIRECTORY = "OfficeDirectory";
    public static final String WARN_BEFORE_DOC_DEPLOY = "WarnBeforeDocDeploy";
    public static final String WARN_BEFORE_PARCEL_DELETE = "WarnBeforeParcelDelete";
    public static final String WARN_AFTER_DIR_DEPLOY = "WarnAfterDirDeploy";
    public static final String WARN_BEFORE_MOUNT = "WarnBeforeMount";

    protected void initialize() {
        super.initialize();

        setWarnBeforeDocDeploy(true);
        setWarnBeforeParcelDelete(true);
        setWarnAfterDirDeploy(true);
        setWarnBeforeMount(true);

        if (getOfficeDirectory() == null) {
            SVersionRCFile sversion = SVersionRCFile.createInstance();

            try {
                Enumeration enum = sversion.getVersions();
                OfficeInstallation oi;

                while (enum.hasMoreElements()) {
                    oi = (OfficeInstallation)enum.nextElement();
                    setOfficeDirectory(oi);
                    return;
                }
            }
            catch (IOException ioe) {
            }
        }
    }

    public String displayName() {
        return "Office Settings";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    public static OfficeSettings getDefault() {
        return (OfficeSettings)findObject(OfficeSettings.class, true);
    }

    public OfficeInstallation getOfficeDirectory() {
        return (OfficeInstallation)getProperty(OFFICE_DIRECTORY);
    }

    public void setOfficeDirectory(OfficeInstallation oi) {
        putProperty(OFFICE_DIRECTORY, oi, true);
    }

    public boolean getWarnBeforeDocDeploy() {
        return ((Boolean)getProperty(WARN_BEFORE_DOC_DEPLOY)).booleanValue();
    }

    public void setWarnBeforeDocDeploy(boolean value) {
        putProperty(WARN_BEFORE_DOC_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnBeforeParcelDelete() {
        return ((Boolean)getProperty(WARN_BEFORE_PARCEL_DELETE)).booleanValue();
    }

    public void setWarnBeforeParcelDelete(boolean value) {
        putProperty(WARN_BEFORE_PARCEL_DELETE, new Boolean(value), true);
    }

    public boolean getWarnAfterDirDeploy() {
        return ((Boolean)getProperty(WARN_AFTER_DIR_DEPLOY)).booleanValue();
    }

    public void setWarnAfterDirDeploy(boolean value) {
        putProperty(WARN_AFTER_DIR_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnBeforeMount() {
        return ((Boolean)getProperty(WARN_BEFORE_MOUNT)).booleanValue();
    }

    public void setWarnBeforeMount(boolean value) {
        putProperty(WARN_BEFORE_MOUNT, new Boolean(value), true);
    }
}
