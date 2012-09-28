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

import com.sun.star.accessibility.*;
import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;



/** This is the tree component that is responsible for displaying the
    contents of the tree model on the screen.
*/
public class AccessibilityTree
    implements TreeExpansionListener, TreeWillExpandListener
{
    /** Create a new accessibility tree.  Use the specified message display
        for displaying messages and the specified canvas to draw the
        graphical representations of accessible objects on.
    */
    public AccessibilityTree ()
    {
        maTree = new JTree ();

        AccessibilityTreeModel aModel =
            new AccessibilityTreeModel (
                new StringNode ("Please press Update button", null));
        maTree.setModel (aModel);

        maCellRenderer = new AccessibleTreeCellRenderer();
        //        setCellRenderer (maCellRenderer);

        // allow editing of XAccessibleText interfaces
        //        setEditable (true);
        //        maTreeModel.addTreeModelListener( new TextUpdateListener() );

        maTree.addMouseListener (new MouseListener (this));

        // Listen to expansions and collapses to change the mouse cursor.
        mnExpandLevel = 0;
        maTree.addTreeWillExpandListener (this);
        maTree.addTreeExpansionListener (this);
    }

    public JTree getComponent ()
    {
        return maTree;
    }

    // Change cursor during expansions to show the user that this is a
    // lengthy operation.
    public void treeWillExpand (TreeExpansionEvent e)
    {
        if (mnExpandLevel == 0)
        {
            maTree.setCursor (new Cursor (Cursor.WAIT_CURSOR));
        }
        mnExpandLevel += 1;
    }
    public void treeWillCollapse (TreeExpansionEvent e)
    {
        if (mnExpandLevel == 0)
        {
            maTree.setCursor (new Cursor (Cursor.WAIT_CURSOR));
        }
        mnExpandLevel += 1;
    }
    public void treeExpanded (TreeExpansionEvent e)
    {
        mnExpandLevel -= 1;
        if (mnExpandLevel == 0)
        {
            maTree.setCursor (new Cursor (Cursor.DEFAULT_CURSOR));
        }
    }
    public void treeCollapsed (TreeExpansionEvent e)
    {
        mnExpandLevel -= 1;
        if (mnExpandLevel == 0)
        {
            maTree.setCursor (new Cursor (Cursor.DEFAULT_CURSOR));
        }
    }



    public void SetCanvas (Canvas aCanvas)
    {
        maCanvas = aCanvas;
        ((AccessibilityTreeModel)maTree.getModel()).setCanvas (maCanvas);
    }

    /** Expand the nodes in the subtree rooted in aNode according to the the
        specified expander.  The tree is locked during the expansion.
    */
    protected void expandTree (AccessibleTreeNode aNode, Expander aExpander)
    {
        if (mnExpandLevel == 0)
        {
            maTree.setEnabled (false);
        }
        mnExpandLevel += 1;

        ((AccessibilityTreeModel)maTree.getModel()).lock ();

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
            maTree.setEnabled (true);
            ((AccessibilityTreeModel)maTree.getModel()).unlock (aNode);
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
                maTree.expandPath (aPath);
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
        expandShapes ((AccessibleTreeNode)maTree.getModel().getRoot());
    }
    public void expandShapes (AccessibleTreeNode aNode)
    {
        expandTree (aNode, new ShapeExpander());
    }

    /** Expand all nodes */
    public void expandAll ()
    {
        expandAll ((AccessibleTreeNode)maTree.getModel().getRoot());
    }
    public void expandAll (AccessibleTreeNode aNode)
    {
        expandTree (aNode, new AllExpander());
    }



    public void disposing (com.sun.star.lang.EventObject e)
    {
        System.out.println ("disposing " + e);
    }

    /*
    public Dimension getPreferredSize ()
    {
        Dimension aPreferredSize = super.getPreferredSize();
        Dimension aMinimumSize = super.getMinimumSize();
        if (aPreferredSize.width < aMinimumSize.width)
            aPreferredSize.width = aMinimumSize.width;
        return aPreferredSize;
    }
    */

    class MouseListener extends MouseAdapter
    {
        public MouseListener (AccessibilityTree aTree)
        {
            maTree=aTree;
        }
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
                int selRow = maTree.getComponent().getRowForLocation(e.getX(), e.getY());
                if (selRow != -1)
                {
                    TreePath aPath = maTree.getComponent().getPathForLocation(e.getX(), e.getY());

                    // check for actions
                    Object aObject = aPath.getLastPathComponent();
                    JPopupMenu aMenu = new JPopupMenu();
                    if( aObject instanceof AccTreeNode )
                    {
                        AccTreeNode aNode = (AccTreeNode)aObject;

                        Vector<String> aActions = new Vector<String>();
                        aMenu.add (new AccessibilityTree.ShapeExpandAction(maTree, aNode));
                        aMenu.add (new AccessibilityTree.SubtreeExpandAction(maTree, aNode));

                        aNode.getActions(aActions);
                        for( int i = 0; i < aActions.size(); i++ )
                        {
                            aMenu.add( new NodeAction(
                                           aActions.elementAt(i).toString(),
                                           aNode, i ) );
                        }
                    }
                    else if (aObject instanceof AccessibleTreeNode)
                    {
                        AccessibleTreeNode aNode = (AccessibleTreeNode)aObject;
                        String[] aActionNames = aNode.getActions();
                        int nCount=aActionNames.length;
                        if (nCount > 0)
                        {
                            for (int i=0; i<nCount; i++)
                                aMenu.add( new NodeAction(
                                    aActionNames[i],
                                    aNode,
                                    i));
                        }
                        else
                            aMenu = null;
                    }
                    if (aMenu != null)
                        aMenu.show (maTree.getComponent(),
                            e.getX(), e.getY());
                }
            }

            return bIsPopup;
        }

        private AccessibilityTree maTree;
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

    /** Predicate class to determine whether a node should be expanded
     * For use with expandTree method */
    abstract class Expander
    {
        abstract public boolean expand (Object aObject);
    }

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



    protected AccessibleTreeCellRenderer
        maCellRenderer;


    private JTree
        maTree;
    private Canvas
        maCanvas;
    private boolean
        mbFirstShapeSeen;
    private int
        mnExpandLevel;
}
