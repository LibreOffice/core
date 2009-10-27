/*************************************************************************
 *
 *  $RCSfile: EditableTextView.java,v $
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
