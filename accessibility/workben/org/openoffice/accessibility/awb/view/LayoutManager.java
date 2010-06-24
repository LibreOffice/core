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

package org.openoffice.accessibility.awb.view;

import java.awt.Component;
import java.awt.Cursor;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Point;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseEvent;
import javax.swing.JComponent;

class LayoutManager
    implements MouseListener,
               MouseMotionListener
{
    public LayoutManager (JComponent aLayoutedComponent)
    {
        maLayoutedComponent = aLayoutedComponent;
        maDraggedView = null;
        mbInsertionPending = false;
    }

    public void mouseClicked (MouseEvent aEvent)
    {
        System.out.println (aEvent);
    }
    public void mousePressed (MouseEvent aEvent)
    {
        mnOldY = aEvent.getPoint().y;
    }
    public void mouseReleased (MouseEvent aEvent)
    {
        if (mbInsertionPending)
        {
            InsertView (maDraggedView, aEvent.getPoint().y);
            mbInsertionPending = false;
            maDraggedView = null;
        }
    }
    public void mouseEntered (MouseEvent aEvent)
    {
    }
    public void mouseExited (MouseEvent aEvent)
    {
        if (mbInsertionPending)
        {
            InsertView (maDraggedView, mnOldY);
            mbInsertionPending = false;
            maDraggedView = null;
        }
    }
    public void mouseDragged (MouseEvent aEvent)
    {
        int dy = mnOldY - aEvent.getPoint().y;
        GridBagLayout aLayout = (GridBagLayout)maLayoutedComponent.getLayout();
        if ( ! mbInsertionPending && dy != 0)
        {
            maDraggedView = RemoveView (mnOldY);
            if (maDraggedView != null)
                mbInsertionPending = true;
        }
    }
    public void mouseMoved (MouseEvent aEvent)
    {
    }




    private ObjectView RemoveView (int y)
    {
        ObjectView aView = null;
        GridBagLayout aLayout = (GridBagLayout)maLayoutedComponent.getLayout();

        Point aGridLocation = aLayout.location (10,y);
        Component[] aComponentList = maLayoutedComponent.getComponents();
        System.out.println ("removing view at " + aGridLocation);
        for (int i=0; i<aComponentList.length && aView==null; i++)
        {
            GridBagConstraints aConstraints = aLayout.getConstraints (
                aComponentList[i]);
            if (aConstraints.gridy == aGridLocation.y)
                aView = (ObjectView)aComponentList[i];
        }
        maNormalCursor = maLayoutedComponent.getCursor();
        if (aView != null)
        {
            System.out.println ("removing view at " + aGridLocation.y);
            maLayoutedComponent.setCursor (new Cursor (Cursor.MOVE_CURSOR));
            maLayoutedComponent.remove (aView);
            maLayoutedComponent.validate();
            maLayoutedComponent.repaint();
        }

        return aView;
    }

    private void InsertView (ObjectView aView, int y)
    {
        if (aView != null)
        {
            GridBagLayout aLayout = (GridBagLayout)maLayoutedComponent.getLayout();
            Point aGridLocation = aLayout.location (0,y);
            Component[] aComponentList = maLayoutedComponent.getComponents();
            System.out.println ("new position is " + aGridLocation.y);
            for (int i=0; i<aComponentList.length; i++)
            {
                GridBagConstraints aConstraints = aLayout.getConstraints (
                    aComponentList[i]);
                if (aConstraints.gridy >= aGridLocation.y)
                {
                    if (aConstraints.gridy == aGridLocation.y)
                        maLayoutedComponent.add (maDraggedView, aConstraints);
                    aConstraints.gridy += 1;
                    aLayout.setConstraints (aComponentList[i], aConstraints);
                }
            }
            maLayoutedComponent.validate();
            maLayoutedComponent.repaint();
        }
        maLayoutedComponent.setCursor (maNormalCursor);
    }




    private JComponent maLayoutedComponent;
    private ObjectView maDraggedView;
    private int mnOldY;
    private boolean mbInsertionPending;
    private Cursor maNormalCursor;
}
