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
