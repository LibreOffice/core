/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DescendantManager.java,v $
 * $Revision: 1.8 $
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

import com.sun.star.accessibility.*;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

import javax.accessibility.AccessibleState;


public abstract class DescendantManager extends Component {
    protected XAccessibleSelection unoAccessibleSelection = null;
    protected javax.accessibility.Accessible activeDescendant = null;
    protected boolean multiselectable = false;

    protected DescendantManager(XAccessible xAccessible,
        XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    protected DescendantManager(XAccessible xAccessible,
        XAccessibleContext xAccessibleContext, boolean multiselectable) {
        super(xAccessible, xAccessibleContext);
        this.multiselectable = multiselectable;
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleDescendantManagerListener
        extends AccessibleUNOComponentListener {
        protected AccessibleDescendantManagerListener() {
            unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(XAccessibleSelection.class,
                    unoAccessibleContext);
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.SELECTION_CHANGED:
                    firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_SELECTION_PROPERTY,
                        null, null);

                    break;

                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected abstract class AccessibleDescendantManager
        extends AccessibleUNOComponent
        implements javax.accessibility.AccessibleSelection {
        protected AccessibleDescendantManager() {
            unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(XAccessibleSelection.class,
                    unoAccessibleContext);
        }

        /*
        * AccessibleContext
        */

        /** Returns the number of accessible children of the object */
        public int getAccessibleChildrenCount() {
            try {
                return unoAccessibleContext.getAccessibleChildCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Returns the AccessibleSelection interface for this object */
        public javax.accessibility.AccessibleSelection getAccessibleSelection() {
            return (unoAccessibleSelection != null) ? this : null;
        }

        /*
        * AccessibleSelection
        */

        /** Adds the specified Accessible child of the object to the object's selection */
        public void addAccessibleSelection(int i) {
            try {
                unoAccessibleSelection.selectAccessibleChild(i);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Clears the selection in the object, so that no children in the object are selected */
        public void clearAccessibleSelection() {
            try {
                unoAccessibleSelection.clearAccessibleSelection();
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Returns the number of Accessible children currently selected */
        public int getAccessibleSelectionCount() {
            try {
                return unoAccessibleSelection.getSelectedAccessibleChildCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Determines if the current child of this object is selected */
        public boolean isAccessibleChildSelected(int i) {
            try {
                return unoAccessibleSelection.isAccessibleChildSelected(i);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return false;
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        /** Removes the specified child of the object from the object's selection */
        public void removeAccessibleSelection(int i) {
            try {
                unoAccessibleSelection.deselectAccessibleChild(i);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }

        /** Causes every child of the object to be selected if the object supports multiple selection */
        public void selectAllAccessibleSelection() {
            try {
                unoAccessibleSelection.selectAllAccessibleChildren();
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
    }
}
