/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleSelectionImpl.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:33:49 $
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
