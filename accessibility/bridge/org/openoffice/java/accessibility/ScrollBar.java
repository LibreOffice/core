/*************************************************************************
 *
 *  $RCSfile: ScrollBar.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:39 $
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

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.swing.SwingConstants;

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.AccessibleEventId;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleValue;

/**
 */
public class ScrollBar extends Component implements SwingConstants, Accessible {

    private int orientation = HORIZONTAL;

    public ScrollBar(XAccessible accessible, XAccessibleComponent component, int orientation) {
        super();
        this.orientation = orientation;
        initialize(accessible, component);
    }

    public ScrollBar(XAccessible accessible, XAccessibleComponent component) {
        super();
        initialize(accessible, component);
    }

    protected void initialize(XAccessible accessible, XAccessibleComponent component) {
        unoAccessible = accessible;
        unoAccessibleComponent = component;
        // To reflect focus and other component state changes, the accessibility
        // event listener must already be added here
        addAccessibleEventListener(new AccessibleScrollBarListener());
    }

    protected void fireValuePropertyChanged(Number oldValue, Number newValue) {
        java.awt.Toolkit.getDefaultToolkit().getSystemEventQueue().invokeLater(
            new PropertyChangeBroadcaster(accessibleContext.ACCESSIBLE_VALUE_PROPERTY,
            oldValue, newValue));
    }

    protected class AccessibleScrollBarListener extends AccessibleUNOComponentListener {
        /** Passes the event to all registered listeners (if any) */
        protected void handleValueEvent(Number oldValue, Number newValue) {
            // Notify property event listeners
            ScrollBar.this.fireValuePropertyChanged(oldValue, newValue);
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch(event.EventId) {
                case AccessibleEventId.ACCESSIBLE_VALUE_EVENT:
                    // Fire the appropriate PropertyChangeEvent
                    handleValueEvent(toNumber(event.OldValue), toNumber(event.NewValue));
                    break;
                default:
                    super.notifyEvent(event);
            }
        }
    }

    /** Returns the AccessibleContext associated with this object */
    public AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleScrollBar();
        }
        return accessibleContext;
    }

    protected class AccessibleScrollBar extends AccessibleUNOComponent implements
        javax.accessibility.AccessibleValue, javax.accessibility.AccessibleAction {

        protected XAccessibleValue unoAccessibleValue;
        protected XAccessibleAction unoAccessibleAction;
        protected int actionCount = 0;

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleScrollBar() {
            super();
            unoAccessibleValue = (XAccessibleValue) UnoRuntime.queryInterface(
                XAccessibleValue.class, unoAccessibleComponent);
            unoAccessibleAction = (XAccessibleAction) UnoRuntime.queryInterface(
                XAccessibleAction.class, ScrollBar.this.unoAccessibleComponent);
            if (unoAccessibleAction != null) {
                actionCount = unoAccessibleAction.getAccessibleActionCount();
            }
        }

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.SCROLL_BAR;
        }

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = super.getAccessibleStateSet();
            switch (orientation) {
                case HORIZONTAL:
                    states.add(AccessibleState.HORIZONTAL);
                    break;
                case VERTICAL:
                    states.add(AccessibleState.VERTICAL);
                    break;
                default:
                    break;
            }
            return states;
        }

        /** Gets the AccessibleValue associated with this object that supports a Numerical value */
        public javax.accessibility.AccessibleValue getAccessibleValue() {
            return this;
        }

        /** Gets the AccessibleAction associated with this object that supports one or more actions */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            return this;
        }

        /*
        * AccessibleAction
        */

        /** Performs the specified Action on the object */
        public boolean doAccessibleAction(int param) {
            if (param < actionCount) {
                try {
                    return unoAccessibleAction.doAccessibleAction(param);
                } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                }
            }
            return false;
        }

        /** Returns a description of the specified action of the object */
        public java.lang.String getAccessibleActionDescription(int param) {
            if(param < actionCount) {
                try {
                    return unoAccessibleAction.getAccessibleActionDescription(param);
                } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                }
            }
            return null;
        }

        /** Returns the number of accessible actions available in this object */
        public int getAccessibleActionCount() {
            return actionCount;
        }

        /*
        * AccessibleValue
        */

        /** Get the minimum value of this object as a Number */
        public java.lang.Number getMinimumAccessibleValue() {
            try {
                return toNumber(unoAccessibleValue.getMinimumValue());
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Get the value of this object as a Number */
        public java.lang.Number getCurrentAccessibleValue() {
            try {
                return toNumber(unoAccessibleValue.getCurrentValue());
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }
        /** Get the maximum value of this object as a Number */
        public java.lang.Number getMaximumAccessibleValue() {
            try {
                return toNumber(unoAccessibleValue.getMaximumValue());
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Set the value of this object as a Number */
        public boolean setCurrentAccessibleValue(java.lang.Number number) {
            try {
                return unoAccessibleValue.setCurrentValue(number);
            } catch (com.sun.star.uno.RuntimeException e) {
                return false;
            }
        }
    }
}

