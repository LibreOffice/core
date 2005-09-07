/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleStateAdapter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:34:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    private static void printOutOfSyncMessage(AccessibleState s, java.awt.Component c) {
        System.err.println("*** ERROR *** " + s + " state out of sync for " + c);
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
                        printOutOfSyncMessage(AccessibleState.FOCUSED, c);
                    }
                    if (!isActiveInSync) {
                        printOutOfSyncMessage(AccessibleState.ACTIVE, c);
                    }
                    if (as.contains(AccessibleState.ENABLED) != c.isEnabled()) {
                        printOutOfSyncMessage(AccessibleState.ENABLED, c);
                    }
                    if (as.contains(AccessibleState.FOCUSABLE) != c.isFocusable()) {
                        printOutOfSyncMessage(AccessibleState.FOCUSABLE, c);
                    }
                    if (as.contains(AccessibleState.SHOWING) != c.isShowing()) {
                        printOutOfSyncMessage(AccessibleState.SHOWING, c);
                    }
                    if (as.contains(AccessibleState.VISIBLE) != c.isVisible()) {
                        printOutOfSyncMessage(AccessibleState.VISIBLE, c);
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
