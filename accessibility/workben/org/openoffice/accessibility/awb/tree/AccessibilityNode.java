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

import org.openoffice.accessibility.misc.AccessibleEventMulticaster;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.SwingUtilities;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;

import com.sun.star.uno.UnoRuntime;

class AccessibilityNode extends DefaultMutableTreeNode implements XAccessible,
        XAccessibleEventListener, XAccessibleEventBroadcaster {

    protected AccessibilityModel treeModel;
    protected XAccessibleContext unoAccessibleContext;

    private XAccessibleEventListener listener;

    public AccessibilityNode(AccessibilityModel treeModel) {
        this.treeModel = treeModel;
    }

    protected void finalize() throws java.lang.Throwable {
        if (userObject != null) {
            treeModel.removeNode(userObject);
        }
    }

    /** Sets the XAccessibleContext object of this node */
    public void setAccessibleContext(XAccessibleContext xAccessibleContext) {
        unoAccessibleContext = xAccessibleContext;
    }

    /** Returns the XAccessibleContext object of this node */
    public XAccessibleContext getAccessibleContext() {
        return unoAccessibleContext;
    }

    /** Attaches or Detaches the itself as listener to unoAccessibleContext */
    protected void setAttached(boolean attach) {
        XAccessibleContext xAccessibleContext = unoAccessibleContext;
        if (xAccessibleContext != null) {
            try {
                XAccessibleEventBroadcaster xAccessibleEventBroadcaster =
                    UnoRuntime.queryInterface( XAccessibleEventBroadcaster.class, xAccessibleContext );
                if (xAccessibleEventBroadcaster != null) {
                    if (attach) {
                        xAccessibleEventBroadcaster.addEventListener(this);
                    } else {
                        xAccessibleEventBroadcaster.removeEventListener(this);
                    }
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                // FIXME: error message !
            }
        }
    }

    public void disposing(com.sun.star.lang.EventObject eventObject) {
        XAccessibleEventListener localListener = this.listener;
        if (localListener != null) {
            localListener.disposing(eventObject);
        }

        treeModel.removeNode(userObject);
        userObject = null;
        unoAccessibleContext = null;
        // FIXME: mark the object as being disposed in the tree view !
    }

    protected void handleChildRemoved(XAccessible xAccessible) {
        final AccessibilityNode node = treeModel.findNode(xAccessible);
        if (node != null) {
            SwingUtilities.invokeLater(new java.lang.Runnable() {
                public void run() {
                    treeModel.removeNodeFromParent(node);
                }
            });
        }
    }

    protected void handleChildAdded(XAccessible xAccessible) {
        final AccessibilityNode parent = this;
        final AccessibilityNode node = treeModel.createNode(xAccessible);
        if (node != null) {
            SwingUtilities.invokeLater(new java.lang.Runnable() {
                public void run() {
                    try {
                        XAccessibleContext xAC = node.getAccessibleContext();
                        if (xAC != null) {
                            treeModel.insertNodeInto(node, parent,
                                xAC.getAccessibleIndexInParent());
                        }
                    } catch (com.sun.star.uno.RuntimeException e) {
                        // FIXME: output
                    }
                }
            });
        }
    }

    public void notifyEvent(AccessibleEventObject accessibleEventObject) {
        if (accessibleEventObject.EventId == AccessibleEventId.CHILD) {
            XAccessible xAccessible = UnoRuntime.queryInterface( XAccessible.class, accessibleEventObject.OldValue );
            if (xAccessible != null) {
                handleChildRemoved(xAccessible);
            }

            xAccessible = UnoRuntime.queryInterface( XAccessible.class, accessibleEventObject.NewValue );
            if (xAccessible != null) {
                handleChildAdded(xAccessible);
            }
        }

        XAccessibleEventListener localListener = this.listener;
        if (localListener != null) {
            localListener.notifyEvent(accessibleEventObject);
        }
    }

    public synchronized void addEventListener(com.sun.star.accessibility.XAccessibleEventListener xAccessibleEventListener) {
         listener = AccessibleEventMulticaster.add(listener, xAccessibleEventListener);
    }

    public synchronized void removeEventListener(com.sun.star.accessibility.XAccessibleEventListener xAccessibleEventListener) {
        listener = AccessibleEventMulticaster.remove(listener, xAccessibleEventListener);
    }
}


