/*************************************************************************
 *
 *  $RCSfile: ComboBox.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:34 $
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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.swing.SwingConstants;

import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleComponent;

/**
 */
public class ComboBox extends Component implements javax.accessibility.Accessible {

    private XAccessibleAction unoAccessibleAction = null;
    private boolean editable = false;

    public ComboBox(XAccessible accessible, XAccessibleComponent component, XAccessibleAction action) {
        super();
        initialize(accessible, component, action);
    }

    private AccessibleContext accessibleTextContext = null;
    private AccessibleContext accessiblePopupMenuContext = null;

    protected void initialize(XAccessible accessible, XAccessibleComponent component,
        XAccessibleAction action) {
        unoAccessibleAction = action;
        unoAccessible = accessible;
        unoAccessibleComponent = component;
        // To reflect focus and other component state changes, the accessibility
        // event listener must already be added here
        addAccessibleEventListener(new AccessibleComboBoxListener());
//      accessiblePopupMenuContext = new AccessiblePopupMenu();
    }

    protected class AccessiblePopupMenu extends AccessibleContext implements javax.accessibility.Accessible {

        /** Gets the accessible context associated with this object */
        public javax.accessibility.AccessibleContext getAccessibleContext() {
            return this;
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.POPUP_MENU;
        }

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = new AccessibleStateSet();
            states.add(AccessibleState.ENABLED);
            return states;
        }

        /** Gets the locale of the component */
        public java.util.Locale getLocale() throws java.awt.IllegalComponentStateException {
            return ComboBox.this.getLocale();
        }

        /** Returns the number of accessible children of the object. */
        public int getAccessibleChildrenCount() {
            return 0;
        }

        /** Returns the specified Accessible child of the object. */
        public javax.accessibility.Accessible getAccessibleChild(int i) {
            return null;
        }

        /** Gets the 0-based index of this object in its accessible parent */
        public int getAccessibleIndexInParent() {
            return 0;
        }

        /** Gets the accessible parent of this object */
        public javax.accessibility.Accessible getAccessibleParent() {
            return ComboBox.this;
        }
    }

    protected class AccessibleComboBoxListener extends AccessibleUNOComponentListener {
    }

    /** Returns the AccessibleContext associated with this object */
    public AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleComboBox();
        }
        return accessibleContext;
    }

    protected class AccessibleComboBox extends AccessibleUNOComponent
        implements javax.accessibility.AccessibleAction {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleComboBox() {
            super();
        }

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.COMBO_BOX;
        }

        /** Returns the number of accessible children of the object */
        public int getAccessibleChildrenCount() {
            return editable ? 2 : 1;
        }

        /** Returns the specified Accessible child of the object */
        public javax.accessibility.Accessible getAccessibleChild(int i) {
            switch (i) {
            case 0:
                return (javax.accessibility.Accessible) accessiblePopupMenuContext;
            case 1:
                if (editable) {
                    return (javax.accessibility.Accessible) accessibleTextContext;
                }
            default:
                return null;
            }
        }

        /** Gets the AccessibleAction associated with this object that supports one or more actions */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            return this;
        }

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
/*
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = super.getAccessibleStateSet();
            return states;
        }
*/
        /*
        * AccessibleAction
        */

        /** Performs the specified Action on the object */
        public boolean doAccessibleAction(int param) {
            if (param == 0) {
                try {
                    return unoAccessibleAction.doAccessibleAction(0);
                } catch(com.sun.star.uno.Exception e) {
                }
            }

            return false;
        }

        /** Returns a description of the specified action of the object */
        public java.lang.String getAccessibleActionDescription(int param) {
            return javax.swing.UIManager.getString("ComboBox.togglePopupText");
        }

        /** Returns the number of accessible actions available in this object */
        public int getAccessibleActionCount() {
            return 1;
        }
    }
}


