/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AbstractButton.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:30:02 $
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

        /*
        * AccessibleContext
        */

        /** Gets the AccessibleAction associated with this object that supports one or more actions */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            return this;
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleText getAccessibleText() {

            if (disposed)
                return null;

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

