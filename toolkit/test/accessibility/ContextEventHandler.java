import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

import java.io.PrintStream;

class ContextEventHandler
    extends EventHandler
{
    public ContextEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        switch (mnEventId)
        {
            case AccessibleEventId.STATE_CHANGED:
                try
                {
                    int nOldValue = AnyConverter.toInt (maEvent.OldValue);
                    out.println ("    turning off state " + nOldValue + " ("
                        + NameProvider.getStateName (nOldValue) + ")");
                }
                catch (com.sun.star.lang.IllegalArgumentException e)
                {}
                try
                {
                    int nNewValue = AnyConverter.toInt (maEvent.NewValue);
                    out.println ("    turning on state " + nNewValue + " ("
                        + NameProvider.getStateName (nNewValue) + ")");
                }
                catch (com.sun.star.lang.IllegalArgumentException e)
                {}
                break;

            default:
                super.PrintOldAndNew (out);
        }

    }

    public void Process ()
    {
        maTreeModel.updateNode (mxEventSource, AccessibleContextHandler.class);
    }
}
