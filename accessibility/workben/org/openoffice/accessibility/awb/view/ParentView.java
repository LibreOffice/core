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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.lang.Integer;
import javax.swing.JLabel;
import javax.swing.JTextField;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.lang.IndexOutOfBoundsException;

import org.openoffice.accessibility.misc.NameProvider;


/** Show informations related to the parent/child relationship.
*/
public class ParentView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (xContext != null)
            return new ParentView (aContainer);
        else
            return null;
    }

    public ParentView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);
        maParentLabel = aLayout.AddLabeledEntry ("Has parent: ");
        maIndexLabel = aLayout.AddLabeledEntry ("Index in parent: ");
        maValidLabel = aLayout.AddLabeledEntry ("Parent/Child relationship valid: ");
        maChildrenLabel = aLayout.AddLabeledEntry ("Child count: ");
    }

    public void Update ()
    {
        if (mxContext == null)
        {
            maParentLabel.setText ("<null object>");
            maIndexLabel.setText ("<null object>");
            maValidLabel.setText ("<null object>");
            maChildrenLabel.setText ("<null object>");
        }
        else
        {
            XAccessible xParent = mxContext.getAccessibleParent();
            int nIndex = mxContext.getAccessibleIndexInParent();
            maIndexLabel.setText (Integer.toString(nIndex));
            if (xParent != null)
            {
                maParentLabel.setText ("yes");
                XAccessibleContext xParentContext =
                    xParent.getAccessibleContext();
                if (xParentContext != null)
                {
                    try
                    {
                        XAccessible xChild =
                            xParentContext.getAccessibleChild(nIndex);
                        if (xChild != mxContext)
                            maValidLabel.setText ("yes");
                        else
                        {
                            maValidLabel.setText ("no");
                            maValidLabel.setBackground (GetContainer().GetErrorColor());
                        }
                    }
                    catch (IndexOutOfBoundsException e)
                    {
                        maValidLabel.setText ("no: invalid index in parent");
                        maValidLabel.setBackground (GetContainer().GetErrorColor());
                    }
                }
                else
                {
                    maValidLabel.setText ("no: parent has no context");
                    maValidLabel.setBackground (GetContainer().GetErrorColor());
                }
            }
            else
                maParentLabel.setText ("no");
            maChildrenLabel.setText (Integer.toString(mxContext.getAccessibleChildCount()));
        }
    }

    public String GetTitle ()
    {
        return ("Parent");
    }


    /** Listen for changes regarding displayed values.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            default:
                Update ();
        }
    }


    private JLabel
        maParentLabel,
        maIndexLabel,
        maValidLabel,
        maChildrenLabel;
}
