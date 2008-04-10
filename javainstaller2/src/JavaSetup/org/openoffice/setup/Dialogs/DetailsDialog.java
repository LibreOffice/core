/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DetailsDialog.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.setup.Dialogs;

import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupFrame;
import org.openoffice.setup.Util.DialogFocusTraversalPolicy;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JViewport;
import javax.swing.ScrollPaneConstants;
import javax.swing.border.EmptyBorder;

public class DetailsDialog extends JDialog implements ActionListener {

    private JButton okButton;
    private String helpFileName;
    private String helpFileString;

    public DetailsDialog(SetupFrame setupFrame) {

        super(setupFrame.getDialog());

        String dialogTitle = ResourceManager.getString("String_InstallationCompleted_Button");
        String dialogText = setupFrame.getCurrentPanel().getDialogText();

        setTitle(dialogTitle);
        this.getContentPane().setLayout(new java.awt.BorderLayout());

        JPanel toppanel = new JPanel();
        toppanel.setLayout(new java.awt.BorderLayout());
        toppanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        JPanel buttonpanel = new JPanel();
        buttonpanel.setLayout(new java.awt.FlowLayout());
        buttonpanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        //Create an editor pane.
        JEditorPane editorPane = createEditorPane(dialogText);
        editorPane.setCaretPosition(0);
        JScrollPane editorScrollPane = new JScrollPane(editorPane,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        JViewport port = editorScrollPane.getViewport();
        port.getVisibleRect().setLocation(0,0);
        editorScrollPane.setViewport(port);

        // String helpTitle1 = ResourceManager.getString("String_Details_Title_1");
        // PanelLabel label1 = new PanelLabel(helpTitle1, true);
        // String helpTitle2 = ResourceManager.getString("String_Details_Title_2");
        // PanelLabel label2 = new PanelLabel(helpTitle2);

        String okString = ResourceManager.getString("String_OK");
        okButton = new JButton(okString);
        okButton.setEnabled(true);
        okButton.addActionListener(this);

        JSeparator separator = new JSeparator();

        // toppanel.add(label1, BorderLayout.NORTH);
        // toppanel.add(label2, BorderLayout.CENTER);
        buttonpanel.add(okButton);

        this.getContentPane().add(toppanel, BorderLayout.NORTH);
        this.getContentPane().add(editorScrollPane, BorderLayout.CENTER);
        this.getContentPane().add(buttonpanel, BorderLayout.SOUTH);

        // JScrollBar ScrollBar = editorScrollPane.getVerticalScrollBar();
        // if ( ScrollBar.isShowing() ) {
        //     editorPane.setFocusable(false);
        // } else {
        //     editorPane.setFocusable(true);
        // }

        // Setting tab-order and focus on okButton
        DialogFocusTraversalPolicy policy = new DialogFocusTraversalPolicy(new JComponent[] {okButton, editorScrollPane});
        this.setFocusTraversalPolicy(policy);  // set policy
        this.setFocusCycleRoot(true); // enable policy
    }

     private JEditorPane createEditorPane(String dialogText) {
        JEditorPane editorPane = new JEditorPane();
        editorPane.setEditable(false);
        editorPane.setContentType("text/html");
        editorPane.setText(dialogText);
        return editorPane;
    }

    public void actionPerformed (java.awt.event.ActionEvent evt) {
        setVisible(false);
        dispose();
    }

}
