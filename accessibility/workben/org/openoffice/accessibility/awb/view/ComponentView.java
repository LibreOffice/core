/*************************************************************************
 *
 *  $RCSfile: ComponentView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:32 $
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

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JLabel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.misc.NameProvider;

/** The <type>ContextView</type> class displays information accessible over
    the <type>XAccessibleContext</type> interface.  This includes name,
    description, and role.
*/
public class ComponentView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (UnoRuntime.queryInterface(
            XAccessibleComponent.class, xContext) != null)
            return new ComponentView (aContainer);
        else
            return null;
    }

    public ComponentView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);

        maRelativeLocationLabel = aLayout.AddLabeledEntry ("Relative Location: ");
        maAbsoluteLocationLabel = aLayout.AddLabeledEntry ("Location on Screen: ");
        maSizeLabel = aLayout.AddLabeledEntry ("Size");
        maBoundingBoxLabel = aLayout.AddLabeledEntry ("Bounding Box: ");
        maConsistencyLabel = aLayout.AddLabeledEntry ("Consistent: ");
        maForegroundColorLabel = aLayout.AddLabeledEntry ("Foreground Color: ");
        maBackgroundColorLabel = aLayout.AddLabeledEntry ("Background Color: ");
    }


    public void SetObject (XAccessibleContext xContext)
    {
        mxComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
            XAccessibleComponent.class, xContext);
        super.SetObject (xContext);
    }

    public void Update ()
    {
        if (mxContext == null)
        {
            maRelativeLocationLabel.setText ("<null object>");
            maAbsoluteLocationLabel.setText ("<null object>");
            maSizeLabel.setText ("<null object>");
            maBoundingBoxLabel.setText ("<null object>");
            maConsistencyLabel.setText ("<null object>");
            maForegroundColorLabel.setText ("<null object>");
            maBackgroundColorLabel.setText ("<null object>");
        }
        else
        {
            com.sun.star.awt.Point aLocation = mxComponent.getLocation();
            maRelativeLocationLabel.setText (
                aLocation.X + ", " + aLocation.Y);
            com.sun.star.awt.Point aLocationOnScreen =
                mxComponent.getLocationOnScreen();
            maAbsoluteLocationLabel.setText (
                aLocationOnScreen.X + ", " + aLocationOnScreen.Y);
            com.sun.star.awt.Size aSize = mxComponent.getSize();
            maSizeLabel.setText (
                aSize.Width + ", " + aSize.Height);
            com.sun.star.awt.Rectangle aBBox = mxComponent.getBounds();
            maBoundingBoxLabel.setText (
                aBBox.X + ", " + aBBox.Y + ","
                + aBBox.Width + ", " + aBBox.Height);
            int nColor = mxComponent.getForeground();
            maForegroundColorLabel.setText (
                "R"+ (nColor>>16&0xff)
                + "G" + (nColor>>8&0xff)
                + "B" + (nColor>>0&0xff)
                + "A" + (nColor>>24&0xff));
            nColor = mxComponent.getBackground();
            maBackgroundColorLabel.setText (
                "R"+ (nColor>>16&0xff)
                + "G" + (nColor>>8&0xff)
                + "B" + (nColor>>0&0xff)
                + "A" + (nColor>>24&0xff));

            // Check consistency of coordinates.
            String sConsistency = new String ();
            if (aBBox.X!=aLocation.X || aBBox.Y!=aLocation.Y)
                sConsistency += (sConsistency.length()!=0?", ":"") +
                    "Bounding box conflicts with relative location";
            if (aBBox.Width!=aSize.Width || aBBox.Height!=aSize.Height)
                sConsistency += (sConsistency.length()!=0?", ":"") +
                    "Bounding box conflicts with size";
            XAccessible xParent = mxContext.getAccessibleParent();
            XAccessibleComponent xParentComponent =
                (XAccessibleComponent)UnoRuntime.queryInterface(
                    XAccessibleComponent.class, xParent);
            if (xParentComponent == null)
            {
                if (aLocation.X != aLocationOnScreen.X
                    || aLocation.Y != aLocationOnScreen.Y)
                    sConsistency += (sConsistency.length()!=0?", ":"") +
                        "location on screen does not equal "
                        + "relative location without parent";
            }
            else
            {
                com.sun.star.awt.Point aParentLocationOnScreen =
                    xParentComponent.getLocationOnScreen();
                if (aLocation.X+aParentLocationOnScreen.X
                    != aLocationOnScreen.X
                    || aLocation.Y+aParentLocationOnScreen.Y
                    != aLocationOnScreen.Y)
                    sConsistency += (sConsistency.length()!=0?", ":"") +
                        "location on screen does not match "
                        + "relative location";
            }
            if (sConsistency.length() == 0)
                sConsistency += "yes";
            else
                maConsistencyLabel.setBackground (GetContainer().GetErrorColor());
            maConsistencyLabel.setText (sConsistency);
        }
    }

    public String GetTitle ()
    {
        return ("Component");
    }

    /** Listen for changes regarding displayed values.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            case AccessibleEventId.BOUNDRECT_CHANGED :
            case AccessibleEventId.VISIBLE_DATA_CHANGED :
                Update ();
        }
    }

    private XAccessibleComponent mxComponent;
    private JLabel
        maRelativeLocationLabel,
        maAbsoluteLocationLabel,
        maSizeLabel,
        maBoundingBoxLabel,
        maConsistencyLabel,
        maForegroundColorLabel,
        maBackgroundColorLabel;
}
