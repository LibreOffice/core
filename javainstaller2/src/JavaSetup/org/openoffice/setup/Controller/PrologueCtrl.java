/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PrologueCtrl.java,v $
 * $Revision: 1.6 $
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
import org.openoffice.setup.Panel.Prologue;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Controller;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.SystemManager;

public class PrologueCtrl extends PanelController {

    private String helpFile;

    public PrologueCtrl() {
        super("Prologue", new Prologue());
        helpFile = "String_Helpfile_Prologue";
    }

    // public void beforeShow() {
    public void duringShow() {
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);

        Thread t = new Thread() {
            public void run() {
                InstallData installData = InstallData.getInstance();
                if ( ! installData.preInstallDone() ) {
                    getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_NEXT);

                    Controller.checkPackagePathExistence(installData);
                    Controller.checkPackageFormat(installData);

                    if (( installData.getOSType().equalsIgnoreCase("SunOS") ) && ( installData.isMultiLingual() )) {
                        Controller.collectSystemLanguages(installData);
                    }

                    PackageDescription packageData = SetupDataProvider.getPackageDescription();
                    Installer installer = InstallerFactory.getInstance();
                    installer.preInstall(packageData);

                    installData.setPreInstallDone(true);

                    if ( SystemManager.logModuleStates() ) {
                        installData.setLogModuleStates(true);
                    }

                    if ( installData.logModuleStates() ) {
                        Dumper.logModuleStates(packageData, "Prologue Dialog");
                    }

                    if (( installData.getOSType().equalsIgnoreCase("SunOS") ) && ( installData.isMultiLingual() )) {
                        ModuleCtrl.checkLanguagesPackages(packageData, installData);

                        if ( installData.logModuleStates() ) {
                            Dumper.logModuleStates(packageData, "Prologue Dialog Language Selection");
                        }
                    }

                    if ( ! installData.isMultiLingual() ) {
                        ModuleCtrl.setHiddenLanguageModuleDefaultSettings(packageData);

                        if ( installData.logModuleStates() ) {
                            Dumper.logModuleStates(packageData, "after setHiddenLanguageModuleDefaultSettings");
                        }
                    }

                    if (( installData.isRootInstallation() ) && ( installData.getOSType().equalsIgnoreCase("SunOS") )) {
                        // Check, if root has write access in /usr and /etc .
                        // In sparse zones with imported directories this is not always the case.
                        if ( Controller.reducedRootWritePrivileges() ) {
                            ModuleCtrl.setIgnoreNonRelocatablePackages(packageData);
                        }

                        if ( installData.logModuleStates() ) {
                            Dumper.logModuleStates(packageData, "after setIgnoreNonRelocatablePackages");
                        }
                    }

                    if ( installData.isRootInstallation() ) {
                        Controller.checkForNewerVersion(installData);
                    }

                    getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_NEXT);
                }
            }
        };
        t.start();
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_PREVIOUS);
        return repeatDialog;
    }

    public String getNext() {
        return new String("AcceptLicense");
    }

    public String getPrevious() {
        return null;
    }

    public final String getHelpFileName() {
        return this.helpFile;
    }

}
