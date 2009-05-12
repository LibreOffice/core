/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WeakTable.java,v $
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

import java.lang.reflect.Method;

import java.util.Hashtable;
import java.util.Observable;
import java.util.Observer;

public class WeakTable extends Observable {
    private static final boolean DEBUG = false;

    private static final void DEBUG(String dbg) {
        if (DEBUG)
            System.err.println(">>> com.sun.star.lib.sandbox.WeakTable - " + dbg);
    }

    static WeakTable weakTable = new WeakTable();

    public Hashtable hash = new Hashtable();

    synchronized private void pput(Object key, Cachable cachable) {
          DEBUG("put:" + key + " " + cachable);

        synchronized(hash) {
            Object hardObject = cachable.getHardObject();
            Class weakClass   = cachable.getClass();

            WeakRef weakRef = new WeakRef(key, hardObject);
            cachable.setWeakRef(weakRef);

            hash.put(key, new WeakEntry(weakClass, weakRef));
        }
        setChanged();
        notifyObservers();
    }

    synchronized private Cachable pget(Object key) {
          DEBUG("get:" + key);

        String error = null;

        Cachable cachable = null;

        synchronized(hash) {
            WeakEntry weakEntry = (WeakEntry)hash.get(key);
            if(weakEntry != null) {
                try {
                    //          Class sig[] = new Class[1];
                    //          sig[0] = weakRef.getClass();
                    //          Method method = weakClass.getMethod("<init>", sig);

                    cachable = (Cachable)weakEntry.weakClass.newInstance();
                    cachable.setWeakRef(weakEntry.weakRef);
                }
                catch(IllegalAccessException e) {
                    error = "#### WeakTable.get:" + e;
                }
                catch(InstantiationException e) {
                    error = "#### WeakTable.get:" + e;
                }
            }
        }
        if(error != null)
            System.err.println(error);

        return cachable;
    }

    synchronized private void premove(Object key) {
        hash.remove(key);

        setChanged();
        notifyObservers();
    }

    synchronized private void pclear() {
        hash.clear();

        setChanged();
        notifyObservers();
    }

    public void addObserver(Observer observer) {
        super.addObserver(observer);

        observer.update(this, null);
    }

    public static void put(Object key, Cachable cachable) {
        weakTable.pput(key, cachable);
    }

    public static Cachable get(Object key) {
        return weakTable.pget(key);
    }

    public static void remove(Object key) {
        weakTable.premove(key);
    }

    public static void AddObserver(Observer observer) {
        weakTable.addObserver(observer);
    }

    public static void DeleteObserver(Observer observer) {
        weakTable.deleteObserver(observer);
    }

    static void clear() {
        weakTable.pclear();
    }

    static java.util.Enumeration elements() {
        return weakTable.hash.elements();
    }
}

