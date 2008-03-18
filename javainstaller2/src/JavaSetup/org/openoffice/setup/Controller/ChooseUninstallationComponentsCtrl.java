/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChooseUninstallationComponentsCtrl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:14:29 $
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
import org.openoffice.setup.Panel.ChooseUninstallationComponents;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.ModuleCtrl;

public class ChooseUninstallationComponentsCtrl extends PanelController {

    private String helpFile;

    public ChooseUninstallationComponentsCtrl() {
        super("ChooseUninstallationComponents", new ChooseUninstallationComponents());
        helpFile = "String_Helpfile_ChooseUninstallationComponents";
    }

    public String getNext() {
        return new String("UninstallationImminent");
    }

    public String getPrevious() {
        return new String("ChooseUninstallationType");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        InstallData data = InstallData.getInstance();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        if ( nextButtonPressed ) {

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "Choose UninstallationComponents: Before checkVisibleModulesUninstall");
            }

            // Check, if at least one visible module is selected for uninstallation
            data.setVisibleModulesChecked(false);
            ModuleCtrl.checkVisibleModulesUninstall(packageData, data);

            if ( ! data.visibleModulesChecked() ) {
                String message = ResourceManager.getString("String_No_Uninstallcomponents_Selected_1") + "\n" +
                                 ResourceManager.getString("String_No_Uninstallcomponents_Selected_2");
                String title = ResourceManager.getString("String_Nothing_To_Uninstall");
                Informer.showInfoMessage(message, title);
                repeatDialog = true;
            } else {
                // Check, if all visible modules are selected for uninstallation.
                // Then this shall be handled as complete uninstallation
                // -> The responsible value is InstallData.isMaskedCompleteUninstallation
                data.setMaskedCompleteUninstallation(true);
                ModuleCtrl.checkMaskedCompleteUninstallation(packageData, data);

                // If this is not a complete uninstallation, at least one language
                // module or one application module has to be installed.

                if ( ! data.isMaskedCompleteUninstallation() ) {

                    data.setApplicationModulesChecked(false);
                    ModuleCtrl.checkApplicationModulesUninstall(packageData, data);

                    if ( ! data.applicationModulesChecked() ) {

                        String message = ResourceManager.getString("String_All_Applicationcomponents_Selected_1") + "\n" +
                                         ResourceManager.getString("String_All_Applicationcomponents_Selected_2");
                        String title = ResourceManager.getString("String_Change_Selection");
                        Informer.showInfoMessage(message, title);
                        repeatDialog = true;
                    } else {
                        if ( data.isMultiLingual()) {
                            data.setLanguageModulesChecked(false);
                            ModuleCtrl.checkLanguageModulesUninstall(packageData, data);

                            if ( ! data.languageModulesChecked() ) {

                                String message = ResourceManager.getString("String_All_Languagecomponents_Selected_1") + "\n" +
                                                 ResourceManager.getString("String_All_Languagecomponents_Selected_2");
                                String title = ResourceManager.getString("String_Change_Selection");
                                Informer.showInfoMessage(message, title);
                                repeatDialog = true;
                            }
                        }
                    }
                }
            }
        } else {  // the back button was pressed
            // Saving typical selection state values (always if back button is pressed!).
            ModuleCtrl.saveCustomSelectionStates(packageData);
            data.setCustomSelectionStateSaved(true);
        }

        return repeatDialog;
    }

}
