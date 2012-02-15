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



package org.openoffice.setup.Dialogs;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupFrame;
import org.openoffice.setup.Util.DialogFocusTraversalPolicy;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
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

        InstallData data = InstallData.getInstance();

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
        if ( data.useRtl() ) { toppanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        JPanel buttonpanel = new JPanel();
        buttonpanel.setLayout(new java.awt.FlowLayout());
        buttonpanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        if ( data.useRtl() ) { buttonpanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        //Create an editor pane.
        editorPane = createEditorPane();
        editorScrollPane = new JScrollPane(editorPane);
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        if ( data.useRtl() ) { editorScrollPane.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

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
        if ( data.useRtl() ) { okButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

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
