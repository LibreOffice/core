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
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.Insets;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.border.EmptyBorder;
import org.openoffice.setup.InstallData;

public class UninstallationImminent extends JPanel {

    private String infoText;
    private JEditorPane ProductInformation;
    private JScrollPane ProductPane;

    public UninstallationImminent() {

        InstallData data = InstallData.getInstance();

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titletext = ResourceManager.getString("String_UninstallationImminent1");
        PanelTitle titlebox = new PanelTitle(titletext);
        add(titlebox, BorderLayout.NORTH);

        JPanel contentpanel = new JPanel();
        contentpanel.setLayout(new java.awt.BorderLayout());
        if ( data.useRtl() ) { contentpanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        String text1 = ResourceManager.getString("String_UninstallationImminent2");
        PanelLabel label1 = new PanelLabel(text1);
        if ( data.useRtl() ) { label1.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        ProductInformation = new JEditorPane("text/html", getInfoText());
        ProductInformation.setEditable(false);
        if ( data.useRtl() ) { ProductInformation.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        ProductPane = new JScrollPane(ProductInformation);
        ProductPane.setPreferredSize(new Dimension(250, 145));
        ProductPane.setBorder(new EmptyBorder(10, 0, 10, 0));
        if ( data.useRtl() ) { ProductPane.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        contentpanel.add(label1, BorderLayout.NORTH);
        contentpanel.add(ProductPane, BorderLayout.CENTER);

        add(contentpanel, BorderLayout.CENTER);
    }

    public void setInfoText(String text) {
        infoText = text;
        updateInfoText();
    }

    public String getInfoText() {
        return infoText;
    }

    public void updateInfoText() {
        ProductInformation.setText(infoText);
    }

    public void setTabOrder() {
        JScrollBar ScrollBar = ProductPane.getVerticalScrollBar();
        if ( ScrollBar.isShowing() ) {
            ProductInformation.setFocusable(true);
        } else {
            ProductInformation.setFocusable(false);
        }
    }

    public void setCaretPosition() {
        ProductInformation.setCaretPosition(0);
    }

}
