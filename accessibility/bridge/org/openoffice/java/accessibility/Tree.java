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

public class Tree extends DescendantManager implements javax.accessibility.Accessible {

    protected Tree(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
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
                    descendant = new TreeItem(unoAccessible);
                }
            }
            setActiveDescendant(descendant);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    protected void add(XAccessible unoAccessible) {
        if (unoAccessible != null) {
            firePropertyChange(AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                null, new TreeItem(unoAccessible));
        }
    }

    protected void remove(XAccessible unoAccessible) {
        if (unoAccessible != null) {
            firePropertyChange(AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                new TreeItem(unoAccessible), null);
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
    protected class AccessibleTreeListener extends AccessibleDescendantManagerListener {

        protected AccessibleTreeListener() {
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

                case AccessibleEventId.LISTBOX_ENTRY_EXPANDED:
                    firePropertyChange(AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                        AccessibleState.COLLAPSED, AccessibleState.EXPANDED);
                    break;

                case AccessibleEventId.LISTBOX_ENTRY_COLLAPSED:
                    firePropertyChange(AccessibleContext.ACCESSIBLE_STATE_PROPERTY,
                        AccessibleState.EXPANDED, AccessibleState.COLLAPSED);
                    break;

                default:
                    super.notifyEvent(event);
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleTreeListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleTree();
    }

    protected class AccessibleTree extends AccessibleDescendantManager implements javax.accessibility.AccessibleExtendedTable {

        protected XAccessibleTable unoAccessibleTable;

        public AccessibleTree() {
            unoAccessibleTable = UnoRuntime.queryInterface(XAccessibleTable.class, unoAccessibleContext);
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.TREE;
        }

        /** Returns the AccessibleTable interface of this object */
        public javax.accessibility.AccessibleTable getAccessibleTable() {
            return ( unoAccessibleTable != null ) ? this : null;
        }

        /** Returns the specified Accessible child of the object */
        public javax.accessibility.Accessible getAccessibleChild(int i) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Tree.this.activeDescendant;
                    if ((activeDescendant instanceof TreeItem) && xAccessible.equals(((TreeItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TreeItem(xAccessible);
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
                XAccessible xAccessible = unoAccessibleComponent.getAccessibleAtPoint(
                    new com.sun.star.awt.Point(p.x, p.y));
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Tree.this.activeDescendant;
                    if ((activeDescendant instanceof TreeItem) && xAccessible.equals(((TreeItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TreeItem(xAccessible);
                    }
                }
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
                XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Tree.this.activeDescendant;
                    if ((activeDescendant instanceof TreeItem) && xAccessible.equals(((TreeItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TreeItem(xAccessible);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }

         /*
        * AccessibleTable
        */

        /** Returns the Accessible at a specified row and column in the table. */
        public javax.accessibility.Accessible getAccessibleAt(int r, int c) {
            javax.accessibility.Accessible child = null;
            try {
                XAccessible xAccessible = unoAccessibleTable.getAccessibleCellAt(r,c);
                if (xAccessible != null) {
                    // Re-use the active descandant wrapper if possible
                    javax.accessibility.Accessible activeDescendant = Tree.this.activeDescendant;
                    if ((activeDescendant instanceof TreeItem) && xAccessible.equals(((TreeItem) activeDescendant).unoAccessible)) {
                        child = activeDescendant;
                    } else {
                        child = new TreeItem(xAccessible);
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return child;
        }

        /** Returns the caption for the table. */
        public javax.accessibility.Accessible getAccessibleCaption() {
            // Not yet supported.
            return null;
        }

        /** Returns the number of columns in the table. */
        public int getAccessibleColumnCount() {
             try {
                return unoAccessibleTable.getAccessibleColumnCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
       }

        /** Returns the description text of the specified column in the table. */
        public javax.accessibility.Accessible getAccessibleColumnDescription(int c) {
            try {
                return new javax.swing.JLabel(
                    unoAccessibleTable.getAccessibleColumnDescription(c));
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return null;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /**
         * Returns the number of columns occupied by the Accessible
         * at a specified row and column in the table.
         */
        public int getAccessibleColumnExtentAt(int r, int c) {
            try {
                return unoAccessibleTable.getAccessibleColumnExtentAt(r,c);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return 0;
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Returns the column headers as an AccessibleTable. */
        public javax.accessibility.AccessibleTable getAccessibleColumnHeader() {
            // Not yet supported
            return null;
        }

        /** Returns the number of rows in the table. */
        public int getAccessibleRowCount() {
             try {
                return unoAccessibleTable.getAccessibleRowCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Returns the description of the specified row in the table. */
        public javax.accessibility.Accessible getAccessibleRowDescription(int r) {
            try {
                return new javax.swing.JLabel(
                    unoAccessibleTable.getAccessibleRowDescription(r));
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return null;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /**
         * Returns the number of rows occupied by the Accessible
         * at a specified row and column in the table.
         */
        public int getAccessibleRowExtentAt(int r, int c) {
            try {
                return unoAccessibleTable.getAccessibleRowExtentAt(r,c);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return 0;
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Returns the row headers as an AccessibleTable. */
        public javax.accessibility.AccessibleTable getAccessibleRowHeader() {
            // Not yet supported
            return null;
        }

        /** Returns the summary description of the table. */
        public javax.accessibility.Accessible getAccessibleSummary() {
            // Not yet supported.
            return null;
        }

        /** Returns the selected columns in a table. */
        public int[] getSelectedAccessibleColumns() {
            try {
                return unoAccessibleTable.getSelectedAccessibleColumns();
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Returns the selected rows in a table. */
        public int[] getSelectedAccessibleRows() {
            try {
                return unoAccessibleTable.getSelectedAccessibleRows();
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Returns a boolean value indicating whether the specified column is selected. */
        public boolean isAccessibleColumnSelected(int c) {
            try {
                return unoAccessibleTable.isAccessibleColumnSelected(c);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return false;
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        /** Returns a boolean value indicating whether the specified row is selected. */
        public boolean isAccessibleRowSelected(int r) {
            try {
                return unoAccessibleTable.isAccessibleRowSelected(r);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return false;
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        /**
         * Returns a boolean value indicating whether the accessible
         * at a specified row and column is selected.
         */
        public boolean isAccessibleSelected(int r, int c) {
            try {
                return unoAccessibleTable.isAccessibleSelected(r,c);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return false;
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }

        /** Sets the caption for the table. */
        public void setAccessibleCaption(javax.accessibility.Accessible accessible) {
            // Not supported by the UNO Accessibility API
        }

        /** Sets the description text of the specified column in the table. */
        public void setAccessibleColumnDescription(int param, javax.accessibility.Accessible accessible) {
            // Not supported by the UNO Accessibility API
        }

        /** Sets the column headers. */
        public void setAccessibleColumnHeader(javax.accessibility.AccessibleTable accessibleTable) {
            // Not supported by the UNO Accessibility API
        }

        /** Sets the description text of the specified row of the table. */
        public void setAccessibleRowDescription(int param, javax.accessibility.Accessible accessible) {
            // Not supported by the UNO Accessibility API
        }

        /** Sets the row headers. */
        public void setAccessibleRowHeader(javax.accessibility.AccessibleTable accessibleTable) {
            // Not supported by the UNO Accessibility API
        }

        /** Sets the summary description of the table */
        public void setAccessibleSummary(javax.accessibility.Accessible accessible) {
            // Not supported by the UNO Accessibility API
        }

        /** Returns the column number of an index in the table */
        public int getAccessibleColumn(int index) {
            try {
                return unoAccessibleTable.getAccessibleColumn(index);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return -1;
            } catch (com.sun.star.uno.RuntimeException e) {
                return -1;
            }
        }

        /** Returns the index of a specified row and column in the table. */
        public int getAccessibleIndex(int r, int c) {
            try {
                return unoAccessibleTable.getAccessibleIndex(r,c);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return -1;
            } catch (com.sun.star.uno.RuntimeException e) {
                return -1;
            }
        }

        /** Returns the row number of an index in the table */
        public int getAccessibleRow(int index) {
            try {
                return unoAccessibleTable.getAccessibleRow(index);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return -1;
            } catch (com.sun.star.uno.RuntimeException e) {
                return -1;
            }
        }
   }

    class TreeItem extends java.awt.Component implements javax.accessibility.Accessible {

        protected XAccessible unoAccessible;

        public TreeItem(XAccessible xAccessible) {
            unoAccessible = xAccessible;
        }

        public Object[] create(Object[] targetSet) {
            try {
                java.util.ArrayList<TreeItem> list = new java.util.ArrayList<TreeItem>(targetSet.length);
                for (int i=0; i < targetSet.length; i++) {
                    XAccessible xAccessible = UnoRuntime.queryInterface(
                        XAccessible.class, targetSet[i]);
                    if (xAccessible != null) {
                        list.add(new TreeItem(xAccessible));
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
                        javax.accessibility.AccessibleContext ac = new AccessibleTreeItem(xAccessibleContext);
                        ac.setAccessibleParent(Tree.this);
                        accessibleContext = ac;
                    }
                } catch (com.sun.star.uno.RuntimeException e) {
                }
            }
            return accessibleContext;
        }

        protected class AccessibleTreeItem extends javax.accessibility.AccessibleContext
            implements javax.accessibility.AccessibleSelection {

            XAccessibleContext unoAccessibleContext;
            XAccessibleSelection unoAccessibleSelection;

            public AccessibleTreeItem(XAccessibleContext xAccessibleContext) {
                unoAccessibleContext = xAccessibleContext;
                unoAccessibleSelection = UnoRuntime.queryInterface(XAccessibleSelection.class, xAccessibleContext);
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
                    return Tree.this.getLocale();
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
                    javax.accessibility.Accessible activeDescendant = Tree.this.activeDescendant;
                    if ((activeDescendant instanceof TreeItem) && ((TreeItem) activeDescendant).unoAccessible.equals(xAccessible)) {
                        child = activeDescendant;
                    } else if (xAccessible != null) {
                        child = new TreeItem(xAccessible);
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                } catch (com.sun.star.uno.RuntimeException e) {
                }
                return child;
            }

            /** Returns the state set of this object */
            public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
                try {
                    return AccessibleStateAdapter.getAccessibleStateSet(TreeItem.this,
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

            /** Returns the AccessibleSelection interface for this object */
            public javax.accessibility.AccessibleSelection getAccessibleSelection() {
                return (unoAccessibleSelection != null) ? this : null;
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

            /*
             * AccessibleSelection
             */

            /** Returns an Accessible representing the specified selected child of the object */
            public javax.accessibility.Accessible getAccessibleSelection(int i) {
                javax.accessibility.Accessible child = null;
                try {
                    XAccessible xAccessible = unoAccessibleContext.getAccessibleChild(i);
                    if (xAccessible != null) {
                        child = new TreeItem(xAccessible);
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                } catch (com.sun.star.uno.RuntimeException e) {
                }
                return child;
            }

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
}

