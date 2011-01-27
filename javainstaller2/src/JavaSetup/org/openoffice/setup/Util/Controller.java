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
import java.util.Vector;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.LogManager;

public class Controller {

    private Controller() {
    }

    static public void checkPackagePathExistence(InstallData installData) {
        String packagePath = installData.getPackagePath();
        if (( packagePath == null ) || ( packagePath.equals(""))) {
            String message = ResourceManager.getString("String_InstallationOngoing_PackagePath_Not_Found");
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            System.exit(1);
        }
    }

    static public void checkPackageFormat(InstallData installData) {
        String packageFormat = installData.getPackageFormat();
        String os = installData.getOSType();

        boolean notSupportedPackageFormat = true;

        // Show warnings for currently not supported combinations of OS and package format.
        // This has to be adapted if further OS or package formats are supported.

        if (( os.equalsIgnoreCase("SunOS") ) && ( packageFormat.equalsIgnoreCase("pkg") )) {
            notSupportedPackageFormat = false;
        }

        if (( os.equalsIgnoreCase("Linux") ) && ( packageFormat.equalsIgnoreCase("rpm") )) {
            notSupportedPackageFormat = false;
        }

        // Inform user about not supported package format and exit program

        if ( notSupportedPackageFormat ) {
            System.err.println("Error: Package format not supported by this OS!");
            String mainmessage = ResourceManager.getString("String_Packageformat_Not_Supported");
            String osstring = ResourceManager.getString("String_Operating_System");
            String formatstring = ResourceManager.getString("String_Packageformat");
            String message = mainmessage + "\n" + osstring + ": " + os + "\n" + formatstring + ": " + packageFormat;
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            System.exit(1);
        }
    }

    static public void collectSystemLanguages(InstallData installData) {
        String pkgCommand = "";
        String[] pkgCommandArray;
        String adminFileName = "";
        String log = "";
        Vector returnVector = new Vector();
        Vector returnErrorVector = new Vector();
        int returnValue;

        pkgCommand = "locale -a";
        pkgCommandArray = new String[2];
        pkgCommandArray[0] = "locale";
        pkgCommandArray[1] = "-a";
        returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);

        if ( returnValue == 0 ) {
            log = pkgCommand + "<br><b>Returns: " + returnValue + " Successful command</b><br>";
            LogManager.addCommandsLogfileComment(log);

            // System.err.println("Available languages 1: ");
            // for (int i = 0; i < returnVector.size(); i++) {
            //     System.err.println(returnVector.get(i));
            // }

            // Collecting "en-US" instead of "en-US.UTF8"
            Vector realVector = new Vector();

            for (int i = 0; i < returnVector.size(); i++) {
                String oneLang = (String)returnVector.get(i);
                int position = oneLang.indexOf(".");
                if ( position > -1 ) {
                    oneLang = oneLang.substring(0, position);
                }
                if ( ! realVector.contains(oneLang)) {
                    realVector.add(oneLang);
                }
            }

            // System.err.println("Available languages 2: ");
            // for (int i = 0; i < realVector.size(); i++) {
            //     System.err.println(realVector.get(i));
            // }

            installData.setSystemLanguages(realVector);
        } else {    // an error occurred
            log = pkgCommand + "<br><b>Returns: " + returnValue + " An error occurred</b><br>";
            LogManager.addCommandsLogfileComment(log);
            System.err.println("Error in command: " + pkgCommand);
            for (int i = 0; i < returnErrorVector.size(); i++) {
                LogManager.addCommandsLogfileComment((String)returnErrorVector.get(i));
                System.err.println(returnErrorVector.get(i));
            }
        }
    }

    static public boolean createdSubDirectory(String dir) {
        boolean createdDirectory = false;
        boolean errorShown = false;
        String subDirName = "testdir";
        File testDir = new File(dir, subDirName);
        try {
            createdDirectory = SystemManager.create_directory(testDir.getPath());
        }
        catch (SecurityException ex) {
            String message = ResourceManager.getString("String_ChooseDirectory_No_Write_Access") + ": " + dir;
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            errorShown = true;
        }

        if (( ! createdDirectory ) && ( ! errorShown )) {
            String message = ResourceManager.getString("String_ChooseDirectory_No_Write_Access") + ": " + dir;
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            errorShown = true;
        }

        if ( SystemManager.exists_directory(testDir.getPath()) ) {
            testDir.delete();
        }

        return createdDirectory;
    }

    static public boolean createdDirectory(String dir) {
        boolean createdDirectory = false;
        try {
            createdDirectory = SystemManager.create_directory(dir);
        }
        catch (SecurityException ex) {
            // message = ResourceManager.getString("String_ChooseDirectory_Not_Allowed") + ": " + dir;
            // title = ResourceManager.getString("String_Error");
            // Informer.showErrorMessage(message, title);
        }

        if ( ! createdDirectory ) {
            String message = ResourceManager.getString("String_ChooseDirectory_No_Success") + ": " + dir;
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
        }

        return createdDirectory;
    }

    static public boolean reducedRootWritePrivileges() {
        Vector vec = new Vector();
        File dir = new File("/usr");
        vec.add(dir);
        dir = new File("/etc");
        vec.add(dir);

        boolean restrictedWritePrivilges = false;

        // Check for zones. If "zonename" is successful and the name is not "global",
        // this is a "sparse zone".
        // Alternative: Simply always check, if root has write access in selected directories.

        for (int i = 0; i < vec.size(); i++) {
            File directory = (File)vec.get(i);
            if ( directory.exists() ) {
                // do we have write privileges inside the directory
                String tempDirName = "temptestdir";
                File tempDir = new File(directory, tempDirName);

                if ( SystemManager.createDirectory(tempDir) ) {
                    SystemManager.removeDirectory(tempDir);
                } else {
                    restrictedWritePrivilges = true;
                    System.err.println("Restricted Root privileges. No write access in " + directory.getPath());
                    break;
                }
            }
        }

        return restrictedWritePrivilges;
    }

    static public void checkForNewerVersion(InstallData installData) {
        LogManager.setCommandsHeaderLine("Checking change installation");
        InstallChangeCtrl.checkInstallChange(installData);

        if ( installData.newerVersionExists() ) {
            // Inform user about a newer version installed
            SetupDataProvider.setNewMacro("DIR", installData.getInstallDefaultDir()); // important for string replacement

            System.err.println("Error: A newer version is already installed in " + installData.getInstallDefaultDir() + " !");
            String message1 = ResourceManager.getString("String_Newer_Version_Installed_Found")
                            + "\n" + installData.getInstallDefaultDir() + "\n";
            String message2 = ResourceManager.getString("String_Newer_Version_Installed_Remove");
            String message = message1 + "\n" + message2;
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            System.exit(1);
        }
    }

    static public void checkForUidFile(InstallData installData) {
        // check existence of getuid.so
        File getuidFile = Controller.findUidFile(installData);

        if (( getuidFile == null ) || (! getuidFile.exists()) ) {
            // Root privileges required -> abort installation
            System.err.println("Root privileges required for installation!");
            String message = ResourceManager.getString("String_Root_Privileges_Required_1") + "\n"
                           + ResourceManager.getString("String_Root_Privileges_Required_2");
            String title = ResourceManager.getString("String_Error");
            Informer.showErrorMessage(message, title);
            System.exit(1);
        } else {
            installData.setGetUidPath(getuidFile.getPath());
        }
    }

    static private File findUidFile(InstallData data) {

        File getuidFile = null;

        if ( data.isInstallationMode()) {
            String getuidpath = System.getProperty("GETUID_PATH");

            if ( getuidpath != null ) {
                getuidFile = new File(getuidpath);

                if (( getuidFile.isDirectory() ) && ( ! getuidFile.isFile() )) {
                    // Testing, if GETUID_PATH only contains the path, not the filename
                    String defaultfilename = "getuid.so";
                    getuidFile = new File(getuidpath, defaultfilename);

                    if ( ! getuidFile.exists() ) {
                        getuidFile = null;
                    }
                }
            }

            // File resourceRoot = data.getResourceRoot();
            // String getuidString = "getuid.so";
            // if ( resourceRoot != null ) {
            //     File getuidDir = new File (data.getInfoRoot(), "getuid");
            //     getuidFile = new File(getuidDir, getuidString);
            // }

        } else {
            getuidFile = new File(data.getGetUidPath());
        }

        return getuidFile;
    }

}
