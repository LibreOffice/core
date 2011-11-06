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
