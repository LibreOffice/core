/*************************************************************************
 *
 *  $RCSfile: ConfigurePanel.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:45:52 $
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

package org.openoffice.idesupport.ui;

import java.io.File;
import java.io.IOException;

import java.util.Vector;
import java.util.Enumeration;

import javax.swing.JFrame;
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

import org.w3c.dom.Document;

import com.sun.star.script.framework.browse.ScriptEntry;
import com.sun.star.script.framework.browse.ParcelDescriptor;

import org.openoffice.idesupport.zip.ParcelZipper;

public class ConfigurePanel extends JPanel {

    private File basedir;
    private Vector classpath;
    private ParcelDescriptor descriptor;

    private MethodPanel methodPanel;
    private ScriptPanel scriptPanel;

    public static final String DIALOG_TITLE =
        "Choose What to Export as Scripts";

    public ConfigurePanel(String basedir, Vector classpath,
        ParcelDescriptor descriptor) {

        this.basedir = new File(basedir);
        this.classpath = classpath;
        this.descriptor = descriptor;
        initUI();
    }

    public ConfigurePanel(String basedir, Vector classpath)
        throws IOException {

        this.basedir = new File(basedir);
        this.classpath = classpath;
        this.descriptor = new ParcelDescriptor(new File(this.basedir,
            ParcelZipper.PARCEL_DESCRIPTOR_XML));
        initUI();
    }

    public void reload(String basedir, Vector classpath,
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

    public void reload(String basedir, Vector classpath)
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
        Enumeration scripts = scriptPanel.getScriptEntries();
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
