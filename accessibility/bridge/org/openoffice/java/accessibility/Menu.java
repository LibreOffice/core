/*************************************************************************
 *
 *  $RCSfile: Menu.java,v $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.java.accessibility;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.*;

public class Menu extends AbstractButton implements javax.accessibility.Accessible {

    private java.util.Vector children;

    protected Menu(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
        try {
            // Create a vector with the correct initial capacity
            int count = unoAccessibleContext.getAccessibleChildCount();
            children = new java.util.Vector(count);
            // Fill the vector with objects
            for (int i=0; i < count; i++) {
                java.awt.Component c = getComponent(unoAccessibleContext.getAccessibleChild(i));
                if (c != null) {
                    children.add(c);
                }
            }
        } catch (com.sun.star.uno.RuntimeException e) {
            if (Build.DEBUG) {
                System.err.println("RuntimeException caught during menu initialization: " + e.getMessage());
            }
            if (children == null) {
                children = new java.util.Vector(0);
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }
    }

    protected synchronized void add(XAccessible unoAccessible) {
        // The AccessBridge for Windows expects an instance of AccessibleContext
        // as parameters
        java.awt.Component c = getComponent(unoAccessible);
        if (c != null) {
            try {
                children.add(unoAccessible.getAccessibleContext().getAccessibleIndexInParent(), c);
                firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                    null, ((javax.accessibility.Accessible) c).getAccessibleContext());
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
    }

    protected synchronized void remove(XAccessible unoAccessible) {
        // The AccessBridge for Windows expects an instance of AccessibleContext
        // as parameters
        java.awt.Component c = getComponent(unoAccessible);
        if (c != null) {
            try {
                children.remove(c);
                firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                    ((javax.accessibility.Accessible) c).getAccessibleContext(), null);
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
    }

    protected void add(Object any) {
        try {
            add((XAccessible) AnyConverter.toObject(AccessibleObjectFactory.XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected void remove(Object any) {
        try {
            remove((XAccessible) AnyConverter.toObject(AccessibleObjectFactory.XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected synchronized int indexOf(Object child) {
        return  children.indexOf(child);
    }

    protected java.awt.Component getComponent(XAccessible unoAccessible) {
        java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(unoAccessible);
        if (c == null) {
            c = AccessibleObjectFactory.createAccessibleComponent(unoAccessible);
            if (c instanceof javax.accessibility.Accessible) {
                ((javax.accessibility.Accessible) c).getAccessibleContext().setAccessibleParent(this);
            }
        }
        return c;
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleMenuListener extends AccessibleUNOComponentListener {

        protected AccessibleMenuListener() {
            super();
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.CHILD:
                    if (AnyConverter.isObject(event.OldValue)) {
                        remove(event.OldValue);
                    }
                    if (AnyConverter.isObject(event.NewValue)) {
                        add(event.NewValue);
                    }
                    break;
                case AccessibleEventId.SELECTION_CHANGED:
                    firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_SELECTION_PROPERTY, null, null);
                    break;
                default:
                    super.notifyEvent(event);
            }
        }

    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleMenuListener();
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            try {
                unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(
                    XAccessibleSelection.class, unoAccessibleContext);
                if (unoAccessibleSelection != null) {
                    accessibleContext = new AccessibleMenu();
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
        return accessibleContext;
    }

    protected XAccessibleSelection unoAccessibleSelection = null;

    protected class AccessibleMenu extends AccessibleAbstractButton implements javax.accessibility.AccessibleSelection {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleMenu() {
            super();
        }

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.MENU;
        }

        /** Gets the 0-based index of this object in its accessible parent */
        public int getAccessibleIndexInParent() {
            if (getAccessibleParent() instanceof Menu) {
                return ((Menu) getAccessibleParent()).indexOf(Menu.this);
            } else {
                return super.getAccessibleIndexInParent();
            }
        }

        /** Returns the number of accessible children of the object */
        public synchronized int getAccessibleChildrenCount() {
            return children.size();
        }

        /** Returns the specified Accessible child of the object */
        public synchronized javax.accessibility.Accessible getAccessibleChild(int i) {
            try {
                if (i < children.size()) {
                    return (javax.accessibility.Accessible) children.get(i);
                } else {
                    return null;
                }
            } catch (ArrayIndexOutOfBoundsException e) {
                return null;
            }
        }

        /** Returns the AccessibleSelection interface for this object */
        public javax.accessibility.AccessibleSelection getAccessibleSelection() {
            // This method is called to determine the SELECTABLE state of every
            // child, so don't do the query interface here.
            return this;
        }

        /*
        * AccessibleComponent
        */

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            try {
                java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(
                    unoAccessibleComponent.getAccessibleAtPoint(new com.sun.star.awt.Point(p.x, p.y)));

                return (javax.accessibility.Accessible) c;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /*
        * AccessibleSelection
        */

        /** Returns an Accessible representing the specified selected child of the object */
        public javax.accessibility.Accessible getAccessibleSelection(int i) {
            try {
                return (javax.accessibility.Accessible)
                    getComponent(unoAccessibleSelection.getSelectedAccessibleChild(i));
            } catch (java.lang.Exception e) {
                /*
                * Possible exceptions are:
                *   java.lang.NullPointerException
                *   com.sun.star.uno.RuntimeException
                *   com.sun.star.lang.IndexOutOfBoundsException
                */
                return null;
            }
        }

        /** Adds the specified Accessible child of the object to the object's selection */
        public void addAccessibleSelection(int i) {
            try {
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

