/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WeakMap.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.lib.util;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * A hash map that holds values of type <code>WeakReference</code>.
 *
 * <p>Like <code>HashMap</code>, this implementation provides all of the
 * optional map operations, and permits the <code>null</code> key.</p>
 *
 * <p>Also like <code>HashMap</code>, this implementation is not synchronized.
 * If multiple threads share an instance, and at least one of them executes any
 * modifying operations on the <code>WeakMap</code>, they have to use external
 * synchronization.</p>
 *
 * <p>Unlike other map implementations, <code>WeakMap</code> is asymmetric in
 * that <code>put</code> expects the given value to be a plain object that is
 * then wrapped in a <code>WeakReference</code>, while the occurences of values
 * in all other methods (<code>containsValue</code>, <code>entrySet</code>,
 * <code>equals</code>, <code>get</code>, <code>hashCode</code>,
 * <code>remove</code>, <code>values</code>, and also the return value of
 * <code>put</code>) expect already wrapped instances of
 * <code>WeakReference</code>.  That is, after <code>weakMap.put("key",
 * o)</code>, <code>weakMap.get("key").equals(o)</code> does not work as
 * na&iuml;vely expected; neither does
 * <code>weakMap1.putAll(weakMap2)</code>.</p>
 *
 * <p>At an arbitrary time after the <code>WeakReference</code> value of an
 * entry has been cleared by the garbage collector, the entry is automatically
 * removed from the map.</p>
 *
 * <p>Values placed into a <code>WeakMap</code> may optionally support the
 * <code>DisposeNotifier</code> interface.  For those that do, the associated
 * <code>WeakReference</code> wrappers are automatically cleared as soon as the
 * values are disposed.</p>
 */
public final class WeakMap implements Map {
    /**
     * Constructs an empty <code>WeakMap</code>.
     */
    public WeakMap() {}

    /**
     * Constructs a new <code>WeakMap</code> with the same mappings as the
     * specified <code>Map</code>.
     *
     * @param m the map whose mappings are to be placed in this map
     */
    public WeakMap(Map m) {
        putAll(m);
    }

    /**
     * Returns the number of key&ndash;value mappings in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return the number of key&ndash;value mappings in this map
     */
    public int size() {
        return map.size();
    }

    /**
     * Returns <code>true</code> if this map contains no key&ndash;value
     * mappings.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return <code>true</code> if this map contains no key&ndash;value
     * mappings
     */
    public boolean isEmpty() {
        return map.isEmpty();
    }

    /**
     * Returns <code>true</code> if this map contains a mapping for the
     * specified key.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @param key the key whose presence in this map is to be tested
     * @return <code>true</code> if this map contains a mapping for the
     * specified key
     */
    public boolean containsKey(Object key) {
        return map.containsKey(key);
    }

    /**
     * Returns <code>true</code> if this map maps one or more keys to the
     * specified value.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @param value the value whose presence in this map is to be tested
     * @return <code>true</code> if this map maps one or more keys to the
     * specified value
     */
    public boolean containsValue(Object value) {
        return map.containsValue(value);
    }

    /**
     * Returns the value to which the specified key is mapped in this map, or
     * <code>null</code> if the map contains no mapping for this key.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @param key the key whose associated value is to be returned
     *
     * @return the value to which this map maps the specified key, or
     * <code>null</code> if the map contains no mapping for this key
     */
    public Object get(Object key) {
        return map.get(key);
    }

    /**
     * Associates the specified value with the specified key in this map.
     *
     * <p>This is a modifying operation.</p>
     *
     * @param key the key with witch the specified value is to be associated
     * @param value the value to be associated with the specified key.  This
     * must be a plain object, which is then wrapped in a
     * <code>WeakReference</code>.
     * @return previous value associated with the specified key, or
     * <code>null</code> if there was no mapping for the key
     */
    public Object put(Object key, Object value) {
        cleanUp();
        return map.put(key, new Entry(key, value, queue));
    }

    /**
     * Removes the mapping for this key from this map if present.
     *
     * <p>This is a modifying operation.</p>
     *
     * @param key the key whose mapping is to be removed from the map
     * @return previous value associated with the specified key, or
     * <code>null</code> if there was no mapping for the key
     */
    public Object remove(Object key) {
        cleanUp();
        return map.remove(key);
    }

    /**
     * Copies all of the mappings from the specified map to this map.
     *
     * <p>This is a modifying operation.</p>
     *
     * @param m mappings to be stored in this map.  The values of those mappings
     * must be plain objects, which are then wrapped in instances of
     * <code>WeakReference</code>.
     */
    public void putAll(Map t) {
        cleanUp();
        for (Iterator i = t.entrySet().iterator(); i.hasNext();) {
            Map.Entry e = (Map.Entry) i.next();
            Object k = e.getKey();
            map.put(k, new Entry(k, e.getValue(), queue));
        }
    }

    /**
     * Removes all mappings from this map.
     *
     * <p>This is a modifying operation.</p>
     */
    public void clear() {
        cleanUp();
        map.clear();
    }

    /**
     * Returns a view of the keys contained in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return a set view of the keys contained in this map
     */
    public Set keySet() {
        return map.keySet();
    }

    /**
     * Returns a collection view of the values contained in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return a collection view of the values contained in this map
     */
    public Collection values() {
        return map.values();
    }

    /**
     * Returns a collection view of the mappings contained in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return a collection view of the mappings contained in this map
     */
    public Set entrySet() {
        return map.entrySet();
    }

    public boolean equals(Object o) {
        return map.equals(o);
    }

    public int hashCode() {
        return map.hashCode();
    }

    /**
     * Returns the referent of a <code>WeakReference</code>, silently handling a
     * <code>null</code> argument.
     *
     * <p>This static method is useful to wrap around the return values of
     * methods like <code>get</code>.</p>
     *
     * @param ref must be either an instance of <code>WeakReference</code> or
     * <code>null</code>
     * @return the referent of the specified <code>WeakReference</code>, or
     * <code>null</code> if <code>ref</code> is <code>null</code>
     */
    public static Object getValue(Object ref) {
        return ref == null ? null : ((WeakReference) ref).get();
    }

    // cleanUp must only be called from within modifying methods.  Otherwise,
    // the implementations of entrySet, keySet and values would break
    // (specificially, iterating over the collections returned by those
    // methods), as non-modifying methods might modify the underlying map.
    private void cleanUp() {
        for (;;) {
            Entry e = (Entry) queue.poll();
            if (e == null) {
                break;
            }
            // It is possible that an Entry e1 becomes weakly reachable, then
            // another Entry e2 is added to the map for the same key, and only
            // then e1 is enqueued.  To not erroneously remove the new e2 in
            // that case, check whether the map still contains e1:
            Object k = e.key;
            if (e == map.get(k)) {
                map.remove(k);
            }
        }
    }

    private static final class Entry extends WeakReference
        implements DisposeListener
    {
        public void notifyDispose(DisposeNotifier source) {
            Entry.this.clear(); // qualification needed for Java 1.3
            enqueue();
        }

        private Entry(Object key, Object value, ReferenceQueue queue) {
            super(value, queue);
            this.key = key;
            if (value instanceof DisposeNotifier) {
                ((DisposeNotifier) value).addDisposeListener(this);
            }
        }

        private final Object key;
    }

    private final HashMap map = new HashMap();
    private final ReferenceQueue queue = new ReferenceQueue();
}
