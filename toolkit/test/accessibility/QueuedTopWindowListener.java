import com.sun.star.awt.XTopWindowListener;
import com.sun.star.lang.EventObject;

class QueuedTopWindowListener
    implements XTopWindowListener
{
    public QueuedTopWindowListener (TopWindowListener aListener)
    {
        maListener = aListener;
    }

    public void windowOpened (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        EventQueue.Instance().addEvent (new Runnable()
            {
                public void run()
                {
                    QueuedTopWindowListener.this.maListener.windowOpened (aEvent);
                }
            }
            );
    }




    public void windowClosing (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        // Ignored.
    }




    public void windowClosed (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        EventQueue.Instance().addEvent (new Runnable()
            {
                public void run()
                {
                    QueuedTopWindowListener.this.maListener.windowClosed (aEvent);
                }
            }
            );
    }




    public void windowMinimized (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("QueuedTopWindowListener: Top window minimized: " + aEvent);
    }

    public void windowNormalized (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("QueuedTopWindowListener: Top window normalized: " + aEvent);
    }

    public void windowActivated (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("QueuedTopWindowListener: Top window actived: " + aEvent);
    }

    public void windowDeactivated (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("QueuedTopWindowListener: Top window deactived: " + aEvent);
    }

    public void disposing( final EventObject aEvent)
    {
        EventQueue.Instance().addDisposingEvent (new Runnable()
            {
                public void run()
                {
                    if (QueuedTopWindowListener.this.maListener != null)
                        QueuedTopWindowListener.this.maListener.disposing (aEvent);
                }
            }
            );
    }

    private TopWindowListener maListener;
}
