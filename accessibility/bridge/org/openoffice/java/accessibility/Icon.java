/*************************************************************************
 *
 *  $RCSfile: Icon.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:36 $
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
import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;

import com.sun.star.awt.Point;
import com.sun.star.awt.Rectangle;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.uno.*;
import drafts.com.sun.star.accessibility.AccessibleEventId;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import drafts.com.sun.star.accessibility.AccessibleStateType;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleImage;

/**
 */
public class Icon extends Component implements Accessible {

    protected Icon(XAccessible accessible, XAccessibleComponent xAccessibleComponent) {
        super();
        initialize(accessible, xAccessibleComponent);
    }

    protected void initialize(XAccessible accessible, XAccessibleComponent xAccessibleComponent) {
        unoAccessible = accessible;
        unoAccessibleComponent = xAccessibleComponent;
        // To reflect focus and other component state changes, the accessibility
        // event listener must already be added here
        addAccessibleEventListener(new AccessibleIconListener());
    }

    protected class AccessibleIconListener extends AccessibleUNOComponentListener {

        protected AccessibleIconListener() {
            super();
        }

        /** Updates the accessible name and fires the appropriate PropertyChangedEvent */
        protected void handleNameChangedEvent(Object any) {
            try {
                // This causes the property change event to be fired in the VCL thread
                // context. If this causes problems, it has to be deligated to the java
                // dispatch thread ..
                if (accessibleContext != null) {
                    accessibleContext.setAccessibleName(AnyConverter.toString(any));
                }
            }
            catch (com.sun.star.lang.IllegalArgumentException e) {
            }
        }

        /** Called by OpenOffice process to notify property changes */
        public void notifyEvent(AccessibleEventObject event) {
            switch (event.EventId) {
                case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
                    // text changes already handled along with ACCESSIBLE_NAME_EVENT
                    break;
                default:
                    super.notifyEvent(event);
            }
        }
    }

    /** Returns the AccessibleContext associated with this object */
    public AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleIcon();
        }
        return accessibleContext;
    }

    protected class AccessibleIcon extends AccessibleUNOComponent
         /* implements AccessibleExtendedComponent */ {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleIcon() {
            super();
        }

        /** Gets the AccessibleText associated with this object presenting text on the display */
        public javax.accessibility.AccessibleIcon[] getAccessibleIcon() {
            try {
                XAccessibleImage unoAccessibleImage = (XAccessibleImage)
                    UnoRuntime.queryInterface(XAccessibleImage.class, unoAccessibleComponent);
                if (unoAccessibleImage != null) {
                    javax.accessibility.AccessibleIcon[] icons = { new AccessibleIconImpl(unoAccessibleImage) };
                    return icons;
                }
            } catch (com.sun.star.uno.RuntimeException e) {
            }
            return null;
        }

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
/*
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = super.getAccessibleStateSet();
            return states;
        }
*/
    }
}


