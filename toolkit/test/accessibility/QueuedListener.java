import com.sun.star.accessibility.*;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.*;
import com.sun.star.accessibility.*;

import java.util.LinkedList;

class QueuedListener
    implements XAccessibleEventListener
{
    public QueuedListener (EventListener aListener)
    {
        maListener = aListener;
    }


    public void disposing( final EventObject aEvent)
    {
        XAccessibleContext xContext = (XAccessibleContext)UnoRuntime.queryInterface(
            XAccessibleContext.class, aEvent.Source);
        if (xContext == null)
        {
            XAccessible xAccessible = (XAccessible)UnoRuntime.queryInterface(
                XAccessible.class, aEvent.Source);
            if (xAccessible != null)
                xContext = xAccessible.getAccessibleContext();
        }
        final XAccessibleContext xSource = xContext;
        EventQueue.Instance().addDisposingEvent (new Runnable()
            {
                public void run()
                {
                    if (QueuedListener.this.maListener != null)
                        QueuedListener.this.maListener.disposing (xSource);
                }
            }
            );
    }

    public void notifyEvent( final AccessibleEventObject aEvent )
    {
        EventQueue.Instance().addEvent (new Runnable()
            {
                public void run()
                {
                    QueuedListener.this.maListener.notifyEvent( aEvent );
                }
            }
            );
    }

    private EventListener maListener;
}


