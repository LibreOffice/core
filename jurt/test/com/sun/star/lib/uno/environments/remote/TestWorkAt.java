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

class TestWorkAt implements TestIWorkAt {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static final int MESSAGES = 35;


    int _counter;

    int _sync_counter;
    int _async_counter;

    Thread _sync_thread;
    Thread _async_thread;

    boolean _passedAync = true;
    boolean _notified = false;

    public void syncCall() throws Throwable {
        ++ _sync_counter;

        if(_async_counter != MESSAGES)
            _passedAync = false;

        if(_sync_thread == null)
            _sync_thread = Thread.currentThread();

//          if(_sync_thread != Thread.currentThread())
//              _passedAync = false;

        if(DEBUG) System.err.println("syncCall:" + _sync_counter + " " + _passedAync + " " + Thread.currentThread());
    }

    public void asyncCall() throws Throwable {
//          Thread.sleep(50);

        ++ _async_counter;

        if(_async_thread == null)
            _async_thread = Thread.currentThread();

//          if(_async_thread != Thread.currentThread())
//              _passedAync = false;

        if(DEBUG) System.err.println("asyncCall:" + _async_counter + " " + Thread.currentThread());
    }

    public synchronized void increment() throws Throwable {
        if(DEBUG) System.err.println("increment - " + Thread.currentThread());

        ++ _counter;
        notifyAll();
    }

    public synchronized void notifyme() {
        if(DEBUG) System.err.println("\t\t\tnotifying me" + Thread.currentThread());

        notifyAll();

        _notified = true;
    }

    public boolean passedAsyncTest() {
        return  _passedAync && (_sync_counter == MESSAGES);
    }
}
