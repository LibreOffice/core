/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.uno.protocols.urp;

import java.util.HashMap;

/**
 * An LRU cache for arbitrary objects.
 *
 * <p>This class is not synchronized, as any necessary synchronization will already
 * take place in the client.</p>
 */
final class Cache {
    /**
     * Create a cache.
     *
     * @param size the maximum cache size, must be between 0, inclusive, and
     * NOT_CACHED, exclusive.
     */
    public Cache(int size) {
        maxSize = size;
    }

    public int add(boolean[] found, Object content) {
        Entry e = map.get(content);
        found[0] = e != null;
        if (e == null) {
            if (map.size() < maxSize) {
                // There is still room for a new entry at the front:
                e = new Entry(content, map.size(), null, first);
                if (first == null) {
                    last = e;
                } else {
                    first.prev = e;
                }
                first = e;
            } else if (last != null) {
                // Take last entry out and recycle as new front:
                map.remove(last.content);
                e = last;
                e.content = content;
                if (first != last) {
                    // Reached only if maxSize > 1:
                    last = last.prev;
                    last.next = null;
                    e.prev = null;
                    e.next = first;
                    first.prev = e;
                    first = e;
                }
            } else {
                // Reached if maxSize == 0:
                return NOT_CACHED;
            }
            map.put(content, e);
        } else if (e != first) {
            // Move to front (reached only if maxSize > 1):
            e.prev.next = e.next;
            if (e.next == null) {
                last = e.prev;
            } else {
                e.next.prev = e.prev;
            }
            e.prev = null;
            e.next = first;
            first.prev = e;
            first = e;
        }
        return e.index;
    }

    public static final int NOT_CACHED = 0xFFFF;

    private static final class Entry {
        public Entry(Object content, int index, Entry prev, Entry next) {
            this.content = content;
            this.index = index;
            this.prev = prev;
            this.next = next;
        }

        public Object content;
        public int index;
        public Entry prev;
        public Entry next;
    }

    // first/last form a list of 0 to maxSize entries, most recently used first;
    // map contains the same entries; each entry has a unique index in the range
    // 0 to maxSize - 1
    private final int maxSize;
    private final HashMap<Object, Entry> map = new HashMap<Object, Entry>(); // from Object to Entry
    private Entry first = null;
    private Entry last = null;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
