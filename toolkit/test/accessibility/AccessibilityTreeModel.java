
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
    public AccessibilityTreeModel( Object aRoot )
    {
        // create default node (unless we have a 'proper' node)
        if( ! (aRoot instanceof AccTreeNode) )
            aRoot = createDefaultNode( aRoot );
        maRoot = aRoot;

        aListeners = new Vector();
        aParents = new HashMap();
        aChildren = new HashMap();
        aNodes = new HashMap();
        aNodes.put( normalize(maRoot), maRoot );

        // syncronous or asyncronous event delivery? (i.e. same thread
        // or in s seperate event delivery thread)
        // xListener = this;                // syncronous event delivery
        xListener = new QueuedListener();   // asyncronous event delivery
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
        return (aParent instanceof AccTreeNode) ?
            ((AccTreeNode)aParent).getChildCount() : 0;
    }

    public Object getChild(Object aParent, int nIndex)
    {
        Object aRet = (aParent instanceof AccTreeNode) ?
            ((AccTreeNode)aParent).getChild(nIndex) : null;
        setNode( aParent, nIndex, aRet );   // update tree cache
        return aRet;
    }

    /** iterate over all children and look for child */
    public int getIndexOfChild(Object aParent, Object aChild)
    {
        aChild = normalize( aChild );

        // compare to all children
        int nChildCount = getChildCount( aParent );
        for( int i = 0; i < nChildCount; i++ )
        {
            if( aChild.equals( normalize( getChild(aParent, i) ) ) )
                return i;
        }

        // not found?
        return -1;
    }


    public boolean isLeaf(Object aNode)
    {
        return (aNode instanceof AccTreeNode) ?
            ((AccTreeNode)aNode).isLeaf() : true;
    }


    //
    // tree cache and child cache
    //

    /** store parents of an object in the tree; HashMap<Object> */
    private HashMap aParents;

    /** store a vector with the children in each tree;
     *  HashMap<Vector<Object>> */
    private HashMap aChildren;

    /** store a mapping of 'normalized' nodes to nodes in the tree */
    private HashMap aNodes;


    /** normalize an object reference:
     * 0) for an Any, get the containing Object
     * 1) for AccTreeNode, get the data object
     * 2) for XAccessible, get the accessible object
     * 3) cast all UNO objects to XInterface
     */
    static protected Object normalize(Object aObject)
    {
        // 0) for an Any, get the containing Object
        if( aObject instanceof Any )
            aObject = ((Any)aObject).getObject();

        // 1) for AccTreeNode, get the data object
        if( aObject instanceof AccTreeNode )
            aObject = ((AccTreeNode)aObject).getDataObject();

        if( aObject instanceof XInterface )
        {
            // 2) for XAccessible, get the accessible object
            XAccessible xAcc = (XAccessible)UnoRuntime.queryInterface(
                                   XAccessible.class, aObject);
            if( xAcc != null )
                aObject = xAcc.getAccessibleContext();

            // 3) cast all UNO objects to XInterface
            aObject = (XInterface)UnoRuntime.queryInterface( XInterface.class,
                                                             aObject);
        }
        return aObject;
    }

    /** add a parent/child int the node cache */
    protected void setNode(Object aOParent, int nIndex, Object aOChild)
    {
        Object aParent = normalize(aOParent);
        Object aChild = normalize(aOChild);

        // store nodes
        aNodes.put( aParent, aOParent );
        aNodes.put( aChild, aOChild );

        // store the child's parent
        aParents.put( aChild, aParent );

        // store the parent's child
        Vector aKids = (Vector)aChildren.get( aParent );
        if( aKids == null )
        {
            // no children known... insert a new vector
            aKids = new Vector();
            aChildren.put( aParent, aKids );
        }

        // make sure we have enough room
        if( aKids.size() <= nIndex )
            aKids.setSize( nIndex + 1 ); // alternative: use child count

        // add child at appropriate positions; remove old listeners,
        // get new ones
        removeAccListener( aKids.elementAt( nIndex ) );
        aKids.setElementAt( aChild, nIndex );
        registerAccListener( aKids.elementAt( nIndex ) );

        // update canvas
        addToCanvas( aChild );
    }

    /** remove a node (and all children) from the node cache */
    protected void removeNode(Object aNode)
    {
        if( aNode == null )
            return;

        aNode = normalize( aNode );
        removeAccListener( aNode );

        // get parent + kids (before this information is deleted)
        Vector aKids = (Vector)aChildren.get( aNode );
        Object aParent = aParents.get( aNode );

        // remove information about this node
        removeFromCanvas( aNode );        // update canvas
        aParents.remove( aNode );
        aChildren.remove( aNode );
        aNodes.remove( aNode );

        // depth-first removal of children
        int nLength = (aKids == null) ? 0 : aKids.size();
        for( int i = nLength-1; i >= 0; i-- )
        {
            removeNode( aKids.elementAt(i) );
        }

        // remove from parents' child vector
        if( aParent != null )
        {
            Vector aParentKids = (Vector)aChildren.get( aParent );
            if( aParentKids != null )
                aParentKids.remove( aNode );
        }
    }

    /** determine whether this node is in the node cache */
    protected boolean knowsNode(Object aNode)
    {
        // we never 'know' null; we 'know' a node if it has a parent
        return (aNode != null) && (aNodes.get(normalize(aNode)) != null);
    }

    protected Object getNode(Object aObject)
    {
        return aNodes.get( normalize(aObject) );
    }

    /** recursively traverse aParents, and add nodes to path in
     * post-order fashion. This causes the root to be the first
     * element, just as demanded by TreeModelEvent.
     * @see javax.swing.event.TreeModelEvent#TreeModelEvent
     */
    private void createPath(Object aNode, Vector aPath)
    {
        aNode = normalize(aNode);
        Object aParent = aParents.get(aNode);
        if( aParent != null )
            createPath( aParent, aPath );
        Object aPathElem = aNodes.get(aNode);
        if( aPathElem != null )
            aPath.add( aPathElem );
        else
            System.out.println("Unknown node in path! (" + aNode +
                               ", pos. " + aPath.size() + ")");
    }

    /** create path to node, suitable for TreeModelEvent constructor
     * @see javax.swing.event.TreeModelEvent#TreeModelEvent
     */
    protected Object[] createPath(Object aNode)
    {
        Vector aPath = new Vector();
        createPath( aNode, aPath );
        return aPath.toArray();
    }

    //
    // listeners (and helper methods)
    //
    // We are registered with listeners as soon as objects are in the
    // tree cache, and we should get removed as soon as they are out.
    //

    Vector aListeners;

    public void addTreeModelListener(TreeModelListener l)
    {
        aListeners.add(l);
    }

    public void removeTreeModelListener(TreeModelListener l)
    {
        aListeners.remove(l);
    }

    protected void fireTreeNodesChanged(TreeModelEvent e)
    {
        System.out.println("treeNodesChanges: " + e);
        for(int i = 0; i < aListeners.size(); i++)
        {
            ((TreeModelListener)aListeners.get(i)).treeNodesChanged(e);
        }
    }

    protected void fireTreeNodesInserted(final TreeModelEvent e)
    {
        System.out.println("treeNodesInserted: " + e);
        for(int i = 0; i < aListeners.size(); i++)
        {
            ((TreeModelListener)aListeners.get(i)).treeNodesInserted(e);
        }
    }

    protected void fireTreeNodesRemoved(final TreeModelEvent e)
    {
        System.out.println("treeNodesRemoved: " + e);
        for(int i = 0; i < aListeners.size(); i++)
        {
            ((TreeModelListener)aListeners.get(i)).treeNodesRemoved(e);
        }
    }

    protected void fireTreeStructureChanged(final TreeModelEvent e)
    {
        System.out.println("treeStructureChanged: " + e);
        for(int i = 0; i < aListeners.size(); i++)
        {
            ((TreeModelListener)aListeners.get(i)).treeStructureChanged(e);
        }
    }

    protected TreeModelEvent createEvent( Object aNode )
    {
        return createEvent( aNode, null );
    }

    protected TreeModelEvent createEvent( Object aParent, Object aChild )
    {
        // get parent node and create the tree path
        Object aParentNode = getNode( aParent );
        Object[] aPath = createPath( aParentNode );

        // if we already know the node (e.g. when deleting a node),
        // use the position from the node cache. Else (e.g. when
        // inserting a node), call getIndexOfChild to look for it.
        int nIndex =-1;
        if( aChild != null )
        {
            if( knowsNode( aChild ) )
            {
                Vector aKids = (Vector)aChildren.get( aParent );
                if( aKids != null )
                {
                    nIndex = aKids.indexOf( getNode( aChild ) );
                }
            }
            else
                nIndex = getIndexOfChild( aParentNode, aChild );
        }


        // If we have a position, broadcast the position, otherwise
        // create a 'position-less' event without the child info.
        return ( nIndex == -1 )
            ? new TreeModelEvent( this, aPath )
                : new TreeModelEvent( this, aPath,
                                      new int[] { nIndex },
                                      new Object[] { getNode( aChild ) } );
    }

    /**
     * broadcast a tree event in a seperate Thread
     * must override fire method
     */
    class EventRunner implements Runnable
    {
        public void run()
        {
            for(int i = 0; i < aListeners.size(); i++)
            {
                fire( (TreeModelListener)aListeners.get(i) );
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


    protected XAccessibleEventBroadcaster getBroadcaster( Object aObject )
    {
        if( aObject instanceof AccTreeNode )
            aObject = ((AccTreeNode)aObject).getDataObject();
        return (XAccessibleEventBroadcaster) UnoRuntime.queryInterface (
            XAccessibleEventBroadcaster.class, aObject);
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

    // default handlers, Vector<HandlerPair>
    private static Vector aDefaultHandlers;

    // initialize default handlers
    static
    {
        aDefaultHandlers = new Vector();
        aDefaultHandlers.add(
            new HandlerPair( Vector.class,
                             new VectorHandler() ) );

        aDefaultHandlers.add(
            new HandlerPair( XAccessibleContext.class,
                             new AccessibleContextHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleContext.class,
                             new AccessibleTreeHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleText.class,
                             new AccessibleTextHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleComponent.class,
                             new AccessibleComponentHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleExtendedComponent.class,
                             new AccessibleExtendedComponentHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleAction.class,
                             new AccessibleActionHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleImage.class,
                             new AccessibleImageHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleTable.class,
                             new AccessibleTableHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleEditableText.class,
                             new AccessibleEditableTextHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleHypertext.class,
                             new AccessibleHypertextHandler() ) );
        aDefaultHandlers.add(
            new HandlerPair( XAccessibleHyperlink.class,
                             new AccessibleHyperlinkHandler() ) );

        // ... ADD NEW DEFAULT HANDLERS HERE ...

    }



    /** add default handlers based on the supported interfaces */
    public static void addDefaultHandlers( AccTreeNode aNode )
    {
        Object aObject = aNode.getDataObject();

        // try each type
        Enumeration aEnum = aDefaultHandlers.elements();
        while( aEnum.hasMoreElements() )
        {
            HandlerPair aPair = (HandlerPair)aEnum.nextElement();
            Class aType = aPair.aType;

            // try instanceof, and a UNO query, if we have an XInterface
            if( aType.isInstance( aObject ) )
            {
                aNode.addHandler( aPair.aHandler );
            }
            else if( XInterface.class.isAssignableFrom( aType ) )
            {
                Object aQuery = UnoRuntime.queryInterface(aType, aObject);
                if( aQuery != null )
                    aNode.addHandler( aPair.aHandler );
            }
        }
    }

    /** create a node with the default handlers */
    public static AccTreeNode createDefaultNode( Object aObject )
    {
        // default: aObject + aDisplay
        Object aDisplay = aObject;

        // if we are accessible, we use the context + name instead
        XAccessible xAccessible =
            (XAccessible) UnoRuntime.queryInterface (
                XAccessible.class, aObject);
        if (xAccessible != null)
        {
            XAccessibleContext aContext = xAccessible.getAccessibleContext();

            // for accessibles, use context + name!
            aObject = aContext;
            aDisplay = aContext.getAccessibleName();
        }

        // create node, and add default handlers
        AccTreeNode aNode = new AccTreeNode( aObject, aDisplay );
        AccessibilityTreeModel.addDefaultHandlers( aNode );
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
        System.out.println("dispose: " + objectToString(aEvent.Source));

        if( knowsNode( aEvent.Source ) )
        {
            System.out.println("ERROR: Dispose for living node called! " +
                               "Maybe notifications don't work?");
            removeNode( aEvent.Source );
//            fireTreeStructureChanged( createEvent( getRoot() ) );
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
        "[UNKNOWN]"
    };

    public void notifyEvent( AccessibleEventObject aEvent )
    {

        int nId = aEvent.EventId;
        if( (nId < 0) || (nId >= aEventNames.length) )
            nId = 0;

        Object aSource = normalize( aEvent.Source );
        Object aOld = normalize( aEvent.OldValue );
        Object aNew = normalize( aEvent.NewValue );

        System.out.println( "notify: " + aEvent.EventId + " " +
                            aEventNames[nId] + ": " +
                            objectToString(aEvent.Source) + " (" +
                            (knowsNode(aEvent.Source) ? "known" : "unknown") +
                            "), " +
                            objectToString(aEvent.OldValue) + "->" +
                            objectToString(aEvent.NewValue) );


        switch( aEvent.EventId )
        {
            case AccessibleEventId.ACCESSIBLE_CHILD_EVENT:
                // fire insertion and deletion events:
                if( aOld != null )
                {
                    TreeModelEvent aTreeEvent = createEvent( aSource, aOld );
                    removeNode( aOld );
                    fireTreeNodesRemoved( aTreeEvent );
                }
                if( aNew != null )
                {
                    fireTreeNodesInserted( createEvent( aSource, aNew ) );
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
                fireTreeNodesChanged( createEvent( aSource ) );
                updateOnCanvas( aSource );
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

    protected void addToCanvas( Object aObject )
    {
        XAccessibleContext xContext = getContext( aObject );
        if( (maCanvas != null) && (xContext != null) )
            maCanvas.addContext( xContext,
                                 new TreePath( createPath( xContext ) ) );
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
