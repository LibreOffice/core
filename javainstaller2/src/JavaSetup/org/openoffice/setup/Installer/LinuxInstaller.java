/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LinuxInstaller.java,v $
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

package org.openoffice.setup.Installer;

import java.io.File;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.InstallerHelper.LinuxHelper;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.ExecuteProcess;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

public class LinuxInstaller extends Installer {

    LinuxHelper helper = new LinuxHelper();

    public LinuxInstaller() {
        super();
    }

    public void preInstall(PackageDescription packageData) {
        InstallData installData = InstallData.getInstance();
        // Collecting package names
        helper.getLinuxPackageNamesFromRpmquery(packageData, installData);
        helper.getLinuxFileInfo(packageData);
        // Dumper.dumpAllRpmInfo(packageData);
    }

    public void postInstall(PackageDescription packageData) {
        InstallData data = InstallData.getInstance();

        if ( ! data.isAbortedInstallation() ) {
            data.setStillRunning(true);
            // Collecting information about installed packages
            // Creating a list containing pairs of package names and rpm file names
            // that has to be used during uninstallation.
            helper.createPackageNameFileAtPostinstall(data, packageData);
            helper.saveModulesLogFile(data);
            data.setStillRunning(false);
        }
    }

    public void preUninstall(PackageDescription packageData) {
        // Collecting information about installed packages
        HashMap packageNames = helper.readPackageNamesFile();
        helper.setFullPackageNameAtUninstall(packageData, packageNames);
        helper.getLinuxFileInfo(packageData);
        // Dumper.dumpAllRpmInfo(packageData);
    }

    public void defineDatabasePath() {

        InstallData data = InstallData.getInstance();
        String oldDatabasePath = data.getDatabasePath();
        data.setDatabasePath(null);

        // Determining the database path (only for user installation).
        // Important if a user installation is done into an existing
        // user installation -> this methode can only be called after
        // determination of installation directory.

        if ( data.isUserInstallation() ) {
            String databasePath = helper.getLinuxDatabasePath(data); // the file does not need to exist!
            data.setDatabasePath(databasePath);

            // If this is a new path to the database, then this database was
            // not analyzed before (when going back in installation wizard)
            if ( ! databasePath.equals(oldDatabasePath) ) {
                data.setDatabaseAnalyzed(false);
                data.setDatabaseQueried(false);
            }
        }
    }

    public String getChangeInstallDir(PackageDescription packageData) {
        String installDir = null;
        InstallData installData = InstallData.getInstance();

        String packageName = packageData.getPkgRealName();

        if ( packageName != null ) {
            String rpmCommand = "rpm -q --queryformat %{INSTALLPREFIX} " + packageName;
            String[] rpmCommandArray = new String[5];
            rpmCommandArray[0] = "rpm";
            rpmCommandArray[1] = "-q";
            rpmCommandArray[2] = "--queryformat";
            rpmCommandArray[3] = "%{INSTALLPREFIX}";
            rpmCommandArray[4] = packageName;

            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, returnVector, returnErrorVector);
            String returnString = (String) returnVector.get(0);
            returnString = returnString.replaceAll("//","/");

            String log = rpmCommand + "<br><b>Returns: " + returnString + "</b><br>";
            LogManager.addCommandsLogfileComment(log);

            installDir = returnString;
        }

        return installDir;
    }

    public void installPackage(PackageDescription packageData) {
        // get ProductDescription for logging mechanism
        ProductDescription productData = SetupDataProvider.getProductDescription();
        InstallData data = InstallData.getInstance();

        // String log = "<br><b>Package: " + packageData.getName() + "</b>";
        // LogManager.addCommandsLogfileComment(log);
        String log = "";

        // setting installDir
        String installDir = data.getInstallDir();
        String packagePath = data.getPackagePath();

        if (( packageData.getPkgSubdir() != null ) && ( ! packageData.getPkgSubdir().equals("") )) {
            File completePackageFile = new File(packagePath, packageData.getPkgSubdir());
            packagePath = completePackageFile.getPath();
        }

        String packageName = packageData.getPackageName();

        if (( packageName.equals("")) || ( packageName == null )) {
            log = "<b>No package name specified. Nothing to do</b>";
            LogManager.addCommandsLogfileComment(log);
        } else {
            log = "<b>Package Name: " + packageName + "</b>";
            LogManager.addCommandsLogfileComment(log);

            File completePackage = new File(packagePath, packageName);
            packageName = completePackage.getPath();

            if ( completePackage.exists() ) {
                String fixedInstallDir = helper.fixInstallationDirectory(installDir);
                String relocations = helper.getRelocationString(packageData, packageName);
                if ( relocations != null ) {
                    relocations = relocations + fixedInstallDir;
                    // relocations: "/opt/staroffice8=" + fixedInstallDir;
                }

                String rpmCommand = "";
                String[] rpmCommandArray;
                String databasePath = null;
                String databaseString = "";
                boolean useLocalDatabase = false;

                if ( data.isUserInstallation() ) {
                    databasePath = data.getDatabasePath();
                    if ( databasePath == null ) {
                        databasePath = helper.getLinuxDatabasePath(data);
                        data.setDatabasePath(databasePath);
                    }
                }

                if (( databasePath != null ) && (! databasePath.equalsIgnoreCase("null"))) {
                    databaseString = "--dbpath";
                    useLocalDatabase = true;
                }

                if (useLocalDatabase) {
                    if ( relocations != null )
                    {
                        rpmCommand = "rpm --upgrade --nodeps --ignoresize -vh " +
                                "--relocate " + relocations + " " + databaseString +
                                " " + databasePath + " " + packageName;
                        rpmCommandArray = new String[10];
                        rpmCommandArray[0] = "rpm";
                        rpmCommandArray[1] = "--upgrade";
                        rpmCommandArray[2] = "--nodeps";
                        rpmCommandArray[3] = "--ignoresize";
                        rpmCommandArray[4] = "-vh";
                        rpmCommandArray[5] = "--relocate";
                        rpmCommandArray[6] = relocations;
                        rpmCommandArray[7] = databaseString;
                        rpmCommandArray[8] = databasePath;
                        rpmCommandArray[9] = packageName;
                    } else {
                        rpmCommand = "rpm --upgrade --nodeps --ignoresize -vh " +
                                databaseString + " " + databasePath + " " + packageName;
                        rpmCommandArray = new String[8];
                        rpmCommandArray[0] = "rpm";
                        rpmCommandArray[1] = "--upgrade";
                        rpmCommandArray[2] = "--nodeps";
                        rpmCommandArray[3] = "--ignoresize";
                        rpmCommandArray[4] = "-vh";
                        rpmCommandArray[5] = databaseString;
                        rpmCommandArray[6] = databasePath;
                        rpmCommandArray[7] = packageName;
                    }
                } else {
                    if ( relocations != null )
                    {
                        rpmCommand = "rpm --upgrade --nodeps --ignoresize -vh " +
                                "--relocate " + relocations + " " + packageName;
                        rpmCommandArray = new String[8];
                        rpmCommandArray[0] = "rpm";
                        rpmCommandArray[1] = "--upgrade";
                        rpmCommandArray[2] = "--nodeps";
                        rpmCommandArray[3] = "--ignoresize";
                        rpmCommandArray[4] = "-vh";
                        rpmCommandArray[5] = "--relocate";
                        rpmCommandArray[6] = relocations;
                        rpmCommandArray[7] = packageName;
                    } else {
                        rpmCommand = "rpm --upgrade --nodeps --ignoresize -vh " + packageName;
                        rpmCommandArray = new String[6];
                        rpmCommandArray[0] = "rpm";
                        rpmCommandArray[1] = "--upgrade";
                        rpmCommandArray[2] = "--nodeps";
                        rpmCommandArray[3] = "--ignoresize";
                        rpmCommandArray[4] = "-vh";
                        rpmCommandArray[5] = packageName;
                    }
                }

                Vector returnVector = new Vector();
                Vector returnErrorVector = new Vector();
                // int returnValue = SystemManager.executeProcessReturnVector(rpmCommand, returnVector, returnErrorVector);
                int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, returnVector, returnErrorVector);

                if ( returnValue == 0 ) {
                    log = rpmCommand + "<br><b>Returns: " + returnValue + " Successful installation</b><br>";
                    LogManager.addCommandsLogfileComment(log);
                } else {    // an error occured during installation
                    log = rpmCommand + "<br><b>Returns: " + returnValue + " Error during installation</b><br>";
                    LogManager.addCommandsLogfileComment(log);
                    for (int i = 0; i < returnErrorVector.size(); i++) {
                        LogManager.addCommandsLogfileComment((String)returnErrorVector.get(i));
                    }
                    data.setIsErrorInstallation(true);
                }

                // saving installation state at package
                packageData.setIsNewInstalled(true);
            } else {
                log = "<b>Error: Did not find package " + packageName + "</b><br>";
                System.err.println(log);
                String message = ResourceManager.getString("String_File_Not_Found") + ": " + packageName;
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                LogManager.addCommandsLogfileComment(log);
                data.setIsErrorInstallation(true);
            }
        }
    }

    public void uninstallPackage(PackageDescription packageData) {
        // get ProductDescription for logging mechanism
        ProductDescription productData = SetupDataProvider.getProductDescription();
        InstallData data = InstallData.getInstance();

        String log  = "";
        // String log = "<br><b>Package: " + packageData.getName() + "</b>";
        // LogManager.addCommandsLogfileComment(log);

        String rpmPackageName = packageData.getPackageName();

        if (( rpmPackageName.equals("")) || ( rpmPackageName == null )) {
            log = "<b>No package name specified. Nothing to do</b>";
            LogManager.addCommandsLogfileComment(log);
        } else {
            log = "<b>Package Name: " + rpmPackageName + "</b>";
            LogManager.addCommandsLogfileComment(log);

            String packageName = packageData.getPkgRealName();
            String databasePath = data.getDatabasePath();
            String databaseString = "";
            boolean useLocalDatabase = false;
            String rpmCommand;
            String[] rpmCommandArray;

            if (( databasePath != null ) && (! databasePath.equalsIgnoreCase("null"))) {
                databaseString = "--dbpath";
                useLocalDatabase = true;
            }

            if (useLocalDatabase) {
                rpmCommand = "rpm -ev --nodeps" + " " + databaseString + " " + databasePath + " " + packageName;
                rpmCommandArray = new String[6];
                rpmCommandArray[0] = "rpm";
                rpmCommandArray[1] = "-ev";
                rpmCommandArray[2] = "--nodeps";
                rpmCommandArray[3] = databaseString;
                rpmCommandArray[4] = databasePath;
                rpmCommandArray[5] = packageName;
            } else {
                rpmCommand = "rpm -ev --nodeps" + " " + packageName;
                rpmCommandArray = new String[4];
                rpmCommandArray[0] = "rpm";
                rpmCommandArray[1] = "-ev";
                rpmCommandArray[2] = "--nodeps";
                rpmCommandArray[3] = packageName;
            }

            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, returnVector, returnErrorVector);

            if ( returnValue == 0 ) {
                log = rpmCommand + "<br><b>Returns: " + returnValue + " Successful uninstallation</b><br>";
                LogManager.addCommandsLogfileComment(log);
            } else {    // an error occured during installation
                log = rpmCommand + "<br><b>Returns: " + returnValue + " Error during uninstallation</b><br>";
                LogManager.addCommandsLogfileComment(log);
                for (int i = 0; i < returnErrorVector.size(); i++) {
                    LogManager.addCommandsLogfileComment((String)returnErrorVector.get(i));
                }
                data.setIsErrorInstallation(true);
            }
        }
    }

    public boolean isPackageInstalledClassic(PackageDescription packageData, InstallData installData) {

        boolean isInstalled = false;
        boolean doCheck = false;

        // only checking existing packages (and always at uninstallation)
        if ( (packageData.pkgExists()) || (installData.isUninstallationMode()) ) {
            doCheck = true;
        }

        String rpmPackageName = packageData.getPackageName();

        if ( rpmPackageName.equals("") ) {
            rpmPackageName = null;
        }

        if (( rpmPackageName != null ) && ( doCheck )) {

            String databaseString = "";
            String databasePath = null;
            String packageName = packageData.getPkgRealName();
            Vector allPackages = null;
            boolean useLocalDatabase = false;

            if (installData.isUserInstallation()) {
                databasePath = installData.getDatabasePath();
            }

            if (( databasePath != null ) && (! databasePath.equals("null"))) {
                databaseString = "--dbpath";
                useLocalDatabase = true;
            }

            if (packageName != null) {

                String rpmCommand;
                String[] rpmCommandArray;

                if (useLocalDatabase) {
                    rpmCommand = "rpm" + " " + databaseString + " " + databasePath + " --query " + packageName;
                    rpmCommandArray = new String[5];
                    rpmCommandArray[0] = "rpm";
                    rpmCommandArray[1] = databaseString;
                    rpmCommandArray[2] = databasePath;
                    rpmCommandArray[3] = "--query";
                    rpmCommandArray[4] = packageName;
                } else {
                    rpmCommand = "rpm" + " --query " + packageName;
                    rpmCommandArray = new String[3];
                    rpmCommandArray[0] = "rpm";
                    rpmCommandArray[1] = "--query";
                    rpmCommandArray[2] = packageName;
                }

                int returnValue = ExecuteProcess.executeProcessReturnValue(rpmCommandArray);

                if ( returnValue == 0 ) {
                    isInstalled = true;
                    packageData.setWasAlreadyInstalled(true); // needed for logging
                    String log = rpmCommand + "<br><b>Returns: " + returnValue + " Package is installed" + "</b><br>";
                    LogManager.addCommandsLogfileComment(log);
                } else {
                    String log = rpmCommand + "<br><b>Returns: " + returnValue + " Package is not installed" + "</b><br>";
                    LogManager.addCommandsLogfileComment(log);
                }
            }
        }

        return isInstalled;
    }

    private void queryAllDatabase(InstallData installData) {

        String databaseString = "";
        String databasePath = null;
        HashMap map = new HashMap();;
        boolean useLocalDatabase = false;

        if (installData.isUserInstallation()) {
            databasePath = installData.getDatabasePath();
        }

        if (( databasePath != null ) && (! databasePath.equals("null"))) {
            databaseString = "--dbpath";
            useLocalDatabase = true;
        }

        String rpmCommand;
        String[] rpmCommandArray;

        if (useLocalDatabase) {
            rpmCommand = "rpm" + " " + databaseString + " " + databasePath + " --query" + " -a";
            rpmCommandArray = new String[5];
            rpmCommandArray[0] = "rpm";
            rpmCommandArray[1] = databaseString;
            rpmCommandArray[2] = databasePath;
            rpmCommandArray[3] = "--query";
            rpmCommandArray[4] = "-a";
        } else {
            rpmCommand = "rpm" + " --query" + " -a";
            rpmCommandArray = new String[3];
            rpmCommandArray[0] = "rpm";
            rpmCommandArray[1] = "--query";
            rpmCommandArray[2] = "-a";
        }

        Vector returnVector = new Vector();
        Vector returnErrorVector = new Vector();
        int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, returnVector, returnErrorVector);

        String log = rpmCommand + "<br><b>Returns: " + returnValue + "</b><br>";
        LogManager.addCommandsLogfileComment(log);
        String value = "1";

        if ( ! returnVector.isEmpty()) {
            for (int i = 0; i < returnVector.size(); i++) {
                String onePackage = (String)returnVector.get(i);
                int pos1 = onePackage.lastIndexOf("-");
                int pos2 = onePackage.substring(0, pos1).lastIndexOf("-");
                map.put(onePackage.substring(0, pos2), value);
            }
        }

        installData.setDatabaseQueried(true);
        installData.setDatabaseMap(map);
    }

    public boolean isPackageInstalled(PackageDescription packageData, InstallData installData) {

        boolean isInstalled = false;
        boolean doCheck = false;

        // only checking existing packages (and always at uninstallation)
        if ( (packageData.pkgExists()) || (installData.isUninstallationMode()) ) {
            doCheck = true;
        }

        String rpmPackageName = packageData.getPackageName();

        if ( rpmPackageName.equals("") ) {
            rpmPackageName = null;
        }

        if (( rpmPackageName != null ) && ( doCheck )) {
            String packageName = packageData.getPkgRealName();

            if (packageName != null) {

                HashMap map = null;
                if ( ! installData.databaseQueried() ) {
                    queryAllDatabase(installData);
                }

                map = installData.getDatabaseMap();

                if ( map.containsKey(packageName)) {
                    isInstalled = true;
                }
            } else {
                System.err.println("Error: No packageName defined for package: " + packageData.getPackageName());
            }
        }

        return isInstalled;
    }

    public boolean isInstallSetPackageOlder(PackageDescription packageData, InstallData installData) {
        boolean installSetPackageIsOlder = false;
        boolean checkIfInstalledIsOlder = false;
        installSetPackageIsOlder = findOlderPackage(packageData, installData, checkIfInstalledIsOlder);
        return installSetPackageIsOlder;
    }

    public boolean isInstalledPackageOlder(PackageDescription packageData, InstallData installData) {
        boolean installedPackageIsOlder = false;
        boolean checkIfInstalledIsOlder = true;
        installedPackageIsOlder = findOlderPackage(packageData, installData, checkIfInstalledIsOlder);
        return installedPackageIsOlder;
    }

    private boolean findOlderPackage(PackageDescription packageData, InstallData installData, boolean checkIfInstalledIsOlder) {

        // The information about the new package is stored in packageData (the version and the release).
        // This information can be stored in xpd files. If it is not stored in xpd files, it is determined
        // during installation process by querying the rpm file. This process costs much time and should
        // therefore be done by the process, that creates the xpd files. On the other hand this requires,
        // that the xpd files contain the correct information.

        boolean isOlder = false;

        // get the version of the installed package
        String rpmPackageName = packageData.getPackageName();
        String log;

        if ( rpmPackageName.equals("")) {
            rpmPackageName = null;
        }

        if ( rpmPackageName != null ) {
            String databaseString = "";
            String databasePath = null;
            String packageName =  packageData.getPkgRealName();
            Vector allPackages = null;
            boolean useLocalDatabase = false;

            if (installData.isUserInstallation()) {
                databasePath = installData.getDatabasePath();
            }

            if (( databasePath != null ) && (! databasePath.equals("null"))) {
                databaseString = "--dbpath";
                useLocalDatabase = true;
            }

            if (packageName != null) {
                // Collect information about the installed package by querying the database.
                // Instead of rpm file name, the real package name has to be used.

                String rpmCommand;
                String[] rpmCommandArray;

                if (useLocalDatabase) {
                    rpmCommand = "rpm" + " " + databaseString + " " + databasePath + " -q --queryformat %{VERSION}\\n " + packageName;
                    rpmCommandArray = new String[7];
                    rpmCommandArray[0] = "rpm";
                    rpmCommandArray[1] = databaseString;
                    rpmCommandArray[2] = databasePath;
                    rpmCommandArray[3] = "-q";
                    rpmCommandArray[4] = "--queryformat";
                    rpmCommandArray[5] = "%{VERSION}\\n";
                    rpmCommandArray[6] = packageName;
                } else {
                    rpmCommand = "rpm" + " -q --queryformat %{VERSION}\\n " + packageName;
                    rpmCommandArray = new String[5];
                    rpmCommandArray[0] = "rpm";
                    rpmCommandArray[1] = "-q";
                    rpmCommandArray[2] = "--queryformat";
                    rpmCommandArray[3] = "%{VERSION}\\n";
                    rpmCommandArray[4] = packageName;
                }

                Vector versionVector = new Vector();
                Vector returnErrorVector = new Vector();
                int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, versionVector, returnErrorVector);
                String version = (String) versionVector.lastElement();
                log = rpmCommand + "<br><b>Returns: " + version + "</b><br>";
                LogManager.addCommandsLogfileComment(log);

                if (useLocalDatabase) {
                    rpmCommand = "rpm" + " " + databaseString + " " + databasePath + " -q --queryformat %{RELEASE}\\n " + packageName;
                    rpmCommandArray[5] = "%{RELEASE}\\n";
                } else {
                    rpmCommand = "rpm" + " -q --queryformat %{RELEASE}\\n " + packageName;
                    rpmCommandArray[3] = "%{RELEASE}\\n";
                }

                Vector releaseVector = new Vector();
                returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, releaseVector, returnErrorVector);
                String release = (String) releaseVector.lastElement();

                log = rpmCommand + "<br><b>Returns: " + release + "</b><br>";
                LogManager.addCommandsLogfileComment(log);

                isOlder = helper.compareVersionAndRelease(version, release, packageData, checkIfInstalledIsOlder);

                if ( checkIfInstalledIsOlder ) {
                    if ( isOlder ) {
                        LogManager.addCommandsLogfileComment("<b>-> Installed package is older</b><br>");
                    } else {
                        LogManager.addCommandsLogfileComment("<b>-> Installed package is not older</b><br>");
                    }
                } else {
                    if ( isOlder ) {
                        LogManager.addCommandsLogfileComment("<b>-> Package in installation set is older</b><br>");
                    } else {
                        LogManager.addCommandsLogfileComment("<b>-> Package in installation set is not older</b><br>");
                    }
                }

            } else {
                System.err.println("Error: No packageName defined for package: " + rpmPackageName);
            }
        }

        return isOlder;
    }

}
