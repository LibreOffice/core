/*************************************************************************
 *
 *  $RCSfile: Separator.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-12-06 11:25:39 $
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

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.swing.SwingConstants;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleComponent;

/**
 */
public class Separator extends Component implements SwingConstants, Accessible {

    private int orientation = HORIZONTAL;

    public Separator(XAccessible accessible, XAccessibleComponent component, int orientation) {
        super();
        this.orientation = orientation;
        initialize(accessible, component);
    }

    public Separator(XAccessible accessible, XAccessibleComponent component) {
        super();
        initialize(accessible, component);
    }

    protected void initialize(XAccessible accessible, XAccessibleComponent component) {
        setFocusable(false);
        unoAccessible = accessible;
        unoAccessibleComponent = component;
    }
/*
        // To reflect focus and other component state changes, the accessibility
        // event listener must already be added here
        addAccessibleEventListener(new AccessibleSeparatorListener());
    }

    protected class AccessibleSeparatorListener extends AccessibleUNOComponentListener {
    }
*/
    /** Returns the AccessibleContext associated with this object */
    public AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleSeparator();
        }
        return accessibleContext;
    }

    protected class AccessibleSeparator extends AccessibleUNOComponent {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleSeparator() {
            super();
        }

        /** Gets the role of this object */
        public javax.accessibility.AccessibleRole getAccessibleRole() {
            return javax.accessibility.AccessibleRole.SEPARATOR;
        }

        /**
        * Get the state set of this object.
        *
        * @return an instance of AccessibleState containing the current state
        * of the object
        * @see AccessibleState
        */
        public AccessibleStateSet getAccessibleStateSet() {
            AccessibleStateSet states = super.getAccessibleStateSet();
            switch (orientation) {
                case HORIZONTAL:
                    states.add(AccessibleState.HORIZONTAL);
                    break;
                case VERTICAL:
                    states.add(AccessibleState.VERTICAL);
                    break;
                default:
                    break;
            }
            return states;
        }
    }
}


