/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Installer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 12:32:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
