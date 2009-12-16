/*************************************************************************
 *
 *  $RCSfile: EventMonitorView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:33 $
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
        maScrollPane.setPreferredSize (new Dimension (300,80));

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
        maText.append (NameProvider.getEventName (aEvent.EventId) + " : "
            + aEvent.OldValue.toString()
            + " -> "
            + aEvent.NewValue.toString() + "\n");
        UpdateVerticalScrollBar();
    }

    private JTextArea maText;
    private JScrollPane maScrollPane;
}
