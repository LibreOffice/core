/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Menu.java,v $
 * $Revision: 1.9 $
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


public class Menu extends AbstractButton
    implements javax.accessibility.Accessible {
    private java.util.Vector children;
    protected XAccessibleSelection unoAccessibleSelection = null;

    protected Menu(XAccessible xAccessible,
        XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);

        try {
            // Create a vector with the correct initial capacity
            int count = unoAccessibleContext.getAccessibleChildCount();
            children = new java.util.Vector(count);

            // Fill the vector with objects
            for (int i = 0; i < count; i++) {
                java.awt.Component c = getComponent(unoAccessibleContext.getAccessibleChild(i));

                if (c != null) {
                    children.add(c);
                }
            }
        } catch (com.sun.star.uno.RuntimeException e) {
            if (Build.DEBUG) {
                System.err.println(
                    "RuntimeException caught during menu initialization: " +
                    e.getMessage());
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
                children.add(unoAccessible.getAccessibleContext()
                                          .getAccessibleIndexInParent(), c);
                firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                    null,
                    ((javax.accessibility.Accessible) c).getAccessibleContext());
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
                    ((javax.accessibility.Accessible) c).getAccessibleContext(),
                    null);
            } catch (com.sun.star.uno.RuntimeException e) {
            }
        }
    }

    protected void add(Object any) {
        try {
            add((XAccessible) AnyConverter.toObject(
                    AccessibleObjectFactory.XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected void remove(Object any) {
        try {
            remove((XAccessible) AnyConverter.toObject(
                    AccessibleObjectFactory.XAccessibleType, any));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected synchronized int indexOf(Object child) {
        return children.indexOf(child);
    }

    protected java.awt.Component getComponent(XAccessible unoAccessible) {
        java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(unoAccessible);

        if (c == null) {
            c = AccessibleObjectFactory.createAccessibleComponent(unoAccessible);

            if (c instanceof javax.accessibility.Accessible) {
                ((javax.accessibility.Accessible) c).getAccessibleContext()
                 .setAccessibleParent(this);
            }
        }

        return c;
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleMenuListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleMenu();
    }

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleMenuListener
        extends AccessibleUNOComponentListener {
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

                // #i56539# Java 1.5 does not fire ACCESSIBLE_SELECTION_PROPERTY for menus
                case AccessibleEventId.SELECTION_CHANGED:
                    break;

                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected class AccessibleMenu extends AccessibleAbstractButton
        implements javax.accessibility.AccessibleSelection {
        protected AccessibleMenu() {
            unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(XAccessibleSelection.class,
                    unoAccessibleContext);
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
        public synchronized javax.accessibility.Accessible getAccessibleChild(
            int i) {
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
                java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(unoAccessibleComponent.getAccessibleAtPoint(
                            new com.sun.star.awt.Point(p.x, p.y)));

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
                return (javax.accessibility.Accessible) getComponent(unoAccessibleSelection.getSelectedAccessibleChild(
                        i));
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
