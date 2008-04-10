/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TreeNodeRenderer.java,v $
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

package org.openoffice.setup.PanelHelper;

import org.openoffice.setup.SetupData.DisplayPackageDescription;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.ResourceManager;
import java.awt.Component;
import javax.swing.ImageIcon;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;

public class TreeNodeRenderer extends DefaultTreeCellRenderer {

    ImageIcon    InstallIcon;
    ImageIcon    InstalledIcon;
    ImageIcon    DontInstallIcon;
    ImageIcon    DontKnowIcon;
    ImageIcon    RemoveIcon;

    public TreeNodeRenderer() {
        InstallIcon     = ResourceManager.getIcon("Icon_Install");
        InstalledIcon   = ResourceManager.getIcon("Icon_Installed");
        DontInstallIcon = ResourceManager.getIcon("Icon_DontInstall");
        DontKnowIcon    = ResourceManager.getIcon("Icon_DontKnow");
        RemoveIcon      = ResourceManager.getIcon("Icon_Remove");
    }

    public Component getTreeCellRendererComponent(
                        JTree tree, Object value, boolean sel, boolean expanded,
                        boolean leaf, int row, boolean hasFocus)    {

        super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);

        DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;
        Object nodeObject = node.getUserObject();

        if (DisplayPackageDescription.is(nodeObject)) {
            DisplayPackageDescription nodeInfo = (DisplayPackageDescription)nodeObject;

            switch (nodeInfo.getState()) {
                case PackageDescription.INSTALL:      setIcon(InstallIcon);      break;
                case PackageDescription.DONT_REMOVE:  setIcon(InstallIcon);      break;
                case PackageDescription.IGNORE:       setIcon(InstalledIcon);    break;
                case PackageDescription.INSTALL_SOME: setIcon(DontKnowIcon);     break;
                case PackageDescription.REMOVE_SOME:  setIcon(DontKnowIcon);     break;
                case PackageDescription.DONT_INSTALL: setIcon(DontInstallIcon);  break;
                case PackageDescription.REMOVE:       setIcon(RemoveIcon);       break;
                default: setIcon(InstalledIcon); break;
            }
        }

        if (sel) {
            setBackground(super.getBackgroundSelectionColor());
            setForeground(textSelectionColor);
        } else {
            setBackground(super.getBackgroundNonSelectionColor());
            setForeground(textSelectionColor);
        }

        return this;
    }
}
