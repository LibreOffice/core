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



package org.openoffice.setup.Panel;

import org.openoffice.setup.Controller.ChooseInstallationTypeCtrl;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
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
import org.openoffice.setup.InstallData;

public class ChooseInstallationType extends JPanel {

    private JRadioButton custom;
    private JRadioButton typical;

    public ChooseInstallationType() {

        InstallData data = InstallData.getInstance();

        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_ChooseInstallationType1");
        String subtitleText = ResourceManager.getString("String_ChooseInstallationType2");
        PanelTitle titleBox = new PanelTitle(titleText, subtitleText);
        titleBox.addVerticalStrut(20);

        if ( data.useRtl() ) { titleBox.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        add(titleBox, BorderLayout.NORTH);

        String borderTitle = ResourceManager.getString("String_ChooseInstallationType3");
        TitledBorder PanelBorder = BorderFactory.createTitledBorder(borderTitle);

        JPanel contentPanel = new JPanel();
        contentPanel.setBorder(PanelBorder);
        contentPanel.setLayout(new GridBagLayout());
        if ( data.useRtl() ) { contentPanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            GridBagConstraints constraints = new GridBagConstraints();
            constraints.insets = new Insets(0, 0, 0, 10);
            // constraints.anchor = GridBagConstraints.NORTHWEST;

            String typicalText = ResourceManager.getString("String_ChooseInstallationType4");
            PanelLabel typicalComment = new PanelLabel(typicalText, true);
            if ( data.useRtl() ) { typicalComment.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
            String customText  = ResourceManager.getString("String_ChooseInstallationType5");
            PanelLabel customComment  = new PanelLabel(customText, true);
            if ( data.useRtl() ) { customComment.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            ButtonGroup group = new ButtonGroup();

            String typicalButtonText = ResourceManager.getString("String_ChooseInstallationType6");
            typical = new JRadioButton(typicalButtonText, true);
            typical.setMnemonic(KeyEvent.VK_C);
            if ( data.useRtl() ) { typical.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            String customButtonText  = ResourceManager.getString("String_ChooseInstallationType7");
            custom  = new JRadioButton(customButtonText,  false);
            custom.setMnemonic(KeyEvent.VK_U);
            if ( data.useRtl() ) { custom.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

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
