/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.accessibility.awb.tree;

import org.openoffice.accessibility.misc.NameProvider;

import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;

import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

/**
 *
 */
public class AccessibilityTree extends javax.swing.JTree {

    /** Creates a new instance of AccessibilityTree */
    public AccessibilityTree(javax.swing.tree.TreeModel model) {
        super(model);
        // always show handles to indicate expandable / collapsable
        showsRootHandles = true;
    }

    public void setToolkit(XExtendedToolkit xToolkit) {
        AccessibilityModel model = (AccessibilityModel) getModel();
        if (model != null) {
            // hide the root node when connected
            setRootVisible(xToolkit == null);
            // update the root node
            model.setRoot(xToolkit);
            model.reload();
        }
    }

     public String convertValueToText(Object value, boolean selected,
            boolean expanded, boolean leaf, int row, boolean hasFocus) {

        if (value instanceof DefaultMutableTreeNode) {
            DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;

            Object userObject = node.getUserObject();
            if (userObject != null && userObject instanceof XAccessible) {
                XAccessible xAccessible = (XAccessible) userObject;
                try {
                    XAccessibleContext xAC = xAccessible.getAccessibleContext();
                    if (xAC != null) {
                        String name = xAC.getAccessibleName();
                        if (name.length() == 0) {
                            name = new String ("<no name>");
                        }
                        value = name + " / " + NameProvider.getRoleName(xAC.getAccessibleRole());
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                    value = "???";
                }
            }
        }

        return super.convertValueToText(value, selected, expanded, leaf, row, hasFocus);
     }

}
