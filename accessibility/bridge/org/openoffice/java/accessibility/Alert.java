/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Alert.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-01-31 18:22:17 $
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

import javax.accessibility.AccessibleRole;
import com.sun.star.accessibility.*;

public class Alert extends Dialog {

    protected Alert(java.awt.Frame owner, XAccessibleComponent xAccessibleComponent) {
        super(owner, xAccessibleComponent);
    }

    protected Alert(java.awt.Frame owner, String name, XAccessibleComponent xAccessibleComponent) {
        super(owner, name, xAccessibleComponent);
    }

    protected Alert(java.awt.Frame owner, String name, boolean modal, XAccessibleComponent xAccessibleComponent) {
        super(owner, name, modal, xAccessibleComponent);
    }

    /** Returns the AccessibleContext associated with this object */
    public javax.accessibility.AccessibleContext getAccessibleContext() {
        if (accessibleContext == null) {
            accessibleContext = new AccessibleAlert();
            accessibleContext.setAccessibleName(getTitle());
        }
        return accessibleContext;
    }

    protected class AccessibleAlert extends AccessibleDialog {

        protected AccessibleAlert() {
            super();
        }

        public AccessibleRole getAccessibleRole() {
            return AccessibleRole.ALERT;
        }
    };
}
