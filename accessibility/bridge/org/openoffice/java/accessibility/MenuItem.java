/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MenuItem.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 15:08:33 $
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
