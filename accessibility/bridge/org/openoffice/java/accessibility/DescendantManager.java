/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DescendantManager.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:37:05 $
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
