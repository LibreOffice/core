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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.*;

public class List extends DescendantManager implements javax.accessibility.Accessible {

    protected List(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    protected void setActiveDescendant(javax.accessibility.Accessible descendant) {
        javax.accessibility.Accessible oldAD = activeDescendant;
        activeDescendant = descendant;
        firePropertyChange(AccessibleContext.ACCESSIBLE_ACTIVE_DESCENDANT_PROPERTY,
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
                    descendant = new ListItem(unoAccessible);
                    if (Build.DEBUG) {
                        try {
                            if (Build.DEBUG) {
                                System.err.println("[List] retrieved active descendant event: new descendant is " +
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
            ListItem item = new ListItem(unoAccessible);
            // The AccessBridge for Windows expects an instance of AccessibleContext
            // as parameters
            firePropertyChange(AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                null, item.getAccessibleContext());
        }
    }

    protected void remove(XAccessible unoAccessible) {
        if (unoAccessible != null) {
            ListItem item = new ListItem(unoAccessible);
            // The AccessBridge for Windows expects an instance of AccessibleContext
            // as parameters
            firePropertyChange(AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                item.getAccessibleContext(), null);
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
    protected class AccessibleListListener extends AccessibleDescendantManagerListener {

        protected AccessibleListListener() {
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
                case AccessibleEventId.INVALIDATE_ALL_CHILDREN:
                    // Since List items a transient a child events are mostly used
                    // to attach/detach listeners, it is save to ignore it here
                    break;
                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleListListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleList();
    }

    protected class AccessibleList extends AccessibleDescendantManager {

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.LIST;
        }

        /** Returns the specified Accessible child of the object */
        public javax.accessibility.Accessible getAccessibleChild(int i) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = List.this.activeDescendant;
                    if ((activeDescendant instanceof ListItem) && xAccessible.equals(((ListItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new ListItem(xAccessible);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }

        /*
         * AccessibleComponent
         */

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleComponent.getAccessibleAtPoint(new com.sun.star.awt.Point(p.x, p.y));
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = List.this.activeDescendant;
                    if ((activeDescendant instanceof ListItem) && xAccessible.equals(((ListItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new ListItem(xAccessible);
                    }
                }
                return child;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
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
                    javax.accessibility.Accessible activeDescendant = List.this.activeDescendant;
                    if ((activeDescendant instanceof ListItem) && xAccessible.equals(((ListItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new ListItem(xAccessible);
                    }
                } else if (Build.DEBUG) {
                    System.out.println(i + "th selected child is not accessible");
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                if (Build.DEBUG) {
                    System.err.println("IndexOutOfBoundsException caught for AccessibleList.getAccessibleSelection(" + i + ")");
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }
    }

    class ListItem extends java.awt.Component implements javax.accessibility.Accessible {

        protected XAccessible unoAccessible;

        public ListItem(XAccessible xAccessible) {
            unoAccessible = xAccessible;
        }

        public Object[] create(Object[] targetSet) {
            try {
                java.util.ArrayList<ListItem> list = new java.util.ArrayList<ListItem>(targetSet.length);
                for (int i=0; i < targetSet.length; i++) {
                    XAccessible xAccessible = UnoRuntime.queryInterface(
                        XAccessible.class, targetSet[i]);
                    if (xAccessible != null) {
                        list.add(new ListItem(xAccessible));
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
                        javax.accessibility.AccessibleContext ac = new AccessibleListItem(xAccessibleContext);
                        if (ac != null) {
                            ac.setAccessibleParent(List.this);
                            accessibleContext = ac;
                        }
                        AccessibleStateAdapter.setComponentState(this, xAccessibleContext.getAccessibleStateSet());
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                }
            }
            return accessibleContext;
        }

        protected class AccessibleListItem extends javax.accessibility.AccessibleContext {

            XAccessibleContext unoAccessibleContext;

            public AccessibleListItem(XAccessibleContext xAccessibleContext) {
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
                    return List.this.getLocale();
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
                    javax.accessibility.Accessible activeDescendant = List.this.activeDescendant;
                    if ((activeDescendant instanceof ListItem) && ((ListItem) activeDescendant).unoAccessible.equals(xAccessible)) {
                        child = activeDescendant;
                    } else if (xAccessible != null) {
                        child = new ListItem(xAccessible);
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                } catch (com.sun.star.uno.RuntimeException e) {
                }
                return child;
            }

            /** Returns the state set of this object */
            public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
                try {
                    return AccessibleStateAdapter.getAccessibleStateSet(ListItem.this,
                        unoAccessibleContext.getAccessibleStateSet());
                } catch (com.sun.star.uno.RuntimeException e) {
                    return AccessibleStateAdapter.getDefunctStateSet();
                }
            }

            /** Gets the AccessibleComponent associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleComponent getAccessibleComponent() {
                try {
                    XAccessibleComponent unoAccessibleComponent = UnoRuntime.queryInterface(XAccessibleComponent.class, unoAccessibleContext);
                    return (unoAccessibleComponent != null) ?
                        new AccessibleComponentImpl(unoAccessibleComponent) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleAction associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleAction getAccessibleAction() {
                try {
                    XAccessibleAction unoAccessibleAction = UnoRuntime.queryInterface(XAccessibleAction.class, unoAccessibleContext);
                    return (unoAccessibleAction != null) ?
                        new AccessibleActionImpl(unoAccessibleAction) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleText associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleText getAccessibleText() {

                if (disposed)
                    return null;

                try {
                    XAccessibleText unoAccessibleText = UnoRuntime.queryInterface(XAccessibleText.class, unoAccessibleContext);
                    return (unoAccessibleText != null) ?
                        new AccessibleTextImpl(unoAccessibleText) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleValue associated with this object that has a graphical representation */
            public javax.accessibility.AccessibleValue getAccessibleValue() {
                try {
                    XAccessibleValue unoAccessibleValue = UnoRuntime.queryInterface(XAccessibleValue.class, unoAccessibleContext);
                    return (unoAccessibleValue != null) ?
                        new AccessibleValueImpl(unoAccessibleValue) : null;
                } catch (com.sun.star.uno.RuntimeException e) {
                    return null;
                }
            }

            /** Gets the AccessibleText associated with this object presenting text on the display */
            public javax.accessibility.AccessibleIcon[] getAccessibleIcon() {
                try {
                    XAccessibleImage unoAccessibleImage = UnoRuntime.queryInterface(XAccessibleImage.class, unoAccessibleContext);
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

