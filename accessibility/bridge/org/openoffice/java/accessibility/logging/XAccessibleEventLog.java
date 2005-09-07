/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XAccessibleEventLog.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:44:03 $
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

package org.openoffice.java.accessibility.logging;

import org.openoffice.java.accessibility.AccessibleObjectFactory;

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
                logMessage(accessibleEventObject.Source, "Retrieved active descendant event.");
                break;
            case AccessibleEventId.STATE_CHANGED:
                logStateChange(accessibleEventObject.Source,
                    accessibleEventObject.OldValue,
                    accessibleEventObject.NewValue);
                break;
            case AccessibleEventId.CHILD:
                logMessage(accessibleEventObject.Source, "Retrieved children event.");
                break;
           case AccessibleEventId.BOUNDRECT_CHANGED:
                logMessage(accessibleEventObject.Source, "Retrieved boundrect changed event.");
                break;
           case AccessibleEventId.VISIBLE_DATA_CHANGED:
                logMessage(accessibleEventObject.Source, "Retrieved visible data changed event.");
                break;
           case AccessibleEventId.INVALIDATE_ALL_CHILDREN:
                logMessage(accessibleEventObject.Source, "Retrieved invalidate children event.");
                break;
            default:
                break;
        }
    }

    public void logStateChange(Object o, Object any1, Object any2) {
        try {
            if (AnyConverter.isShort(any1)) {
                logStateChange(o, AnyConverter.toShort(any1), " is no longer ");
            }

            if (AnyConverter.isShort(any2)) {
                logStateChange(o, AnyConverter.toShort(any2), " is now ");
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
    }

    public void logStateChange(Object o, short n, String s) {
        switch(n) {
            case AccessibleStateType.ACTIVE:
                logMessage(o, s + javax.accessibility.AccessibleState.ACTIVE);
                break;
            case AccessibleStateType.ARMED:
                logMessage(o, s + javax.accessibility.AccessibleState.ARMED);
                break;
            case AccessibleStateType.CHECKED:
                logMessage(o, s + javax.accessibility.AccessibleState.CHECKED);
                break;
            case AccessibleStateType.ENABLED:
                logMessage(o, s + javax.accessibility.AccessibleState.ENABLED);
                break;
            case AccessibleStateType.FOCUSED:
                logMessage(o, s + javax.accessibility.AccessibleState.FOCUSED);
                break;
            case AccessibleStateType.PRESSED:
                logMessage(o, s + javax.accessibility.AccessibleState.PRESSED);
                break;
            case AccessibleStateType.SELECTED:
                logMessage(o, s + javax.accessibility.AccessibleState.SELECTED);
                break;
            case AccessibleStateType.SHOWING:
                logMessage(o, s + javax.accessibility.AccessibleState.SHOWING);
                break;
            case AccessibleStateType.VISIBLE:
                logMessage(o, s + javax.accessibility.AccessibleState.VISIBLE);
                break;
            default:
                logMessage(o, s + "??? (FIXME)");
                break;
        }
    }

    protected static void logMessage(Object o, String s) {
        XAccessible xa = (XAccessible) UnoRuntime.queryInterface(XAccessible.class, o);
        logMessage((javax.accessibility.Accessible) AccessibleObjectFactory.getAccessibleComponent(xa), s);
    }

    protected static void logMessage(javax.accessibility.Accessible a, String s) {
        if (a != null) {
            logMessage(a.getAccessibleContext(), s);
        } else {
            logMessage(s);
        }
    }

    protected static void logMessage(javax.accessibility.AccessibleContext ac, String s) {
        if (ac != null) {
            logMessage("[" + ac.getAccessibleRole() + "] "
                + ac.getAccessibleName() + ": " + s);
        } else {
            logMessage(s);
        }
    }

    protected static void logMessage(String s) {
        System.err.println(s);
    }
}
