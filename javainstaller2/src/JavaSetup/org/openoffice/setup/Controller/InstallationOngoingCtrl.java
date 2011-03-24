/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.InstallationOngoing;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.InfoDir;
import org.openoffice.setup.Util.PackageCollector;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.Collections;
import java.util.Vector;

public class InstallationOngoingCtrl extends PanelController {

    private String helpFile;
    private String nextDialog = new String("InstallationCompleted");
    private String previousDialog = new String("InstallationImminent");

    public InstallationOngoingCtrl() {
        super("InstallationOngoing", new InstallationOngoing());
        helpFile = "String_Helpfile_InstallationOngoing";
    }

    public String getNext() {
        return new String("InstallationCompleted");
    }

    public String getPrevious() {
        return new String("InstallationImminent");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_NEXT);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_CANCEL);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_HELP);

        InstallationOngoing panel = (InstallationOngoing)getPanel();
        panel.setStopButtonActionCommand(getSetupFrame().ACTION_STOP);
        panel.addStopButtonActionListener(getSetupFrame().getSetupActionListener());

        // creating sorted list of packages to install
        InstallData installData = InstallData.getInstance();
        Vector installPackages = installData.getInstallPackages();

        Vector sortedPackages = new Vector();
        PackageCollector.sortPackages(installPackages, sortedPackages, "install");
        installData.setInstallPackages(sortedPackages);

        if ( installData.isMajorUpgrade() ) {
            // PackageCollector.findOldPackages(installData);
            // Sorting for correct order of uninstallation
            Vector sortedUninstallPackages = new Vector();
            PackageCollector.sortPackages(installData.getOldPackages(), sortedUninstallPackages, "uninstall");
            installData.setOldPackages(sortedUninstallPackages);
        }

        Installer installer = InstallerFactory.getInstance();
        installer.preInstallationOngoing();
    }

    public void duringShow() {

        Thread t = new Thread() {

            InstallData installData = InstallData.getInstance();
            InstallationOngoing panel = (InstallationOngoing)getPanel();
            Vector installPackages = installData.getInstallPackages();
            Vector removePackages = installData.getOldPackages();
            private Vector installedPackages = new Vector();

            public void run() {
                boolean ignoreMajorUpgrade = false;
                LogManager.setCommandsHeaderLine("Installation");
                Installer installer = InstallerFactory.getInstance();
                String titleText = ResourceManager.getString("String_InstallationOngoing1");
                panel.setTitle(titleText);

                for (int i = 0; i < installPackages.size(); i++) {
                    PackageDescription packageData = (PackageDescription) installPackages.get(i);
                    int progress = java.lang.Math.round((100*(i+1))/installPackages.size());
                    panel.setProgressValue(progress);
                    panel.setProgressText(packageData.getPackageName());

                    // Creating an upgrade process for Solaris packages
                    if ( installData.getOSType().equalsIgnoreCase("SunOS") ) {
                        if ( installer.isPackageInstalled(packageData, installData) ) {
                            if ( installer.isInstalledPackageOlder(packageData, installData) ) {
                                packageData.setIgnoreDependsForUninstall(true);
                                installer.uninstallPackage(packageData);
                            } else {
                                continue;  // no downgrading
                            }
                        }
                    }

                    installer.installPackage(packageData);
                    installedPackages.add(packageData);

                    if (( installData.isAbortedInstallation() ) || ( installData.isErrorInstallation() )) {
                        ignoreMajorUpgrade = true;
                        break;
                    }
                }

                if (( installData.isMajorUpgrade() ) && ( ! ignoreMajorUpgrade )) {
                    for (int i = 0; i < removePackages.size(); i++) {
                        PackageDescription packageData = (PackageDescription) removePackages.get(i);
                        installer.uninstallPackage(packageData);
                    }
                }

                if (( installData.isAbortedInstallation() ) || ( installData.isErrorInstallation() )) {
                    // undoing the installation
                    if ( installData.isAbortedInstallation() ) {
                        LogManager.setCommandsHeaderLine("Installation aborted!");
                        titleText = ResourceManager.getString("String_UninstallationOngoing1");
                    } else {
                        LogManager.setCommandsHeaderLine("Error during installation!");
                        titleText = ResourceManager.getString("String_UninstallationOngoing1");
                    }
                    panel.setTitle(titleText);
                    panel.setStopButtonEnabled(false);

                    LogManager.setCommandsHeaderLine("Uninstallation");

                    // Inverting the package order for uninstallation
                    Collections.reverse(installedPackages);

                    for (int i = 0; i < installedPackages.size(); i++) {
                        PackageDescription packageData = (PackageDescription) installedPackages.get(i);
                        int progress = java.lang.Math.round(100/installedPackages.size()) * (i+1);
                        panel.setProgressValue(progress);
                        panel.setProgressText(packageData.getPackageName());
                        if ( i == 0 ) {
                            installData.setIsFirstPackage(true);
                        } else {
                            installData.setIsFirstPackage(false);
                        }
                        installer.uninstallPackage(packageData);
                     }

                    // removing already created helper files (admin files)
                    Vector removeFiles = installData.getRemoveFiles();
                    for (int i = 0; i < removeFiles.size(); i++) {
                        File removeFile = new File((String)removeFiles.get(i));
                        SystemManager.deleteFile(removeFile);
                    }
                }

                installer.postInstallationOngoing();

                String next = getNext();
                getSetupFrame().setCurrentPanel(next, false, true);
            }
         };

         t.start();
     }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_PREVIOUS);
        getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_NEXT);
        getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_CANCEL);

        InstallData installData = InstallData.getInstance();
        if (( ! installData.isAbortedInstallation() ) && ( ! installData.isErrorInstallation() )) {
            InfoDir.prepareUninstallation();
        }

        return repeatDialog;
    }

}
