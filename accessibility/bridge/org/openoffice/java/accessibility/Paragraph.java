/*************************************************************************
 *
 *  $RCSfile: Paragraph.java,v $
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

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.*;

/**
 */
public class Paragraph extends Container implements javax.accessibility.Accessible {

    boolean multiLine = false;
    boolean editable = false;

    protected Paragraph(XAccessible xAccessible, XAccessibleContext xAccessibleContext, XAccessibleStateSet xAccessibleStateSet) {
        super(javax.accessibility.AccessibleRole.TEXT, xAccessible, xAccessibleContext);
        editable = xAccessibleStateSet.contains(AccessibleStateType.EDITABLE);
        multiLine = xAccessibleStateSet.contains(AccessibleStateType.MULTILINE);
    }

    protected class AccessibleParagraphListener extends AccessibleContainerListener {

        protected AccessibleParagraphListener() {
            super();
        }

        /** Fires the appropriate PropertyChangeEvent */
        protected void handleTextChangedEvent(Object any1, Object any2) {
            Object[] values = new Object[2];
            try {
                if (AnyConverter.isObject(any1)) {
                    com.sun.star.awt.Selection s = (com.sun.star.awt.Selection)
                        AnyConverter.toObject(Component.SelectionType, any1);
                    if (s != null) {
                        // Since there is nothing like a "range" object in the JAA yet,
                        // the Integer[2] is a private negotiation with the JABG
                        Integer[] deleted = { new Integer(s.Min), new Integer(s.Max) };
                        values[0] = deleted;
                        if (Build.DEBUG) {
                            System.err.println("text range (" + s.Min + "," + s.Max + ") deleted");
                        }
                    }
                }

                if (AnyConverter.isObject(any2)) {
                    com.sun.star.awt.Selection s = (com.sun.star.awt.Selection)
                        AnyConverter.toObject(Component.SelectionType, any2);
                    if (s != null) {
                        // Since there is nothing like a "range" object in the JAA yet,
                        // the Integer[2] is a private negotiation with the JABG
                        Integer[] inserted = { new Integer(s.Min), new Integer(s.Max) };
                        values[1] = inserted;
                        if (Build.DEBUG) {
                            System.err.println("text range (" + s.Min + "," + s.Max + ") inserted");
                        }
                    }
                }
                firePropertyChange(AccessibleContext.ACCESSIBLE_TEXT_PROPERTY, values[0], values[1]);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.EDITABLE:
                    editable = enable;
                    fireStatePropertyChange(javax.accessibility.AccessibleState.EDITABLE, enable);
                    break;
                case AccessibleStateType.MULTILINE:
                    multiLine = enable;
                    fireStatePropertyChange(javax.accessibility.AccessibleState.MULTI_LINE, enable);
                    break;
                case AccessibleStateType.SINGLE_LINE:
                    break;
                default:
                    super.setComponentState(state, enable);
                    break;
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
                    handleTextChangedEvent(event.OldValue, event.NewValue);
                    break;
                case AccessibleEventId.ACCESSIBLE_CARET_EVENT:
                    firePropertyChange(accessibleContext.ACCESSIBLE_CARET_PROPERTY, Component.toNumber(event.OldValue), Component.toNumber(event.NewValue));
                    break;
                default:
                    super.notifyEvent(event);
                    break;
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleParagraphListener();
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleParagraph();
        }
        return accessibleContext;
    }

    protected class AccessibleParagraph extends AccessibleContainer {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleParagraph() {
            super();
        }

        /** Returns an AccessibleStateSet that contains corresponding Java states to the UAA state types */
        protected javax.accessibility.AccessibleStateSet getAccessibleStateSetImpl(XAccessibleStateSet unoAS) {
            javax.accessibility.AccessibleStateSet states = super.getAccessibleStateSetImpl(unoAS);

            if (editable) {
                states.add(javax.accessibility.AccessibleState.EDITABLE);
            }
            if (multiLine) {
                states.add(javax.accessibility.AccessibleState.MULTI_LINE);
            } else {
                states.add(javax.accessibility.AccessibleState.SINGLE_LINE);
            }

            return states;
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.TEXT;
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

        /** Gets the AccessibleEditableText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleEditableText getAccessibleEditableText() {
            try {
                XAccessibleEditableText unoAccessibleText = (XAccessibleEditableText)
                    UnoRuntime.queryInterface(XAccessibleEditableText.class,unoAccessibleComponent);
                if (unoAccessibleText != null) {
                    return new AccessibleEditableTextImpl(unoAccessibleText);
                } else {
                    return null;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Gets the AccessibleAction associated with this object that has a graphical representation */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            try {
                XAccessibleAction unoAccessibleAction = (XAccessibleAction)
                    UnoRuntime.queryInterface(XAccessibleAction.class, unoAccessibleComponent);
                return (unoAccessibleAction != null) ?
                    new AccessibleActionImpl(unoAccessibleAction) : null;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

        /** Returns the relation set of this object */
        public javax.accessibility.AccessibleRelationSet getAccessibleRelationSet() {
            try {
                XAccessibleRelationSet unoAccessibleRelationSet =
                    unoAccessible.getAccessibleContext().getAccessibleRelationSet();
                if (unoAccessibleRelationSet == null) {
                    return super.getAccessibleRelationSet();
                }

                javax.accessibility.AccessibleRelationSet relationSet = new javax.accessibility.AccessibleRelationSet();
                int count = unoAccessibleRelationSet.getRelationCount();
                for (int i = 0; i < count; i++) {
                    AccessibleRelation unoAccessibleRelation = unoAccessibleRelationSet.getRelation(i);
                    switch (unoAccessibleRelation.RelationType) {
                        case AccessibleRelationType.CONTROLLED_BY:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                javax.accessibility.AccessibleRelation.CONTROLLED_BY,
                                getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                            break;
                        case AccessibleRelationType.CONTROLLER_FOR:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                javax.accessibility.AccessibleRelation.CONTROLLER_FOR,
                                getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                            break;
                        case AccessibleRelationType.LABELED_BY:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                javax.accessibility.AccessibleRelation.LABELED_BY,
                                getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                            break;
                        case AccessibleRelationType.MEMBER_OF:
                            relationSet.add(new javax.accessibility.AccessibleRelation(
                                javax.accessibility.AccessibleRelation.MEMBER_OF,
                                getAccessibleComponents(unoAccessibleRelation.TargetSet)));
                            break;
                        default:
                            break;
                    }
                }
                return relationSet;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                return super.getAccessibleRelationSet();
            } catch (com.sun.star.uno.RuntimeException e) {
                return super.getAccessibleRelationSet();
            }
        }
    }
}

