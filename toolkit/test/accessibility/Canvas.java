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

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.TreeSelectionListener;
import java.awt.geom.Rectangle2D;

/** This canvas displays accessible objects graphically.  Each accessible
    object with graphical representation is represented by an
    CanvasShape object and has to be added by the
    <member>addAccessible</member> member function.

    <p>The canvas listens to selection events of the associated JTree and
    highlights the first selected node of that tree.</p>
*/
class Canvas
    extends JPanel
    implements MouseListener, MouseMotionListener, TreeSelectionListener//, Scrollable
{
    // This constant can be passed to SetZoomMode to always show the whole screen.
    public static final int WHOLE_SCREEN = -1;

    public Canvas ()
    {
        super (true);
        maObjects = new java.util.HashMap ();
        maNodes = new Vector ();
        maObjectList = new Vector ();
        maContexts = new Vector ();
        addMouseListener (this);
        addMouseMotionListener (this);
        maBoundingBox = new Rectangle (0,0,100,100);
        maTree = null;
        mnHOffset = 0;
        mnVOffset = 0;
        mnScale = 1;
        setShowText(false);
        setShowDescriptions (true);
        setShowNames (true);
        setAntialiasing (true);
        maLastWidgetSize = new Dimension (0,0);
    }

    /** Tell the canvas which tree view to use to highlight accessible
        objects.
    */
    public void setTree (JTree aTree)
    {
        if (maTree != null)
            maTree.removeTreeSelectionListener (this);
        maTree = aTree;
        if (maTree != null)
            maTree.addTreeSelectionListener (this);
    }




    public void addNode (AccTreeNode aNode)
    {
        if (maNodes.indexOf (aNode) == -1)
        {
            maNodes.add (aNode);

            CanvasShape aObject = (CanvasShape) maObjects.get (aNode);
            if (aObject == null)
            {
                aObject = new CanvasShape (aNode);
                // Update bounding box that includes all objects.
                if (maObjects.size() == 0)
                    maBoundingBox = aObject.getBBox();
                else
                    maBoundingBox = maBoundingBox.union (aObject.getBBox());

                maObjects.put (aNode, aObject);
                maObjectList.add (aObject);

            }
            repaint ();
        }
    }

    public void removeNode (AccTreeNode aNode)
    {
        int i = maNodes.indexOf (aNode);
        if( i != -1 )
        {
            Object aObject = maObjects.get(aNode);
            maObjectList.remove (aObject);
            maObjects.remove (aObject);
            maNodes.remove (aNode);
            repaint ();
        }
    }

    public void updateNode (AccTreeNode aNode)
    {
        int i = maNodes.indexOf (aNode);
        if (i != -1)
        {
            CanvasShape aObject = (CanvasShape)maObjects.get(aNode);
            if (aObject != null)
                aObject.update();
        }
    }

    public void updateNodeGeometry (AccTreeNode aNode)
    {
        CanvasShape aObject = (CanvasShape)maObjects.get(aNode);
        if (aObject != null)
            aObject.updateGeometry();
    }

    public void clear ()
    {
        while (maNodes.size() > 0)
            removeNode ((AccTreeNode)maNodes.elementAt(0));

        maNodes.clear();
        maObjects.clear();
        maObjectList.clear();
    }

    public boolean getShowDescriptions ()
    {
        return Options.GetBoolean ("ShowDescriptions");
    }

    public void setShowDescriptions (boolean bNewValue)
    {
        Options.SetBoolean ("ShowDescriptions", bNewValue);
        repaint ();
    }

    public boolean getShowNames ()
    {
        return Options.GetBoolean ("ShowNames");
    }

    public void setShowNames (boolean bNewValue)
    {
        Options.SetBoolean ("ShowNames", bNewValue);
        repaint ();
    }

    public boolean getAntialiasing ()
    {
        return Options.GetBoolean ("Antialiasing");
    }

    public void setAntialiasing (boolean bNewValue)
    {
        Options.SetBoolean ("Antialiasing", bNewValue);
        repaint ();
    }

    public boolean getShowText ()
    {
        return Options.GetBoolean ("ShowText");
    }

    public void setShowText (boolean bNewValue)
    {
        Options.SetBoolean ("ShowText", bNewValue);
        repaint ();
    }

    public void setZoomMode (int nZoomMode)
    {
        Options.SetInteger ("ZoomMode", nZoomMode);
        repaint ();
    }

    public int getZoomMode ()
    {
        return Options.GetInteger ("ZoomMode", WHOLE_SCREEN);
    }


    public void paintComponent (Graphics g)
    {
        synchronized (g)
        {
            super.paintComponent (g);

            Graphics2D g2 = (Graphics2D)g;
            if (getAntialiasing())
                g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_ON);
            else
                g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_OFF);

            setupTransformation ();

            // Draw the screen representation to give a hint of the location of the
            // accessible object on the screen.
            Dimension aScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
            Rectangle2D.Double aScreen = new Rectangle2D.Double (
                mnHOffset,
                mnVOffset,
                mnScale*aScreenSize.getWidth(),
                mnScale*aScreenSize.getHeight());
            // Fill the screen rectangle and draw a frame arround it to increase its visibility.
            g2.setColor (new Color (250,240,230));
            g2.fill (aScreen);
            g2.setColor (Color.BLACK);
            g2.draw (aScreen);

            synchronized (maObjectList)
            {
                int nCount = maObjectList.size();
                boolean bShowDescriptions = getShowDescriptions();
                boolean bShowNames = getShowNames();
                boolean bShowText = getShowText();
                for (int i=0; i<nCount; i++)
                {
                    CanvasShape aCanvasShape = (CanvasShape)maObjectList.elementAt(i);
                    aCanvasShape.paint (
                        g2,
                        mnHOffset, mnVOffset, mnScale,
                        bShowDescriptions, bShowNames, bShowText);
                }
            }

            // Paint highlighted frame around active object as the last thing.
            if (maActiveObject != null)
                maActiveObject.paint_highlight (
                    g2,
                    mnHOffset, mnVOffset, mnScale);
        }
    }




    /** Set up the transformation so that the graphical display can show a
        centered representation of the whole screen.
    */
    private void setupTransformation ()
    {
        // Turn off scrollbars when showing the whole screen.  Otherwise show them when needed.
        JViewport aViewport = (JViewport)getParent();
        JScrollPane aScrollPane = (JScrollPane)aViewport.getParent();
        int nZoomMode = getZoomMode();
        if (nZoomMode == WHOLE_SCREEN)
        {
            if (aScrollPane.getHorizontalScrollBarPolicy()
                != JScrollPane.HORIZONTAL_SCROLLBAR_NEVER)
                aScrollPane.setHorizontalScrollBarPolicy (JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            if (aScrollPane.getVerticalScrollBarPolicy()
                != JScrollPane.VERTICAL_SCROLLBAR_NEVER)
                aScrollPane.setVerticalScrollBarPolicy (JScrollPane.VERTICAL_SCROLLBAR_NEVER);
        }
        else
        {
            if (aScrollPane.getHorizontalScrollBarPolicy()
                != JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED)
                aScrollPane.setHorizontalScrollBarPolicy (JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            if (aScrollPane.getVerticalScrollBarPolicy()
                != JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED)
                aScrollPane.setVerticalScrollBarPolicy (JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
        }

        Dimension aScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension aWidgetSize = aViewport.getSize();
        {
            if ((aScreenSize.getWidth() > 0) && (aScreenSize.getHeight() > 0))
            {
                if (nZoomMode == WHOLE_SCREEN)
                {
                    // Calculate the scales that would map the screen onto the
                    // widget in both of the coordinate axes and select the
                    // smaller
                    // of the two: it maps the screen onto the widget in both
                    // axes at the same time.
                    double nHScale = (aWidgetSize.getWidth() - 10) / aScreenSize.getWidth();
                    double nVScale = (aWidgetSize.getHeight() - 10) / aScreenSize.getHeight();
                    if (nHScale < nVScale)
                        mnScale = nHScale;
                    else
                        mnScale = nVScale;
                }
                else
                {
                    mnScale = nZoomMode / 100.0;
                }

                // Calculate offsets that center the scaled screen inside the widget.
                mnHOffset = (aWidgetSize.getWidth() - mnScale*aScreenSize.getWidth()) / 2.0;
                mnVOffset = (aWidgetSize.getHeight() - mnScale*aScreenSize.getHeight()) / 2.0;
                if (mnHOffset < 0)
                    mnHOffset = 0;
                if (mnVOffset < 0)
                    mnVOffset = 0;

                setPreferredSize (new Dimension (
                    (int)(2*mnHOffset + mnScale * aScreenSize.getWidth()),
                    (int)(2*mnVOffset + mnScale * aScreenSize.getHeight())));
                revalidate ();
            }
            else
            {
                // In case of a degenerate (not yet initialized?) screen size
                // use some meaningless default values.
                mnScale = 1;
                mnHOffset = 0;
                mnVOffset = 0;
            }
        }
        maLastWidgetSize = aWidgetSize;
    }



    /**  Call getAccessibleAt to determine accessible object under mouse.
    */
    public void mouseClicked (MouseEvent e)
    {
    }

    public void mousePressed (MouseEvent e)
    {
        CanvasShape aObjectUnderMouse = FindCanvasShapeUnderMouse (e);
        highlightObject (aObjectUnderMouse);
        if ((e.getModifiers() & InputEvent.CTRL_MASK) != 0)
        {
            maTree.expandPath (aObjectUnderMouse.getPath());
        }
    }

    public void mouseReleased (MouseEvent e)
    {
    }

    public void mouseEntered (MouseEvent e)
    {
    }

    public void mouseExited (MouseEvent e)
    {
        // Deselect currently active object.
        if (maActiveObject != null)
        {
            maActiveObject.unhighlight ();
            maActiveObject = null;
            repaint ();
        }
    }

    public void mouseDragged (MouseEvent e)
    {
    }

    public void mouseMoved (MouseEvent e)
    {
        if ((e.getModifiers() & InputEvent.SHIFT_MASK) != 0)
            highlightObject (FindCanvasShapeUnderMouse (e));
    }

    protected CanvasShape FindCanvasShapeUnderMouse (MouseEvent e)
    {
        int nObjects = maObjects.size();
        CanvasShape aObjectUnderMouse = null;
        int nCount = maObjectList.size();
        for (int i=nCount-1; i>=0; --i)
        {
            CanvasShape aObject = (CanvasShape)maObjectList.elementAt(i);
            if (aObject != null)
                if (aObject.contains (e.getX(),e.getY()))
                {
                    aObjectUnderMouse = aObject;
                    break;
                }
        }
        return aObjectUnderMouse;
    }

    protected boolean highlightObject (CanvasShape aNewActiveObject)
    {
        if (aNewActiveObject != maActiveObject)
        {
            if (maActiveObject != null)
                maActiveObject.unhighlight();

            maActiveObject = aNewActiveObject;
            if (maActiveObject != null)
            {
                if (maTree != null)
                {
                    maTree.scrollPathToVisible (maActiveObject.getPath());
                    maTree.setSelectionPath (maActiveObject.getPath());
                    maTree.repaint ();
                }
                maActiveObject.highlight ();
                repaint ();
            }
            return true;
        }
        else
            return false;
    }

    /** Called when the selection of the tree changes.  Highlight the
        corresponding graphical representation of the first selected object.
    */
    public void valueChanged (javax.swing.event.TreeSelectionEvent event)
    {
        TreePath aPath = event.getPath();
        Object aObject = aPath.getLastPathComponent();
        if (aObject instanceof AccTreeNode)
        {
            CanvasShape aCanvasShape = (CanvasShape)maObjects.get ((AccTreeNode)aObject);
            if (highlightObject (aCanvasShape))
                repaint();
        }
    }

    private int
        mnXAnchor,
        mnYAnchor,
        maResizeFlag;
    private double
        mnHOffset,
        mnVOffset,
        mnScale;
    private CanvasShape
        maActiveObject;
    private java.util.HashMap
        maObjects;
    private Vector
        maObjectList,
        maContexts,
        maNodes;
    private Rectangle
        maBoundingBox;
    private JTree
        maTree;
    // The size of the widget at the last call of setupTransformation()
    private Dimension
        maLastWidgetSize;
}
