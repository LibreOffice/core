/*************************************************************************
 *
 *  $RCSfile: java_environment.java,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-13 17:22:40 $
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

package com.sun.star.lib.uno.environments.java;

import com.sun.star.lib.sandbox.Disposable;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

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
public final class java_environment implements IEnvironment, Disposable {
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

    // @see com.sun.star.uno.IEnvironment#dispose
    public void dispose() {
// TODO???
//       synchronized (proxies) {
//           if (!proxies.isEmpty()) {
//               list();
//           }
//           proxies.clear();
//       }
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
        public Object register(Object object, String oid, Type type) {
            synchronized (map) {
                cleanUp();
                Level1Entry l1 = getLevel1Entry(oid);
                if (l1 != null) {
                    Level2Entry l2 = l1.get(type);
                    if (l2 != null) {
                        Object o = l2.get();
                        if (o != null) {
                            l2.acquire();
                            return o;
                        }
                    }
                }
                // TODO  If a holder references an unreachable object, but still
                // has a positive count, it is replaced with a new holder
                // (referencing a reachable object, and with a count of 1).  Any
                // later calls to revoke that should decrement the count of the
                // previous holder would now decrement the count of the new
                // holder, removing it prematurely.  This is a design flaw that
                // will be fixed when IEnvironment.revokeInterface is changed to
                // no longer use counting.  (And this problem is harmless, as
                // currently a holder either references a strongly held object
                // and uses register/revoke to control it, or references a
                // weakly held proxy and never revokes it.)
                if (l1 == null) {
                    l1 = new Level1Entry();
                    map.put(oid, l1);
                }
                l1.add(new Level2Entry(oid, type, object, queue));
            }
            return object;
        }

        public boolean revoke(String oid, Type type) {
            synchronized (map) {
                Level1Entry l1 = getLevel1Entry(oid);
                Level2Entry l2 = null;
                if (l1 != null) {
                    l2 = l1.get(type);
                    if (l2 != null && l2.release()) {
                        removeLevel2Entry(oid, l1, l2);
                    }
                }
                cleanUp();
                return l2 != null;
            }
        }

        public Object get(String oid, Type type) {
            synchronized (map) {
                Level1Entry l1 = getLevel1Entry(oid);
                return l1 == null ? null : l1.find(type);
            }
        }

        // must only be called while synchronized on map:
        private void cleanUp() {
            for (;;) {
                Level2Entry l2 = (Level2Entry) queue.poll();
                if (l2 == null) {
                    break;
                }
                // It is possible that a Level2Entry e1 for the OID/type pair
                // (o,t) becomes weakly reachable, then another Level2Entry e2
                // is registered for the same pair (o,t) (a new Level2Entry is
                // created since now e1.get() == null), and only then e1 is
                // enqueued.  To not erroneously remove the new e2 in that case,
                // check whether the map still contains e1:
                String oid = l2.getOid();
                Level1Entry l1 = getLevel1Entry(oid);
                if (l1 != null && l1.get(l2.getType()) == l2) {
                    removeLevel2Entry(oid, l1, l2);
                }
            }
        }

        // must only be called while synchronized on map:
        private Level1Entry getLevel1Entry(String oid) {
            return (Level1Entry) map.get(oid);
        }

        // must only be called while synchronized on map:
        private void removeLevel2Entry(String oid, Level1Entry l1,
                                       Level2Entry l2) {
            if (l1.remove(l2)) {
                map.remove(oid);
            }
        }

        private static final class Level1Entry {
            // must only be called while synchronized on map:
            public Level2Entry get(Type type) {
                for (Iterator i = list.iterator(); i.hasNext();) {
                    Level2Entry l2 = (Level2Entry) i.next();
                    if (l2.getType().equals(type)) {
                        return l2;
                    }
                }
                return null;
            }

            // must only be called while synchronized on map:
            public Object find(Type type) {
                // First, look for an exactly matching entry; then, look for an
                // arbitrary entry for a subtype of the request type:
                for (Iterator i = list.iterator(); i.hasNext();) {
                    Level2Entry l2 = (Level2Entry) i.next();
                    if (l2.getType().equals(type)) {
                        Object o = l2.get();
                        if (o != null) {
                            return o;
                        }
                    }
                }
                for (Iterator i = list.iterator(); i.hasNext();) {
                    Level2Entry l2 = (Level2Entry) i.next();
                    if (type.isSupertypeOf(l2.getType())) {
                        Object o = l2.get();
                        if (o != null) {
                            return o;
                        }
                    }
                }
                return null;
            }

            // must only be called while synchronized on map:
            public void add(Level2Entry l2) {
                list.add(l2);
            }

            // must only be called while synchronized on map:
            public boolean remove(Level2Entry l2) {
                list.remove(l2);
                return list.isEmpty();
            }

            private final LinkedList list = new LinkedList(); // of Level2Entry
        }

        private static final class Level2Entry extends WeakReference {
            public Level2Entry(String oid, Type type, Object object,
                               ReferenceQueue queue) {
                super(object, queue);
                this.oid = oid;
                this.type = type;
            }

            public String getOid() {
                return oid;
            }

            public Type getType() {
                return type;
            }

            // must only be called while synchronized on map:
            public void acquire() {
                ++count;
            }

            // must only be called while synchronized on map:
            public boolean release() {
                return --count == 0;
            }

            private final String oid;
            private final Type type;
            private int count = 1;
        }

        private final HashMap map = new HashMap();
            // from OID (String) to Level1Entry
        private final ReferenceQueue queue = new ReferenceQueue();
    }

    private boolean isProxy(Object object) {
        return object instanceof com.sun.star.lib.uno.Proxy;
    }

    private static final Registry localObjects = new Registry();

    private final Object context;
    private final Registry proxies = new Registry();
}
