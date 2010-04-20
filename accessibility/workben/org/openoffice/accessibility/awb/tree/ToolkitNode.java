/*************************************************************************
 *
 *  $RCSfile: ToolkitNode.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2003/09/19 09:21:41 $
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
