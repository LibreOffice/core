
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;

import java.util.Vector;
import java.util.HashMap;
import java.util.Enumeration;

import drafts.com.sun.star.accessibility.*;
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
        System.out.println("dispose: " + objectToString(xContext));
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
            case AccessibleEventId.ACCESSIBLE_CHILD_EVENT:
                aHandler = new ChildEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACCESSIBLE_BOUNDRECT_EVENT:
            case AccessibleEventId.ACCESSIBLE_VISIBLE_DATA_EVENT:
                aHandler = new GeometryEventHandler (aEvent, maTreeModel);
                break;


            case AccessibleEventId.ACCESSIBLE_NAME_EVENT:
            case AccessibleEventId.ACCESSIBLE_DESCRIPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_STATE_EVENT:
            case AccessibleEventId.CONTROLLED_BY_EVENT:
            case AccessibleEventId.CONTROLLER_FOR_EVENT:
            case AccessibleEventId.LABEL_FOR_EVENT:
            case AccessibleEventId.LABELED_BY_EVENT:
            case AccessibleEventId.MEMBER_OF_EVENT:
            case AccessibleEventId.ACCESSIBLE_SELECTION_EVENT:
                aHandler = new ContextEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACCESSIBLE_TABLE_MODEL_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_CAPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_DESCRIPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_HEADER_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_DESCRIPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_HEADER_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_SUMMARY_EVENT:
                aHandler = new TableEventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACCESSIBLE_ACTION_EVENT:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACCESSIBLE_HYPERTEXT_EVENT:
                aHandler = new EventHandler (aEvent, maTreeModel);
                break;

            case AccessibleEventId.ACCESSIBLE_ACTIVE_DESCENDANT_EVENT:
            case AccessibleEventId.ACCESSIBLE_CARET_EVENT:
            case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
            case AccessibleEventId.ACCESSIBLE_VALUE_EVENT:
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
            aHandler.Print (System.out);
            aHandler.Process ();
        }
    }


    private AccessibilityTreeModel maTreeModel;
}
