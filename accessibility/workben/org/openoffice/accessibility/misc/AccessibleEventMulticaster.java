/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.accessibility.misc;

import com.sun.star.lang.EventObject;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;

/**
 *
 */
public class AccessibleEventMulticaster implements XAccessibleEventListener {

    private final XAccessibleEventListener a;
    private final XAccessibleEventListener b;

    /** Creates a new instance of AccessibleEventMulticaster */
    protected AccessibleEventMulticaster(XAccessibleEventListener a,
                                         XAccessibleEventListener b) {
        this.a = a;
        this.b = b;
    }

    protected XAccessibleEventListener remove(XAccessibleEventListener l) {
        if (l == a)
            return b;
        if (l == b)
            return a;
        XAccessibleEventListener a2 = remove(a, l);
        XAccessibleEventListener b2 = remove(b, l);
        if (a2 == a && b2 == b) {
            return this; // not found
        }
        return add(a2, b2);
    }

    public void notifyEvent(AccessibleEventObject accessibleEventObject) {
        a.notifyEvent(accessibleEventObject);
        b.notifyEvent(accessibleEventObject);
    }

    public void disposing(EventObject eventObject) {
        a.disposing(eventObject);
        b.disposing(eventObject);
    }

    public static XAccessibleEventListener add(XAccessibleEventListener a, XAccessibleEventListener b) {
        if (a == null)
            return b;
        if (b == null)
            return a;
        return new AccessibleEventMulticaster(a,b);
    }

    public static XAccessibleEventListener remove(XAccessibleEventListener l, XAccessibleEventListener oldl) {
        if (l == oldl || l == null) {
            return null;
        } else if (l instanceof AccessibleEventMulticaster) {
            return ((AccessibleEventMulticaster) l).remove(oldl);
        } else {
            return l;
        }
    }

}
