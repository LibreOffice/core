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
 * then wrapped in a <code>WeakReference</code>, while the occurrences of values
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
 *
 * Note that this class does not actually implement the Map interface properly,
 * the type of the return value of the entrySet and values methods is wrong,
 * but the "implements Map" is retained for backward compatibility.
 */
public final class WeakMap<K,V> implements Map {

    /**
     * Declare the map as WeakReference instead of Entry because it makes the return
     * type signatures of values() and keySet() cleaner.
     */
    private final HashMap<K, WeakReference<V>> map = new HashMap<K, WeakReference<V>>();
    private final ReferenceQueue<V> queue = new ReferenceQueue<V>();

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
    public WeakMap(Map<K,V> m) {
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
    public boolean containsKey(/*K*/ Object key) {
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
    public boolean containsValue(Object /*WeakReference<V>*/ value) {
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
    public WeakReference<V> get(/*K*/ Object key) {
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
    public Object /*WeakReference<V>*/ put(/*K*/ Object key, /*V*/ Object value) {
        cleanUp();
        return map.put((K) key, new Entry<K,V>((K) key, (V) value, queue));
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
    public Object /*WeakReference<V>*/ remove(/*K*/ Object key) {
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
    public void putAll(Map/*<K,V>*/ m) {
        cleanUp();
        for (Iterator<Map.Entry<K,V>> i = m.entrySet().iterator(); i.hasNext();) {
            Map.Entry<K,V> e = i.next();
            K k = e.getKey();
            map.put(k, new Entry<K,V>(k, e.getValue(), queue));
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
    public Set<K> keySet() {
        return map.keySet();
    }

    /**
     * Returns a collection view of the values contained in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return a collection view of the values contained in this map
     */
    public Collection<WeakReference<V>> values() {
        return map.values();
    }

    /**
     * Returns a collection view of the mappings contained in this map.
     *
     * <p>This is a non-modifying operation.</p>
     *
     * @return a collection view of the mappings contained in this map
     */
    public Set/*<Map.Entry<K,WeakReference<V>>>*/ entrySet() {
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
    public static <T> T getValue(Object /*WeakReference<T>*/ ref) {
        return ref == null ? null : ((WeakReference<T>) ref).get();
    }

    /**
     * cleanUp() must only be called from within modifying methods.  Otherwise,
     * the implementations of entrySet, keySet and values would break
     * (Specifically, iterating over the collections returned by those
     * methods), as non-modifying methods might modify the underlying map.
     **/
    private void cleanUp() {
        for (;;) {
            Entry<K,V> e = (Entry<K,V>) queue.poll();
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

    private static final class Entry<K,V> extends WeakReference<V>
        implements DisposeListener
    {
        private final K key;

        private Entry(K key, V value, ReferenceQueue<V> queue) {
            super(value, queue);
            this.key = key;
            if (value instanceof DisposeNotifier) {
                ((DisposeNotifier) value).addDisposeListener(this);
            }
        }

        /**
         * @see DisposeListener#notifyDispose(DisposeNotifier)
         */
        public void notifyDispose(DisposeNotifier source) {
            clear();
            enqueue();
        }
    }

}
