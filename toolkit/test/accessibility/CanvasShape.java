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

import java.awt.*;
import javax.swing.tree.*;
import java.awt.geom.Rectangle2D;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.accessibility.AccessibleStateType;

class CanvasShape
{
    public final Color maHighlightColor = Color.red;
    public final Color maSelectionColor = Color.green;
    public final Color maFocusColor = Color.blue;

    //    public AccessibleObject (XAccessibleContext xContext, TreePath aPath)
    public CanvasShape (AccTreeNode aNode)
    {
        maNode = aNode;
        mxContext = aNode.getContext();
        msName = "name unknown";
        msDescription = "description unknown";
        maShape = new Rectangle2D.Double (-10,-10,10,10);
        maPosition = new Point (-10,-10);
        maSize = new Dimension (10,10);
        maFgColor = java.awt.Color.black;
        maBgColor = Color.blue;
        mnRole = -1;
        mbHighlighted = false;
        mbSelected = false;
        mbFocused = false;
        mxComponent = aNode.getComponent();

        update ();
    }



    /** Update the data obtained from the xAccessible.
    */
    public void update ()
    {
        if (mxContext != null)
        {
            msName = mxContext.getAccessibleName();
            msDescription = mxContext.getAccessibleDescription();
            mnRole = mxContext.getAccessibleRole();

            // Extract the selected and focused flag.
            XAccessibleStateSet xStateSet = mxContext.getAccessibleStateSet ();
            if (xStateSet != null)
            {
                mbSelected = xStateSet.contains (AccessibleStateType.SELECTED);
                mbFocused = xStateSet.contains (AccessibleStateType.FOCUSED);
            }
        }

        updateGeometry ();
        if (mxComponent != null)
        {
            // Note: alpha values in office 0..255 have to be mapped to
            //       255..0 in Java
            Color aCol = new Color (mxComponent.getForeground(), true);
            maFgColor = new Color (aCol.getRed (),
                                   aCol.getGreen (),
                                   aCol.getBlue (),
                                   0xff - aCol.getAlpha ());
            aCol = new Color (mxComponent.getBackground(), true);
            maBgColor = new Color (aCol.getRed (),
                                   aCol.getGreen (),
                                   aCol.getBlue (),
                                   0xff - aCol.getAlpha ());
        }
    }

    public void updateGeometry ()
    {
        if (mxComponent != null)
        {
            com.sun.star.awt.Point aLocationOnScreen = mxComponent.getLocationOnScreen();
            com.sun.star.awt.Size aSizeOnScreen = mxComponent.getSize();
            maPosition = new Point (
                aLocationOnScreen.X,
                aLocationOnScreen.Y);
            maSize = new Dimension (
                aSizeOnScreen.Width,
                aSizeOnScreen.Height);
        }
    }


    /** Paint the object into the specified canvas.  It is transformed
        according to the specified offset and scale.
    */
    public void paint (Graphics2D g,
        double nXOffset, double nYOffset, double nScaleFactor,
        boolean bShowDescription, boolean bShowName, boolean bShowText)
    {
        try{
            // Transform the object's position and size according to the
            // specified offset and scale.
            Point aLocation = new Point();
            maShape = new Rectangle2D.Double (
                maPosition.x * nScaleFactor + nXOffset,
                maPosition.y * nScaleFactor + nYOffset,
                maSize.width * nScaleFactor,
                maSize.height * nScaleFactor);

            // Fill the object's bounding box with its background color if it
            // has no children.
            if (mxContext.getAccessibleChildCount() == 0)
            {
                g.setColor (maBgColor);
                g.fill (maShape);
            }

            // Remove alpha channel from color before drawing the frame.
            Color color = maFgColor;
            if (maFgColor.getAlpha()<128)
                color = new Color (maFgColor.getRed(), maFgColor.getGreen(), maFgColor.getBlue());
            g.setColor (color);
            g.draw (maShape);

            if (mbFocused)
            {
                g.setColor (maFocusColor);
                for (int x=0; x<=2; x++)
                    for (int y=0; y<=2; y++)
                        g.fill (
                            new Rectangle2D.Double (
                                maShape.x + x/2.0 * maShape.width-3,
                                maShape.y + y/2.0 * maShape.height-3,
                                6,
                                6));
            }
            if (mbSelected)
            {
                g.setColor (maSelectionColor);
                for (int x=0; x<=2; x++)
                    for (int y=0; y<=2; y++)
                        g.draw (
                            new Rectangle2D.Double (
                                maShape.x + x/2.0 * maShape.width-2,
                                maShape.y + y/2.0 * maShape.height-2,
                                4,
                                4));
            }

            // Write the object's text OR name and description.
            g.setColor (maFgColor);
            if (bShowName)
                paintName (g);
            if (bShowDescription)
                paintDescription (g);
            if (bShowText)
                paintText (g);
        }
        catch (Exception e)
        { // don't care
        }
    }

    public void paint_highlight (Graphics2D g,
        double nXOffset, double nYOffset, double nScaleFactor)
    {
        if (mbHighlighted)
            g.setColor (maHighlightColor);
        else
            g.setColor (maFgColor);
        g.draw (maShape);
    }




    private void paintName (Graphics2D g)
    {
        g.drawString ("Name: " + msName,
            (float)maShape.x+5,
            (float)maShape.y+15);
    }



    private void paintDescription (Graphics2D g)
    {
        g.drawString ("Description: " + msDescription,
            (float)maShape.x+5,
            (float)maShape.y+35);
    }




    private void paintText (Graphics2D g)
    {
        XAccessibleText xText = null;
        // get XAccessibleText
        xText = maNode.getText();

        // Draw every character in the text string.
        if (xText != null)
        {
            String sText = xText.getText();
            try
            {
                for(int i = 0; i < sText.length(); i++)
                {
                    com.sun.star.awt.Rectangle aRect =
                        xText.getCharacterBounds(i);

                    double x = maShape.x + aRect.X;
                    double y = maShape.y + aRect.Y + aRect.Height;

                    g.drawString(sText.substring(i, i+1), (float)x, (float)y);
                }
            }
            catch (com.sun.star.lang.IndexOutOfBoundsException e)
            {}
        }
    }




    /** Callback for disposing events.
    */
    public void disposing (com.sun.star.lang.EventObject e)
    {
        System.out.println ("Disposing");
    }




    /** Compute whether the specified point lies inside the object's
        bounding box.
    */
    public boolean contains (int x, int y)
    {
        return (maShape.contains (x,y));
    }

    public void highlight ()
    {
        mbHighlighted = true;
    }

    public void unhighlight ()
    {
        mbHighlighted = false;
    }

    public boolean isHighlighted ()
    {
        return mbHighlighted;
    }

    public Rectangle getBBox ()
    {
        return new Rectangle (maPosition, maSize);
    }

    public Point getOrigin ()
    {
        return maPosition;
    }

    public TreePath getPath ()
    {
        return new TreePath (maNode.createPath());
    }

    public int getRole ()
    {
        return mnRole;
    }

    public XAccessibleContext getContext ()
    {
        return mxContext;
    }

    public XAccessibleComponent getComponent ()
    {
        return mxComponent;
    }

    public String toString ()
    {
        return ">"+msName+", "+msDescription+" +"+maPosition.x+"+"+maPosition.y
            +"x"+maSize.width+"x"+maSize.height+"<";
    }

    private AccTreeNode
        maNode;
    private XAccessibleContext
        mxContext;
    private XAccessibleComponent
        mxComponent;
    private String
        msDescription,
        msName;
    private Rectangle2D.Double
        maShape;
    private Point
        maPosition;
    private Dimension
        maTransformedSize,
        maSize;
    private Color
        maFgColor,
        maBgColor;
    private boolean
        // Highlighting objects is an internal concept.  Corresponds to selection in the tree view.
        mbHighlighted,
        // Set when the accessible object is selected.
        mbSelected,
        // Set when the accessible object is focused.
        mbFocused;
    private int
        mnRole;
}
