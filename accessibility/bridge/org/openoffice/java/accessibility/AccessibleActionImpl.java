/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleActionImpl.java,v $
 * $Revision: 1.5 $
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

import com.sun.star.accessibility.XAccessibleAction;

/** The AccessibleActionWrapper maps the calls to the java AccessibleAction interface
 *  to the corresponding methods of the UNO XAccessibleAction interface.
 */
public class AccessibleActionImpl implements javax.accessibility.AccessibleAction {

    protected XAccessibleAction unoObject;

    /** Creates new AccessibleActionWrapper */
    public AccessibleActionImpl(XAccessibleAction xAccessibleAction) {
        unoObject = xAccessibleAction;
    }

    public boolean doAccessibleAction(int param) {
        try {
            return unoObject.doAccessibleAction(param);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return false;
        } catch (com.sun.star.uno.RuntimeException e) {
            return false;
        }
    }

    public java.lang.String getAccessibleActionDescription(int param) {
        try {
            return unoObject.getAccessibleActionDescription(param);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return null;
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    public int getAccessibleActionCount() {
        try {
            return unoObject.getAccessibleActionCount();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }
}
