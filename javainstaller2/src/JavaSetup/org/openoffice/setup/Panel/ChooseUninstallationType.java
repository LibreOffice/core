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

import org.openoffice.setup.Controller.ChooseUninstallationTypeCtrl;
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

public class ChooseUninstallationType extends JPanel {

    private JRadioButton custom;
    private JRadioButton complete;

    public ChooseUninstallationType() {

        InstallData data = InstallData.getInstance();

        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_ChooseUninstallationType1");
        String subtitleText = ResourceManager.getString("String_ChooseUninstallationType2");
        PanelTitle titleBox = new PanelTitle(titleText, subtitleText);
        titleBox.addVerticalStrut(20);
        add(titleBox, BorderLayout.NORTH);

        String borderTitle = ResourceManager.getString("String_ChooseUninstallationType1");
        TitledBorder PanelBorder = BorderFactory.createTitledBorder(borderTitle);

        JPanel contentPanel = new JPanel();
        contentPanel.setBorder(PanelBorder);
        contentPanel.setLayout(new GridBagLayout());
        if ( data.useRtl() ) { contentPanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            GridBagConstraints constraints = new GridBagConstraints();
            constraints.insets = new Insets(0, 0, 0, 10);
            // constraints.anchor = GridBagConstraints.NORTHWEST;

            String completeText = ResourceManager.getString("String_ChooseUninstallationType4");
            PanelLabel completeComment = new PanelLabel(completeText, true);
            if ( data.useRtl() ) { completeComment.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
            String customText  = ResourceManager.getString("String_ChooseUninstallationType5");
            PanelLabel customComment  = new PanelLabel(customText, true);
            if ( data.useRtl() ) { customComment.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            ButtonGroup group = new ButtonGroup();

            String completeButtonText = ResourceManager.getString("String_ChooseUninstallationType6");
            complete = new JRadioButton(completeButtonText, true);
            complete.setMnemonic(KeyEvent.VK_C);
            if ( data.useRtl() ) { complete.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
            String customButtonText  = ResourceManager.getString("String_ChooseUninstallationType3");
            custom  = new JRadioButton(customButtonText,  false);
            custom.setMnemonic(KeyEvent.VK_U);
            if ( data.useRtl() ) { custom.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

            group.add(complete);
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

        contentPanel.add(complete, constraints);

            constraints.gridx = 1;
            constraints.gridy = 1;
            constraints.weightx = 1;
            constraints.weighty = 0;
            constraints.fill = GridBagConstraints.HORIZONTAL;

        contentPanel.add(completeComment, constraints);

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

    public void setActionListener(ChooseUninstallationTypeCtrl actionListener) {
        complete.addActionListener(actionListener);
        custom.addActionListener(actionListener);
    }

    public void removeActionListener(ChooseUninstallationTypeCtrl actionListener) {
        complete.removeActionListener(actionListener);
        custom.removeActionListener(actionListener);
    }

    public void setCompleteActionCommand(String completeActionCommand) {
        complete.setActionCommand(completeActionCommand);
    }

    public void setCustomActionCommand(String customActionCommand) {
        custom.setActionCommand(customActionCommand);
    }

}
