/*************************************************************************
 *
 *  $RCSfile: WeakTable.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
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

