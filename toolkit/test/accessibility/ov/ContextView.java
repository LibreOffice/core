/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package ov;

import java.awt.Color;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JLabel;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;

import tools.NameProvider;

public class ContextView
    extends ListeningObjectView
    implements ActionListener
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        System.out.println ("ContextView.CreateView");
        if (xContext != null)
            return new ContextView (aContainer);
        else
            return null;
    }

    public ContextView (ObjectViewContainer aContainer)
    {
        super (aContainer);
        maNameLabel = new JLabel ("Name: ");
        maName = new JLabel ("");
        maDescriptionLabel = new JLabel ("Description: ");
        maDescription = new JLabel ("");
        maRoleLabel = new JLabel ("Role: ");
        maRole = new JLabel ("");

        // Make the background of name and description white and opaque so
        // that leading and trailing spaces become visible.
        maName.setOpaque (true);
        maName.setBackground (Color.WHITE);
        maDescription.setOpaque (true);
        maDescription.setBackground (Color.WHITE);
        maRole.setOpaque (true);
        maRole.setBackground (Color.WHITE);

        GridBagLayout aLayout = new GridBagLayout();
        setLayout (aLayout);
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = 0;
        constraints.weighty = 1;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.fill = GridBagConstraints.NONE;
        add (maNameLabel, constraints);
        constraints.gridy = 1;
        add (maDescriptionLabel, constraints);
        constraints.gridy = 2;
        add (maRoleLabel, constraints);
        constraints.gridy = 0;
        constraints.gridx = 1;
        constraints.weightx = 2;
        add (maName, constraints);
        constraints.gridy = 1;
        add (maDescription, constraints);
        constraints.gridy = 2;
        add (maRole, constraints);
    }

    public void Update ()
    {
        if (mxContext == null)
        {
            maName.setText ("<null object>");
            maDescription.setText ("<null object>");
            maRole.setText ("<null object>");
        }
        else
        {
            maName.setText (mxContext.getAccessibleName());
            maDescription.setText (mxContext.getAccessibleDescription());
            maRole.setText (NameProvider.getRoleName (mxContext.getAccessibleRole()));
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
        maName,
        maDescriptionLabel,
        maDescription,
        maRoleLabel,
        maRole;
}
