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

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Insets;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public class UninstallationPrologue extends JPanel {

    public UninstallationPrologue() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_Prologue1");
        PanelTitle titleBox = new PanelTitle(titleText);
        add(titleBox, BorderLayout.NORTH);

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_UninstallationPrologue2");
        PanelLabel label1 = new PanelLabel(text1, true);
        String text2 = ResourceManager.getString("String_Prologue3");
        PanelLabel label2 = new PanelLabel(text2);

        contentPanel.add(label1, BorderLayout.NORTH);
        contentPanel.add(label2, BorderLayout.CENTER);

        add(contentPanel, BorderLayout.CENTER);
    }
}
