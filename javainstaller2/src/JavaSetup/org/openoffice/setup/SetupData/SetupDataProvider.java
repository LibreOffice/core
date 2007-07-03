/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SetupDataProvider.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:59:25 $
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

package org.openoffice.setup.SetupData;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 *
 * @author Christof Pintaske
 */
public class SetupDataProvider {

    static private PackageDescription packageData;
    static private ProductDescription productData;

    private SetupDataProvider() {
    }

    public static PackageDescription getPackageDescription() {
        return packageData;
    }

    public static ProductDescription getProductDescription() {
        return productData;
    }

    public static String replaceMacros(String s) {
        return productData.replaceMacros(s);
    }

    public static String getString(String key) {
        return productData.get(key);
    }

    public static void setNewMacro(String key, String value) {
        productData.setNewMacro(key, value);
    }

    public static void dumpMacros() {
        productData.dumpMacros();
    }

    private static DefaultMutableTreeNode createTree(PackageDescription data, InstallData installData) {
        DefaultMutableTreeNode node = new DefaultMutableTreeNode();

        node.setUserObject(new DisplayPackageDescription(data));

        for (Enumeration e = data.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();

            // Do not display modules with "showinuserinstall" set to false in xpd file
            // if this is a user installation.
            if (( installData.isUserInstallation() ) && ( ! child.showInUserInstall() )) {
                child.setIsHidden(true);
                child.setSelectionState(PackageDescription.IGNORE);
            }

            // Only add modules, if they have not display type="hidden" in xpd file
            if (!child.isHidden()) {
                node.add(createTree(child, installData));
            }
        }

        return node;
    }

    public static DefaultMutableTreeNode createTree() {
        InstallData installData = InstallData.getInstance();
        return createTree(getPackageDescription(), installData);
    }

    static {
        XMLPackageDescription rawData = new XMLPackageDescription();
        rawData.read();
        // rawData.dump();
        packageData = new PackageDescription(rawData);
        productData = new ProductDescription(rawData);
    }
}
