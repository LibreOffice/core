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

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.SetupFrame;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Insets;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import org.openoffice.setup.ResourceManager;

public class DatabaseDialog extends JDialog {

    public DatabaseDialog(SetupFrame setupFrame) {

        // super(setupFrame.getDialog());

        String text1 = ResourceManager.getString("String_Analyzing_Database1");
        String text2 = ResourceManager.getString("String_Analyzing_Database2");

        setTitle(text1);
        this.getContentPane().setLayout(new java.awt.BorderLayout());

        JPanel toppanel = new JPanel();
        toppanel.setLayout(new java.awt.BorderLayout());
        toppanel.setBorder(new EmptyBorder(new Insets(50, 50, 50, 50)));

        PanelLabel label1 = new PanelLabel(text2);
        toppanel.add(label1, BorderLayout.NORTH);

        Container contentPane = this.getContentPane();
        contentPane.add(toppanel);
        // this.getContentPane().add(toppanel, BorderLayout.CENTER);
    }

}
