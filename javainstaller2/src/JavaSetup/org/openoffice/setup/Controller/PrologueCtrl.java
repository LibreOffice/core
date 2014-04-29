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

                        // int count = installData.getPreselectedLanguages();
                        // System.err.println("Number of preselected language packages: " + count);

                        if ( installData.getPreselectedLanguages() == 0 ) {
                            // Something mysterious happened. Setting all languages again.
                            ModuleCtrl.setLanguagesPackages(packageData);
                        }

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

                        // Setting installation directory!
                        String dir = "/";
                        installData.setInstallDir(dir);
                        installData.setInstallDefaultDir(installData.getDefaultDir());

                        Controller.checkForNewerVersion(installData);

                        // Check Write privileges in installation directory (installData.getInstallDefaultDir())
                        // If the directory exists, is has to be tested, whether the user has write access
                        dir = installData.getInstallDefaultDir();

                        if ( SystemManager.exists_directory(dir) ) {
                            if ( ! Controller.createdSubDirectory(dir) ) {
                                System.err.println("ERROR: No write privileges inside directory: " + dir);
                                System.exit(1);
                            }
                        }

                        // If the directory does not exist, is has to be tested, whether the user can create it
                        if ( ! SystemManager.exists_directory(dir)) {
                            if ( ! Controller.createdDirectory(dir) ) {
                                System.err.println("ERROR: No privileges to create directory: " + dir);
                                System.exit(1);
                            }
                        }

                        // Setting macro
                        SetupDataProvider.setNewMacro("DIR", dir); // important for string replacement

                        // Calculate available disc space
                        int discSpace = SystemManager.calculateDiscSpace(dir);
                        installData.setAvailableDiscSpace(discSpace);

                        if ( ! installData.databaseAnalyzed()) {
                            ModuleCtrl.defaultDatabaseAnalysis(installData);
                            installData.setDatabaseAnalyzed(true);
                        }
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
        InstallData data = InstallData.getInstance();

        if ( data.hideEula() ) {
            if ( data.isRootInstallation() ) {
                if ( data.olderVersionExists() ) {
                    return new String("InstallationImminent");
                } else if ( data.sameVersionExists() ) {
                    return new String("ChooseComponents");
                } else {
                    return new String("ChooseInstallationType");
                }
            } else {
                return new String("ChooseDirectory");
            }
        } else {
            return new String("AcceptLicense");
        }
    }

    public String getPrevious() {
        return null;
    }

    public final String getHelpFileName() {
        return this.helpFile;
    }

}
