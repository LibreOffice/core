/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UninstallationOngoingCtrl.java,v $
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

package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationOngoing;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InfoDir;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.PackageCollector;
import java.util.Vector;
public class UninstallationOngoingCtrl extends PanelController {

    private String helpFile;

    public UninstallationOngoingCtrl() {
        super("UninstallationOngoing", new UninstallationOngoing());
        helpFile = "String_Helpfile_UninstallationOngoing";
    }

    public String getNext() {
        return new String("UninstallationCompleted");
    }

    public String getPrevious() {
        return new String("UninstallationImminent");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_NEXT);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_CANCEL);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_HELP);

        UninstallationOngoing panel = (UninstallationOngoing)getPanel();
        panel.setStopButtonActionCommand(getSetupFrame().ACTION_STOP);
        panel.addStopButtonActionListener(getSetupFrame().getSetupActionListener());

        // creating list of packages to uninstall
        InstallData data = InstallData.getInstance();
        Vector uninstallPackages = new Vector();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();
        PackageCollector.collectUninstallPackages(packageData, uninstallPackages);
        data.setInstallPackages(uninstallPackages);

        // collectPackages(packageData);

        Installer installer = InstallerFactory.getInstance();
        installer.preInstallationOngoing();
    }

    public void duringShow() {

        Thread t = new Thread() {

            UninstallationOngoing panel = (UninstallationOngoing)getPanel();
            InstallData installData = InstallData.getInstance();
            Vector uninstallPackages = installData.getInstallPackages();

            public void run() {
                LogManager.setCommandsHeaderLine("Uninstallation");
                Installer installer = InstallerFactory.getInstance();

                for (int i = 0; i < uninstallPackages.size(); i++) {
                    PackageDescription packageData = (PackageDescription) uninstallPackages.get(i);
                    int progress = java.lang.Math.round(100/uninstallPackages.size()) * (i+1);
                    panel.setProgressValue(progress);
                    panel.setProgressText(packageData.getPackageName());

                    installer.uninstallPackage(packageData);

                    if ( installData.isAbortedInstallation() ) {
                        break;
                    }
                }

                if ( installData.isAbortedInstallation() ) {
                    LogManager.setCommandsHeaderLine("Uninstallation aborted!");
                    // undoing the uninstallation is not possible
                }

                installer.postUninstallationOngoing();

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

        InstallData data = InstallData.getInstance();

        if ( ! data.isAbortedInstallation() ) {
            if (( data.isTypicalInstallation() ) || ( data.isMaskedCompleteUninstallation() )) {
                InfoDir.removeUninstallationFiles();
            }
        }

        return repeatDialog;
    }

}
