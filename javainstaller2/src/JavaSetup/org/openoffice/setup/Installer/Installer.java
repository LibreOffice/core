/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Installer.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.setup.Installer;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.SetupData.PackageDescription;

public abstract class Installer {

    public Installer() {
    }

    public void preInstall(PackageDescription packageData) {
    }

    public void postInstall(PackageDescription packageData) {
    }

    public void preUninstall(PackageDescription packageData) {
    }

    public void postUninstall(PackageDescription packageData) {
    }

    public void preInstallationOngoing () {
    }

    public void postInstallationOngoing () {
    }

    public void postUninstallationOngoing () {
    }

    public void defineDatabasePath() {
    }

    public String getChangeInstallDir(PackageDescription packageData) {
        return null;
    }

    public boolean isPackageNameInstalled(String packageName, InstallData installData) {
        return false;
    }

    public abstract void installPackage(PackageDescription packageData);

    public abstract void uninstallPackage(PackageDescription packageData);

    public abstract boolean isPackageInstalled(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstalledPackageOlder(PackageDescription packageData, InstallData installData);

    public abstract boolean isInstallSetPackageOlder(PackageDescription packageData, InstallData installData);
}
