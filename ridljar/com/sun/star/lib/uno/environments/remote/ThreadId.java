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

package com.sun.star.lib.uno.environments.remote;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicLong;

import com.sun.star.uno.UnoRuntime;

public final class ThreadId {
    public static ThreadId createFresh() {
        long c = count.getAndIncrement();
        try {
            return new ThreadId((PREFIX + c).getBytes("UTF-8"));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("this cannot happen: " + e);
        }
    }

    public ThreadId(byte[] id) {
        this.id = id;
    }

    /**
     * Indicates whether some other object is equal to this one.
     *
     * @param obj the reference object with which to compare.
     * @return <code>true</code> if this object is the same as the obj argument;
     * <code>false</code> otherwise.
     *
     * @see java.lang.Object#equals
     */
    @Override
    public boolean equals(Object obj) {
        return obj instanceof ThreadId
            && Arrays.equals(id, ((ThreadId) obj).id);
    }

    /**
     * Returns a hash code value for the object.
     *
     * @return a hash code value for this object.
     * @see java.lang.Object#hashCode
     */
    @Override
    public int hashCode() {
        int h = hash;
        if (h == 0) {
            // Same algorithm as java.util.List.hashCode (also see Java 1.5
            // java.util.Arrays.hashCode(byte[])):
            h = 1;
            for (int i = 0; i < id.length; ++i) {
                h = 31 * h + id[i];
            }
            hash = h;
        }
        return h;
    }

    /**
     * Returns a string representation of the object.
     *
     * @return a string representation of the object.
     * @see java.lang.Object#toString
     */
    @Override
    public String toString() {
        StringBuffer b = new StringBuffer("[ThreadId:");
        for (int i = 0; i < id.length; ++i) {
            String n = Integer.toHexString(id[i] & 0xFF);
            if (n.length() == 1) {
                b.append('0');
            }
            b.append(n);
        }
        b.append(']');
        return b.toString();
    }

    public byte[] getBytes() {
        return id;
    }

    private static final String PREFIX = "java:" + UnoRuntime.getUniqueKey() + ":";
    private static final AtomicLong count = new AtomicLong(0);

    private final byte[] id;
    private int hash = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
