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



package org.openoffice.setup.PanelHelper;

import java.awt.ComponentOrientation;
import java.awt.FlowLayout;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import org.openoffice.setup.InstallData;

public class PanelTitle extends Box {

    private JLabel TitleLabel;

    public PanelTitle() {
         super(BoxLayout.PAGE_AXIS);
    }

    public PanelTitle(String title, String subtitle, int rows, int columns) {
        super(BoxLayout.PAGE_AXIS);
        init(title, subtitle, rows, columns);
    }

    public PanelTitle(String title, String subtitle) {
        super(BoxLayout.PAGE_AXIS);
        init(title, subtitle, 0, 0);
    }

    public PanelTitle(String title) {
        super (BoxLayout.PAGE_AXIS);
        init(title, null, 0, 0);
    }

    public void addVerticalStrut(int strut) {
        add(createVerticalStrut(strut));
    }

    public void setTitle(String title) {
        TitleLabel.setText(title);
    }

    // public void setSubtitle(String subtitle) {
    //     SubtitleLabel.setText(subtitle);
    // }

    private void init(String title, String subtitle, int rows, int columns) {

        InstallData data = InstallData.getInstance();

        TitleLabel = new JLabel(title);
        TitleLabel.setFocusable(false);
        JPanel TitlePanel = new JPanel();
        if ( data.useRtl() ) {
            TitlePanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 0, 0));
            TitleLabel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
        } else {
            TitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        }
        TitlePanel.add(TitleLabel);

        add(createVerticalStrut(10));
        add(TitlePanel);
        add(createVerticalStrut(10));
        add(new JSeparator());
        add(createVerticalStrut(20));

        if (subtitle != null) {
            PanelLabel SubtitleLabel = null;
            if ( rows > 0 ) {
                SubtitleLabel = new PanelLabel(subtitle, rows, columns );
            } else {
                SubtitleLabel = new PanelLabel(subtitle);
            }
            SubtitleLabel.setFocusable(false);
            // PanelLabel SubtitleLabel = new PanelLabel(subtitle, true);
            JPanel SubtitlePanel = new JPanel();
            if ( data.useRtl() ) {
                SubtitlePanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 0, 0));
                SubtitleLabel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
            } else {
                SubtitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            }
            SubtitlePanel.add(SubtitleLabel);

            add(SubtitlePanel);
        }
    }
}