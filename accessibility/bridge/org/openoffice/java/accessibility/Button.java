/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Button.java,v $
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

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

class Button extends AbstractButton implements javax.accessibility.Accessible {

    public Button(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(xAccessible, xAccessibleContext);
    }

    protected XAccessibleEventListener createEventListener() {
        return new AccessibleButtonListener();
    }

    protected class AccessibleButtonListener
        extends AccessibleUNOComponentListener {
        protected AccessibleButtonListener() {
            super();
        }

        protected javax.accessibility.AccessibleContext getContext( Object any ) {
            try {
                XAccessible xAccessible = (XAccessible)
                    AnyConverter.toObject( AccessibleObjectFactory.XAccessibleType, any );

                    javax.accessibility.Accessible accessible =
                    (javax.accessibility.Accessible) Button.this.getComponent( xAccessible );

                    return accessible.getAccessibleContext();
            } catch( com.sun.star.uno.Exception e ) {
                return null;
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.CHILD:
                    java.awt.Component c = getComponent(unoAccessible);

                    Object values[] = { null, null };

                    if (AnyConverter.isObject(event.OldValue)) {
                        values[0] = getContext( event.OldValue );
                    }

                    if (AnyConverter.isObject(event.NewValue)) {
                        values[1] = getContext( event.NewValue);
                    }

                    firePropertyChange(javax.accessibility.AccessibleContext.ACCESSIBLE_CHILD_PROPERTY,
                        values[0], values[1]);
                    break;

                default:
                    super.notifyEvent(event);
            }
        }
    }

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleButton();
    }

    protected java.awt.Component getComponent(XAccessible unoAccessible) {
        java.awt.Component c = AccessibleObjectFactory.getAccessibleComponent(unoAccessible);

        if (c == null) {
            c = AccessibleObjectFactory.createAccessibleComponent(unoAccessible);

            if (c instanceof javax.accessibility.Accessible) {
                ((javax.accessibility.Accessible) c).getAccessibleContext()
                 .setAccessibleParent(this);
            }

            if( c instanceof java.awt.Container ) {
                AccessibleObjectFactory.populateContainer((java.awt.Container) c, unoAccessible.getAccessibleContext() );
            }
        }

        return c;
    }

    protected class AccessibleButton extends AccessibleAbstractButton {

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.PUSH_BUTTON;
        }

        /** Returns the number of accessible children of the object */
        public int getAccessibleChildrenCount() {
            try {
                return unoAccessibleContext.getAccessibleChildCount();
            } catch (com.sun.star.uno.RuntimeException e) {
                return 0;
            }
        }

        /** Returns the specified Accessible child of the object */
        public synchronized javax.accessibility.Accessible getAccessibleChild( int i) {
            try {
                 return (javax.accessibility.Accessible) getComponent( unoAccessibleContext.getAccessibleChild(i) );
            } catch (com.sun.star.uno.RuntimeException e) {
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            }
            return null;
        }

        /*
        * AccessibleComponent
        */

        /** Returns the Accessible child, if one exists, contained at the local coordinate Point */
        public javax.accessibility.Accessible getAccessibleAt(java.awt.Point p) {
            try {
                java.awt.Component c = getComponent(unoAccessibleComponent.getAccessibleAtPoint(
                            new com.sun.star.awt.Point(p.x, p.y)));

                return (javax.accessibility.Accessible) c;
            } catch (com.sun.star.uno.RuntimeException e) {
                return null;
            }
        }

    }
}

