import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class GeometryEventHandler
    extends EventHandler
{
    public GeometryEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        out.println ("   children not relevant");
    }

    public void Process ()
    {
        maTreeModel.updateNode (mxEventSource,
            AccessibleComponentHandler.class,
            AccessibleExtendedComponentHandler.class);
    }
}
