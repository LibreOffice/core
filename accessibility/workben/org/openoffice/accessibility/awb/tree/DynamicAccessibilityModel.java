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
