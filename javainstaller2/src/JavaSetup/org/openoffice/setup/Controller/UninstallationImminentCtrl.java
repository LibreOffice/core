/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UninstallationImminentCtrl.java,v $
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
