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
            unoAccessibleAction = UnoRuntime.queryInterface(
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
                XAccessibleValue unoAccessibleValue = UnoRuntime.queryInterface(XAccessibleValue.class, unoAccessibleContext);
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

