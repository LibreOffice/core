/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleSelectionImpl.java,v $
 * $Revision: 1.7 $
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
