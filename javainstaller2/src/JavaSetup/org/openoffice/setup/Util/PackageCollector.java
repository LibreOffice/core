/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PackageCollector.java,v $
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

package org.openoffice.setup.Util;

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

}
