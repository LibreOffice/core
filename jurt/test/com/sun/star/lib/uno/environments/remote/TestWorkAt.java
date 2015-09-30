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

import static org.junit.Assert.*;

class TestWorkAt implements TestIWorkAt {
    /**
     * When set to true, enables various debugging output.
     */
    private static final boolean DEBUG = false;

    static final int MESSAGES = 35;


    int _counter;

    int _sync_counter;
    int _async_counter;

    private Thread _sync_thread;
    private Thread _async_thread;

    private boolean _passedAync = true;
    boolean _notified = false;

    public synchronized void syncCall() throws Throwable {
        ++ _sync_counter;

        // at least in currently run tests this should never fire, so don't
        // defer the check until passedAsyncTest and assert here
        assertEquals(MESSAGES, _async_counter);
        if(_async_counter != MESSAGES)
            _passedAync = false;

        if(_sync_thread == null)
            _sync_thread = Thread.currentThread();

        if(DEBUG) System.err.println("syncCall:" + _sync_counter + " " + _passedAync + " " + Thread.currentThread());
    }

    public synchronized void asyncCall() throws Throwable {
        ++ _async_counter;

        if(_async_thread == null)
            _async_thread = Thread.currentThread();

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
        assertEquals(MESSAGES, _sync_counter);
        assertTrue(_passedAync);
        return  _passedAync && (_sync_counter == MESSAGES);
    }
}
