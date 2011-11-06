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
import org.openoffice.setup.Panel.installationCompleted;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InfoCtrl;

public class InstallationCompletedCtrl extends PanelController {

    private String helpFile;
    private String mDialogText;
    private String htmlInfoText;

    public InstallationCompletedCtrl() {
        super("InstallationCompleted", new installationCompleted());
        helpFile = "String_Helpfile_InstallationCompleted";
    }

    public void beforeShow() {
        InstallData installData = InstallData.getInstance();
        ProductDescription productData = SetupDataProvider.getProductDescription();

        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_CANCEL);
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_HELP);
        getSetupFrame().removeButtonIcon(getSetupFrame().BUTTON_NEXT);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);

        installationCompleted panel = (installationCompleted)getPanel();
        panel.setDetailsButtonActionCommand(getSetupFrame().ACTION_DETAILS);
        panel.addDetailsButtonActionListener(getSetupFrame().getSetupActionListener());

        if ( installData.isAbortedInstallation() ) {
            String titleText = ResourceManager.getString("String_InstallationCompleted1_Abort");
            panel.setTitleText(titleText);
            String dialogText = ResourceManager.getString("String_InstallationCompleted2_Abort");
            panel.setDialogText(dialogText);
        } else if ( installData.isErrorInstallation() ) {
            String titleText = ResourceManager.getString("String_InstallationCompleted1_Error");
            panel.setTitleText(titleText);
            String dialogText = ResourceManager.getString("String_InstallationCompleted2_Error");
            panel.setDialogText(dialogText);
        }

        htmlInfoText = InfoCtrl.setHtmlFrame("header", htmlInfoText);
        htmlInfoText = InfoCtrl.setInstallLogInfoText(productData, htmlInfoText);
        htmlInfoText = InfoCtrl.setHtmlFrame("end", htmlInfoText);
    }

    public void duringShow() {
        Thread t = new Thread() {
            public void run() {
                PackageDescription packageData = SetupDataProvider.getPackageDescription();
                Installer installer = InstallerFactory.getInstance();
                installer.postInstall(packageData);
            }
        };

        t.start();
    }

    public String getNext() {
        return null;
    }

    public String getPrevious() {
        return null;
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public String getDialogText() {
        return htmlInfoText;
    }

}
