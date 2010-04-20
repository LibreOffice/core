/*************************************************************************
 *
 *  $RCSfile: StateSetView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:37 $
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

package org.openoffice.accessibility.awb.view;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;



import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;

import org.openoffice.accessibility.misc.NameProvider;

public class StateSetView
    extends ObjectView
{
    /** Create a FocusView when the given object supports the
        XAccessibleComponent interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        ObjectView aView = null;
        if (xContext != null)
            aView = new StateSetView (aContainer);

        return aView;
    }

    public StateSetView (ObjectViewContainer aContainer)
    {
        super (aContainer);
        setPreferredSize (new Dimension(300,110));
        setMinimumSize (new Dimension(200,80));
    }

    public String GetTitle ()
    {
        return ("StateSet");
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        if (aEvent.EventId == AccessibleEventId.STATE_CHANGED)
            Update();
    }


    public void Update ()
    {
        repaint ();
    }

    public void paintChildren (Graphics g)
    {
        if (g != null)
            synchronized (g)
            {
                super.paintChildren (g);

                // Calculcate the are inside the border.
                Insets aInsets = getInsets ();
                Dimension aSize = getSize();
                Rectangle aWidgetArea = new Rectangle (
                    aInsets.left,
                    aInsets.top,
                    aSize.width-aInsets.left-aInsets.right,
                    aSize.height-aInsets.top-aInsets.bottom);

                PaintAllStates ((Graphics2D)g, aWidgetArea);
            }
    }

    private void PaintAllStates (Graphics2D g, Rectangle aWidgetArea)
    {
        Color aTextColor = g.getColor();

        g.setRenderingHint (
            RenderingHints.KEY_ANTIALIASING,
            RenderingHints.VALUE_ANTIALIAS_ON);

        XAccessibleStateSet xStateSet = ( mxContext != null ) ? mxContext.getAccessibleStateSet() : null;
        if (xStateSet != null)
        {
            short aStates[] = xStateSet.getStates ();
            final int nMaxStateIndex = AccessibleStateType.VISIBLE;//MANAGES_DESCENDANTS;
            int nStateWidth = (aWidgetArea.width-12) / (nMaxStateIndex+1);
            AffineTransform aTransform = g.getTransform ();
            g.setColor (aTextColor);
            int y = aWidgetArea.y+aWidgetArea.height - 25;
            double nTextRotation = -0.9;//-java.lang.Math.PI/2;
            double nScale = 0.6;

            // Create a shape for the boxes.
            int nBoxWidth = 8;
            Rectangle aCheckBox = new Rectangle (-nBoxWidth/2,0,nBoxWidth,nBoxWidth);

            // For each state draw a box, fill it appropriately, and draw
            // thre states name.
            for (short i=0; i<=nMaxStateIndex; i++)
            {
                int x = nStateWidth + i * nStateWidth;
                String sStateName = NameProvider.getStateName (i);
                if (sStateName == null)
                    sStateName = new String ("<unknown state " + i + ">");
                boolean bStateSet = xStateSet.contains (i);
                g.setTransform (aTransform);
                g.translate (x,y);
                if (bStateSet)
                {
                    switch (i)
                    {
                        case AccessibleStateType.INVALID:
                        case AccessibleStateType.DEFUNC:
                            g.setColor (saInvalidColor);
                            break;
                        case AccessibleStateType.FOCUSED:
                            g.setColor (saFocusColor);
                            break;
                        case AccessibleStateType.SELECTED:
                            g.setColor (saSelectionColor);
                            break;
                        case AccessibleStateType.EDITABLE:
                            g.setColor (saEditColor);
                            break;
                        default:
                            g.setColor (saDefaultColor);
                            break;
                    }
                    g.fill (aCheckBox);
                    g.setColor (aTextColor);
                }
                g.draw (aCheckBox);
                g.rotate (nTextRotation);
                g.scale (nScale, nScale);
                g.translate (2,-2);
                g.drawString (sStateName, 0,0);
            }

            // Draw string of set states.
            String sStates = new String ();
            for (int i=0; i<aStates.length; i++)
            {
                if (i > 0)
                    sStates = sStates + ", ";
                sStates = sStates + NameProvider.getStateName(aStates[i]);
            }
            g.setTransform (aTransform);
            g.translate (10,aWidgetArea.y+aWidgetArea.height-3);
            g.scale (0.9,0.9);
            g.drawString (sStates,0,0);
        }
    }

    static private Color
        saInvalidColor = new Color (255,0,255),
        saFocusColor = new Color (100,100,255),
        saSelectionColor = Color.GREEN,
        saDefaultColor = new Color (90,90,90),
        saEditColor = new Color (240,240,0);
}


