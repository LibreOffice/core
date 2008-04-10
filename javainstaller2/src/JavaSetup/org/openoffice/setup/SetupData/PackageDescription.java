/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PackageDescription.java,v $
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

package org.openoffice.setup.SetupData;

import org.openoffice.setup.Util.Parser;
import java.io.Reader;
import java.io.StringReader;
import java.util.Collections;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.Locale;
import java.util.Vector;
import javax.swing.tree.TreeNode;

/**
 *
 * @author Ingo Schmidt
 */
public class PackageDescription implements TreeNode {

    public final static int NOTA_UNIT= 0;
    public final static int RPM_UNIT = 1;
    public final static int PKG_UNIT = 2;
    /* public final static int MSI_UNIT = 3;
     * public final static int TAR_UNIT = 4;
     * public final static int TGZ_UNIT = 5;
     */

    public final static int DONT_INSTALL      = 0;
    public final static int INSTALL           = 1;
    public final static int INSTALL_SOME      = 2;
    public final static int REMOVE            = 3;
    public final static int DONT_REMOVE       = 4;
    public final static int REMOVE_SOME       = 5;
    public final static int IGNORE            = 6;
    public final static int DONT_KNOW         = 7;

    /* hierarchy information */
    private Vector children           = null;
    private PackageDescription parent = null;

    /* did the user select this package */
    private int     usrSelectionState = DONT_KNOW;

    /* display information */
    private String  dpyName            = "";
    private String  dpyDescription     = "";
    private String  checkSolaris       = "";
    private int     instSize    = 0;
    private int     dpySortKey  = 0;
    private boolean isDefault   = true;
    private boolean isHidden    = false;
    private boolean showInUserInstall = true;
    private boolean isOptional  = true;
    private boolean dontUninstall = false;
    private boolean allChildrenHidden = false;

    /* package information */
    private String  pkgMD5Sum   = "";
    private String  pkgFileName = "";
    private String  pkgFullName = null;
    private String  pkgVersion = null;
    private String  pkgRelease = null;
    private String  pkgRealName = null;
    private String  pkgSubdir = null;
    private String  pkgLanguage = null;
    private int     pkgSize     = 0;
    private int     pkgType     = NOTA_UNIT;
    private boolean pkgExists = true;  // must be default, especially for uninstallation
    private boolean isRelocatable = true;
    private boolean isUpdatePackage = false;
    private boolean showMultiLingualOnly = false;
    private boolean isApplicationPackage = false;
    private boolean isJavaPackage = false;
    private boolean isNewInstalled = false;
    private boolean wasAlreadyInstalled = false;

    /* Saving the default selection state. This is necessary, if the user chooses
     * the custom installation type, makes changes, and then changes into
     * the typical installation set. Then all user settings have to be removed again.
     * On the other hand, if the user then changes to the custom installation type
     * again, he probably wants to see the settings he has done before.
     */

    private int     typicalSelectionState = DONT_KNOW;  // Saving settings for typical installation
    private int     customSelectionState = DONT_KNOW;   // Saving settings for custom installation
    private int     startSelectionState = DONT_KNOW;    // Saving settings at start of installation

    /**
     * construct only with package information to wrap
     */
    protected PackageDescription(XMLPackageDescription p) {
        this(p, (PackageDescription)null);
    }

    private PackageDescription(XMLPackageDescription descriptionData, PackageDescription parent) {

        this.parent = parent;
        children = new Vector();
        parse(descriptionData);

        /* sort according to display sort key */
        Collections.sort(children, new PackageComparator());
     }

    /**
     * retrieve information about the package properties
     */

    public String getName() {
        return dpyName;
    }

    public String getDescription() {
        return dpyDescription;
    }

    public String getCheckSolaris() {
        return checkSolaris;
    }

    public int getSize() {
        return pkgSize;
    }

    public void setSize(int size) {
        pkgSize = size;
    }

    // public int getAccumulatedSize() {
    //     int size = getSize();
    //
    //     for (Enumeration e = this.children(); e.hasMoreElements(); ) {
    //         // Should only be accumulated for selected modules
    //         PackageDescription data = (PackageDescription) e.nextElement();
    //         size += data.getSize();
    //     }
    //
    //     return size;
    // }

    public boolean isOptional() {
        return isOptional;
    }

    public boolean isHidden() {
        return isHidden;
    }

    public boolean showInUserInstall() {
        return showInUserInstall;
    }

    public boolean dontUninstall() {
        return dontUninstall;
    }

    public boolean isUpdatePackage() {
        return isUpdatePackage;
    }

    public boolean showMultiLingualOnly() {
        return showMultiLingualOnly;
    }

    public boolean isApplicationPackage() {
        return isApplicationPackage;
    }

    public boolean isJavaPackage() {
        return isJavaPackage;
    }

    public void setIsNewInstalled(boolean installed) {
        isNewInstalled = installed;
    }

    public boolean isNewInstalled() {
        return isNewInstalled;
    }

    public void setWasAlreadyInstalled(boolean installed) {
        wasAlreadyInstalled = installed;
    }

    public boolean wasAlreadyInstalled() {
        return wasAlreadyInstalled;
    }

    public boolean isDefault() {
        return isDefault;
    }

    public String getMD5() {
        return pkgMD5Sum;
    }

    public boolean isRelocatable() {
        return isRelocatable;
    }

    public String getPackageName() {
        return pkgFileName;
    }

    public String getFullPackageName() {
        return pkgFullName;
    }

    public void setFullPackageName(String fullPackageName) {
        pkgFullName = fullPackageName;
    }

    public int getSelectionState() {
        return usrSelectionState;
    }

    public void setSelectionState(int state) {
        usrSelectionState = state;
    }

    public int getCustomSelectionState() {
        return customSelectionState;
    }

    public void setCustomSelectionState(int state) {
        customSelectionState = state;
    }

     public int getTypicalSelectionState() {
        return typicalSelectionState;
    }

    public void setTypicalSelectionState(int state) {
        typicalSelectionState = state;
    }

    public int getStartSelectionState() {
        return startSelectionState;
    }

    public void setStartSelectionState(int state) {
        startSelectionState = state;
    }

    public boolean isAllChildrenHidden() {
        return allChildrenHidden;
    }

    public void setAllChildrenHidden(boolean hidden) {
        allChildrenHidden = hidden;
    }

    public void setIsHidden(boolean hidden) {
        isHidden = hidden;
    }

    public boolean pkgExists() {
        return pkgExists;
    }

    public void setPkgExists(boolean exists) {
        pkgExists = exists;
    }

    public String getPkgVersion() {
        return pkgVersion;
    }

    public void setPkgVersion(String version) {
        pkgVersion = version;
    }

    public String getPkgRelease() {
        return pkgRelease;
    }

    public void setPkgRelease(String release) {
        pkgRelease = release;
    }

    public String getPkgRealName() {
        return pkgRealName;
    }

    public void setPkgRealName(String realName) {
        pkgRealName = realName;
    }

    public String getPkgSubdir() {
        return pkgSubdir;
    }

    public void setPkgSubdir(String subdir) {
        pkgSubdir = subdir;
    }

    public String getPkgLanguage() {
        return pkgLanguage;
    }

    public void setPkgLanguage(String language) {
        pkgLanguage = language;
    }

    /**
     * extract the name and the description according to the locale
     */
    private String getLocalizedValue(XMLPackageDescription packageData, String section, Locale l) {
        String localizedValue = "";

        String countryString  = l.getCountry();
        String languageString = l.getLanguage();
        String localeString   = languageString + "_" + countryString;

        XMLPackageDescription subPackage = packageData.getElement(section, "lang", localeString);
        if (subPackage == null) {
            subPackage = packageData.getElement(section, "lang", languageString);
            if (subPackage == null) {
                subPackage = packageData.getElement(section, "lang", "en_US");
            }
        }
        if (subPackage != null) {
            localizedValue = subPackage.getValue();
        }

        return localizedValue;
    }

    /**
     * parse the wrapped package description
     */

    private void parse(XMLPackageDescription data) {

        XMLPackageDescription section;
        XMLPackageDescription subSection;

        /* information about how to display the node */
        section = data.getElement("display");
        if (section != null) {
            /* display types: hidden, visible */
            String displayType = section.getAttribute("type");
            if (displayType != null) {
                isHidden = displayType.equals("hidden");
            }

            /* name and description according to the current locale */
            Locale locale = Locale.getDefault();
            dpyName        = getLocalizedValue(section, "name",        locale);
            dpyDescription = getLocalizedValue(section, "description", locale);

            subSection = section.getElement("sortkey");
            if (subSection != null) {
                String sort = subSection.getValue();
                dpySortKey = Integer.parseInt(sort);
            }

            subSection = section.getElement("default");
            if (subSection != null) {
                String defaultValue = subSection.getValue();
                isDefault = Parser.parseBoolean(defaultValue);
                // isDefault = Boolean.parseBoolean(defaultValue);
            }

            subSection = section.getElement("showinuserinstall");
            if (subSection != null) {
                String showInUserInstallValue = subSection.getValue();
                showInUserInstall = Parser.parseBoolean(showInUserInstallValue);
                // showInUserInstall = Boolean.parseBoolean(showInUserInstallValue);
            }

            subSection = section.getElement("dontuninstall");
            if (subSection != null) {
                String dontUninstallValue = subSection.getValue();
                dontUninstall = Parser.parseBoolean(dontUninstallValue);
                // dontUninstall = Boolean.parseBoolean(dontUninstallValue);
            }

            subSection = section.getElement("checksolaris");
            if (subSection != null) {
                checkSolaris = subSection.getValue();
            }

            subSection = section.getElement("isupdatepackage");
            if (subSection != null) {
                String isUpdatePackageValue = subSection.getValue();
                isUpdatePackage = Parser.parseBoolean(isUpdatePackageValue);
                // isUpdatePackage = Boolean.parseBoolean(isUpdatePackageValue);
            }

            subSection = section.getElement("showmultilingualonly");
            if (subSection != null) {
                String showMultiLingualOnlyValue = subSection.getValue();
                showMultiLingualOnly = Parser.parseBoolean(showMultiLingualOnlyValue);
            }

            subSection = section.getElement("applicationmodule");
            if (subSection != null) {
                String isApplicationPackageValue = subSection.getValue();
                isApplicationPackage = Parser.parseBoolean(isApplicationPackageValue);
            }

            subSection = section.getElement("isjavapackage");
            if (subSection != null) {
                String isJavaPackageValue = subSection.getValue();
                isJavaPackage = Parser.parseBoolean(isJavaPackageValue);
                // isJavaPackage = Boolean.parseBoolean(isJavaPackageValue);
            }

        }

        /* query information about the physical (rpm/pkg/msi...) package itself */
        section = data.getElement("installunit");
        if (section != null) {
            String pkgTypeName = section.getAttribute("type");
            if (pkgTypeName != null) {
                if (pkgTypeName.equals("rpm")) {
                    pkgType = RPM_UNIT;
                } else if (pkgTypeName.equals("pkg")) {
                    pkgType = PKG_UNIT;
                }
            }

            subSection = section.getElement("size");
            if (subSection != null) {
                String sz = subSection.getValue();
                pkgSize = Integer.parseInt(sz);
            }
            subSection = section.getElement("md5");
            if (subSection != null) {
                pkgMD5Sum = subSection.getValue();
            }
            subSection = section.getElement("name");
            if (subSection != null) {
                pkgFileName = subSection.getValue();
            }
            subSection = section.getElement("fullpkgname");
            if (subSection != null) {
                pkgFullName = subSection.getValue();
            }
            subSection = section.getElement("pkgversion");
            if (subSection != null) {
                pkgVersion = subSection.getValue();
            }
            subSection = section.getElement("subdir");
            if (subSection != null) {
                pkgSubdir = subSection.getValue();
            }
            subSection = section.getElement("relocatable");
            if (subSection != null) {
                String relocatableValue = subSection.getValue();
                isRelocatable = Parser.parseBoolean(relocatableValue);
                // isRelocatable = Boolean.parseBoolean(relocatableValue);
            }
            subSection = section.getElement("solarislanguage");
            if (subSection != null) {
                pkgLanguage = subSection.getValue();
            }

        }

        /* line up the subpackages */
        for (Enumeration enumPackages = data.elements(); enumPackages.hasMoreElements(); ) {
            XMLPackageDescription p = (XMLPackageDescription) enumPackages.nextElement();
            if (p.getKey().equals("package")) {
                children.add(new PackageDescription(p, this));
            }
        }
    }

    /**
     * sort according to the display sortkey
     */

    private class PackageComparator implements Comparator {
        public int compare(Object w1, Object w2) {
            return ((PackageDescription) w1).dpySortKey - ((PackageDescription) w2).dpySortKey;
        }
    }

    /**
     * implement a TreeNode interface for convenient travelling through the data
     */

    private class PackageEnumeration implements Enumeration {

        Enumeration e;

        protected PackageEnumeration() {
            e = children.elements();
        }
        public boolean hasMoreElements() {
            return e.hasMoreElements();
        }
        public Object nextElement() {
            return e.nextElement();
        }
    }

    /**
     * TreeNode interface
     */
    public Enumeration children() {
        return new PackageEnumeration();
    }

    public boolean getAllowsChildren() {
        return true;
    }

//    public PackageDescription getChildAt(int childIndex) {
//        return (PackageDescription) children.elementAt(childIndex);
//    }

    public TreeNode getChildAt(int childIndex) {
        return (TreeNode)children.elementAt(childIndex);
    }

    public int getChildCount() {
        return children.size();
    }

    public int getIndex(TreeNode node) {
        return children.indexOf(node);
    }

//    public PackageDescription getParent() {
//        return parent;
//    }

    public TreeNode getParent() {
        return (TreeNode)parent;
    }

    public boolean isLeaf() {
        return children.size() == 0;
    }

}
