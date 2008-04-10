/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InstallationImminentCtrl.java,v $
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
