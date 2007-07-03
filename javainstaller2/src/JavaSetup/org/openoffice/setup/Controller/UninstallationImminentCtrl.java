/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UninstallationImminentCtrl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:52:30 $
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
