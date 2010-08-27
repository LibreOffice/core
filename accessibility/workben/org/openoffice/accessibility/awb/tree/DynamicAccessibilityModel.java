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

import javax.swing.tree.TreeNode;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeWillExpandListener;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

/**
 *
 */
public class DynamicAccessibilityModel extends AccessibilityModel implements TreeExpansionListener, TreeWillExpandListener {

    /* Creates a AccessibilityNode object for a window */
    protected AccessibilityNode createWindowNode(XAccessible xAccessible,
            XAccessibleContext xAccessibleContext) {
        if (xAccessible != null) {
            // Some objects inherit XAccessible, but should not appear in
            // the hierarchy as toplevels (like sub-menus), so they don't
            // return an accessible context.
            if (xAccessibleContext != null) {
                AccessibilityNode node = new AccessibilityNode(this);
                node.setUserObject(xAccessible);
                node.setAccessibleContext(xAccessibleContext);
                putNode(xAccessible, node);
                return node;
            }
        }
        return null;
    }

    /* Creates a DynamicAccessibilityNode object */
    protected AccessibilityNode createNode(XAccessible xAccessible) {
        if (xAccessible != null) {
            try {
                // Some objects inherit XAccessible, but should not appear in
                // the hierarchy as toplevels (like sub-menus), so they don't
                // return an accessible context.
                XAccessibleContext xAccessibleContext = xAccessible.getAccessibleContext();
                if (xAccessibleContext != null) {
                    AccessibilityNode node = new DynamicAccessibilityNode(this);
                    node.setUserObject(xAccessible);
                    node.setAccessibleContext(xAccessibleContext);
                    putNode(xAccessible, node);
                    return node;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
        return null;
    }

    public void treeCollapsed(javax.swing.event.TreeExpansionEvent treeExpansionEvent) {
        TreeNode node = (TreeNode) treeExpansionEvent.getPath().getLastPathComponent();
        if (node instanceof DynamicAccessibilityNode) {
            DynamicAccessibilityNode dynode = (DynamicAccessibilityNode) node;
            dynode.clear();
        }
    }

    public void treeExpanded(javax.swing.event.TreeExpansionEvent treeExpansionEvent) {
        TreeNode node = (TreeNode) treeExpansionEvent.getPath().getLastPathComponent();
        if (node instanceof AccessibilityNode) {
            // Calling oneway methods from an UNO thread may cause
            // deadlocks, so adding the listeners here.
            for (java.util.Enumeration e = node.children(); e.hasMoreElements(); ) {
                ((AccessibilityNode) e.nextElement()).setAttached(true);
            }
        }
    }

    public void treeWillCollapse(javax.swing.event.TreeExpansionEvent treeExpansionEvent)
            throws javax.swing.tree.ExpandVetoException {
        TreeNode node = (TreeNode) treeExpansionEvent.getPath().getLastPathComponent();
        if (node instanceof AccessibilityNode) {
            // Calling oneway methods from an UNO thread may cause
            // deadlocks, so adding the listeners here.
            for (java.util.Enumeration e = node.children(); e.hasMoreElements(); ) {
                ((AccessibilityNode) e.nextElement()).setAttached(false);
            }
        }
    }

    public void treeWillExpand(javax.swing.event.TreeExpansionEvent treeExpansionEvent)
            throws javax.swing.tree.ExpandVetoException {
        TreeNode node = (TreeNode) treeExpansionEvent.getPath().getLastPathComponent();
        if (node instanceof DynamicAccessibilityNode) {
            DynamicAccessibilityNode dynode = (DynamicAccessibilityNode) node;
            dynode.populate();
        }
    }
}
