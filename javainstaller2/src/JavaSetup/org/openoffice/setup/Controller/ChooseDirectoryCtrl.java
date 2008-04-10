/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChooseDirectoryCtrl.java,v $
 * $Revision: 1.5 $
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

import java.awt.Dimension;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseDirectory;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InstallChangeCtrl;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.Vector;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import org.openoffice.setup.Dialogs.DatabaseDialog;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.Controller;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.PackageCollector;

public class ChooseDirectoryCtrl extends PanelController {

    private String helpFile;

    public ChooseDirectoryCtrl() {
        super("ChooseDirectory", new ChooseDirectory());
        helpFile = "String_Helpfile_ChooseDirectory";
    }

    public String getNext() {

        InstallData data = InstallData.getInstance();

        if ( data.olderVersionExists() ) {
            return new String("InstallationImminent");
        } else if ( data.sameVersionExists() ) {
            return new String("ChooseComponents");
        } else {
            return new String("ChooseInstallationType");
        }
    }

    public String getPrevious() {
        return new String("AcceptLicense");
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    private String removeEndingDelimiter(String s) {
        s.trim();
        if (( s.endsWith("/")) || ( s.endsWith("\\"))) {
            // removing the last character
            s = s.substring(0, s.length() - 1);
        }
        // mask spaces
        // s.replaceAll(" ", "\\ ");

        return s;
    }

    public void beforeShow() {
        ChooseDirectory panel = (ChooseDirectory)getPanel();
        InstallData data = InstallData.getInstance();
        panel.setDirectory(data.getInstallDir());

        if ( data.isSolarisUserInstallation() ) {
            if ( data.getInstallRoot() == null ) {
                String rootDir = "/";
                // System.getenv only supported in Java 1.5, property set in shell script
                // if (( System.getenv("HOME") != null ) && ( ! System.getenv("HOME").equals(""))) {
                //     rootDir = System.getenv("HOME");
                // }
                if (( System.getProperty("HOME") != null ) && ( ! System.getProperty("HOME").equals("") )) {
                    rootDir = System.getProperty("user.home");
                }
                data.setInstallRoot(rootDir);
            }
            panel.setRootDirectory(data.getInstallRoot());
        }

        // In change of installations with root privileges, the destination directory is fix.
        // Therefore before the directory selection dialog is shown, it has to be controlled,
        // if there are already installed products.
        // In installations with user privileges, this can only be checked, after the
        // destination directory is controlled.
        if ( data.isRootInstallation() ) {
            // LogManager.setCommandsHeaderLine("Checking change installation");
            // InstallChangeCtrl.checkInstallChange(data, panel);  // <- much earlier!
            // InstallChangeCtrl.checkInstallChange(data);  // <- much earlier!

            if (data.isChangeInstallation()) {
                panel.setDirectory(data.getInstallDir());
                panel.disableDirectoryField();
                panel.disableBrowseButton();

                // Maintenance mode
                if ( data.sameVersionExists() ) {
                    String dialogTitle = ResourceManager.getString("String_ChooseDirectory1_Maintain");
                    panel.setTitleText(dialogTitle);
                    // String dialogText = ResourceManager.getString("String_ChooseDirectory2_Maintain");
                    // panel.setDialogText(dialogText);
                }

                // Update mode
                if ( data.olderVersionExists() ) {
                    String dialogTitle = ResourceManager.getString("String_ChooseDirectory1_Update");
                    panel.setTitleText(dialogTitle);
                    // String dialogText = ResourceManager.getString("String_ChooseDirectory2_Update");
                    // panel.setDialogText(dialogText);
                }
            }
        }

    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        ChooseDirectory panel = (ChooseDirectory)getPanel();
        String dir = panel.getDirectory();
        dir = removeEndingDelimiter(dir);
        InstallData data = InstallData.getInstance();
        data.setInstallDir(dir);

        if ( data.isSolarisUserInstallation() ) {
            String rootDir = panel.getRootDirectory();
            rootDir = removeEndingDelimiter(rootDir);
            data.setInstallRoot(rootDir);
            File completeDir = new File(rootDir, dir);
            dir = completeDir.getPath();
        }

        SetupDataProvider.setNewMacro("DIR", dir); // important for string replacement
        // SetupDataProvider.dumpMacros();

        // Check existence of directory. Try to create, if it does not exist.
        // If successufully created, calculate available disc space

        if ( nextButtonPressed ) {

            // If the directory exists, is has to be tested, whether the user has write access

            if ( SystemManager.exists_directory(dir) ) {
                if ( ! Controller.createdSubDirectory(dir) ) {
                    repeatDialog= true;
                }
            }

            // If the directory does not exist, is has to be tested, whether the user can create it

            if ( ! SystemManager.exists_directory(dir)) {
                String title = ResourceManager.getString("String_ChooseDirectory4_Question_Title");
                String message = null;
                message = ResourceManager.getString("String_ChooseDirectory5_Question_Message");

                int returnValue = JOptionPane.showConfirmDialog(null, message, title, JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);

                if (  returnValue == JOptionPane.YES_OPTION ) {
                    if ( ! Controller.createdDirectory(dir) ) {
                        repeatDialog= true;
                    }
                }
                else if (returnValue == JOptionPane.NO_OPTION) {
                    repeatDialog = true;
                }
            }

            // Additional tasks, if the directory is okay

            if ( ! repeatDialog ) {
                // Calculate available disc space
                int discSpace = SystemManager.calculateDiscSpace(dir);
                data.setAvailableDiscSpace(discSpace);

                Installer installer = InstallerFactory.getInstance();

                // Is this a new directory, or one that was already set before.
                // In this case, the database does not need to be evaluated again.
                // -> Testing, whether the database path has changed. This can
                // only happen in user installations.
                if ( data.isUserInstallation() ) {
                    String oldDatabasePath = data.getDatabasePath();
                    // Setting the database path
                    installer.defineDatabasePath();

                    if (( oldDatabasePath == null ) || ( ! oldDatabasePath.equals(data.getDatabasePath()))) {
                        data.setDatabaseAnalyzed(false);
                        data.setDatabaseQueried(false);
                    } else {
                        data.setDatabaseAnalyzed(true);
                    }
                }

                // In installations with user privileges, now it can be controlled,
                // if there are products installed in the selected directory.
                // Therefore the directory selection dialog has to be shown before.
                // In installations with root privileges, this can only be checked,
                // before the destination directory can be set, because it is fix.
                if ( data.isUserInstallation() ) {
                    LogManager.setCommandsHeaderLine("Checking change installation");
                    InstallChangeCtrl.checkInstallChange(data);
                    // InstallChangeCtrl.checkInstallChange(data, panel);
                }

                // At this point it is clear, whether the update-Package is installed or not.
                // If it is installed, it is also clear, in which version it is installed.
                // Therefore the following dialog is also defined.

                if ( data.newerVersionExists() ) {
                    // This can happen only in installation with user privileges.
                    // Installations with root privileges have cancelled installation
                    // already at first dialog.
                    // Possibility to select a new directory, in which no
                    // newer product exists
                    String message = ResourceManager.getString("String_Newer_Version_Installed_Found") + "\n" + data.getInstallDir() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Database") + ": " + data.getDatabasePath() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Tip");
                    String title = ResourceManager.getString("String_Error");
                    Informer.showErrorMessage(message, title);

                    repeatDialog = true;
                }

                // If an older version is found, and the update is forbidden, the user
                // can select another installation directory.

                if ( data.olderVersionExists() && data.dontUpdate() ) {
                    // This can happen only in installation with user privileges.
                    // Installations with root privileges have cancelled installation
                    // already at first dialog (InstallChangeCtrl.java).
                    // Possibility to select a new directory, in which no
                    // older product exists
                    String message = ResourceManager.getString("String_Older_Version_Installed_Found") + "\n" + data.getInstallDir() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Database") + ": " + data.getDatabasePath() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Tip");
                    String title = ResourceManager.getString("String_Error");
                    Informer.showErrorMessage(message, title);

                    repeatDialog = true;
                }

                if ( ! repeatDialog ) {

                    if ( ! data.databaseAnalyzed()) {
                        ModuleCtrl.defaultDatabaseAnalysis(data);
                        data.setDatabaseAnalyzed(true);
                    }
                }
            }
        }

        return repeatDialog;
    }

}
