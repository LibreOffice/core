/*************************************************************************
 *
 *  $RCSfile: Table.java,v $
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

import javax.accessibility.AccessibleState;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.*;

public class Table extends DescendantManager implements javax.accessibility.Accessible {

    protected Table(XAccessible xAccessible, XAccessibleContext xAccessibleContext, boolean multiselectable) {
        super(xAccessible, xAccessibleContext, multiselectable);
    }

    protected void setActiveDescendant(javax.accessibility.Accessible descendant) {
        javax.accessibility.Accessible oldAD = activeDescendant;
        activeDescendant = descendant;
        firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_ACTIVE_DESCENDANT_PROPERTY,
            oldAD, descendant);
    }

    protected void setActiveDescendant(Object any) {
        javax.accessibility.Accessible descendant = null;
        try {
            if (AnyConverter.isObject(any)) {
                XAccessible unoAccessible = (XAccessible) AnyConverter.toObject(
                    AccessibleObjectFactory.XAccessibleType, any);
                if (unoAccessible != null) {
                    // FIXME: have to handle non transient objects here ..
                    descendant = new TableCell(unoAccessible);
                    if (Build.DEBUG) {
                        try {
                            if (Build.DEBUG) {
                                System.err.println("[Table] retrieved active descendant event: new descendant is " +
                                    unoAccessible.getAccessibleContext().getAccessibleName());
                            }
                        } catch (java.lang.NullPointerException e) {
                            System.err.println("*** ERROR *** new active descendant not accessible");
                        }
                    }
                }
            }
            setActiveDescendant(descendant);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected void add(XAccessible unoAccessible) {
        if (unoAccessible != null) {
            TableCell cell = new TableCell(unoAccessible);
            // The AccessBridge for Windows expects an instance of AccessibleContext
            // as parameters
            firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                null, cell.getAccessibleContext());
        }
    }

    protected void remove(XAccessible unoAccessible) {
        if (unoAccessible != null) {
            TableCell cell = new TableCell(unoAccessible);
            // The AccessBridge for Windows expects an instance of AccessibleContext
            // as parameters
            firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                cell.getAccessibleContext(), null);
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

    /**
    * Update the proxy objects appropriatly on property change events
    */
    protected class AccessibleTableListener extends AccessibleDescendantManagerListener {

        protected AccessibleTableListener() {
            super();
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.ACTIVE_DESCENDANT_CHANGED:
                    setActiveDescendant(event.NewValue);
                    break;
                case AccessibleEventId.CHILD:
                    if (AnyConverter.isObject(event.OldValue)) {
                        remove(event.OldValue);
                    }
                    if (AnyConverter.isObject(event.NewValue)) {
                        add(event.NewValue);
                    }
                    break;
                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleTableListener();
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            try {
                unoAccessibleContext = unoAccessible.getAccessibleContext();
                unoAccessibleSelection = (XAccessibleSelection) UnoRuntime.queryInterface(
                    XAccessibleSelection.class, unoAccessibleContext);
                accessibleContext = new AccessibleTable();
            } catch (java.lang.NullPointerException e) {
                if (Build.DEBUG) {
                    System.err.println("NullPointerException caught: " + e.getMessage());
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                if (Build.DEBUG) {
                    System.err.println("RuntimeException caught: " + e.getMessage());
                }
            }
        }
        return accessibleContext;
    }

    protected class AccessibleTable extends AccessibleDescendantManager {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleTable() {
            super();
        }

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.TABLE;
        }

        /** Returns the specified Accessible child of the object */
        public javax.accessibility.Accessible getAccessibleChild(int i) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Table.this.activeDescendant;
                    if ((activeDescendant instanceof TableCell) && xAccessible.equals(((TableCell) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TableCell(xAccessible);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }

        /*
        * AccessibleSelection
        */

        /** Returns an Accessible representing the specified selected child of the object */
        public javax.accessibility.Accessible getAccessibleSelection(int i) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleSelection.getSelectedAccessibleChild(i);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Table.this.activeDescendant;
                    if ((activeDescendant instanceof TableCell) && xAccessible.equals(((TableCell) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TableCell(xAccessible);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }
    }

    class TableCell implements javax.accessibility.Accessible {

        protected XAccessible unoAccessible;

        public TableCell(XAccessible xAccessible) {
            unoAccessible = xAccessible;
        }

        public Object[] create(Object[] targetSet) {
            try {
                java.util.ArrayList list = new java.util.ArrayList(targetSet.length);
                for (int i=0; i < targetSet.length; i++) {
                    XAccessible xAccessible = (XAccessible) UnoRuntime.queryInterface(
                        XAccessible.class, targetSet[i]);
                    if (xAccessible != null) {
                        list.add(new TableCell(xAccessible));
                    }
                }
                list.trimToSize();
                return list.toArray();
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        javax.accessibility.AccessibleContext accessibleContext = null;

        /** Returns the AccessibleContext associated with this object */
        public javax.accessibility.AccessibleContext getAccessibleContext() {
            if (accessibleContext == null) {
                try {
                    XAccessibleContext xAccessibleContext = unoAccessible.getAccessibleContext();
                    if (xAccessibleContext != null) {
                        accessibleContext = new AccessibleTableCell(xAccessibleContext);
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                }
            }
            return accessibleContext;
        }

        protected class AccessibleTableCell extends javax.accessibility.AccessibleContext {

            XAccessibleContext unoAccessibleContext;

            public AccessibleTableCell(XAccessibleContext xAccessibleContext) {
                unoAccessibleContext = xAccessibleContext;
            }

            /** Returns the accessible name of this object */
            public String getAccessibleName() {
                try {
                    return unoAccessibleContext.getAccessibleName();
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Sets the accessible name of this object */
            public void setAccessibleName(String name) {
                // Not supported
            }

            /** Returns the accessible name of this object */
            public String getAccessibleDescription() {
                try {
                    return unoAccessibleContext.getAccessibleDescription();
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Sets the accessible name of this object */
            public void setAccessibleDescription(String name) {
                // Not supported
            }

            /** Returns the accessible role of this object */
            public javax.accessibility.AccessibleRole getAccessibleRole() {
                try {
                    javax.accessibility.AccessibleRole role = AccessibleRoleAdapter.getAccessibleRole(
                        unoAccessibleContext.getAccessibleRole());
                    return (role != null) ? role : javax.accessibility.AccessibleRole.LABEL;
                } catch(com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the locale of the component */
            public java.util.Locale getLocale() throws java.awt.IllegalComponentStateException {
                try {
                    com.sun.star.lang.Locale unoLocale = unoAccessibleContext.getLocale();
                    return new java.util.Locale(unoLocale.Language, unoLocale.Country);
                } catch (IllegalAccessibleComponentStateException e) {
                    throw new java.awt.IllegalComponentStateException(e.getMessage());
                } catch (com.sun.star.uno.RuntimeException e) {
                    return Table.this.getLocale();
                }
            }

            /** Gets the 0-based index of this object in its accessible parent */
            public int getAccessibleIndexInParent() {
                try {
                    return unoAccessibleContext.getAccessibleIndexInParent();
                } catch (com.sun.star.uno.RuntimeException e) {
                    return -1;
                }
            }

            /** Returns the number of accessible children of the object. */
            public int getAccessibleChildrenCount() {
                try {
                    return unoAccessibleContext.getAccessibleChildCount();
                } catch (com.sun.star.uno.RuntimeException e) {
                    return 0;
                }
            }

            /** Returns the specified Accessible child of the object. */
            public javax.accessibility.Accessible getAccessibleChild(int i) {
                javax.accessibility.Accessible child = null;
                try {
                    XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Table.this.activeDescendant;
                    if ((activeDescendant instanceof TableCell) && ((TableCell) activeDescendant).unoAccessible.equals(xAccessible)) {
                        child = activeDescendant;
                    } else if (xAccessible != null) {
                        child = new TableCell(xAccessible);
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                } catch (com.sun.star.uno.RuntimeException e) {
                }
                return child;
            }

            /** Returns the state set of this object */
            public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
                try {
                    javax.accessibility.AccessibleStateSet stateSet = new javax.accessibility.AccessibleStateSet();
                    // table cells are transient and so neither focusable nor focused
                    stateSet.add(javax.accessibility.AccessibleState.TRANSIENT);

                    XAccessibleStateSet unoAccessibleStateSet = unoAccessibleContext.getAccessibleStateSet();
                    if (unoAccessibleStateSet.contains(AccessibleStateType.ENABLED)) {
                        stateSet.add(javax.accessibility.AccessibleState.ENABLED);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.OPAQUE)) {
                        stateSet.add(javax.accessibility.AccessibleState.OPAQUE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.RESIZABLE)) {
                        stateSet.add(javax.accessibility.AccessibleState.RESIZABLE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.SHOWING)) {
                        stateSet.add(javax.accessibility.AccessibleState.SHOWING);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.VISIBLE)) {
                        stateSet.add(javax.accessibility.AccessibleState.VISIBLE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.SINGLE_LINE)) {
                        stateSet.add(javax.accessibility.AccessibleState.SINGLE_LINE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.MULTI_LINE)) {
                        stateSet.add(javax.accessibility.AccessibleState.MULTI_LINE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.SELECTABLE)) {
                        stateSet.add(javax.accessibility.AccessibleState.SELECTABLE);
                    }
                    if (unoAccessibleStateSet.contains(AccessibleStateType.SELECTED)) {
                        stateSet.add(javax.accessibility.AccessibleState.SELECTED);
                    }
                    return stateSet;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Returns the relation set of this object */
            public javax.accessibility.AccessibleRelationSet getAccessibleRelationSet() {
                try {
                    XAccessibleRelationSet unoAccessibleRelationSet = unoAccessibleContext.getAccessibleRelationSet();
                    if (unoAccessibleRelationSet == null) {
                        return null;
                    }

                    javax.accessibility.AccessibleRelationSet relationSet = new javax.accessibility.AccessibleRelationSet();
                    int count = unoAccessibleRelationSet.getRelationCount();
                    for (int i = 0; i < count; i++) {
                        AccessibleRelation unoAccessibleRelation = unoAccessibleRelationSet.getRelation(i);
                        switch (unoAccessibleRelation.RelationType) {
                            case AccessibleRelationType.CONTROLLED_BY:
                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.CONTROLLED_BY,
                                    create(unoAccessibleRelation.TargetSet)));
                                break;
                            case AccessibleRelationType.CONTROLLER_FOR:
                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.CONTROLLER_FOR,
                                    create(unoAccessibleRelation.TargetSet)));
                                break;
                            case AccessibleRelationType.LABELED_BY:
                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.LABELED_BY,
                                    create(unoAccessibleRelation.TargetSet)));
                                break;
                            case AccessibleRelationType.LABEL_FOR:
                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.LABEL_FOR,
                                    create(unoAccessibleRelation.TargetSet)));
                                break;
                            case AccessibleRelationType.MEMBER_OF:
                                relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.MEMBER_OF,
                                    create(unoAccessibleRelation.TargetSet)));
                                break;
                            default:
                                break;
                        }
                    }
                    return relationSet;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    return null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleComponent associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleComponent getAccessibleComponent() {
                try {
                    XAccessibleComponent unoAccessibleComponent = (XAccessibleComponent)
                        UnoRuntime.queryInterface(XAccessibleComponent.class, unoAccessibleContext);
                    return (unoAccessibleComponent != null) ?
                        new AccessibleComponentImpl(unoAccessibleComponent) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleAction associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleAction getAccessibleAction() {
                try {
                    XAccessibleAction unoAccessibleAction = (XAccessibleAction)
                        UnoRuntime.queryInterface(XAccessibleAction.class, unoAccessibleContext);
                    return (unoAccessibleAction != null) ?
                        new AccessibleActionImpl(unoAccessibleAction) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleText associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleText getAccessibleText() {
                try {
                    XAccessibleText unoAccessibleText = (XAccessibleText)
                        UnoRuntime.queryInterface(XAccessibleText.class, unoAccessibleContext);
                    return (unoAccessibleText != null) ?
                        new AccessibleTextImpl(unoAccessibleText) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleValue associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleValue getAccessibleValue() {
                try {
                    XAccessibleValue unoAccessibleValue = (XAccessibleValue)
                        UnoRuntime.queryInterface(XAccessibleValue.class, unoAccessibleContext);
                    return (unoAccessibleValue != null) ?
                        new AccessibleValueImpl(unoAccessibleValue) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleText associated with this object presenting text on the display */
            public javax.accessibility.AccessibleIcon[] getAccessibleIcon() {
                try {
                    XAccessibleImage unoAccessibleImage = (XAccessibleImage)
                        UnoRuntime.queryInterface(XAccessibleImage.class, unoAccessibleContext);
                    if (unoAccessibleImage != null) {
                        javax.accessibility.AccessibleIcon[] icons = { new AccessibleIconImpl(unoAccessibleImage) };
                        return icons;
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                }
                return null;
            }
        }
    }
}

