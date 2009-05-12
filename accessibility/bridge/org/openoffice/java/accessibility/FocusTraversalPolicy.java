/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FocusTraversalPolicy.java,v $
 * $Revision: 1.6 $
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

package org.openoffice.java.accessibility;


public class FocusTraversalPolicy extends java.awt.FocusTraversalPolicy {

    protected javax.accessibility.Accessible getSelectedAccessibleChild(javax.accessibility.Accessible a) {
        javax.accessibility.AccessibleContext ac = a.getAccessibleContext();
        if (ac != null) {
            javax.accessibility.AccessibleSelection as = ac.getAccessibleSelection();
            if (as != null) {
                return as.getAccessibleSelection(0);
            }
        }
        return null;
    }

    /** Returns the Component that should receive the focus after aComponent */
    public java.awt.Component getComponentAfter(java.awt.Container focusCycleRoot,
        java.awt.Component aComponent) {
        return null;
    }

    /** Returns the Component that should receive the focus before aComponent */
    public java.awt.Component getComponentBefore(java.awt.Container focusCycleRoot,
        java.awt.Component aComponent) {
        return null;
    }

    /** Returns the default Component to focus */
    public java.awt.Component getDefaultComponent(java.awt.Container focusCycleRoot) {
        // getDefaultComponent must not return null for Windows to make them focusable.
        if (focusCycleRoot instanceof NativeFrame) {
            java.awt.Component c = ((NativeFrame) focusCycleRoot).getInitialComponent();
            if (c != null) {
                return c;
            }
        }

        if (focusCycleRoot instanceof javax.accessibility.Accessible) {
            return (java.awt.Component) getSelectedAccessibleChild((javax.accessibility.Accessible) focusCycleRoot);
        }
        return null;
    }

    /** Returns the first Component in the traversal cycle */
    public java.awt.Component getFirstComponent(java.awt.Container focusCycleRoot) {
        return null;
    }

    /** Returns the Component that should receive the focus when a Window is made visible for the first time */
    public java.awt.Component getInitialComponent(java.awt.Window window) {
        if (window instanceof NativeFrame) {
            return ((NativeFrame) window).getInitialComponent();
        }
        return null;
    }

    /** Returns the last Component in the traversal cycle */
    public java.awt.Component getLastComponent(java.awt.Container focusCycleRoot) {
        return null;
    }
}
