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

class AccessibleSelectionImpl implements javax.accessibility.AccessibleSelection {
    XAccessibleSelection unoAccessibleSelection;

    AccessibleSelectionImpl(XAccessibleSelection xAccessibleSelection) {
        unoAccessibleSelection = xAccessibleSelection;
    }

    /** Returns an Accessible representing the specified selected child of the object */
    public javax.accessibility.Accessible getAccessibleSelection(int i) {
        try {
            return (javax.accessibility.Accessible) AccessibleObjectFactory.getAccessibleComponent(
                unoAccessibleSelection.getSelectedAccessibleChild(i));
        } catch (com.sun.star.uno.Exception e) {
            return null;
        }
    }

    /** Adds the specified Accessible child of the object to the object's selection */
    public void addAccessibleSelection(int i) {
        try {
            unoAccessibleSelection.selectAccessibleChild(i);
        } catch (com.sun.star.uno.Exception e) {
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
        } catch (com.sun.star.uno.Exception e) {
            return false;
        }
    }

    /** Removes the specified child of the object from the object's selection */
    public void removeAccessibleSelection(int i) {
        try {
            unoAccessibleSelection.deselectAccessibleChild(i);
        } catch (com.sun.star.uno.Exception e) {
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
