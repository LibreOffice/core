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

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
import java.awt.Insets;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public class Prologue extends JPanel {

    public Prologue() {

        InstallData data = InstallData.getInstance();

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText = ResourceManager.getString("String_Prologue1");
        PanelTitle titleBox = new PanelTitle(titleText);
        if ( data.useRtl() ) { titleBox.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        add(titleBox, BorderLayout.NORTH);

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_Prologue2");
        PanelLabel label1 = new PanelLabel(text1, true);
        if ( data.useRtl() ) { label1.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        String text2 = ResourceManager.getString("String_Prologue3");
        PanelLabel label2 = new PanelLabel(text2);
        if ( data.useRtl() ) { label2.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        contentPanel.add(label1, BorderLayout.NORTH);
        contentPanel.add(label2, BorderLayout.CENTER);

        if ( data.isUserInstallation() ) {
            String text3 = ResourceManager.getString("String_Prologue4");
            PanelLabel label3 = new PanelLabel(text3, true);
            if ( data.useRtl() ) { label3.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
            contentPanel.add(label3, BorderLayout.SOUTH);
        }

        add(contentPanel, BorderLayout.CENTER);
    }
}
