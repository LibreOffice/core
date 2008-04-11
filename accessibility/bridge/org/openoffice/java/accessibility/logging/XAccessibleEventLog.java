/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XAccessibleEventLog.java,v $
 * $Revision: 1.9 $
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

package org.openoffice.java.accessibility.logging;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/**
 *
 */
public class XAccessibleEventLog implements XAccessibleEventListener {

    private static XAccessibleEventLog theEventListener = null;

    private static java.util.Hashtable proxyList = new java.util.Hashtable();

    /** Creates a new instance of UNOAccessibleEventListener */
    public XAccessibleEventLog() {
    }

    private static XAccessibleEventListener get() {
        if (theEventListener == null) {
            theEventListener = new XAccessibleEventLog();
        }
        return theEventListener;
    }

    public static void addEventListener(XAccessibleContext xac, java.awt.Component c) {
        XAccessibleEventBroadcaster broadcaster = (XAccessibleEventBroadcaster)
            UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class, xac);
        if (broadcaster != null) {
            broadcaster.addEventListener(XAccessibleEventLog.get());

            // remember the proxy objects
            synchronized (proxyList) {
//                proxyList.put(UnoRuntime.generateOid(xac), new WeakReference(c));
                proxyList.put(UnoRuntime.generateOid(xac), c);
            }
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
            case AccessibleStateType.SENSITIVE:
                logMessage(o, s + "sensitive");
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
        XAccessibleContext xac = (XAccessibleContext) UnoRuntime.queryInterface(XAccessibleContext.class, o);
        if( xac != null ) {
            String oid = UnoRuntime.generateOid(xac);
            synchronized (proxyList) {
                  logMessage( (javax.accessibility.Accessible) proxyList.get( oid ), s );
//                WeakReference r = (WeakReference) proxyList.get( oid );
//                if(r != null) {
//                    System.err.println( "*** Warning *** event is " + r.get() );
//                    logMessage( (javax.accessibility.Accessible) r.get(), s );
//                } else {
//                    System.err.println( "*** Warning *** event source not found in broadcaster list" );
//                }
            }
        } else
            System.err.println( "*** Warning *** event source does not implement XAccessibleContext" );
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
