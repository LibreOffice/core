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

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XTopWindow;

import com.sun.star.uno.UnoRuntime;

import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 *
 */
public class ToolkitNode extends DefaultMutableTreeNode
        implements com.sun.star.awt.XTopWindowListener {

    protected XExtendedToolkit xToolkit;

    private AccessibilityModel treeModel;

    /** Creates a new instance of TopWindowListener */
    public ToolkitNode(XExtendedToolkit xToolkit, AccessibilityModel treeModel) {
        super("<connected>");
        this.xToolkit = xToolkit;
        this.treeModel = treeModel;

        // Initially fill the child list
        try {
            for (int i=0,j=xToolkit.getTopWindowCount(); i<j; i++) {
                XTopWindow xTopWindow = xToolkit.getTopWindow(i);
                if (xTopWindow != null) {
                    AccessibilityNode an = getTopWindowNode(xTopWindow);
                    if (an != null) {
                        add(an);
                        // Calling oneway methods from an UNO thread may cause
                        // deadlocks, so adding the listeners here.
                        an.setAttached(true);
                    }
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            // This should never happen since we properly check the count
            // before - anyway returning what we got so far.
        }
    }

    /** Returns an AccessibilityNode if xAccessible has a valid toplevel */
    private AccessibilityNode getTopWindowNode(XAccessible xAccessible) {
        XAccessibleContext xAC = xAccessible.getAccessibleContext();
        if (xAC != null) {
            short role = xAC.getAccessibleRole();
            if ((role == AccessibleRole.FRAME) || (role == AccessibleRole.DIALOG) || (role == AccessibleRole.WINDOW)) {
                return treeModel.createWindowNode(xAccessible, xAC);
            }
        }
        return null;
    }

    /** Returns an AccessibilityNode if xAccessible has a valid toplevel */
    private AccessibilityNode getTopWindowNode(XAccessible xAccessible, XAccessibleContext xAC) {
        if (xAC != null) {
            short role = xAC.getAccessibleRole();
            if ((role == AccessibleRole.FRAME) || (role == AccessibleRole.DIALOG) || (role == AccessibleRole.WINDOW)) {
                AccessibilityNode parent = treeModel.createWindowNode(xAccessible, xAC);
                if (parent != null) {
                    try {
                        int n = xAC.getAccessibleChildCount();
                        for (int i=0; i<n; i++) {
                            AccessibilityNode child = treeModel.createNode(xAC.getAccessibleChild(i));
                            if (child != null) {
                                parent.add(child);
                            }
                        }
                    } catch (com.sun.star.lang.IndexOutOfBoundsException e) {

                    }
                }
                return parent;
            }
        }
        return null;
    }

    /** Returns the XAccessible interface corresponding to the toplevel window */
    private AccessibilityNode getTopWindowNode(XTopWindow w) {
        XAccessible xAccessible = (XAccessible)
            UnoRuntime.queryInterface(XAccessible.class, w);
        if (xAccessible != null) {
            // XTopWindows usually have an accessible parent, which is the
            // native container window ..
            XAccessibleContext xAC = xAccessible.getAccessibleContext();
            if (xAC != null) {
                XAccessible xParent = xAC.getAccessibleParent();
                if (xParent != null) {
                    AccessibilityNode parent = getTopWindowNode(xParent);
                    AccessibilityNode child = treeModel.createNode(xAccessible);
                    if (parent != null && child != null) {
                        parent.add(child);
                    }
                    return parent;
                } else {
                    return getTopWindowNode(xAccessible, xAC);
                }
            }
        }
        return null;
    }

    public void disposing(com.sun.star.lang.EventObject eventObject) {
        // FIXME : message
        // prevent setRoot from removing this as event listener
        xToolkit = null;
        treeModel.setRoot(treeModel.disconnectedRootNode);
    }

    public void windowActivated(com.sun.star.lang.EventObject eventObject) {
    }

    public void windowClosed(com.sun.star.lang.EventObject eventObject) {
        XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(
            XAccessible.class, eventObject.Source);
        if (xAccessible != null) {
            AccessibilityNode node = treeModel.findNode(xAccessible);

            // The object implementing XTopWindow is often not the toplevel
            // accessible object.
            if (node != null && node.getParent() != this) {
                node = (AccessibilityNode) node.getParent();
            }

            if (node != null) {
                final AccessibilityNode an = node;
                Runnable removeRun = new Runnable() {
                    public void run() {
                        try {
                            treeModel.removeNodeFromParent(an);
                            // Calling oneway methods from an UNO thread may cause
                            // deadlocks, so removing the listeners here.
                            an.setAttached(false);
                        } catch (IllegalArgumentException e) {
                            // for some toplevel we get more than one event -
                            // ignoring
                        }
                    }
                };
                SwingUtilities.invokeLater(removeRun);
            }
        }
    }

    public void windowClosing(com.sun.star.lang.EventObject eventObject) {
    }

    public void windowDeactivated(com.sun.star.lang.EventObject eventObject) {
    }

    public void windowMinimized(com.sun.star.lang.EventObject eventObject) {
    }

    public void windowNormalized(com.sun.star.lang.EventObject eventObject) {
    }

    public void windowOpened(com.sun.star.lang.EventObject eventObject) {
        final XTopWindow xTopWindow = (XTopWindow) UnoRuntime.queryInterface(
            XTopWindow.class, eventObject.Source);
        if (xTopWindow != null) {
            final ToolkitNode tn = this;
            Runnable addNodeRun = new Runnable() {
                public void run() {
                    // Note: UNO does not allow to make synchronous callbacks
                    // to oneway calls, so we have to fetch the node here.
                    AccessibilityNode an = getTopWindowNode(xTopWindow);
                    if (an != null) {
                        treeModel.addNodeInto(an, tn);
                        // Calling oneway methods from an UNO thread may cause
                        // deadlocks, so adding the listeners here.
                        an.setAttached(true);
                    }
                }
            };
            SwingUtilities.invokeLater(addNodeRun);
        }
    }
}
