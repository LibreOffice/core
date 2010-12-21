/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.setup.Util;

import java.io.File;
import java.util.Enumeration;
import java.util.Vector;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.Panel.ChooseDirectory;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Informer;

public class ModuleCtrl {

    private ModuleCtrl() {
    }

    static public void setModuleSize(PackageDescription packageData) {
        // Setting the package size for visible node modules, that have hidden children
        // -> Java module has three hidden children and 0 byte size

        if (( ! packageData.isLeaf() ) && ( ! packageData.isHidden() )) {
            boolean setNewSize = false;
            int size = packageData.getSize();

            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                // if (( child.isHidden() ) && ( child.getSelectionState() == PackageDescription.DONT_KNOW )) {
                if ( child.isHidden() ) {
                    setNewSize = true;
                    size = size + child.getSize();
                }
            }

            if ( setNewSize ) {
                packageData.setSize(size);
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setModuleSize(child);
        }

    }

    static public void setDefaultModuleSettings(PackageDescription data) {
        // Setting default module settings for modules, that are not hidden
        // Hidden modules do not get a defined state now
        boolean isInstalled = false;
        InstallData installdata = InstallData.getInstance();
        boolean isUninstall = installdata.isUninstallationMode();

        if (isUninstall) {
            isInstalled = true;
        }

        if (isUninstall) {
            if (isInstalled) {
                data.setSelectionState(PackageDescription.REMOVE);
            } else {
                data.setSelectionState(PackageDescription.IGNORE);
                System.err.println("NEVER");
            }
        } else {
            if (isInstalled) {
                data.setSelectionState(PackageDescription.IGNORE);
                System.err.println("NEVER");
            } else if (data.isDefault()) {
                data.setSelectionState(PackageDescription.INSTALL);
            } else if ( ! data.isDefault()) {
                data.setSelectionState(PackageDescription.DONT_INSTALL);
            } else {
                data.setSelectionState(PackageDescription.DONT_INSTALL);
            }
        }
    }

    static public void setParentDefaultModuleSettings(PackageDescription packageData) {
        // Setting the module states of parent modules.
        // Called after ChooseDirectoryCtrl.java, because
        // the database has to be known. In user installation it is important,
        // that the installation directory is known, to find the database.
        // Called during uninstallation in UninstallationPrologueCtrl.java

        // Iteration before setting the module states. Because of this, all children
        // get their final setting before the parent.

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setParentDefaultModuleSettings(child);
        }

        if ( ! packageData.isLeaf() ) {
            // System.err.println("setParentDefaultModuleSettings: " + packageData.getName());
            int state = packageData.getSelectionState();
            InstallData installdata = InstallData.getInstance();
            boolean allChildrenIgnored = true;
            boolean atLeastOneInstalled = false;
            boolean allChildrenHidden = true;

            // System.err.println("    STATE before iterating over children: " + state);

            for (Enumeration e = packageData.children(); e.hasMoreElements();) {
                PackageDescription child = (PackageDescription) e.nextElement();
                int childState = child.getSelectionState();

                // System.err.println("    Child: " + child.getName() + " : " + childState);

                if ( childState != PackageDescription.IGNORE) {
                    allChildrenIgnored = false;
                }

                if (( childState == PackageDescription.INSTALL) || ( childState == PackageDescription.INSTALL_SOME)) {
                    atLeastOneInstalled = true;
                }

                if ( ! child.isHidden() ) {
                    allChildrenHidden = false;
                }

                if ((state == PackageDescription.DONT_KNOW) || (state == PackageDescription.IGNORE)) {
                    state = childState;
                // } else if ((state != childState) && (childState != PackageDescription.IGNORE)) {
                } else if ((state != childState) && (childState != PackageDescription.IGNORE) && (childState != PackageDescription.DONT_KNOW)) {
                    if ( installdata.isUninstallationMode() ) {
                        state = PackageDescription.REMOVE_SOME;
                    } else {
                        state = PackageDescription.INSTALL_SOME;
                    }
                }

                // System.err.println("    NEW state after child: " + state);
            }

            if ( allChildrenIgnored ) {
                state = PackageDescription.IGNORE;
            }

            if ( installdata.isInstallationMode() ) {
                if (( state == PackageDescription.INSTALL_SOME ) && ( ! atLeastOneInstalled )) {
                    state = PackageDescription.DONT_INSTALL;
                }
            }

            if ( allChildrenHidden ) {
                packageData.setAllChildrenHidden(true);
                // System.err.println("Setting allChildrenHidden for module " + packageData.getName() );
            }

            // If older version exist, only modules without packages shall be updated,
            // because all packages are already determined by querying the database.
            if ( installdata.olderVersionExists() ) {
                if ( packageData.getPackageName().equals("") ) {
                    packageData.setSelectionState(state);
                }
            } else {
                packageData.setSelectionState(state);
            }
        }
    }

    static public void setHiddenModuleSettingsInstall(PackageDescription packageData) {
        // update selection states for hidden modules during installation
        if (( packageData.isHidden() ) && ( packageData.getSelectionState() != packageData.IGNORE )) {
            PackageDescription parent = (PackageDescription)packageData.getParent();
            if ( parent != null ) {
                packageData.setSelectionState(parent.getSelectionState());
                // hidden modules at root module always have to be installed, if they are not already installed
                if ( parent.getName() == "" ) {
                    packageData.setSelectionState(packageData.INSTALL);
                    // System.err.println("Setting 1 INSTALL flag to: " + packageData.getName());
                }
            }

            // INSTALL_SOME is not valid for leaves
            if (( packageData.getSelectionState() == packageData.INSTALL_SOME ) && ( packageData.isLeaf() )) {
                packageData.setSelectionState(packageData.INSTALL);
                // System.err.println("Setting 2 INSTALL flag to: " + packageData.getName());
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setHiddenModuleSettingsInstall(child);
        }
    }

    static public void setHiddenModuleSettingsUninstall(PackageDescription packageData) {
        InstallData data = InstallData.getInstance();
        // update selection states for hidden modules during uninstallation
        if (( packageData.isHidden() ) && ( packageData.getSelectionState() != packageData.IGNORE )) {
            // System.err.println("Package name: " + packageData.getName());
            // System.err.println("Selection: " + packageData.getSelectionState());

            PackageDescription parent = (PackageDescription)packageData.getParent();
            if ( parent != null ) {
                packageData.setSelectionState(parent.getSelectionState());
                // Hidden modules at root module have to be uninstalled at complete uninstallation
                // In Uninstallation the complete is the typical installation type
                if (( parent.getName() == "" ) && ( data.isTypicalInstallation() ))  {
                    packageData.setSelectionState(packageData.REMOVE);
                }
                // Hidden modules at root module must not be uninstalled at custom uninstallation
                // But if all visible modules are selected for uninstallation, this shall be handled
                // as complete uninstallation.
                if ( ! data.isMaskedCompleteUninstallation() )
                {
                    if (( parent.getName() == "" ) && ( data.isCustomInstallation() ))  {
                        packageData.setSelectionState(packageData.IGNORE);
                    }
                }
            }

            // REMOVE_SOME is not valid for leaves
            // if ( data.isTypicalInstallation() ) {
            if (( packageData.getSelectionState() == packageData.REMOVE_SOME ) && ( packageData.isLeaf() )) {
                packageData.setSelectionState(packageData.REMOVE);
            }
            // }

        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setHiddenModuleSettingsUninstall(child);
        }
    }

    static private boolean checkRequiredCoreModule(PackageDescription packageData) {

        // This function uses a similar mechanism to determine
        // core modules as function "setHiddenModuleSettingsInstall"
        // -> only hidden parents, until there is a module without name (getName)
        // Only searching until grandpa.

        boolean requiredCoreModule = false;

        // if packageData.getSelectionState() DONT_KNOW  && parent auch DONT_KNOW
        if (( packageData.isHidden() ) &&
                ( packageData.getSelectionState() != packageData.IGNORE ) &&
                ( packageData.getPackageName() != null )) {
                //( packageData.isLeaf() )) {
            PackageDescription parent = (PackageDescription)packageData.getParent();
            if ( parent != null ) {
                if (( parent.getName().equals("") ) || ( parent.getName() == null )) {
                    requiredCoreModule = true;
                } else {
                    if ( parent.isHidden() ) {
                        PackageDescription grandpa = (PackageDescription)parent.getParent();
                        if ( grandpa != null ) {
                            if (( grandpa.getName().equals("") ) || ( grandpa.getName() == null )) {
                                requiredCoreModule = true;
                            }
                        }
                    }
                }
            }
        }

        return requiredCoreModule;
    }

    static public void setDatabaseSettings(PackageDescription packageData, InstallData installData, Installer installer) {
        // Analyzing the system database and setting the module states.
        // Called during installation in ChooseInstallationTypeCtrl.java, because
        // the database has to be known. In user installation it is important,
        // the the installation directory is known, to find the database.
        // Called during uninstallation in UninstallationPrologueCtrl.java

        boolean isUninstall = installData.isUninstallationMode();
        boolean isInstalled = installer.isPackageInstalled(packageData, installData);

        if (isUninstall) {
            if (isInstalled) {
                packageData.setSelectionState(PackageDescription.REMOVE);
                // The following is no longer required !? (IS, 06/05/08)
                // PackageDescription parent = packageData.getParent();
                // if ( parent != null ) {
                //     if ( parent.getSelectionState() != PackageDescription.REMOVE ) {
                //         parent.setSelectionState(PackageDescription.REMOVE);
                //         System.err.println("Setting remove to " +  parent.getName());
                //     }
                // }
            } else {
                // Attention: Setting all nodes to ignore! If a children gets REMOVE,
                // then REMOVE is also set to the parent. Setting REMOVE happens after
                // setting IGNORE, because children are evaluated after the parents.
                // The default for uninstallation is set in setDefaultModuleSettings to REMOVE.
                packageData.setSelectionState(PackageDescription.IGNORE);
            }
        } else {
            boolean goodDepends = true;
            if ( installData.getOSType().equalsIgnoreCase("SunOS") ) {
                if (( installData.isRootInstallation() ) && ( packageData.getCheckSolaris() != null ) && ( ! packageData.getCheckSolaris().equals("") )) {
                    // the package has to be installed. Creating a new package with only packagename
                    if ( ! installer.isPackageNameInstalled(packageData.getCheckSolaris(), installData) ) {
                        goodDepends = false;
                    }
                }
            }

            if ( ! goodDepends ) {
                // The package dependencies are not valid -> ignoring package.
                packageData.setSelectionState(PackageDescription.IGNORE);
                // too late to hide the module
                // packageData.setIsHidden(true);
            }
            else {
                if ( isInstalled ) {
                    // Maybe a required core module is installed in an older version from another product
                    boolean isRequiredCoreModule = checkRequiredCoreModule(packageData);
                    if (( packageData.isJavaPackage() ) || ( isRequiredCoreModule )) {   // only selected checks, because of performance reasons
                        boolean installedPackageIsOlder = installer.isInstalledPackageOlder(packageData, installData);
                        if ( ! installedPackageIsOlder ) {
                            // The package is already installed in the same or in a newer version
                            packageData.setSelectionState(PackageDescription.IGNORE);
                        } else {
                            // This is also something like migrating feature states
                            packageData.setSelectionState(PackageDescription.INSTALL);
                            LogManager.addLogfileComment("<b>Adding required older installed package:</b> " + packageData.getPackageName() + "</br>");
                        }
                    } else {  // no version check done -> so what is a good setting for already installed packages?
                        if ( installData.olderVersionExists() ) {  // should never be the case in this function
                            packageData.setSelectionState(PackageDescription.INSTALL);
                        } else {
                            packageData.setSelectionState(PackageDescription.IGNORE);
                        }
                    }
                }
                else {
                    // Special handling for core modules, which are required, but not installed.
                    // This can be deinstalled by hand for example.
                    boolean isRequiredCoreModule = checkRequiredCoreModule(packageData);
                    if ( isRequiredCoreModule ) {
                        if ( packageData.getSelectionState() != PackageDescription.INSTALL ) {
                            packageData.setSelectionState(PackageDescription.INSTALL);
                            LogManager.addLogfileComment("<b>Adding required package:</b> " + packageData.getPackageName() + "</br>");
                        }
                        // This package has to exist!
                        if ( ! packageExists(packageData, installData) ) {

                            String packagePath = installData.getPackagePath();
                            if (( packageData.getPkgSubdir() != null ) && ( ! packageData.getPkgSubdir().equals("") )) {
                                File completePackageFile = new File(packagePath, packageData.getPkgSubdir());
                                packagePath = completePackageFile.getPath();
                            }
                            String packageName = packageData.getPackageName();
                            File packageFile = new File(packagePath, packageName);

                            String log = "<b>Error: Missing required package " + packageFile.getPath() + "</b><br>";
                            System.err.println(log);
                            String message = ResourceManager.getString("String_File_Not_Found") + ": " + packageFile.getPath();
                            String title = ResourceManager.getString("String_Error");
                            Informer.showErrorMessage(message, title);
                            System.exit(1);
                        }
                    }
                }
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setDatabaseSettings(child, installData, installer);
        }
    }

    static public void setShowInUserInstallFlags(PackageDescription packageData) {

        // This function is not needed during deinstallation, because a
        // module that could not be selected during installation, is always
        // not installed during deinstallation and therefore gets "IGNORE"
        // in function setDatabaseSettings

        if ( ! packageData.showInUserInstall() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
            // too late to hide the module
            // packageData.setIsHidden(true);
            // packageData.setAllChildrenHidden(true);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setShowInUserInstallFlags(child);
        }
    }

    static public void setForcedUpdateProductSettings(PackageDescription packageData) {

        if ( packageData.forceIntoUpdate() ) {
            packageData.setSelectionState(PackageDescription.INSTALL);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setForcedUpdateProductSettings(child);
        }
    }

    static public void setShowInUserInstallOnlyFlags(PackageDescription packageData) {

        // This function is not needed during deinstallation, because a
        // module that could not be selected during installation, is always
        // not installed during deinstallation and therefore gets "IGNORE"
        // in function setDatabaseSettings

        if ( packageData.showInUserInstallOnly() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
            // too late to hide the module
            // packageData.setIsHidden(true);
            // packageData.setAllChildrenHidden(true);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setShowInUserInstallOnlyFlags(child);
        }
    }

    static public void setIgnoreNonRelocatablePackages(PackageDescription packageData) {
        if ( ! packageData.isRelocatable() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
            System.err.println("Ignoring package " + packageData.getName() + " " + packageData.getPackageName());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setIgnoreNonRelocatablePackages(child);
        }
    }

    static public void setHiddenLanguageModuleDefaultSettings(PackageDescription packageData) {

        // This function is needed during installation for the language modules,
        // if there is only one language in the installation set. In this case the language
        // modules are hidden (no selection possible) and therefore get no value in
        // setDefaultModuleSettings(). This default value is set now.

        if ( packageData.showMultiLingualOnly() ) {
            packageData.setSelectionState(PackageDescription.INSTALL);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setHiddenLanguageModuleDefaultSettings(child);
        }
    }

    static private boolean packageExists(PackageDescription packageData, InstallData installData) {
        boolean fileExists = false;
        String packagePath = installData.getPackagePath();

        if (( packageData.getPkgSubdir() != null ) && ( ! packageData.getPkgSubdir().equals("") )) {
            File completePackageFile = new File(packagePath, packageData.getPkgSubdir());
            packagePath = completePackageFile.getPath();
        }

        String packageName = packageData.getPackageName();
        File packageFile = new File(packagePath, packageName);

        if ( packageFile.exists() ) {
            fileExists = true;
        }

        return fileExists;
    }

    static public void disableNonExistingPackages(PackageDescription packageData, InstallData installData) {
        if ((( packageData.getPackageName() == null ) || ( packageData.getPackageName().equals("") ))
               && packageData.isLeaf() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
        } else if ( ! packageExists(packageData, installData) ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            disableNonExistingPackages(child, installData);
        }
    }

    static public void setDontUninstallFlags(PackageDescription packageData) {
        if ( packageData.dontUninstall() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setDontUninstallFlags(child);
        }
    }

    static public void setDontUninstallUserInstallOnylFlags(PackageDescription packageData) {
        if ( packageData.showInUserInstallOnly() ) {
            packageData.setSelectionState(PackageDescription.IGNORE);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setDontUninstallUserInstallOnylFlags(child);
        }
    }

    static public void checkVisibleModulesInstall(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        if (( ! packageData.isHidden() ) && ( packageData.getSelectionState() == packageData.INSTALL )) {
            setToTrue = true;
            data.setVisibleModulesChecked(true);
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkVisibleModulesInstall(child, data);
            }
        }
    }

    static public void checkApplicationSelection(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        if (( packageData.isApplicationPackage() ) &&
            ( ! packageData.isHidden() ) &&
            ( packageData.getSelectionState() == packageData.INSTALL )) {
                setToTrue = true;
                data.setApplicationModulesChecked(true);
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkApplicationSelection(child, data);
            }
        }
    }

    static public void checkLanguageSelection(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        if (( packageData.showMultiLingualOnly() ) &&
            ( ! packageData.isHidden() ) &&
            ( packageData.getSelectionState() == packageData.INSTALL )) {
                setToTrue = true;
                data.setLanguageModulesChecked(true);
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkLanguageSelection(child, data);
            }
        }
    }

    static public void checkVisibleModulesUninstall(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        if (( ! packageData.isHidden() ) && ( packageData.getSelectionState() == packageData.REMOVE )) {
            // ignoring the top level module, that has the state REMOVE (but no name)
            if (( packageData.getName() != null ) && ( ! packageData.getName().equals("") )) {
                setToTrue = true;
                data.setVisibleModulesChecked(true);
            }
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkVisibleModulesUninstall(child, data);
            }
        }
    }

    static public void checkApplicationModulesUninstall(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        // At least one language module should not be uninstalled. Then this function returns true.
        // An exeption is the complete uninstallation or the masked complete uninstallation.

        if (( packageData.isApplicationPackage() ) &&
            ( ! packageData.isHidden() ) &&
            ( packageData.getSelectionState() != packageData.IGNORE ) &&
            ( packageData.getSelectionState() != packageData.REMOVE )) {
                setToTrue = true;
                data.setApplicationModulesChecked(true);
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkApplicationModulesUninstall(child, data);
            }
        }
    }

    static public void checkLanguageModulesUninstall(PackageDescription packageData, InstallData data) {
        boolean setToTrue = false;

        // At least one language module should not be uninstalled. Then this function returns true.
        // An exeption is the complete uninstallation or the masked complete uninstallation.

        if (( packageData.showMultiLingualOnly() ) &&
            ( ! packageData.isHidden() ) &&
            ( packageData.getSelectionState() != packageData.IGNORE ) &&
            ( packageData.getSelectionState() != packageData.REMOVE )) {
                setToTrue = true;
                data.setLanguageModulesChecked(true);
        }

        if ( ! setToTrue ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkLanguageModulesUninstall(child, data);
            }
        }
    }

    static public void checkMaskedCompleteUninstallation(PackageDescription packageData, InstallData data) {
        boolean setToFalse = false;

        // If there is at least one visible module, that is not selected for removal
        // this is no masked complete uninstallation

        if (( ! packageData.isHidden() )
                 && ( packageData.getSelectionState() != packageData.REMOVE )
                 && ( packageData.getSelectionState() != packageData.IGNORE )) {
            // ignoring the top level module, that has no name
            if (( packageData.getName() != null ) && ( ! packageData.getName().equals("") )) {
                setToFalse = true;
                data.setMaskedCompleteUninstallation(false);
                // System.err.println("This is no masked complete uninstallation!");
                // System.err.println("Caused by: " + packageData.getName() + " with " + packageData.getSelectionState());
            }
        }

        if ( ! setToFalse ) {
            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                checkMaskedCompleteUninstallation(child, data);
            }
        }
    }

    static public void saveTypicalSelectionStates(PackageDescription packageData) {
        packageData.setTypicalSelectionState(packageData.getSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            saveTypicalSelectionStates(child);
        }
    }

    static public void saveCustomSelectionStates(PackageDescription packageData) {
        packageData.setCustomSelectionState(packageData.getSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            saveCustomSelectionStates(child);
        }
    }

    static public void saveStartSelectionStates(PackageDescription packageData) {
        packageData.setStartSelectionState(packageData.getSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            saveStartSelectionStates(child);
        }
    }

    static public void restoreTypicalSelectionStates(PackageDescription packageData) {
        packageData.setSelectionState(packageData.getTypicalSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            restoreTypicalSelectionStates(child);
        }
    }

    static public void restoreCustomSelectionStates(PackageDescription packageData) {
        packageData.setSelectionState(packageData.getCustomSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            restoreCustomSelectionStates(child);
        }
    }

    static public void restoreStartSelectionStates(PackageDescription packageData) {
        packageData.setSelectionState(packageData.getStartSelectionState());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            restoreStartSelectionStates(child);
        }
    }

    static public void setUpdateOlderProductSettings(PackageDescription packageData, InstallData data, Installer installer) {
        if (( packageData.getPackageName() != null ) && ( ! packageData.getPackageName().equals(""))) {
            if ( installer.isPackageInstalled(packageData, data) ) {
                packageData.setSelectionState(PackageDescription.INSTALL);

                // Special handling for jre package, because this is not necessarily older, if an older product is updated.
                if ( packageData.isJavaPackage() ) {
                    if ( ! installer.isInstalledPackageOlder(packageData, data) ) {
                        packageData.setSelectionState(PackageDescription.DONT_INSTALL);
                    }
                }
            } else {
                packageData.setSelectionState(PackageDescription.DONT_INSTALL);
                // Special handling for Major Upgrade
                if ( data.isMajorUpgrade() ) {
                    String basis = "ooobasis3";
                    if ( data.getOSType().equalsIgnoreCase("Linux") ) { basis = basis + "."; }
                    String search = basis + data.getProductMinor();
                    String replacestring = basis + data.getInstalledProductMinor();
                    int pos = packageData.getPackageName().indexOf(search);
                    if ( pos > -1  ) {
                        // Check if this package is installed with a lower product minor
                        // Creating new package for removal, very simple PackageDescription
                        PackageDescription localPackage = new PackageDescription();
                        localPackage.setUninstallCanFail(true);
                        localPackage.setIsRelocatable(packageData.isRelocatable());
                        String localName = packageData.getPackageName();
                        localName = localName.replace(search, replacestring);
                        localPackage.setPackageName(localName);

                        if ( ( packageData.getPkgRealName() != null ) && ( ! packageData.getPkgRealName().equals("") )) {
                            localName = packageData.getPkgRealName();
                            localName = localName.replace(search, replacestring);
                            localPackage.setPkgRealName(localName);
                        }

                        if (( packageData.getName() != null ) && ( ! packageData.getName().equals("") )) {
                            localName = packageData.getName();
                            localName = localName.replace(search, replacestring);
                            localPackage.setName(localName);
                        }

                        // saving also the order, needed for order of uninstallation
                        localPackage.setOrder(packageData.getOrder());

                        // If the old package is installed, the new package can be installed, too,
                        // and the old package can be marked for removal (with dependency check).
                        if ( installer.isPackageInstalled(localPackage, data) ) {
                            packageData.setSelectionState(PackageDescription.INSTALL);

                            // Collecting all installed older packages for uninstallation
                            Vector oldPackages = data.getOldPackages();
                            oldPackages.add(localPackage);
                            data.setOldPackages(oldPackages);
                        }
                    }
                }
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setUpdateOlderProductSettings(child, data, installer);
        }
    }

    static public void checkLanguagesPackages(PackageDescription packageData, InstallData installData) {
        if (( packageData.getPkgLanguage() != null ) && ( ! packageData.getPkgLanguage().equals(""))) {
            // This is a package with a specific language.
            // pkgLanguage can be a comma separated list, for example "ja,ja_JP.PCK,ja_JP.UTF-8"
            String allLang = packageData.getPkgLanguage();
            String[] allLangs = allLang.split(",");

            Vector systemLanguages = installData.getSystemLanguages();

            boolean foundLang = false;
            for (int i = 0; i < allLangs.length; i++) {
                String oneLang = allLangs[i];
                oneLang = oneLang.trim();
                if ( systemLanguages.contains(oneLang)) {
                    foundLang = true;
                    int count = installData.getPreselectedLanguages();
                    count++;
                    installData.setPreselectedLanguages(count);
                    break;
                }
            }

            if ( ! foundLang ) {
                packageData.setSelectionState(PackageDescription.DONT_INSTALL);
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            checkLanguagesPackages(child, installData);
        }
    }

    static public void setLanguagesPackages(PackageDescription packageData) {
        if (( packageData.getPkgLanguage() != null ) && ( ! packageData.getPkgLanguage().equals(""))) {
            // This is a package with a specific language.
            packageData.setSelectionState(PackageDescription.INSTALL);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setLanguagesPackages(child);
        }
    }

    static public void setRequiredNewCoreModules(PackageDescription packageData, InstallData installData) {
        // Special handling for core modules, which are required, but not installed.
        boolean isRequiredCoreModule = checkRequiredCoreModule(packageData);
        if ( isRequiredCoreModule ) {
            if ( packageData.getSelectionState() != PackageDescription.INSTALL ) {
                packageData.setSelectionState(PackageDescription.INSTALL);
                LogManager.addLogfileComment("<b>Adding required package:</b> " + packageData.getPackageName() + "</br>");
            }
            // This package has to exist!
            if ( ! packageExists(packageData, installData) ) {

                String packagePath = installData.getPackagePath();
                if (( packageData.getPkgSubdir() != null ) && ( ! packageData.getPkgSubdir().equals("") )) {
                    File completePackageFile = new File(packagePath, packageData.getPkgSubdir());
                    packagePath = completePackageFile.getPath();
                }
                String packageName = packageData.getPackageName();
                File packageFile = new File(packagePath, packageName);

                String log = "<b>Error: Missing required package " + packageFile.getPath() + "</b><br>";
                System.err.println(log);
                String message = ResourceManager.getString("String_File_Not_Found") + ": " + packageFile.getPath();
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                System.exit(1);
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setRequiredNewCoreModules(child, installData);
        }
    }

    static public void defaultDatabaseAnalysis(InstallData data) {

        PackageDescription packageData = SetupDataProvider.getPackageDescription();
        Installer installer = InstallerFactory.getInstance();

        // restore default settings
        if ( data.startSelectionStateSaved() ) {
            // System.err.println("Restoring start selection states");
            ModuleCtrl.restoreStartSelectionStates(packageData);
        } else {
            ModuleCtrl.saveStartSelectionStates(packageData);
            data.setStartSelectionStateSaved(true);
        }

        // Special ToDos, if this is an update installation of an older product.
        // In this case, "chooseInstallationType" and "chooseComponents" are not called.
        // Is it necessary to call "analyzeDatabase" ?
        if ( data.olderVersionExists() ) {
            // Calculation of size is not necessary, because only
            // already installed packages will be updated.

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: Before setUpdateOlderProductSettings");
            }

            // Updating only those packages that are installed.
            ModuleCtrl.setUpdateOlderProductSettings(packageData, data, installer);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After setUpdateOlderProductSettings");
            }

            // Setting packages that are forced into update, because they did not exist in older version.
            ModuleCtrl.setForcedUpdateProductSettings(packageData);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After setForcedUpdateProductSettings");
            }

            // Setting required root module packages (that are new in the update product).
            ModuleCtrl.setRequiredNewCoreModules(packageData, data);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After setRequiredNewCoreModules");
            }

            // Checking, if all packages are available
            ModuleCtrl.disableNonExistingPackages(packageData, data);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After disableNonExistingPackages");
            }

            // disable packages, that are not valid in user installation
            if ( data.isUserInstallation() ) {
                ModuleCtrl.setShowInUserInstallFlags(packageData);

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseDirectory: After setShowInUserInstallFlags");
                }
            }
            else { // disable packages, that are not valid in root installation
                ModuleCtrl.setShowInUserInstallOnlyFlags(packageData);

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseDirectory: After setShowInUserInstallOnlyFlags");
                }
            }

            // Setting parent module settings. Only required for displaying correct module settings before starting installation.
            ModuleCtrl.setParentDefaultModuleSettings(packageData);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After setParentDefaultModuleSettings");
            }

            // Collecting packages to install
            // This has to be done here, because "ChooseInstallationType" and "ChooseComponents"
            // are not called.
            Vector installPackages = new Vector();
            PackageCollector.collectInstallPackages(packageData, installPackages);
            data.setInstallPackages(installPackages);

        } else {   // same version exists or no version exists

            // database changed -> ignore saved states
            data.setTypicalSelectionStateSaved(false);
            data.setCustomSelectionStateSaved(false);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "analyzeDatabase: Start");
            }

            // searching in the database for already installed packages
            LogManager.setCommandsHeaderLine("Analyzing system database");
            ModuleCtrl.setDatabaseSettings(packageData, data, installer);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "analyzeDatabase: After setDatabaseSettings");
            }

            // ModuleCtrl.analyzeDatabase();
            ModuleCtrl.disableNonExistingPackages(packageData, data);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After disableNonExistingPackages");
            }

            // disable packages, that are not valid in user installation
            if ( data.isUserInstallation() ) {
                ModuleCtrl.setShowInUserInstallFlags(packageData);

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseDirectory: After setShowInUserInstallFlags");
                }
            } else { // disable packages, that are not valid in root installation
                ModuleCtrl.setShowInUserInstallOnlyFlags(packageData);

                if ( data.logModuleStates() ) {
                    Dumper.logModuleStates(packageData, "ChooseDirectory: After setShowInUserInstallOnlyFlags");
                }
            }

            // Problem: If all submodules have flag IGNORE, the parent can also get IGNORE
            // That is interesting for language packs with three submodules.
            ModuleCtrl.setParentDefaultModuleSettings(packageData);

            if ( data.logModuleStates() ) {
                Dumper.logModuleStates(packageData, "ChooseDirectory: After setParentDefaultModuleSettings");
            }
        }
    }

}
