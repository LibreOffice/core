/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InstallChangeCtrl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:02:11 $
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

package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.Panel.ChooseDirectory;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import java.util.Enumeration;


public class InstallChangeCtrl {

    private InstallChangeCtrl() {
    }

    static private void setUpdatePackage(PackageDescription packageData, InstallData installData) {
        if (( packageData.isUpdatePackage() == true )) {
            installData.setUpdatePackage(packageData);
        } else {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                setUpdatePackage(child, installData);
            }
        }
    }

    static private void setChangeInstallDir(InstallData installData, Installer installer) {
        // setting the new install dir after analyzing the installation directory
        // of the installed update package.
        String changeInstallDir = installer.getChangeInstallDir(installData.getUpdatePackage());
        installData.setInstallDir(changeInstallDir);
    }

    // static public void checkInstallChange(InstallData data, ChooseDirectory panel) {
    static public void checkInstallChange(InstallData data) {

        Installer installer = InstallerFactory.getInstance();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        if ( data.getUpdatePackage() == null ) {
            setUpdatePackage(packageData, data);
        }

        if ( data.getUpdatePackage() != null ) {

            // resetting values, if database was changed during user installation
            data.setOlderVersionExists(false);
            data.setNewerVersionExists(false);
            data.setSameVersionExists(false);

            boolean packageIsInstalled = installer.isPackageInstalled(data.getUpdatePackage(), data);
            if ( packageIsInstalled ) {
                // Checking version of installed package:
                // If installed package is older: Force update mode, no selection of packages
                // If installed package is equal: Force maintenance mode, only selection of packages
                // If installed package is newer: Abort installation with message

                // Setting specific values for the different update scenarios
                if ( installer.isInstalledPackageOlder(data.getUpdatePackage(), data) ) {
                    data.setOlderVersionExists(true);
                    // All installed packages will be updated -> determining which packages are installed
                    System.err.println("An older product is installed");
                } else if ( installer.isInstallSetPackageOlder(data.getUpdatePackage(), data) ) {
                    data.setNewerVersionExists(true);
                    System.err.println("A newer product is installed");
                } else {
                    data.setSameVersionExists(true);
                    System.err.println("Same product is installed");
                }

                // If installed package is older or equal, the installdir has to be fixed
                // if this is a root installation
                if ( data.isRootInstallation() ) {
                    setChangeInstallDir(data, installer);
                    data.setIsChangeInstallation(true);
                }
            }
        }
    }

}
