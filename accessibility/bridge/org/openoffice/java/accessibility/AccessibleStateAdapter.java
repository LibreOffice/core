/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.java.accessibility;

import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;

import com.sun.star.uno.AnyConverter;
import com.sun.star.accessibility.AccessibleStateType;
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
        System.err.println("*** ERROR *** " + s + " state out of sync (UNO state set: " + !enabled + ", Java component state: " + enabled + ") for " + getDisplayName(c));
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

    public static void setComponentState(java.awt.Component c,
            XAccessibleStateSet xAccessibleStateSet) {

        try {
            if (xAccessibleStateSet != null) {
                // Set the boundings of the component if it is visible ..
                if (!xAccessibleStateSet.contains(AccessibleStateType.VISIBLE)) {
                    c.setVisible(false);
                }
                // Set the components' enabled state ..
                if (!xAccessibleStateSet.contains(AccessibleStateType.ENABLED)) {
                    c.setEnabled(false);
                }
                // Set the components' focusable state ..
                if (!xAccessibleStateSet.contains(AccessibleStateType.FOCUSABLE)) {
                    c.setFocusable(false);
                }
            }
        } catch (com.sun.star.uno.RuntimeException e) {
        }
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
