/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Label.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:38:37 $
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

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;


/**
 */
public class Label extends Component implements javax.accessibility.Accessible {
    protected Label(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleLabel();
    }

    protected class AccessibleLabel extends AccessibleUNOComponent {
        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleLabel() {
            super();
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.LABEL;
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleText getAccessibleText() {

            if (disposed)
                return null;

            try {
                XAccessibleText unoAccessibleText = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class,
                        unoAccessibleContext);

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
                        case AccessibleRelationType.LABEL_FOR:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.LABEL_FOR,
                                    getAccessibleComponents(
                                        unoAccessibleRelation.TargetSet)));

                            break;

                        case AccessibleRelationType.MEMBER_OF:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                    javax.accessibility.AccessibleRelation.MEMBER_OF,
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
    }
}
