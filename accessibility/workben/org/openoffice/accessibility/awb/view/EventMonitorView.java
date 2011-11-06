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
