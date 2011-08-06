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

package com.sun.star.comp.beans;


//---------------------------------------------------------------------------
/** Helper class to watch calls into OOo with a timeout.
 */
//Do not add the thread instances to a threadgroup. When testing the bean in
//an applet it turned out the the ThreadGroup was in an inconsistent state
//after navigating off the site that contains the applet and back to it.
//That was tested with a Sun JRE 1.4.2_06
public class CallWatchThread extends Thread
{
    private static boolean DEBUG = false;

    private Thread aWatchedThread;
    private String aTag;
    private boolean bAlive;
    private long nTimeout;

    public CallWatchThread(long nTimeout)
    {
        this(nTimeout, "");
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
        throws java.lang.InterruptedException
    {
        dbgPrint( "CallWatchThread(" + this + ".cancel(" + aTag + ")" );
        if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
            throw new RuntimeException( "wrong thread" );
        aWatchedThread = null;
        if ( interrupted() )
            throw new InterruptedException();
    }

    public synchronized void restart()
        throws java.lang.InterruptedException
    {
        dbgPrint( "CallWatchThread(" + this + ".restart(" + aTag + ")" );
        if ( aWatchedThread != null && aWatchedThread != Thread.currentThread() )
            throw new RuntimeException( "wrong thread" );
        bAlive = true;
        if ( interrupted() )
            throw new InterruptedException();
        notify();
    }

    public void run()
    {
        dbgPrint( "CallWatchThread(" + this + ".run(" + aTag + ") ***** STARTED *****" );
        long n = 0;
        while ( aWatchedThread != null )
        {
            dbgPrint( "CallWatchThread(" + this + ").run(" + aTag + ") running #" + ++n );
            synchronized(this)
            {
                bAlive = false;
                try
                {
                    wait( nTimeout );
                }
                catch ( java.lang.InterruptedException aExc )
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




