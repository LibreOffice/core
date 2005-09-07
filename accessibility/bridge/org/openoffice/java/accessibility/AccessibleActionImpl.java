/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleActionImpl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:30:24 $
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
