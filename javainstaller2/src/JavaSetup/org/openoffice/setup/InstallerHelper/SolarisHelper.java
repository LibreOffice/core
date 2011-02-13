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

package org.openoffice.setup.InstallerHelper;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.Controller;
import org.openoffice.setup.Util.Converter;
import org.openoffice.setup.Util.ExecuteProcess;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.HashMap;
import java.util.Vector;

public class SolarisHelper {

    public SolarisHelper() {
    }

    private void createLocalTempDir() {
        String mktempString = "/usr/bin/mktemp";
        File mktemp = new File(mktempString);
        if ( mktemp.exists() ) {
            // this is Solaris 10
            LogManager.setCommandsHeaderLine("Preparing Solaris 10 user installation");
            InstallData data = InstallData.getInstance();
            String mktempCommand = mktempString + " -d";
            String[] mktempCommandArray = new String[2];
            mktempCommandArray[0] = mktempString;
            mktempCommandArray[1] = "-d";
            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(mktempCommandArray, returnVector, returnErrorVector);
            String uniqueLocalDir = (String) returnVector.get(0);

            data.setLocalTempPath(uniqueLocalDir);

            String log = mktempCommand + "<br><b>Returns: " + uniqueLocalDir + "</b><br>";
            LogManager.addCommandsLogfileComment(log);

            String installRoot = data.getInstallDir();
            File installRootTemp = new File(installRoot, "tmp");

            if ( installRootTemp.exists() ) {
                SystemManager.removeDirectory(installRootTemp);
            }

            String linkCommand = "ln -s " + uniqueLocalDir + " " + installRootTemp.getPath();
            String[] linkCommandArray = new String[4];
            linkCommandArray[0] = "ln";
            linkCommandArray[1] = "-s";
            linkCommandArray[2] = uniqueLocalDir;
            linkCommandArray[3] = installRootTemp.getPath();

            // Vector returnVector = new Vector();
            returnValue = ExecuteProcess.executeProcessReturnValue(linkCommandArray);

            log = linkCommand + "<br><b>Returns: " + returnValue + "</b><br>";
            LogManager.addCommandsLogfileComment(log);
        }
    }

    private void removeLocalTempDir() {
        InstallData data = InstallData.getInstance();

        if ( data.getLocalTempPath() != null ) {
            File installRootTemp = new File(data.getInstallDir(), "tmp");
            if ( installRootTemp.exists() ) {
                installRootTemp.delete(); // removing the link
                SystemManager.createDirectory(installRootTemp);
            }

            File localTemp = new File(data.getLocalTempPath());
            if ( localTemp.exists() ) {
                SystemManager.removeDirectory(localTemp);
            }
        }
    }

    private Vector getAdminFileContent(boolean relocatable, boolean rdepends) {

        Vector adminFile = new Vector();
        InstallData data = InstallData.getInstance();

        // if ( relocatable ) {
        //     String installDir =  data.getInstallDir();
        //     // installDir = installDir.replace(" ", "\\ ");
        //     String baseDirLine = "basedir=" + installDir;
        //     adminFile.add(baseDirLine);
        // }

        String mailLine = "mail=";
        adminFile.add(mailLine);

        String conflictLine = "conflict=nochange";
        if ( data.isUserInstallation() ) { conflictLine = "conflict=nochange"; }
        adminFile.add(conflictLine);

        String runlevelLine = "runlevel=nocheck";
        adminFile.add(runlevelLine);

        String setuidLine = "setuid=quit";
        if ( data.isUserInstallation() ) { setuidLine = "setuid=nocheck"; }
        adminFile.add(setuidLine);

        String actionLine = "action=nocheck";
        adminFile.add(actionLine);

        String partialLine = "partial=quit";
        if ( data.isUserInstallation() ) { partialLine = "partial=nocheck"; }
        adminFile.add(partialLine);

        String instanceLine = "instance=unique";
        adminFile.add(instanceLine);

        // String idependLine = "idepend=quit";
        String idependLine = "idepend=nocheck";
        // if ( data.isUserInstallation() ) { idependLine = "idepend=nocheck"; }
        adminFile.add(idependLine);

        // String rdependLine = "rdepend=nocheck";
        String rdependLine = "rdepend=quit";
        if ( ! rdepends ) { rdependLine = "rdepend=nocheck"; }
        if ( data.isUserInstallation() ) { rdependLine = "rdepend=nocheck"; }
        adminFile.add(rdependLine);

        String spaceLine = "space=quit";
        if ( data.isUserInstallation() ) { spaceLine = "space=nocheck"; }
        adminFile.add(spaceLine);

        return adminFile;
    }

    private String getMainVersion (String version) {
        String mainVersion = null;

        int pos = version.indexOf(",");
        if ( pos > -1 ) {
            mainVersion = version.substring(0, pos);
        }

        return mainVersion;
    }

    private String getPackageRevision(String version) {
        String revision = null;

        int pos = version.indexOf("=");
        if ( pos > -1 ) {
            revision = version.substring(pos + 1, version.length() );
        }

        return revision;
    }

    private Vector getVectorOfNumbers(String version) {
        Vector numbers = new Vector();
        int pos = -1;
        // getting number from a string in format 2.0.0
        do {
            pos = version.indexOf(".");
            if ( pos > -1 ) {
                String number = version.substring(0, pos);
                version = version.substring(pos + 1, version.length());
                numbers.add(number);
            }
        } while ( pos > -1 );

        numbers.add(version);
        return numbers;
    }

    private int getMinimum(int a, int b) {
        int minimum;

        if ( a < b ) {
             minimum = a;
        } else {
            minimum = b;
        }

        return minimum;
    }

    private String compareVersion(String firstVersion, String secondVersion) {
        // comparing strings with syntax 2.0.0
        String comparison = "bothPackagesAreEqual";
        Vector firstVector = getVectorOfNumbers(firstVersion);
        Vector secondVector = getVectorOfNumbers(secondVersion);

        int firstLength = firstVector.size();
        int secondLength = secondVector.size();
        int minimum = getMinimum(firstLength, secondLength);

        for (int i = 0; i < minimum; i++) {
            String firstS = (String)firstVector.get(i);
            String secondS = (String)secondVector.get(i);

            int first = Integer.parseInt(firstS);
            int second_ = Integer.parseInt(secondS);

            if ( second_ > first ) {
                comparison = "firstPackageIsOlder";
                break;
            } else if ( second_ < first ) {
                comparison = "secondPackageIsOlder";
                break;
            }
        }

        return comparison;
    }

    public void saveModulesLogFile(InstallData data) {
        if ( data.logModuleStates() ) {
            Vector logContent = LogManager.getModulesLogFile();
            File baseDir = new File(data.getInstallDefaultDir(), data.getProductDir());
            File uninstallDir = new File(baseDir, data.getUninstallDirName());
            File modulesLogFile = new File(uninstallDir, "moduleSettingsLog.txt");
            // System.err.println("Saving file: " + modulesLogFile.getPath());
            SystemManager.saveCharFileVector(modulesLogFile.getPath(), logContent);
        }
    }

    public void removeSolarisLockFile() {
        String lockFileName = "/tmp/.ai.pkg.zone.lock-afdb66cf-1dd1-11b2-a049-000d560ddc3e";
        File lockFile = new File(lockFileName);

        if ( lockFile.exists() ) {
            // System.err.println("Found file: " + lockFileName);
            boolean deleted = lockFile.delete();
        }
    }

    public String getSolarisDatabasePath(InstallData data) {
        String databasePath = null;
        databasePath = data.getInstallDir();
        return databasePath;
    }

    public void createAdminFile(boolean relocatable, boolean rdepends) {
        InstallData data = InstallData.getInstance();
        Vector removeFiles = data.getRemoveFiles();
        String adminFileName = "";

        if ( relocatable ) {
            if ( rdepends ) {
                adminFileName = "adminFileReloc";
            } else {
                adminFileName = "adminFileRelocNoDepends";
            }
        } else {
            if ( rdepends ) {
                adminFileName = "adminFileNoReloc";
            } else {
                adminFileName = "adminFileNoRelocNoDepends";
            }
        }

        Vector fileContent = getAdminFileContent(relocatable, rdepends);
        File adminFile = new File(data.getInstallDir(), adminFileName);
        String completeAdminFileName = adminFile.getPath();

        if ( relocatable ) {
            if ( rdepends ) {
                data.setAdminFileNameReloc(completeAdminFileName);
            } else {
                data.setAdminFileNameRelocNoDepends(completeAdminFileName);
            }
        } else {
            if ( rdepends ) {
                data.setAdminFileNameNoReloc(completeAdminFileName);
            } else {
                data.setAdminFileNameNoRelocNoDepends(completeAdminFileName);
            }
        }

        if ( ! adminFile.exists() ) {
            // only saving, if it did not exist
            SystemManager.saveCharFileVector(completeAdminFileName, fileContent);
            // only removing file in aborted installation, if it was created before
            removeFiles.add(completeAdminFileName);
        }

        // File dumpFile = new File(data.getInstallDir(), "dumpFile");
        // String dumpFileName = dumpFile.getPath();
        // SystemManager.dumpFile(adminFileName, dumpFileName);
    }

    // for user installations an environment hashmap is used
    public void setEnvironmentForUserInstall() {
        InstallData data = InstallData.getInstance();
        HashMap env = SystemManager.getEnvironmentHashMap();
        env.put("LD_PRELOAD_32", data.getGetUidPath());
        data.setShellEnvironment(env);
    }

    public String getVersionString(Vector returnVector) {
        String versionString = null;
        String versionLine = null;

        for (int i = 0; i < returnVector.size(); i++) {
            String line = (String) returnVector.get(i);
            int pos = line.indexOf("REV=");
            if ( pos > -1 ) {
                versionLine = line;
                break;
            }
        }

        if ( versionLine != null ) {
            versionLine = versionLine.trim();
            int pos = versionLine.lastIndexOf(" ");
            versionString = versionLine.substring(pos + 1, versionLine.length());
        }

        return versionString;
    }

    public int getInstalledMinor(String version) {

        int minor = 0;

        int pos = version.indexOf(".");
        if ( pos > -1 ) {
            String reduced = version.substring(pos + 1, version.length());

            pos = reduced.indexOf(".");
            if ( pos > -1 ) {
                reduced = reduced.substring(0, pos);
                minor = Integer.parseInt(reduced);
            }
        }

        return minor;
    }

    public boolean comparePackageVersions(String firstPackageVersion, String secondPackageVersion) {
       // Analyzing strings

        boolean firstPackageIsOlder = false;
        String comparison = null;

        String firstPackageMainVersion = getMainVersion(firstPackageVersion);
        String secondPackageMainVersion = getMainVersion(secondPackageVersion);

        if (( firstPackageMainVersion != null ) && ( secondPackageMainVersion != null )) {
            comparison = compareVersion(firstPackageMainVersion, secondPackageMainVersion);
        }

        if ( comparison.equals("firstPackageIsOlder") ) {
            firstPackageIsOlder = true;
        } else if ( comparison.equals("secondPackageIsOlder") ) {
            firstPackageIsOlder = false;
        } else if ( comparison.equals("bothPackagesAreEqual") ) {
            String firstPackageRevision = getPackageRevision(firstPackageVersion);
            String secondPackageRevision = getPackageRevision(secondPackageVersion);

            if (( firstPackageRevision != null ) && ( secondPackageRevision != null )) {
                comparison = compareVersion(firstPackageRevision, secondPackageRevision);
                if ( comparison.equals("firstPackageIsOlder") ) {
                    firstPackageIsOlder = true;
                } else {
                    firstPackageIsOlder = false;
                }
            }
        }

        // If version is equal, the patchlevel has to be analyzed
        // -> pkginfo does not offer this information
        // -> looking into file <root>/opt/var/sadm/pkg/<package>/pkginfo?

        return firstPackageIsOlder;
    }

}
