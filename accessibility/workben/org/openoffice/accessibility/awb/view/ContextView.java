/*************************************************************************
 *
 *  $RCSfile: ContextView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:32 $
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
