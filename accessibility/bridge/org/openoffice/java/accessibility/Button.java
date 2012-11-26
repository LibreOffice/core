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
            } catch (com.sun.star.lang.IllegalArgumentException e) {
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

