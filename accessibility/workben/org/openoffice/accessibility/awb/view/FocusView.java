/*************************************************************************
 *
 *  $RCSfile: FocusView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:34 $
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

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JLabel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.uno.UnoRuntime;

public class FocusView
    extends ObjectView
    implements ActionListener
{
    /** Create a FocusView when the given object supports the
        XAccessibleComponent interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleComponent xComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, xContext);
        if (xComponent != null)
            return new FocusView (aContainer);
        else
            return null;
    }

    public FocusView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        setLayout (new GridBagLayout());
        GridBagConstraints aConstraints = new GridBagConstraints ();

        maFocused = new JLabel ();
        maFocused.setFont (GetContainer().GetViewFont());
        aConstraints.gridy = 0;
        aConstraints.weightx = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (maFocused, aConstraints);

        maGrabFocus = new JButton ("grabFocus");
        maGrabFocus.setFont (GetContainer().GetViewFont());
        aConstraints.gridy = 1;
        aConstraints.fill = GridBagConstraints.NONE;
        aConstraints.anchor = GridBagConstraints.WEST;
        add (maGrabFocus, aConstraints);

        maGrabFocus.addActionListener (this);
    }

    /** Additionally to the context store a reference to the
        XAccessibleComponent interface.
    */
    public void SetObject (XAccessibleContext xObject)
    {
        mxComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, xObject);
        super.SetObject (xObject);
    }

    synchronized public void Destroy ()
    {
        super.Destroy();
        maGrabFocus.removeActionListener (this);
    }

    synchronized public void Update ()
    {
        if (mxContext == null)
        {
            maFocused.setText ("<null object>");
            maGrabFocus.setEnabled (false);
        }
        else
        {
            XAccessibleStateSet aStateSet = mxContext.getAccessibleStateSet();
            if (aStateSet.contains(AccessibleStateType.FOCUSED))
                maFocused.setText ("focused");
            else
                maFocused.setText ("not focused");
            if (maGrabFocus != null)
                maGrabFocus.setEnabled (true);
        }
    }

    public String GetTitle ()
    {
        return ("Focus");
    }

    synchronized public void actionPerformed (ActionEvent aEvent)
    {
        if (aEvent.getActionCommand().equals("grabFocus"))
        {
            mxComponent.grabFocus();
        }
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        System.out.println (aEvent);
        if (aEvent.EventId == AccessibleEventId.STATE_CHANGED)
            Update ();
    }

    private JLabel maFocused;
    private JButton maGrabFocus;
    private XAccessibleComponent mxComponent;
}
