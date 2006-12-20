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
            allPackages.add(packageData);
        }

        // also allowing packages at nodes!
        if (( ! packageData.isLeaf() ) &&
                ( packageData.getPackageName() != null ) &&
                ( ! packageData.getPackageName().equals("")) &&
                ( packageData.getSelectionState() == packageData.REMOVE )) {
            allPackages.add(packageData);
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            collectUninstallPackages(child, allPackages);
        }
    }

}
