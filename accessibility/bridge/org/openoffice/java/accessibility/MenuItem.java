/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MenuItem.java,v $
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

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventListener;


class MenuItem extends ToggleButton {
    public MenuItem(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    protected class AccessibleMenuItemListener extends AccessibleUNOComponentListener {

        protected AccessibleMenuItemListener() {
        }

        protected void setComponentState(short state, boolean enable) {

            // #i56538# menu items in Java 1.5 are ARMED, not SELECTED
            if( state == com.sun.star.accessibility.AccessibleStateType.SELECTED )
                fireStatePropertyChange(javax.accessibility.AccessibleState.ARMED, enable);
            else
                super.setComponentState(state, enable);
        }
    };

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleMenuItemListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleMenuItem();
    }

    protected class AccessibleMenuItem extends AccessibleToggleButton {
        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.MENU_ITEM;
        }

        /** Gets the 0-based index of this object in its accessible parent */
        public int getAccessibleIndexInParent() {
            if (getAccessibleParent() instanceof Menu) {
                return ((Menu) getAccessibleParent()).indexOf(MenuItem.this);
            } else {
                return super.getAccessibleIndexInParent();
            }
        }

        /**
        * Gets the current state set of this object.
        *
        * @return an instance of <code>AccessibleStateSet</code>
        *    containing the current state set of the object
        * @see AccessibleState
        */
        public javax.accessibility.AccessibleStateSet getAccessibleStateSet() {
            javax.accessibility.AccessibleStateSet stateSet = super.getAccessibleStateSet();

            // #i56538# menu items in Java do not have SELECTABLE ..
            stateSet.remove(javax.accessibility.AccessibleState.SELECTABLE);

            // .. and also ARMED insted of SELECTED
            if( stateSet.remove(javax.accessibility.AccessibleState.SELECTED) )
                stateSet.add(javax.accessibility.AccessibleState.ARMED);

            return stateSet;
        }

    }
}
