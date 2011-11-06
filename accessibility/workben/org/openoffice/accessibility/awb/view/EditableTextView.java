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
