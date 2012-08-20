/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.java.accessibility.logging;

import java.awt.Component;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/**
 *
 */
public class XAccessibleEventLog implements XAccessibleEventListener {

    private static XAccessibleEventLog theEventListener = null;

    private static java.util.Hashtable<String, Component> proxyList = new java.util.Hashtable<String, Component>();

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
        XAccessibleEventBroadcaster broadcaster = UnoRuntime.queryInterface(XAccessibleEventBroadcaster.class, xac);
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
        XAccessibleContext xac = UnoRuntime.queryInterface(XAccessibleContext.class, o);
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
