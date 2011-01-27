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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.geom.Rectangle2D;
import java.util.Iterator;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JViewport;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.tree.TreePath;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;

import org.openoffice.accessibility.misc.Options;

/** This canvas displays accessible objects graphically.  Each accessible
    object with graphical representation is represented by an
    CanvasShape object and has to be added by the
    <member>addAccessible</member> member function.

    <p>The canvas listens to selection events of the associated JTree and
    highlights the first selected node of that tree.</p>
*/
public class Canvas
    extends JPanel
{
    // This constant can be passed to SetZoomMode to always show the whole screen.
    public static final int WHOLE_SCREEN = -1;

    public Canvas ()
    {
        super (true);
        maShapeList = new ShapeContainer (this);
        maMouseObserver = new MouseObserver (this);
        maTree = null;
        mnHOffset = 0;
        mnVOffset = 0;
        mnScale = 1;
        maLastWidgetSize = new Dimension (0,0);
    }



    /** Tell the canvas which tree to use to highlight accessible
        objects and to observe for changes in the tree structure.
    */
    public void SetTree (javax.swing.JTree aTree)
    {
        if (aTree != maTree)
        {
            maTree = aTree;
            maShapeList.SetTree (maTree);
            maMouseObserver.SetTree (maTree);
        }
    }




    private void Clear ()
    {
        maShapeList.Clear();
    }




    public Iterator GetShapeIterator ()
    {
        return maShapeList.GetIterator();
    }




    public void paintComponent (Graphics g)
    {
        synchronized (g)
        {
            super.paintComponent (g);

            Graphics2D g2 = (Graphics2D)g;
            if (Options.GetBoolean("Antialiasing"))
                g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_ON);
            else
                g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                    RenderingHints.VALUE_ANTIALIAS_OFF);

            setupTransformation ();
            g2.translate (mnHOffset, mnVOffset);
            g2.scale (mnScale, mnScale);

            // Draw the screen representation to give a hint of the location of the
            // accessible object on the screen.
            Dimension aScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
            Rectangle2D.Double aScreen = new Rectangle2D.Double (
                0,
                0,
                aScreenSize.getWidth(),
                aScreenSize.getHeight());
            // Fill the screen rectangle and draw a frame arround it to increase its visibility.
            g2.setColor (new Color (250,240,230));
            g2.fill (aScreen);
            g2.setColor (Color.BLACK);
            g2.draw (aScreen);

            synchronized (maShapeList)
            {
                Iterator aShapeIterator = maShapeList.GetIterator();
                boolean bShowDescriptions = Options.GetBoolean ("ShowDescriptions");
                boolean bShowNames = Options.GetBoolean ("ShowNames");
                boolean bShowText = Options.GetBoolean ("ShowText");
                while (aShapeIterator.hasNext())
                {
                    CanvasShape aCanvasShape =
                        (CanvasShape)aShapeIterator.next();
                    try
                    {
                        aCanvasShape.paint (
                            g2,
                            bShowDescriptions, bShowNames, bShowText);
                    }
                    catch (Exception aException)
                    {
                        System.err.println ("caught exception while painting a shape:"
                            + aException);
                        aException.printStackTrace (System.err);
                    }
                }
            }

            // Paint highlighted frame around active object as the last thing.
            if (maActiveObject != null)
                maActiveObject.paint_highlight (g2);
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
        int nZoomMode = Options.GetInteger ("ZoomMode", WHOLE_SCREEN);
        if (nZoomMode == WHOLE_SCREEN)
        {
            if (aScrollPane.getHorizontalScrollBarPolicy()
                != JScrollPane.HORIZONTAL_SCROLLBAR_NEVER)
                aScrollPane.setHorizontalScrollBarPolicy (
                    JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            if (aScrollPane.getVerticalScrollBarPolicy()
                != JScrollPane.VERTICAL_SCROLLBAR_NEVER)
                aScrollPane.setVerticalScrollBarPolicy (
                    JScrollPane.VERTICAL_SCROLLBAR_NEVER);
        }
        else
        {
            if (aScrollPane.getHorizontalScrollBarPolicy()
                != JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED)
                aScrollPane.setHorizontalScrollBarPolicy (
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
            if (aScrollPane.getVerticalScrollBarPolicy()
                != JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED)
                aScrollPane.setVerticalScrollBarPolicy (
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
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
                    double nHScale = (aWidgetSize.getWidth() - 10)
                           / aScreenSize.getWidth();
                    double nVScale = (aWidgetSize.getHeight() - 10)
                        / aScreenSize.getHeight();
                    if (nHScale < nVScale)
                        mnScale = nHScale;
                    else
                        mnScale = nVScale;
                }
                else
                {
                    mnScale = nZoomMode / 100.0;
                }

                // Calculate offsets that center the scaled screen inside
                // the widget.
                mnHOffset = (aWidgetSize.getWidth()
                             - mnScale*aScreenSize.getWidth()) / 2.0;
                mnVOffset = (aWidgetSize.getHeight()
                             - mnScale*aScreenSize.getHeight()) / 2.0;
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



    protected boolean HighlightObject (CanvasShape aNewActiveObject)
    {
        if (aNewActiveObject != maActiveObject)
        {
            if (maActiveObject != null)
                maActiveObject.Highlight (false);

            maActiveObject = aNewActiveObject;
            if (maActiveObject != null)
            {
                /*           if (maTree != null)
                {
                    TreePath aPath = new TreePath (
                        maActiveObject.GetNode().GetPath());
                    maTree.scrollPathToVisible (aPath);
                    maTree.setSelectionPath (aPath);
                    maTree.repaint ();
                }
                */
                maActiveObject.Highlight (true);
            }
            repaint ();
            return true;
        }
        else
            return false;
    }




    /** Called when the selection of the tree changes.  Highlight the
        corresponding graphical representation of the object.
    */
    public void SelectObject (javax.swing.tree.TreeNode aNode)
    {
        CanvasShape aCanvasShape = maShapeList.Get (aNode);
        HighlightObject (aCanvasShape);
    }




    private int
        mnXAnchor,
        mnYAnchor,
        maResizeFlag;
    private double
        mnHOffset,
        mnVOffset,
        mnScale;
    private CanvasShape maActiveObject;
    private javax.swing.JTree maTree;
    // The size of the widget at the last call of setupTransformation()
    private Dimension maLastWidgetSize;
    private ShapeContainer maShapeList;
    private MouseObserver maMouseObserver;
}
