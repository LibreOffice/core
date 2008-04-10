/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChooseInstallationType.java,v $
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

import org.openoffice.setup.Controller.ChooseInstallationTypeCtrl;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.KeyEvent;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;

public class ChooseInstallationType extends JPanel {

    private JRadioButton custom;
    private JRadioButton typical;

    public ChooseInstallationType() {

        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_ChooseInstallationType1");
        String subtitleText = ResourceManager.getString("String_ChooseInstallationType2");
        PanelTitle titleBox = new PanelTitle(titleText, subtitleText);
        titleBox.addVerticalStrut(20);
        add(titleBox, BorderLayout.NORTH);

        String borderTitle = ResourceManager.getString("String_ChooseInstallationType3");
        TitledBorder PanelBorder = BorderFactory.createTitledBorder(borderTitle);

        JPanel contentPanel = new JPanel();
        contentPanel.setBorder(PanelBorder);
        contentPanel.setLayout(new GridBagLayout());

            GridBagConstraints constraints = new GridBagConstraints();
            constraints.insets = new Insets(0, 0, 0, 10);
            // constraints.anchor = GridBagConstraints.NORTHWEST;

            String typicalText = ResourceManager.getString("String_ChooseInstallationType4");
            PanelLabel typicalComment = new PanelLabel(typicalText, true);
            String customText  = ResourceManager.getString("String_ChooseInstallationType5");
            PanelLabel customComment  = new PanelLabel(customText, true);

            ButtonGroup group = new ButtonGroup();

            String typicalButtonText = ResourceManager.getString("String_ChooseInstallationType6");
            typical = new JRadioButton(typicalButtonText, true);
            typical.setMnemonic(KeyEvent.VK_C);
            String customButtonText  = ResourceManager.getString("String_ChooseInstallationType7");
            custom  = new JRadioButton(customButtonText,  false);
            custom.setMnemonic(KeyEvent.VK_U);

            group.add(typical);
            group.add(custom);

            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.weightx = 0;
            constraints.weighty = 1;
            constraints.fill = GridBagConstraints.VERTICAL;

        contentPanel.add(new JPanel(), constraints);

            constraints.gridx = 0;
            constraints.gridy = 1;
            constraints.weightx = 0;
            constraints.weighty = 0;
            constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(typical, constraints);

            constraints.gridx = 1;
            constraints.gridy = 1;
            constraints.weightx = 1;
            constraints.weighty = 0;
            constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(typicalComment, constraints);

            constraints.gridx = 0;
            constraints.gridy = 2;
            constraints.weightx = 0;
            constraints.weighty = 1;
            constraints.fill = GridBagConstraints.VERTICAL;

        contentPanel.add(new JPanel(), constraints);

            constraints.gridx = 0;
            constraints.gridy = 3;
            constraints.weightx = 0;
            constraints.weighty = 0;
            constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(custom, constraints);

            constraints.gridx = 1;
            constraints.gridy = 3;
            constraints.weightx = 1;
            constraints.weighty = 0;
            constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(customComment, constraints);

            constraints.gridx = 0;
            constraints.gridy = 4;
            constraints.weightx = 0;
            constraints.weighty = 1;
            constraints.fill = GridBagConstraints.VERTICAL;

        contentPanel.add(new JPanel(), constraints);

        add(contentPanel, BorderLayout.CENTER);
    }

    public void setActionListener(ChooseInstallationTypeCtrl actionListener) {
        typical.addActionListener(actionListener);
        custom.addActionListener(actionListener);
    }

    public void removeActionListener(ChooseInstallationTypeCtrl actionListener) {
        typical.removeActionListener(actionListener);
        custom.removeActionListener(actionListener);
    }

    public void setTypicalActionCommand(String typicalActionCommand) {
        typical.setActionCommand(typicalActionCommand);
    }

    public void setCustomActionCommand(String customActionCommand) {
        custom.setActionCommand(customActionCommand);
    }

}
