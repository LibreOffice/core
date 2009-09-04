/*************************************************************************
 *
 *  $RCSfile: LayoutManager.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:34 $
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
