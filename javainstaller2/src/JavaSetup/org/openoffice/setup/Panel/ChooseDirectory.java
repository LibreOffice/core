/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChooseDirectory.java,v $
 * $Revision: 1.3 $
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

package org.openoffice.setup.Panel;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;

public class ChooseDirectory extends JPanel implements ActionListener {

    private JFileChooser directoryChooser;
    private JFileChooser directoryChooserRootdir;
    private JButton      directoryButton;
    private JButton      directoryButtonRootdir;
    private JTextField   directoryField;
    private JTextField   directoryFieldRootdir;
    private PanelLabel   databaseProgress;
    private PanelTitle   titleBox;

    public ChooseDirectory() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_ChooseDirectory1");
        String subtitleText = ResourceManager.getString("String_ChooseDirectory2");

        titleBox = new PanelTitle(titleText, subtitleText);
        titleBox.addVerticalStrut(10);
        add(titleBox, BorderLayout.NORTH);

        Box contentBox   = new Box(BoxLayout.Y_AXIS);

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new GridBagLayout());

        directoryChooser = new JFileChooser();
        directoryChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

        String browseText = ResourceManager.getString("String_ChooseDirectory3");
        directoryButton = new JButton(browseText);
        directoryButton.addActionListener(this);

        directoryField = new JTextField();

        GridBagConstraints constraints = new GridBagConstraints();

        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.weightx = 1;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(directoryField, constraints);

        constraints.gridx = 1;
        constraints.gridy = 0;
        constraints.weightx = 0;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(directoryButton, constraints);

        constraints.gridx = 0;
        constraints.gridy = 1;
        constraints.weightx = 0;
        constraints.weighty = 1;
        constraints.fill = GridBagConstraints.VERTICAL;

        contentPanel.add(new JPanel(), constraints);

        contentBox.add(contentPanel);

        // new Panel for Solaris User installation
        InstallData data = InstallData.getInstance();

        if ( data.isSolarisUserInstallation() ) {

            Box contentBoxSolarisUser = new Box(BoxLayout.Y_AXIS);

            JPanel titlePanel = new JPanel();
            String subtitleTextRootdir = ResourceManager.getString("String_ChooseDirectory6");
            PanelLabel subTitleLabel = new PanelLabel(subtitleTextRootdir);
            titlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            titlePanel.add(subTitleLabel);

            contentBoxSolarisUser.add(titlePanel);
            contentBoxSolarisUser.add(contentBoxSolarisUser.createVerticalStrut(10));

            JPanel contentPanelRootdir = new JPanel();
            contentPanelRootdir.setLayout(new GridBagLayout());

            directoryChooserRootdir = new JFileChooser();
            directoryChooserRootdir.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

            String browseTextRootdir = ResourceManager.getString("String_ChooseDirectory3");
            directoryButtonRootdir = new JButton(browseTextRootdir);
            directoryButtonRootdir.addActionListener(this);

            directoryFieldRootdir = new JTextField();

            GridBagConstraints constraintsRootdir = new GridBagConstraints();

            constraintsRootdir.gridx = 0;
            constraintsRootdir.gridy = 0;
            constraintsRootdir.weightx = 1;
            constraintsRootdir.weighty = 0;
            constraintsRootdir.fill = GridBagConstraints.HORIZONTAL;

            contentPanelRootdir.add(directoryFieldRootdir, constraintsRootdir);

            constraintsRootdir.gridx = 1;
            constraintsRootdir.gridy = 0;
            constraintsRootdir.weightx = 0;
            constraintsRootdir.weighty = 0;
            constraintsRootdir.fill = GridBagConstraints.HORIZONTAL;

            contentPanelRootdir.add(directoryButtonRootdir, constraintsRootdir);

            constraintsRootdir.gridx = 0;
            constraintsRootdir.gridy = 1;
            constraintsRootdir.weightx = 0;
            constraintsRootdir.weighty = 1;
            constraintsRootdir.fill = GridBagConstraints.VERTICAL;

            contentPanelRootdir.add(new JPanel(), constraintsRootdir);

            contentBoxSolarisUser.add(contentPanelRootdir);

            contentBox.add(contentBoxSolarisUser);
        }

        // defining a place for text output
        databaseProgress = new PanelLabel(""); // planned for database progress
        contentBox.add(databaseProgress);

        add(contentBox, BorderLayout.CENTER);
    }

    public void setDatabaseText(String s) {
        databaseProgress.setText(s);
    }

    public void setTitleText(String s) {
        titleBox.setTitle(s);
    }

    public void setDirectory(String dir) {
        directoryField.setText(dir);
    }

    public void disableDirectoryField() {
        directoryField.setEditable(false) ;
    }

    public void disableBrowseButton() {
        directoryButton.setEnabled(false);
    }

    public void enableDirectoryField() {
        directoryField.setEditable(true) ;
    }

    public String getDirectory() {
        return directoryField.getText();
    }

    public void setRootDirectory(String dir) {
        directoryFieldRootdir.setText(dir);
    }

    public String getRootDirectory() {
        return directoryFieldRootdir.getText();
    }

    public void actionPerformed(ActionEvent e) {

        //Handle open button action.
        if (e.getSource() == directoryButton) {
            int ReturnValue = directoryChooser.showOpenDialog(ChooseDirectory.this);

            if (ReturnValue == JFileChooser.APPROVE_OPTION) {
                File file = directoryChooser.getSelectedFile();
                directoryField.setText(file.getAbsolutePath());
            } else {
                // do nothing for now
            }
        }

        if (e.getSource() == directoryButtonRootdir) {
            int ReturnValue = directoryChooserRootdir.showOpenDialog(ChooseDirectory.this);

            if (ReturnValue == JFileChooser.APPROVE_OPTION) {
                File file = directoryChooserRootdir.getSelectedFile();
                directoryFieldRootdir.setText(file.getAbsolutePath());
            } else {
                // do nothing for now
            }
        }

    }

}
