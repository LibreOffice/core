package ov;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/** Base class for object views that regsiters as accessibility event
    listener.
*/
abstract class ListeningObjectView
    extends ObjectView
    implements XAccessibleEventListener
{
    public ListeningObjectView (ObjectViewContainer aContainer)
    {
        super (aContainer);
    }

    /** Add this object as event listener at the broadcasting
        accessible object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        super.SetObject (xContext);
        XAccessibleEventBroadcaster xBroadcaster =
            (XAccessibleEventBroadcaster)UnoRuntime.queryInterface(
                XAccessibleEventBroadcaster.class, xContext);
        if (xBroadcaster != null)
            xBroadcaster.addEventListener (this);
    }


    /** Remove this object as event listener from the broadcasting
        accessible object.
    */
    public void Destroy ()
    {
        super.Destroy ();
        XAccessibleEventBroadcaster xBroadcaster =
            (XAccessibleEventBroadcaster)UnoRuntime.queryInterface(
                XAccessibleEventBroadcaster.class, mxContext);
        if (xBroadcaster != null)
            xBroadcaster.removeEventListener (this);
    }

    /** Derived classes have to implement this method to handle incoming
        events.
    */
    abstract public void notifyEvent (AccessibleEventObject aEvent);

    /** The disposing event is ignored per default.  If a derived class is
        interested it can overwrite this method.
    */
    public void disposing (EventObject aEvent)
    {
    }
}
