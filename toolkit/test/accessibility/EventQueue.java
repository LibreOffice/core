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

import java.util.LinkedList;

/** The event queue singleton dispatches events received from OpenOffice.org
    applications in a thread separate from the AWB main thread.

    The queue of event objects, LinkedList<Runnable> The queue object will
    also serve as lock for the consumer/producer type syncronization.
*/
class EventQueue
    implements Runnable
{
    public boolean mbVerbose = false;
    public boolean mbHandleDisposingEventsSynchronous = true;

    public synchronized static EventQueue Instance ()
    {
        if (maInstance == null)
            maInstance = new EventQueue ();
        return maInstance;
    }

    public void addEvent (Runnable aEvent)
    {
        synchronized (maMonitor)
        {
            if (mbVerbose)
                System.out.println ("queing regular event " + aEvent);
            maRegularQueue.addLast (aEvent);
            maMonitor.notify ();
        }
    }


    public void addDisposingEvent (Runnable aEvent)
    {
        if (mbHandleDisposingEventsSynchronous)
            aEvent.run ();
        else
            synchronized (maMonitor)
            {
                if (mbVerbose)
                    System.out.println ("queing disposing event " + aEvent);
                maDisposingQueue.addLast (aEvent);
                maMonitor.notify ();
            }
    }


    private EventQueue ()
    {
        maMonitor = new Boolean (true);
        maRegularQueue = new LinkedList<Runnable>();
        maDisposingQueue = new LinkedList<Runnable>();
        new Thread(this, "AWB.EventQueue").start();
    }


    /// This thread's main method: deliver all events
    public void run()
    {
        // in an infinite loop, check for events to deliver, then
        // wait on lock (which will be notified when new events arrive)
        while( true )
        {
            Runnable aEvent = null;
            do
            {
                synchronized (maMonitor)
                {
                    if (maDisposingQueue.size() > 0)
                    {
                        aEvent = maDisposingQueue.removeFirst();
                        if (mbVerbose)
                            System.out.println ("delivering disposing event " + aEvent);
                    }
                    else if (maRegularQueue.size() > 0)
                    {
                        aEvent = maRegularQueue.removeFirst();
                        if (mbVerbose)
                            System.out.println ("delivering regular event " + aEvent);
                    }
                    else
                        aEvent = null;
                }
                if (aEvent != null)
                {
                    try
                    {
                        aEvent.run();
                    }
                    catch( Throwable e )
                    {
                        System.out.println(
                            "caught exception during event delivery: " + e );
                        e.printStackTrace();
                    }
                }
            }
            while( aEvent != null );

            try
            {
                synchronized (maMonitor)
                {
                    maMonitor.wait();
                }
            }
            catch (Exception e)
            {
                // can't wait? odd!
                System.err.println("Can't wait!");
                e.printStackTrace();
            }
        }
    }

    private static EventQueue maInstance = null;
    private Object maMonitor;
    private LinkedList<Runnable> maRegularQueue;
    private LinkedList<Runnable> maDisposingQueue;
}


