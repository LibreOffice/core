import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;


/** Base class for handling of accessibility events.
*/
class EventHandler
{
    public EventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        maEvent = aEvent;
        maTreeModel = aTreeModel;

        mnEventId = aEvent.EventId;

        mxEventSource = (XAccessibleContext)UnoRuntime.queryInterface(
            XAccessibleContext.class, aEvent.Source);
        if (mxEventSource == null)
        {
            XAccessible xAccessible = (XAccessible)UnoRuntime.queryInterface(
                XAccessible.class, aEvent.Source);
            if (xAccessible != null)
                mxEventSource = xAccessible.getAccessibleContext();
        }
    }

    public void Print (PrintStream out)
    {
        out.println ("Event id is " + mnEventId
            + " (" + NameProvider.getEventName(mnEventId)+")"
            + " for " + mxEventSource.getAccessibleName() + " / "
            + NameProvider.getRoleName (mxEventSource.getAccessibleRole()));
        PrintOldAndNew (out);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        out.println ("    old value is " + maEvent.OldValue);
        out.println ("    new value is " + maEvent.NewValue);
    }

    public void Process ()
    {
        System.out.println ("processing of event " + maEvent + " not implemented");
    }

    protected AccessibleEventObject maEvent;
    protected AccessibilityTreeModel maTreeModel;

    protected int mnEventId;
    protected XAccessibleContext mxEventSource;
}
