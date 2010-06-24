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

import java.awt.BorderLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;

import org.openoffice.accessibility.misc.NameProvider;


/** A simple event monitor that shows all events sent to one accessible
    object.
*/
class EventMonitorView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (xContext != null)
            return new EventMonitorView (aContainer);
        else
            return null;
    }

    public EventMonitorView (ObjectViewContainer aContainer)
    {
        super (aContainer);
        mnLineNo = 0;
        Layout();
    }

    public String GetTitle ()
    {
        return "Event Monitor";
    }

    /** Create and arrange the widgets for this view.
    */
    private void Layout ()
    {
        setLayout (new GridBagLayout ());

        maText = new JTextArea();
        maText.setBackground (new Color (255,250,240));
        maText.setFont (new Font ("Helvetica", Font.PLAIN, 9));

        maScrollPane = new JScrollPane (maText,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        maScrollPane.setPreferredSize (new Dimension (300,200));

        GridBagConstraints aConstraints = new GridBagConstraints ();
        aConstraints.weightx = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (maScrollPane, aConstraints);
    }


    public void Update ()
    {
    }


    private void UpdateVerticalScrollBar ()
    {
        JScrollBar sb = maScrollPane.getVerticalScrollBar();
        if (sb != null)
        {
            int nScrollBarValue = sb.getMaximum() - sb.getVisibleAmount() - 1;
            sb.setValue (nScrollBarValue);
        }
    }


    public void notifyEvent (AccessibleEventObject aEvent)
    {
        maText.append ((mnLineNo++) + ". " + NameProvider.getEventName (aEvent.EventId) + " : "
            + aEvent.OldValue.toString()
            + " -> "
            + aEvent.NewValue.toString() + "\n");
        UpdateVerticalScrollBar();
    }

    private JTextArea maText;
    private int mnLineNo;
    private JScrollPane maScrollPane;
}
