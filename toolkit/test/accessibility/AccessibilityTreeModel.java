
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;

import java.util.Vector;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.LinkedList;

import drafts.com.sun.star.accessibility.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Any;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceName;

public class AccessibilityTreeModel
    implements TreeModel, XAccessibleEventListener
{
    // Map to translate from accessible object to corresponding tree node.
    protected HashMap maXAccessibleToNode;

    // Making both of these static is clearly a hack.
    protected static MessageInterface maMessageArea;
    protected static Print maPrinter;
    protected static boolean mbVerbose = true;

    // If the lock count is higher then zero, then no events are processed.
    private int mnLockCount;

    // The list of TreeModelListener objects.
    private Vector maTMListeners;

    // The root node of the tree.  Use setRoot to change it.
    private AccessibleTreeNode maRoot = null;

    // default handlers
    private static Vector aDefaultHandlers;
    private static NodeHandler maContextHandler = new AccessibleContextHandler();
    private static NodeHandler maTextHandler = new AccessibleTextHandler();
    private static NodeHandler maEditableTextHandler = new AccessibleEditableTextHandler();
    private static NodeHandler maComponentHandler = new AccessibleComponentHandler();
    private static NodeHandler maExtendedComponentHandler = new AccessibleExtendedComponentHandler();
    private static NodeHandler maActionHandler = new AccessibleActionHandler();
    private static NodeHandler maImageHandler = new AccessibleImageHandler();
    private static NodeHandler maTableHandler = new AccessibleTableHandler();
    private static NodeHandler maCellHandler = new AccessibleCellHandler();
    private static NodeHandler maHypertextHandler = new AccessibleHypertextHandler();
    private static NodeHandler maHyperlinkHandler = new AccessibleHyperlinkHandler();
    private static NodeHandler maSelectionHandler = new AccessibleSelectionHandler();
    private static NodeHandler maRelationHandler = new AccessibleRelationHandler();
    private static NodeHandler maTreeHandler = new AccessibleTreeHandler();

    private Canvas maCanvas;


    public AccessibilityTreeModel (AccessibleTreeNode aRoot, MessageInterface aMessageArea, Print aPrinter)
    {
        // create default node (unless we have a 'proper' node)
        if( ! (aRoot instanceof AccessibleTreeNode) )
            aRoot = new StringNode ("Root", null);
        setRoot (aRoot);
        maMessageArea = aMessageArea;
        maPrinter = aPrinter;

        maTMListeners = new Vector();
        maXAccessibleToNode = new HashMap ();

        // syncronous or asyncronous event delivery? (i.e. same thread
        // or in s seperate event delivery thread)
        // xListener = this;                // syncronous event delivery
        xListener = new QueuedListener();   // asyncronous event delivery
    }

    /** Lock the tree.  While the tree is locked, events from the outside are
        not processed.  Lock the tree when you change its internal structure.
    */
    public void lock ()
    {
        mnLockCount += 1;
        System.out.println ("locking: " + mnLockCount);
    }

    /** Unlock the tree.  After unlocking the tree as many times as locking
        it, a treeStructureChange event is sent to the event listeners.
        @param aNodeHint
            If not null and treeStructureChange events are thrown then this
            node is used as root of the modified subtree.
    */
    public void unlock (AccessibleTreeNode aNodeHint)
    {
        mnLockCount -= 1;
        System.out.println ("unlocking: " + mnLockCount);
        if (mnLockCount == 0)
        {
            fireTreeStructureChanged (
                new TreeModelEvent (this,
                    new TreePath (aNodeHint.createPath())));
        }
    }

    public Object getRoot()
    {
        return maRoot;
    }

    public synchronized void setRoot (AccessibleTreeNode aRoot)
    {
        if (maRoot == null)
            maRoot = aRoot;
        else
        {
            lock ();
            clear ();
            maRoot = aRoot;
            unlock (maRoot);
        }
    }

    /** Clears the model.  That removes all nodes from the internal structures.
    */
    public void clear ()
    {
        System.out.println ("clearing the whole tree");
        Object[] aNodes = maXAccessibleToNode.values().toArray();
        for (int i=0; i<aNodes.length; i++)
            removeNode ((AccessibleTreeNode)aNodes[i]);
    }

    //
    // child management:
    //

    public int getChildCount(Object aParent)
    {
        return (aParent instanceof AccessibleTreeNode) ?
            ((AccessibleTreeNode)aParent).getChildCount() : 0;
    }

    public Object getChild (Object aParent, int nIndex)
    {
        Object aChild = null;
        try
        {
            if (aParent instanceof AccessibleTreeNode)
                aChild = getChild ((AccessibleTreeNode)aParent, nIndex);
            else
                System.out.println ("getChild called for unknown parent node");
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            aChild = ("no child " + nIndex + " from " + aParent + ": " + e);
        }
        return aChild;
    }

    /** Delegate the request to the parent and then register listeners at
        the child and add the child to the canvas.
    */
    public AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
        throws com.sun.star.lang.IndexOutOfBoundsException
    {
        AccessibleTreeNode aChild = null;
        if (aParent != null)
            aChild = aParent.getChild(nIndex);

        // Keep translation table up-to-date.
        addNode (aChild);

        if (aChild == null)
            System.out.println ("getChild: child not found");

        return aChild;
    }

    /** iterate over all children and look for child */
    public int getIndexOfChild (Object aParent, Object aChild)
    {
        int nIndex = -1;
        try
        {
            if ((aParent instanceof AccessibleTreeNode) && (aChild instanceof AccessibleTreeNode))
            {
                AccessibleTreeNode aParentNode = (AccessibleTreeNode) aParent;
                AccessibleTreeNode aChildNode = (AccessibleTreeNode) aChild;

                int nChildCount = aParentNode.getChildCount();
                for( int i = 0; i < nChildCount; i++ )
                {
                    if (aChildNode.equals (aParentNode.getChild (i)))
                    {
                        nIndex = i;
                        break;
                    }
                }
            }
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            // Return -1 by falling through.
        }

        // not found?
        return nIndex;
    }


    public boolean isLeaf (Object aNode)
    {
        return (aNode instanceof AccessibleTreeNode) ?
            ((AccessibleTreeNode)aNode).isLeaf() : true;
    }



    /** Remove a node (and all children) from the tree model.
    */
    protected boolean removeChild (AccessibleTreeNode aNode)
    {
        try
        {
            if( aNode == null )
            {
                System.out.println ("can't remove null node");
                return false;
            }
            else
            {
                // depth-first removal of children
                while (aNode.getChildCount() > 0)
                    if ( ! removeChild (aNode.getChild (0)))
                        break;

                // Remove node from its parent.
                AccessibleTreeNode aParent = aNode.getParent();
                if (aParent != null)
                {
                    int nIndex = aParent.indexOf(aNode);
                    aParent.removeChild (nIndex);
                }

                removeNode (aNode);
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while removing child " + aNode + " : " + e);
            return false;
        }
        return true;
    }

    protected void removeNode (AccessibleTreeNode aNode)
    {
        try
        {
            if ((aNode != null) && (aNode instanceof AccTreeNode))
            {
                // Remove node itself from internal data structures.
                removeFromCanvas ((AccTreeNode)aNode);
                removeAccListener ((AccTreeNode)aNode);
                maXAccessibleToNode.remove (((AccTreeNode)aNode).getAccessible());
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while removing node " + aNode + " : " + e);
        }
    }

    /** Add add a new child to a parent.
        @return
            Returns the new or existing representation of the specified
            accessible object.
    */
    protected AccessibleTreeNode addChild (AccTreeNode aParentNode, XAccessible xNewChild)
    {
        AccessibleTreeNode aChildNode = null;
        try
        {
            boolean bRet = false;

            // First make sure that the accessible object does not already have
            // a representation.
            aChildNode = (AccessibleTreeNode)maXAccessibleToNode.get (xNewChild);
            if (aChildNode == null)
                aChildNode = aParentNode.addAccessibleChild (xNewChild);
            else
                System.out.println ("node already present");
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while adding child " + xNewChild
                + " to parent " + aParentNode + ": " + e);
        }
        return aChildNode;
    }


    /** Add the child node to the internal tree structure.
        @param aNode
            The node to insert into the internal tree structure.
    */
    protected boolean addNode (AccessibleTreeNode aNode)
    {
        boolean bRet = false;
        try
        {
            if (aNode instanceof AccTreeNode)
            {
                AccTreeNode aChild = (AccTreeNode)aNode;
                XAccessible xChild = aChild.getAccessible();
                if (maXAccessibleToNode.get (xChild) == null)
                {
                    registerAccListener (aChild);
                maXAccessibleToNode.put (xChild, aChild);
                addToCanvas (aChild);
                }
                bRet = true;
            }

        }
        catch (Exception e)
        {
            System.out.println ("caught exception while adding node " + aNode + ": " + e);
        }
        return bRet;
    }

    protected AccessibleTreeNode getNode (XAccessible xAccessible)
    {
        return (AccessibleTreeNode)maXAccessibleToNode.get (xAccessible);
    }

    /** create path to node, suitable for TreeModelEvent constructor
     * @see javax.swing.event.TreeModelEvent#TreeModelEvent
     */
    protected Object[] createPath (AccessibleTreeNode aNode)
    {
        Vector aPath = new Vector();
        aNode.createPath (aPath);
        return aPath.toArray();
    }

    //
    // listeners (and helper methods)
    //
    // We are registered with listeners as soon as objects are in the
    // tree cache, and we should get removed as soon as they are out.
    //

    public void addTreeModelListener(TreeModelListener l)
    {
        maTMListeners.add(l);
    }

    public void removeTreeModelListener(TreeModelListener l)
    {
        maTMListeners.remove(l);
    }

    protected void fireTreeNodesChanged(TreeModelEvent e)
    {
        System.out.println("treeNodesChanges: " + e);
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            ((TreeModelListener)maTMListeners.get(i)).treeNodesChanged(e);
        }
    }

    protected void fireTreeNodesInserted(final TreeModelEvent e)
    {
        System.out.println("treeNodesInserted: " + e);
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            ((TreeModelListener)maTMListeners.get(i)).treeNodesInserted(e);
        }
    }

    protected void fireTreeNodesRemoved(final TreeModelEvent e)
    {
        System.out.println("treeNodesRemoved: " + e);
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            ((TreeModelListener)maTMListeners.get(i)).treeNodesRemoved(e);
        }
    }

    protected void fireTreeStructureChanged(final TreeModelEvent e)
    {
        System.out.println("treeStructureChanged: " + e);
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            ((TreeModelListener)maTMListeners.get(i)).treeStructureChanged(e);
        }
    }

    protected TreeModelEvent createEvent (XAccessible xParent)
    {
        AccessibleTreeNode aParentNode = (AccessibleTreeNode)maXAccessibleToNode.get (xParent);
        return new TreeModelEvent (this, createPath (aParentNode));
    }

    /** Create a TreeModelEvent object that informs listeners that one child
        has been removed from or inserted into its parent.
    */
    protected TreeModelEvent createEvent (XAccessible xParent, XAccessible xChild)
    {
        // get parent node and create the tree path
        AccessibleTreeNode aParentNode = (AccessibleTreeNode)maXAccessibleToNode.get (xParent);
        System.out.println (xParent);
        Object[] aPathToParent = createPath (aParentNode);

        AccessibleTreeNode aChildNode = null;
        if (xChild != null)
            aChildNode = (AccessibleTreeNode)maXAccessibleToNode.get (xChild);
        int nIndexInParent = -1;
        if (xChild != null)
            nIndexInParent = aParentNode.indexOf (aChildNode);

        if (nIndexInParent == -1)
            // This event may be passed only to treeStructureChanged of the listeners.
            return new TreeModelEvent (this,
                aPathToParent);
        else
            // General purpose event for removing or inserting known nodes.
            return new TreeModelEvent (this,
                aPathToParent,
                new int[] {nIndexInParent},
                new Object[] {aChildNode} );
    }

    /** Create a TreeModelEvent that indicates changes at those children of
        the specified node with the specified indices.
    */
    protected TreeModelEvent createChangeEvent (AccTreeNode aNode, Vector aChildIndices)
    {
        // Build a list of child objects that are indicated by the given indices.
        int nCount = aChildIndices.size();
        Object aChildObjects[] = new Object[nCount];
        int nChildIndices[] = new int[nCount];
        for (int i=0; i<nCount; i++)
        {
            int nIndex = ((Integer)aChildIndices.elementAt(i)).intValue();
            aChildObjects[i] = aNode.getChild (nIndex);
            nChildIndices[i] = nIndex;
        }

        return new TreeModelEvent (this,
            createPath(aNode),
            nChildIndices,
            aChildObjects);
    }



    /**
     * broadcast a tree event in a seperate Thread
     * must override fire method
     */
    class EventRunner implements Runnable
    {
        public void run()
        {
            for(int i = 0; i < maTMListeners.size(); i++)
            {
                fire( (TreeModelListener)maTMListeners.get(i) );
            }
        }

        protected void fire( TreeModelListener l) { }
    }

    /** The listener to be registered with the accessible objects.
     * Could be set to 'this' for same-thread event delivery, or to an
     * instance of QueuedListener for multi-threaded delivery.  May
     * not be changed, since this would trip the
     * register/removeAccListener logic. */
    private final XAccessibleEventListener xListener;


    protected XAccessibleEventBroadcaster getBroadcaster (Object aObject)
    {
        if (aObject instanceof AccTreeNode)
            return (XAccessibleEventBroadcaster) UnoRuntime.queryInterface (
                XAccessibleEventBroadcaster.class, ((AccTreeNode)aObject).getContext());
        else
            return null;
    }

    protected void registerAccListener( Object aObject )
    {
        // register this as listener for XAccessibleEventBroadcaster
        // implementations
        XAccessibleEventBroadcaster xBroadcaster = getBroadcaster( aObject );
        if (xBroadcaster != null)
        {
            xBroadcaster.addEventListener( xListener );
        }
    }

    protected void removeAccListener( Object aObject )
    {
        XAccessibleEventBroadcaster xBroadcaster = getBroadcaster( aObject );
        if (xBroadcaster != null)
        {
            xBroadcaster.removeEventListener( xListener );
        }
    }


    //
    // tree model edit
    //
    public void valueForPathChanged(TreePath path, Object newValue) { }


    //
    // static methods + members for creating default nodes
    //



    /** add default handlers based on the supported interfaces */
    public static void addDefaultHandlers (AccTreeNode aNode, XAccessibleContext xContext)
    {
        if (false)
        {
            // Slow but complete version: try each handler type separately.
            aNode.addHandler (maContextHandler.createHandler (xContext));
            aNode.addHandler (maTextHandler.createHandler (xContext));
            aNode.addHandler (maEditableTextHandler.createHandler (xContext));
            aNode.addHandler (maComponentHandler.createHandler (xContext));
            aNode.addHandler (maExtendedComponentHandler.createHandler (xContext));
            aNode.addHandler (maActionHandler.createHandler (xContext));
            aNode.addHandler (maImageHandler.createHandler (xContext));
            aNode.addHandler (maTableHandler.createHandler (xContext));
            aNode.addHandler (maCellHandler.createHandler (xContext));
            aNode.addHandler (maHypertextHandler.createHandler (xContext));
            aNode.addHandler (maHyperlinkHandler.createHandler (xContext));
            aNode.addHandler (maSelectionHandler.createHandler (xContext));
            aNode.addHandler (maRelationHandler.createHandler (xContext));
            aNode.addHandler (maTreeHandler.createHandler (xContext));
        }
        else
        {
            // Exploit dependencies between interfaces.
            NodeHandler aHandler;
            aNode.addHandler (maContextHandler.createHandler (xContext));

            aHandler = maTextHandler.createHandler (xContext);
            if (aHandler != null)
            {
                aNode.addHandler (aHandler);
                aNode.addHandler (maEditableTextHandler.createHandler (xContext));
                aNode.addHandler (maHypertextHandler.createHandler (xContext));
                aNode.addHandler (maHyperlinkHandler.createHandler (xContext));
            }
            aHandler = maComponentHandler.createHandler (xContext);
            if (aHandler != null)
            {
                aNode.addHandler (aHandler);
                aNode.addHandler (maExtendedComponentHandler.createHandler (xContext));
            }
            aNode.addHandler (maActionHandler.createHandler (xContext));
            aNode.addHandler (maImageHandler.createHandler (xContext));
            aNode.addHandler (maTableHandler.createHandler (xContext));
            aNode.addHandler (maRelationHandler.createHandler (xContext));
            aNode.addHandler (maCellHandler.createHandler (xContext));
            aNode.addHandler (maSelectionHandler.createHandler (xContext));
            aNode.addHandler (maTreeHandler.createHandler (xContext));
        }
    }

    /** create a node with the default handlers */
    public static AccTreeNode createDefaultNode (XAccessible xAccessible, AccessibleTreeNode aParent)
    {
        // default: aObject + aDisplay
        String sDisplay;

        // if we are accessible, we use the context + name instead
        XAccessibleContext xContext = null;
        if (xAccessible != null)
            xContext = xAccessible.getAccessibleContext();
        if (xContext != null)
        {
            sDisplay = xContext.getAccessibleName();
            if (sDisplay.length()==0)
            {
                sDisplay = "<no name>";
                // Try to determine some usefull name that indicates the
                // function of the object in question.
                XServiceName xSN = (XServiceName) UnoRuntime.queryInterface (
                    XServiceName.class, xAccessible);
                if (xSN != null)
                    sDisplay = xSN.getServiceName ();
                else
                {
                    XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface (
                    XServiceInfo.class, xAccessible);
                    if (xSI != null)
                        sDisplay = xSI.getImplementationName ();
                }
            }
        }
        else
            sDisplay = new String ("not accessible");


        // create node, and add default handlers
        AccTreeNode aNode = new AccTreeNode (xAccessible, xContext, sDisplay, aParent);
        AccessibilityTreeModel.addDefaultHandlers (aNode, xContext);

        if (mbVerbose)
            maPrinter.print (". ");

        if (aNode == null)
            System.out.println ("createDefaultNode == null");
        return aNode;
    }


    //
    // XAccessibleEventListener interface
    //

    private static String objectToString(Object aObject)
    {
        if (aObject == null)
            return null;
        else
            return aObject.toString();
        /*
        if( aObject instanceof Any )
            aObject = ((Any)aObject).getObject();

        if( aObject instanceof XInterface )
        {
            XServiceInfo xInfo =
                (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class,
                                                         aObject);
            aObject = (xInfo != null) ? xInfo.getImplementationName()
                                      : aObject.getClass().toString();
        }

        return (aObject != null) ? aObject.toString() : null;
        */
    }

    public void disposing( EventObject aEvent)
    {
        System.out.println("dispose: " + objectToString(aEvent.Source));
        removeChild ((AccessibleTreeNode)maXAccessibleToNode.get (aEvent.Source));
        /*
        if( knowsNode( aEvent.Source ) )
        {
            System.out.println("ERROR: Dispose for living node called! " +
                               "Maybe notifications don't work?");
            removeNode( aEvent.Source );
//            fireTreeStructureChanged( createEvent( getRoot() ) );
        }
        */
    }
    private void handleEvent (XAccessible xSource, java.lang.Class class1)
    { handleEvent (xSource, class1,null); }
    private void handleEvent (XAccessible xSource, java.lang.Class class1, java.lang.Class class2)
    {
        AccessibleTreeNode aNode = (AccessibleTreeNode)maXAccessibleToNode.get (xSource);
        if (aNode instanceof AccTreeNode)
        {
            Vector aChildIndices = ((AccTreeNode)aNode).update (
                class1, class2);
            fireTreeNodesChanged (
                createChangeEvent ((AccTreeNode)aNode, aChildIndices));
            updateOnCanvas ((AccTreeNode)aNode);
            maCanvas.repaint ();
        }
    }

    static final String[] aEventNames =
    {
        "[UNKNOWN]", "ACTION", "ACTIVE_DESCENDANT", "CARET", "CHILD",
        "DESCRIPTION", "HYPERTEXT_OFFSET", "NAME", "SELECTION", "STATE",
        "TABLE_CAPTION_CHANGED", "TABLE_COLUMN_DESCRIPTION_CHANGED",
        "TABLE_COLUMN_HEADER_CHANGED", "TABLE_MODEL_CHANGED",
        "TABLE_ROW_DESCRIPTION_CHANGED", "TABLE_ROW_HEADER_CHANGED",
        "TABLE_SUMMARY_CHANGED", "TEXT", "VALUE", "VISIBLE_DATA",
        "CONTROLLED_BY_PROPERTY", "CONTROLLER_FOR_PROPERTY",
        "LABEL_FOR_PROPERTY", "LABELED_BY_PROPERTY", "MEMBER_OF_PROPERTY",
        "CONTENT_FLOWS_FROM", "CONTENT_FLOWS_TO",
        "[UNKNOWN]"
    };

    /** This method is called from accessible objects that broadcast
        modifications of themselves or from their children.  The event is
        processed only, except printing some messages, if the tree is not
        locked.  It should be locked during changes to its internal
        structure like expanding nodes.
    */
    public void notifyEvent( AccessibleEventObject aEvent )
    {

        int nId = aEvent.EventId;
        if( (nId < 0) || (nId >= aEventNames.length) )
            nId = 0;

        System.out.println( "notify: " + aEvent.EventId + " "
            + aEventNames[nId] + ": ["
            + objectToString(aEvent.Source) + "] "
            + objectToString(aEvent.OldValue) + "->"
            + objectToString(aEvent.NewValue) );

        if (mnLockCount > 0)
        {
            System.out.println ("ignoring event because tree is locked");
            return;
        }

        XAccessible xSource = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class,aEvent.Source);

        switch( aEvent.EventId )
        {
            case AccessibleEventId.ACCESSIBLE_CHILD_EVENT:
                // fire insertion and deletion events:
                if (aEvent.OldValue != null)
                {
                    XAccessible xOld = (XAccessible)UnoRuntime.queryInterface(
                        XAccessible.class,aEvent.OldValue);
                    // Create event before removing the node to get the old
                    // index of the node.
                    TreeModelEvent aRemoveEvent = createEvent (xSource, xOld);
                    removeChild ((AccessibleTreeNode)maXAccessibleToNode.get (xOld));
                    fireTreeNodesRemoved (aRemoveEvent);
                    handleEvent (xSource, AccessibleTreeHandler.class);
                }

                // Insertion and removal of children should be mutually
                // exclusive.  But then there is this 'should' ...
                if (aEvent.NewValue != null)
                {
                    XAccessible xNew = (XAccessible)UnoRuntime.queryInterface(
                        XAccessible.class,aEvent.NewValue);
                    // Create event after inserting it so that its new index
                    // in the parent can be determined.
                    AccessibleTreeNode aParentNode = getNode (xSource);
                    if (aParentNode instanceof AccTreeNode)
                    {
                        AccessibleTreeNode aChild = addChild ((AccTreeNode)aParentNode, xNew);
                        if (addNode (aChild))
                        {
                            //                            ((AccTreeNode)aParentNode).update ();
                            updateOnCanvas ((AccTreeNode)aParentNode);

                            // A call to fireTreeNodesInserted for xNew
                            // should be sufficient but at least the
                            // StringNode object that contains the number of
                            // children also changes and we do not know its
                            // index relative to its parent.  Therefore the
                            // more expensive fireTreeStructureChanged is
                            // necessary.
                            fireTreeNodesInserted (createEvent (xSource, xNew));
                            handleEvent (xSource, AccessibleTreeHandler.class);
                        }
                    }
                }
                maCanvas.repaint ();
                break;

            case AccessibleEventId.ACCESSIBLE_TABLE_MODEL_CHANGED:
                AccessibleTableModelChange aModelChange = (AccessibleTableModelChange)aEvent.NewValue;
                System.out.println( "Range: StartRow " + aModelChange.FirstRow +
                                    " StartColumn " + aModelChange.FirstColumn +
                                    " EndRow " + aModelChange.LastRow +
                                    " EndColumn " + aModelChange.LastColumn +
                                    " Id " + aModelChange.Type);
                break;

            case AccessibleEventId.ACCESSIBLE_VISIBLE_DATA_EVENT:
                handleEvent (xSource,
                    AccessibleComponentHandler.class,
                    AccessibleExtendedComponentHandler.class);
                break;


            case AccessibleEventId.ACCESSIBLE_NAME_EVENT:
            case AccessibleEventId.ACCESSIBLE_DESCRIPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_STATE_EVENT:
            case AccessibleEventId.CONTROLLED_BY_PROPERTY:
            case AccessibleEventId.CONTROLLER_FOR_PROPERTY:
            case AccessibleEventId.LABEL_FOR_PROPERTY:
            case AccessibleEventId.LABELED_BY_PROPERTY:
            case AccessibleEventId.MEMBER_OF_PROPERTY:
            case AccessibleEventId.ACCESSIBLE_SELECTION_EVENT:
                handleEvent (xSource, AccessibleContextHandler.class);
                break;

            case AccessibleEventId.ACCESSIBLE_TABLE_CAPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_DESCRIPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_HEADER_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_DESCRIPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_HEADER_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_SUMMARY_CHANGED:
                handleEvent (xSource, AccessibleTableHandler.class);
                break;

            case AccessibleEventId.ACCESSIBLE_ACTION_EVENT:
                handleEvent (xSource, AccessibleActionHandler.class);
                break;

            case AccessibleEventId.ACCESSIBLE_HYPERTEXT_OFFSET:
                handleEvent (xSource, AccessibleHypertextHandler.class);
                break;

            case AccessibleEventId.ACCESSIBLE_ACTIVE_DESCENDANT_EVENT:
            case AccessibleEventId.ACCESSIBLE_CARET_EVENT:
            case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
            case AccessibleEventId.ACCESSIBLE_VALUE_EVENT:
                handleEvent (xSource, AccessibleTextHandler.class);
                break;

            default:
                break;
        }
    }


    //
    // canvas
    //

    public void setCanvas( Canvas aCanvas )
    {
        maCanvas = aCanvas;
    }

    protected void addToCanvas (AccTreeNode aNode)
    {
        if (maCanvas != null)
            maCanvas.addNode (aNode);
    }

    protected void removeFromCanvas (AccTreeNode aNode)
    {
        if (maCanvas != null)
            maCanvas.removeNode (aNode);
    }

    protected void updateOnCanvas (AccTreeNode aNode)
    {
        if (maCanvas != null)
            maCanvas.updateNode (aNode);
    }





    /** QueuedListener implements an AccessibleEventListener which
     * delegates all events to another such listener, but does so in a
     * seperate thread */
    class QueuedListener
        implements XAccessibleEventListener, Runnable
    {
        public QueuedListener()
        {
            System.out.println ("starting new queued listener");

            // initiate thread
            new Thread(this, "QueuedListener").start();
        }

        /** The queue of event objects, LinkedList<Runnable>
         * The queue object will also serve as lock for the
         * consumer/producer type syncronization.
         */
        protected LinkedList aQueue = new LinkedList();

        /// This thread's main method: deliver all events
        public void run()
        {
            // in an infinite loop, check for events to deliver, then
            // wait on lock (which will be notified when new events arrive)
            while( true )
            {
                Runnable aEvent = null;
                do
                {
                    synchronized( aQueue )
                    {
                        aEvent = (aQueue.size() > 0) ?
                            (Runnable)aQueue.removeFirst() : null;
                    }
                    if( aEvent != null )
                    {
                        System.out.println("Deliver event: " +
                                           aEvent.hashCode());
                        try
                        {
                            aEvent.run();
                        }
                        catch( Throwable e )
                        {
                            System.out.println(
                                "Exception during event delivery: " + e );
                        }
                    }
                }
                while( aEvent != null );

                try
                {
                    synchronized( aQueue )
                    {
                        aQueue.wait();
                    }
                }
                catch( Exception e )
                {
                    // can't wait? odd!
                    System.err.println("Can't wait!");
                }
            }
        }


        public void disposing( final EventObject aEvent)
        {
            System.out.println( "Queue disposing: " + aEvent.hashCode() );
            synchronized( aQueue )
            {
                aQueue.addLast( new Runnable()
                    {
                        public void run()
                        {
                            AccessibilityTreeModel.this.disposing( aEvent );
                        }
                        public int hashCode()
                        {
                            return aEvent.hashCode();
                        }
                    } );
                aQueue.notify();
            }
        }

        public void notifyEvent( final AccessibleEventObject aEvent )
        {
            System.out.println( "Queue notifyEvent: " + aEvent.hashCode() );
            synchronized( aQueue )
            {
                aQueue.addLast( new Runnable()
                    {
                        public void run()
                        {
                            AccessibilityTreeModel.this.notifyEvent( aEvent );
                        }
                        public int hashCode()
                        {
                            return aEvent.hashCode();
                        }
                    } );
                aQueue.notify();
            }
        }
    }

}
