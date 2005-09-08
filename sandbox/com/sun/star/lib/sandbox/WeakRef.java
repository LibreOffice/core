/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WeakRef.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:10:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

