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

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JButton;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.awb.view.text.TextDialogFactory;


public class EditableTextView
    extends ObjectView
    implements ActionListener
{
    /** Create a EditableTextView when the given object supports the
        XAccessibleEditableText interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleEditableText xEditableText =
            (XAccessibleEditableText)UnoRuntime.queryInterface(
                XAccessibleEditableText.class, xContext);
        if (xEditableText != null)
            return new EditableTextView (aContainer);
        else
            return null;
    }

    public EditableTextView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        JButton aButton = new JButton ("cut...");
        aButton.setFont (ViewGridLayout.GetFont());
        aButton.addActionListener (this);
        add (aButton);
        aButton = new JButton ("paste...");
        aButton.setFont (ViewGridLayout.GetFont());
        aButton.addActionListener (this);
        add (aButton);
        aButton = new JButton ("edit...");
        aButton.setFont (ViewGridLayout.GetFont());
        aButton.addActionListener (this);
        add (aButton);
        aButton = new JButton ("format...");
        aButton.setFont (ViewGridLayout.GetFont());
        aButton.addActionListener (this);
        add (aButton);
    }


    /** Additionally to the context store a reference to the
        XAccessibleEditableText interface.
    */
    public void SetObject (XAccessibleContext xObject)
    {
        mxEditableText = (XAccessibleEditableText)UnoRuntime.queryInterface(
            XAccessibleEditableText.class, xObject);
        super.SetObject (xObject);
    }

    public String GetTitle ()
    {
        return ("Editable Text");
    }

    synchronized public void Destroy ()
    {
        mxEditableText = null;
        super.Destroy();
    }

    public void actionPerformed (ActionEvent aEvent)
    {
        String sCommand = aEvent.getActionCommand();
        if (sCommand.equals ("cut..."))
            TextDialogFactory.CreateCutDialog (mxContext);
        else if (sCommand.equals ("past..."))
            TextDialogFactory.CreatePasteDialog (mxContext);
        else if (sCommand.equals ("edit..."))
            TextDialogFactory.CreateEditDialog (mxContext);
        else if (sCommand.equals ("format..."))
            TextDialogFactory.CreateFormatDialog (mxContext);
    }

    private XAccessibleEditableText mxEditableText;
}
