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
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationImminent;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InfoCtrl;
import org.openoffice.setup.Util.ModuleCtrl;

public class UninstallationImminentCtrl extends PanelController {

    private String helpFile;
    private String htmlInfoText = "";

    public UninstallationImminentCtrl() {
        super("UninstallationImminent", new UninstallationImminent());
        helpFile = "String_Helpfile_UninstallationImminent";
    }

    public String getNext() {
        return new String("UninstallationOngoing");
    }

    public String getPrevious() {

        InstallData data = InstallData.getInstance();

        if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
            return new String("ChooseUninstallationComponents");
        } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
            return new String("ChooseUninstallationType");
        } else {
            System.err.println("Error: Unknown uninstallation type!" );
            return new String("Error");
        }
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        String StringInstall = ResourceManager.getString("String_Uninstall");
        getSetupFrame().setButtonText(StringInstall, getSetupFrame().BUTTON_NEXT);

        ProductDescription productData = SetupDataProvider.getProductDescription();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        htmlInfoText = InfoCtrl.setHtmlFrame("header", htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(productData, htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(packageData, htmlInfoText);
        htmlInfoText = InfoCtrl.setHtmlFrame("end", htmlInfoText);

        UninstallationImminent panel = (UninstallationImminent)getPanel();
        panel.setInfoText(htmlInfoText);
        panel.setCaretPosition();

        // System.err.println("\nUninstallation module state dump 3:");
        // Dumper.dumpModuleStates(packageData);
    }

    public void duringShow() {
        UninstallationImminent panel = (UninstallationImminent)getPanel();
        panel.setTabOrder();
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        if ( nextButtonPressed ) {
            PackageDescription packageData = SetupDataProvider.getPackageDescription();
            ModuleCtrl.setHiddenModuleSettingsUninstall(packageData);
            // Dumper.dumpUninstallPackages(packageData);

            // System.err.println("\nUninstallation module state dump 4:");
            // Dumper.dumpModuleStates(packageData);
        }

        return repeatDialog;
    }

}
