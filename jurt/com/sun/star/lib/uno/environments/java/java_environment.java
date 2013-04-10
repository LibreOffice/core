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

package com.sun.star.lib.uno.environments.java;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;

/**
 * The java_environment is the environment where objects and
 * interfaces are registered, which are mapped out of java or
 * into java.
 *
 * <p>The java_environment implements the <code>IEnvironment</code> interface
 * defined in the uno runtime.</p>
 *
 * @see com.sun.star.uno.UnoRuntime
 * @see com.sun.star.uno.IEnvironment
 * @since UDK1.0
 */
public final class java_environment implements IEnvironment {
    public java_environment(Object context) {
        this.context = context;
    }

    // @see com.sun.star.uno.IEnvironment#getContext
    public Object getContext() {
        return context;
    }

    // @see com.sun.star.uno.IEnvironment#getName
    public String getName() {
        return "java";
    }

    // @see com.sun.star.uno.IEnvironment#registerInterface
    public Object registerInterface(Object object, String[] oid, Type type) {
        if (oid[0] == null) {
            oid[0] = UnoRuntime.generateOid(object);
        }
        return (isProxy(object) ? proxies : localObjects).register(
            object, oid[0], type);
    }

    /**
     * You have to revoke ANY interface that has been registered via this
     * method.
     *
     * @param oid object id of interface to be revoked
     * @param type the type description of the interface
     * @see com.sun.star.uno.IEnvironment#revokeInterface
     */
    public void revokeInterface(String oid, Type type) {
        if (!proxies.revoke(oid, type)) {
            localObjects.revoke(oid, type);
        }
    }

    /**
     * Retrieves an interface identified by its object id and type from this
     * environment.
     *
     * @param oid object id of interface to be retrieved
     * @param type the type description of the interface to be retrieved
     * @see com.sun.star.uno.IEnvironment#getRegisteredInterface
     */
    public Object getRegisteredInterface(String oid, Type type) {
        Object o = proxies.get(oid, type);
        if (o == null) {
            o = localObjects.get(oid, type);
        }
        return o;
    }

    /**
     * Retrieves the object identifier for a registered interface from this
     * environment.
     *
     * @param object a registered interface
     * @see com.sun.star.uno.IEnvironment#getRegisteredObjectIdentifier
     */
    public String getRegisteredObjectIdentifier(Object object) {
        return UnoRuntime.generateOid(object);
    }

    // @see com.sun.star.uno.IEnvironment#list
    public void list() {
// TODO???
//      synchronized (proxies) {
//          System.err.println("##### " + getClass().getName() + ".list: "
//                             + getName() + ", " + getContext());
//          for (Iterator it = proxies.values().iterator(); it.hasNext();) {
//              System.err.println("#### entry: " + it.next());
//          }
//      }
    }

    /**
     * Revokes all registered proxy interfaces.
     *
     * <p>This method should be part of <code>IEnvironment</code>.  It is called
     * from <code>com.sun.star.lib.uno.bridges.java_remote.<!--
     * -->java_remote_bridge.dispose</code>.</p>
     */
    public void revokeAllProxies() {
        proxies.clear();
    }

    // TODO  What's this???  java.lang.Object#equals requires reflexivity...
    //
    // Maybe this was hacked in so that different bridges use different
    // instances of java_environment.  That is desirable for the following
    // reason:  An OID is bridged in over bridge A, a proxy is created on the
    // Java side, and recorded in the java_environment.  The same OID is then
    // bridged in over another bridge B.  If there were only one
    // java_environment shared by both bridges, the proxy from bridge A would be
    // reused.  If now bridge A is taken down programatically (e.g., because
    // some controlling code somehow deduced that no objects are mapped over
    // that bridge any longer), but the proxy is still used by bridge B, using
    // the proxy would now result in errors.  The explicit API to control
    // bridges forbids to transparently share proxies between bridges, and using
    // different java_environment instances for different bridges is the way to
    // enforce this.
    public boolean equals(Object obj) {
        return false;
    }

    private static final class Registry {
        public synchronized Object register(
            Object object, String oid, Type type)
        {
            cleanUp();
            Level1Entry l1 = level1map.get(oid);
            if (l1 != null) {
                Level2Entry l2 = l1.level2map.get(type);
                if (l2 != null) {
                    Object o = l2.get();
                    if (o != null) {
                        l2.acquire();
                        return o;
                    }
                }
            }
            // TODO  If a holder references an unreachable object, but still has
            // a positive count, it is replaced with a new holder (referencing a
            // reachable object, and with a count of 1).  Any later calls to
            // revoke that should decrement the count of the previous holder
            // would now decrement the count of the new holder, removing it
            // prematurely.  This is a design flaw that will be fixed when
            // IEnvironment.revokeInterface is changed to no longer use
            // counting.  (And this problem is harmless, as currently a holder
            // either references a strongly held object and uses register/revoke
            // to control it, or references a weakly held proxy and never
            // revokes it.)
            if (l1 == null) {
                l1 = new Level1Entry();
                level1map.put(oid, l1);
            }
            l1.level2map.put(type, new Level2Entry(oid, type, object, queue));
            return object;
        }

        public synchronized boolean revoke(String oid, Type type) {
            Level1Entry l1 = level1map.get(oid);
            Level2Entry l2 = null;
            if (l1 != null) {
                l2 = l1.level2map.get(type);
                if (l2 != null && l2.release()) {
                    removeLevel2Entry(l1, oid, type);
                }
            }
            cleanUp();
            return l2 != null;
        }

        public synchronized Object get(String oid, Type type) {
            Level1Entry l1 = level1map.get(oid);
            return l1 == null ? null : l1.find(type);
        }

        public synchronized void clear() {
            level1map.clear();
            cleanUp();
        }

        // must only be called while synchronized on this Registry:
        private void cleanUp() {
            for (;;) {
                Object tmp = queue.poll();
                Level2Entry l2 = (Level2Entry) tmp;
                if (l2 == null) {
                    break;
                }
                // It is possible that a Level2Entry e1 for the OID/type pair
                // (o,t) becomes weakly reachable, then another Level2Entry e2
                // is registered for the same pair (o,t) (a new Level2Entry is
                // created since now e1.get() == null), and only then e1 is
                // enqueued.  To not erroneously remove the new e2 in that case,
                // check whether the map still contains e1:
                Level1Entry l1 = level1map.get(l2.oid);
                if (l1 != null && l1.level2map.get(l2.type) == l2) {
                    removeLevel2Entry(l1, l2.oid, l2.type);
                }
            }
        }

        // must only be called while synchronized on this Registry:
        private void removeLevel2Entry(Level1Entry l1, String oid, Type type) {
            l1.level2map.remove(type);
            if (l1.level2map.isEmpty()) {
                level1map.remove(oid);
            }
        }

        private static final class Level1Entry {
            // must only be called while synchronized on enclosing Registry:
            public Object find(Type type) {
                // First, look for an exactly matching entry; then, look for an
                // arbitrary entry for a subtype of the request type:
                Level2Entry l2 = level2map.get(type);
                if (l2 != null) {
                    Object o = l2.get();
                    if (o != null) {
                        return o;
                    }
                }
                for (Iterator<Level2Entry> i = level2map.values().iterator();
                     i.hasNext();)
                {
                    l2 = i.next();
                    if (type.isSupertypeOf(l2.type)) {
                        Object o = l2.get();
                        if (o != null) {
                            return o;
                        }
                    }
                }
                return null;
            }

            public final HashMap<Type, Level2Entry> level2map =
                new HashMap<Type, Level2Entry>();
        }

        private static final class Level2Entry extends WeakReference<Object> {
            public Level2Entry(
                String oid, Type type, Object object, ReferenceQueue<Object> queue)
            {
                super(object, queue);
                this.oid = oid;
                this.type = type;
            }

            // must only be called while synchronized on enclosing Registry:
            public void acquire() {
                ++count;
            }

            // must only be called while synchronized on enclosing Registry:
            public boolean release() {
                return --count == 0;
            }

            public final String oid;
            public final Type type;

            private int count = 1;
        }

        private final HashMap<String, Level1Entry> level1map =
            new HashMap<String, Level1Entry>();
        private final ReferenceQueue<Object> queue = new ReferenceQueue<Object>();
    }

    private boolean isProxy(Object object) {
        return object instanceof com.sun.star.lib.uno.Proxy;
    }

    private static final Registry localObjects = new Registry();

    private final Object context;
    private final Registry proxies = new Registry();
}
