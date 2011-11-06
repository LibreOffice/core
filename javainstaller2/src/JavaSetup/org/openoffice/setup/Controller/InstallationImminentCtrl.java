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
import org.openoffice.setup.Panel.InstallationImminent;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.InfoCtrl;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.ModuleCtrl;
import java.util.Vector;

public class InstallationImminentCtrl extends PanelController {

    private String helpFile;
    private String htmlInfoText = "";

    public InstallationImminentCtrl() {
        super("InstallationImminent", new InstallationImminent());
        helpFile = "String_Helpfile_InstallationImminent";
    }

    public String getNext() {
        return new String("InstallationOngoing");
    }

    public String getPrevious() {

        InstallData data = InstallData.getInstance();

        if ( data.isRootInstallation() ) {
            if ( data.olderVersionExists() ) {
                if ( data.hideEula() ) {
                    return new String("Prologue");
                } else {
                    return new String("AcceptLicense");
                }
            } else if ( data.sameVersionExists() ) {
                return new String("ChooseComponents");
            } else {
                if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
                    return new String("ChooseComponents");
                } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
                    return new String("ChooseInstallationType");
                } else {
                    System.err.println("Error: Unknown installation type!" );
                    return new String("Error");
                }
            }
        } else {
            if ( data.olderVersionExists() ) {
                return new String("ChooseDirectory");
            } else if ( data.sameVersionExists() ) {
                return new String("ChooseComponents");
            } else {
                if ( data.getInstallationType().equals(data.getCustomActionCommand()) ) {
                    return new String("ChooseComponents");
                } else if ( data.getInstallationType().equals(data.getTypicalActionCommand()) ) {
                    return new String("ChooseInstallationType");
                } else {
                    System.err.println("Error: Unknown installation type!" );
                    return new String("Error");
                }
            }
        }
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {
        String StringInstall = ResourceManager.getString("String_Install");
        getSetupFrame().setButtonText(StringInstall, getSetupFrame().BUTTON_NEXT);

        ProductDescription productData = SetupDataProvider.getProductDescription();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();
        // Dumper.dumpPackageSettings(packageData);
        htmlInfoText = InfoCtrl.setHtmlFrame("header", htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(productData, htmlInfoText);
        htmlInfoText = InfoCtrl.setReadyToInstallInfoText(packageData, htmlInfoText);
        htmlInfoText = InfoCtrl.setHtmlFrame("end", htmlInfoText);

        InstallationImminent panel = (InstallationImminent)getPanel();
        panel.setInfoText(htmlInfoText);
        panel.setCaretPosition();

        // Update mode
        InstallData data = InstallData.getInstance();
        if ( data.olderVersionExists() ) {
            String dialogTitle = ResourceManager.getString("String_InstallationImminent1_Update");
            panel.setTitleText(dialogTitle);
        }
    }

    public void duringShow() {
        InstallationImminent panel = (InstallationImminent)getPanel();
        panel.setTabOrder();
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        if ( nextButtonPressed ) {

            InstallData data = InstallData.getInstance();

            // determining the packagePath
            if ( data.getPackagePath() != null ) {
                String log = "<b>Packages path:</b> " + data.getPackagePath() + "<br>";
                LogManager.addLogfileComment(log);
            } else {
                String message = ResourceManager.getString("String_InstallationOngoing_PackagePath_Not_Found");
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                String log = "<b>Error: No path for packages exists!</b><br>";
                LogManager.addLogfileComment(log);
                repeatDialog = true;
            }
        }

        return repeatDialog;
    }
}
