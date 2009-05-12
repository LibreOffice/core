/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeSettings.java,v $
 * $Revision: 1.8 $
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
