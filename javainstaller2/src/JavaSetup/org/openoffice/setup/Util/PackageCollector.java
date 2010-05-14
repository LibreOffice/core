/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PackageCollector.java,v $
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

package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.SetupData.PackageDescription;
import java.util.Enumeration;
import java.util.Vector;

public class PackageCollector {

    private PackageCollector() {
    }

    static public void collectInstallPackages(PackageDescription packageData, Vector allPackages) {

        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.INSTALL )) {
            allPackages.add(packageData);
            // System.err.println("Adding to collector 1: " + packageData.getPackageName());
        }

        // also allowing packages at nodes!
        if (( ! packageData.isLeaf() ) &&
                ( packageData.getPackageName() != null ) &&
                ( ! packageData.getPackageName().equals("")) &&
                (( packageData.getSelectionState() == packageData.INSTALL ) ||
                ( packageData.getSelectionState() == packageData.INSTALL_SOME ))) {
            allPackages.add(packageData);
            // System.err.println("Adding to collector 2: " + packageData.getPackageName());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            collectInstallPackages(child, allPackages);
        }

    }

    static public void collectUninstallPackages(PackageDescription packageData, Vector allPackages) {
        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.REMOVE )) {
            allPackages.add(0, packageData);
        }

        // also allowing packages at nodes!
        if (( ! packageData.isLeaf() ) &&
                ( packageData.getPackageName() != null ) &&
                ( ! packageData.getPackageName().equals("")) &&
                ( packageData.getSelectionState() == packageData.REMOVE )) {
            allPackages.add(0, packageData);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            collectUninstallPackages(child, allPackages);
        }
    }

    // Special handling for packages, that change their name, and therefore need to be uninstalled

    // static public void findOldPackages( InstallData installData ) {
    //
    //    String basis = "ooobasis3";
    //    if ( installData.getOSType().equalsIgnoreCase("Linux") ) { basis = basis + "."; }
    //    String search = basis + installData.getProductMinor();

    //    Vector allPackages = installData.getInstallPackages();
    //    Vector oldPackages = new Vector();

    //    for (int i = 0; i < allPackages.size(); i++) {
    //        PackageDescription packageData = (PackageDescription) allPackages.get(i);
    //        int pos = packageData.getPackageName().indexOf(search);

    //        if ( pos > -1 ) {
    //            String substring = packageData.getPackageName().substring(pos, pos + 1);
    //            for (int j = 0; j < installData.getProductMinor(); j++) {
    //                String replace = basis + j;
    //                // Creating new package for removal, very simple PackageDescription
    //                PackageDescription localPackage = new PackageDescription();
    //                localPackage.setUninstallCanFail(true);
    //                localPackage.setIsRelocatable(packageData.isRelocatable());
    //                String localName = packageData.getPackageName();
    //                localName = localName.replace(search, replace);
    //                localPackage.setPackageName(localName);

    //                if ( ( packageData.getPkgRealName() != null ) && ( ! packageData.getPkgRealName().equals("") )) {
    //                    localName = packageData.getPkgRealName();
    //                    localName = localName.replace(search, replace);
    //                    localPackage.setPkgRealName(localName);
    //                }

    //                if (( packageData.getName() != null ) && ( ! packageData.getName().equals("") )) {
    //                    localName = packageData.getName();
    //                    localName = localName.replace(search, replace);
    //                    localPackage.setName(localName);
    //                }

    //                oldPackages.add(localPackage);
    //            }
    //        }
    //    }

    //    // reverse order for uninstallation
    //    int number = oldPackages.size();
    //    for (int i = 0; i < number; i++) {
    //        if ( i > 0 ) {
    //            PackageDescription oldPackageData = (PackageDescription) oldPackages.remove(i);
    //            oldPackages.add(0,oldPackageData);
    //        }
    //    }

    //    installData.setOldPackages(oldPackages);
    // }

    static public void sortPackages(Vector allPackages, Vector sortedPackages, String mode) {
        for (int i = 0; i < allPackages.size(); i++) {
            boolean integrated = false;
            PackageDescription packageData = (PackageDescription) allPackages.get(i);

            if ( i == 0 ) {
                sortedPackages.add(packageData);
                integrated = true;
            } else {
                int position = packageData.getOrder();
                for (int j = 0; j < sortedPackages.size(); j++) {
                    PackageDescription sortedPackageData = (PackageDescription) sortedPackages.get(j);
                    int compare = sortedPackageData.getOrder();

                    if ( position < compare ) {
                        sortedPackages.add(j, packageData);
                        integrated = true;
                        break;
                    }
                }

                // no break used -> adding at the end
                if ( ! integrated ) {
                    sortedPackages.add(packageData);
                }
            }
        }

        // reverse order for uninstallation
        if ( mode.equalsIgnoreCase("uninstall")) {
            int number = sortedPackages.size();
            for (int i = 0; i < number; i++) {
                if ( i > 0 ) {
                    PackageDescription sortPackageData = (PackageDescription) sortedPackages.remove(i);
                    sortedPackages.add(0,sortPackageData);
                }
            }
        }
    }

}
