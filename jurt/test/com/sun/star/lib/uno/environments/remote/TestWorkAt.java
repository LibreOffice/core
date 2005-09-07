/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestWorkAt.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:13:29 $
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
