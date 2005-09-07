/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CallWatchThread.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:58:33 $
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




