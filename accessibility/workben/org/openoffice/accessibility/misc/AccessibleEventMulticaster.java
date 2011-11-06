/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
