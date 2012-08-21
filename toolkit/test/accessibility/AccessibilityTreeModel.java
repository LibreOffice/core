/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreePath;


import java.util.Vector;
import com.sun.star.accessibility.*;

import com.sun.star.uno.UnoRuntime;

public class AccessibilityTreeModel
    extends AccessibilityTreeModelBase
{
    public boolean mbVerbose = false;

    public AccessibilityTreeModel (AccessibleTreeNode aRoot)
    {
        // create default node (unless we have a 'proper' node)
        if( ! (aRoot instanceof AccessibleTreeNode) )
            aRoot = new StringNode ("Root", null);
        setRoot (aRoot);

        maNodeMap = new NodeMap();

        maEventListener = new EventListener (this);
        mxListener = new QueuedListener (maEventListener);
    }

    public void clear ()
    {
        maNodeMap.Clear();
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
            fireTreeStructureChanged (
                new TreeModelEvent (this,
                    new TreePath (aNodeHint.createPath())));
    }




    /** Inform all listeners (especially the renderer) of a change of the
        tree's structure.
        @param aNode This node specifies the sub tree in which all changes
        take place.
    */
    public void FireTreeStructureChanged (AccessibleTreeNode aNode)
    {
    }





    public synchronized void setRoot (AccessibleTreeNode aRoot)
    {
        if (getRoot() == null)
            super.setRoot (aRoot);
        else
        {
            lock ();
            maNodeMap.ForEach (new NodeMapCallback () {
                    public void Apply (AccTreeNode aNode)
                    {
                        if (maCanvas != null)
                            maCanvas.removeNode (aNode);
                        removeAccListener (aNode);
                    }
                });
            maNodeMap.Clear ();

            setRoot (aRoot);
            unlock (aRoot);
        }
    }


    //
    // child management:
    //



    /** Delegate the request to the parent and then register listeners at
        the child and add the child to the canvas.
    */
    public Object getChild (Object aParent, int nIndex)
    {
        AccessibleTreeNode aChild = (AccessibleTreeNode)super.getChild (aParent, nIndex);

        if (aChild == null)
            System.out.println ("getChild: child not found");
        else
            // Keep translation table up-to-date.
            addNode (aChild);

        return aChild;
    }

    public Object getChildNoCreate (Object aParent, int nIndex)
    {
        AccessibleTreeNode aChild = (AccessibleTreeNode)super.getChildNoCreate (aParent, nIndex);

        return aChild;
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
                    if ( ! removeChild (aNode.getChildNoCreate (0)))
                        break;

                // Remove node from its parent.
                AccessibleTreeNode aParent = aNode.getParent();
                if (aParent != null)
                {
                    int nIndex = aParent.indexOf(aNode);
                    aParent.removeChild (nIndex);
                }

                maNodeMap.RemoveNode (aNode);
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while removing child "
                + aNode + " : " + e);
            e.printStackTrace ();
            return false;
        }
        return true;
    }

    public void removeNode (XAccessibleContext xNode)
    {
        if (xNode != null)
        {
            AccessibleTreeNode aNode = maNodeMap.GetNode (xNode);
            AccessibleTreeNode aRootNode = (AccessibleTreeNode)getRoot();
            TreeModelEvent aEvent = createEvent (aRootNode, aNode);
            removeChild (aNode);
            if (mbVerbose)
                System.out.println (aNode);
            fireTreeNodesRemoved (aEvent);
            maCanvas.repaint ();
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
            aChildNode = maNodeMap.GetNode(xNewChild);
            if (aChildNode == null)
                aChildNode = aParentNode.addAccessibleChild (xNewChild);
            else
                System.out.println ("node already present");
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while adding child "
                + xNewChild + " to parent " + aParentNode + ": " + e);
            e.printStackTrace ();
        }
        return aChildNode;
    }

    public void addChild (XAccessibleContext xParent, XAccessible xChild)
    {
        AccessibleTreeNode aParentNode = maNodeMap.GetNode (xParent);
        if (aParentNode instanceof AccTreeNode)
        {
            AccessibleTreeNode aChild = addChild ((AccTreeNode)aParentNode, xChild);
            if (addNode (aChild))
            {
                if (maCanvas != null)
                    maCanvas.updateNode ((AccTreeNode)aParentNode);

                // A call to fireTreeNodesInserted for xNew
                // should be sufficient but at least the
                // StringNode object that contains the number of
                // children also changes and we do not know its
                // index relative to its parent.  Therefore the
                // more expensive fireTreeStructureChanged is
                // necessary.
                fireTreeNodesInserted (createEvent (xParent, xChild));
                updateNode (xParent, AccessibleTreeHandler.class);
            }
            maCanvas.repaint ();
        }
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
            if ( ! maNodeMap.ValueIsMember (aNode))
            {
                if (aNode instanceof AccTreeNode)
                {
                    AccTreeNode aChild = (AccTreeNode)aNode;
                    XAccessibleContext xChild = aChild.getContext();
                    registerAccListener (aChild);
                    if (maCanvas != null)
                        maCanvas.addNode (aChild);
                    maNodeMap.InsertNode (xChild, aChild);
                }
                bRet = true;
            }

        }
        catch (Exception e)
        {
            System.out.println ("caught exception while adding node "
                + aNode + ": " + e);
            e.printStackTrace ();
        }
        return bRet;
    }




    /** create path to node, suitable for TreeModelEvent constructor
     * @see javax.swing.event.TreeModelEvent#TreeModelEvent
     */
    protected Object[] createPath (AccessibleTreeNode aNode)
    {
        Vector<AccessibleTreeNode> aPath = new Vector<AccessibleTreeNode>();
        aNode.createPath (aPath);
        return aPath.toArray();
    }

    //
    // listeners (and helper methods)
    //
    // We are registered with listeners as soon as objects are in the
    // tree cache, and we should get removed as soon as they are out.
    //

    protected void fireTreeNodesChanged(TreeModelEvent e)
    {
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            maTMListeners.get(i).treeNodesChanged(e);
        }
    }

    protected void fireTreeNodesInserted(final TreeModelEvent e)
    {
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            maTMListeners.get(i).treeNodesInserted(e);
        }
    }

    protected void fireTreeNodesRemoved(final TreeModelEvent e)
    {
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            maTMListeners.get(i).treeNodesRemoved(e);
        }
    }

    protected void fireTreeStructureChanged(final TreeModelEvent e)
    {
        for(int i = 0; i < maTMListeners.size(); i++)
        {
            maTMListeners.get(i).treeStructureChanged(e);
        }
    }

    protected TreeModelEvent createEvent (XAccessibleContext xParent)
    {
        AccessibleTreeNode aParentNode = maNodeMap.GetNode (xParent);
        return new TreeModelEvent (this, createPath (aParentNode));
    }

    /** Create a TreeModelEvent object that informs listeners that one child
        has been removed from or inserted into its parent.
    */
    public TreeModelEvent createEvent (XAccessibleContext xParent, XAccessible xChild)
    {
        AccessibleTreeNode aParentNode = maNodeMap.GetNode (xParent);
        return createEvent (aParentNode, xParent);
    }

    public TreeModelEvent createEvent (AccessibleTreeNode aParentNode, XAccessibleContext xChild)
    {
        AccessibleTreeNode aChildNode = null;
        if (xChild != null)
            aChildNode = maNodeMap.GetNode (xChild);
        return createEvent (aParentNode, aChildNode);
    }



    protected TreeModelEvent createEvent (
        AccessibleTreeNode aParentNode,
        AccessibleTreeNode aChildNode)
    {
        Object[] aPathToParent = createPath (aParentNode);

        int nIndexInParent = -1;
        if (aChildNode != null)
            nIndexInParent = aParentNode.indexOf (aChildNode);
        if (mbVerbose)
            System.out.println (aChildNode + " " + nIndexInParent);

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
    protected TreeModelEvent createChangeEvent (AccTreeNode aNode, Vector<Integer> aChildIndices)
    {
        // Build a list of child objects that are indicated by the given indices.
        int nCount = aChildIndices.size();
        Object aChildObjects[] = new Object[nCount];
        int nChildIndices[] = new int[nCount];
        for (int i=0; i<nCount; i++)
        {
            int nIndex = aChildIndices.elementAt(i).intValue();
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
                fire( maTMListeners.get(i) );
            }
        }

        protected void fire( TreeModelListener l) { }
    }



    protected XAccessibleEventBroadcaster getBroadcaster (Object aObject)
    {
        if (aObject instanceof AccTreeNode)
            return UnoRuntime.queryInterface (
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
            xBroadcaster.addEventListener( mxListener );
        }
    }

    protected void removeAccListener( Object aObject )
    {
        XAccessibleEventBroadcaster xBroadcaster = getBroadcaster( aObject );
        if (xBroadcaster != null)
        {
            xBroadcaster.removeEventListener( mxListener );
        }
    }



    public void setCanvas (Canvas aCanvas)
    {
        maCanvas = aCanvas;
    }

    public Canvas getCanvas ()
    {
        return maCanvas;
    }

    public void updateNode (XAccessibleContext xSource, java.lang.Class class1)
    {
        updateNode (xSource, class1,null);
    }

    /** Get a list of children of the node associated with xSource that are
        affected by the given handlers.  Fire events that these children may
        have changed in the tree view.  Update the canvas representation of
        xSource.
    */
    public AccTreeNode updateNode (XAccessibleContext xSource,
        java.lang.Class class1, java.lang.Class<AccessibleExtendedComponentHandler> class2)
    {
        AccessibleTreeNode aTreeNode = maNodeMap.GetNode (xSource);
        AccTreeNode aNode = null;
        if (mbVerbose)
            System.out.println ("updating node " + xSource + "  " + aTreeNode);
        if (aTreeNode instanceof AccTreeNode)
        {
            aNode = (AccTreeNode) aTreeNode;
            // Get list of affected children.
            Vector<Integer> aChildIndices = (aNode).updateChildren (
                class1, class2);
            // Fire events that these children may have changed.
            fireTreeNodesChanged (
                createChangeEvent (aNode, aChildIndices));
        }
        return aNode;
    }

    /** The listener to be registered with the accessible objects.
     * Could be set to 'this' for same-thread event delivery, or to an
     * instance of QueuedListener for multi-threaded delivery.  May
     * not be changed, since this would trip the
     * register/removeAccListener logic. */
    private final XAccessibleEventListener mxListener;

    // Map to translate from accessible object to corresponding tree node.
    private NodeMap maNodeMap;

    // If the lock count is higher then zero, then no events are processed.
    private int mnLockCount;

    private Canvas maCanvas;

    private EventListener maEventListener;
}
