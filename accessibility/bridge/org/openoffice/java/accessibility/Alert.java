/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Alert.java,v $
 * $Revision: 1.3 $
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
