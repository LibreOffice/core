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

// JDK 1.4.
// import java.util.regex.Pattern;
// import java.util.regex.Matcher;



/** This class is a start to collect the handling of a JTree and a DefaultTreeModel.
*/
public class AccessibilityTree
    extends JTree
{
    /** Create a new accessibility tree.  Use the specified message display
        for displaying messages and the specified canvas to draw the
        graphical representations of accessible objects on.
    */
    public AccessibilityTree (
        MessageInterface aMessageDisplay,
        Print aPrinter)
    {
        maMessageDisplay = aMessageDisplay;
        maPrinter = aPrinter;

        AccessibilityTreeModel aModel =
            new AccessibilityTreeModel (
                new StringNode ("Please press Update button", null),
                aMessageDisplay,
                aPrinter);
        setModel (aModel);

        maCellRenderer = new AccessibleTreeCellRenderer();
        //        setCellRenderer (maCellRenderer);

        // allow editing of XAccessibleText interfaces
        //        setEditable (true);
        //        maTreeModel.addTreeModelListener( new TextUpdateListener() );

        addMouseListener (new MouseListener (this));
    }


    public void SetCanvas (Canvas aCanvas)
    {
        maCanvas = aCanvas;
        ((AccessibilityTreeModel)getModel()).setCanvas (maCanvas);
    }

    /** Predicate class to determine whether a node should be expanded
     * For use with expandTree method */
    abstract class Expander
    { abstract public boolean expand(Object aObject);  }

    /** expand all nodes */
    class AllExpander extends Expander
    {
        public boolean expand(Object aObject) { return true; }
    }

    /** expand all nodes with accessibility roles > 100 */
    class ShapeExpander extends Expander
    {
        public boolean expand (Object aObject)
        {
            if (aObject instanceof AccTreeNode)
            {
                AccTreeNode aNode = (AccTreeNode)aObject;
                XAccessibleContext xContext = aNode.getContext();
                if (xContext != null)
                    if (xContext.getAccessibleRole() >= 100)
                        return true;
            }
            return false;
        }
    }

    /** Expand the nodes in the subtree rooted in aNode according to the the
        specified expander.  The tree is locked during the expansion.
    */
    protected void expandTree (AccessibleTreeNode aNode, Expander aExpander)
    {
        message ("Expanding tree");

        setEnabled (false);
        ((AccessibilityTreeModel)getModel()).lock ();

        try
        {
            expandTree (new TreePath (aNode.createPath()), aExpander);
        }
        catch (Exception e)
        {
            // Ignore
        }

        setEnabled (true);
        ((AccessibilityTreeModel)getModel()).unlock (aNode);
        message ("");
    }

    private TreePath expandTree( TreePath aPath, Expander aExpander )
    {
        // return first expanded object
        TreePath aFirst = null;

        //        System.out.print ("e");

        try
        {
            // get 'our' object
            Object aObj = aPath.getLastPathComponent();

            // expand this object, if the Expander tells us so
            if( aExpander.expand( aObj ) )
            {
                expandPath (aPath);
                if( aFirst == null )
                    aFirst = aPath;
            }

            // visit all children
            if (aObj instanceof AccessibleTreeNode)
            {
                AccessibleTreeNode aNode = (AccessibleTreeNode)aObj;
                int nLength = aNode.getChildCount();
                for( int i = 0; i < nLength; i++ )
                {
                    TreePath aRet = expandTree(
                        aPath.pathByAddingChild( aNode.getChild( i ) ),
                        aExpander );
                    if( aFirst == null )
                        aFirst = aRet;
                }
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while expanding tree path " + aPath + ": " + e);
        }

        return aFirst;
    }


    /** Expand all nodes and their subtrees that represent shapes.  Call
     *  this method from the outside. */
    public void expandShapes ()
    {
        expandShapes ((AccessibleTreeNode)getModel().getRoot());
    }
    public void expandShapes (AccessibleTreeNode aNode)
    {
        expandTree (aNode, new ShapeExpander());
    }

    /** Expand all nodes */
    public void expandAll ()
    {
        expandAll ((AccessibleTreeNode)getModel().getRoot());
    }
    public void expandAll (AccessibleTreeNode aNode)
    {
        expandTree (aNode, new AllExpander());
    }



    protected void message (String message)
    {
        maMessageDisplay.message (message);
    }

    public void disposing (com.sun.star.lang.EventObject e)
    {
        System.out.println ("disposing " + e);
    }



    class MouseListener extends MouseAdapter
    {
        private AccessibilityTree maTree;
        public MouseListener (AccessibilityTree aTree) {maTree=aTree;}
        public void mousePressed(MouseEvent e) { popupTrigger(e); }
        public void mouseClicked(MouseEvent e) { popupTrigger(e); }
        public void mouseEntered(MouseEvent e) { popupTrigger(e); }
        public void mouseExited(MouseEvent e) { popupTrigger(e); }
        public void mouseReleased(MouseEvent e) { popupTrigger(e); }

        public boolean popupTrigger( MouseEvent e )
        {
            boolean bIsPopup = e.isPopupTrigger();
            if( bIsPopup )
            {
                int selRow = getRowForLocation(e.getX(), e.getY());
                if (selRow != -1)
                {
                    TreePath aPath = getPathForLocation(e.getX(), e.getY());

                    // check for actions
                    Object aObject = aPath.getLastPathComponent();
                    if( aObject instanceof AccTreeNode )
                    {
                        AccTreeNode aNode = (AccTreeNode)aObject;

                        JPopupMenu aMenu = new JPopupMenu();

                        Vector aActions = new Vector();
                        aMenu.add (new ShapeExpandAction(maTree, aNode));
                        aMenu.add (new SubtreeExpandAction(maTree, aNode));

                        aNode.getActions(aActions);
                        for( int i = 0; i < aActions.size(); i++ )
                        {
                            aMenu.add( new NodeAction(
                                           aActions.elementAt(i).toString(),
                                           aNode, i ) );
                        }

                        aMenu.show( AccessibilityTree.this, e.getX(), e.getY() );
                    }
                }
            }

            return bIsPopup;
        }
    }

    class NodeAction extends AbstractAction
    {
        private int mnIndex;
        private AccTreeNode maNode;

        public NodeAction( String aName, AccTreeNode aNode, int nIndex )
        {
            super( aName );
            maNode = aNode;
            mnIndex = nIndex;
        }

        public void actionPerformed(ActionEvent e)
        {
            maNode.performAction(mnIndex);
        }
    }
    // This action expands all shapes in the subtree rooted in the specified node.
    class ShapeExpandAction extends AbstractAction
    {
        private AccessibilityTree maTree;
        private AccTreeNode maNode;
        public ShapeExpandAction (AccessibilityTree aTree, AccTreeNode aNode)
        {
            super ("Expand Shapes");
            maTree = aTree;
            maNode = aNode;
        }
        public void actionPerformed (ActionEvent e)
        {
            maTree.expandShapes (maNode);
        }
    }
    // This action expands all nodes in the subtree rooted in the specified node.
    class SubtreeExpandAction extends AbstractAction
    {
        private AccessibilityTree maTree;
        private AccTreeNode maNode;
        public SubtreeExpandAction (AccessibilityTree aTree, AccTreeNode aNode)
        {
            super ("Expand Subtree");
            maTree = aTree;
            maNode = aNode;
        }
        public void actionPerformed (ActionEvent e)
        {
            maTree.expandAll (maNode);
        }
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
                (aIndices.length > 0) )
            {
                // we have a parent... lets check for XAccessibleText then
                DefaultMutableTreeNode aParent = (DefaultMutableTreeNode)
                    (e.getTreePath().getLastPathComponent());
                DefaultMutableTreeNode aNode = (DefaultMutableTreeNode)
                    (aParent.getChildAt(aIndices[0]));
                if( aParent.getUserObject() instanceof XAccessibleText)
                {
                    // aha! we have an xText. So we can now check for
                    // the various cases we support
                    XAccessibleText xText =
                        (XAccessibleText)aParent.getUserObject();

                    if( aIndices[0] == 0 )
                    {
                        // first child! Then we call updateText
                        updateText( xText, aNode.toString() );
                    }
                    else
                    {
// JDK 1.4:
//                        // check for pattern "Selection:"
//                         Matcher m = Pattern.compile(
//                             "selection: \\[(-?[0-9]+),(-?[0-9]+)\\] \".*" ).
//                             matcher( aNode.toString() );
//                         if( m.matches() )
//                         {
//                             try
//                             {
//                                 // aha! Selection:
//                                 setSelection( xText,
//                                               Integer.parseInt(m.group(1)),
//                                               Integer.parseInt(m.group(2)) );
//                             }
//                             catch( NumberFormatException f )
//                             {
//                                 // ignore
//                             }
//                         }
                    }
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

        boolean setSelection( XAccessibleText xText, int p1, int p2 )
        {
            try
            {
                return xText.setSelection( p1, p2 );
            }
            catch( com.sun.star.lang.IndexOutOfBoundsException f )
            {
                return false;
            }
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
    protected Print
        maPrinter;
    protected AccessibleTreeCellRenderer
        maCellRenderer;

    private Canvas
        maCanvas;
    private boolean
        mbFirstShapeSeen;
}
