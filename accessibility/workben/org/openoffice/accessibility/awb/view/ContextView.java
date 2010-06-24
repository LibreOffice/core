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

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JLabel;
import javax.swing.JTextField;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;

import org.openoffice.accessibility.misc.NameProvider;

/** The <type>ContextView</type> class displays information accessible over
    the <type>XAccessibleContext</type> interface.  This includes name,
    description, and role.
*/
public class ContextView
    extends ObjectView
    implements ActionListener
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (xContext != null)
            return new ContextView (aContainer);
        else
            return null;
    }

    public ContextView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);
        maNameLabel = aLayout.AddLabeledString ("Name:");
        maDescriptionLabel = aLayout.AddLabeledString ("Description:");
        maRoleLabel = aLayout.AddLabeledEntry ("Role:");
    }

    public void Update ()
    {
        if (mxContext == null)
        {
            maNameLabel.setText ("<null object>");
            maDescriptionLabel.setText ("<null object>");
            maRoleLabel.setText ("<null object>");
        }
        else
        {
            maNameLabel.setText (mxContext.getAccessibleName());
            maDescriptionLabel.setText (mxContext.getAccessibleDescription());
            maRoleLabel.setText (NameProvider.getRoleName (mxContext.getAccessibleRole()));
        }
    }

    public String GetTitle ()
    {
        return ("Context");
    }

    /** Listen for changes regarding displayed values.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            case AccessibleEventId.NAME_CHANGED :
            case AccessibleEventId.DESCRIPTION_CHANGED :
                Update ();
        }
    }

    public void actionPerformed (ActionEvent aEvent)
    {
    }


    private JLabel
        maNameLabel,
        maDescriptionLabel,
        maRoleLabel;
}
