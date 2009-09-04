/*************************************************************************
 *
 *  $RCSfile: AccessibilityModel.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2003/09/19 09:21:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
