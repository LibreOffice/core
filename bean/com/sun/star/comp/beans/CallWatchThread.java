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

package com.sun.star.comp.beans;


/** Helper class to watch calls into OOo with a timeout.
 */
// Do not add the thread instances to a threadgroup. When testing the bean in
// an applet it turned out the ThreadGroup was in an inconsistent state
// after navigating off the site that contains the applet and back to it.
// That was tested with a Sun JRE 1.4.2_06
public class CallWatchThread extends Thread
{
    private static boolean DEBUG = false;

    private Thread aWatchedThread;
    private String aTag;
    private boolean bAlive;
    private long nTimeout;

    public CallWatchThread(long nTimeout)
    {
        this(nTimeout, "CallWatchThread");
    }

    public CallWatchThread( long nTimeout, String aTag )
    {
        super(aTag);
        this.aWatchedThread = Thread.currentThread();
        this.nTimeout = nTimeout;

        this.aTag = aTag;
        setDaemon( true );
        dbgPrint( "CallWatchThread(" + this + ").start(" + aTag + ")" );
        start();
    }

    public void cancel()
        throws InterruptedException
    {
        dbgPrint( "CallWatchThread(" + this + ".cancel(" + aTag + ")" );
        if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
            throw new RuntimeException( "wrong thread" );
        aWatchedThread = null;
        if ( interrupted() )
            throw new InterruptedException();
    }

    public synchronized void restart()
        throws InterruptedException
    {
        dbgPrint( "CallWatchThread(" + this + ".restart(" + aTag + ")" );
        if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
            throw new RuntimeException( "wrong thread" );
        bAlive = true;
        if ( interrupted() )
            throw new InterruptedException();
        notify();
    }

    @Override
    public void run()
    {
        dbgPrint( "CallWatchThread(" + this + ".run(" + aTag + ") ***** STARTED *****" );
        long n = 0;
        synchronized(this)
        {
            while ( aWatchedThread != null )
            {
                dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") running #" + ++n );
                bAlive = false;
                try
                {
                    wait( nTimeout );
                }
                catch ( InterruptedException aExc )
                {
                    bAlive = false;
                }

                // watched thread seems to be dead (not answering)?
                if ( !bAlive && aWatchedThread != null )
                {
                    dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") interrupting" );
                    aWatchedThread.interrupt();
                    aWatchedThread = null;
                }
            }
        }

        dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") terminated" );
    }

    private void dbgPrint( String aMessage )
    {
        if (DEBUG)
            System.err.println( "OOoBean: " + aMessage );
    }
}




