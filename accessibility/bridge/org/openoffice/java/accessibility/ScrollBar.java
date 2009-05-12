/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScrollBar.java,v $
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

import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.swing.SwingConstants;

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

/**
 */
public class ScrollBar extends Component implements SwingConstants, javax.accessibility.Accessible {

    public ScrollBar(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleScrollBar();
    }

    protected class AccessibleScrollBar extends AccessibleUNOComponent implements
        javax.accessibility.AccessibleAction {

        protected XAccessibleAction unoAccessibleAction;
        protected int actionCount = 0;

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleScrollBar() {
            super();
            unoAccessibleAction = (XAccessibleAction) UnoRuntime.queryInterface(
                XAccessibleAction.class, unoAccessibleContext);
            if (unoAccessibleAction != null) {
                actionCount = unoAccessibleAction.getAccessibleActionCount();
            }
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.SCROLL_BAR;
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
    }
}

