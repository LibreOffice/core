/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WeakRef.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

public class WeakRef {
    static private final boolean DEBUG = false;

    private int refCnt;
    private Object ref;
    private Object key;

    WeakRef(Object key, Object ref) {
        this.key       = key;
        this.ref       = ref;
    }

    public Object getRef() {
        return ref;
    }

    public Object getKey() {
        return key;
    }

    int getRefCount() {
        return refCnt;
    }

    // the following two methods where synchronized, but need not to be
    public void incRefCnt() {
        refCnt ++;
    }

    public synchronized void decRefCnt() {
        refCnt --;
        if(refCnt <= 0) {
            WeakTable.remove(key);

            if(DEBUG)System.err.println("#### WeakRef - object freeed:" + key + " " + ref);
            if(ref instanceof Disposable) {
                ((Disposable)ref).dispose();
            }

            ref = null;
        }
    }

    public String toString() {
        return "WeakRef - " + key + " " + ((ref == null) ? "null" : ref.toString());
    }
}

