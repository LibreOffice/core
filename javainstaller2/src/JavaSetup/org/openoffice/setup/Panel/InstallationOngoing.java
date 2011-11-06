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
import org.openoffice.setup.SetupActionListener;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
import java.awt.Container;
import java.awt.Insets;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.border.EmptyBorder;
import org.openoffice.setup.InstallData;

public class InstallationOngoing extends JPanel {

    private PanelLabel currentProgress;
    private JProgressBar progressBar;
    private JButton mStopButton;
    private String mTitle = "";
    private PanelTitle mTitlebox;

    public InstallationOngoing() {

        InstallData data = InstallData.getInstance();

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        // String titleText = ResourceManager.getString("String_InstallationOngoing1");
        // PanelTitle titlebox = new PanelTitle(titleText);
        // PanelTitle titlebox = new PanelTitle(mTitle);
        mTitlebox = new PanelTitle(mTitle);
        mTitlebox.addVerticalStrut(20);
        add(mTitlebox, BorderLayout.NORTH);

        Container contentbox = Box.createVerticalBox();
        if ( data.useRtl() ) { contentbox.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        // String progressText = ResourceManager.getString("String_InstallationOngoing2");
        String progressText = "";
        currentProgress = new PanelLabel(progressText);

        Container innerbox = Box.createHorizontalBox();
        if ( data.useRtl() ) { innerbox.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        progressBar = new JProgressBar(0, 100);
        if ( data.useRtl() ) { progressBar.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        mStopButton = new JButton();
        String progressButtonText = ResourceManager.getString("String_InstallationOngoing3");
        mStopButton.setText(progressButtonText);
        mStopButton.setEnabled(true);
        if ( data.useRtl() ) { mStopButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        innerbox.add(progressBar);
        innerbox.add(Box.createHorizontalStrut(10));
        innerbox.add(mStopButton);

        contentbox.add(currentProgress);
        contentbox.add(Box.createVerticalStrut(10));
        contentbox.add(innerbox);
        contentbox.add(Box.createVerticalStrut(20));

        add(contentbox, BorderLayout.SOUTH);
    }

    public void setProgressText(String s) {
        currentProgress.setText(s);
    }

    public void setProgressValue(int i) {
        progressBar.setValue(i);
    }

    public void setTitle(String title) {
        mTitlebox.setTitle(title);
        mTitle = title;
    }

    public void setStopButtonActionCommand(String actionCommand) {
        mStopButton.setActionCommand(actionCommand);
    }

    public void addStopButtonActionListener(SetupActionListener actionListener) {
        mStopButton.addActionListener(actionListener);
    }

    public void setStopButtonEnabled(boolean enabled) {
        mStopButton.setEnabled(enabled);
    }

}
