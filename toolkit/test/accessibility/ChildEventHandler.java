import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class ChildEventHandler
    extends EventHandler
{
    public ChildEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
        mxOldChild = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, aEvent.OldValue);
        mxNewChild = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, aEvent.NewValue);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        if (mxOldChild != null)
            out.println ("   removing child " + mxOldChild);
        if (mxNewChild != null)
            out.println ("   adding child " + mxNewChild);
    }

    public void Process ()
    {
        // Insertion and removal of children should be mutually exclusive.
        // But this is a test tool and should take everything into account.
        if (mxOldChild != null)
        {
            maTreeModel.removeNode (mxOldChild.getAccessibleContext());
            maTreeModel.updateNode (mxEventSource, AccessibleTreeHandler.class);
        }

        if (mxNewChild != null)
        {
            maTreeModel.addChild (mxEventSource, mxNewChild);
        }
    }


    private XAccessible mxOldChild;
    private XAccessible mxNewChild;
}
