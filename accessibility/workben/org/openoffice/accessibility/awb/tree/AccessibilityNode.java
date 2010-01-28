/*************************************************************************
 *
 *  $RCSfile: AccessibilityNode.java,v $
 *
 *  $Revision: 1.2 $
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
                    (XAccessibleEventBroadcaster) UnoRuntime.queryInterface(
                        XAccessibleEventBroadcaster.class, xAccessibleContext);
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
        XAccessibleEventListener listener = this.listener;
        if (listener != null) {
            listener.disposing(eventObject);
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
            XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(
                XAccessible.class, accessibleEventObject.OldValue);
            if (xAccessible != null) {
                handleChildRemoved(xAccessible);
            }

            xAccessible = (XAccessible) UnoRuntime.queryInterface(
                XAccessible.class, accessibleEventObject.NewValue);
            if (xAccessible != null) {
                handleChildAdded(xAccessible);
            }
        }

        XAccessibleEventListener listener = this.listener;
        if (listener != null) {
            listener.notifyEvent(accessibleEventObject);
        }
    }

    public synchronized void addEventListener(com.sun.star.accessibility.XAccessibleEventListener xAccessibleEventListener) {
         listener = AccessibleEventMulticaster.add(listener, xAccessibleEventListener);
    }

    public synchronized void removeEventListener(com.sun.star.accessibility.XAccessibleEventListener xAccessibleEventListener) {
        listener = AccessibleEventMulticaster.remove(listener, xAccessibleEventListener);
    }
}


