/*************************************************************************
 *
 *  $RCSfile: CanvasShape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2003/09/19 09:21:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.accessibility.awb.canvas;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;


import com.sun.star.accessibility.*;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;


class CanvasShape implements XAccessibleEventListener
{
    public final Color maHighlightColor = Color.red;
    public final Color maSelectionColor = Color.green;
    public final Color maFocusColor = Color.blue;

    public CanvasShape (javax.swing.tree.TreeNode aNode, Canvas aCanvas)
    {
        maNode = aNode;
        msName = "<no name>";
        msDescription = "<no description>";
        maShape = new Rectangle2D.Double (-10,-10,10,10);
        maPosition = new Point (-10,-10);
        maSize = new Dimension (10,10);
        maFgColor = java.awt.Color.black;
        maBgColor = Color.blue;
        mnRole = -1;
        mbHighlighted = false;
        mbSelected = false;
        mbFocused = false;
        maCanvas = aCanvas;

        Update ();
    }




    public javax.swing.tree.TreePath getNodePath (javax.swing.tree.TreeNode node)
    {
        javax.swing.tree.TreeNode parent = node.getParent();
        return (parent != null) ?
            getNodePath(parent).pathByAddingChild(node) :
            new javax.swing.tree.TreePath(node);
    }

    public javax.swing.tree.TreePath getNodePath ()
    {
        return getNodePath(maNode);
    }



    /** Update the data obtained from the <type>AccessibilityNode</type>
        object.
    */
    public void Update ()
    {
        if (maNode instanceof XAccessible) {
            mxContext = ((XAccessible) maNode).getAccessibleContext();
            mxComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, mxContext);
        }

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

        UpdateGeometry ();

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



    public void UpdateGeometry ()
    {
        if (mxComponent != null)
        {
            com.sun.star.awt.Point aLocationOnScreen =
                mxComponent.getLocationOnScreen();
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
    public void paint (
        Graphics2D g,
        boolean bShowDescription,
        boolean bShowName,
        boolean bShowText)
    {
        try{
            // Transform the object's position and size according to the
            // specified offset and scale.
            Point aLocation = new Point();
            maShape = new Rectangle2D.Double (
                maPosition.x,
                maPosition.y,
                maSize.width,
                maSize.height);
            maTransformation = g.getTransform();

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


    public void paint_highlight (Graphics2D g)
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
        xText = (XAccessibleText)UnoRuntime.queryInterface(
            XAccessibleText.class, mxContext);

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

                    g.drawString (sText.substring(i, i+1), (float)x, (float)y);
                }
            }
            catch (com.sun.star.lang.IndexOutOfBoundsException e)
            {}
        }
    }


    /** Compute whether the specified point lies inside the object's
        bounding box.
    */
    public boolean Contains (int x, int y)
    {
        Point2D aPosition = new Point2D.Double (x,y);
        try
        {
            maTransformation.inverseTransform (aPosition, aPosition);
            //            System.out.println ("transformed "+x+","+y+" to "+aPosition);
        }
        catch (NoninvertibleTransformException aException)
        {
            return false;
        }
        return (maShape.contains (aPosition));
    }

    public void Highlight (boolean bFlag)
    {
        mbHighlighted = bFlag;
    }

    public boolean IsHighlighted ()
    {
        return mbHighlighted;
    }

    public Rectangle GetBBox ()
    {
        return new Rectangle (maPosition, maSize);
    }

    public Point getOrigin ()
    {
        return maPosition;
    }

    public Dimension GetSize ()
    {
        return maSize;
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

    /** */
    public void notifyEvent(com.sun.star.accessibility.AccessibleEventObject aEvent) {
        try {
            switch (aEvent.EventId) {
                case AccessibleEventId.BOUNDRECT_CHANGED:
                case AccessibleEventId.VISIBLE_DATA_CHANGED:
                    UpdateGeometry ();
                    maCanvas.repaint();
                    break;
                default:
                    break;
            }
        } catch (Exception aException) {
            System.err.println ("caught exception while updating a shape:"
                + aException);
            aException.printStackTrace (System.err);
        }
    }

    /** Callback for disposing events.
    */
    public void disposing (com.sun.star.lang.EventObject e)
    {
        System.out.println ("Disposing");
    }




    private Canvas
        maCanvas;
    private javax.swing.tree.TreeNode
        maNode;
    private XAccessibleContext
        mxContext;
    private XAccessibleComponent
        mxComponent;
    private String
        msDescription,
        msName;
    private Rectangle2D.Double maShape;
    private AffineTransform maTransformation;
    private Point maPosition;
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
