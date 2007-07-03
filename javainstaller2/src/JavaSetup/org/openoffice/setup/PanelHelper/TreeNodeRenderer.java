/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TreeNodeRenderer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 11:58:37 $
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
