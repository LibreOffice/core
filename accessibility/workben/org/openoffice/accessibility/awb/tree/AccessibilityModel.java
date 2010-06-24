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

import javax.swing.SwingUtilities;
import javax.swing.tree.TreeNode;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.DefaultMutableTreeNode;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;
import com.sun.star.awt.XTopWindowListener;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;

/**
 *
 */
public abstract class AccessibilityModel extends javax.swing.tree.DefaultTreeModel {

    protected java.util.Hashtable nodeList;
    protected static DefaultMutableTreeNode disconnectedRootNode =
        new DefaultMutableTreeNode("<not connected>");

    /** Creates a new instance of AccessibilityModel */
    public AccessibilityModel() {
        super(disconnectedRootNode, false);
        nodeList = new java.util.Hashtable();
    }

    /* Convenience method that creates a new Toolkit node from xToolkit
     * and sets as the new root object of the tree.
     */
    public synchronized void setRoot(XExtendedToolkit xToolkit) {
        if (xToolkit != null) {
            try {
                // remove old root node as topwindow listener
                if (getRoot() instanceof ToolkitNode) {
                    ToolkitNode tn = (ToolkitNode) getRoot();
                    if (tn.xToolkit != null) {
                        tn.xToolkit.removeTopWindowListener(tn);
                    }
                }
                nodeList.clear();
                setRoot(new ToolkitNode(xToolkit, this));
                xToolkit.addTopWindowListener((ToolkitNode) getRoot());
            } catch (com.sun.star.uno.RuntimeException e) {
                // FIXME: error message !
            }
        }
    }

    /* Appends the new child to parent's child list */
    public void addNodeInto(MutableTreeNode newChild, MutableTreeNode parent) {
        int index = parent.getChildCount();
        if (newChild != null && newChild.getParent() == parent) {
            index -= 1;
        }
        insertNodeInto(newChild, parent, index);
    }

    /** Adds listener to the listener chain of node */
    public static void addEventListener(TreeNode node, XAccessibleEventListener listener) {
        if (node instanceof AccessibilityNode) {
            ((AccessibilityNode) node).addEventListener(listener);
        }
    }

    /** Removes listener from the listener chain of node */
    public static void removeEventListener(TreeNode node, XAccessibleEventListener listener) {
        if (node instanceof AccessibilityNode) {
            ((AccessibilityNode) node).removeEventListener(listener);
        }
    }

    protected abstract AccessibilityNode createWindowNode(XAccessible xAccessible,
            XAccessibleContext xAccessibleContext);
    protected abstract AccessibilityNode createNode(XAccessible xAccessible);

    /** Adds xAccessible,node to the internal hashtable */
    public AccessibilityNode putNode(XAccessible xAccessible, AccessibilityNode node) {
        if (xAccessible != null) {
            String oid = UnoRuntime.generateOid(xAccessible);
            java.lang.ref.WeakReference ref = (java.lang.ref.WeakReference)
                nodeList.put(oid, new java.lang.ref.WeakReference(node));
            if (ref != null) {
                return (AccessibilityNode) ref.get();
            }
        }
        return null;
    }

    /** Returns the AccessibilityNode for xAccessible */
    public AccessibilityNode findNode(XAccessible xAccessible) {
        if (xAccessible != null) {
            String oid = UnoRuntime.generateOid(xAccessible);
            java.lang.ref.WeakReference ref =
                (java.lang.ref.WeakReference) nodeList.get(oid);
            if (ref != null) {
                return (AccessibilityNode) ref.get();
            }
        }
        return null;
    }

    /** Removes the AccessibilityNode for xAccessible from the internal hashtable */
    public AccessibilityNode removeNode(XAccessible xAccessible) {
        if (xAccessible != null) {
            String oid = UnoRuntime.generateOid(xAccessible);
            java.lang.ref.WeakReference ref =
                (java.lang.ref.WeakReference) nodeList.remove(oid);
            if (ref != null) {
                return (AccessibilityNode) ref.get();
            }
        }
        return null;
    }

    public AccessibilityNode removeNode(Object o) {
        if (o instanceof XAccessible) {
            return removeNode((XAccessible) o);
        }
        return null;
    }
}
