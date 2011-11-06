/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
