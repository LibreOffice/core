/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.idesupport.ui;

import java.io.File;
import java.io.IOException;

import java.util.Vector;
import java.util.Enumeration;

import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.AbstractButton;
import javax.swing.ImageIcon;
import javax.swing.border.LineBorder;

import java.awt.BorderLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.sun.star.script.framework.container.ParcelDescriptor;
import com.sun.star.script.framework.container.ScriptEntry;

import org.openoffice.idesupport.zip.ParcelZipper;

public class ConfigurePanel extends JPanel {

    private File basedir;
    private Vector<String> classpath;
    private ParcelDescriptor descriptor;

    private MethodPanel methodPanel;
    private ScriptPanel scriptPanel;

    public static final String DIALOG_TITLE =
        "Choose What to Export as Scripts";

    public ConfigurePanel(String basedir, Vector<String> classpath,
        ParcelDescriptor descriptor) {

        this.basedir = new File(basedir);
        this.classpath = classpath;
        this.descriptor = descriptor;
        initUI();
    }

    public ConfigurePanel(String basedir, Vector<String> classpath)
        throws IOException {

        this.basedir = new File(basedir);
        this.classpath = classpath;
        this.descriptor = new ParcelDescriptor(new File(this.basedir,
            ParcelZipper.PARCEL_DESCRIPTOR_XML));
        initUI();
    }

    public void reload(String basedir, Vector<String> classpath,
        ParcelDescriptor descriptor) {

        if (basedir != null)
            this.basedir = new File(basedir);

        if (classpath != null)
            this.classpath = classpath;

        if (descriptor != null) {
            descriptor = descriptor;
        }

        methodPanel.reload(this.basedir, this.classpath,
            descriptor.getLanguage());
        scriptPanel.reload(descriptor.getScriptEntries());
    }

    public void reload(String basedir, Vector<String> classpath)
        throws IOException {

        if (basedir != null)
            this.basedir = new File(basedir);

        if (classpath != null)
            this.classpath = classpath;

        this.descriptor = new ParcelDescriptor(new File(this.basedir,
            ParcelZipper.PARCEL_DESCRIPTOR_XML));

        methodPanel.reload(this.basedir, this.classpath,
            descriptor.getLanguage());
        scriptPanel.reload(descriptor.getScriptEntries());
    }

    public ParcelDescriptor getConfiguration() throws Exception {
        Enumeration<ScriptEntry> scripts = scriptPanel.getScriptEntries();
        descriptor.setScriptEntries(scripts);
        return descriptor;
    }

    private void initUI() {

        JPanel leftPanel = new JPanel();
        JPanel methodButtons = initMethodButtons();
        methodPanel = new MethodPanel(basedir, classpath, descriptor.getLanguage());

        leftPanel.setLayout(new BorderLayout());
        leftPanel.add(methodPanel, BorderLayout.CENTER);

        JPanel rightPanel = new JPanel();
        JPanel scriptButtons = initScriptButtons();
        scriptPanel = new ScriptPanel(descriptor.getScriptEntries());

        rightPanel.setLayout(new BorderLayout());
        rightPanel.add(scriptPanel, BorderLayout.CENTER);
        rightPanel.add(scriptButtons, BorderLayout.SOUTH);

        setLayout(new GridBagLayout());
        setPreferredSize(new java.awt.Dimension(700, 300));
        setBorder(LineBorder.createBlackLineBorder());

        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.ipadx = 40;
        gbc.anchor = java.awt.GridBagConstraints.WEST;
        gbc.insets = new Insets(10, 5, 5, 5);
        gbc.weightx = 0.75;
        add(leftPanel, gbc);

        gbc = new java.awt.GridBagConstraints();
        gbc.gridx = 1;
        gbc.gridy = 0;
        add(methodButtons, gbc);

        gbc = new java.awt.GridBagConstraints();
        gbc.gridx = 2;
        gbc.gridy = 0;
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.anchor = java.awt.GridBagConstraints.EAST;
        gbc.insets = new Insets(10, 5, 5, 5);
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        add(rightPanel, gbc);
    }

    private JPanel initMethodButtons() {
        JPanel panel = new JPanel();
        panel.setLayout(new GridBagLayout());
        ImageIcon icon = new ImageIcon(getClass().getResource("/org/openoffice/idesupport/ui/add.gif"));
        JButton addButton = new JButton("Add", icon);
        addButton.setHorizontalTextPosition(AbstractButton.LEFT);

        addButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.addScriptEntries(methodPanel.getSelectedEntries());
                }
            }
        );

        GridBagConstraints gbc = new java.awt.GridBagConstraints();
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(5, 5, 5, 5);
        panel.add(addButton, gbc);

        JPanel dummyPanel = new JPanel();
        gbc = new java.awt.GridBagConstraints();
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.gridheight = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        panel.add(dummyPanel, gbc);

        return panel;
    }

    private JPanel initScriptButtons() {
        JPanel panel = new JPanel();
        JButton removeButton = new JButton("Remove");
        JButton removeAllButton = new JButton("Remove All");

        removeButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.removeSelectedRows();
                }
            }
        );

        removeAllButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.removeAllRows();
                }
            }
        );

        panel.add(removeButton);
        panel.add(removeAllButton);

        return panel;
    }
}
