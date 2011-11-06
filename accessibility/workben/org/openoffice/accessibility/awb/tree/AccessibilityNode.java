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


