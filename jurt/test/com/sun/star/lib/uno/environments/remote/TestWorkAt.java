/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
