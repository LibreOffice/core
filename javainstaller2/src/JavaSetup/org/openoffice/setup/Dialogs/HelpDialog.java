/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: HelpDialog.java,v $
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

import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupFrame;
import org.openoffice.setup.Util.DialogFocusTraversalPolicy;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.ActionListener;
import java.io.File;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.border.EmptyBorder;

public class HelpDialog extends JDialog implements ActionListener {

    private JButton okButton;
    private JEditorPane editorPane;
    private JScrollPane editorScrollPane;
    private String helpFileName;
    private String helpFileString;

    public HelpDialog(SetupFrame setupFrame) {

        super(setupFrame.getDialog());

        helpFileString = setupFrame.getCurrentPanel().getHelpFileName();
        helpFileName = ResourceManager.getFileName(helpFileString);
        // String dialogName = setupFrame.getCurrentPanel().getName();

        String helpTitle = ResourceManager.getString("String_Help");
        setTitle(helpTitle);
        // setLayout(new java.awt.BorderLayout());
        this.getContentPane().setLayout(new java.awt.BorderLayout());

        JPanel toppanel = new JPanel();
        toppanel.setLayout(new java.awt.BorderLayout());
        toppanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        JPanel buttonpanel = new JPanel();
        buttonpanel.setLayout(new java.awt.FlowLayout());
        buttonpanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        //Create an editor pane.
        editorPane = createEditorPane();
        editorScrollPane = new JScrollPane(editorPane);
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));

        // String helpTitle1 = null;
        // InstallData data = InstallData.getInstance();
        // if ( data.isInstallationMode() ) {
        //     helpTitle1 = ResourceManager.getString("String_Help_Title_1");
        // } else {
        //     helpTitle1 = ResourceManager.getString("String_Help_Title_1_Uninstallation");
        // }

        // PanelLabel label1 = new PanelLabel(helpTitle1, true);
        // String helpTitle2 = ResourceManager.getString("String_Help_Title_2");
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

        // Setting tab-order and focus on okButton
        DialogFocusTraversalPolicy policy = new DialogFocusTraversalPolicy(new JComponent[] {okButton, editorScrollPane});
        this.setFocusTraversalPolicy(policy);  // set policy
        this.setFocusCycleRoot(true); // enable policy
    }

     private JEditorPane createEditorPane() {
        JEditorPane editorPane = new JEditorPane();
        editorPane.setEditable(false);

        InstallData data = InstallData.getInstance();
        File htmlDirectory = data.getInfoRoot("html");

        if ( htmlDirectory != null) {
            File htmlFile = new File(htmlDirectory, helpFileName);
            if (! htmlFile.exists()) {
                System.err.println("Couldn't find file: " + htmlFile.toString());
            }

            try {
                // System.err.println("URLPath: " + htmlFile.toURL());
                editorPane.setContentType("text/html;charset=utf-8");
                editorPane.setPage(htmlFile.toURL());
            } catch (Exception e) {
                e.printStackTrace();
                System.err.println("Attempted to read a bad URL");
            }
        }
        else {
            System.err.println("Did not find html directory");
        }

        return editorPane;
    }

//    public void setTabForScrollPane() {
//        JScrollBar ScrollBar = editorScrollPane.getVerticalScrollBar();
//        editorPane.setFocusable(true);
//        if ( ScrollBar.isShowing() ) {
//        } else {
//            editorPane.setFocusable(false);
//        }
//    }

    public void actionPerformed (java.awt.event.ActionEvent evt) {
        setVisible(false);
        dispose();
    }

}
