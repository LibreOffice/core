import drafts.com.sun.star.accessibility.*;
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
    implements TreeExpansionListener, TreeWillExpandListener
{
    /** Create a new accessibility tree.  Use the specified message display
        for displaying messages and the specified canvas to draw the
        graphical representations of accessible objects on.
    */
    public AccessibilityTree ()
    {
        AccessibilityTreeModel aModel =
            new AccessibilityTreeModel (
                new StringNode ("Please press Update button", null));
        setModel (aModel);

        maCellRenderer = new AccessibleTreeCellRenderer();
        //        setCellRenderer (maCellRenderer);

        // allow editing of XAccessibleText interfaces
        //        setEditable (true);
        //        maTreeModel.addTreeModelListener( new TextUpdateListener() );

        addMouseListener (new MouseListener (this));

        // Listen to expansions and collapses to change the mouse cursor.
        mnExpandLevel = 0;
        addTreeWillExpandListener (this);
        addTreeExpansionListener (this);
    }

    // Change cursor during expansions to show the user that this is a
    // lengthy operation.
    public void treeWillExpand (TreeExpansionEvent e)
    {
        if (mnExpandLevel == 0)
        {
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
        }
        mnExpandLevel += 1;
    }
    public void treeWillCollapse (TreeExpansionEvent e)
    {
        if (mnExpandLevel == 0)
        {
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
        }
        mnExpandLevel += 1;
    }
    public void treeExpanded (TreeExpansionEvent e)
    {
        mnExpandLevel -= 1;
        if (mnExpandLevel == 0)
        {
            setCursor (new Cursor (Cursor.DEFAULT_CURSOR));
        }
    }
    public void treeCollapsed (TreeExpansionEvent e)
    {
        mnExpandLevel -= 1;
        if (mnExpandLevel == 0)
        {
            setCursor (new Cursor (Cursor.DEFAULT_CURSOR));
        }
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
        if (mnExpandLevel == 0)
        {
            setEnabled (false);
        }
        mnExpandLevel += 1;

        ((AccessibilityTreeModel)getModel()).lock ();

        try
        {
            expandTree (new TreePath (aNode.createPath()), aExpander);
        }
        catch (Exception e)
        {
            // Ignore
        }

        mnExpandLevel -= 1;
        if (mnExpandLevel == 0)
        {
            setEnabled (true);
            ((AccessibilityTreeModel)getModel()).unlock (aNode);
        }
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
            System.out.println ("caught exception while expanding tree path "
                + aPath + ": " + e);
            e.printStackTrace ();
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



    public void disposing (com.sun.star.lang.EventObject e)
    {
        System.out.println ("disposing " + e);
    }


    public Dimension getPreferredSize ()
    {
        Dimension aPreferredSize = super.getPreferredSize();
        Dimension aMinimumSize = super.getMinimumSize();
        if (aPreferredSize.width < aMinimumSize.width)
            aPreferredSize.width = aMinimumSize.width;
        return aPreferredSize;
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
                    else if (aObject instanceof AccessibleTreeNode)
                    {
                        AccessibleTreeNode aNode = (AccessibleTreeNode)aObject;
                        String[] aActionNames = aNode.getActions();
                        int nCount=aActionNames.length;
                        if (nCount > 0)
                        {
                            JPopupMenu aMenu = new JPopupMenu();
                            for (int i=0; i<nCount; i++)
                                aMenu.add( new NodeAction(
                                    aActionNames[i],
                                    aNode,
                                    i));
                            aMenu.show (AccessibilityTree.this,
                                e.getX(), e.getY());
                        }
                    }
                }
            }

            return bIsPopup;
        }
    }

    class NodeAction extends AbstractAction
    {
        private int mnIndex;
        private AccessibleTreeNode maNode;

        public NodeAction( String aName, AccessibleTreeNode aNode, int nIndex )
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



    protected AccessibleTreeCellRenderer
        maCellRenderer;


    private Canvas
        maCanvas;
    private boolean
        mbFirstShapeSeen;
    private int
        mnExpandLevel;
}
