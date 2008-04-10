/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PanelTitle.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.setup.PanelHelper;

import java.awt.FlowLayout;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;

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

        TitleLabel = new JLabel(title);
        TitleLabel.setFocusable(false);
        JPanel TitlePanel = new JPanel();
        TitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
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
            SubtitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            SubtitlePanel.add(SubtitleLabel);

            add(SubtitlePanel);
        }
    }
}