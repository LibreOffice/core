/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UninstallationPrologueCtrl.java,v $
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
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationPrologue;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Converter;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.HashMap;
import java.util.Vector;
import org.openoffice.setup.ResourceManager;

public class UninstallationPrologueCtrl extends PanelController {

    private String helpFile;

    public UninstallationPrologueCtrl() {
        super("UninstallationPrologue", new UninstallationPrologue());
        helpFile = "String_Helpfile_UninstallationPrologue";
    }

    public String getNext() {
        return new String("ChooseUninstallationType");
    }

    public String getPrevious() {
        return null;
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    private HashMap getInfoFileData() {
        InstallData data = InstallData.getInstance();
        File infoRootDir = data.getInfoRoot();
        String infoFilename = "infoFile";
        File infoFile = new File(infoRootDir, infoFilename);
        Vector infoFileContent = SystemManager.readCharFileVector(infoFile.getPath());
        HashMap map = Converter.convertVectorToHashmap(infoFileContent);

        // for (int i = 0; i < infoFileContent.size(); i++) {
        //    System.out.println(infoFileContent.get(i));
        // }

        // Iterator m = map.entrySet().iterator();
        // while ( m.hasNext() ) {
        //     Map.Entry entry = (Map.Entry) m.next();
        //     System.out.println( "MAP:" + entry.getKey() + ":" + entry.getValue() );
        // }

        return map;
    }

    private void setNewInstallData(HashMap map) {
        InstallData data = InstallData.getInstance();
        // adding information to installData
        data.setPackagePath((String)map.get("PackagePath"));
        data.setAdminFileNameReloc((String)map.get("AdminFileReloc"));
        data.setAdminFileNameNoReloc((String)map.get("AdminFileNoReloc"));
        data.setDatabasePath((String)map.get("DatabasePath"));
        data.setInstallDir((String)map.get("InstallationDir"));
        data.setInstallRoot((String)map.get("InstallationRoot"));
        data.setStoredInstallationPrivileges((String)map.get("InstallationPrivileges"));
        data.setGetUidPath((String)map.get("GetUidFile"));
    }

    private void readInfoFile() {
        HashMap map = getInfoFileData();
        setNewInstallData(map);
        Dumper.dumpNewInstallData();
    }

    private void checkUninstallPrivileges() {
        InstallData data = InstallData.getInstance();
        // data.setStoredInstallationPrivileges((String)map.get("InstallationPrivileges"));
        String originalPrivileges = data.getStoredInstallationPrivileges();
        String currentPrivileges = data.getInstallationPrivileges();
        if ( ! currentPrivileges.equalsIgnoreCase(originalPrivileges) ) {
            // aborting installation with error message
            if ( currentPrivileges.equalsIgnoreCase("root")) {
                String message = ResourceManager.getString("String_UninstallationPrologue_Wrong_Privileges_Current_Root");
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                String log = "<b>Error: Wrong uninstallation privileges (currently Root)!</b><br>";
                System.err.println(log);
                // LogManager.addLogfileComment(log);
            } else {
                String message = ResourceManager.getString("String_UninstallationPrologue_Wrong_Privileges_Current_User");
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                String log = "<b>Error: Wrong uninstallation privileges (currently User)!</b><br>";
                System.err.println(log);
                // LogManager.addLogfileComment(log);
            }
            System.exit(1);
        }

    }

    public void beforeShow() {
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        // System.err.println("\nUninstallation module state dump 1:");
        // PackageDescription packageData = SetupDataProvider.getPackageDescription();
        // ModuleCtrl.dumpModuleStates(packageData);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);
    }

    public void duringShow() {

        Thread t = new Thread() {
            public void run() {
                InstallData installData = InstallData.getInstance();
                if ( ! installData.databaseAnalyzed() ) {
                    getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_NEXT);

                    // now it is time to read the infoFile in directory "installData.getInfoRoot()"
                    readInfoFile();

                    // controlling the installation privileges. Are the original installation privileges
                    // identical with the current deinstallation privileges?
                    checkUninstallPrivileges();

                    PackageDescription packageData = SetupDataProvider.getPackageDescription();
                    Installer installer = InstallerFactory.getInstance();
                    installer.preUninstall(packageData);

                    // searching in the database for already installed packages
                    LogManager.setCommandsHeaderLine("Analyzing system database");
                    ModuleCtrl.setDatabaseSettings(packageData, installData, installer);
                    installData.setDatabaseAnalyzed(true);
                    ModuleCtrl.setDontUninstallFlags(packageData);
                    ModuleCtrl.setParentDefaultModuleSettings(packageData);
                    getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_NEXT);
                    getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);
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

}
