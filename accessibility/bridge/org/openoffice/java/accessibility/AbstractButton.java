/*************************************************************************
 *
 *  $RCSfile: AbstractButton.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:30 $
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

import java.text.BreakIterator;

import javax.accessibility.AccessibleAction;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.accessibility.AccessibleText;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;

import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.uno.UnoRuntime;

import drafts.com.sun.star.accessibility.*;

/**
 */
public abstract class AbstractButton extends Component {

    protected AbstractButton() {
        super();
    }

    protected class AccessibleAbstractButtonListener extends AccessibleUNOComponentListener {

        protected AccessibleAbstractButtonListener() {
            super();
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.ARMED:
                    fireStatePropertyChange(AccessibleState.ARMED, enable);
                    break;
                case AccessibleStateType.CHECKED:
                    fireStatePropertyChange(AccessibleState.CHECKED, enable);
                    break;
                case AccessibleStateType.PRESSED:
                    fireStatePropertyChange(AccessibleState.PRESSED, enable);
                    break;
                default:
                    super.setComponentState(state, enable);
                    break;
            }
        }
    }

    protected abstract class AccessibleAbstractButton extends AccessibleUNOComponent
        implements AccessibleAction /*, AccessibleExtendedComponent */ {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleAbstractButton() {
            super();
        }

        /*
        * AccessibleContext
        */

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = super.getAccessibleStateSet();
            try {
                XAccessibleStateSet unoAccessibleStateSet =
                    unoAccessible.getAccessibleContext().getAccessibleStateSet();
                if (unoAccessibleStateSet.contains(AccessibleStateType.ARMED)) {
                    states.add(AccessibleState.ARMED);
                }
                if (unoAccessibleStateSet.contains(AccessibleStateType.PRESSED)) {
                    states.add(AccessibleState.PRESSED);
                }
                if (unoAccessibleStateSet.contains(AccessibleStateType.CHECKED)) {
                    states.add(AccessibleState.CHECKED);
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            } catch (NullPointerException e) {
            }
            return states;
        }

        /** Gets the AccessibleAction associated with this object that supports one or more actions */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            return this;
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleText getAccessibleText() {
            try {
                XAccessibleText unoAccessibleText = (XAccessibleText)
                    UnoRuntime.queryInterface(XAccessibleText.class,unoAccessibleComponent);
                if (unoAccessibleText != null) {
                    return new AccessibleTextImpl(unoAccessibleText);
                } else {
                    return null;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Gets the locale of the component */
        public java.util.Locale getLocale() throws java.awt.IllegalComponentStateException {
            // The java loader sets the UI locale as default at startup
            return java.util.Locale.getDefault();
        }

        /*
        * AccessibleAction
        */

        /** Performs the specified Action on the object */
        public boolean doAccessibleAction(int param) {
            if (param == 0) {
                // Actions of MenuItems may also be performed if the item is not
                // visible, so just try ..
                try {
                    XAccessibleComponent xAccessibleComponent = AbstractButton.this.unoAccessibleComponent;
                    if (xAccessibleComponent != null) {
                        // Query for XAccessibleValue interface
                        XAccessibleAction xAccessibleAction = (XAccessibleAction)
                            UnoRuntime.queryInterface(XAccessibleAction.class,
                            xAccessibleComponent);

                        if (xAccessibleAction != null) {
                            return xAccessibleAction.doAccessibleAction(0);
                        }
                    }
                }

                // Catch any type of uno exception
                catch(com.sun.star.uno.Exception e) {
                    return false;
                }
            }

            return false;
        }

        /** Returns a description of the specified action of the object */
        public java.lang.String getAccessibleActionDescription(int param) {
            return javax.swing.UIManager.getString("AbstractButton.clickText");
        }

        /** Returns the number of accessible actions available in this object */
        public int getAccessibleActionCount() {
            return 1;
        }
    }
}


