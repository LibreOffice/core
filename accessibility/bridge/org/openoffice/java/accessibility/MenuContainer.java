/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MenuContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 15:08:20 $
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

import javax.accessibility.AccessibleContext;

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

/**
 * Specialized container for MenuBar and Popup-Menu(s)
 * FIXME: join with Menu ?
 */
public class MenuContainer extends Container implements javax.accessibility.Accessible {

    protected XAccessibleSelection unoAccessibleSelection = null;

    protected MenuContainer(javax.accessibility.AccessibleRole role, XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(role, xAccessible, xAccessibleContext);
    }

    protected class AccessibleMenuContainerListener extends AccessibleContainerListener {

        protected AccessibleMenuContainerListener() {
            super();
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {

                // #i56539# Java 1.5 does not fire ACCESSIBLE_SELECTION_PROPERTY for menus
                case AccessibleEventId.SELECTION_CHANGED:
                    break;

                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleMenuContainerListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleMenuContainer();
    }

    protected class AccessibleMenuContainer extends AccessibleContainer implements javax.accessibility.AccessibleSelection {

        protected AccessibleMenuContainer() {
            unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(XAccessibleSelection.class,
                    unoAccessibleContext);
        }

        /** Returns the AccessibleSelection interface for this object */
        public javax.accessibility.AccessibleSelection getAccessibleSelection() {
            return this;
        }

        /*
        * AccessibleSelection
        */

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
                javax.accessibility.Accessible a = getAccessibleChild(i);

                // selecting menu items invokes the click action in Java 1.5
                if( a instanceof MenuItem )
                    a.getAccessibleContext().getAccessibleAction().doAccessibleAction(0);
                else
                    unoAccessibleSelection.selectAccessibleChild(i);
            } catch (java.lang.Exception e) {
                /*
                * Possible exceptions are:
                *   java.lang.NullPointerException
                *   com.sun.star.uno.RuntimeException
                *   com.sun.star.lang.IndexOutOfBoundsException
                */
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
            } catch (java.lang.Exception e) {
                /*
                * Possible exceptions are:
                *   java.lang.NullPointerException
                *   com.sun.star.uno.RuntimeException
                *   com.sun.star.lang.IndexOutOfBoundsException
                */
                return false;
            }
        }

        /** Removes the specified child of the object from the object's selection */
        public void removeAccessibleSelection(int i) {
            if (isAccessibleChildSelected(i)) {
                clearAccessibleSelection();
            }
        }

        /** Causes every child of the object to be selected if the object supports multiple selection */
        public void selectAllAccessibleSelection() {
            // not supported
        }
    }
}

