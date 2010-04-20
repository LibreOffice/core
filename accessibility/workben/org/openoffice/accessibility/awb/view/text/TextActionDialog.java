/*************************************************************************
 *
 *  $RCSfile: TextActionDialog.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:42 $
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

package org.openoffice.accessibility.awb.view.text;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.text.JTextComponent;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.uno.UnoRuntime;


/**
 * Display a dialog with a text field and a pair of cancel/do-it buttons
 */
class TextActionDialog
    extends JDialog
    implements ActionListener
{
    public TextActionDialog (
        XAccessibleContext xContext,
        String sExplanation,
        String sTitle)
    {
        super();// AccessibilityWorkBench.Instance() );

        mxContext = xContext;
        msTitle = sTitle;
        msExplanation = sExplanation;
        Layout  ();
        setSize (350, 225);

    }


    /** build dialog */
    protected void Layout()
    {
        setTitle (msTitle);

        // vertical stacking of the elements
        Container aContent = getContentPane();
        //        aContent.setLayout( new BorderLayout() );

        // Label with explanation.
        if (msExplanation.length() > 0)
            aContent.add (new JLabel (msExplanation), BorderLayout.NORTH);

        // the text field
        maText = new JTextArea();
        maText.setLineWrap (true);
        maText.setEditable (false);
        aContent.add (maText, BorderLayout.CENTER);

        XAccessibleText xText = (XAccessibleText)UnoRuntime.queryInterface(
            XAccessibleText.class, mxContext);
        String sText = xText.getText();
        maText.setText (sText);
        maText.setRows (sText.length() / 40 + 1);
        maText.setColumns (Math.min (Math.max (40, sText.length()), 20));

        JPanel aButtons = new JPanel();
        aButtons.setLayout (new FlowLayout());
        maIndexToggle = new JCheckBox ("reverse selection");
        aButtons.add (maIndexToggle);

        JButton aActionButton = new JButton (msTitle);
        aActionButton.setActionCommand ("Action");
        aActionButton.addActionListener (this);
        aButtons.add (aActionButton);

        JButton aCancelButton = new JButton ("cancel");
        aCancelButton.setActionCommand ("Cancel");
        aCancelButton.addActionListener (this);
        aButtons.add (aCancelButton);

        // add Panel with buttons
        aContent.add (aButtons, BorderLayout.SOUTH);
    }

    protected void Cancel()
    {
        hide();
        dispose();
    }

    public void actionPerformed(ActionEvent e)
    {
        String sCommand = e.getActionCommand();

        if( "Cancel".equals( sCommand ) )
            Cancel();
        else if( "Action".equals( sCommand ) )
            Action();
    }


    protected int GetSelectionStart()
    {
        return GetSelection(true);
    }
    protected int GetSelectionEnd()
    {
        return GetSelection(false);
    }
    private int GetSelection (boolean bStart)
    {
        if (bStart ^ maIndexToggle.isSelected())
            return maText.getSelectionStart();
        else
            return maText.getSelectionEnd();
    }



    protected void Action ()
    {
        String sError = null;
        boolean bSuccess = true;
        try
        {
            XAccessibleText xText =
                (XAccessibleText)UnoRuntime.queryInterface(
                    XAccessibleText.class, mxContext);
            if (xText != null)
                bSuccess = bSuccess && TextAction (xText);

            XAccessibleEditableText xEditableText =
                (XAccessibleEditableText)UnoRuntime.queryInterface(
                    XAccessibleEditableText.class, mxContext);
            if (xEditableText != null)
                bSuccess = bSuccess && EditableTextAction (xEditableText);

            if ( ! bSuccess)
                sError = "Can't execute";
        }
        catch (IndexOutOfBoundsException e)
        {
            sError = "Index out of bounds";
        }

        if (sError != null)
            JOptionPane.showMessageDialog (
                this,// AccessibilityWorkBench.Instance(),
                sError,
                msTitle,
                JOptionPane.ERROR_MESSAGE);

        Cancel();
    }

    /** override this for dialog-specific action */
    boolean TextAction (XAccessibleText xText)
        throws IndexOutOfBoundsException
    {
        return true;
    }

    boolean EditableTextAction (XAccessibleEditableText xText)
        throws IndexOutOfBoundsException
    {
        return true;
    }

    private XAccessibleContext mxContext;
    protected JTextArea maText;
    private String msTitle;
    private String msExplanation;
    private JCheckBox maIndexToggle;
}
