import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;
import drafts.com.sun.star.accessibility.XAccessibleAction;
import drafts.com.sun.star.accessibility.XAccessibleImage;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;
import drafts.com.sun.star.accessibility.XAccessibleText;
import drafts.com.sun.star.accessibility.XAccessibleEditableText;
import drafts.com.sun.star.accessibility.AccessibleTextType;
import drafts.com.sun.star.accessibility.XAccessibleEventListener;
import drafts.com.sun.star.accessibility.XAccessibleEventBroadcaster;
import drafts.com.sun.star.accessibility.AccessibleEventObject;
import drafts.com.sun.star.accessibility.AccessibleEventId;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;

import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;



/** This class is a start to collect the handling of a JTree and a DefaultTreeModel.
*/
public class AccessibilityTree
    extends JTree
    implements XAccessibleEventListener
{
    /** Create a new accessibility tree.  Use the specified message display
        for displaying messages and the specified canvas to draw the
        graphical representations of accessible objects on.
    */
    public AccessibilityTree (
        MessageInterface aMessageDisplay,
        Canvas aCanvas)
    {
        maMessageDisplay = aMessageDisplay;
        maCanvas = aCanvas;

        maRoot = new DefaultMutableTreeNode ("Accessibility Tree");
        maTreeModel = new DefaultTreeModel (maRoot);
        setModel (maTreeModel);

        setEditable (true);

        maCellRenderer = new AccessibleTreeCellRenderer();
        setCellRenderer (maCellRenderer);

        MouseListener ml = new MouseAdapter() {
                public void mousePressed(MouseEvent e) {
                    int selRow = getRowForLocation(e.getX(), e.getY());
                    TreePath selPath = getPathForLocation(e.getX(), e.getY());
                    if(selRow != -1) {
                        if (e.getClickCount() == 2)
                        {
                            System.out.println ("double click : " + selRow + " , " + selPath);
                            Object aObject = selPath.getLastPathComponent();
                            if (TreeNode.class.isInstance (aObject))
                            {
                                TreeNode aNode = (TreeNode)aObject;
                                createTree (aNode.maAccessibleObject.getAccessible(),
                                    selPath);
                                expandShapes ();
                            }
                        }
                    }
                }
            };
        addMouseListener (ml);

        // allow editing of XAccessibleText interfaces
        maTreeModel.addTreeModelListener( new TextUpdateListener() );
    }




    public void createTree (XAccessible xRoot)
    {
        createTree (xRoot, new TreePath (maRoot));
    }




    public void createTree (XAccessible xRoot, TreePath aPath)
    {
        TreeNode aTree = createAccessibilityTree (
            xRoot,
            0,
            new java.awt.Point(0,0),
            aPath);
        DefaultMutableTreeNode aRoot = (DefaultMutableTreeNode)aPath.getLastPathComponent();
        aRoot.insert (aTree, aRoot.getChildCount());
        maTreeModel.reload ();
    }



    public DefaultMutableTreeNode getRoot ()
    {
        return maRoot;
    }




    protected TreeNode createAccessibilityTree (
        XAccessible xRoot, int depth, java.awt.Point aOrigin, TreePath aPath)
    {
        TreeNode aRoot = null;

        try
        {
            aRoot = new TreeNode (xRoot, this);

            // Register as event listener.
            XAccessibleEventBroadcaster xEventBroadcaster =
                (XAccessibleEventBroadcaster) UnoRuntime.queryInterface (
                    XAccessibleEventBroadcaster.class, xRoot);
            if (xEventBroadcaster != null)
                xEventBroadcaster.addEventListener (this);


            TreePath aNewPath = aPath.pathByAddingChild (aRoot);

            // Create the accessible object and register this tree as listener at them.
            AccessibleObject aObject = new AccessibleObject (xRoot, aNewPath);

            aRoot.maAccessibleObject = aObject;
            message ("creating accessibility tree at depth " + depth + ": "
                     + aObject.toString());

            if (maCanvas != null)
            {
                if (aObject.getOrigin().x != 0 || aObject.getOrigin().y != 0)
                {
                    maCanvas.addAccessible (aObject);
                }
            }
            else
                System.out.println ("no canvas");

            aOrigin = aObject.getOrigin ();


            //  Iterate over accessible children, create their subtrees and
            //  inserert these trees into the new node.
            XAccessibleContext xContext = xRoot.getAccessibleContext();
            if (xContext != null)
            {
                int n = xContext.getAccessibleChildCount();
                for (int i=0; i<n; i++)
                {
                    DefaultMutableTreeNode aNode =
                        createAccessibilityTree (xContext.getAccessibleChild(i),
                            depth+1, aOrigin, aNewPath);
                    aRoot.insert (aNode, aRoot.getChildCount());
                }
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while creating accessibility tree: " + e);
        }

        return aRoot;
    }


    /** Remove this tree as event listener from all the nodes in the subtree
        of the specified root.
    */
    protected void removeEventListeners (DefaultMutableTreeNode aRoot)
    {
        java.util.Enumeration aEnumeration = aRoot.breadthFirstEnumeration();
        while (aEnumeration.hasMoreElements())
        {
            Object aElement = aEnumeration.nextElement();
            if (TreeNode.class.isInstance (aElement))
            {
                TreeNode aNode = (TreeNode)aElement;
                XAccessibleEventBroadcaster xEventBroadcaster =
                    (XAccessibleEventBroadcaster) UnoRuntime.queryInterface (
                        XAccessibleEventBroadcaster.class, aNode.getContext());
                if (xEventBroadcaster != null)
                    xEventBroadcaster.addEventListener (this);
            }
        }
    }


    /** Expand all nodes and their subtrees that represent shapes.  Call
        this method from the outside.
    */
    public void expandShapes ()
    {
        message ("Expanding shapes.");

        mbFirstShapeSeen = false;
        // Iterate over all nodes in the tree.
        java.util.Enumeration aEnumeration = maRoot.breadthFirstEnumeration();
        while (aEnumeration.hasMoreElements())
        {
            Object aElement = aEnumeration.nextElement();
            if (TreeNode.class.isInstance (aElement))
            {
                TreeNode aNode = (TreeNode)aElement;
                AccessibleObject aAccessibleObject = (AccessibleObject)aNode.maAccessibleObject;
                if (aAccessibleObject != null)
                    // Expand every node that has one of the new OO roles.
                    if (aAccessibleObject.getRole() >= 100)
                    {
                        TreePath aPath = new TreePath (aNode.getPath());
                        expandPath (aPath);
                        if ( ! mbFirstShapeSeen)
                        {
                            mbFirstShapeSeen = true;
                            makeVisible (aPath);
                        }
                    }
            }
        }
    }




    public void addDocument (XAccessible xAccessible, String sURL)
    {
        if (maTreeModel != null && maRoot != null)
        {
            if (sURL.length() == 0)
                sURL = "<unnamed>";
            TreeNode aNode = new TreeNode ("Document: " + sURL);
            aNode.maAccessibleObject = new AccessibleObject (xAccessible,
                new TreePath (maRoot).pathByAddingChild(aNode));
            maRoot.insert (aNode, maRoot.getChildCount());
        }
        maTreeModel.reload ();
    }


    public void clear ()
    {
        removeEventListeners (maRoot);
        maRoot.removeAllChildren();
    }


    /** Search for node in tree that represents the specified accessible object.
        If found return this node, else return null.
    */
    public TreeNode getNodeForXAccessible (XAccessible xAccessible)
    {
        try
        {
            java.util.Enumeration aEnumeration = maRoot.depthFirstEnumeration ();
            while (aEnumeration.hasMoreElements())
            {
                Object aElement = aEnumeration.nextElement();
                if (TreeNode.class.isInstance (aElement))
                {
                    TreeNode aNode = (TreeNode)aElement;
                    if (aNode != null)
                    {
                        AccessibleObject aAccessibleObject = (AccessibleObject)aNode.maAccessibleObject;
                        if (aAccessibleObject!=null)
                            if (aAccessibleObject.getAccessible().equals (xAccessible))
                                return aNode;
                    }
                }
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while getting XAccessible: " + e);
        }
        return null;
    }


    /** Callback for accessible notify events.
    */
    public void notifyEvent (AccessibleEventObject e)
    {
        try
        {
            System.out.println ("notify Event ("+e.EventId+") : " + e);
            XAccessible xSource = (XAccessible) UnoRuntime.queryInterface (
                XAccessible.class, e.Source);
            switch (e.EventId)
            {
                case AccessibleEventId.ACCESSIBLE_CHILD_EVENT:
                    XAccessible aOldChild = (XAccessible)e.OldValue;
                    XAccessible aNewChild = (XAccessible)e.NewValue;
                    if (aNewChild == null)
                    {
                        System.out.println ("child event deletion of " + aOldChild);
                        removeNode (getNodeForXAccessible (aOldChild));
                    }
                    else
                    {
                        System.out.println ("adding child "+ aNewChild);
                        addNode (getNodeForXAccessible (xSource), aNewChild);
                    }
                    break;

                case AccessibleEventId.ACCESSIBLE_VISIBLE_DATA_EVENT:
                    System.out.println ("xSource = " + xSource);
                    System.out.println ("Source = " + e.Source);
                    System.out.println ("old and new values =" + e.OldValue + ", " + e.NewValue);
                    updateNode (getNodeForXAccessible (xSource));
                    break;

                default:
                    System.out.println ("   event has unhandled id " + e.EventId);

            }
        }
        catch (Exception event)
        {
            System.out.println ("caught exception processing notifyEvent: " + event);
        }
    }




    /** Remove the specified node and its sub-tree from the tree and the canvas.
        @param aNode
            The root node of the sub-tree to remove.  If it is null then
            nothing happens.
    */
    protected void removeNode (TreeNode aRoot)
    {
        if (aRoot != null)
        {
            System.out.println ("   removing node " + aRoot);
            removeEventListeners (aRoot);
            if (TreeNode.class.isInstance (aRoot.getParent()))
                ((TreeNode)aRoot.getParent()).updateChildCount();
            aRoot.removeFromParent();
            maTreeModel.reload ();
            expandShapes ();

            // Remove all nodes of the subtree from the canvas by iterating
            // over an enumeration of that tree.
            System.out.println ("removing object from canvas");
            java.util.Enumeration aSubtree = aRoot.depthFirstEnumeration();
            while (aSubtree.hasMoreElements())
            {
                Object aNode = aSubtree.nextElement ();
                if (TreeNode.class.isInstance (aNode))
                    maCanvas.removeAccessible (((TreeNode)aNode).maAccessibleObject);
            }
        }
    }




    /** Add the specified node to the tree and the canvas.
        @param aRoot
            The root node under which to add the new child.  If it is null then
            nothing happens.
        @param aNewChild
            The new child node to add.
    */
    protected void addNode (TreeNode aRoot, XAccessible aNewChild)
    {
        if (aRoot != null)
        {
            aRoot.insert (
                createAccessibilityTree (
                    aNewChild,
                    0,
                    aRoot.maAccessibleObject.getOrigin(),
                    new TreePath (aRoot.getPath())),
                aRoot.getChildCount());
            maTreeModel.reload ();
            expandShapes ();
        }
    }


    /** Update the visible data of the specified node.  This method is
        called after resize and movement operations of objects.
    */
    protected void updateNode (TreeNode aNode)
    {
        try
        {
            System.out.println ("aNode = " + aNode);
            if (aNode != null)
            {
                maCellRenderer.clearAllChanges ();

                // Update the graphical represenation of the node.
                aNode.maAccessibleObject.update ();

                // Create a new list to collect all changed nodes in.
                Vector aChangedNodes = new Vector ();
                if (aNode.maAccessibleObject != null)
                {
                    aNode.updateVisibleData (aChangedNodes);
                    maCellRenderer.addChangedNodes (aChangedNodes, this);
                }

                // Repaint the tree and the canvas.
                repaint ();
                maCanvas.repaint ();
            }
        }
        catch (Exception event)
        {
            System.out.println ("caught exception while updating a node: " + event);
        }
    }


    protected void message (String message)
    {
        maMessageDisplay.message (message);
    }


    /** listen to tree model changes in order to update XAccessibleText objects
     */
    class TextUpdateListener implements TreeModelListener
    {
        public void treeNodesChanged(TreeModelEvent e)
        {
            // if the change is to the first child of a DefaultMutableTreeNode
            // with an XAccessibleText child, then we call updateText
            int[] aIndices = e.getChildIndices();
            if( (aIndices != null) &&
                (aIndices.length > 0) &&
                (aIndices[0] == 0) )
            {
                // so there is a first child, check for XAccessibleText then
                DefaultMutableTreeNode aParent = (DefaultMutableTreeNode)
                    (e.getTreePath().getLastPathComponent());
                DefaultMutableTreeNode aNode = (DefaultMutableTreeNode)
                    (aParent.getChildAt(0));

                if( aParent.getUserObject() instanceof XAccessibleText)
                {
                    // joy! Call updateText with the data
                    XAccessibleText xText =
                        (XAccessibleText)aParent.getUserObject();
                    updateText( xText, aNode.toString() );

//                     // and update the tree (by deleting the old children)
//                     // (this is a work-around for the currently missing
//                     //  notifications)
//                     updateNode( xText, aParent );
                }
            }
        }

        // don't care:
        public void treeNodesInserted(TreeModelEvent e) { ; }
        public void treeNodesRemoved(TreeModelEvent e) { ; }
        public void treeStructureChanged(TreeModelEvent e) { ; }

        /** update the text */
        boolean updateText( XAccessibleText xText, String sNew )
        {
            // is this text editable? if not, fudge you and return
            XAccessibleEditableText xEdit =
                (XAccessibleEditableText) UnoRuntime.queryInterface (
                            XAccessibleEditableText.class, xText);
            if (xEdit == null)
                return false;

            String sOld = xText.getText();

            // false alarm? Early out if no change was done!
            if( sOld.equals( sNew ) )
                return false;

            // get the minimum length of both strings
            int nMinLength = sOld.length();
            if( sNew.length() < nMinLength )
                nMinLength = sNew.length();

            // count equal characters from front and end
            int nFront = 0;
            while( (nFront < nMinLength) &&
                   (sNew.charAt(nFront) == sOld.charAt(nFront)) )
                nFront++;
            int nBack = 0;
            while( (nBack < nMinLength) &&
                   ( sNew.charAt(sNew.length()-nBack-1) ==
                     sOld.charAt(sOld.length()-nBack-1)    ) )
                nBack++;
            if( nFront + nBack > nMinLength )
                nBack = nMinLength - nFront;

            // so... the first nFront and the last nBack characters
            // are the same. Change the others!
            String sDel = sOld.substring( nFront, sOld.length() - nBack );
            String sIns = sNew.substring( nFront, sNew.length() - nBack );

            System.out.println("edit text: " +
                               sOld.substring(0, nFront) +
                               " [ " + sDel + " -> " + sIns + " ] " +
                               sOld.substring(sOld.length() - nBack) );

            boolean bRet = false;
            try
            {
                // edit the text, and use
                // (set|insert|delete|replace)Text as needed
                if( nFront+nBack == 0 )
                    bRet = xEdit.setText( sIns );
                else if( sDel.length() == 0 )
                    bRet = xEdit.insertText( sIns, nFront );
                else if( sIns.length() == 0 )
                    bRet = xEdit.deleteText( nFront, sOld.length()-nBack );
                else
                    bRet = xEdit.replaceText(nFront, sOld.length()-nBack,sIns);
            }
            catch( IndexOutOfBoundsException e )
            {
                bRet = false;
            }

            return bRet;
        }

//         /** replace the given node with a new xText node */
//         void updateNode( XAccessibleText xText,
//                          DefaultMutableTreeNode aNode )
//         {
//             // create a new node
//             DefaultMutableTreeNode aNew = newTextTreeNode( xText );
//
//             // get parent (must be DefaultMutableTreeNode)
//             DefaultMutableTreeNode aParent =
//                 (DefaultMutableTreeNode)aNode.getParent();
//             if( aParent != null )
//             {
//                 // remove old sub-tree, and insert new one
//                 int nIndex = aParent.getIndex( aNode );
//                 aParent.remove( nIndex );
//                 aParent.insert( aNew, nIndex );
//             }
//         }
    }


    protected MessageInterface
        maMessageDisplay;
    protected AccessibleTreeCellRenderer
        maCellRenderer;

    private DefaultMutableTreeNode
        maRoot;
    private DefaultTreeModel
        maTreeModel;
    private Canvas
        maCanvas;
    private boolean
        mbFirstShapeSeen;
}
