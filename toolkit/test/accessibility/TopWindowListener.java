import com.sun.star.awt.XWindow;
import drafts.com.sun.star.awt.XExtendedToolkit;
import drafts.com.sun.star.accessibility.XAccessible;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import javax.swing.event.TreeModelEvent;

/** Listen for top window events and create or delete children of the tree
    model accordingly.
*/
class TopWindowListener
{
    TopWindowListener (AccessibilityTreeModel aModel, SimpleOffice aOffice)
    {
        maModel = aModel;
        maOffice = aOffice;
    }




    /** Use this function to initially fill the accessibility object tree
        view with nodes for top level windows.
    */
    public void Initialize ()
    {
        XExtendedToolkit xToolkit = maOffice.getExtendedToolkit();
        if (xToolkit != null)
        {
            maModel.lock ();
            int nTopWindowCount = xToolkit.getTopWindowCount();
            for (int i=0; i<nTopWindowCount; i++)
            {
                try
                {
                    XAccessible xAccessible = maOffice.getAccessibleObject(
                        xToolkit.getTopWindow (i));
                    // Uncomment the following line to get the real root of
                    //   the accessible tree that xAccessible belongs to.
                    // xAccessible = maOffice.getAccessibleRoot(xAccessible);
                    AddTopLevelNode (xAccessible);
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception: " + e);
                    e.printStackTrace();
                }
            }
            maModel.unlock ((AccessibleTreeNode)maModel.getRoot());
        }
    }



    /** Add a new top level node which, to be exact, will be placed on the
        second layer of the tree.
        @param xNewTopLevelObject
            The accessible object of the new top level window.
    */
    private void AddTopLevelNode (XAccessible xNewTopLevelObject)
    {
        Object aObject = maModel.getRoot();
        if ((xNewTopLevelObject != null) && xNewTopLevelObject.getAccessibleContext()== null)
            System.out.println ("top level window not accessible");
        if (aObject instanceof VectorNode && xNewTopLevelObject != null)
        {
            VectorNode aRoot = (VectorNode) aObject;
            AccessibleTreeNode aNode =
                NodeFactory.Instance().createDefaultNode (xNewTopLevelObject, aRoot);
            aRoot.addChild (aNode);
            maModel.fireTreeNodesInserted (maModel.createEvent (aRoot, aNode));
        }
    }




    /** Remove an existing top level node from the tree.
        @param xNewTopLevelObject
            The accessible object to remove.
    */
    private void RemoveTopLevelNode (XAccessible xTopLevelObject)
    {
        Object aObject = maModel.getRoot();
        if (aObject instanceof VectorNode && xTopLevelObject != null)
        {
            System.out.println ("removing node " + xTopLevelObject);
            VectorNode aRoot = (VectorNode) aObject;
            maModel.removeNode (xTopLevelObject.getAccessibleContext());
            /*
            AccessibleTreeNode aNode = maModel.getNode (xTopLevelObject);
            TreeModelEvent aEvent = maModel.createEvent (aRoot, aNode);
            maModel.removeChild (aNode);
            System.out.println (aNode);
            maModel.fireTreeNodesRemoved (aEvent);
            */
        }
    }





    /**  This method exists for debugging.  It prints a list of all top
         level windows.
    */
    private void ShowAllTopLevelWindows ()
    {
        XExtendedToolkit xToolkit = maOffice.getExtendedToolkit();
        if (xToolkit != null)
        {
            int nTopWindowCount = xToolkit.getTopWindowCount();
            for (int i=0; i<nTopWindowCount; i++)
            {
                try
                {
                    System.out.println (i + " : " + xToolkit.getTopWindow (i));
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception; " + e);
                }
            }
        }
    }




    // XTopWindowListener
    public void windowOpened (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("Top window opened: " + aEvent.Source);
        if (maModel != null)
        {
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(
                XWindow.class, aEvent.Source);
            if (xWindow == null)
                System.out.println ("event source is no XWindow");
            else
            {
                XAccessible xAccessible = maOffice.getAccessibleObject(xWindow);
                if (xAccessible == null)
                    System.out.println ("event source is no XAccessible");
                else
                    AddTopLevelNode (xAccessible);
            }
        }
    }




    public void windowClosed (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        System.out.println ("Top window closed: " + aEvent);
        if (maModel != null)
        {
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(
                XWindow.class, aEvent.Source);
            if (xWindow == null)
                System.out.println ("event source is no XWindow");
            else
            {
                XAccessible xAccessible = maOffice.getAccessibleObject(xWindow);
                if (xAccessible == null)
                    System.out.println ("event source is no XAccessible");
                else
                    RemoveTopLevelNode (xAccessible);
            }
        }
    }

    public void disposing (final com.sun.star.lang.EventObject aEvent)
    {
        System.out.println ("Top window disposed: " + aEvent);
    }




    private AccessibilityTreeModel
        maModel;
    private SimpleOffice
        maOffice;
}
