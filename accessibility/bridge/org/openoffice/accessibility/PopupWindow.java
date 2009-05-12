/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PopupWindow.java,v $
 * $Revision: 1.4 $
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
package org.openoffice.accessibility;

import org.openoffice.java.accessibility.*;


/**
 *
 */
public class PopupWindow extends java.awt.Window {
    javax.accessibility.AccessibleContext accessibleContext = null;
    ContainerProxy layeredPane = new ContainerProxy(javax.accessibility.AccessibleRole.LAYERED_PANE);
    ContainerProxy rootPane = new ContainerProxy(javax.accessibility.AccessibleRole.ROOT_PANE);
    ContainerProxy popupLayer = new ContainerProxy(javax.accessibility.AccessibleRole.PANEL);
    boolean opened = false;
    boolean visible = false;

    /** Creates a new instance of PopupWindow */
    public PopupWindow(java.awt.Window owner) {
        super(owner);
        super.add(rootPane);
        rootPane.add(layeredPane);

        javax.accessibility.AccessibleContext ac = rootPane.getAccessibleContext();

        if (ac != null) {
            ac.setAccessibleParent(this);
        }
    }

    static PopupWindow create(
        com.sun.star.accessibility.XAccessible xAccessible) {
        java.awt.Window parent = java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager()
                                                              .getActiveWindow();

        if (parent != null) {
            PopupWindow w = new PopupWindow(parent);
            w.setVisible(true);
            AccessibleObjectFactory.invokeAndWait();
            AccessibleObjectFactory.addChild(w, xAccessible);

            return w;
        }

        return null;
    }

    public boolean isShowing() {
        if (isVisible()) {
            java.awt.Container parent = getParent();

            return (parent == null) || parent.isShowing();
        }

        return false;
    }

    public void addNotify() {
    }

    public void removeNotify() {
    }

    public boolean isVisible() {
        return visible;
    }

    public void setVisible(boolean b) {
        if (visible != b) {
            visible = b;

            if (b) {
                // If it is the first show, fire WINDOW_OPENED event
                if (!opened) {
                    AccessibleObjectFactory.postWindowOpened(this);
                    opened = true;
                }
            }
        }
    }

    public java.awt.Component add(java.awt.Component c) {
        popupLayer.add(c);
        layeredPane.add(popupLayer);

        if (c instanceof javax.accessibility.Accessible) {
            javax.accessibility.AccessibleContext ac = layeredPane.getAccessibleContext();

            if (ac != null) {
                ac.firePropertyChange(ac.ACCESSIBLE_CHILD_PROPERTY, null,
                    popupLayer.getAccessibleContext());
            }
        }

        return c;
    }

    public void remove(java.awt.Component c) {
        layeredPane.remove(popupLayer);

        if (c instanceof javax.accessibility.Accessible) {
            javax.accessibility.AccessibleContext ac = layeredPane.getAccessibleContext();

            if (ac != null) {
                ac.firePropertyChange(ac.ACCESSIBLE_CHILD_PROPERTY,
                    popupLayer.getAccessibleContext(), null);
            }
        }

        popupLayer.remove(c);
    }

    public void dispose() {
        setVisible(false);
        AccessibleObjectFactory.postWindowClosed(this);
    }

    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessiblePopupWindow();
        }

        return accessibleContext;
    }

    protected class AccessiblePopupWindow
        extends java.awt.Window.AccessibleAWTWindow {
        AccessiblePopupWindow() {
        }
    }

    protected class ContainerProxy extends java.awt.Container
        implements javax.accessibility.Accessible {
        javax.accessibility.AccessibleContext accessibleContext = null;
        javax.accessibility.AccessibleRole role;

        protected ContainerProxy(javax.accessibility.AccessibleRole role) {
            this.role = role;
        }

        public java.awt.Component add(java.awt.Component c) {
            if (c instanceof javax.accessibility.Accessible) {
                javax.accessibility.Accessible a = (javax.accessibility.Accessible) c;
                javax.accessibility.AccessibleContext ac = a.getAccessibleContext();

                if (ac != null) {
                    ac.setAccessibleParent(this);
                }
            }

            return super.add(c);
        }

        public void remove(java.awt.Component c) {
            if (c instanceof javax.accessibility.Accessible) {
                javax.accessibility.Accessible a = (javax.accessibility.Accessible) c;
                javax.accessibility.AccessibleContext ac = a.getAccessibleContext();

                if (ac != null) {
                    ac.setAccessibleParent(null);
                }
            }

            super.remove(c);
        }

        public javax.accessibility.AccessibleContext getAccessibleContext() {
            if (accessibleContext == null) {
                accessibleContext = new AccessibleContainerProxy();
            }

            return accessibleContext;
        }

        private class AccessibleContainerProxy
            extends java.awt.Container.AccessibleAWTContainer {
            AccessibleContainerProxy() {
            }

            public javax.accessibility.AccessibleRole getAccessibleRole() {
                return ContainerProxy.this.role;
            }
        }
    }
}
