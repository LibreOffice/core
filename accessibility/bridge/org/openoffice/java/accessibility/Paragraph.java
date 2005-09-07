/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Paragraph.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:40:35 $
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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleText;

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

/**
 */
public class Paragraph extends Container implements javax.accessibility.Accessible {

    protected Paragraph(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(javax.accessibility.AccessibleRole.TEXT, xAccessible, xAccessibleContext);
    }

    protected class AccessibleParagraphListener extends AccessibleContainerListener {

        protected AccessibleParagraphListener() {
            super();
        }

        protected void setComponentState(short state, boolean enable) {
            switch (state) {
                case AccessibleStateType.EDITABLE:
                    fireStatePropertyChange(javax.accessibility.AccessibleState.EDITABLE, enable);
                    break;
                case AccessibleStateType.MULTI_LINE:
                    fireStatePropertyChange(javax.accessibility.AccessibleState.MULTI_LINE, enable);
                    break;
                case AccessibleStateType.SINGLE_LINE:
                    break;
                default:
                    super.setComponentState(state, enable);
                    break;
            }
        }


        protected void handleVisibleDataChanged() {
            if (Paragraph.this.isFocusOwner()) {
                AccessibleContext ac = accessibleContext;
                if (ac != null) {
                    AccessibleText at = ac.getAccessibleText();
                    if (at != null) {
                        int pos = at.getCaretPosition();
                        // Simulating a caret event here should help at tools
                        // that re not aware of the paragraph approach of OOo.
                        firePropertyChange(ac.ACCESSIBLE_CARET_PROPERTY,
                            new Integer(-1), new Integer(pos));
                    }
                }
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.CARET_CHANGED:
                    firePropertyChange(accessibleContext.ACCESSIBLE_CARET_PROPERTY,
                        Component.toNumber(event.OldValue),
                        Component.toNumber(event.NewValue));
                    break;
                case AccessibleEventId.VISIBLE_DATA_CHANGED:
                case AccessibleEventId.BOUNDRECT_CHANGED:
                    // Whenever a paragraph gets inserted above the currently
                    // focused one, this is the only event that will occur for.
                    handleVisibleDataChanged();
                default:
                    super.notifyEvent(event);
                break;
            }
        }
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleParagraphListener();
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleParagraph();
    }

    protected class AccessibleParagraph extends AccessibleContainer {

        protected AccessibleParagraph() {
            // Don't do the queryInterface on XAccessibleText already ..
            super(false);
            /* Since getAccessibleText() is heavily used by the java access
             * bridge for gnome and the gnome at-tools, we do a query interface
             * here and remember the result.
             */
            accessibleText = AccessibleHypertextImpl.get(unoAccessibleContext);
        }

        /*
        * AccessibleContext
        */

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.TEXT;
        }

        /** Gets the AccessibleEditableText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleEditableText getAccessibleEditableText() {

            if (disposed)
                return null;

            try {
                XAccessibleEditableText unoAccessibleText = (XAccessibleEditableText)
                    UnoRuntime.queryInterface(XAccessibleEditableText.class,
                    unoAccessibleComponent);
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

