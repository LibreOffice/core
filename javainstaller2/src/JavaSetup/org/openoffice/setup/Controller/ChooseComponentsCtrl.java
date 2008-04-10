/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChooseComponentsCtrl.java,v $
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

import java.util.Vector;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseComponents;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Calculator;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.PackageCollector;

public class ChooseComponentsCtrl extends PanelController {

    private String helpFile;

    public ChooseComponentsCtrl() {
        super("ChooseComponents", new ChooseComponents());
        helpFile = "String_Helpfile_ChooseComponents";
    }

    public String getNext() {
        return new String("InstallationImminent");
    }

    public String getPrevious() {

        InstallData data = InstallData.getInstance();

        if ( data.sameVersionExists() ) {
            return new String("ChooseDirectory");
        } else {
            return new String("ChooseInstallationType");
        }

    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    public void beforeShow() {

        InstallData data = InstallData.getInstance();

        // Setting the package size for node modules, that have hidden children
        // -> Java module has three hidden children and 0 byte size

        if ( ! data.moduleSizeSet() ) {
            PackageDescription packageData = SetupDataProvider.getPackageDescription();
            ModuleCtrl.setModuleSize(packageData);
            data.setModuleSizeSet(true);
        }

        if ( data.sameVersionExists() ) {
            ChooseComponents panel = (ChooseComponents)getPanel();
            String dialogTitle = ResourceManager.getString("String_ChooseComponents1_Maintain");
            panel.setTitleText(dialogTitle);
        }

    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;

        InstallData data = InstallData.getInstance();
        PackageDescription packageData = SetupDataProvider.getPackageDescription();

        if ( nextButtonPressed ) {

            // Check, if at least one visible module was selected for installation
            data.setVisibleModulesChecked(false);
            ModuleCtrl.checkVisibleModulesInstall(packageData, data);

            if ( data.visibleModulesChecked() ) {

                // Check, if at least one application module was selected for installation
                // (not necessary, if an older product is updated or additional modules are
                // added in maintenance mode).

                boolean applicationSelected = false;
                if ( data.olderVersionExists() || data.sameVersionExists() ) {
                    applicationSelected = true;
                } else {
                    data.setApplicationModulesChecked(false);
                    ModuleCtrl.checkApplicationSelection(packageData, data);
                    applicationSelected = data.applicationModulesChecked();
                }

                if ( applicationSelected ) {

                    // Check, if at least one language module was selected for installation
                    // (not necessary, if an older product is updated or additional modules are
                    // added in maintenance mode).

                    boolean languageSelected = false;
                    if ( data.olderVersionExists() || data.sameVersionExists() || ( ! data.isMultiLingual())) {
                        languageSelected = true;
                    } else {
                        data.setLanguageModulesChecked(false);
                        ModuleCtrl.checkLanguageSelection(packageData, data);
                        languageSelected = data.languageModulesChecked();
                    }

                    if ( languageSelected ) {

                        // Set module settings for hidden modules.
                        // Then it is possible to calculate the size of the installed product,
                        // to show a warning and to set the repeatDialog value to true

                        if ( data.logModuleStates() ) {
                            Dumper.logModuleStates(packageData, "ChooseComponentsCtrl: Before setHiddenModuleSettingsInstall");
                        }

                        ModuleCtrl.setHiddenModuleSettingsInstall(packageData);
                        // Dumper.dumpInstallPackages(packageData);

                        if ( data.logModuleStates() ) {
                            Dumper.logModuleStates(packageData, "ChooseComponentsCtrl: After setHiddenModuleSettingsInstall");
                        }

                        // Collecting packages to install
                        Vector installPackages = new Vector();
                        PackageCollector.collectInstallPackages(packageData, installPackages);
                        data.setInstallPackages(installPackages);

                        // Check disc space
                        if ( Calculator.notEnoughDiscSpace(data) ) {
                            repeatDialog = true;
                            System.err.println("Not enough disc space");
                        }
                    } else {   // no language modules selected for installation
                        String message = ResourceManager.getString("String_No_Language_Selected_1") + "\n" +
                                         ResourceManager.getString("String_No_Language_Selected_2");
                        String title = ResourceManager.getString("String_Change_Selection");
                        Informer.showInfoMessage(message, title);
                        repeatDialog = true;
                    }
                } else {
                    String message = ResourceManager.getString("String_No_Application_Selected_1") + "\n" +
                                     ResourceManager.getString("String_No_Application_Selected_2");
                    String title = ResourceManager.getString("String_Change_Selection");
                    Informer.showInfoMessage(message, title);
                    repeatDialog = true;
                }
            } else {  // no modules selected for installation
                String message = ResourceManager.getString("String_No_Components_Selected_1") + "\n" +
                                 ResourceManager.getString("String_No_Components_Selected_2");
                String title = ResourceManager.getString("String_Nothing_To_Install");
                Informer.showInfoMessage(message, title);
                repeatDialog = true;
            }
        } else {  // the back button was pressed
            // Saving typical selection state values (always if back button is pressed!).
            // System.err.println("Saving custom selection states");
            ModuleCtrl.saveCustomSelectionStates(packageData);
            data.setCustomSelectionStateSaved(true);
        }

        return repeatDialog;
    }

}
