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

        Vector sortedPackages = new Vector();
        PackageCollector.sortPackages(uninstallPackages, sortedPackages, "uninstall");
        data.setInstallPackages(sortedPackages);

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
                    int progress = java.lang.Math.round((100*(i+1))/uninstallPackages.size());
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
