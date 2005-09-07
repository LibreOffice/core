/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ComboBox.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:36:08 $
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

import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleState;
import javax.accessibility.AccessibleStateSet;

import javax.swing.SwingConstants;


/**
 */
public class ComboBox extends Container {
    private XAccessibleAction unoAccessibleAction = null;

    public ComboBox(XAccessible xAccessible, XAccessibleContext xAccessibleContext) {
        super(javax.accessibility.AccessibleRole.COMBO_BOX, xAccessible,
            xAccessibleContext);
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

    /** Creates the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext createAccessibleContext() {
        return new AccessibleComboBox();
    }

    protected class AccessibleComboBox extends AccessibleContainer
        implements javax.accessibility.AccessibleAction {
        /**
        * Though the class is abstract, this should be called by all sub-classes
        */
        protected AccessibleComboBox() {
            super();
        }

        /*
        * AccessibleContext
        */

        /** Gets the AccessibleAction associated with this object that supports one or more actions */
        public javax.accessibility.AccessibleAction getAccessibleAction() {
            if (unoAccessibleAction == null) {
                unoAccessibleAction = (XAccessibleAction) UnoRuntime.queryInterface(XAccessibleAction.class,
                        unoAccessibleContext);

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
                } catch (com.sun.star.uno.Exception e) {
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
