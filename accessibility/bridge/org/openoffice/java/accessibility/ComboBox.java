/*************************************************************************
 *
 *  $RCSfile: ComboBox.java,v $
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
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;
import javax.swing.SwingConstants;

import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

/**
 */
public class ComboBox extends Container {

    private XAccessibleAction unoAccessibleAction = null;

    public ComboBox(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(javax.accessibility.AccessibleRole.COMBO_BOX, xAccessible, xAccessibleContext);
    }

    /** Appends the specified component to the end of this container */
    public java.awt.Component add(java.awt.Component c) {
        // List should be always the first child
        if (c instanceof List) {
            return super.add(c, 0);
        } else {
            return super.add(c);
        }
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleComboBox();
        }
        return accessibleContext;
    }

    protected class AccessibleComboBox extends AccessibleContainer
        implements javax.accessibility.AccessibleAction {

        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleComboBox() {
            super();
        }

        /** Returns an AccessibleStateSet that contains corresponding Java states to the UAA state types */
        protected javax.accessibility.AccessibleStateSet getAccessibleStateSetImpl(XAccessibleStateSet unoAS) {
            javax.accessibility.AccessibleStateSet states = super.getAccessibleStateSetImpl(unoAS);

            try {
                if (unoAS != null) {
                    if (unoAS.contains(AccessibleStateType.EXPANDABLE)) {
                        states.add(AccessibleState.EXPANDABLE);
                    }
                    if (unoAS.contains(AccessibleStateType.EXPANDED)) {
                        states.add(AccessibleState.EXPANDED);
                    }
                    if (unoAS.contains(AccessibleStateType.COLLAPSED)) {
                        states.add(AccessibleState.COLLAPSED);
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
            if (unoAccessibleAction == null) {
                unoAccessibleAction = (XAccessibleAction)
                    UnoRuntime.queryInterface(XAccessibleAction.class, unoAccessibleContext);
                if (unoAccessibleAction == null) {
                    return null;
                }
            }
            return this;
        }

        /*
        * AccessibleAction
        */

        /** Performs the specified Action on the object */
        public boolean doAccessibleAction(int param) {
            if (param == 0) {
                try {
                    return unoAccessibleAction.doAccessibleAction(0);
                } catch(com.sun.star.uno.Exception e) {
                }
            }

            return false;
        }

        /** Returns a description of the specified action of the object */
        public java.lang.String getAccessibleActionDescription(int param) {
            return javax.swing.UIManager.getString("ComboBox.togglePopupText");
        }

        /** Returns the number of accessible actions available in this object */
        public int getAccessibleActionCount() {
            return 1;
        }
    }
}


