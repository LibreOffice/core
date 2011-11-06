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
