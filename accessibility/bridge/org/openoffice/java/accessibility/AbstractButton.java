/*************************************************************************
 *
 *  $RCSfile: AbstractButton.java,v $
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

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

/**
 */
public abstract class AbstractButton extends Component {

    protected AbstractButton(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    protected abstract class AccessibleAbstractButton extends AccessibleUNOComponent
        implements javax.accessibility.AccessibleAction {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleAbstractButton() {
            super();
        }

        /** Returns an AccessibleStateSet that contains corresponding Java states to the UAA state types */
        protected javax.accessibility.AccessibleStateSet getAccessibleStateSetImpl(XAccessibleStateSet unoAS) {
            javax.accessibility.AccessibleStateSet states = super.getAccessibleStateSetImpl(unoAS);

            try {
                if (unoAS != null) {
                    if (unoAS.contains(AccessibleStateType.ARMED)) {
                        states.add(AccessibleState.ARMED);
                    }
                    if (unoAS.contains(AccessibleStateType.PRESSED)) {
                        states.add(AccessibleState.PRESSED);
                    }
                    if (unoAS.contains(AccessibleStateType.CHECKED)) {
                        states.add(AccessibleState.CHECKED);
                    }
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }

            return states;
        }

        /*
        * AccessibleContext
        */

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
                        case AccessibleRelationType.MEMBER_OF:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.MEMBER_OF,
                                    getAccessibleComponents(
                                        unoAccessibleRelation.TargetSet)));
                            break;

                        case AccessibleRelationType.LABELED_BY:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.LABELED_BY,
                                    getAccessibleComponents(
                                        unoAccessibleRelation.TargetSet)));
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

        /*
        * AccessibleAction
        */

        /** Performs the specified Action on the object */
        public boolean doAccessibleAction(int param) {
            if (param == 0) {
                // HACK: this action might open a modal dialog and therefor block
                // until the dialog is closed. In case of this thread being the
                // AWT EventDispatcherThread this means, the opened dialog will
                // not be accessible, so deligate this request to another thread.
                if (java.awt.EventQueue.isDispatchThread()) {
                    Thread t = new Thread () {
                        public void run() {
                            AbstractButton.AccessibleAbstractButton.this.doAccessibleAction(0);
                        }
                    };
                    t.start();
                    return true;
                } else {
                    // Actions of MenuItems may also be performed if the item is not
                    // visible, so just try ..
                    try {
                        XAccessibleContext xAccessibleContext = unoAccessibleContext;
                        if (xAccessibleContext != null) {
                            // Query for XAccessibleAction interface
                            XAccessibleAction xAccessibleAction = (XAccessibleAction)
                                UnoRuntime.queryInterface(XAccessibleAction.class, xAccessibleContext);

                            if (xAccessibleAction != null) {
                                return xAccessibleAction.doAccessibleAction(0);
                            }
                        }
                    } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    } catch (com.sun.star.uno.RuntimeException e) {
                    }
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

