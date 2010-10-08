/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.accessibility.awb.canvas;

import java.awt.Rectangle;
import java.util.Iterator;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeModelEvent;
import javax.swing.tree.TreeNode;

import com.sun.star.accessibility.XAccessibleEventBroadcaster;

/** Each canvas has a shape container that is responsible for maintaining
    a collection of shapes that are displayed by the canvas.
*/
public class ShapeContainer
    implements TreeModelListener,
               TreeExpansionListener,
               TreeWillExpandListener
{
    public ShapeContainer (Canvas aCanvas)
    {
        maShapeList = new java.util.Hashtable();
        maBoundingBox = new Rectangle (0,0,100,100);
        maCanvas = aCanvas;
        maTree = null;
    }




    public synchronized void SetTree (javax.swing.JTree aTree)
    {
        if (aTree != maTree)
        {
            if (maTree != null)
            {
                maTree.getModel().removeTreeModelListener (this);
                maTree.removeTreeExpansionListener (this);
                maTree.removeTreeWillExpandListener (this);
            }

            Clear();

            maTree = aTree;

            maTree.getModel().addTreeModelListener (this);
            maTree.addTreeExpansionListener (this);
            maTree.addTreeWillExpandListener (this);
        }
    }




    public synchronized boolean AddNode (TreeNode aNode)
    {
        CanvasShape aShape = (CanvasShape)maShapeList.get (aNode);
        if (aShape == null)
        {
            aShape = new CanvasShape (aNode, maCanvas);

                        if (aNode instanceof XAccessibleEventBroadcaster)
                           ((XAccessibleEventBroadcaster) aNode).addEventListener(aShape);

            // Update bounding box that includes all objects.
           if (maShapeList.size() == 0)
               maBoundingBox = aShape.GetBBox();
           else
               maBoundingBox = maBoundingBox.union (aShape.GetBBox());

            maShapeList.put (aNode, aShape);

            maCanvas.repaint();

            return true;
        }
        else
            return false;
    }


    /**
    */
    public synchronized boolean RemoveNode (TreeNode aNode)
    {
        CanvasShape aShape = (CanvasShape)maShapeList.get (aNode);
        if (aShape != null)
        {
                        if (aNode instanceof XAccessibleEventBroadcaster)
                           ((XAccessibleEventBroadcaster) aNode).removeEventListener(aShape);

            maShapeList.remove (aNode);
            maCanvas.SelectObject (null);
            maCanvas.repaint ();
            return true;
        }
        else
            return false;
    }




    public synchronized void Clear ()
    {
        maShapeList.clear ();
    }




    public Iterator GetIterator ()
    {
        return maShapeList.values().iterator ();
    }




    public CanvasShape Get (TreeNode aNode)
    {
            if (aNode != null) {
        return (CanvasShape)maShapeList.get (aNode);
            }
            return null;
    }


    private void PrintMessage (String aMessage, java.util.EventObject aEvent)
    {
        //        System.out.println ("ShapeContainer: " + aMessage + ": " + aEvent);
    }

    public void treeNodesChanged (TreeModelEvent aEvent)
    {
        PrintMessage ("treeNodesChanged", aEvent);
    }
    public void treeNodesInserted (TreeModelEvent aEvent)
    {
        PrintMessage ("treeNodesInserted", aEvent);
        Object[] aNewNodes = aEvent.getChildren();
        for (int i=0; i<aNewNodes.length; i++)
            AddNode ((TreeNode)aNewNodes[i]);
    }
    public void treeNodesRemoved (TreeModelEvent aEvent)
    {
        PrintMessage ("treeNodesRemoved", aEvent);
        Object[] aOldNodes = aEvent.getChildren();
        for (int i=0; i<aOldNodes.length; i++)
            RemoveNode ((TreeNode)aOldNodes[i]);
    }
    public void treeStructureChanged (TreeModelEvent aEvent)
    {
        PrintMessage ("treeStructureChanged", aEvent);
        TreeNode aNode = (TreeNode)aEvent.getTreePath().getLastPathComponent();
        RemoveAllChildren(aNode);
        AddAllChildren(aNode);
    }

    public void treeWillExpand (TreeExpansionEvent aEvent)
    {
        PrintMessage ("treeWillExpand", aEvent);
    }
    public void treeWillCollapse (TreeExpansionEvent aEvent)
    {
        PrintMessage ("treeWillCollapse", aEvent);
        TreeNode aNode = (TreeNode)aEvent.getPath().getLastPathComponent();
        RemoveAllChildren (aNode);
    }
    public void treeExpanded (TreeExpansionEvent aEvent)
    {
        PrintMessage ("treeExpanded", aEvent);
        TreeNode aNode = (TreeNode)aEvent.getPath().getLastPathComponent();
        AddAllChildren (aNode);
    }
    public void treeCollapsed (TreeExpansionEvent aEvent)
    {
        PrintMessage ("treeCollapsed", aEvent);
    }

    private void AddAllChildren (TreeNode aNode) {
        java.util.Enumeration aChildList = aNode.children();
        while (aChildList.hasMoreElements()) {
            TreeNode aChild = (TreeNode) aChildList.nextElement();
            if (aChild != null) {
                AddAllChildren (aChild);
                AddNode (aChild);
            }
        }
    }

    private void RemoveAllChildren (TreeNode aNode) {
        java.util.Enumeration aChildList = aNode.children();
        while (aChildList.hasMoreElements()) {
            TreeNode aChild = (TreeNode) aChildList.nextElement();
            if (aChild != null) {
                RemoveAllChildren (aChild);
                RemoveNode (aChild);
            }
        }
    }


    private java.util.Hashtable maShapeList;
    private Rectangle maBoundingBox;
    private Canvas maCanvas;
    private javax.swing.JTree maTree;
}
