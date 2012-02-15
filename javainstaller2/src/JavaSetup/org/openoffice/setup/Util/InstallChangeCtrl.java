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



package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.Panel.ChooseDirectory;
import org.openoffice.setup.ResourceManager;
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
        if ( installData.isRootInstallation() ) {
            String changeInstallDir = "/";
            installData.setInstallDir(changeInstallDir);
        } else {
            String changeInstallDir = installer.getChangeInstallDir(installData.getUpdatePackage());
            installData.setInstallDir(changeInstallDir);
        }
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
                    // But if this is a kind of Major Upgrade with different Minor and therefore different package names,
                    // it is necessary to remove the old product.
                    if ( data.getProductMinor() > data.getInstalledProductMinor() )
                    {
                        data.setMajorUpgrade(true);
                        System.err.println("Major Upgrade");
                    }
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

                // Exit installation, if update is not wanted and this is a root installation.
                // In installations without root privileges, the user can choose another installation
                // directory (ChooseDirectoryCtrl.java).
                if ( data.isRootInstallation() && data.dontUpdate() && data.olderVersionExists() ) {
                    System.err.println("Error: An older version is already installed in directory " + data.getInstallDir() + "!");
                    String message1 = ResourceManager.getString("String_Older_Version_Installed_Found")
                                    + "\n" + data.getInstallDir() + "\n";
                    String message2 = ResourceManager.getString("String_Older_Version_Installed_Remove");
                    String message = message1 + "\n" + message2;
                    String title = ResourceManager.getString("String_Error");
                    Informer.showErrorMessage(message, title);
                    System.exit(1);
                }
            }
        }
    }

}
