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

import com.sun.star.accessibility.*;
import com.sun.star.uno.UnoRuntime;


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
            unoAccessibleSelection = UnoRuntime.queryInterface(XAccessibleSelection.class,
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
            unoAccessibleSelection = UnoRuntime.queryInterface(XAccessibleSelection.class,
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
