
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;

import java.util.Vector;
import java.util.HashMap;
import java.util.Enumeration;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;

/** Objects of this class (usually one, singleton?) listen to accessible
    events of all objects in all trees.
*/
public class EventListener
{
    public boolean mbVerbose = false;

    public EventListener (AccessibilityTreeModel aTreeModel)
    {
        maTreeModel = aTreeModel;
    }


    private static String objectToString(Object aObject)
    {
        if (aObject == null)
            return null;
        else
            return aObject.toString();
    }



    /** This method handles accessibility objects that are being disposed.
     */
    public void disposing (XAccessibleContext xContext)
    {
        if (mbVerbose)
            System.out.println("disposing " + xContext);
        maTreeModel.removeNode (xContext);
    }

    /** This method is called from accessible objects that broadcast
        modifications of themselves or from their children.  The event is
        processed only, except printing some messages, if the tree is not
        locked.  It should be locked during changes to its internal
        structure like expanding nodes.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        EventHandler aHandler;

        switch (aEvent.EventId)
        {
            case AccessibleEventId.CHILD:
                aHandler = new ChildEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.BOUNDRECT_CHANGED:
            case AccessibleEventId.VISIBLE_DATA_CHANGED:
                aHandler = new GeometryEventHandler (aEvent, maTreeModel);
                break;


            case AccessibleEventId.NAME_CHANGED:
            case AccessibleEventId.DESCRIPTION_CHANGED:
            case AccessibleEventId.STATE_CHANGED:
            case AccessibleEventId.SELECTION_CHANGED:
                aHandler = new ContextEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.TABLE_MODEL_CHANGED:
            case AccessibleEventId.TABLE_CAPTION_CHANGED:
            case AccessibleEventId.TABLE_COLUMN_DESCRIPTION_CHANGED:
            case AccessibleEventId.TABLE_COLUMN_HEADER_CHANGED:
            case AccessibleEventId.TABLE_ROW_DESCRIPTION_CHANGED:
            case AccessibleEventId.TABLE_ROW_HEADER_CHANGED:
            case AccessibleEventId.TABLE_SUMMARY_CHANGED:
                aHandler = new TableEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACTION_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.HYPERTEXT_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACTIVE_DESCENDANT_CHANGED:
            case AccessibleEventId.CARET_CHANGED:
            case AccessibleEventId.TEXT_CHANGED:
            case AccessibleEventId.VALUE_CHANGED:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            default:
                aHandler = null;
                break;
        }

        if (aHandler == null)
            System.out.println ("    unhandled event");
        else
        {
            if (mbVerbose)
                aHandler.Print (System.out);
            aHandler.Process ();
        }
    }


    private AccessibilityTreeModel maTreeModel;
}
