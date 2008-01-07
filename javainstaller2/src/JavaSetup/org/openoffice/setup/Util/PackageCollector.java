/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PackageCollector.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 12:34:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
