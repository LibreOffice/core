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

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.uno.UnoRuntime;
import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.util.Arrays;

public final class ThreadId {
    public static ThreadId createFresh() {
        BigInteger c;
        synchronized (PREFIX) {
            c = count;
            count = count.add(BigInteger.ONE);
        }
        try {
            return new ThreadId((PREFIX + c).getBytes("UTF-8"));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("this cannot happen: " + e);
        }
    }

    public ThreadId(byte[] id) {
        this.id = id;
    }

    public boolean equals(Object obj) {
        return obj instanceof ThreadId
            && Arrays.equals(id, ((ThreadId) obj).id);
    }

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

    private static final String PREFIX
    = "java:" + UnoRuntime.getUniqueKey() + ":";
    private static BigInteger count = BigInteger.ZERO;

    private byte[] id;
    private int hash = 0;
}
