import drafts.com.sun.star.accessibility.*;
import com.sun.star.lang.EventObject;

import java.util.LinkedList;

/** The event queue singleton dispatches events received from OpenOffice.org
    applications in a thread separate from the AWB main thread.

    The queue of event objects, LinkedList<Runnable> The queue object will
    also serve as lock for the consumer/producer type syncronization.
*/
class EventQueue
    implements Runnable
{
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
            System.out.println ("adding regular event " + aEvent);
            maRegularQueue.addLast (aEvent);
            maMonitor.notify ();
        }
    }


    public void addDisposingEvent (Runnable aEvent)
    {
        aEvent.run ();
        /*
        synchronized (maMonitor)
        {
            System.out.println ("adding disposing event " + aEvent);
            maDisposingQueue.addLast (aEvent);
            maMonitor.notify ();
        }
        */
    }


    private EventQueue ()
    {
        maMonitor = new Boolean (true);
        maRegularQueue = new LinkedList();
        maDisposingQueue = new LinkedList();
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
                        aEvent = (Runnable)maDisposingQueue.removeFirst();
                        System.out.println ("delivering disposing event " + aEvent);
                    }
                    else if (maRegularQueue.size() > 0)
                    {
                        aEvent = (Runnable)maRegularQueue.removeFirst();
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
                            "Exception during event delivery: " + e );
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
    private LinkedList maRegularQueue;
    private LinkedList maDisposingQueue;
}


