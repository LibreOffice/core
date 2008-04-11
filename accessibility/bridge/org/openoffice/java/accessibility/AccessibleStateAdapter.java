/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleStateAdapter.java,v $
 * $Revision: 1.5 $
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

import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;

import com.sun.star.uno.AnyConverter;
import com.sun.star.accessibility.XAccessibleStateSet;

public class AccessibleStateAdapter {

    private static final AccessibleState[] stateTypeMap = {
        null,
        AccessibleState.ACTIVE,
        AccessibleState.ARMED,
        AccessibleState.BUSY,
        AccessibleState.CHECKED,
        AccessibleExtendedState.DEFUNCT,
        AccessibleState.EDITABLE,
        AccessibleState.ENABLED,
        AccessibleState.EXPANDABLE,
        AccessibleState.EXPANDED,
        AccessibleState.FOCUSABLE,
        AccessibleState.FOCUSED,
        AccessibleState.HORIZONTAL,
        AccessibleState.ICONIFIED,
        AccessibleExtendedState.INDETERMINATE,
        AccessibleExtendedState.MANAGES_DESCENDANTS,
        AccessibleState.MODAL,
        AccessibleState.MULTI_LINE,
        AccessibleState.MULTISELECTABLE,
        AccessibleState.OPAQUE,
        AccessibleState.PRESSED,
        AccessibleState.RESIZABLE,
        AccessibleState.SELECTABLE,
        AccessibleState.SELECTED,
        AccessibleExtendedState.SENSITIVE,
        AccessibleState.SHOWING,
        AccessibleState.SINGLE_LINE,
        AccessibleExtendedState.STALE,
        AccessibleState.TRANSIENT,
        AccessibleState.VERTICAL,
        AccessibleState.VISIBLE
    };

    private static void printToplevelStateMessage(AccessibleState s, java.awt.Component c) {
        System.err.println("*** ERROR *** " + s + " state is a toplevel window state " + c);
    }

    private static String getDisplayName(java.awt.Component c) {
        javax.accessibility.Accessible a = (javax.accessibility.Accessible) c;
        if( a != null) {
            javax.accessibility.AccessibleContext ac = a.getAccessibleContext();
            return "[" + ac.getAccessibleRole() + "] " + ac.getAccessibleName();
        } else {
            return c.toString();
        }
    }

    private static void printOutOfSyncMessage(AccessibleState s, java.awt.Component c, boolean enabled) {
        System.err.println("*** ERROR *** " + s + " state out of sync (" + !enabled + ", " + enabled + ") for " + getDisplayName(c));
    }

    public static AccessibleState getAccessibleState(Object any) {
       try {
            if (AnyConverter.isShort(any)) {
                return getAccessibleState(AnyConverter.toShort(any));
            }
            return null;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            return null;
        }
    }

    public static AccessibleState getAccessibleState(short unoStateType) {
        if (unoStateType > 0 && unoStateType < stateTypeMap.length) {
            return stateTypeMap[unoStateType];
        }
        return null;
    }

    public static AccessibleStateSet getDefunctStateSet() {
        AccessibleStateSet ass = new AccessibleStateSet();
        ass.add(AccessibleExtendedState.DEFUNCT);
        return ass;
    }

    public static AccessibleStateSet getAccessibleStateSet(java.awt.Component c,
            XAccessibleStateSet xAccessibleStateSet) {

        try {
            if (xAccessibleStateSet != null) {
                AccessibleStateSet as = new AccessibleStateSet();
                short[] unoStateTypes = xAccessibleStateSet.getStates();
                for (int i=0; i<unoStateTypes.length; i++) {
                    if (unoStateTypes[i] > 0 &&
                            unoStateTypes[i] < stateTypeMap.length) {
                        as.add(stateTypeMap[unoStateTypes[i]]);
                    }
                }

                // Note: COLLAPSED does not exists in the UAA.
                if (as.contains(AccessibleState.EXPANDABLE) &&
                        ! as.contains(AccessibleState.EXPANDED)) {
                    as.add(AccessibleState.COLLAPSED);
                }

                // Sync office and Java FOCUSED state
                boolean isFocusInSync;
                if (c.isFocusOwner()) {
                    isFocusInSync = !as.add(AccessibleState.FOCUSED);
                } else {
                    isFocusInSync = !as.remove(AccessibleState.FOCUSED);
                }

                // Sync office and Java ACTIVE state
                boolean isActiveInSync;
                if (c instanceof java.awt.Window && ((java.awt.Window) c).isActive()) {
                    isActiveInSync = !as.add(AccessibleState.ACTIVE);
                } else {
                    isActiveInSync = !as.remove(AccessibleState.ACTIVE);
                }

                // Report out-of-sync messages
                if (!Build.PRODUCT) {
                    if (!isFocusInSync) {
                        printOutOfSyncMessage(AccessibleState.FOCUSED, c, c.isFocusOwner());
                    }
                    if (!isActiveInSync) {
                        printOutOfSyncMessage(AccessibleState.ACTIVE, c, ((java.awt.Window) c).isActive());
                    }
                    if (as.contains(AccessibleState.ENABLED) != c.isEnabled()) {
                        printOutOfSyncMessage(AccessibleState.ENABLED, c, c.isEnabled());
                    }
                    if (as.contains(AccessibleState.FOCUSABLE) != c.isFocusable()) {
                        printOutOfSyncMessage(AccessibleState.FOCUSABLE, c, c.isFocusable());
                    }
                    if (as.contains(AccessibleState.SHOWING) != c.isShowing()) {
                        printOutOfSyncMessage(AccessibleState.SHOWING, c, c.isShowing());
                    }
                    if (as.contains(AccessibleState.VISIBLE) != c.isVisible()) {
                        printOutOfSyncMessage(AccessibleState.VISIBLE, c, c.isVisible());
                    }

                    // The following states are for toplevel windows only
                    if (! (c instanceof java.awt.Window)) {
                        if (as.contains(AccessibleState.ACTIVE)) {
                            printToplevelStateMessage(AccessibleState.ACTIVE, c);
                        }
                        if (as.contains(AccessibleState.ICONIFIED)) {
                            printToplevelStateMessage(AccessibleState.ICONIFIED, c);
                        }
                        if (as.contains(AccessibleState.MODAL)) {
                            printToplevelStateMessage(AccessibleState.MODAL, c);
                        }
                    }
                }
                return as;
            }
            return null;
        } catch (com.sun.star.uno.RuntimeException e) {
            return getDefunctStateSet();
        }
    }
}
