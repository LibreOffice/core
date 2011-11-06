/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
