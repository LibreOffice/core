
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

    // default handlers, Vector<HandlerPair>
    private static Vector aDefaultHandlers;
    private static NodeHandler maContextHandler = new AccessibleContextHandler();
    private static NodeHandler maTextHandler = new AccessibleTextHandler();
    private static NodeHandler maEditableTextHandler = new AccessibleEditableTextHandler();
    private static NodeHandler maComponentHandler = new AccessibleComponentHandler();
    private static NodeHandler maExtendedComponentHandler = new AccessibleExtendedComponentHandler();
    private static NodeHandler maActionHandler = new AccessibleActionHandler();
    private static NodeHandler maImageHandler = new AccessibleImageHandler();
    private static NodeHandler maTableHandler = new AccessibleTableHandler();
    private static NodeHandler maHypertextHandler = new AccessibleHypertextHandler();
    private static NodeHandler maHyperlinkHandler = new AccessibleHyperlinkHandler();
    private static NodeHandler maTreeHandler = new AccessibleTreeHandler();

    public AccessibilityTreeModel (Object aRoot, MessageInterface aMessageArea, Print aPrinter)
    {
        // create default node (unless we have a 'proper' node)
        if( ! (aRoot instanceof AccessibleTreeNode) )
            aRoot = new StringNode ("Root", null);
        maRoot = aRoot;
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
        if (mnLockCount == 0)
        {
            if (aNodeHint instanceof AccTreeNode)
                fireTreeStructureChanged (createEvent (((AccTreeNode)aNodeHint).getAccessible()));
        }
    }

    //
    // the root node
    //

    private Object maRoot;

    public Object getRoot()
    {
        return maRoot;
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
    public synchronized AccessibleTreeNode getChild (AccessibleTreeNode aParent, int nIndex)
        throws com.sun.star.lang.IndexOutOfBoundsException
    {
        AccessibleTreeNode aChild = null;
        if (aParent != null)
            aChild = aParent.getChild(nIndex);

        // Keep translation table up-to-date.
        if (aChild != null)
            if (aChild instanceof AccTreeNode)
                if (maXAccessibleToNode.get (((AccTreeNode)aChild).getAccessible()) == null)
                {
                    registerAccListener (aChild);
                    maXAccessibleToNode.put (((AccTreeNode)aChild).getAccessible(), aChild);
                    addToCanvas ((AccTreeNode)aChild);
                }

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
    protected synchronized void removeNode (AccessibleTreeNode aNode)
    {
        try
        {
            if( aNode == null )
            {
                System.out.println ("can't remove null node");
                return;
            }
            removeAccListener (aNode);
            removeFromCanvas (aNode);
            if (aNode instanceof AccTreeNode)
                maXAccessibleToNode.remove (((AccTreeNode)aNode).getAccessible());
            AccessibleTreeNode aParent = aNode.getParent();
            if (aParent != null)
            {
                int nIndex = aParent.indexOf(aNode);
                aParent.removeChild (nIndex);
            }

            // depth-first removal of children
            while (aNode.getChildCount() > 0)
                removeNode (aNode.getChild (0));
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            System.out.println ("caught exception while removing node " + aNode + " : " + e);
        }
    }

    protected synchronized boolean addNode (AccTreeNode aParentNode, XAccessible xNewChild)
    {
        boolean bRet = false;

        AccessibleTreeNode aChildNode = (AccessibleTreeNode)maXAccessibleToNode.get (xNewChild);
        if (aChildNode == null)
        {
            AccTreeNode aChild = (AccTreeNode)((AccTreeNode)aParentNode).addAccessibleChild (xNewChild);
            TreePath aPath = new TreePath (createPath (aChild));
            if (aChild != null)
            {
                registerAccListener (aChild);
                maXAccessibleToNode.put (aChild.getAccessible(), aChild);
                addToCanvas (aChild);
            }
            bRet = true;
        }
        else
            System.out.println ("node already present");

        return bRet;
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

    protected synchronized void fireTreeStructureChanged(final TreeModelEvent e)
    {
        System.out.println("treeStructureChanged: " + e);
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            ((TreeModelListener)maTMListeners.get(i)).treeStructureChanged(e);
        }
    }

    protected TreeModelEvent createEvent (XAccessible xParent)
    {
        return createEvent (xParent, null );
    }

    /** Create a TreeModelEvent object that informs listeners that one child
        has been removed from or inserted into its parent.
    */
    protected TreeModelEvent createEvent (XAccessible xParent, XAccessible xChild)
    {
        // get parent node and create the tree path
        AccessibleTreeNode aParentNode = (AccessibleTreeNode)maXAccessibleToNode.get (xParent);
        Object[] aPathToParent = createPath (aParentNode);

        AccessibleTreeNode aChildNode = null;
        if (xChild != null)
            aChildNode = (AccessibleTreeNode)maXAccessibleToNode.get (xChild);
        int nIndexInParent = -1;
        if (xChild != null)
            nIndexInParent = aParentNode.indexOf (aChildNode);

        System.out.println (aParentNode + " " + aChildNode);

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
            aNode.addHandler (maHypertextHandler.createHandler (xContext));
            aNode.addHandler (maHyperlinkHandler.createHandler (xContext));
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
            sDisplay = xContext.getAccessibleName();
        else
            sDisplay = new String ("not accessible");


        // create node, and add default handlers
        AccTreeNode aNode = new AccTreeNode (xContext, sDisplay, aParent);
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
    }

    public void disposing( EventObject aEvent)
    {
        /*        System.out.println("dispose: " + objectToString(aEvent.Source));

        if( knowsNode( aEvent.Source ) )
        {
            System.out.println("ERROR: Dispose for living node called! " +
                               "Maybe notifications don't work?");
            removeNode( aEvent.Source );
//            fireTreeStructureChanged( createEvent( getRoot() ) );
        }
        */
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
        "[UNKNOWN]"
    };

    /** This method is called from accessible objects that broadcast
        modifications of themselves or from their children.  The event is
        processed only, except printing some messages, if the tree is not
        locked.  It should be locked during changes to its internal
        structure like expanding nodes.
    */
    public synchronized void notifyEvent( AccessibleEventObject aEvent )
    {

        int nId = aEvent.EventId;
        if( (nId < 0) || (nId >= aEventNames.length) )
            nId = 0;

        System.out.println( "notify: " + aEvent.EventId + " "
            + aEventNames[nId] + ": "
            + objectToString(aEvent.Source) + " "
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
                    removeNode ((AccessibleTreeNode)maXAccessibleToNode.get (xOld));
                    fireTreeNodesRemoved (aRemoveEvent);
                }
                // Insertion and removal of children should be mutually
                // exclusive.  But then there is this 'should' ...
                if (aEvent.NewValue != null)
                {
                    XAccessible xNew = (XAccessible)UnoRuntime.queryInterface(
                        XAccessible.class,aEvent.NewValue);
                    // Create event after inserting it so that its new index
                    // in the parent can be determined.
                    AccessibleTreeNode aParentNode = (AccessibleTreeNode)maXAccessibleToNode.get (xSource);
                    if (aParentNode instanceof AccTreeNode)
                    {
                        if (addNode ((AccTreeNode)aParentNode, xNew))
                        {
                            ((AccTreeNode)aParentNode).update ();
                            updateOnCanvas (xSource);

                            // A call to fireTreeNodesInserted for xNew
                            // should be sufficient but at least the
                            // StringNode object that contains the number of
                            // children also changes and we do not know its
                            // index relative to its parent.  Therefore the
                            // more expensive fireTreeStructureChanged is
                            // necessary.
                            fireTreeStructureChanged (createEvent (xSource));
                        }
                    }
                }
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
            case AccessibleEventId.ACCESSIBLE_ACTIVE_DESCENDANT_EVENT:
            case AccessibleEventId.ACCESSIBLE_ACTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_CARET_EVENT:
            case AccessibleEventId.ACCESSIBLE_DESCRIPTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_HYPERTEXT_OFFSET:
            case AccessibleEventId.ACCESSIBLE_NAME_EVENT:
            case AccessibleEventId.ACCESSIBLE_SELECTION_EVENT:
            case AccessibleEventId.ACCESSIBLE_STATE_EVENT:
            case AccessibleEventId.ACCESSIBLE_TABLE_CAPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_DESCRIPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_COLUMN_HEADER_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_DESCRIPTION_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_ROW_HEADER_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TABLE_SUMMARY_CHANGED:
            case AccessibleEventId.ACCESSIBLE_TEXT_EVENT:
            case AccessibleEventId.ACCESSIBLE_VALUE_EVENT:
            case AccessibleEventId.CONTROLLED_BY_PROPERTY:
            case AccessibleEventId.CONTROLLER_FOR_PROPERTY:
            case AccessibleEventId.LABEL_FOR_PROPERTY:
            case AccessibleEventId.LABELED_BY_PROPERTY:
            case AccessibleEventId.MEMBER_OF_PROPERTY:
                //                fireTreeNodesChanged (createEvent
                //                (xSource));
                // Some child(s) of xSource have changed.  We do not know
                // which so all we can do is send a structure change event
                // to all listeners.
                fireTreeStructureChanged (createEvent (xSource));
                updateOnCanvas (xSource);
                break;



            default:
                break;
        }
    }


    //
    // canvas
    //

    private Canvas maCanvas;

    public void setCanvas( Canvas aCanvas )
    {
        maCanvas = aCanvas;
    }


    private XAccessibleContext getContext(Object aObject)
    {
        XAccessibleContext xAcc =
            (XAccessibleContext)UnoRuntime.queryInterface(
                 XAccessibleContext.class, aObject);
        return xAcc;
    }

    protected void addToCanvas (AccTreeNode aNode)
    {
        XAccessibleContext xContext = aNode.getContext();
        if ((maCanvas != null) && (xContext != null))
            maCanvas.addContext (
                xContext,
                new TreePath (createPath (aNode)));
    }

    protected void removeFromCanvas( Object aObject )
    {
        XAccessibleContext xContext = getContext( aObject );
        if( (maCanvas != null) && (xContext != null) )
            maCanvas.removeContext( xContext );
    }

    protected void updateOnCanvas( Object aObject )
    {
        XAccessibleContext xContext = getContext( aObject );
        if( (maCanvas != null) && (xContext != null) )
            maCanvas.updateContext( xContext );
    }





    /** QueuedListener implements an AccessibleEventListener which
     * delegates all events to another such listener, but does so in a
     * seperate thread */
    class QueuedListener
        implements XAccessibleEventListener, Runnable
    {
        public QueuedListener()
        {
            // initiate thread
            new Thread(this).start();
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


/** HandlerPair stores a NodeHandler and the appropriate type */
class HandlerPair
{
    public Class aType;
    public NodeHandler aHandler;

    public HandlerPair( Class aT, NodeHandler aH )
    {
        aType = aT;
        aHandler = aH;
    }
}
