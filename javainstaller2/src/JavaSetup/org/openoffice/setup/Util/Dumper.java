/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Dumper.java,v $
 * $Revision: 1.3 $
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

import org.openoffice.setup.InstallData;
import org.openoffice.setup.SetupData.PackageDescription;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;
import java.util.Vector;


public class Dumper {

    private Dumper() {
    }

    static public void dumpPackageSettings(PackageDescription packageData) {

        if ( packageData.isLeaf() ) {
            System.out.println("Name: " + packageData.getName() +
                               " State: " + packageData.getSelectionState()  +
                               " " + packageData.getPackageName());
        } else {
            System.out.println("Nod-Name: " + packageData.getName() +
                               " State: " + packageData.getSelectionState());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpPackageSettings(child);
        }

    }

    static public void dumpModuleStates(PackageDescription packageData) {

        System.err.println("Name: " + packageData.getName() +
                           " State: " + packageData.getSelectionState() +
                           " " + packageData.getPackageName());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpModuleStates(child);
        }
    }

    static private String getStateString(int state) {
        String stateString = null;

        if ( state == 0 ) {
            stateString = "DONT_INSTALL";
        } else if ( state == 1 ) {
            stateString = "INSTALL";
        } else if ( state == 2 ) {
            stateString = "INSTALL_SOME";
        } else if ( state == 3 ) {
            stateString = "REMOVE";
        } else if ( state == 4 ) {
            stateString = "DONT_REMOVE";
        } else if ( state == 5 ) {
            stateString = "REMOVE_SOME";
        } else if ( state == 6 ) {
            stateString = "IGNORE";
        } else if ( state == 7 ) {
            stateString = "DONT_KNOW";
        } else {
            stateString = null;
        }

        return stateString;
    }

    static private void logModuleStatesHelper(PackageDescription packageData) {
        int state = packageData.getSelectionState();
        String stateStr = getStateString(state);

        LogManager.addModulesLogfileComment("Name: " + packageData.getName() +
                           " State: " + stateStr +
                           " " + packageData.getPackageName());

        // System.err.println("Name: " + packageData.getName() +
        //                    " State: " + stateStr +
        //                    " " + packageData.getPackageName());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            logModuleStatesHelper(child);
        }
    }

    static public void logModuleStates(PackageDescription packageData, String text) {
        LogManager.setModulesLogFileHeaderLine(text);
        logModuleStatesHelper(packageData);
    }

    static public void logPackagesToInstall(Vector packages, String text) {
        PackageDescription packageData = null;
        LogManager.setModulesLogFileHeaderLine(text);
        for (int i = 0; i < packages.size(); i++) {
            packageData = (PackageDescription)packages.get(i);
            LogManager.addModulesLogfileComment("Name: " + packageData.getName() +
                                                " " + packageData.getPackageName());
        }
    }

    static public void dumpInstallPackages(PackageDescription packageData) {

        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.INSTALL )) {
            System.out.println("Now installing: " + packageData.getPackageName());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpInstallPackages(child);
        }
    }

    static public void dumpUninstallPackages(PackageDescription packageData) {

        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.REMOVE )) {
            System.out.println("Now uninstalling: " + packageData.getPackageName());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpUninstallPackages(child);
        }
    }

    static public void dumpAllRpmInfo(PackageDescription packageData) {

        if (( packageData.getPackageName() != null ) && ( ! packageData.getPackageName().equals(""))) {
            if ( packageData.pkgExists() ) {
                System.err.println("RPM data: " + packageData.getPkgRealName() + " : " +
                                                  packageData.getPkgVersion() + " : " +
                                                  packageData.getPkgRelease() + " : " +
                                                  packageData.getPackageName() );
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpAllRpmInfo(child);
        }

    }

    static public void dumpNewInstallData() {
        InstallData data = InstallData.getInstance();
        System.err.println("PackagePath: " + data.getPackagePath());
        System.err.println("AdminFileReloc: " + data.getAdminFileNameReloc());
        System.err.println("AdminFileNoReloc: " + data.getAdminFileNameNoReloc());
        System.err.println("DatabasePath: " + data.getDatabasePath());
        System.err.println("InstallDir: " + data.getInstallDir());
        System.err.println("InstallRoot: " + data.getInstallRoot());
        System.err.println("Original privileges: " + data.getStoredInstallationPrivileges());
        System.err.println("getuid.so File: " + data.getGetUidPath());
    }

    static public void dumpAllProperties() {
        Properties properties = System.getProperties();

        int size = properties.size();
        Iterator m = properties.entrySet().iterator();
        int counter = 0;

        while ( m.hasNext() ) {
            Map.Entry entry = (Map.Entry) m.next();
            String env = entry.getKey() + "=" + entry.getValue();
            System.err.println(env);
        }
    }

}
