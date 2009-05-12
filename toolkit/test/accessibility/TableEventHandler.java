import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleTableModelChange;
import com.sun.star.uno.UnoRuntime;

import java.io.PrintStream;

class TableEventHandler
    extends EventHandler
{
    public TableEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        switch (mnEventId)
        {
            case AccessibleEventId.TABLE_MODEL_CHANGED:
                AccessibleTableModelChange aModelChange =
                    (AccessibleTableModelChange)maEvent.NewValue;
                out.println( "Range: StartRow " + aModelChange.FirstRow +
                    " StartColumn " + aModelChange.FirstColumn +
                    " EndRow " + aModelChange.LastRow +
                    " EndColumn " + aModelChange.LastColumn +
                    " Id " + aModelChange.Type);
                break;
            default:
                super.PrintOldAndNew (out);
        }
    }

    public void Process ()
    {
        maTreeModel.updateNode (mxEventSource, AccessibleTableHandler.class);
    }


    private XAccessible mxOldChild;
    private XAccessible mxNewChild;
}
