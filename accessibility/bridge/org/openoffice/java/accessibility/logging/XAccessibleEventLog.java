/*************************************************************************
 *
 *  $RCSfile: XAccessibleEventLog.java,v $
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

package org.openoffice.java.accessibility.logging;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/**
 *
 */
public class XAccessibleEventLog implements XAccessibleEventListener {

    private static XAccessibleEventLog theEventListener = null;

    /** Creates a new instance of UNOAccessibleEventListener */
    public XAccessibleEventLog() {
    }

    private static XAccessibleEventListener get() {
        if (theEventListener == null) {
            theEventListener = new XAccessibleEventLog();
        }
        return theEventListener;
    }

    public static void addEventListener(XAccessibleContext xac) {
        XAccessibleEventBroadcaster broadcaster = (XAccessibleEventBroadcaster)
            UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class, xac);
        if (broadcaster != null) {
            broadcaster.addEventListener(XAccessibleEventLog.get());
        }
    }

    public void disposing(com.sun.star.lang.EventObject eventObject) {
    }

    public void notifyEvent(com.sun.star.accessibility.AccessibleEventObject accessibleEventObject) {
        switch (accessibleEventObject.EventId) {
            case AccessibleEventId.ACTIVE_DESCENDANT_CHANGED:
                System.err.println("Retrieved active descendant event.");
                break;
            case AccessibleEventId.CHILD:
                System.err.println("Retrieved children event.");
                break;
           case AccessibleEventId.BOUNDRECT_CHANGED:
                System.err.println("Retrieved boundrect changed event.");
                break;
           case AccessibleEventId.VISIBLE_DATA_CHANGED:
                System.err.println("Retrieved visible data changed event.");
                break;
           case AccessibleEventId.INVALIDATE_ALL_CHILDREN:
                System.err.println("Retrieved invalidate children event.");
                break;
            default:
                break;
        }
    }

}
